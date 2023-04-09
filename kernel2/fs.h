#ifndef BAIZE_FS_H
#define BAIZE_FS_H

#include "io.h"

namespace baize {

class IFile: public IReader, public IWriter {
public:
    ~IFile() {}
};

result<shared_ptr<IFile>> OpenFile(string name);

inline void Println(shared_ptr<IWriter> w, slice<byte> line) {
    w->Write(line.append('\n'));
}

}  // namespace baize

#endif  // BAIZE_FS_H