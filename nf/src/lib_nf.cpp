#include <nf/api.h>
#include <nf/config.h>
#include "lib.h"

namespace nf::imp {

void lib_nf_init(Thread* self)
{
    auto old_top = top(self);
    nf::new_str(self, "nf");
    nf::new_table(self);

    nf::new_str(self, "version");
    nf::new_str_fmt(
        self, "%d.%d.%d", NF_VERSION_MAJOR, NF_VERSION_MINOR, NF_VERSION_PATCH);
    nf::set_table(self, -3, -2);
    nf::pop(self, 1);

    nf::set_global(self, -2);
    set_top(self, old_top);
}

} // namespace nf::imp