#ifndef BAIZE_QUICCONFIG_H
#define BAIZE_QUICCONFIG_H

#include "quiche.h"
#include "QuicType.h"

namespace baize
{

namespace net
{

class QuicConfig //noncopyable
{
public:
    QuicConfig(uint32_t version);
    ~QuicConfig();

    void setCertAndKey(const char* cert, const char* key);
    void setClientConfig();
    void setServerConfig();
    quiche_config* getConfig() { return config_; }
private:
    quiche_config* config_;
};
    
} // namespace net
    
} // namespace baize


#endif //BAIZE_QUICCONFIG_H