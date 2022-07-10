#ifndef BAIZE_QUICTYPE_H
#define BAIZE_QUICTYPE_H

#include "FixedArray.h"
#include <memory>
#include "quiche.h"

namespace baize
{

namespace net
{

class QuicConnection;
typedef std::shared_ptr<QuicConnection> QuicConnSptr;

class QuicConfig;
typedef std::unique_ptr<QuicConfig> QuicConfigUptr;

const int kConnIdLen = 16;
typedef FixedArray<kConnIdLen> QuicConnId;

const int kMaxDatagramSize = 1350;
} // namespace net
    
} // namespace baize

#endif //BAIZE_QUICTYPE_H