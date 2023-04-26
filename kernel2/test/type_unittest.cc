#include "type.h"

#include <iostream>

#include "gtest/gtest.h"

namespace baize {

void print() {
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

  slice<byte> s5 = s3.as_slice(6, s3.len());
  std::cout << s5.dump("s5") << std::endl;
  s5.append('z');
  std::cout << s5.dump("s5") << std::endl;
}

TEST(TypeTest, SliceByteInit) {
  slice<byte> s(5, 5);
  EXPECT_EQ(5, s.len());
  EXPECT_EQ(5, s.cap());

  slice<byte> s2(0, 5);
  EXPECT_EQ(0, s2.len());
  EXPECT_EQ(5, s2.cap());

  slice<byte> s3(0, 0);
  EXPECT_EQ(0, s3.len());
  EXPECT_EQ(0, s3.cap());

  slice<byte> s4;
  EXPECT_EQ(0, s4.len());
  EXPECT_EQ(0, s4.cap());
  EXPECT_EQ(0, s4.data<byte>());

  double d = 1.345;
  slice<byte> s5(&d, sizeof(d), sizeof(d));
  EXPECT_EQ(8, s5.len());
  EXPECT_EQ(8, s5.cap());

  slice<byte> s6("hello world");
  EXPECT_EQ(11, s6.len());

  slice<byte> s7;
  EXPECT_EQ(0, s7.len());
  EXPECT_EQ(0, s7.cap());
}

TEST(TypeTest, SliceByteInit2) {
  slice<byte> s(5, 5);
  s[0] = 's';
  EXPECT_EQ(s[0], 's');

  auto s2 = s;
  EXPECT_EQ(s.data<byte>(), s2.data<byte>());
  EXPECT_EQ(s2[0], 's');

  byte* s_pointer = s.data<byte>();
  s2.append('z');
  s2[0] = 'd';
  EXPECT_NE(s.data<byte>(), s2.data<byte>());
  EXPECT_EQ(s.data<byte>(), s_pointer);
  EXPECT_EQ(s2[0], 'd');
  EXPECT_EQ(s[0], 's');

  slice<byte> s3;
  s3 = s;
  EXPECT_EQ(s.data<byte>(), s3.data<byte>());
  EXPECT_EQ(s3[0], 's');

  s3.append('z');
  s3[0] = 'f';
  EXPECT_NE(s.data<byte>(), s3.data<byte>());
  EXPECT_EQ(s.data<byte>(), s_pointer);
  EXPECT_EQ(s3[0], 'f');
  EXPECT_EQ(s[0], 's');

  slice<byte> s4 = std::move(s2);
  EXPECT_EQ(s4[0], 'd');
  EXPECT_EQ(0, s2.len());
  EXPECT_EQ(0, s2.cap());
  EXPECT_EQ(6, s4.len());
  EXPECT_EQ(12, s4.cap());

  slice<byte> s5;
  s5 = std::move(s3);
  EXPECT_EQ(s5[0], 'f');
  EXPECT_EQ(0, s3.len());
  EXPECT_EQ(0, s3.cap());
  EXPECT_EQ(6, s5.len());
  EXPECT_EQ(12, s5.cap());
}

TEST(TypeTest, AsSlice) {
  slice<byte> s;
  s.append('h');
  s.append('e');
  s.append('l');
  s.append('l');
  s.append('o');

  auto s2 = s.as_slice(1, 3);
  EXPECT_EQ(2, s2.len());
  EXPECT_EQ('e', s2[0]);

  s2[0] = 'b';
  EXPECT_EQ('b', s[1]);
}

TEST(TypeTest, String) {
  slice<string> s;
  s.append("hello");
  s.append("world");
  s.append("zhngs");
  s.append("www.example.com");
  EXPECT_EQ(4, s.len());
  EXPECT_EQ(std::string("world"), s[1]);

  auto s2 = s.as_slice(1, 3);
  EXPECT_EQ(2, s2.len());
  EXPECT_EQ(std::string("zhngs"), s2[1]);
}

TEST(TypeTest, Compare) {
  slice<byte> s("1234");
  slice<byte> s2("12345");
  slice<byte> s3("1294578");
  slice<byte> s4("1234");
  
  EXPECT_EQ(true, s < s2);
  EXPECT_EQ(true, s3 > s);
  EXPECT_EQ(true, s == s4);
}

TEST(TypeTest, Print) {
  print();
}

}  // namespace baize