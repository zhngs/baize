#include "type.h"

#include <iostream>

#include "gtest/gtest.h"

namespace baize {

void test() {
  slice<byte> s(5, 5);
  s[0] = 1;
  std::cout << s.dump("s") << std::endl;

  slice<byte> s2 = s.as_slice(4, 5);
  std::cout << s2.dump("s2") << std::endl;

  s2.append(1);
  s2[0] = 2;
  std::cout << s.dump("s") << std::endl;
  std::cout << s2.dump("s2") << std::endl;

  slice<byte> s3("hello world");
  std::cout << s3.dump("s3") << std::endl;

  slice<byte> s4(
      "123456789alksnga alkgakerkas..salkkan?laskdnatrnaslahttp lasldkfa");
  s4.copy(s3.as_slice(1, 5));
  std::cout << s4.dump("s4") << std::endl;

  double d = 1.345;
  slice<byte> s5(&d, sizeof(d));
  std::cout << s5.dump("s5") << std::endl;

  slice<string> s6(0, 0);
  s6.append("hello");
  s6.append("world");
  s6.append("zhngs");
  s6.append("www.example.com");
  for (auto it = s6.begin(); it != s6.end(); it++) {
    std::cout << *it << std::endl;
  }

  auto s7 = s6.as_slice(1, 3);
  for (auto it = s7.begin(); it != s7.end(); it++) {
    std::cout << *it << std::endl;
  }
}

TEST(TypeTest, Negative) {
  slice<byte> s(5, 5);
  EXPECT_EQ(5, s.len());
  EXPECT_EQ(5, s.cap());
}

}  // namespace baize