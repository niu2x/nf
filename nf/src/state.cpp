#include "object.h"
#include "api.h"
#include "utils.h"

namespace nf {

State* State_open()
{
    struct Twin {
        State state;
        GlobalState global;
    };

    auto twin = NF_ALLOC(Twin);
    if (!twin)
        return nullptr;

    auto self = &(twin->state);
    State_global(self) = &(twin->global);

    self->type = Type::Thread;
    TValue_set_nil(&(self->gt));

    TValue_set_nil(State_registry(self));

    State_global(self)->root = self;

    return self;
}

void State_close(State* self) { NF_FREE(State, self); }

} // namespace nf
