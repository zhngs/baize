#ifndef BAIZE_RANDOMFILE_H
#define BAIZE_RANDOMFILE_H

#include "log/Logger.h"

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

namespace baize
{

class RandomFile
{
public:
    static RandomFile& getInstance()
    {
        static RandomFile instance;
        return instance;
    }

    bool genRandom(void* buf, int len)
    {
        ssize_t ret = read(fd_, buf, len);
        if (ret != len) {
            LOG_SYSERR << "random file read failed";
            return false;
        }
        return true;
    }
private:
    RandomFile()
    {
        fd_ = open("/dev/urandom", O_RDONLY);
        assert(fd_ != -1);
    }
    ~RandomFile() = default;
    RandomFile(const RandomFile&) = default;
    RandomFile& operator=(const RandomFile&) = default;

    int fd_;
};
    
} // namespace baize


#endif //BAIZE_RANDOMFILE_H