#ifndef BAIZE_HTTP_TASKLET_H_
#define BAIZE_HTTP_TASKLET_H_

#include <vector>

#include "http/http_message.h"
#include "net/net_buffer.h"

namespace baize
{

namespace net
{

class HttpTasklet  // uncopyable
{
public:
    HttpTasklet() : cur_state_(kHandleRequestLine) {}
    ~HttpTasklet() {}
    HttpTasklet(const HttpTasklet&) = delete;
    HttpTasklet& operator=(const HttpTasklet&) = delete;

    // 小于0表示解析错误，其他表示解析出的消息数
    int Parse(Buffer* buf);

    // getter
    std::vector<HttpRequest> moved_requests() { return std::move(requests_); }

private:
    enum TaskletState {
        kContinue,
        kFinish,
        kError,
    };

    enum HttpState {
        kHandleRequestLine,
        kHandleHeaders,
        kHandleBody,
        kHandleComplete,
        kHandleError,
    };

    // getter
    Result<StringPiece, int> SliceMessage(Buffer* buf);

    TaskletState NextStep(StringPiece& message);
    TaskletState HandleRequestLine(StringPiece& message);
    TaskletState HandleHeaders(StringPiece& message);
    TaskletState HandleBody(StringPiece& message);
    TaskletState HandleComplete(StringPiece& message);

    HttpState cur_state_;
    HttpRequest cur_request_;
    std::vector<HttpRequest> requests_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_HTTP_TASKLET_H_