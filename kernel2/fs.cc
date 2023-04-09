#include "fs.h"

namespace baize
{

class CFile: public IFile, public noncopyable {
public:
    CFile(): f_(nullptr) {}
    ~CFile() { if(f_ && f_ != stdin && f_ != stdout && f_ != stderr) fclose(f_); }

    result<int> Open(string name) {
        if (name == "stdin") {
            f_ = stdin;
            return {0};
        } else if (name == "stdout") {
            f_ = stdout;
            return {0};
        } else if (name == "stderr") {
            f_ = stderr;
            return {0};
        }

        f_ = fopen(name.c_str(), "rb+");
        if (!f_) {
            return {0, errno, strerror(errno)};
        }
        return {0};
    }

    result<int> Read(slice<byte> p) override {
        size n = fread(p.data<byte>(), 1, p.len(), f_);
        if (n == 0) {
            if (feof(f_)) {
                return {feof(f_), ErrEOF};
            } else if (ferror(f_)) {
                return {ferror(f_), strerror(errno)};
            }
        }
        return {static_cast<int>(n)};
    };

    result<int> Write(slice<byte> p) override {
        size n = fwrite(p.data<byte>(), 1, p.len(), f_);
        if (n < p.len() || n == 0) {
            return {static_cast<int>(n), ferror(f_), strerror(errno)};
        }
        return {static_cast<int>(n)};
    };
private:
    FILE* f_;
};

result<shared_ptr<IFile>> OpenFile(string name) {
    auto file = std::make_shared<CFile>();
    auto r = file->Open(name);
    if (r.err) {
        return {r.err, r.errstring};
    }
    return {file};
}

} // namespace baize
