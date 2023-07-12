#ifndef NF_PUBLIC_API_H
#define NF_PUBLIC_API_H

#include <stdio.h>

namespace nf {

struct Thread;

Thread* Thread_open();
void Thread_close(Thread*);
void Thread_run(Thread*, const char* code);
void Thread_run(Thread* self, FILE* fp);

} // namespace nf

#endif