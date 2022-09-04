#include "crypto/util.h"

#include "log/logger.h"
#include "util/string_piece.h"

using namespace baize;

int main()
{
    char key[] = "012345678";
    char data[] = "hello world";

    StringPiece res = crypto::HmacSha1(key, data);
    LOG_INFO << "hmac-sha1: " << log::DumpHex(res);
}