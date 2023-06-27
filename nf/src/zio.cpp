#include "zio.h"

namespace nf {

void ZIO_init(Thread* th, ZIO* z, Reader reader, void* data)
{
    z->th = th;
    z->reader = reader;
    z->data = data;
    z->n = 0;
    z->p = NULL;
}

} // namespace nf