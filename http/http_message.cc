#include "http_message.h"

namespace baize
{

namespace net
{

void HttpResponseBuilder::AppendResponseLine(StringPiece version,
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

void HttpResponseBuilder::AppendHeader(StringPiece key, StringPiece value)
{
    content_.Append(key);
    content_.Append(": ");
    content_.Append(value);
    content_.Append("\r\n");
}

void HttpResponseBuilder::AppendEmptyBody() { content_.Append("\r\n"); }

void HttpResponseBuilder::AppendBody(StringPiece body)
{
    content_.Append("\r\n");
    content_.Append(body);
}

}  // namespace net

}  // namespace baize
