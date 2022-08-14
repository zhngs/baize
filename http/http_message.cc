#include "http_message.h"

namespace baize
{

namespace net
{

void HttpRequest::Reset()
{
    HttpRequest tmp;
    swap(tmp);
}

void HttpRequest::swap(HttpRequest& rhs)
{
    method_.swap(rhs.method_);
    path_.swap(rhs.path_);
    query_.swap(rhs.query_);
    version_.swap(rhs.version_);
    headers_.swap(rhs.headers_);
    body_.swap(rhs.body_);
}

}  // namespace net

}  // namespace baize
