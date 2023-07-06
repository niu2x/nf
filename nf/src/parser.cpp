#include <stdio.h>

#include "api.h"
#include "zio.h"
#include "object.h"

namespace nf {

struct SParser {
    ZIO* z;
    MBuffer buff;
    const char* name;
};

union SemInfo {
    Integer i;
    Number n;
    Str* s;
}; /* semantics information */

enum TokenType {
    TT_NONE = 256,
    TT_EOF,
    TT_INTEGER,
    TT_NUMBER,
};

struct Token {
    int token;
    SemInfo seminfo;
};

static Token NONE = { .token = TT_NONE };

struct LexState {
    int current; /* current character (charint) */
    Token t; /* current token */
    Token peek; /* look ahead token */

    Thread* th;
    ZIO* z;
    MBuffer* buff;
};

struct FuncState {
    LexState* ls;
    Proto* proto;
};

#define next_chr(ls) (ls)->current = ZIO_next((ls)->z)

static Token next_token(LexState* ls)
{
    if (ls->current == EOF) {
        ls->current = ZIO_next(ls->z);
    }

    switch (ls->current) {
        case EOF: {
            next_chr(ls);
            return Token { .token = TT_EOF };
        }

        case '-': {
            next_chr(ls);
            return Token { .token = '-' };
        }

        case '+': {
            next_chr(ls);
            return Token { .token = '+' };
        }

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            Integer i = ls->current - '0';
            TokenType tt = TT_INTEGER;
        }
    }

    Thread_throw(ls->th, E::PARSE);

    return NONE;
}

static NF_INLINE Token* peek(LexState* ls)
{
    if (ls->peek.token == TT_NONE) {
        ls->peek = next_token(ls);
    }
    return &(ls->peek);
}

static NF_INLINE Token* next(LexState* ls)
{
    if (ls->peek.token == TT_NONE) {
        ls->t = next_token(ls);
    } else {
        ls->t = ls->peek;
        ls->peek = NONE;
    }
    return &(ls->t);
}

static bool stmt(FuncState* ls) { return false; }

static bool stmt_with_semi(FuncState* fs)
{
    bool chunk_finished = stmt(fs);
    if (fs->ls->t.token == ';')
        next(fs->ls);

    return chunk_finished;
}

static void chunk(FuncState* fs)
{
    bool chunk_finished = stmt_with_semi(fs);
    while (!chunk_finished) {
        chunk_finished = stmt_with_semi(fs);
    }
}

static Proto* y_parser(Thread* th, ZIO* z, MBuffer* buff, const char* name)
{
    LexState ls;
    ls.th = th;
    ls.z = z;
    ls.buff = buff;
    ls.t = { .token = TT_NONE };
    ls.peek = { .token = TT_NONE };
    ls.current = EOF;

    FuncState fs;
    fs.ls = &ls;
    fs.proto = nullptr;

    auto proto = Proto_new(th);
    fs.proto = proto;

    chunk(&fs);

    return fs.proto;
}

static void f_parser(Thread* th, void* ud)
{

    struct Proto* proto;
    struct Func* func;
    struct SParser* p = (struct SParser*)(ud);

    int c = ZIO_peek(p->z);
    proto = y_parser(th, p->z, &(p->buff), p->name);
    func = Func_new(th, proto);
    Thread_push_func(th, func);
}

Error protected_parser(Thread* th, ZIO* z, const char* name)
{
    struct SParser p;
    Error err;
    p.z = z;
    p.name = name;
    MBuffer_init(&p.buff);
    err = Thread_pcall(th, f_parser, &p);
    MBuffer_free(&p.buff);
    return err;
}

} // namespace nf