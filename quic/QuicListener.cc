#include "QuicListener.h"

#include "log/Logger.h"
#include "net/UdpStream.h"
#include "QuicConfig.h"
#include "QuicConnection.h"
#include "RandomFile.h"

using namespace baize;

struct net::QuicListenerData
{
    QuicListenerData()
      : type(0),
        version(0),
        scid(),
        scid_len(sizeof(scid)),
        dcid(),
        dcid_len(sizeof(dcid)),
        odcid(),
        odcid_len(sizeof(odcid)),
        token(),
        token_len(sizeof(token))
    {
        memZero(&peeraddr, sizeof(peeraddr));
    }

    uint8_t type;
    uint32_t version;
    uint8_t scid[kMaxConnIdLen];
    size_t scid_len;
    uint8_t dcid[kMaxConnIdLen];
    size_t dcid_len;
    uint8_t odcid[kMaxConnIdLen];
    size_t odcid_len;
    uint8_t token[kMaxTokenLen];
    size_t token_len;
    InetAddress peeraddr;
};

net::QuicListener::QuicListener(uint16_t port)
  : udpstream_(UdpStream::asServer(port)),
    localaddr_(udpstream_->getLocalAddr()),
    config_(std::make_shared<QuicConfig>(QUICHE_PROTOCOL_VERSION))
{
    config_->setCertAndKey("../quic/cert.crt", "../quic/cert.key");
    config_->setServerConfig();
}

net::QuicListener::~QuicListener()
{
}

void net::QuicListener::loopAndAccept()
{
    while (1) {
        QuicListenerData data;

        int rn = udpstream_->asyncRecvfrom(quicReadBuffer, sizeof(quicReadBuffer), &data.peeraddr);
        if (rn < 0) {
            continue;
        }

        int rc = quiche_header_info(quicReadBuffer, rn, kConnIdLen, &data.version,
                                    &data.type, data.scid, &data.scid_len, data.dcid, &data.dcid_len,
                                    data.token, &data.token_len);
        if (rc < 0) {
            LOG_ERROR << "failed to parse header: " << rc;
            continue;
        }

        QuicConnId dconnid(data.dcid, data.dcid + data.dcid_len);
        if (conns_.find(dconnid) != conns_.end()) {
            auto& conn = conns_[dconnid];
            conn->quicConnRead(quicReadBuffer, rn, data.peeraddr);
        } else {
            bool ret = quicNegotiate(&data);
            if (!ret) {
                continue;
            } 

            ret = validateToken(&data);
            if (!ret) {
                continue;
            }

            QuicConnSptr conn = quicAccept(&data);
            if (!conn) {
                continue;
            } else {
                QuicConnId key(data.dcid, data.dcid + data.dcid_len);
                conns_[key] = conn;
                conn->quicConnRead(quicReadBuffer, rn, data.peeraddr);
            }
        }
    }
}

bool net::QuicListener::quicNegotiate(QuicListenerData* data)
{
    LOG_INFO << "quicNegitiate";
    if (quiche_version_is_supported(data->version)) {
        LOG_INFO << "quicNegitiate sucess";
        return true;
    }
    ssize_t written = quiche_negotiate_version(data->scid, data->scid_len,
                                               data->dcid, data->dcid_len,
                                              quicWriteBuffer, sizeof(quicWriteBuffer));
    if (written < 0) {
        LOG_ERROR << "failed to create vneg packet: " << written;
        return false;
    }

    int sent = udpstream_->asyncSendto(quicWriteBuffer, static_cast<int>(written), data->peeraddr);
    if (sent != written) {
        LOG_ERROR << "failed to send";
        return false;
    }
    return false;
}

static void mint_token(const uint8_t* dcid, size_t dcid_len,
                       struct sockaddr* addr, socklen_t addr_len,
                       uint8_t* token, size_t* token_len)
{
    memcpy(token, "quiche", sizeof("quiche") - 1);
    memcpy(token + sizeof("quiche") - 1, addr, addr_len);
    memcpy(token + sizeof("quiche") - 1 + addr_len, dcid, dcid_len);
    *token_len = sizeof("quiche") - 1 + addr_len + dcid_len;
}

static bool validate_token(const uint8_t* token, size_t token_len,
                           struct sockaddr* addr, socklen_t addr_len,
                           uint8_t* odcid, size_t* odcid_len) {
    if ((token_len < sizeof("quiche") - 1) ||
         memcmp(token, "quiche", sizeof("quiche") - 1)) {
        return false;
    }

    token += sizeof("quiche") - 1;
    token_len -= sizeof("quiche") - 1;

    if ((token_len < addr_len) || memcmp(token, addr, addr_len)) {
        return false;
    }

    token += addr_len;
    token_len -= addr_len;

    if (*odcid_len < token_len) {
        return false;
    }

    memcpy(odcid, token, token_len);
    *odcid_len = token_len;

    return true;
}

bool net::QuicListener::validateToken(QuicListenerData* data)
{
    LOG_INFO << "validateToken";
    if (data->token_len == 0) {
        mint_token(data->dcid, data->dcid_len,
                   data->peeraddr.getSockAddr(), data->peeraddr.getSockLen(),
                   data->token, &data->token_len);

        uint8_t new_cid[kConnIdLen];
        bool ret = RandomFile::getInstance().genRandom(new_cid, sizeof(new_cid));
        if (!ret) {
            return false;
        }

        ssize_t written = quiche_retry(data->scid, data->scid_len,
                                       data->dcid, data->dcid_len,
                                       new_cid, sizeof(new_cid),
                                       data->token, data->token_len,
                                       data->version, quicWriteBuffer, sizeof(quicWriteBuffer));
        if (written < 0) {
            LOG_ERROR << "failed to create retry packet: " << written;
            return false;
        }

        int sent = udpstream_->asyncSendto(quicWriteBuffer, static_cast<int>(written), data->peeraddr);
        if (sent != written) {
            LOG_ERROR << "failed to send";
            return false;
        }
        return false;
    }

    if (!validate_token(data->token, data->token_len,
                        data->peeraddr.getSockAddr(), data->peeraddr.getSockLen(),
                        data->odcid, &data->odcid_len)) {
        LOG_ERROR << "invalid address vallidation token";
        return false;
    }
    LOG_INFO << "validateToken sucess";
    return true;
}

net::QuicConnSptr net::QuicListener::quicAccept(QuicListenerData* data)
{
    assert(data->scid_len == kConnIdLen);
    quiche_conn* conn = quiche_accept(data->dcid, data->dcid_len,
                                      data->odcid, data->odcid_len,
                                      localaddr_.getSockAddr(),
                                      localaddr_.getSockLen(),
                                      data->peeraddr.getSockAddr(),
                                      data->peeraddr.getSockLen(),
                                      config_->getConfig());
    if (conn == nullptr) {
        return QuicConnSptr();
    }

    return QuicConnSptr(std::make_shared<QuicConnection>(udpstream_, localaddr_,
                                                         data->peeraddr, config_, conn));
}