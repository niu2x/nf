#ifndef NF_API_H
#define NF_API_H

namespace nf {

struct State;

State* State_open();
void State_close(State*);

using Pfunc = void (*)(State* L, void* ud);

enum class Error : uint8_t {
    OK,
};
using E = Error;

Error State_run_protected(State* self, Pfunc f, void* ud);
void State_throw(State* self, Error err);

} // namespace nf

#endif