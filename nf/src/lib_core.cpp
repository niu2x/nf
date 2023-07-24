#include <nf/api.h>
#include <nf/config.h>
#include "lib.h"
#include "api.h"

namespace nf {
using Thread = imp::Thread;
}

namespace nf::imp {

static int print(Thread* self) { return print(self, self->base); }

void lib_core_init(Thread* self)
{
    auto old_top = top(self);
    new_str(self, "print");
    push_func(self, &print);
    set_global(self, -2);
    set_top(self, old_top);
}

} // namespace nf::imp
