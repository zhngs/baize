#include "http_message.h"

namespace baize
{

namespace net
{

string HttpMessage::MakeResponseLine(ResponseLine& line)
{
    string rsp;
    if (line.version == Version::kHttp11) {
        rsp += "HTTP/1.1 ";
    } else {
        return "";
    }

    if (line.code == StatusCode::k200) {
        rsp += "200 ";
    } else {
        return "";
    }

    if (line.description == StatusDescription::kOK) {
        rsp += "OK";
    } else {
        return "";
    }

    return rsp;
}

static int CheckCompleted(StringPiece message)
{
    // 判断是否有一条完整消息
    const char* pos = message.Find("\r\n\r\n");
    if (pos == message.end()) return 0;

    StringPiece length_header(message.begin(), pos);
    const char* header_begin = length_header.Find("Content-Length");
    if (header_begin == length_header.end()) {
        // 得到一条没有body的消息
        return static_cast<int>(pos - message.begin() + 4);
    } else {
        // 取Content-Length header
        length_header.RemovePrefixUntil(header_begin);
        const char* header_end = length_header.Find("\r\n");
        if (header_end == length_header.end()) return -1;
        length_header.RemoveSuffixUntil(header_end);

        const char* tmp = length_header.Find(':');
        if (tmp == length_header.end()) return -1;

        StringPiece length(tmp + 1, header_end);
        length.TrimSpace();

        int len = 0;
        tmp = length.ParseInt(len);
        if (tmp != length.end()) {
            return -1;
        }

        StringPiece body(pos + 4, message.end());
        if (body.size() < len) {
            return 0;
        } else {
            return static_cast<int>(pos - message.begin() + 4 + len);
        }
    }
}

int HttpMessage::Decode(Buffer& message)
{
    StringPiece slice = message.slice();
    int parsed_len = CheckCompleted(slice);
    if (parsed_len <= 0) {
        return parsed_len;
    }

    // 获得first_line
    const char* first_line_end = slice.Find("\r\n");
    if (first_line_end == slice.end()) return -1;
    first_line_ = StringPiece(slice.begin(), first_line_end);
    slice.RemovePrefixUntil(first_line_end + 2);

    // 获得headers
    const char* headers_end = slice.Find("\r\n\r\n");
    if (headers_end == slice.end()) return -1;

    const char* header_crlf = slice.Find("\r\n");
    if (header_crlf == slice.end()) return -1;

    while (header_crlf != headers_end) {
        StringPiece header(slice.begin(), header_crlf);
        const char* seg = header.Find(':');
        if (seg == header.end()) return -1;
        StringPiece key(header.begin(), seg);
        key.TrimSpace();
        StringPiece value(seg + 1, header.end());
        value.TrimSpace();

        headers_[key] = value;

        slice.RemovePrefixUntil(header_crlf + 2);
        header_crlf = slice.Find("\r\n");
    }
    slice.RemovePrefixUntil(headers_end + 4);

    // 获得body
    int before_body_len =
        static_cast<int>(headers_end - first_line_.begin() + 4);
    int body_len = parsed_len - before_body_len;
    body_ = StringPiece(slice.begin(), body_len);

    message.Take(parsed_len);

    return parsed_len;
}

int HttpMessage::Encode(Buffer& message)
{
    message.Append(first_line_);
    message.Append("\r\n");
    for (auto& item : headers_) {
        message.Append(item.first);
        message.Append(": ");
        message.Append(item.second);
        message.Append("\r\n");
    }
    message.Append("\r\n");
    message.Append(body_);

    return message.readable_bytes();
}

HttpMessage::RequestLine HttpMessage::request_line()
{
    RequestLine req;
    auto slice_vector = first_line_.Split(' ');
    if (slice_vector.size() != 3) return req;
    StringPiece method = slice_vector[0];
    if (method == "GET") {
        req.method = Method::kGet;
    } else if (method == "post") {
        req.method = Method::kPost;
    } else {
        req.method = Method::kNone;
        return req;
    }

    StringPiece path = slice_vector[1];
    if (path.Find('?') == path.end()) {
        req.url = path;
    } else {
        const char* pos = path.Find('?');
        if (pos == path.end()) {
            return req;
        }

        req.url = StringPiece(path.begin(), pos);
        StringPiece query = StringPiece(pos + 1, path.end());
        auto query_vector = query.Split('&');

        for (auto& item : query_vector) {
            const char* seg = item.Find('=');
            if (seg == item.end()) {
                return req;
            }
            StringPiece key(item.begin(), seg);
            StringPiece value(seg + 1, item.end());
            req.query[key] = value;
        }
    }

    StringPiece version = slice_vector[2];
    if (version == "HTTP/1.1") {
        req.version = Version::kHttp11;
    } else {
        req.version = Version::kNone;
    }

    return req;
}

void HttpMessage::set_response_line(StringPiece line) { first_line_ = line; }

void HttpMessage::set_headers(StringPiece key, StringPiece value)
{
    headers_[key] = value;
}

void HttpMessage::set_body(StringPiece body) { body_ = body; }

}  // namespace net

}  // namespace baize
