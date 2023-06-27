#ifndef NF_PUBLIC_API_H
#define NF_PUBLIC_API_H

namespace nf {

struct Thread;

Thread* Thread_open();
void Thread_close(Thread*);

} // namespace nf

#endif