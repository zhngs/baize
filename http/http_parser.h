#ifndef BAIZE_HTTP_PARSER_H_
#define BAIZE_HTTP_PARSER_H_

#include "http/http_message.h"

namespace baize
{

namespace net
{

int HttpRequestParse(StringPiece slice, HttpRequest& req);

}  // namespace net

}  // namespace baize

#endif  // BAIZE_HTTP_PARSER_H_