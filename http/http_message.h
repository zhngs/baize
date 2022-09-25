#ifndef BAIZE_HTTP_MESSAGE_H_
#define BAIZE_HTTP_MESSAGE_H_

#include <map>

#include "net/net_buffer.h"
#include "util/string_piece.h"
#include "util/types.h"

namespace baize
{

namespace net
{

class HttpMessage  // copyable
{
public:  // types
    enum class Type { kNone, kRequest, kResponse };
    enum class Method { kNone, kGet, kPost };
    enum class Version { kNone, kHttp11 };
    enum class StatusCode { kNone, k200 };
    enum class StatusDescription { kNone, kOK };

    struct RequestLine {
        Method method = Method::kNone;
        StringPiece url;
        std::map<StringPiece, StringPiece> query;
        Version version = Version::kNone;
    };

    struct ResponseLine {
        Version version = Version::kNone;
        StatusCode code = StatusCode::kNone;
        StatusDescription description = StatusDescription::kNone;
    };

public:  // static function
    static string MakeRequestLine(RequestLine& req);
    static string MakeResponseLine(ResponseLine& rsp);

public:  // normal function
    int Decode(Buffer& message);
    int Encode(Buffer& message);

    // getter
    RequestLine request_line();
    ResponseLine response_line();

    // setter
    void set_request_line(StringPiece line);
    void set_response_line(StringPiece line);
    void set_headers(StringPiece key, StringPiece value);
    void set_body(StringPiece body);

public:
    StringPiece first_line_;
    std::map<StringPiece, StringPiece> headers_;
    StringPiece body_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_HTTP_MESSAGE_H_