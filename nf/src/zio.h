#ifndef NF_ZIO_H
#define NF_ZIO_H
#include <string.h>
#include "basic_types.h"
#include "utils.h"
#include "api.h"

namespace nf::imp {

struct MBuffer {
    char* data;
    Size nr;
    Size alloc;
};

#define MBuffer_init(self)                                                     \
    ((self)->data = nullptr, (self)->alloc = (self)->nr = 0)
#define MBuffer_reset(self) ((self)->nr = 0)
#define MBuffer_reserve(th, self, new_alloc)                                   \
    (self)->data                                                               \
        = (char*)NF_REALLOC_P((th), (void*)((self)->data), (new_alloc)),       \
        (self)->alloc = (new_alloc)
#define MBuffer_free(self) NF_FREE((self)->data)
#define MBuffer_append(th, self, ptr, size)                                    \
    {                                                                          \
        if ((self)->nr + (size) > (self)->alloc) {                             \
            MBuffer_reserve((th), (self), ((self)->nr + (size)) * 3 / 2 + 16); \
        }                                                                      \
        memcpy((self)->data + (self)->nr, (ptr), (size));                      \
        (self)->nr += (size);                                                  \
    }

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
int ZIO_peek(ZIO* z);
int ZIO_next(ZIO* z);
int ZIO_fill(ZIO* z);
Size ZIO_read(ZIO* z, void* b, Size n);

} // namespace nf::imp

#endif