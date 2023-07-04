#include "zio.h"
#include <string.h>
#include <stdio.h>

namespace nf {

void ZIO_init(Thread* th, ZIO* z, Reader reader, void* data)
{
    z->th = th;
    z->reader = reader;
    z->data = data;
    z->n = 0;
    z->p = NULL;
}

int ZIO_peek(ZIO* self)
{
    if (self->n == 0) {
        if (ZIO_fill(self) == EOF)
            return EOF;
        else {
            /* ZIO_fill removed first byte; put back it */
            self->n++;
            self->p--;
        }
    }
    return (int)(*self->p);
}

int ZIO_fill(ZIO* self)
{
    Size size;
    auto* th = self->th;
    const char* buff;
    buff = self->reader(th, self->data, &size);
    if (buff == nullptr || size == 0)
        return EOF;
    self->n = size - 1;
    self->p = buff;
    return (int)(*(self->p++));
}

/* --------------------------------------------------------------- read --- */
Size ZIO_read(ZIO* self, void* b, Size n)
{
    while (n) {
        Size m;
        if (ZIO_peek(self) == EOF)
            return n; /* return number of missing bytes */
        m = (n <= self->n) ? n : self->n; /* min. between n and self->n */
        memcpy(b, self->p, m);
        self->n -= m;
        self->p += m;
        b = (char*)b + m;
        n -= m;
    }
    return 0;
}

} // namespace nf