#include <nf/api.h>
#include <nf/config.h>
#include "lib.h"

namespace nf {
using Thread = imp::Thread;
}

namespace nf::imp {

static int print(Thread* self) { return 0; }

void lib_core_init(Thread* self)
{
    auto old_top = top(self);
    nf::new_str(self, "printf");
    nf::push_func(self, &print);
    nf::set_global(self, -2);
    set_top(self, old_top);
}

} // namespace nf::imp
