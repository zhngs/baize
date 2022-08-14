#ifndef BAIZE_HTTP_MESSAGE_H_
#define BAIZE_HTTP_MESSAGE_H_

#include <map>

#include "util/string_piece.h"
#include "util/types.h"

namespace baize
{

namespace net
{

class HttpRequest  // copyable
{
public:
    void Reset();
    void swap(HttpRequest& rhs);

public:
    StringPiece method_;
    StringPiece path_;
    std::map<StringPiece, StringPiece> query_;
    StringPiece version_;
    std::map<StringPiece, StringPiece> headers_;
    StringPiece body_;
    StringPiece all_data_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_HTTP_MESSAGE_H_