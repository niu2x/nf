#ifndef NF_API_H
#define NF_API_H

namespace nf {

struct State;

State* State_open();
void State_close(State*);

// LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);

// void throw (lua_State *L, int errcode);
// int rawrunprotected (lua_State *L, Pfunc f, void *ud);

} // namespace nf

#endif