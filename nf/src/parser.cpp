

#include <stdio.h>
#include <ctype.h>

#include "api.h"
#include "zio.h"
#include "bytecode.h"
#include "object.h"

#define printf(...)

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

struct Keyword {
    const char* symbol;
    int token;
};

static Keyword keywords[] = {
    { "print", TT_PRINT },
    { nullptr, 0 },
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
        next_chr(ls);
    }

    while (true) {
        printf("ls->current %d %c\n", ls->current, ls->current);
        switch (ls->current) {
            case EOF: {
                next_chr(ls);
                return Token { .token = TT_EOF };
            }

            case '*':
            case '/':
            case '-':
            case '+': {
                auto c = ls->current;
                next_chr(ls);
                return Token { .token = c };
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
            case '\t':
            case ' ':
            case '\r':
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
                    printf("ls->current %c %d\n", ls->current, ls->current);

                    while (ls->current == '_' || isalnum(ls->current)) {
                        char ch = ls->current;
                        MBuffer_append(ls->th, buff, &ch, 1);
                        next_chr(ls);
                        printf("ls->current %c %d\n", ls->current, ls->current);
                    }

                    auto str = Str_new(ls->th, buff->data, buff->nr);

                    auto keyword = keywords;
                    while (keyword->symbol) {
                        if (!strcmp(keyword->symbol, str->base)) {
                            return Token { .token = keyword->token };
                        }
                        keyword++;
                    }

                    return Token { .token = TT_SYMBOL,
                        .seminfo = { .s = str } };
                } else {
                    Thread_throw(
                        ls->th, E::PARSE, "when next_token unexpected char");
                }
            }
        }
    }
    Thread_throw(ls->th, E::PARSE, "never happened");

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

static void emit(FuncState* fs, Instruction ins)
{
    Proto_append_ins(fs->ls->th, fs->proto, ins);
}

static void emit_const(FuncState* fs, TValue* c)
{
    auto const_index = Proto_insert_const(fs->ls->th, fs->proto, c);
    next(fs->ls);
    emit(fs, INS_FROM_OP_ABCDEF(Opcode::CONST, const_index));
}

static void const_value(FuncState* fs)
{
    auto token = peek(fs->ls);
    switch (token->token) {
        case TT_INTEGER: {
            TValue value = { .type = Type::Integer, .i = token->seminfo.i };
            emit_const(fs, &value);
            break;
        }

        case TT_NUMBER: {
            TValue value = { .type = Type::Number, .n = token->seminfo.n };
            emit_const(fs, &value);
            break;
        }
        default: {
            printf("token->token %d\n", token->token);
            Thread_throw(
                fs->ls->th, E::PARSE, "when const_value, unexpected token");
        }
    }
}

static void mul_or_div_elem(FuncState* fs) { const_value(fs); }

static void add_or_sub_elem(FuncState* fs)
{
    mul_or_div_elem(fs);
    while (true) {
        auto token = peek(fs->ls);
        switch (token->token) {
            case '*': {
                next(fs->ls);
                mul_or_div_elem(fs);
                emit(fs, INS_FROM_OP_NO_ARGS(Opcode::MUL));
                break;
            }
            case '/': {
                next(fs->ls);
                mul_or_div_elem(fs);
                emit(fs, INS_FROM_OP_NO_ARGS(Opcode::DIV));
                break;
            }
            default: {
                goto end_loop;
            }
        }
    }
end_loop:

    (void)0;
}

static void expr(FuncState* fs)
{
    add_or_sub_elem(fs);
    while (true) {
        auto token = peek(fs->ls);
        switch (token->token) {
            case '+': {
                next(fs->ls);
                add_or_sub_elem(fs);
                emit(fs, INS_FROM_OP_NO_ARGS(Opcode::ADD));
                break;
            }
            case '-': {
                next(fs->ls);
                add_or_sub_elem(fs);
                emit(fs, INS_FROM_OP_NO_ARGS(Opcode::SUB));
                break;
            }
            default: {
                goto end_loop;
            }
        }
    }
end_loop:

    (void)0;
}

static void stmt_print(FuncState* fs)
{
    expr(fs);
    emit(fs, INS_FROM_OP_NO_ARGS(Opcode::PRINT));
}

static bool stmt(FuncState* fs)
{
    auto token = peek(fs->ls);
    bool chunk_finished = false;

    switch (token->token) {
        case TT_EOF: {
            chunk_finished = true;
            break;
        }
        case TT_PRINT: {
            next(fs->ls);
            stmt_print(fs);
            break;
        }

        default: {
            Thread_throw(fs->ls->th, E::PARSE, "when stmt, unexpected token");
        }
    }

    return chunk_finished;
}

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
    emit(fs, INS_FROM_OP_NO_ARGS(Opcode::RET_0));
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