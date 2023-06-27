#ifndef NF_ZIO_H
#define NF_ZIO_H

#include "basic_types.h"
#include "utils.h"
#include "api.h"

namespace nf {

struct MBuffer {
    char* data;
    size_t nr;
    size_t alloc;
};

#define MBuffer_init(self)                                                     \
    ((self)->data = nullptr, (self)->alloc = (self)->nr = 0)
#define MBuffer_reset(self) ((self)->nr = 0)
#define MBuffer_resize(th, self, alloc)                                        \
    (self->data = NF_REALLOC_P((self)->data, (alloc)), (self)->alloc = alloc)
#define MBuffer_free(self) NF_FREE(self->data)

struct ZIO {
    /* bytes still unread */
    Size n;
    /* current position in buffer */
    const char* p;
    Reader reader;
    /* additional data */
    void* data;
    Thread* th;
};

void ZIO_init(Thread* L, ZIO* z, Reader reader, void* data);

} // namespace nf

#endif