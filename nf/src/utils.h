#ifndef NF_UTILS_H
#define NF_UTILS_H

namespace nf {
void die(const char* fmt, ...);

class Noncopyable {
public:
    Noncopyable() { }
    ~Noncopyable() { }
    Noncopyable(const Noncopyable& other) = delete;
    Noncopyable& operator=(const Noncopyable& other) = delete;
};
}

#endif