#include "http_message.h"

namespace baize
{

namespace net
{

void HttpResponse::AppendResponseLine(StringPiece version,
                                      StringPiece num,
                                      StringPiece state)
{
    content_.Append(version);
    content_.Append(" ");
    content_.Append(num);
    content_.Append(" ");
    content_.Append(state);
    content_.Append("\r\n");
}

void HttpResponse::AppendHeader(StringPiece key, StringPiece value)
{
    content_.Append(key);
    content_.Append(": ");
    content_.Append(value);
    content_.Append("\r\n");
}

void HttpResponse::AppendEmptyBody() { content_.Append("\r\n"); }

void HttpResponse::AppendBody(StringPiece body)
{
    content_.Append("\r\n");
    content_.Append(body);
}

}  // namespace net

}  // namespace baize
