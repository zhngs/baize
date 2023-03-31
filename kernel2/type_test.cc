#include "type.h"
#include <iostream>
using namespace baize;

int main() {
    slice s(5, 5);
    s[0] = 1;
    std::cout << s.dump("s") << std::endl;

    slice s2 = s.as_slice(4, 5);
    std::cout << s2.dump("s2") << std::endl;

    s2.append(1);
    s2[0] = 2;
    std::cout << s.dump("s") << std::endl;
    std::cout << s2.dump("s2") << std::endl;

    slice s3("hello world");
    std::cout << s3.dump("s3") << std::endl;

    slice s4("123456789alksnga alkgakerkas..salkkan?laskdnatrnaslahttp lasldkfa");
    s4.copy(s3.as_slice(1, 5));
    std::cout << s4.dump("s4") << std::endl;

    double d = 1.345;
    slice s5(&d, sizeof(d));
    std::cout << s5.dump("s5") << std::endl;

    long l = 65535;
    slice s6(&l, sizeof(l));
    std::cout << s6.dump("s6") << std::endl;
}