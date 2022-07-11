#ifndef BAIZE_QUICTYPE_H
#define BAIZE_QUICTYPE_H

#include <memory>
#include "quiche.h"
#include <vector>

namespace baize
{

namespace net
{

class QuicConnection;
typedef std::shared_ptr<QuicConnection> QuicConnSptr;

class QuicConfig;
typedef std::shared_ptr<QuicConfig> QuicConfigSptr;

const int kConnIdLen = 16;
const int kMaxTokenLen =  sizeof("quiche") - 1 + sizeof(sockaddr_storage) + QUICHE_MAX_CONN_ID_LEN;
const int kMaxDatagramSize = 1350;
const int kMaxConnIdLen = 20;

typedef std::vector<uint8_t> QuicConnId;
typedef std::vector<uint8_t> Token;

} // namespace net
    
} // namespace baize

#endif //BAIZE_QUICTYPE_H