#include <stdio.h>
#include <ctype.h>

#include "api.h"
#include "zio.h"
#include "bytecode.h"
#include "object.h"

// #define printf(...)

namespace nf::imp {

struct SParser {
    ZIO* z;
    MBuffer buff;
    const char* name;
};

union SemInfo {
    Integer i;
    Number n;
    Str* s;
};

static SemInfo null_seminfo = { .s = nullptr };

enum TokenType {
    TT_NONE = 256,
    TT_EOF,
    TT_INTEGER,
    TT_NUMBER,
    TT_SYMBOL,
    TT_STRING,
    TT_LOCAL,
    TT_FUNCTION,
    TT_RETURN,
    TT_IF,
    TT_ELSE,
    TT_NIL,
    TT_LE,
    TT_EQ,
    TT_GE,
    TT_NE,
    TT_WHILE,
};

struct Keyword {
    const char* symbol;
    int token;
};

static Keyword keywords[] = {
    { "local", TT_LOCAL },   { "function", TT_FUNCTION },
    { "return", TT_RETURN }, { "while", TT_WHILE },
    { "if", TT_IF },         { "else", TT_ELSE },
    { "nil", TT_NIL },       { nullptr, 0 },
};

struct Token {
    int token;
    SemInfo seminfo;
};

static Token NONE = { .token = TT_NONE, .seminfo = { .s = nullptr } };

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
    Scope* scope;
};

enum class SingleValueType : uint8_t {
    NONE,
    NORMAL,
    TABLE_SLOT,
    UP_VALUE,
};

struct SingleValue {
    StackIndex uv_index;
    StackIndex index;
    StackIndex extras[1];

    SingleValueType type;
    bool assignable;
};

struct OperationRule {
    char type;
    int operations[8];
};

static SingleValue expr(FuncState* fs, const OperationRule* rule);

#define MAX_USED_SLOT(fs) ((StackIndex)((fs)->proto->used_slots - 1))

#define SINGLE_NORMAL_VALUE_AT_TOP(fs, assignable)                             \
    single_normal_value(MAX_USED_SLOT(fs), (assignable))

#define SINGLE_TABLE_SLOT_KEY_AT_TOP(fs, table, assignable)                    \
    single_table_slot((table), MAX_USED_SLOT(fs), (assignable))

static void stmt_local(FuncState* fs);
static void chunk(FuncState* fs);
static void func_body(FuncState* fs);

static SingleValue single_normal_value(StackIndex index, bool assignable)
{
    return { .uv_index = 0,
             .index = index,
             .extras = {},
             .type = SingleValueType::NORMAL,
             .assignable = assignable };
}

static SingleValue single_up_value(StackIndex uv_index, bool assignable)
{
    return { .uv_index = uv_index,
             .index = 0,
             .extras = {},
             .type = SingleValueType::UP_VALUE,
             .assignable = assignable };
}

static SingleValue single_table_slot(StackIndex table_index,
                                     StackIndex key_index,
                                     bool assignable)
{
    return { .uv_index = 0,
             .index = table_index,
             .extras = { key_index },
             .type = SingleValueType::TABLE_SLOT,
             .assignable = assignable };
}

static SingleValue single_value_none = {
    .uv_index = 0,
    .index = 0,
    .extras = {},
    .type = SingleValueType::NONE,
    .assignable = false,
};

#define next_chr(ls) (ls)->current = ZIO_next((ls)->z)

static NF_INLINE Token token_build(int token)
{
    return Token { .token = token, .seminfo = null_seminfo };
}

static Token next_token(LexState* ls)
{
    auto buff = ls->buff;
    if (ls->current == EOF) {
        next_chr(ls);
    }

    while (true) {
        switch (ls->current) {
            case EOF: {
                next_chr(ls);
                return token_build(TT_EOF);
            }

            case '!': {
                next_chr(ls);
                if (ls->current == '=') {
                    next_chr(ls);
                    return token_build(TT_NE);
                } else {
                    return token_build('!');
                }
            }

            case '<':
            case '=':
            case '>': {
                char pre = ls->current;
                next_chr(ls);
                if (ls->current == '=') {
                    next_chr(ls);
                    return token_build(pre - '<' + TT_LE);
                } else {
                    return token_build(pre);
                }
            }

            case '/': {
                next_chr(ls);
                if (ls->current == '/') {
                    next_chr(ls);
                    while (ls->current != EOF && ls->current != '\n') {
                        next_chr(ls);
                    }

                    if (ls->current == '\n')
                        next_chr(ls);
                } else {
                    return token_build('/');
                }

                break;
            }

            case ',':
            case '#':
            case ';':
            case '[':
            case ']':
            case '{':
            case '}':
            case '(':
            case ')':
            case '*':
            case '-':
            case '+': {
                auto c = ls->current;
                next_chr(ls);
                return token_build(c);
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

            case '\'':
            case '"': {
                auto quote = ls->current;
                next_chr(ls);
                MBuffer_reset(buff);
                while (ls->current != quote) {
                    char ch = ls->current;
                    MBuffer_append(ls->th, buff, &ch, 1);
                    next_chr(ls);
                }
                next_chr(ls);
                auto str = Str_new(ls->th, buff->data, buff->nr);

                return Token { .token = TT_STRING, .seminfo = { .s = str } };
            }

            default: {
                if (ls->current == '_' || isalpha(ls->current)) {
                    char ch = ls->current;
                    MBuffer_reset(buff);
                    MBuffer_append(ls->th, buff, &ch, 1);
                    next_chr(ls);

                    while (ls->current == '_' || isalnum(ls->current)) {
                        char ch = ls->current;
                        MBuffer_append(ls->th, buff, &ch, 1);
                        next_chr(ls);
                    }

                    auto str = Str_new(ls->th, buff->data, buff->nr);

                    auto keyword = keywords;
                    while (keyword->symbol) {
                        if (!strcmp(keyword->symbol, str->base)) {
                            return token_build(keyword->token);
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

static NF_INLINE void next(LexState* ls)
{
    if (ls->peek.token == TT_NONE) {
        ls->t = next_token(ls);
    } else {
        ls->t = ls->peek;
        ls->peek = NONE;
    }
}

static NF_INLINE void expect(LexState* ls, int token)
{
    if (peek(ls)->token == token) {
        next(ls);
    } else {
        printf("expect %c, it is %d\n", token, peek(ls)->token);
        Thread_throw(ls->th, E::PARSE, "expect error");
    }
}

static NF_INLINE bool maybe_expect(LexState* ls, int token)
{
    if (peek(ls)->token == token) {
        next(ls);
        return true;
    } else {
        return false;
    }
}

static const OperationRule operations_order[] = {
    { '2', { '=', 0 } },
    { '2', { '<', '>', TT_LE, TT_EQ, TT_NE, TT_GE, 0 } },
    { '2', { '+', '-', 0 } },
    { '2', { '/', '*', '%', 0 } },
    { '1', { '-', '#', 0 } },
    { 'X', { '(', '[', 0 } },
    { 0, { 0 } },
};

static InsIndex emit(FuncState* fs, Instruction ins, int slots_changed)
{
    auto ii = Proto_append_ins(fs->ls->th, fs->proto, ins);
    fs->proto->used_slots += slots_changed;
    return ii;
}

static InsIndex emit_pop_to(FuncState* fs, StackIndex new_top)
{
    auto ins = INS_FROM_OP_AB(Opcode::POP_TO, new_top);
    auto ii = emit(fs, ins, 0);
    fs->proto->used_slots = new_top;
    return ii;
}

static void emit_const(FuncState* fs, TValue* c)
{
    auto const_index = Proto_insert_const(fs->ls->th, fs->proto, c);
    emit(fs, INS_BUILD(CONST, const_index, fs->proto->used_slots), 1);
}

static SingleValue const_value(FuncState* fs)
{
    auto token = peek(fs->ls);
    switch (token->token) {
        case TT_INTEGER: {
            TValue value = { .type = Type::Integer, .i = token->seminfo.i };
            emit_const(fs, &value);
            next(fs->ls);
            break;
        }
        case TT_NUMBER: {
            TValue value = { .type = Type::Number, .n = token->seminfo.n };
            emit_const(fs, &value);
            next(fs->ls);
            break;
        }
        case TT_STRING: {
            TValue value = { .type = Type::String, .obj = token->seminfo.s };
            emit_const(fs, &value);
            next(fs->ls);
            break;
        }
    }
    return SINGLE_NORMAL_VALUE_AT_TOP(fs, false);
}

static SingleValue table_value(FuncState* fs)
{
    next(fs->ls);
    expect(fs->ls, ']');
    emit(fs, INS_BUILD(NEW_TABLE, fs->proto->used_slots), 1);
    return SINGLE_NORMAL_VALUE_AT_TOP(fs, false);
}

static SingleValue ensure_at_top(FuncState* fs, SingleValue value)
{
    NF_ASSERT(fs->ls->th, value.type == SingleValueType::NORMAL,
        "must be normal value");

    if (value.index != MAX_USED_SLOT(fs)) {
        emit(fs, INS_BUILD(PUSH, value.index, fs->proto->used_slots), 1);
        value.index = MAX_USED_SLOT(fs);
    }
    return value;
}

static SingleValue ensure_normal_value(FuncState* fs, SingleValue value)
{
    if (value.type == SingleValueType::NORMAL) {
        return value;
    } else if (value.type == SingleValueType::TABLE_SLOT) {
        emit(
            fs,
            INS_BUILD(
                TABLE_GET, value.index, value.extras[0], fs->proto->used_slots),
            1);
        return SINGLE_NORMAL_VALUE_AT_TOP(fs, false);
    } else if (value.type == SingleValueType::UP_VALUE) {
        emit(fs, INS_FROM_OP_AB(Opcode::GET_UP_VALUE, value.uv_index), 1);
        return SINGLE_NORMAL_VALUE_AT_TOP(fs, false);
    } else {
        // never reach
        return value;
    }
}

static SingleValue lookup_var(FuncState* fs, Token* token)
{
    StackIndex slot;
    SingleValue value;
    auto var_name = token->seminfo.s->base;
    if ((slot = Scope_search(fs->scope, var_name, nullptr, true)) < 0) {
        UpValuePos uv_pos;
        if ((uv_pos = UpValue_search(fs->ls->th, fs->proto->parent, var_name))
                .u32
            != 0xFFFFFFFF) {
            auto uv_index = Proto_insert_uv(fs->ls->th, fs->proto, uv_pos);
            value = single_up_value(uv_index, true);
        } else {
            TValue key = { .type = Type::String, .obj = token->seminfo.s };
            emit_const(fs, &key);
            value = SINGLE_TABLE_SLOT_KEY_AT_TOP(fs, PSEUDO_INDEX_GLOBAL, true);
        }
    } else {
        value = single_normal_value(slot, true);
    }
    next(fs->ls);
    return value;
}

static void clear_scope_slots(FuncState* fs)
{
    emit(fs, INS_FROM_OP_AB(Opcode::CLOSE_UV_TO, fs->proto->used_slots), 0);
    emit_pop_to(fs, fs->proto->used_slots);
}
static void block(FuncState* fs)
{
    chunk(fs);
    clear_scope_slots(fs);
}

static SingleValue function(FuncState* parent_fs)
{
    FuncState fs;
    Scope scope;
    fs.ls = parent_fs->ls;
    fs.proto = nullptr;

    Scope_init(&scope, fs.ls->th);
    fs.scope = &scope;

    auto proto = Proto_new(fs.ls->th);
    fs.proto = proto;
    proto->name = Str_new(fs.ls->th, "", 0);
    proto->parent = parent_fs->proto;

    scope.parent_used_slots = 0;

    expect(fs.ls, TT_FUNCTION);
    expect(fs.ls, '(');

    StackIndex args_nr = 0;
    auto token = peek(fs.ls);
    if (token->token == TT_SYMBOL) {
        do {
            const char* var_name = token->seminfo.s->base;

            NF_CHECK(fs.ls->th,
                     Scope_search(fs.scope, var_name, nullptr) < 0,
                     "already defined args");
            Scope_insert(fs.scope, var_name, MAX_USED_SLOT(&fs) + 1);
            fs.proto->used_slots++;
            args_nr++;
            next(fs.ls);
        } while (maybe_expect(fs.ls, ',') && (token = peek(fs.ls)));
    }

    fs.proto->args_nr = args_nr;

    expect(fs.ls, ')');
    expect(fs.ls, '{');

    func_body(&fs);

    expect(fs.ls, '}');

    TValue tv_proto = { .type = Type::Proto, .obj = fs.proto };
    emit_const(parent_fs, &tv_proto);
    emit(
        parent_fs, INS_BUILD(NEW_NF_FUNC, parent_fs->proto->used_slots - 1), 0);
    return SINGLE_NORMAL_VALUE_AT_TOP(parent_fs, false);
}

static SingleValue base_elem(FuncState* fs)
{

    auto token = peek(fs->ls);
    if (token->token == TT_SYMBOL) {
        return lookup_var(fs, token);
    } else if (token->token == '[') {
        return table_value(fs);
    } else if (token->token == TT_INTEGER || token->token == TT_NUMBER
               || token->token == TT_STRING) {
        return const_value(fs);
    } else if (token->token == '(') {
        next(fs->ls);
        auto v = expr(fs, operations_order);
        expect(fs->ls, ')');
        return v;
    } else if (token->token == TT_FUNCTION) {
        return function(fs);
    } else if (token->token == TT_NIL) {
        next(fs->ls);
        emit(fs, INS_BUILD(LOAD_NIL, fs->proto->used_slots), 1);
        return SINGLE_NORMAL_VALUE_AT_TOP(fs, false);
    }

    else {
        Thread_throw(fs->ls->th, E::PARSE, "when parse expr");
    }

    // NEVER REACH
    return single_value_none;
}

static void assignemnt(FuncState* fs,
                       const OperationRule* rule,
                       SingleValue* left_value)
{

    NF_CHECK(fs->ls->th, left_value->assignable, "not assignable");

    auto second = expr(fs, rule + 1);
    second = ensure_normal_value(fs, second);

    if (left_value->type == SingleValueType::NORMAL) {
        auto ins = INS_BUILD(SET, left_value->index, second.index);
        emit(fs, ins, 0);

    } else if (left_value->type == SingleValueType::TABLE_SLOT) {
        second = ensure_at_top(fs, second);
        auto ins = INS_BUILD(
            TABLE_SET, left_value->index, left_value->extras[0]);
        emit(fs, ins, -1);
    } else if (left_value->type == SingleValueType::UP_VALUE) {
        auto ins = INS_FROM_OP_AB_CD(
            Opcode::SET_UP_VALUE, left_value->uv_index, second.index);
        emit(fs, ins, 0);
    }
}

static SingleValue bin_op(FuncState* fs, const OperationRule* operations)
{
    auto first = expr(fs, operations + 1);
    auto target_op = peek(fs->ls)->token;
    while (true) {
        auto op_ptr = operations->operations;
        while (*op_ptr) {
            if (*op_ptr == target_op) {

                next(fs->ls);

                switch (target_op) {

#define SIMPLE_BIN_OP(opcode)                                                  \
    first = ensure_normal_value(fs, first);                                    \
    auto second = expr(fs, operations + 1);                                    \
    second = ensure_normal_value(fs, second);                                  \
    emit(fs,                                                                   \
         INS_BUILD(opcode, first.index, second.index, fs->proto->used_slots),  \
         1);                                                                   \
    first = SINGLE_NORMAL_VALUE_AT_TOP(fs, false);

                    case '+': {
                        SIMPLE_BIN_OP(ADD);
                        break;
                    }
                    case '-': {
                        SIMPLE_BIN_OP(SUB);
                        break;
                    }
                    case '/': {
                        SIMPLE_BIN_OP(DIV);

                        break;
                    }
                    case '*': {
                        SIMPLE_BIN_OP(MUL);
                        break;
                    }

                    case '<': {
                        SIMPLE_BIN_OP(LESS);
                        break;
                    }
                    case '>': {
                        SIMPLE_BIN_OP(GREATE);
                        break;
                    }

                    case TT_LE: {
                        SIMPLE_BIN_OP(LE);
                        break;
                    }
                    case TT_GE: {
                        SIMPLE_BIN_OP(GE);
                        break;
                    }

                    case TT_EQ: {
                        SIMPLE_BIN_OP(EQ);
                        break;
                    }
                    case TT_NE: {
                        SIMPLE_BIN_OP(NE);
                        break;
                    }
#undef SIMPLE_BIN_OP
                        // '%':
                    case '=': {
                        assignemnt(fs, operations, &first);
                        break;
                    }

                    default: {
                        Thread_throw(fs->ls->th, E::PARSE, "unsupport bin_op");
                        break;
                    }
                }
                break;
            }
            op_ptr++;
        }

        if (*op_ptr) {
            target_op = peek(fs->ls)->token;
        } else {
            break;
        }
    }

    return first;
}

static SingleValue uni_op(FuncState* fs, const OperationRule* rule)
{

    auto target_op = peek(fs->ls)->token;

    auto op_ptr = rule->operations;
    ;
    while (*op_ptr) {
        if (*op_ptr == target_op) {
            next(fs->ls);
            auto first = expr(fs, rule);
            first = ensure_normal_value(fs, first);

            switch (target_op) {
                case '-': {
                    emit(fs,
                         INS_BUILD(NEG, first.index, fs->proto->used_slots),
                         1);
                    first = SINGLE_NORMAL_VALUE_AT_TOP(fs, false);

                    break;
                }
                case '#': {
                    emit(fs,
                         INS_BUILD(LEN, first.index, fs->proto->used_slots),
                         1);
                    first = SINGLE_NORMAL_VALUE_AT_TOP(fs, false);
                    break;
                }
            }

            return first;
        }
        op_ptr++;
    }

    return expr(fs, rule + 1);
}

static SingleValue call_or_table_access(FuncState* fs,
                                        const OperationRule* rule)
{

    auto first = expr(fs, rule + 1);
    auto token = peek(fs->ls)->token;

    while (true) {

        if (token == '(') {
            first = ensure_normal_value(fs, first);

            next(fs->ls);

            StackIndex args[MAX_ARGS];
            int args_nr = 0;

            auto token = peek(fs->ls);
            if (token->token != ')') {
                while (true) {
                    auto v = expr(fs, operations_order);
                    v = ensure_normal_value(fs, v);
                    NF_ASSERT(fs->ls->th,
                        v.type == SingleValueType::NORMAL,
                        "operand must normal value");
                    NF_CHECK(
                        fs->ls->th, args_nr < MAX_ARGS, "too many func args");
                    args[args_nr++] = v.index;
                    if (!maybe_expect(fs->ls, ',')) {
                        break;
                    }
                }
            }
            expect(fs->ls, ')');

            first = ensure_at_top(fs, first);

            for (int i = 0; i < args_nr; i++) {
                emit(fs, INS_BUILD(PUSH, args[i], fs->proto->used_slots), 1);
            }

            int desired_retvals = 1;
            emit(fs,
                 INS_FROM_OP_AB_CD(Opcode::CALL, first.index, desired_retvals),
                 -args_nr - 1 + desired_retvals);
            first = SINGLE_NORMAL_VALUE_AT_TOP(fs, false);

        } else if (token == '[') {
            next(fs->ls);
            first = ensure_normal_value(fs, first);
            auto key = expr(fs, operations_order);
            key = ensure_normal_value(fs, key);
            first.type = SingleValueType::TABLE_SLOT;
            first.extras[0] = key.index;
            first.assignable = true;
            expect(fs->ls, ']');

        } else
            return first;

        token = peek(fs->ls)->token;
    }
}

static SingleValue expr(FuncState* fs, const OperationRule* rule)
{
    if (rule->type == 0) {
        return base_elem(fs);
    } else {
        if (rule->type == '2') {
            return bin_op(fs, rule);
        } else if (rule->type == '1') {
            return uni_op(fs, rule);
        } else if (rule->type == 'X') {
            return call_or_table_access(fs, rule);
        } else {
            Thread_throw(fs->ls->th, E::PARSE, "when parse expr");
        }
    }

    return single_value_none;
}

static void stmt_local(FuncState* fs)
{
    auto token = peek(fs->ls);
    if (token->token != TT_SYMBOL) {
        Thread_throw(fs->ls->th, E::PARSE, "expect a symbol");
    }
    const char* var_name = token->seminfo.s->base;
    StackIndex slot;
    if ((slot = Scope_search(fs->scope, var_name, nullptr)) < 0) {
        slot = MAX_USED_SLOT(fs) + 1;
        Scope_insert(fs->scope, var_name, slot);
        emit(fs, INS_BUILD(LOAD_NIL, fs->proto->used_slots), 1);
    }
    next(fs->ls);

    token = peek(fs->ls);
    if (token->token == '=') {
        next(fs->ls);

        auto v = expr(fs, operations_order);
        v = ensure_normal_value(fs, v);
        auto ins = INS_FROM_OP_AB_CD(Opcode::SET, slot, v.index);
        emit(fs, ins, 0);
    }

    // return single_normal_value(slot, true);
}

struct StmtResult {
    bool chunk_finished;
    // SingleValue value;
};

static StmtResult stmt_with_semi(FuncState* fs);
static void single_stmt_block(FuncState* fs);

static void embed_stmt_block(FuncState* fs)
{
    auto token = peek(fs->ls);
    if (token->token == '{') {
        next(fs->ls);
        block(fs);
        expect(fs->ls, '}');
    } else {
        single_stmt_block(fs);
    }
}

static StmtResult stmt(FuncState* fs)
{
    StmtResult r = { false };
    while (true) {

        auto token = peek(fs->ls);
        if (token->token != TT_EOF) {
            if (token->token == TT_RETURN) {
                next(fs->ls);
                ensure_at_top(
                    fs, ensure_normal_value(fs, expr(fs, operations_order)));
                emit(fs, INS_BUILD(RET_TOP), 0);

            } else if (token->token == '}') {
                r.chunk_finished = true;
            }

            else if (token->token == '{') {
                next(fs->ls);
                block(fs);
                expect(fs->ls, '}');
            }

            else if (token->token == TT_IF) {
                next(fs->ls);
                expect(fs->ls, '(');
                auto cond = expr(fs, operations_order);
                expect(fs->ls, ')');
                cond = ensure_normal_value(fs, cond);
                cond = ensure_at_top(fs, cond);

                auto jump_else = emit(fs, 0, 0);
                embed_stmt_block(fs);
                auto has_else = maybe_expect(fs->ls, TT_ELSE);
                InsIndex jump_end = 0;
                if (has_else) {
                    jump_end = emit(fs, 0, 0);
                }

                auto ins_nr = Proto_ins_nr(fs->proto);
                Proto_update_ins(
                    fs->proto,
                    jump_else,
                    INS_FROM_OP_ABCD(Opcode::JUMP_IF_FALSE, ins_nr));

                if (has_else) {
                    embed_stmt_block(fs);
                    ins_nr = Proto_ins_nr(fs->proto);

                    Proto_update_ins(fs->proto,
                                     jump_end,
                                     INS_FROM_OP_ABCD(Opcode::JUMP, ins_nr));
                }
            } else if (token->token == ';') {
                next(fs->ls);
                break;
            }

            else if (token->token == TT_LOCAL) {
                next(fs->ls);
                stmt_local(fs);
                break;
            }

            else if (token->token == TT_WHILE) {
                next(fs->ls);
                expect(fs->ls, '(');
                auto cond_label = Proto_ins_nr(fs->proto);
                emit_pop_to(fs, Scope_vars_nr2(fs->scope));
                auto cond = expr(fs, operations_order);
                expect(fs->ls, ')');
                cond = ensure_normal_value(fs, cond);
                cond = ensure_at_top(fs, cond);
                auto jemp_end = emit(fs, 0, 0);
                embed_stmt_block(fs);
                emit(fs, INS_FROM_OP_ABCD(Opcode::JUMP, cond_label), 0);
                auto end_label = Proto_ins_nr(fs->proto);
                Proto_update_ins(
                    fs->proto,
                    jemp_end,
                    INS_FROM_OP_ABCD(Opcode::JUMP_IF_FALSE, end_label));
                break;
            }

            else {
                expr(fs, operations_order);
            }
        } else {
            r.chunk_finished = true;
        }

        Size tmp_nr = fs->proto->used_slots - Scope_vars_nr2(fs->scope);
        if (tmp_nr != 0) {
            emit_pop_to(fs, Scope_vars_nr2(fs->scope));
        }

        return r;
    }

    // never reach
    return r;
}

static StmtResult stmt_with_semi(FuncState* fs)
{
    auto r = stmt(fs);
    if (!r.chunk_finished) {
        maybe_expect(fs->ls, ';');
    }
    return r;
}

static void enter_scope(FuncState* fs, Scope* scope)
{
    Scope_init(scope, fs->ls->th);
    scope->parent = fs->scope;
    fs->scope = scope;
    fs->proto->scope = fs->scope;
    scope->parent_used_slots = fs->proto->used_slots;
}

static void exit_scope(FuncState* fs)
{
    fs->proto->used_slots = fs->scope->parent_used_slots;
    fs->scope = fs->scope->parent;
    fs->proto->scope = fs->scope;
}

static void multi_stmts(FuncState* fs)
{
    auto r = stmt_with_semi(fs);
    while (!r.chunk_finished) {
        r = stmt_with_semi(fs);
    }
}

static void chunk(FuncState* fs)
{
    Scope scope;
    enter_scope(fs, &scope);
    multi_stmts(fs);
    exit_scope(fs);
}

static void single_stmt_chunk(FuncState* fs)
{
    Scope scope;
    Scope_init(&scope, fs->ls->th);
    scope.parent = fs->scope;
    fs->scope = &scope;
    fs->proto->scope = fs->scope;
    scope.parent_used_slots = fs->proto->used_slots;

    stmt_with_semi(fs);
    fs->scope = fs->scope->parent;
    fs->proto->scope = fs->scope;

    fs->proto->used_slots = scope.parent_used_slots;
}

static void single_stmt_block(FuncState* fs)
{
    single_stmt_chunk(fs);
    clear_scope_slots(fs);
}

static void func_body(FuncState* fs)
{
    chunk(fs);
    emit(fs, INS_BUILD(RET_0), 0);
}

static Proto* y_parser(Thread* th, ZIO* z, MBuffer* buff, const char* name)
{
    LexState ls;
    ls.th = th;
    ls.z = z;
    ls.buff = buff;
    ls.t = NONE;
    ls.peek = NONE;
    ls.current = EOF;

    FuncState fs;
    fs.ls = &ls;
    fs.proto = nullptr;
    fs.scope = nullptr;

    auto proto = Proto_new(th);
    fs.proto = proto;
    proto->name = Str_new(th, name, strlen(name));

    func_body(&fs);

    return fs.proto;
}

static void f_parser(Thread* th, void* ud)
{

    struct Proto* proto;
    struct Func* func;
    struct SParser* p = (struct SParser*)(ud);

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

void Scope_init(Scope* self, Thread* th)
{
    self->nr = 0;
    self->parent = nullptr;
    self->th = th;
    memset(self->flags, 0, sizeof(self->flags));
}

StackIndex
Scope_search(Scope* self, const char* name, uint8_t** flags, bool recursive)
{
    for (Size i = 0; i < self->nr; i++) {
        if (self->var_names[i] == name) {
            if (flags)
                *flags = &(self->flags[i]);
            return self->var_slots[i];
        }
    }

    if (recursive && self->parent) {
        return Scope_search(self->parent, name, flags, recursive);
    }
    return -1;
}

UpValuePos
UpValue_search(Thread* th, Proto* proto, const char* name, StackIndex deep)
{
    UpValuePos r;
    if (!proto) {
        r.u32 = 0xffffffff;
        return r;
    }

    if (proto->scope) {
        uint8_t* flags;
        auto slot = Scope_search(proto->scope, name, &flags, true);
        if (slot >= 0) {
            r.is_parent_uv = false;
            r.parent_frame_slot = slot;
            *flags |= 1;
            return r;
        }
    }

    auto parent_uv = UpValue_search(th, proto->parent, name, deep + 1);
    if (parent_uv.u32 == 0xffffffff)
        return parent_uv;

    auto uv_index = Proto_insert_uv(th, proto, parent_uv);
    r.is_parent_uv = true;
    r.parent_uv_index = uv_index;
    return r;
}

void Scope_insert(Scope* self, const char* name, StackIndex index)
{
    if (self->nr == MAX_VAR_NR)
        Thread_throw(self->th, E::PARSE, "Too many vars");

    self->var_names[self->nr++] = name;
    self->var_slots[self->nr - 1] = index;
}

} // namespace nf::imp