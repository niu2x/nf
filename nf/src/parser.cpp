#include <stdio.h>
#include <ctype.h>

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
    TT_SYMBOL,
    TT_PRINT,
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
    auto buff = ls->buff;
    if (ls->current == EOF) {
        ls->current = ZIO_next(ls->z);
    }

    while (true)
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
                Number n;
                Number scale;
                TokenType tt = TT_INTEGER;
                next_chr(ls);

                while (true) {
                    if (tt == TT_INTEGER) {
                        if (ls->current >= '0' && ls->current <= '9') {
                            i = i * 10 + ls->current - '0';
                            next_chr(ls);
                        } else if (ls->current == '.') {
                            tt = TT_NUMBER;
                            n = i;
                            scale = 0.1;
                            next_chr(ls);
                        } else {
                            break;
                        }
                    } else {
                        if (ls->current >= '0' && ls->current <= '9') {
                            n = n + scale * (ls->current - '0');
                            scale *= 0.1;
                            next_chr(ls);
                        } else
                            break;
                    }
                }

                if (tt == TT_INTEGER)
                    return Token { .token = tt, .seminfo = { .i = i } };
                else
                    return Token { .token = tt, .seminfo = { .n = n } };
            }
            case '\r':
            case '\t':
            case '\n': {
                next_chr(ls);
                break;
            }

            default: {
                if (ls->current == '_' || isalpha(ls->current)) {
                    char ch = ls->current;
                    MBuffer_reset(buff);
                    MBuffer_append(ls->th, buff, &ch, 1);
                    next_chr(ls);

                    while (ls->current == '_' || isalnum(ls->current)) {
                        MBuffer_append(ls->th, buff, &ch, 1);
                        next_chr(ls);
                    }

                    auto str = Str_new(ls->th, buff->data, buff->nr);
                    return Token { .token = TT_SYMBOL,
                        .seminfo = { .s = str } };
                }
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

static bool stmt(FuncState* ls) { return true; }

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