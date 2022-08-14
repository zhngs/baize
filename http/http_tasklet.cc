#include "http/http_tasklet.h"

#include "log/logger.h"

namespace baize
{

namespace net
{

int HttpTasklet::Parse(Buffer* buf)
{
    while (1) {
        auto req = SliceMessage(buf);
        if (req.err < 0) {
            return -1;
        } else if (req.err == 0) {
            return static_cast<int>(requests_.size());
        } else {
            cur_request_.all_data_ = req.ret;
            while (1) {
                TaskletState state = NextStep(req.ret);
                if (state == kContinue) {
                    continue;
                } else if (state == kFinish) {
                    requests_.push_back(cur_request_);
                    cur_request_.Reset();
                    break;
                } else if (state == kError) {
                    return -1;
                }
            }
        }
    }
}

Result<StringPiece, int> HttpTasklet::SliceMessage(Buffer* buf)
{
    // 判断是否有一条完整消息
    StringPiece buffer(buf->read_index(), buf->readable_bytes());
    const char* pos = buffer.Find("\r\n\r\n");
    if (pos == buffer.end()) return {StringPiece(), 0};

    StringPiece head(buffer.begin(), pos);
    const char* plen = head.Find("Content-Length");
    if (plen == head.end()) {
        // 得到一条没有body的消息
        buf->TakeUntil(pos + 4);
        return {StringPiece(head.begin(), head.end() + 4), 1};
    } else {
        // 取Content-Length header
        head.RemovePrefixUntil(plen);
        const char* head_end = head.Find("\r\n");
        if (head_end == head.end()) return {StringPiece(), -1};
        head.RemoveSuffix(static_cast<int>(head.end() - head_end));

        // 对header做切分，解析body长度
        auto slice_vector = head.Split(':');
        if (slice_vector.size() != 2) return {StringPiece(), -1};
        auto res = slice_vector[1].ParseInt();
        if (res.err != slice_vector[1].end()) return {StringPiece(), -1};

        int content_length = res.ret;
        StringPiece body(pos + 4, buffer.end());
        if (body.size() < content_length) {
            return {StringPiece(), 0};
        } else {
            buf->TakeUntil(body.begin() + content_length);
            return {StringPiece(buffer.begin(), body.begin() + content_length),
                    1};
        }
    }
}

HttpTasklet::TaskletState HttpTasklet::NextStep(StringPiece& message)
{
    switch (cur_state_) {
        case kHandleRequestLine:
            return HandleRequestLine(message);
        case kHandleHeaders:
            return HandleHeaders(message);
        case kHandleBody:
            return HandleBody(message);
        case kHandleComplete:
            return HandleComplete(message);

        case kHandleError:
        default:
            return kError;
    }
}

HttpTasklet::TaskletState HttpTasklet::HandleRequestLine(StringPiece& message)
{
    const char* pos = message.Find("\r\n");
    if (pos == message.end()) return kError;

    StringPiece requestline(message.begin(), pos + 2);
    message.RemovePrefixUntil(requestline.end());

    requestline.TrimSpace();
    auto items = requestline.Split(' ');
    if (items.size() != 3) {
        cur_state_ = kHandleError;
        return kError;
    }

    cur_request_.method_ = items[0];
    cur_request_.path_ = items[1];
    cur_request_.version_ = items[2];

    LOG_DEBUG << "get method:" << cur_request_.method_
              << ", path:" << cur_request_.path_
              << ", version:" << cur_request_.version_;

    cur_state_ = kHandleHeaders;
    return kContinue;
}

HttpTasklet::TaskletState HttpTasklet::HandleHeaders(StringPiece& message)
{
    const char* crlf = message.Find("\r\n\r\n");
    if (crlf == message.end()) return kError;

    StringPiece headers(message.begin(), crlf + 4);
    message.RemovePrefixUntil(headers.end());

    headers.TrimSpace();
    auto header_vector = headers.Split("\r\n");
    for (auto& head : header_vector) {
        const char* pos = head.Find(':');
        if (pos == head.end()) {
            cur_state_ = kHandleError;
            return kError;
        }

        StringPiece key(head.begin(), pos);
        key.TrimSpace();
        StringPiece value(pos + 1, head.end());
        value.TrimSpace();
        LOG_DEBUG << "get head {" << key << ":" << value << "}";

        // todo: do check
        cur_request_.headers_[key] = value;
    }

    cur_state_ = kHandleBody;
    return kContinue;
}

HttpTasklet::TaskletState HttpTasklet::HandleBody(StringPiece& message)
{
    if (cur_request_.headers_.find("Content-Length") ==
        cur_request_.headers_.end()) {
        cur_request_.body_.set(message.begin(), message.begin());
        cur_state_ = kHandleComplete;
        return kContinue;
    }

    StringPiece content_length(cur_request_.headers_["Content-Length"]);
    auto res = content_length.ParseInt();
    if (res.err != content_length.end()) {
        cur_state_ = kHandleError;
        return kError;
    }

    if (message.size() < res.ret) {
        return kError;
    } else {
        cur_request_.body_.set(message.begin(), res.ret);
        LOG_DEBUG << "get body {" << cur_request_.body_ << "}";
        cur_state_ = kHandleComplete;
        return kContinue;
    }
}

HttpTasklet::TaskletState HttpTasklet::HandleComplete(StringPiece& message)
{
    if (cur_request_.body_.end() == message.end()) {
        return kFinish;
    } else {
        return kError;
    }
}

}  // namespace net

}  // namespace baize
