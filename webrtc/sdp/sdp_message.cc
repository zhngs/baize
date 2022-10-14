#include "webrtc/sdp/sdp_message.h"

#include "log/logger.h"

namespace baize
{

namespace net
{

thread_local SdpMessage gt_pub_sdp;
SdpMessage& current_pub_sdp() { return gt_pub_sdp; }
thread_local SdpMessage gt_sub_sdp;
SdpMessage& current_sub_sdp() { return gt_sub_sdp; }

int SdpMessage::set_remote_sdp(StringPiece message)
{
    StringPiece ice_ufrag = message.SliceFragment("ice-ufrag:", "\r\n");
    if (!ice_ufrag.empty()) {
        net_.ice_ufrag_ = ice_ufrag.AsString();
    }

    StringPiece ice_pwd = message.SliceFragment("ice-pwd:", "\r\n");
    if (!ice_pwd.empty()) {
        net_.ice_pwd_ = ice_pwd.AsString();
    }

    StringPiece finger_print = message.SliceFragment("fingerprint:", "\r\n");
    if (!finger_print.empty()) {
        net_.finger_print_ = finger_print.AsString();
    }

    LOG_INFO << "ice-ufrag:" << net_.ice_ufrag_;
    LOG_INFO << "ice-pwd:" << net_.ice_pwd_;
    LOG_INFO << "fingerprint:" << net_.finger_print_;

    StringPiece cname = message.SliceFragment("cname:", "\r\n");
    if (!cname.empty()) {
        cname_ = cname.AsString();
    }

    LOG_INFO << "cname:" << cname_;

    StringPiece media_slice = message;
    const char* media_m = media_slice.Find("m=");
    media_slice.RemovePrefixUntil(media_m);
    auto media_vector = media_slice.Split("m=");
    for (auto& media_item : media_vector) {
        TrackSdp track;
        if (media_item.StartsWith("audio")) {
            track.media_type_ = "audio";
        } else if (media_item.StartsWith("video")) {
            track.media_type_ = "video";
        } else {
            track.media_type_ = "unknow";
            continue;
        }

        const char* ssrc_group = media_item.Find("ssrc-group:FID");
        if (ssrc_group == media_item.end()) {
            StringPiece ssrc_id = media_item.SliceFragment("ssrc:", " ");
            if (!ssrc_id.empty()) {
                track.ssrc_group_.push_back(ssrc_id.AsString());
            }
        } else {
            StringPiece ssrc_ids =
                media_item.SliceFragment("ssrc-group:FID", "\r\n");
            if (!ssrc_ids.empty()) {
                auto ssrc_vector = ssrc_ids.Split(" ");
                for (auto& ssrc_id : ssrc_vector) {
                    track.ssrc_group_.push_back(ssrc_id.AsString());
                }
            }
        }

        tracks_.push_back(std::move(track));
    }

    for (auto& track : tracks_) {
        LOG_INFO << "media_type:" << track.media_type_;
        for (auto& ssrc : track.ssrc_group_) {
            LOG_INFO << "ssrc:" << ssrc;
        }
    }

    return 0;
}

thread_local char t_local_sdp[8192];
string SdpMessage::local_sdp()
{
    int ssrc = 1234;
    snprintf(t_local_sdp,
             sizeof(t_local_sdp),
             "v=0\r\n"
             "o=- 1495799811084970 1495799811084970 IN IP4 0.0.0.0\r\n"
             "s=baize webrtc sfu server\r\n"
             "c=IN IP4 0.0.0.0\r\n"
             "t=0 0\r\n"
             "a=ice-options:%s\r\n"
             "a=msid-semantic: WMS baize\r\n"
             "a=group:BUNDLE 0\r\n"
             "m=video 9 RTP/SAVPF 127\r\n"
             "a=rtpmap:127 H264/90000\r\n"
             "a=setup:passive\r\n"
             "a=mid:0\r\n"
             "a=sendrecv\r\n"
             "a=ice-ufrag:%s\r\n"
             "a=ice-pwd:%s\r\n"
             "a=fingerprint:sha-256 %s\r\n"
             "a=msid:baize baizev0\r\n"
             "a=ssrc:%d cname:baizevideo\r\n"
             "a=ssrc:%d msid:baize baizev0\r\n"
             "a=candidate:1 1 udp 1 %s %d typ host\r\n"
             "a=rtcp-mux\r\n"
             "a=rtcp-rsize\r\n",
             net_.ice_option_.c_str(),
             net_.ice_ufrag_.c_str(),
             net_.ice_pwd_.c_str(),
             net_.finger_print_.c_str(),
             ssrc,
             ssrc,
             net_.ip_.c_str(),
             net_.port_);
    return t_local_sdp;
}

}  // namespace net

}  // namespace baize
