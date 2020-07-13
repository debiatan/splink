/* date = July 8th 2020 7:11 am */

#ifndef COMPUTE_H
#define COMPUTE_H

typedef enum{
    C_TokenKind_NULL,
    C_TokenKind_Space,
    C_TokenKind_Newline,
    C_TokenKind_Comment,
#define C_TokenKind_FIRST_HARD C_TokenKind_OpenParen
    C_TokenKind_OpenParen,
    C_TokenKind_CloseParen,
    C_TokenKind_Quote,
    C_TokenKind_Label,
#define C_TokenKind_LAST_HARD C_TokenKind_Label
    C_TokenKind_COUNT,
} C_TokenKind;

typedef struct C_Token C_Token;
struct C_Token{
    C_TokenKind kind;
    STR_Index string;
};

typedef struct C_TokenArray C_TokenArray;
struct C_TokenArray{
    C_Token *vals;
    u64 count;
};

////////////////////////////////

typedef u64 C_Id;

typedef struct C_Deferred C_Deferred;
struct C_Deferred{
    void *user_ptr;
    struct C_Cell *env;
    struct C_Cell *cell;
    b32 doing_eval;
    b32 finished_eval;
};

#define C_BUILT_IN_SIG(name) struct C_Cell* name(struct C_EvalCtx *ctx, struct C_Cell *cell, struct C_Cell **env)
typedef C_BUILT_IN_SIG(C_BuiltInFunctionType);

typedef enum{
    C_CellKind_Nil,
    C_CellKind_Constructed,
    C_CellKind_F64,
    C_CellKind_Id,
    C_CellKind_Identifier,
    
    // NOTE(allen): Used in eval but not emitted by parser
    C_CellKind_BuiltIn,
    C_CellKind_Environment,
    C_CellKind_Function,
    C_CellKind_Macro,
    C_CellKind_FunctionV,
    C_CellKind_MacroV,
    C_CellKind_Register,
    C_CellKind_Deferred,
    C_CellKind_Space,
    
    // NOTE(allen): Used in memory allocation strategy,
    // a correctly functioning lisp eval should not encounter this.
    // If it does it will be treated as Nil.
    C_CellKind_BlockHeader,
} C_CellKind;

// NOTE(allen): Forming verbs (functions, macros, variadics):
//  first_child -> constructed list of parameters
//              -> identifier for variadics
//  next -> expression forming body of function
//  env  -> environment inherited from enclosing scope

typedef struct C_Cell C_Cell;
struct C_Cell{
    C_CellKind kind;
    C_Cell *next;
    union{
        C_Cell *first_child;
        C_BuiltInFunctionType *built_in;
    };
    union{
        u64 x;
        struct C_CellBucket *bucket;
        void *space;
        f64 f;
        STR_Index identifier;
        C_Cell *env;
        C_Deferred *deferred;
        C_Id id;
        u64 depth_counter;
    };
};

// NOTE(allen): We want fast divides on whatever size we pick here.
#define C_BLOCK_CAP 256

typedef struct C_CellMemory C_CellMemory;
struct C_CellMemory{
    M_Arena arena;
    C_Cell *cells;
    C_Cell *free_block;
};

typedef struct C_CellBucket C_CellBucket;
struct C_CellBucket{
    C_CellMemory *memory;
    C_Cell *first_block;
    C_Cell *last_block;
    u64 cursor;
};

typedef struct C_ListBuilder C_ListBuilder;
struct C_ListBuilder{
    C_Cell *f;
    C_Cell **u;
    u64 c;
};

////////////////////////////////

typedef struct C_ParseError C_ParseError;
struct C_ParseError{
    String8 message;
    C_Token *token;
};

typedef struct C_ParseCtx C_ParseCtx;
struct C_ParseCtx{
    struct C_Statics *statics;
    C_CellBucket *bucket;
    C_Token *token;
    C_Token *opl;
    u64 error_count;
    u64 error_max;
    C_ParseError *errors;
};

////////////////////////////////

#define C_BuiltInXList(X) \
X("define", Define) \
X("function", Function) \
X("function...", FunctionV) \
X("macro", Macro) \
X("macro...", MacroV) \
X("write-reg", WriteRegister) \
X("read-reg", ReadRegister) \
X("get-thread-local-reg", GetUniqueRegister) \
X("block", Block) \
X("first", First) \
X("next", Next) \
X("push", Push) \
X("list", List) \
X("list-count", ListCount) \
X("id", Id) \
X("all-ids", AllIds) \
X("quote", Quote) \
X("eval", Eval) \
X("apply", Apply) \
X("far-eval", FarEval) \
X("kind", Kind) \
X("if", If) \
X("switch", Switch) \
X("loop", Loop) \
X("+1", Increment) \
X("-1", Decrement) \
X("+", Add) \
X("-", Sub) \
X("*", Mul) \
X("/", Div) \
X("%", Mod) \
X("^", Pow) \
X("<", Ls) \
X(">", Gr) \
X("<=", LsEq) \
X(">=", GrEq) \
X("==", Eq) \
X("!=", NotEq) \
X("not", Not) \
X("and", And) \
X("or", Or) \
X("ceil", Ceil) \
X("floor", Floor) \
X("round", Round)

typedef enum{
    C_BuiltInIndex_Nil,
    
#define EnumMember(N,F) C_BuiltInIndex_##F,
    C_BuiltInXList(EnumMember)
#undef EnumMember
    
    C_BuiltInIndex_COUNT,
} C_BuiltInIndex;

typedef struct C_Statics C_Statics;
struct C_Statics{
    union{
        struct{
            C_Cell *Nil;
#define StaticMember(N,F) C_Cell *F;
            C_BuiltInXList(StaticMember)
#undef StaticMember
        };
        C_Cell *built_in[C_BuiltInIndex_COUNT];
    };
    C_Cell *env;
};

////////////////////////////////

typedef struct C_EvalCtx C_EvalCtx;
struct C_EvalCtx{
    C_Statics *statics;
    C_CellBucket *bucket;
    C_Cell *id_env;
    C_Cell *regs;
    u64 max_depth;
    u64 depth;
    u64 loop_lim;
    C_Id reg_id_counter;
};

#endif //COMPUTE_H
