#include "http/http_parser.h"

namespace baize
{

namespace net
{

static int CheckRequestCompleted(StringPiece request)
{
    // 判断是否有一条完整消息
    const char* pos = request.Find("\r\n\r\n");
    if (pos == request.end()) return 0;

    StringPiece length_header(request.begin(), pos);
    const char* header_begin = length_header.Find("Content-Length");
    if (header_begin == length_header.end()) {
        // 得到一条没有body的消息
        return static_cast<int>(pos - request.begin() + 4);
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

        StringPiece body(pos + 4, request.end());
        if (body.size() < len) {
            return 0;
        } else {
            return static_cast<int>(pos - request.begin() + 4 + len);
        }
    }
}

int HttpRequestParse(StringPiece slice, HttpRequest& req)
{
    int parsed_len = CheckRequestCompleted(slice);
    if (parsed_len <= 0) {
        return parsed_len;
    }
    req.all_data_ = StringPiece(slice.begin(), parsed_len);

    // 获得request_line
    const char* request_line_end = slice.Find("\r\n");
    if (request_line_end == slice.end()) return -1;
    StringPiece request_line(slice.begin(), request_line_end);
    auto slice_vector = request_line.Split(' ');
    if (slice_vector.size() != 3) return -1;
    req.method_ = slice_vector[0];
    req.path_ = slice_vector[1];
    req.version_ = slice_vector[2];
    slice.RemovePrefixUntil(request_line_end + 2);

    // 获得headers
    const char* headers_end = slice.Find("\r\n\r\n");
    const char* header_crlf = slice.Find("\r\n");
    while (header_crlf != headers_end) {
        StringPiece header(slice.begin(), header_crlf);
        const char* seg = header.Find(':');
        if (seg == header.end()) return -1;
        StringPiece key(header.begin(), seg);
        key.TrimSpace();
        StringPiece value(seg + 1, header.end());
        value.TrimSpace();

        req.headers_[key] = value;

        slice.RemovePrefixUntil(header_crlf + 2);
        header_crlf = slice.Find("\r\n");
    }
    slice.RemovePrefixUntil(headers_end + 4);

    // 获得body
    int before_body_len =
        static_cast<int>(headers_end - request_line.begin() + 4);
    int body_len = parsed_len - before_body_len;
    req.body_ = StringPiece(slice.begin(), body_len);

    return parsed_len;
}

}  // namespace net

}  // namespace baize
