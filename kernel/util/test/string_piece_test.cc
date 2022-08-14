#include "util/string_piece.h"

#include "log/logger.h"

using namespace baize;

int main()
{
    StringPiece slice1("&&hello&&nihao&&hi&&&");
    StringPiece slice2(" \r\n hello lse hi \r\n ");

    auto slices = slice1.Split('&');
    for (auto& slice : slices) {
        LOG_INFO << slice;
    }
    slice2.TrimSpace();
    LOG_INFO << slice2;

    StringPiece slice3("/////");
    slices = slice3.Split('/');
    assert(slices.empty());

    StringPiece slice4(" \n \r ");
    slice4.TrimSpace();
    assert(slice4.empty());

    StringPiece slice5("/usr/bin");
    slices = slice5.Split('?');
    for (auto& slice : slices) {
        LOG_INFO << slice;
    }

    StringPiece slice6("/&hello/&hhaha/&nihao/&");
    slices = slice6.Split("/&");
    for (auto& slice : slices) {
        LOG_INFO << slice;
    }

    StringPiece slice7("12566hello");
    auto res = slice7.ParseInt();
    LOG_INFO << "parse int:" << res.ret;
}