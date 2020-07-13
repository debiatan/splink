////////////////////////////////
// NOTE(allen): Cells

internal C_CellMemory
C_InitCellMemory(void){
    C_CellMemory memory = {0};
    memory.arena = M_ArenaInitializeWithAlign(1);
    memory.cells = PushArray(&memory.arena, C_Cell, C_BLOCK_CAP);
    {
        C_Cell *block = memory.cells;
        block->kind = C_CellKind_BlockHeader;
        SLLStackPush(memory.free_block, block);
    }
    return(memory);
}

internal C_Cell*
_C_AllocateBlock(C_CellMemory *memory){
    C_Cell *result = memory->free_block;
    if (result != 0){
        SLLStackPop(memory->free_block);
    }
    else{
        result = PushArray(&memory->arena, C_Cell, C_BLOCK_CAP);
        result->kind = C_CellKind_BlockHeader;
    }
    Assert((result - memory->cells)%C_BLOCK_CAP == 0);
    return(result);
}

internal void
_C_FreeBlock(C_CellMemory *memory, C_Cell *block){
    Assert((block - memory->cells)%C_BLOCK_CAP == 0);
    Assert(block->kind == C_CellKind_BlockHeader);
    SLLStackPush(memory->free_block, block);
}

internal C_CellBucket
C_InitCellBucket(C_CellMemory *memory){
    C_CellBucket bucket = {memory};
    bucket.first_block = bucket.last_block = 0;
    return(bucket);
}

internal C_Cell*
C_AllocateCell(C_CellBucket *bucket){
    C_Cell *result = 0;
    if (bucket->last_block != 0 && bucket->cursor < C_BLOCK_CAP){
        result = bucket->last_block + bucket->cursor;
        bucket->cursor += 1;
    }
    else{
        C_Cell *new_block = _C_AllocateBlock(bucket->memory);
        SLLQueuePush(bucket->first_block, bucket->last_block, new_block);
        new_block->bucket = bucket;
        result = new_block + 1;
        bucket->cursor = 2;
    }
    MemoryZeroStruct(result);
    return(result);
}

internal void
C_ClearBucket(C_CellBucket *bucket){
    for (C_Cell *block = bucket->first_block, *next = 0;
         block != 0;
         block = next){
        next = block->next;
        _C_FreeBlock(bucket->memory, block);
    }
    bucket->first_block = bucket->last_block = 0;
    bucket->cursor = 0;
}

internal C_Cell*
_C_BlockHeaderFromCell(C_CellMemory *memory, C_Cell *cell){
    u64 indx = (cell - memory->cells);
    u64 header_indx = indx - (indx%C_BLOCK_CAP);
    return(memory->cells + header_indx);
}

internal C_CellBucket*
_C_BucketFromCell(C_CellMemory *memory, C_Cell *cell){
    C_Cell *block = _C_BlockHeaderFromCell(memory, cell);
    return(block->bucket);
}

////////////////////////////////
// NOTE(allen): Cell Inits

internal C_Cell*
C_GetNilCell(C_Statics *statics){
    return(statics->Nil);
}

internal C_Cell*
C_GetBuiltIn(C_Statics *statics, C_BuiltInIndex index){
    C_Cell *result = statics->Nil;
    if (index < C_BuiltInIndex_COUNT){
        result = statics->built_in[index];
    }
    return(result);
}

internal String8
C_GetBuiltInKeyword(C_BuiltInIndex index){
    String8 result = {0};
    if (0 < index && index < C_BuiltInIndex_COUNT){
        local_persist String8 keywords[] = {
#define DefineS8Lit(N,F) {N, sizeof(N) - 1},
            C_BuiltInXList(DefineS8Lit)
#undef DefineS8Lit
        };
        result = keywords[index - 1];
    }
    return(result);
}

internal C_Cell*
C_NewCellPair(C_CellBucket *bucket, C_CellKind kind, C_Cell *left, C_Cell *right){
    C_Cell *cell = C_AllocateCell(bucket);
    cell->kind = kind;
    cell->first_child = left;
    cell->next = right;
    cell->depth_counter = right->depth_counter;
    AssertImplies(kind == C_CellKind_Constructed,
                  right->kind == C_CellKind_Constructed ||
                  right->kind == C_CellKind_Nil);
    return(cell);
}

#define C_InitListBuilder(C) {(C), (&(C)), 0}
#define C_ListBuilderPush_(Bt,Br,C) \
Stmnt( C_Cell *m_ = *Br.u = C_AllocateCell(Bt); m_->kind = C_CellKind_Constructed; m_->first_child = C; Br.u = &m_->next; Br.c += 1; )
#define C_ListBuilderPush(Bt,Br,C) C_ListBuilderPush_((Bt),(Br),(C))

internal void
_C_ListBuilderTerminate(C_ListBuilder builder){
    for (C_Cell *node = builder.f;
         node != 0;
         node = node->next){
        node->depth_counter = builder.c;
        builder.c -= 1;
    }
}
#define C_ListBuilderTerminate(S,Br) \
Stmnt( (*(Br).u = C_GetNilCell(S)); _C_ListBuilderTerminate(Br); )

internal C_Cell*
_C_NewCellList(C_Statics *statics, C_CellBucket *bucket, ...){
    va_list args;
    va_start(args, bucket);
    C_Cell *result = 0;
    C_ListBuilder builder = C_InitListBuilder(result);
    for (;;){
        C_Cell *cell = va_arg(args, C_Cell*);
        if (cell == 0){
            break;
        }
        C_ListBuilderPush(bucket, builder, cell);
    }
    C_ListBuilderTerminate(statics, builder);
    va_end(args);
    return(result);
}

#define C_NewCellList(S,B,...) _C_NewCellList((S), (B), __VA_ARGS__, 0)

internal C_Cell*
C_NewF64Cell(C_CellBucket *bucket, f64 f){
    C_Cell *cell = C_AllocateCell(bucket);
    cell->kind = C_CellKind_F64;
    cell->f = f;
    return(cell);
}

internal C_Cell*
C_NewIdCell(C_CellBucket *bucket, C_Id id){
    C_Cell *cell = C_AllocateCell(bucket);
    cell->kind = C_CellKind_Id;
    cell->id = id;
    return(cell);
}

internal C_Cell*
C_NewRegCell(C_CellBucket *bucket, C_Id id){
    C_Cell *cell = C_AllocateCell(bucket);
    cell->kind = C_CellKind_Register;
    cell->id = id;
    return(cell);
}

internal C_Cell*
C_NewIdentifierCell(C_CellBucket *bucket, STR_Index string){
    C_Cell *cell = C_AllocateCell(bucket);
    cell->kind = C_CellKind_Identifier;
    cell->identifier = string;
    return(cell);
}

internal C_Cell*
C_NewIdentifierCellLit(C_CellBucket *bucket, String8 string){
    return(C_NewIdentifierCell(bucket, STR_Save(APP_GetStringHash(), string)));
}

internal C_Cell*
C_NewCellFromLabel(C_Statics *statics, C_CellBucket *bucket, STR_Index string_indx){
    C_Cell *cell = C_GetNilCell(statics);
    
    String8 string = STR_Read(APP_GetStringHash(), string_indx);
    if (string.size > 0){
        if (CharIsDigit(string.str[0]) || string.str[0] == '.'){
            f64 f = 0;
            b32 error = 0;
            u8 *ptr = string.str;
            u64 i = 0;
            for (; i < string.size; i += 1, ptr += 1){
                if (*ptr == '.'){
                    i += 1;
                    ptr += 1;
                    break;
                }
                if (!CharIsDigit(*ptr)){
                    error = 1;
                    break;
                }
                f *= 10.;
                u64 x = (*ptr - '0');
                f += (f64)x;
            }
            f64 div = 1.;
            f64 frac = 0.;
            for (; i < string.size; i += 1, ptr += 1){
                if (!CharIsDigit(*ptr)){
                    error = 1;
                    break;
                }
                frac *= 10.;
                div *= 10.;
                u64 x = (*ptr - '0');
                frac += ((f64)x);
            }
            f += frac/div;
            if (!error){
                cell = C_NewF64Cell(bucket, f);
            }
        }
        
        else if (string.str[0] == '#'){
            C_Id id = 0;
            b32 error = 0;
            u8 *ptr = string.str + 1;
            u64 i = 1;
            for (; i < string.size; i += 1, ptr += 1){
                if (!CharIsDigit(*ptr)){
                    error = 1;
                    break;
                }
                id *= 10;
                u64 x = (*ptr - '0');
                id += (C_Id)x;
            }
            if (!error){
                cell = C_NewIdCell(bucket, id);
            }
        }
        
        else{
            cell = C_NewIdentifierCell(bucket, string_indx);
        }
    }
    
    return(cell);
}

internal C_Cell*
C_NewBuiltInCell(C_CellBucket *bucket, String8 name, C_BuiltInFunctionType *built_in){
    C_Cell *cell = C_AllocateCell(bucket);
    cell->kind = C_CellKind_BuiltIn;
    cell->built_in = built_in;
    cell->identifier = STR_Save(APP_GetStringHash(), name);
    return(cell);
}

internal C_Cell*
C_ExtendEnvironment(C_CellBucket *bucket, u64 x, C_Cell *body, C_Cell *env){
    Assert(x != 0 && body != 0 && env != 0);
    C_Cell *result = C_NewCellPair(bucket, C_CellKind_Environment, body, env);
    result->x = x;
    return(result);
}

internal C_Cell*
C_ExtendEnvironmentLiteral(C_CellBucket *bucket, String8 key, C_Cell *body, C_Cell *env){
    C_Cell *result = C_ExtendEnvironment(bucket, STR_Save(APP_GetStringHash(), key), body, env);
    return(result);
}

internal C_Cell*
C_NewVerbCell(C_CellBucket *bucket, C_CellKind kind, C_Cell *params, C_Cell *body, C_Cell *env){
    C_Cell *result = 0;
    if (params->kind == C_CellKind_Constructed ||
        params->kind == C_CellKind_Nil){
        b32 valid = 1;
        for (C_Cell *node = params;
             !C_IsNil(node);
             node = node->next){
            if (node->first_child->kind != C_CellKind_Identifier){
                valid = 0;
                break;
            }
        }
        if (valid){
            result = C_AllocateCell(bucket);
            result->kind = kind;
            result->first_child = params;
            result->next = body;
            result->env = env;
        }
    }
    return(result);
}

internal C_Cell*
C_NewVerbCellV(C_CellBucket *bucket, C_CellKind kind, C_Cell *params, C_Cell *body, C_Cell *env){
    C_Cell *result = 0;
    if (params->kind == C_CellKind_Identifier){
        result = C_AllocateCell(bucket);
        result->kind = kind;
        result->first_child = params;
        result->next = body;
        result->env = env;
    }
    return(result);
}

internal C_Cell*
C_NewDeferredCell(C_CellBucket *bucket, C_Deferred *deferred, C_Cell *cell){
    C_Cell *result  = C_AllocateCell(bucket);
    result->kind = C_CellKind_Deferred;
    result->first_child = cell;
    result->deferred = deferred;
    deferred->cell = cell;
    return(result);
}

internal C_Cell*
C_NewSpaceCell(C_CellBucket *bucket, void *space_ptr){
    C_Cell *cell = C_AllocateCell(bucket);
    cell->kind = C_CellKind_Space;
    cell->space = space_ptr;
    return(cell);
}

////////////////////////////////
// NOTE(allen): Parser

internal void
_C_ParseSkip(C_ParseCtx *ctx){
    C_Token *token = ctx->token;
    C_Token *opl = ctx->opl;
    for (;token < opl; token += 1){
        if (C_TokenKind_FIRST_HARD <= token->kind && token->kind <= C_TokenKind_LAST_HARD){
            break;
        }
    }
    ctx->token = token;
}

internal C_ParseCtx
_C_InitParseCtx(C_Statics *statics, C_CellBucket *bucket, C_Token *tokens, u64 count, C_ParseError *error_buffer, u64 error_count){
    C_ParseCtx ctx = {0};
    ctx.statics = statics;
    ctx.bucket = bucket;
    ctx.token = tokens;
    ctx.opl = tokens + count;
    ctx.error_max = error_count;
    ctx.errors = error_buffer;
    _C_ParseSkip(&ctx);
    return(ctx);
}

internal void
_C_ParseError(C_ParseCtx *ctx, String8 message, C_Token *token){
    if (ctx->error_count < ctx->error_max){
        C_ParseError *new_error = ctx->errors + ctx->error_count;
        ctx->error_count += 1;
        new_error->message = message;
        new_error->token = token;
    }
}


internal b32
_C_ParseEOF(C_ParseCtx *ctx){
    b32 result = (ctx->token >= ctx->opl);
    return(result);
}

internal b32
_C_ParsePeekToken(C_ParseCtx *ctx, C_Token **token_ptr){
    b32 result = 0;
    if (ctx->token < ctx->opl){
        *token_ptr = ctx->token;
        result = 1;
    }
    return(result);
}

internal b32
_C_ParseMatch(C_ParseCtx *ctx, C_TokenKind kind, C_Token **token_ptr){
    b32 result = 0;
    if (ctx->token < ctx->opl && ctx->token->kind == kind){
        *token_ptr = ctx->token;
        ctx->token += 1;
        _C_ParseSkip(ctx);
        result = 1;
    }
    return(result);
}

internal C_Cell* _C_ParseExpr(C_ParseCtx *ctx);

internal C_Cell*
_C_ParseExpr(C_ParseCtx *ctx){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Token *token = 0;
    
    // NOTE(allen): Any label starts an expression
    if (_C_ParseMatch(ctx, C_TokenKind_Label, &token)){
        result = C_NewCellFromLabel(ctx->statics, ctx->bucket, token->string);
    }
    
    // NOTE(allen): ' starts an expression
    else if (_C_ParseMatch(ctx, C_TokenKind_Quote, &token)){
        C_ListBuilder builder = C_InitListBuilder(result);
        C_Cell *quote = C_GetBuiltIn(ctx->statics, C_BuiltInIndex_Quote);
        C_ListBuilderPush(ctx->bucket, builder, quote);
        C_ListBuilderPush(ctx->bucket, builder, _C_ParseExpr(ctx));
        C_ListBuilderTerminate(ctx->statics, builder);
    }
    
    // NOTE(allen): An open paren starts an expression
    else if (_C_ParseMatch(ctx, C_TokenKind_OpenParen, &token)){
        C_ListBuilder builder = C_InitListBuilder(result);
        for (;;){
            C_Token *ignore = 0;
            if (_C_ParseMatch(ctx, C_TokenKind_CloseParen, &ignore)){
                break;
            }
            if (_C_ParseEOF(ctx)){
                _C_ParseError(ctx, S8Lit("Unclosed parentheses"), token);
                break;
            }
            C_ListBuilderPush(ctx->bucket, builder, _C_ParseExpr(ctx));
        }
        C_ListBuilderTerminate(ctx->statics, builder);
    }
    
    // NOTE(allen): No other valid ways to start an expression
    else{
        if (_C_ParsePeekToken(ctx, &token)){
            _C_ParseError(ctx, S8Lit("Unexpected token"), token);
        }
        if (_C_ParseEOF(ctx)){
            _C_ParseError(ctx, S8Lit("Unexpected end of file"), 0);
        }
    }
    
    return(result);
}

internal C_Cell*
C_Parse(C_Statics *statics, C_CellBucket *bucket, C_Token *tokens, u64 count,
        C_ParseError *error_buffer, u64 error_count, u64 *error_count_out){
    C_ParseCtx ctx = _C_InitParseCtx(statics, bucket, tokens, count, error_buffer, error_count);
    C_Cell *result = _C_ParseExpr(&ctx);
    *error_count_out = ctx.error_count;
    return(result);
}

////////////////////////////////
// NOTE(allen): Evaluation

internal C_EvalCtx
C_InitEvalCtx(C_Statics *statics, C_CellBucket *bucket, C_Cell *id_env){
    C_EvalCtx ctx = {0};
    ctx.statics = statics;
    ctx.bucket = bucket;
    ctx.id_env = id_env;
    ctx.regs = C_GetNilCell(statics);
    ctx.max_depth = 100;
    ctx.loop_lim = Thousand(1);
    ctx.reg_id_counter = 1;
    return(ctx);
}

#define DeclBuiltIn(N,F) internal C_BUILT_IN_SIG(C_BuiltIn_##F);
C_BuiltInXList(DeclBuiltIn);
#undef DeclBuiltIn

internal C_Cell* C_Evaluate(C_EvalCtx *ctx, C_Cell *cell, C_Cell **env);

internal b32
C_IsNil(C_Cell *cell){
    return(cell == 0 || cell->kind == C_CellKind_Nil || cell->kind >= C_CellKind_BlockHeader);
}

internal b32
C_IsF64(C_Cell *cell){
    return(cell != 0 && cell->kind == C_CellKind_F64);
}

internal b32
C_IsTruthy(C_Cell *cell){
    b32 result = 0;
    if (!C_IsNil(cell)){
        if (cell->kind != C_CellKind_F64 || cell->f != 0.){
            result = 1;
        }
    }
    return(result);
}

internal STR_Index
C_GetIdentifierFromCell(C_Cell *cell){
    STR_Index result = 0;
    if (cell != 0 && cell->kind == C_CellKind_Identifier){
        result = cell->identifier;
    }
    return(result);
}

internal C_Id
C_GetIdFromCell(C_Cell *cell){
    C_Id result = 0;
    if (cell != 0 && cell->kind == C_CellKind_Id){
        result = cell->id;
    }
    return(result);
}

internal u64
C_ListCount(C_Cell *cell){
    u64 count = 0;
    for (C_Cell *node = cell;
         !C_IsNil(node);
         node = node->next){
        count += 1;
    }
    return(count);
}

internal b32
C_CellEquality(C_Cell *a, C_Cell *b){
    b32 result = 0;
    b32 an = C_IsNil(a);
    b32 bn = C_IsNil(b);
    if (an || bn){
        if (an && bn){
            result = 1;
        }
    }
    else if (a->kind == b->kind){
        switch (a->kind){
            default:
            {
                result = 1;
            }break;
            
            case C_CellKind_Constructed:
            {
                result = (C_CellEquality(a->first_child, b->first_child) && C_CellEquality(a->next, b->next));
            }break;
            
            case C_CellKind_F64:
            case C_CellKind_Identifier:
            case C_CellKind_Id:
            case C_CellKind_Register:
            case C_CellKind_BuiltIn:
            {
                result = (a->x == b->x);
            }break;
            
            case C_CellKind_Function:
            case C_CellKind_FunctionV:
            case C_CellKind_Macro:
            case C_CellKind_MacroV:
            {
                result = (a->first_child == b->first_child && a->next == b->next && C_CellEquality(a->env, b->env));
            }break;
        }
    }
    return(result);
}

internal C_Cell*
C_EnvironmentResolve(C_Statics *statics, C_Cell *env, u64 lookup_key){
    C_Cell *result = C_GetNilCell(statics);
    
    for (;!C_IsNil(env) && env->kind == C_CellKind_Environment;
         env = env->next){
        u64 key = env->x;
        C_Cell *val = env->first_child;
        if (key == 0 || val == 0){
            break;
        }
        if (key == lookup_key){
            result = val;
            break;
        }
    }
    
    return(result);
}

internal C_Cell*
C_Apply(C_EvalCtx *ctx, C_Cell *verb, C_Cell *args, C_Cell **env){
    C_Cell *result = C_GetNilCell(ctx->statics);
    
    Assert(verb != 0 && args != 0 && *env != 0);
    
    if (ctx->depth < ctx->max_depth){
        ctx->depth += 1;
        
        switch (verb->kind){
            default: break;
            
            case C_CellKind_BuiltIn:
            {
                result = verb->built_in(ctx, args, env);
            }break;
            
            case C_CellKind_Function:
            {
                // NOTE(allen): Step 1: check that arguments match parameters
                C_Cell *params = verb->first_child;
                u64 args_count = C_ListCount(args);
                u64 param_count = C_ListCount(verb->first_child);
                if (args_count == param_count){
                    
                    // NOTE(allen): Step 2: evaluate and record each argument
                    C_Cell *old_env = verb->env;
                    C_Cell *new_env = old_env;
                    for (C_Cell *arg_node = args, *param_node = params;
                         !C_IsNil(arg_node) && !C_IsNil(params);
                         arg_node = arg_node->next, param_node = param_node->next){
                        C_Cell *evaluated = C_Evaluate(ctx, arg_node->first_child, env);
                        STR_Index name = C_GetIdentifierFromCell(param_node->first_child);
                        new_env = C_ExtendEnvironment(ctx->bucket, name, evaluated, new_env);
                    }
                    
                    // NOTE(allen): Step 3: evaluate the body expression and return the result
                    result = C_Evaluate(ctx, verb->next, &new_env);
                }
            }break;
            
            case C_CellKind_FunctionV:
            {
                // NOTE(allen): Step 1: evaluate and list each argument
                C_Cell *params = 0;
                C_ListBuilder builder = C_InitListBuilder(params);
                for (C_Cell *arg_node = args;
                     !C_IsNil(arg_node);
                     arg_node = arg_node->next){
                    C_Cell *evaluated = C_Evaluate(ctx, arg_node->first_child, env);
                    C_ListBuilderPush(ctx->bucket, builder, evaluated);
                }
                C_ListBuilderTerminate(ctx->statics, builder);
                
                // NOTE(allen): Step 2: record the argument list
                STR_Index name = C_GetIdentifierFromCell(verb->first_child);
                C_Cell *new_env = C_ExtendEnvironment(ctx->bucket, name, params, verb->env);
                
                // NOTE(allen): Step 3: evaluate the body expression and return the result
                result = C_Evaluate(ctx, verb->next, &new_env);
            }break;
            
            case C_CellKind_Macro:
            {
                // NOTE(allen): Step 1: check that arguments match parameters
                C_Cell *params = verb->first_child;
                u64 args_count = C_ListCount(args);
                u64 param_count = C_ListCount(verb->first_child);
                if (args_count == param_count){
                    
                    // NOTE(allen): Step 2: record each argument
                    C_Cell *old_env = verb->env;
                    C_Cell *new_env = old_env;
                    for (C_Cell *arg_node = args, *param_node = params;
                         !C_IsNil(arg_node) && !C_IsNil(params);
                         arg_node = arg_node->next, param_node = param_node->next){
                        STR_Index name = C_GetIdentifierFromCell(param_node->first_child);
                        new_env = C_ExtendEnvironment(ctx->bucket, name, arg_node->first_child, new_env);
                    }
                    
                    // NOTE(allen): Step 3: evaluate the body expression and return the result
                    C_Cell *macro_eval = C_Evaluate(ctx, verb->next, &new_env);
                    
                    // NOTE(allen): Step 4: evaluate in the calling context
                    result = C_Evaluate(ctx, macro_eval, env);
                }
            }break;
            
            case C_CellKind_MacroV:
            {
                // NOTE(allen): Step 1: list each argument
                C_Cell *params = 0;
                C_ListBuilder builder = C_InitListBuilder(params);
                for (C_Cell *arg_node = args;
                     !C_IsNil(arg_node);
                     arg_node = arg_node->next){
                    C_ListBuilderPush(ctx->bucket, builder, arg_node->first_child);
                }
                C_ListBuilderTerminate(ctx->statics, builder);
                
                // NOTE(allen): Step 2: record the argument list
                STR_Index name = C_GetIdentifierFromCell(verb->first_child);
                C_Cell *new_env = C_ExtendEnvironment(ctx->bucket, name, params, verb->env);
                
                // NOTE(allen): Step 3: evaluate the body expression and return the result
                C_Cell *macro_eval = C_Evaluate(ctx, verb->next, &new_env);
                
                // NOTE(allen): Step 4: evaluate in the calling context
                result = C_Evaluate(ctx, macro_eval, env);
            }break;
        }
        
        ctx->depth -= 1;
    }
    
    Assert(result != 0);
    Assert(result->kind < C_CellKind_BlockHeader);
    
    return(result);
}

internal C_Cell*
C_EvaluateDeferred(C_EvalCtx *ctx, C_Deferred *deferred, C_Cell **env){
    C_Cell *result = C_GetNilCell(ctx->statics);
    if (!deferred->doing_eval){
        if (!deferred->finished_eval){
            deferred->doing_eval = 1;
            C_Cell *defer_env = deferred->env;
            deferred->cell = C_Evaluate(ctx, deferred->cell, &defer_env);
            Assert(deferred->cell->kind != C_CellKind_Deferred);
            deferred->doing_eval = 0;
        }
        deferred->finished_eval = 1;
        result = deferred->cell;
    }
    return(result);
}

internal C_Cell*
C_Evaluate(C_EvalCtx *ctx, C_Cell *cell, C_Cell **env){
    C_Cell *result = C_GetNilCell(ctx->statics);
    
    if (cell != 0 && env != 0 && *env != 0){
        switch (cell->kind){
            default: break;
            
            case C_CellKind_Constructed:
            {
                C_Cell *verb = C_Evaluate(ctx, cell->first_child, env);
                result = C_Apply(ctx, verb, cell->next, env);
            }break;
            
            case C_CellKind_F64:
            case C_CellKind_BuiltIn:
            case C_CellKind_Function:
            case C_CellKind_Macro:
            case C_CellKind_FunctionV:
            case C_CellKind_MacroV:
            case C_CellKind_Deferred:
            case C_CellKind_Space:
            case C_CellKind_Register:
            {
                result = cell;
            }break;
            
            case C_CellKind_Identifier:
            {
                result = C_EnvironmentResolve(ctx->statics, *env, C_GetIdentifierFromCell(cell));
            }break;
            
            case C_CellKind_Id:
            {
                result = C_EnvironmentResolve(ctx->statics, ctx->id_env, cell->id);
            }break;
        }
    }
    
    if (result->kind == C_CellKind_Deferred){
        result = C_EvaluateDeferred(ctx, result->deferred, env);
    }
    
    Assert(result != 0);
    Assert(result->kind < C_CellKind_BlockHeader);
    
    return(result);
}

internal void
C_EvaluateArray(C_EvalCtx *ctx, C_Cell **cell, u64 count, C_Cell **env){
    for (u64 i = 0; i < count; i += 1){
        cell[i] = C_Evaluate(ctx, cell[i], env);
    }
}

////////////////////////////////
// NOTE(allen): Built Ins

typedef struct C_NumComp C_NumComp;
struct C_NumComp{
    b32 valid;
    f64 f;
};

internal C_Cell*
C_GetCellFromNumComp(C_EvalCtx *ctx, C_NumComp *comp){
    C_Cell *result = C_GetNilCell(ctx->statics);
    if (comp->valid){
        result = C_NewF64Cell(ctx->bucket, comp->f);
    }
    return(result);
}

internal C_Cell*
C_GetCellFromBoolean(C_EvalCtx *ctx, b32 b){
    C_Cell *result = C_GetNilCell(ctx->statics);
    if (b){
        result = C_NewF64Cell(ctx->bucket, 1.f);
    }
    return(result);
}

internal b32
C_TakeFixedArguments(C_Cell *cell, C_Cell **out, u64 count){
    b32 result = 1;
    for (u64 i = 0; i < count; i += 1){
        if (cell == 0 || cell->kind != C_CellKind_Constructed ||
            cell->first_child == 0){
            result = 0;
            break;
        }
        out[i] = cell->first_child;
        cell = cell->next;
    }
    if (!C_IsNil(cell)){
        result = 0;
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Function){
    C_Cell *result = 0;
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        args[0] = C_Evaluate(ctx, args[0], env);
        result = C_NewVerbCell(ctx->bucket, C_CellKind_Function, args[0], args[1], *env);
    }
    if (result == 0){
        result = C_GetNilCell(ctx->statics);
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_FunctionV){
    C_Cell *result = 0;
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        args[0] = C_Evaluate(ctx, args[0], env);
        result = C_NewVerbCellV(ctx->bucket, C_CellKind_FunctionV, args[0], args[1], *env);
    }
    if (result == 0){
        result = C_GetNilCell(ctx->statics);
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Macro){
    C_Cell *result = 0;
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        args[0] = C_Evaluate(ctx, args[0], env);
        result = C_NewVerbCell(ctx->bucket, C_CellKind_Macro, args[0], args[1], *env);
    }
    if (result == 0){
        result = C_GetNilCell(ctx->statics);
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_MacroV){
    C_Cell *result = 0;
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        args[0] = C_Evaluate(ctx, args[0], env);
        result = C_NewVerbCellV(ctx->bucket, C_CellKind_MacroV, args[0], args[1], *env);
    }
    if (result == 0){
        result = C_GetNilCell(ctx->statics);
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_WriteRegister){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_Register){
            result = args[1];
            ctx->regs = C_ExtendEnvironment(ctx->bucket, args[0]->id, args[1], ctx->regs);
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_ReadRegister){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_Register){
            result = C_EnvironmentResolve(ctx->statics, ctx->regs, args[0]->id);
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_GetUniqueRegister){
    C_Cell *result = C_NewRegCell(ctx->bucket, ctx->reg_id_counter);
    ctx->reg_id_counter += 1;
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Block){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *local_env = *env;
    for (C_Cell *node = cell;
         !C_IsNil(node);
         node = node->next){
        result = C_Evaluate(ctx, node->first_child, &local_env);
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Quote){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        result = args[0];
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_First){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_Constructed){
            result = args[0]->first_child;
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Next){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_Constructed){
            result = args[0]->next;
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Push){
    C_Cell *result = C_GetNilCell(ctx->statics);
    M_Arena *scratch = OS_GetScratch();
    C_Cell **push_cells = PushArray(scratch, C_Cell*, 0);
    for (C_Cell *node = cell;
         !C_IsNil(node);
         node = node->next){
        C_Cell **cell_ptr = PushArray(scratch, C_Cell*, 1);
        *cell_ptr = C_Evaluate(ctx, node->first_child, env);
    }
    C_Cell **opl = PushArray(scratch, C_Cell*, 0);
    u64 count = (opl - push_cells);
    if (count > 0){
        result = opl[-1];
        if (result->kind == C_CellKind_Nil ||
            result->kind == C_CellKind_Constructed){
            for (C_Cell **cell_ptr = opl - 2;
                 cell_ptr >= push_cells;
                 cell_ptr -= 1){
                result = C_NewCellPair(ctx->bucket, C_CellKind_Constructed, *cell_ptr, result);
            }
        }
    }
    OS_ReleaseScratch(scratch);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_List){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_ListBuilder builder = C_InitListBuilder(result);
    for (C_Cell *node = cell;
         !C_IsNil(node);
         node = node->next){
        C_Cell *evaluated = C_Evaluate(ctx, node->first_child, env);
        C_ListBuilderPush(ctx->bucket, builder, evaluated);
    }
    C_ListBuilderTerminate(ctx->statics, builder);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_ListCount){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_Constructed ||
            args[0]->kind == C_CellKind_Nil){
            result = C_NewF64Cell(ctx->bucket, (f64)args[0]->f);
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Id){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_Identifier){
            C_Cell *resolved = C_EnvironmentResolve(ctx->statics, *env, args[0]->identifier);
            if (resolved != 0 && resolved->kind == C_CellKind_Deferred){
                E_Definition *resolved_definition = (E_Definition*)resolved->deferred->user_ptr;
                result = C_NewIdCell(ctx->bucket, resolved_definition->id);
            }
        }
        else if (args[0]->kind == C_CellKind_Id){
            result = args[0];
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_AllIds){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_Space){
            E_Space *space = (E_Space*)args[0]->space;
            result = space->id_list;
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Eval){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        result = C_Evaluate(ctx, args[0], env);
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Apply){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        result = C_Apply(ctx, args[0], args[1], env);
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_FarEval){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_Space){
            E_Space *space = (E_Space*)args[0]->space;
            C_EvalCtx far_ctx = C_InitEvalCtx(ctx->statics, ctx->bucket, space->id_env);
            far_ctx.max_depth = ctx->max_depth;
            far_ctx.depth = ctx->depth;
            far_ctx.regs = ctx->regs;
            C_Cell *far_env = space->defines_env;
            result = C_Evaluate(&far_ctx, args[1], &far_env);
            ctx->regs = far_ctx.regs;
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Kind){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        switch (args[0]->kind){
            case C_CellKind_Nil:
            {
                result = C_NewIdentifierCellLit(ctx->bucket, S8Lit("Nil"));
            }break;
            
            case C_CellKind_Constructed:
            {
                result = C_NewIdentifierCellLit(ctx->bucket, S8Lit("Constructed"));
            }break;
            
            case C_CellKind_F64:
            case C_CellKind_BuiltIn:
            case C_CellKind_Function:
            case C_CellKind_Macro:
            case C_CellKind_FunctionV:
            case C_CellKind_MacroV:
            case C_CellKind_Space:
            case C_CellKind_Register:
            {
                result = C_NewIdentifierCellLit(ctx->bucket, S8Lit("Idempotent"));
            }break;
            
            case C_CellKind_Id:
            {
                result = C_NewIdentifierCellLit(ctx->bucket, S8Lit("Id"));
            }break;
            
            case C_CellKind_Identifier:
            {
                result = C_NewIdentifierCellLit(ctx->bucket, S8Lit("Identifier"));
            }break;
            
            case C_CellKind_Environment:
            {
                result = C_NewIdentifierCellLit(ctx->bucket, S8Lit("Environment"));
            }break;
            
            case C_CellKind_Deferred:
            {
                result = C_NewIdentifierCellLit(ctx->bucket, S8Lit("Deferred"));
            }break;
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Define){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        args[0] = C_Evaluate(ctx, args[0], env);
        STR_Index name = C_GetIdentifierFromCell(args[0]);
        if (name != 0){
            C_Cell *body = C_Evaluate(ctx, args[1], env);
            *env = C_ExtendEnvironment(ctx->bucket, name, body, *env);
            // NOTE(allen): When we define a function, the function's environment will
            // not see itself! This is inconvenient so we poke it in here.
            if (body->kind == C_CellKind_Function){
                body->env = *env;
            }
            result = body;
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Increment){
    C_NumComp comp = {0};
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_F64){
            comp.f = args[0]->f + 1.0;
            comp.valid = 1;
        }
    }
    C_Cell *result = C_GetCellFromNumComp(ctx, &comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Decrement){
    C_NumComp comp = {0};
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_F64){
            comp.f = args[0]->f - 1.0;
            comp.valid = 1;
        }
    }
    C_Cell *result = C_GetCellFromNumComp(ctx, &comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Add){
    C_NumComp comp = {1};
    for (C_Cell *node = cell;
         !C_IsNil(node);
         node = node->next){
        C_Cell *evaluated = C_Evaluate(ctx, node->first_child, env);
        if (C_IsF64(evaluated)){
            comp.f += evaluated->f;
        }
        else{
            comp.valid = 0;
        }
    }
    C_Cell *result = C_GetCellFromNumComp(ctx, &comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Mul){
    C_NumComp comp = {1, 1.};
    for (C_Cell *node = cell;
         !C_IsNil(node);
         node = node->next){
        C_Cell *evaluated = C_Evaluate(ctx, node->first_child, env);
        if (C_IsF64(evaluated)){
            comp.f *= evaluated->f;
        }
        else{
            comp.valid = 0;
        }
    }
    C_Cell *result = C_GetCellFromNumComp(ctx, &comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Sub){
    C_NumComp comp = {0};
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (C_IsF64(args[0]) && C_IsF64(args[1])){
            comp.valid = 1;
            comp.f = args[0]->f - args[1]->f;
        }
    }
    C_Cell *result = C_GetCellFromNumComp(ctx, &comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Div){
    C_NumComp comp = {0};
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (C_IsF64(args[0]) && C_IsF64(args[1])){
            comp.valid = 1;
            comp.f = args[0]->f/args[1]->f;
        }
    }
    C_Cell *result = C_GetCellFromNumComp(ctx, &comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Mod){
    C_NumComp comp = {0};
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (C_IsF64(args[0]) && C_IsF64(args[1])){
            comp.valid = 1;
            comp.f = FMod(args[0]->f, args[1]->f);
        }
    }
    C_Cell *result = C_GetCellFromNumComp(ctx, &comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Pow){
    C_NumComp comp = {0};
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (C_IsF64(args[0]) && C_IsF64(args[1])){
            comp.valid = 1;
            comp.f = Pow32(args[0]->f, args[1]->f);
        }
    }
    C_Cell *result = C_GetCellFromNumComp(ctx, &comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Ls){
    b32 comp = 0;
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (C_IsF64(args[0]) && C_IsF64(args[1])){
            comp = (args[0]->f < args[1]->f);
        }
    }
    C_Cell *result = C_GetCellFromBoolean(ctx, comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Gr){
    b32 comp = 0;
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (C_IsF64(args[0]) && C_IsF64(args[1])){
            comp = (args[0]->f > args[1]->f);
        }
    }
    C_Cell *result = C_GetCellFromBoolean(ctx, comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_LsEq){
    b32 comp = 0;
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (C_IsF64(args[0]) && C_IsF64(args[1])){
            comp = (args[0]->f <= args[1]->f);
        }
    }
    C_Cell *result = C_GetCellFromBoolean(ctx, comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_GrEq){
    b32 comp = 0;
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (C_IsF64(args[0]) && C_IsF64(args[1])){
            comp = (args[0]->f >= args[1]->f);
        }
    }
    C_Cell *result = C_GetCellFromBoolean(ctx, comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Eq){
    b32 comp = 0;
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        comp = C_CellEquality(args[0], args[1]);
    }
    C_Cell *result = C_GetCellFromBoolean(ctx, comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_NotEq){
    b32 comp = 0;
    C_Cell *args[2];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        comp = !C_CellEquality(args[0], args[1]);
    }
    C_Cell *result = C_GetCellFromBoolean(ctx, comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Not){
    b32 comp = 0;
    C_Cell *args[1];
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        comp = !C_IsTruthy(args[0]);
    }
    C_Cell *result = C_GetCellFromBoolean(ctx, comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_And){
    b32 comp = 1;
    for (C_Cell *node = cell;
         comp && !C_IsNil(node);
         node = node->next){
        comp = comp && C_IsTruthy(node->first_child);
    }
    C_Cell *result = C_GetCellFromBoolean(ctx, comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Or){
    b32 comp = 0;
    for (C_Cell *node = cell;
         !comp && !C_IsNil(node);
         node = node->next){
        comp = comp || C_IsTruthy(node->first_child);
    }
    C_Cell *result = C_GetCellFromBoolean(ctx, comp);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_If){
    C_Cell *args[3];
    C_Cell *result = C_GetNilCell(ctx->statics);
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        if (C_IsTruthy(C_Evaluate(ctx, args[0], env))){
            result = C_Evaluate(ctx, args[1], env);
        }
        else{
            result = C_Evaluate(ctx, args[2], env);
        }
    }
    else if (C_TakeFixedArguments(cell, args, 2)){
        if (C_IsTruthy(C_Evaluate(ctx, args[0], env))){
            result = C_Evaluate(ctx, args[1], env);
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Switch){
    C_Cell *result = C_GetNilCell(ctx->statics);
    C_Cell *node = cell;
    C_Cell *switch_val = C_Evaluate(ctx, node->first_child, env);
    node = node->next;
    for (;!C_IsNil(node);){
        C_Cell *extended_env = *env;
        C_Cell *check_val = C_Evaluate(ctx, node->first_child, &extended_env);
        if (C_CellEquality(switch_val, check_val)){
            node = node->next;
            result = C_Evaluate(ctx, node->first_child, &extended_env);
            *env = extended_env;
            break;
        }
        node = node->next;
        if (!C_IsNil(node)){
            node = node->next;
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Loop){
    u64 count = 0;
    for (u64 lim = 0;
         lim < ctx->loop_lim &&
         C_IsTruthy(C_Evaluate(ctx, cell->first_child, env));
         lim += 1){
        for (C_Cell *node = cell->next;
             !C_IsNil(node);
             node = node->next){
            C_Evaluate(ctx, node->first_child, env);
        }
        count += 1;
    }
    C_Cell *result = C_NewF64Cell(ctx->bucket, (f64)count);
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Ceil){
    C_Cell *args[1];
    C_Cell *result = C_GetNilCell(ctx->statics);
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_F64){
            result = C_NewF64Cell(ctx->bucket, f32Ceil(args[0]->f));
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Floor){
    C_Cell *args[1];
    C_Cell *result = C_GetNilCell(ctx->statics);
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_F64){
            result = C_NewF64Cell(ctx->bucket, f32Floor(args[0]->f));
        }
    }
    return(result);
}

internal
C_BUILT_IN_SIG(C_BuiltIn_Round){
    C_Cell *args[1];
    C_Cell *result = C_GetNilCell(ctx->statics);
    if (C_TakeFixedArguments(cell, args, ArrayCount(args))){
        C_EvaluateArray(ctx, args, ArrayCount(args), env);
        if (args[0]->kind == C_CellKind_F64){
            result = C_NewF64Cell(ctx->bucket, f32Round(args[0]->f));
        }
    }
    return(result);
}

////////////////////////////////
// NOTE(allen): Statics

internal C_Statics
C_InitStatics(C_CellBucket *bucket){
    C_Statics statics = {0};
    
    statics.Nil = C_AllocateCell(bucket);
    statics.env = statics.Nil;
#define DefineStatic(N,F) \
statics.F = C_NewBuiltInCell(bucket, S8Lit(N), C_BuiltIn_##F); \
statics.env = C_ExtendEnvironmentLiteral(bucket, S8Lit(N), statics.F, statics.env);
    C_BuiltInXList(DefineStatic)
#undef DefineStatic
    return(statics);
}

////////////////////////////////
// NOTE(allen): Stringize

internal void
C_StringizeCellStream(M_Arena *arena, C_Cell *cell, String8_List *list){
    switch (cell->kind){
        default:
        {
            StringListPush(arena, list, S8Lit("[err]()"));
        }break;
        
        case C_CellKind_Nil:
        {
            StringListPush(arena, list, S8Lit("()"));
        }break;
        
        case C_CellKind_Constructed:
        {
            b32 is_list = 1;
            for (C_Cell *node = cell;
                 !C_IsNil(node);
                 node = node->next){
                if (node->kind != C_CellKind_Constructed){
                    is_list = 0;
                    break;
                }
            }
            
            StringListPush(arena, list, S8Lit("("));
            for (C_Cell *node = cell;
                 !C_IsNil(node);
                 node = node->next){
                if (node != cell){
                    StringListPush(arena, list, S8Lit(" "));
                }
                if (node->kind == C_CellKind_Constructed){
                    C_StringizeCellStream(arena, node->first_child, list);
                }
                else{
                    C_StringizeCellStream(arena, node, list);
                    break;
                }
            }
            StringListPush(arena, list, S8Lit(")"));
        }break;
        
        case C_CellKind_F64:
        {
            String8_Node *node = StringListPushF(arena, list, "%f", cell->f);
            u64 og_size = node->string.size;
            u8 *first = node->string.str;
            u8 *ptr = first;
            u8 *opl = first + node->string.size;
            u8 *saw_period_at = 0;
            u8 *last_significand_at = 0;
            for (;ptr < opl; ptr += 1){
                if (*ptr == '.'){
                    saw_period_at = ptr;
                    ptr += 1;
                    for (;ptr < opl; ptr += 1){
                        if (*ptr != '0'){
                            last_significand_at = ptr;
                        }
                    }
                }
            }
            if (last_significand_at != 0){
                node->string.size = (last_significand_at + 1 - first);
            }
            else if (saw_period_at != 0){
                node->string.size = (saw_period_at - first);
            }
            list->total_size -= (og_size - node->string.size);
        }break;
        
        case C_CellKind_Identifier:
        {
            StringListPush(arena, list, STR_Read(APP_GetStringHash(), cell->identifier));
        }break;
        
        case C_CellKind_Id:
        {
            StringListPushF(arena, list, "#%llu", cell->id);
        }break;
        
        case C_CellKind_Register:
        {
            StringListPushF(arena, list, "[reg] #%llu", cell->id);
        }break;
        
        case C_CellKind_Environment:
        {
            StringListPush(arena, list, S8Lit("[env] "));
            for (C_Cell *node = cell;
                 !C_IsNil(node);
                 node = node->next){
                if (node != cell){
                    StringListPush(arena, list, S8Lit(" "));
                }
                C_StringizeCellStream(arena, node->first_child, list);
            }
        }break;
        
        case C_CellKind_BuiltIn:
        {
            String8 identifier = STR_Read(APP_GetStringHash(), cell->identifier);
            StringListPushF(arena, list, "[prm]%.*s", StringExpand(identifier));
        }break;
        
        case C_CellKind_Function:
        {
            StringListPush(arena, list, S8Lit("[fun] "));
        }goto verb_body;
        
        case C_CellKind_FunctionV:
        {
            StringListPush(arena, list, S8Lit("[fnv] "));
        }goto verb_body;
        
        case C_CellKind_Macro:
        {
            StringListPush(arena, list, S8Lit("[mac] "));
        }goto verb_body;
        
        case C_CellKind_MacroV:
        {
            StringListPush(arena, list, S8Lit("[mcv] "));
        }goto verb_body;
        
        verb_body:
        {
            C_StringizeCellStream(arena, cell->first_child, list);
            StringListPush(arena, list, S8Lit(" "));
            C_StringizeCellStream(arena, cell->next, list);
        }break;
        
        case C_CellKind_Deferred:
        {
            StringListPush(arena, list, S8Lit("[def]"));
        }break;
        
        case C_CellKind_BlockHeader:
        {
            StringListPush(arena, list, S8Lit("[blk]"));
        }break;
    }
}

internal String8
C_StringizeCell(M_Arena *arena, C_Cell *cell){
    String8_List list = {0};
    C_StringizeCellStream(arena, cell, &list);
    return(StringListJoin(arena, &list, 0));
}

////////////////////////////////
// NOTE(allen): Lex

internal b32
C_ValidLabel(String8 string){
    b32 result = (string.size > 0);
    u8 *ptr = string.str;
    u8 *opl = string.str + string.size;
    for (;result && ptr < opl; ptr += 1){
        switch (*ptr){
            case '(':
            case ')':
            case '\'':
            case ';':
            {
                result = 0;
            }break;
            default:
            {
                if (*ptr <= ' ' || 127 <= *ptr){
                    result = 0;
                }
            }break;
        }
    }
    return(result);
}

internal void
C_StringizeTokensStream(M_Arena *arena, C_Token *tokens, u64 count, String8_List *out){
    u8 escape_byte = 1;
    String8 escape = S8(&escape_byte, 1);
    STR_Hash *string_hash = APP_GetStringHash();
    C_Token *token = tokens;
    for (u64 i = 0; i < count; i += 1, token += 1){
        switch (token->kind){
            default:
            {
                StringListPush(arena, out, escape);
                StringListPushF(arena, out, "%u", token->kind);
                StringListPush(arena, out, escape);
            }break;
            
            case C_TokenKind_Space:
            {
                StringListPush(arena, out, S8Lit(" "));
            }break;
            
            case C_TokenKind_Newline:
            {
                StringListPush(arena, out, S8Lit("\n"));
            }break;
            
            case C_TokenKind_Comment:
            {
                String8 string = STR_Read(string_hash, token->string);
                StringListPushF(arena, out, ";%.*s\t", StringExpand(string));
            }break;
            
            case C_TokenKind_OpenParen:
            {
                StringListPush(arena, out, S8Lit("("));
            }break;
            
            case C_TokenKind_CloseParen:
            {
                StringListPush(arena, out, S8Lit(")"));
            }break;
            
            case C_TokenKind_Quote:
            {
                StringListPush(arena, out, S8Lit("\'"));
            }break;
            
            case C_TokenKind_Label:
            {
                String8 string = STR_Read(string_hash, token->string);
                StringListPushF(arena, out, "%.*s", StringExpand(string));
            }break;
        }
    }
}

internal C_TokenArray
C_Lex(M_Arena *arena, String8 string){
    STR_Hash *string_hash = APP_GetStringHash();
    
    C_TokenArray result = {0};
    result.vals = PushArray(arena, C_Token, 0);
    
    u8 *ptr = string.str;
    u8 *opl = string.str + string.size;
    for (;ptr < opl;){
        switch (*ptr){
            case 0:
            {
                ptr = opl;
            }break;
            
            case 1:
            {
                ptr += 1;
                u8 *first = ptr;
                for (;ptr < opl; ptr += 1){
                    if (*ptr == 1){
                        break;
                    }
                }
                C_Token *token = PushArrayZero(arena, C_Token, 1);
                token->kind = (C_TokenKind)GetFirstIntegerFromString(S8Range(first, ptr));
                ptr += 1;
            }break;
            
            case ' ':
            {
                C_Token *token = PushArrayZero(arena, C_Token, 1);
                token->kind = C_TokenKind_Space;
                ptr += 1;
            }break;
            
            case '\n':
            {
                C_Token *token = PushArrayZero(arena, C_Token, 1);
                token->kind = C_TokenKind_Newline;
                ptr += 1;
            }break;
            
            case '(':
            {
                C_Token *token = PushArrayZero(arena, C_Token, 1);
                token->kind = C_TokenKind_OpenParen;
                ptr += 1;
            }break;
            
            case ')':
            {
                C_Token *token = PushArrayZero(arena, C_Token, 1);
                token->kind = C_TokenKind_CloseParen;
                ptr += 1;
            }break;
            
            case '\'':
            {
                C_Token *token = PushArrayZero(arena, C_Token, 1);
                token->kind = C_TokenKind_Quote;
                ptr += 1;
            }break;
            
            case ';':
            {
                ptr += 1;
                u8 *first = ptr;
                for (;ptr < opl; ptr += 1){
                    if (*ptr == '\t'){
                        break;
                    }
                }
                
                C_Token *token = PushArrayZero(arena, C_Token, 1);
                token->kind = C_TokenKind_Comment;
                token->string = STR_Save(string_hash, S8Range(first, ptr));
                ptr += 1;
            }break;
            
            default:
            {
                u8 *first = ptr;
                ptr += 1;
                for (;ptr < opl; ptr += 1){
                    u8 c = *ptr;
                    if (c == 0 || c == 1 || c == ' ' || c == '\n' || c == '(' || c == ')' || c == '\'' || c == ';'){
                        break;
                    }
                }
                
                C_Token *token = PushArrayZero(arena, C_Token, 1);
                token->kind = C_TokenKind_Label;
                token->string = STR_Save(string_hash, S8Range(first, ptr));
            }break;
        }
    }
    
    result.count = (PushArray(arena, C_Token, 0) - result.vals);
    return(result);
}
