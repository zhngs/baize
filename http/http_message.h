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

class HttpRequest  // copyable
{
public:
public:
    StringPiece method_;
    StringPiece path_;
    std::map<StringPiece, StringPiece> query_;
    StringPiece version_;
    std::map<StringPiece, StringPiece> headers_;
    StringPiece body_;
    StringPiece all_data_;
};

class HttpResponse  // copyable
{
public:
    void AppendResponseLine(StringPiece version,
                            StringPiece num,
                            StringPiece state);
    void AppendHeader(StringPiece key, StringPiece value);
    void AppendEmptyBody();
    void AppendBody(StringPiece body);

    // getter
    StringPiece slice() { return content_.slice(); };

private:
    Buffer content_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_HTTP_MESSAGE_H_