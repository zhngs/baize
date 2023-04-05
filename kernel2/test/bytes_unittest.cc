#include "bytes.h"

#include <iostream>

#include "gtest/gtest.h"

namespace baize {

TEST(BytesTest, IndexTest) {
    slice<byte> s("alshellolasdk");
    int pos = Index(s, slice<byte>("hello"));
    EXPECT_EQ(3, pos);

    pos = Index(s, slice<byte>("xx"));
    EXPECT_EQ(-1, pos);

    pos = Index(s, slice<byte>());
    EXPECT_EQ(0, pos);
}

TEST(BytesTest, SplitTest) {
    slice<byte> s("hello;world;test;nihao");
    auto r = Split(s, slice<byte>(";"));
    EXPECT_EQ(4, r.len());
    EXPECT_EQ(std::string("hello"), r[0].as_string());
    EXPECT_EQ(std::string("world"), r[1].as_string());
    EXPECT_EQ(std::string("test"), r[2].as_string());
    EXPECT_EQ(std::string("nihao"), r[3].as_string());

    std::cout << s.dump("s") << std::endl;
    for (auto i = r.begin(); i != r.end(); i++) {
        std::cout << i->dump("item") << std::endl;
    }
}

}  // namespace baize