#ifndef BAIZE_FIXEDVECTOR_H
#define BAIZE_FIXEDVECTOR_H

#include "util/types.h"

namespace baize
{

template <int SIZE>
class FixedArray// copyable
{
public:
    FixedArray()
      : data_()
    {}

    // default copy is good

    uint8_t* data() { return data_; }
    char* c_str() { return static_cast<char*>(data_); }
    int length() { return sizeof(data_); }
    void bzero() { memZero(data_, sizeof(data_)); }
private:
    uint8_t data_[SIZE];
};

} // namespace baize


#endif 