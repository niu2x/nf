#ifndef NF_PUBLIC_API_H
#define NF_PUBLIC_API_H

#include <cstdint>
#include <cstdio>

namespace nf {

using StackIndex = int16_t;
enum PseudoIndex {
    PSEUDO_INDEX_GLOBAL = -30005,
};

namespace imp {
struct Thread;
};

using Thread = imp::Thread;
using ThreadPtr = Thread*;
using CFunc = int (*)(ThreadPtr self);

ThreadPtr open();
void close(ThreadPtr);
void run(ThreadPtr, const char* code);
void run(ThreadPtr self, FILE* fp);
void set_debug(ThreadPtr, bool debug);

void new_table(ThreadPtr self);
void new_str(ThreadPtr self, const char* sz, int len = 0);
void new_str_fmt(ThreadPtr self, const char* sz, ...);

void set_global(ThreadPtr self, StackIndex key);
void set_table(ThreadPtr self, StackIndex table, StackIndex key);
void pop(ThreadPtr self, StackIndex n);
void set_top(ThreadPtr self, StackIndex top);
StackIndex top(ThreadPtr self);
void push_func(ThreadPtr self, CFunc func);

} // namespace nf

#endif