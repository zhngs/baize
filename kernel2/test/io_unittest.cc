#include "io.h"
#include "fs.h"

#include <iostream>

#include "gtest/gtest.h"

namespace baize {

TEST(IOTest, ZeroAndNull) {
    shared_ptr<IReader> reader = std::make_shared<DevZero>();
    slice<byte> s(512, 512);
    auto res = reader->Read(s);
    EXPECT_EQ(res.v, 512);
    EXPECT_EQ(res.err, 0);
    EXPECT_EQ(res.errstring, "");
}

TEST(IOTest, FsTest) {
    RESULT(file, err_file, estr_file, OpenFile("stdout"));
    RESULT(file2, err_file2, estr_file2, OpenFile("Makefile"));
    auto s = ReadAll(file2);
    file->Write(s.v);
}

}  // namespace baize