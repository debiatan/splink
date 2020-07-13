////////////////////////////////
// NOTE(allen): Buffer Structure

internal C_Token
E_MakeToken(C_TokenKind kind, STR_Index string){
    C_Token result = {kind, string};
    return(result);
}

internal E_TokenBuffer
E_InitTokenBuffer(void){
    E_TokenBuffer buffer = {0};
    buffer.arena = M_ArenaInitializeWithAlign(1);
    buffer.max = 1024;
    buffer.tokens = PushArray(&buffer.arena, C_Token, buffer.max);
    buffer.count = 0;
    return(buffer);
}

internal E_TokenBuffer
E_InitTextFieldTokenBuffer(C_Token *text_field_token){
    E_TokenBuffer buffer = {0};
    buffer.tokens = text_field_token;
    buffer.count = 0;
    buffer.max = 1;
    buffer.text_field_mode = 1;
    return(buffer);
}

internal void
E_ReleaseTokenBuffer(E_TokenBuffer *buffer){
    if (!buffer->text_field_mode){
        M_ArenaRelease(&buffer->arena);
    }
    MemoryZeroStruct(buffer);
}

internal void
E_TokenBufferNotifyChange(E_TokenBuffer *buffer){
    buffer->dirty = 1;
}

internal b32
E_TokenBufferHasChange(E_TokenBuffer *buffer){
    return(buffer->dirty);
}

internal void
E_TokenBufferChangeHandled(E_TokenBuffer *buffer){
    buffer->dirty = 0;
}

internal u64
E_TokenBufferGetCount(E_TokenBuffer *buffer){
    return(buffer->count);
}

internal C_Token*
E_TokenBufferReadRange(E_TokenBuffer *buffer, Rangeu range){
    C_Token *result = 0;
    if (range.min <= range.max && range.max <= buffer->count){
        result = buffer->tokens + range.min;
    }
    return(result);
}

internal b32
E_TokenBufferReplaceRange(E_TokenBuffer *buffer, Rangeu range, C_Token *new_tokens, u64 new_token_count){
    b32 result = 0;
    
    if (range.min <= range.max && range.max <= buffer->count){
        result = 1;
        
        i64 shift = new_token_count - (range.max - range.min);
        
        AssertImplies(shift < 0, (-shift) <= buffer->count);
        u64 new_count = buffer->count + shift;
        if (new_count > buffer->max){
            if (buffer->text_field_mode){
                result = 0;
            }
            else{
                PushArray(&buffer->arena, C_Token, new_count - buffer->max);
                buffer->max = new_count;
            }
        }
        
        if (result && buffer->text_field_mode){
            Assert(new_token_count <= 1);
            if (new_token_count == 1 && new_tokens[0].kind != C_TokenKind_Label){
                result = 0;
            }
        }
        
        if (result){
            u64 size_of_tail = buffer->count - range.max;
            if (size_of_tail > 0){
                u64 old_start_of_tail = range.max;
                u64 new_start_of_tail = old_start_of_tail + shift;
                MemoryCopy(buffer->tokens + new_start_of_tail,
                           buffer->tokens + old_start_of_tail,
                           sizeof(*buffer->tokens)*size_of_tail);
            }
            
            MemoryCopy(buffer->tokens + range.min,
                       new_tokens,
                       sizeof(*buffer->tokens)*new_token_count);
            
            buffer->count = new_count;
            
            E_TokenBufferNotifyChange(buffer);
        }
    }
    
    return(result);
}

internal b32
E_TokenBufferInsert(E_TokenBuffer *buffer, u64 pos, C_Token new_token){
    b32 result = E_TokenBufferReplaceRange(buffer, MakeRangeu(pos, pos), &new_token, 1);
    return(result);
}

internal b32
E_TokenBufferModify(E_TokenBuffer *buffer, u64 pos, C_Token new_token){
    b32 result = 0;
    if (pos < buffer->count){
        buffer->tokens[pos] = new_token;
        E_TokenBufferNotifyChange(buffer);
    }
    return(result);
}

internal b32
E_TokenBufferModifyString(E_TokenBuffer *buffer, u64 pos, String8 new_string){
    b32 result = 0;
    if (pos < buffer->count){
        STR_Hash *string_hash = APP_GetStringHash();
        buffer->tokens[pos].string = STR_Save(string_hash, new_string);
        E_TokenBufferNotifyChange(buffer);
    }
    return(result);
}

internal b32
E_TokenBufferDelete(E_TokenBuffer *buffer, u64 pos){
    b32 result = E_TokenBufferReplaceRange(buffer, MakeRangeu(pos, pos + 1), 0, 0);
    return(result);
}

internal void
E_TokenBufferScrub(E_TokenBuffer *buffer, u64 *cursor_pos, u64 cursor_count){
    M_Arena *scratch = OS_GetScratch();
    STR_Hash *string_hash = APP_GetStringHash();
    
    u64 shift = 0;
    u64 i = 0;
    C_Token *r = buffer->tokens;
    C_Token *w = buffer->tokens;
    C_Token *opl = r + buffer->count;
    b32 next_can_be_space = 1;
    b32 merge_back_label = 0;
    b32 merge_back_comment = 0;
    for (;r < opl; r += 1, i += 1){
        
        // NOTE(allen): Correct cursors
        if (shift != 0){
            for (u64 j = 0; j < cursor_count; j += 1){
                if (cursor_pos[j] == i){
                    cursor_pos[j] -= shift;
                }
            }
        }
        
        // NOTE(allen): Transfer logic
        b32 write = ((r->kind != C_TokenKind_Space) || next_can_be_space);
        if (write){
            b32 merge_back = (((r->kind == C_TokenKind_Label) && merge_back_label) ||
                              ((r->kind == C_TokenKind_Comment) && merge_back_comment));
            if (merge_back){
                C_Token *pw = w - 1;
                String8 pre = STR_Read(string_hash, pw->string);
                String8 post = STR_Read(string_hash, r->string);
                String8 str = PushStringCat(scratch, pre, post);
                pw->string = STR_Save(string_hash, str);
                shift += 1;
            }
            else{
                if (w < r){
                    MemoryCopyStruct(w, r);
                }
                w += 1;
            }
        }
        else{
            shift += 1;
        }
        
        switch (r->kind){
            default:
            {
                next_can_be_space = 1;
                merge_back_label = 0;
                merge_back_comment = 0;
            }break;
            
            case C_TokenKind_Comment:
            {
                next_can_be_space = 1;
                merge_back_label = 0;
                merge_back_comment = 1;
            }break;
            
            case C_TokenKind_Label:
            {
                next_can_be_space = 1;
                merge_back_label = 1;
                merge_back_comment = 0;
            }break;
            
            case C_TokenKind_Space:
            case C_TokenKind_Newline:
            case C_TokenKind_OpenParen:
            {
                next_can_be_space = 1;
                merge_back_label = 0;
                merge_back_comment = 0;
            }break;
        }
    }
    
    // NOTE(allen): Correct cursors
    if (shift != 0){
        for (u64 j = 0; j < cursor_count; j += 1){
            if (cursor_pos[j] == i){
                cursor_pos[j] -= shift;
            }
        }
    }
    
    buffer->count = w - buffer->tokens;
    
    OS_ReleaseScratch(scratch);
}

////////////////////////////////
// NOTE(allen): Layout

internal E_LayoutCtx
_E_InitLayoutCtx(M_Arena *out_arena, M_Arena *temp_arena, E_RuneLayout *layout){
    Assert(out_arena != temp_arena);
    E_LayoutCtx ctx = {0};
    ctx.arena = out_arena;
    ctx.temp_arena = temp_arena;
    ctx.layout = layout;
    ctx.layout->vals = PushArray(out_arena, E_Rune, 0);
    return(ctx);
}

internal void
_E_PushIndent(E_LayoutCtx *ctx, f32 pre, f32 post){
    E_LayoutIndent *indent = ctx->free_indent;
    if (indent != 0){
        SLLStackPop(ctx->free_indent);
    }
    else{
        indent = PushArray(ctx->temp_arena, E_LayoutIndent, 1);
    }
    SLLStackPush(ctx->indent, indent);
    indent->pre_indent = pre;
    indent->post_indent = post;
}

internal void
_E_PopIndent(E_LayoutCtx *ctx){
    if (ctx->indent != 0){
        E_LayoutIndent *indent = ctx->indent;
        SLLStackPop(ctx->indent);
        SLLStackPush(ctx->free_indent, indent);
    }
}

internal f32
_E_ReadIndent(E_LayoutCtx *ctx, b32 pre){
    f32 result = 0.f;
    if (ctx->indent != 0){
        if (pre){
            result = ctx->indent->pre_indent;
        }
        else{
            result = ctx->indent->post_indent;
        }
    }
    return(result);
}

internal E_Rune*
_E_PushRuneLineStart(E_LayoutCtx *ctx, b32 pre_indented){
    E_RuneLayout *layout = ctx->layout;
    if (layout->first_line != 0){
        ctx->p.y += ctx->space_dim.y;
    }
    
    E_Rune *result = PushArrayZero(ctx->arena, E_Rune, 1);
    result->cursor_pos = ctx->cursor_pos;
    f32 x = _E_ReadIndent(ctx, pre_indented) + ctx->initial_padding;
    f32 y = ctx->p.y;
    result->rect = MakeRect(0.f, y, x, y + ctx->space_dim.y);
    result->color = ctx->cl_whitespace;
    result->kind = E_RuneKind_LineStart;
    ctx->cursor_pos += 1;
    ctx->p.x = x;
    
    DLLPushBack(layout->first_line, layout->last_line, result);
    layout->dim.x = Max(layout->dim.x, result->rect.x1);
    layout->dim.y = Max(layout->dim.y, result->rect.y1);
    
    return(result);
}

internal E_Rune*
_E_PushRune(E_LayoutCtx *ctx, v4 color, String8 string, E_RuneKind kind){
    v2 dim = R_StringDimWithFont(ctx->font, ctx->text_scale, string);
    E_Rune *result = PushArrayZero(ctx->arena, E_Rune, 1);
    result->cursor_pos = ctx->cursor_pos;
    v2 p = ctx->p;
    result->string = string;
    result->rect = MakeRect(V2Expand(p), p.x + dim.x, p.y + ctx->space_dim.y);
    result->color = color;
    result->kind = kind;
    result->text_scale = ctx->text_scale;
    ctx->cursor_pos += 1;
    ctx->p.x += dim.x;
    
    E_RuneLayout *layout = ctx->layout;
    layout->dim.x = Max(layout->dim.x, result->rect.x1);
    layout->dim.y = Max(layout->dim.y, result->rect.y1);
    
    return(result);
}

internal E_Rune*
_E_PushRuneError(E_LayoutCtx *ctx){
    E_Rune *result = _E_PushRune(ctx, ctx->cl_error, S8Lit("ERROR"), E_RuneKind_Error);
    
    E_RuneLayout *layout = ctx->layout;
    DLLPushBack(layout->first_error, layout->last_error, result);
    
    return(result);
}

internal E_Rune*
_E_PushRuneSpace(E_LayoutCtx *ctx){
    E_Rune *result = _E_PushRune(ctx, ctx->cl_whitespace, S8Lit(" "), E_RuneKind_Space);
    return(result);
}

internal E_RuneLayout
E_LayoutTokenBuffer(M_Arena *arena, R_Font *font, E_TokenBuffer *buffer){
    STR_Hash *string_hash = APP_GetStringHash();
    
    M_Arena *scratch = OS_GetScratch1(arena);
    
    E_RuneLayout result = {0, 0};
    
    E_LayoutCtx ctx = _E_InitLayoutCtx(arena, scratch, &result);
    ctx.font = font;
    ctx.text_scale = 1.f;
    ctx.initial_padding = 4.f;
    ctx.space_dim = R_StringDimWithFont(font, ctx.text_scale, S8Lit(" "));
    ctx.cl_whitespace = v4(V3Expand(cl_white), 1.f);
    ctx.cl_error = v4(V3Expand(cl_error), 1.f);
    
    _E_PushRuneLineStart(&ctx, 0);
    
    u64 index = 0;
    C_Token *token = buffer->tokens;
    C_Token *opl = token + buffer->count;
    for (; token < opl; token += 1, index += 1){
        switch (token->kind){
            default:
            {
                _E_PushRuneError(&ctx);
            }break;
            
            case C_TokenKind_Space:
            {
                _E_PushRuneSpace(&ctx);
            }break;
            
            case C_TokenKind_Newline:
            {
                b32 pre_indented_line = 0;
                if (token + 1 < opl &&
                    (token + 1)->kind == C_TokenKind_CloseParen){
                    pre_indented_line = 1;
                }
                _E_PushRuneLineStart(&ctx, pre_indented_line);
            }break;
            
            case C_TokenKind_Comment:
            {
                String8 string = STR_Read(string_hash, token->string);
                _E_PushRune(&ctx, v4(V3Expand(cl_comment), 1.f), string, E_RuneKind_Comment);
            }break;
            
            case C_TokenKind_OpenParen:
            {
                f32 before = ctx.p.x;
                _E_PushRune(&ctx, v4(V3Expand(cl_text), 1.f), S8Lit("("), E_RuneKind_Symbol);
                _E_PushIndent(&ctx, before - ctx.initial_padding, ctx.p.x - ctx.initial_padding);
            }break;
            
            case C_TokenKind_CloseParen:
            {
                _E_PushRune(&ctx, v4(V3Expand(cl_text), 1.f), S8Lit(")"), E_RuneKind_Symbol);
                _E_PopIndent(&ctx);
            }break;
            
            case C_TokenKind_Quote:
            {
                _E_PushRune(&ctx, v4(V3Expand(cl_text), 1.f), S8Lit("'"), E_RuneKind_Symbol);
            }break;
            
            case C_TokenKind_Label:
            {
                v3 color = cl_text;
                STR_Index token_string = token->string;
                STR_Index *keyword = vars->keyword_table;
                for (u64 i = 0; i < ArrayCount(vars->keyword_table); i += 1){
                    if (keyword[i] == token_string){
                        color = cl_keyword;
                        break;
                    }
                }
                String8 string = STR_Read(string_hash, token_string);
                _E_PushRune(&ctx, v4(V3Expand(color), 1.f), string, E_RuneKind_Label);
            }break;
        }
    }
    
    result.count = PushArray(arena, E_Rune, 0) - result.vals;
    
    OS_ReleaseScratch(scratch);
    
    return(result);
}

////////////////////////////////
// NOTE(allen): Editor State

internal String8
E_LexemeOrDummy(E_EditorState *state, u64 pos){
    Assert(pos <= state->buffer->count);
    String8 result = S8Lit(" ");
    if (pos > 0){
        C_Token *token = state->buffer->tokens + pos - 1;
        if (token->kind == C_TokenKind_Comment ||
            token->kind == C_TokenKind_Label){
            result = STR_Read(APP_GetStringHash(), token->string);
        }
    }
    return(result);
}

#if 1
internal void
_E_AssertEditorInvariants(E_EditorState *state){
    E_Cursor *check[2];
    check[0] = &state->cursor;
    check[1] = &state->mark;
    for (u64 i = 0; i < ArrayCount(check); i += 1){
        Assert(check[i]->pos <= state->buffer->count);
        Assert(check[i]->sub_pos > 0);
        String8 lexeme = E_LexemeOrDummy(state, check[i]->pos);
        Assert(check[i]->sub_pos <= lexeme.size);
    }
}
#else
#define _E_AssertEditorInvariants(s)
#endif

internal E_Cursor
E_InitCursor(void){
    E_Cursor result = {0, 1};
    return(result);
}

internal b32
E_CursorEq(E_Cursor a, E_Cursor b){
    return((a.pos == b.pos) && (a.sub_pos == b.sub_pos));
}

internal void
E_CursorSetToEndOfToken(E_EditorState *state, E_Cursor *cursor){
    if (cursor->pos == 0){
        cursor->sub_pos = 1;
    }
    else{
        String8 lexeme = E_LexemeOrDummy(state, cursor->pos);
        cursor->sub_pos = lexeme.size;
    }
}

internal E_EditorState
E_InitEditorState(E_TokenBuffer *buffer, R_Font *font){
    E_EditorState result = {0};
    result.buffer = buffer;
    result.font = font;
    result.cursor = result.mark = E_InitCursor();
    if (buffer->text_field_mode){
        result.text_field_mode = E_EditorFieldMode_AnyString;
    }
    _E_AssertEditorInvariants(&result);
    return(result);
}

internal void
E_EditorSetFieldMode(E_EditorState *state, E_EditorFieldMode mode){
    if (state->buffer->text_field_mode){
        state->text_field_mode = mode;
        if (mode == E_EditorFieldMode_None){
            state->text_field_mode = E_EditorFieldMode_AnyString;
        }
    }
    else{
        state->text_field_mode = E_EditorFieldMode_None;
    }
}

internal E_Rune*
E_GetRuneForPosition(E_EditorState *state, u64 pos){
    E_Rune *result = 0;
    if (pos < state->runes.count){
        result = state->runes.vals + pos;
    }
    return(result);
}

internal E_Rune*
E_GetLineStartRune(E_EditorState *state, u64 p){
    E_Rune *node = state->runes.first_line->next;
    for (; node != 0; node = node->next){
        if (p < node->cursor_pos){
            node = node->prev;
            break;
        }
    }
    if (node == 0){
        node = state->runes.last_line;
    }
    return(node);
}

internal E_Rune*
E_GetNearestRuneOnLine(E_EditorState *state, E_Rune *rune, f32 x){
    Assert(rune->kind == E_RuneKind_LineStart);
    E_Rune *opl = rune->next;
    if (opl == 0){
        opl = state->runes.vals + state->runes.count;
    }
    f32 best_dist = Inf32();
    E_Rune *result = 0;
    for (;rune < opl; rune += 1){
        f32 dist = 0.f;
        if (rune->kind == E_RuneKind_Label ||
            rune->kind == E_RuneKind_Comment){
            if (x < rune->rect.x0){
                dist = rune->rect.x0 - x;
            }
            else if (x > rune->rect.x1){
                dist = x - rune->rect.x1;
            }
        }
        else{
            dist = AbsoluteValue(rune->rect.x1 - x);
        }
        if (dist < best_dist){
            result = rune;
            best_dist = dist;
        }
    }
    return(result);
}

internal E_Cursor
E_GetCursor(E_EditorState *state){
    return(state->cursor);
}

internal E_Cursor
E_GetMark(E_EditorState *state){
    return(state->mark);
}

internal b32
E_HasRange(E_EditorState *state){
    return(!E_CursorEq(state->cursor, state->mark));
}

internal E_Range
E_GetClosedRange(E_EditorState *state){
    E_Range range = {0};
    if (state->cursor.pos == state->mark.pos){
        range.kind = E_RangeKind_SingleTokenTextRange;
        range.token = state->cursor.pos;
        range.range = MakeRangeu(state->cursor.sub_pos, state->mark.sub_pos);
    }
    else{
        E_Cursor *min = 0;
        E_Cursor *max = 0;
        if (state->cursor.pos < state->mark.pos){
            min = &state->cursor;
            max = &state->mark;
        }
        else{
            min = &state->mark;
            max = &state->cursor;
        }
        
        String8 min_lexeme = E_LexemeOrDummy(state, min->pos);
        b32 min_is_right_side = (min->sub_pos >= min_lexeme.size);
        
        String8 max_lexeme = E_LexemeOrDummy(state, max->pos);
        b32 max_is_right_side = (max->sub_pos >= max_lexeme.size);
        
        if (min->pos + 1 == max->pos && min_is_right_side && !max_is_right_side){
            range.kind = E_RangeKind_SingleTokenTextRange;
            range.token = max->pos;
            range.range = MakeRangeu(0, max->sub_pos);
        }
        else{
            range.kind = E_RangeKind_MultiTokenRange;
            range.range.min = min->pos;
            if (range.range.min > 0 && !min_is_right_side){
                range.range.min -= 1;
            }
            range.range.max = max->pos;
        }
    }
    return(range);
}

internal b32
E_HasRangeWithTokenGranularity(E_EditorState *state){
    E_Range range = E_GetClosedRange(state);
    return(range.kind == E_RangeKind_MultiTokenRange);
}

internal E_Rune*
E_GetRuneAtXY(E_EditorState *state, v2 p){
    E_Rune *result = 0;
    E_RuneLayout *layout = &state->runes;
    for (E_Rune *node = layout->first_line;
         node != 0;
         node = node->next){
        if (node == layout->last_line){
            result = E_GetNearestRuneOnLine(state, node, p.x);
            break;
        }
        Range y_range = RectGetRange(node->rect, Dimension_Y);
        if (RangeContains(y_range, p.y)){
            result = E_GetNearestRuneOnLine(state, node, p.x);
            break;
        }
    }
    return(result);
}

internal u64
E_GetNearestSubPosOnRune(E_EditorState *state, E_Rune *rune, f32 x){
    u64 pos = 0;
    switch (rune->kind){
        default:
        {
            pos = 1;
        }break;
        
        case E_RuneKind_Label:
        case E_RuneKind_Comment:
        {
            R_Font *font = state->font;
            String8 string = rune->string;
            f32 text_scale = rune->text_scale;
            f32 best_dist = Inf32();
            f32 x0 = rune->rect.x0;
            for (u64 i = 0; i < string.size; i += 1){
                v2 dim = R_StringDimWithFont(font, text_scale, S8(&string.str[i], 1));
                x0 += dim.x;
                f32 dist = AbsoluteValue(x0 - x);
                if (dist < best_dist){
                    best_dist = dist;
                    pos = i + 1;
                }
            }
        }break;
    }
    
    pos = ClampBot(1, pos);
    return(pos);
}

internal f32
E_GetCursorX(E_EditorState *state){
    f32 x = 0.f;
    E_Rune *rune = E_GetRuneForPosition(state, state->cursor.pos);
    if (rune != 0){
        switch (rune->kind){
            default:
            {
                String8 string = StringPrefix(rune->string, state->cursor.sub_pos);
                v2 dim = R_StringDimWithFont(state->font, rune->text_scale, string);
                x = rune->rect.x0 + dim.x;
            }break;
            
            case E_RuneKind_Error:
            case E_RuneKind_Space:
            case E_RuneKind_LineStart:
            {
                x = rune->rect.x1;
            }break;
        }
    }
    return(x);
}

internal b32
E_OnTokenKind(E_EditorState *state, C_TokenKind kind){
    b32 result = 0;
    if (state->cursor.pos > 0){
        C_Token *token = state->buffer->tokens + state->cursor.pos - 1;
        if (token->kind == kind){
            result = 1;
        }
    }
    return(result);
}

internal void
E_EditorSetMark(E_EditorState *state, E_Cursor mark){
    u64 token_count = state->buffer->count;
    mark.pos = ClampTop(mark.pos, token_count);
    u64 sub_pos = mark.sub_pos;
    E_CursorSetToEndOfToken(state, &mark);
    state->mark.pos = mark.pos;
    state->mark.sub_pos = Clamp(1, sub_pos, mark.sub_pos);
}

internal void
E_EditorUpdatePreferredXToCursor(E_EditorState *state){
    state->preferred_x = E_GetCursorX(state);
}

internal void
E_EditorInsertCharacter(E_EditorState *state, u64 character){
    b32 pass = 0;
    switch (state->text_field_mode){
        case E_EditorFieldMode_None:
        case E_EditorFieldMode_AnyString:
        {
            pass = 1;
        }break;
        case E_EditorFieldMode_CodeIdentifier:
        {
            if (character != ' ' && character != ';' &&
                character != '(' && character != ')' &&
                character != '\''){
                pass = 1;
            }
        }break;
    }
    if (!pass){
        return;
    }
    
    M_Arena *scratch = OS_GetScratch();
    STR_Hash *string_hash = APP_GetStringHash();
    E_TokenBuffer *buffer = state->buffer;
    
    b32 modify_prev_token = 0;
    if (state->cursor.pos > 0){
        C_Token *current_token = buffer->tokens + state->cursor.pos - 1;
        if (current_token != 0 &&
            (current_token->kind == C_TokenKind_Comment ||
             current_token->kind == C_TokenKind_Label)){
            String8 string = STR_Read(string_hash, current_token->string);
            String8 pre_str = StringPrefix(string, state->cursor.sub_pos);
            String8 post_str = StringSkip(string, state->cursor.sub_pos);
            String8 new_string = PushStringF(scratch, "%.*s%c%.*s", StringExpand(pre_str), (u8)character, StringExpand(post_str));
            C_Token new_token = E_MakeToken(current_token->kind, STR_Save(string_hash, new_string));
            E_TokenBufferModify(buffer, state->cursor.pos - 1, new_token);
            modify_prev_token = 1;
            state->cursor.sub_pos += 1;
        }
    }
    
    b32 modify_next_token = 0;
    if (state->cursor.pos < buffer->count){
        String8 lexeme = E_LexemeOrDummy(state, state->cursor.pos);
        if (state->cursor.sub_pos == lexeme.size){
            C_Token *current_token = buffer->tokens + state->cursor.pos;
            if (current_token != 0 &&
                (current_token->kind == C_TokenKind_Comment ||
                 current_token->kind == C_TokenKind_Label)){
                String8 string = STR_Read(string_hash, current_token->string);
                String8 new_string = PushStringF(scratch, "%c%.*s", (u8)character, StringExpand(string));
                C_Token new_token = E_MakeToken(current_token->kind, STR_Save(string_hash, new_string));
                E_TokenBufferModify(buffer, state->cursor.pos, new_token);
                modify_prev_token = 1;
                state->cursor.pos += 1;
                state->cursor.sub_pos = 1;
            }
        }
    }
    
    if (!modify_prev_token && !modify_next_token){
        String8 new_string = PushStringF(scratch, "%c", (u8)character);
        C_Token new_token = E_MakeToken(C_TokenKind_Label, STR_Save(string_hash, new_string));
        E_TokenBufferInsert(buffer, state->cursor.pos, new_token);
        state->cursor.pos += 1;
        state->cursor.sub_pos = 1;
    }
    
    state->mark = state->cursor;
    OS_ReleaseScratch(scratch);
    
    _E_AssertEditorInvariants(state);
}

internal b32
E_EditorInsertTokenString(E_EditorState *state, C_TokenKind kind, String8 token_string){
    b32 result = 0;
    
    STR_Hash *string_hash = APP_GetStringHash();
    
    String8 lexeme = E_LexemeOrDummy(state, state->cursor.pos);
    u64 sub_pos = state->cursor.sub_pos;
    if (sub_pos < lexeme.size){
        C_Token *token = state->buffer->tokens + state->cursor.pos - 1;
        String8 string = STR_Read(string_hash, token->string);
        
        C_Token split[3];
        split[0].kind   = token->kind;
        split[0].string = STR_Save(string_hash, StringPrefix(string, sub_pos));
        split[1].kind   = kind;
        split[1].string = STR_Save(string_hash, token_string);
        split[2].kind   = token->kind;
        split[2].string = STR_Save(string_hash, StringSkip(string, sub_pos));
        
        result = E_TokenBufferReplaceRange(state->buffer, MakeRangeu(state->cursor.pos - 1, state->cursor.pos), split, ArrayCount(split));
    }
    else{
        STR_Index string_indx = 0;
        if (token_string.size > 0){
            string_indx = STR_Save(string_hash, token_string);
        }
        result = E_TokenBufferInsert(state->buffer, state->cursor.pos, E_MakeToken(kind, string_indx));
    }
    
    state->cursor.pos += 1;
    state->cursor.sub_pos = 1;
    state->mark = state->cursor;
    
    _E_AssertEditorInvariants(state);
    return(result);
}

internal void
E_EditorInsertSpace(E_EditorState *state){
    b32 insert_into_comment = 0;
    if (state->cursor.pos > 0){
        E_TokenBuffer *buffer = state->buffer;
        C_Token *current_token = buffer->tokens + state->cursor.pos - 1;
        if (current_token->kind == C_TokenKind_Comment){
            E_EditorInsertCharacter(state, ' ');
            insert_into_comment = 1;
        }
    }
    
    if (!insert_into_comment){
        E_EditorInsertTokenString(state, C_TokenKind_Space, S8Zero());
    }
}

internal void
E_EditorInsertComment(E_EditorState *state){
    b32 insert_into_comment = 0;
    if (state->cursor.pos > 0){
        E_TokenBuffer *buffer = state->buffer;
        C_Token *current_token = buffer->tokens + state->cursor.pos - 1;
        if (current_token->kind == C_TokenKind_Comment){
            E_EditorInsertCharacter(state, ';');
            insert_into_comment = 1;
        }
    }
    
    if (!insert_into_comment){
        E_EditorInsertTokenString(state, C_TokenKind_Comment, S8Lit(";"));
    }
}

internal void
E_EditorDelete(E_EditorState *state, Side side){
    if (side == Side_Min && state->cursor.pos > 0){
        E_TokenBufferDelete(state->buffer, state->cursor.pos - 1);
        state->cursor.pos -= 1;
        E_CursorSetToEndOfToken(state, &state->cursor);
    }
    else if (side == Side_Max && state->cursor.pos < state->buffer->count){
        E_TokenBufferDelete(state->buffer, state->cursor.pos);
    }
    state->mark = state->cursor;
    
    _E_AssertEditorInvariants(state);
}

internal void
E_EditorDeleteSmall(E_EditorState *state, Side side){
    M_Arena *scratch = OS_GetScratch();
    STR_Hash *string_hash = APP_GetStringHash();
    E_TokenBuffer *buffer = state->buffer;
    
    C_Token *current_token = 0;
    String8 new_string = {0};
    if (side == Side_Min && state->cursor.pos > 0){
        current_token = buffer->tokens + state->cursor.pos - 1;
        if (current_token->kind == C_TokenKind_Label ||
            current_token->kind == C_TokenKind_Comment){
            String8 string = STR_Read(string_hash, current_token->string);
            if (string.size > 0){
                String8 pre_str = StringPrefix(string, state->cursor.sub_pos);
                String8 post_str = StringSkip(string, state->cursor.sub_pos);
                pre_str = StringChop(pre_str, 1);
                new_string = PushStringF(scratch, "%.*s%.*s", StringExpand(pre_str), StringExpand(post_str));
                state->cursor.sub_pos -= 1;
            }
        }
    }
    else if (side == Side_Max && state->cursor.pos <= buffer->count){
        b32 delete_in_current_token = 0;
        if (state->cursor.pos > 0){
            current_token = buffer->tokens + state->cursor.pos - 1;
            if (current_token->kind == C_TokenKind_Label ||
                current_token->kind == C_TokenKind_Comment){
                String8 string = STR_Read(string_hash, current_token->string);
                if (state->cursor.sub_pos < string.size){
                    String8 pre_str = StringPrefix(string, state->cursor.sub_pos);
                    String8 post_str = StringSkip(string, state->cursor.sub_pos);
                    post_str = StringSkip(post_str, 1);
                    new_string = PushStringF(scratch, "%.*s%.*s", StringExpand(pre_str), StringExpand(post_str));
                    delete_in_current_token = 1;
                }
            }
        }
        if (!delete_in_current_token){
            current_token = buffer->tokens + state->cursor.pos;
            if (current_token->kind == C_TokenKind_Label ||
                current_token->kind == C_TokenKind_Comment){
                new_string = STR_Read(string_hash, current_token->string);
                if (new_string.size > 0){
                    new_string = StringPostfix(new_string, new_string.size - 1);
                }
            }
        }
    }
    
    if (new_string.size != 0){
        STR_Index new_string_indx = STR_Save(string_hash, new_string);
        current_token->string = new_string_indx;
        E_TokenBufferNotifyChange(buffer);
    }
    else{
        E_EditorDelete(state, side);
    }
    
    if (state->cursor.sub_pos == 0){
        if (state->cursor.pos > 0){
            state->cursor.pos -= 1;
            E_CursorSetToEndOfToken(state, &state->cursor);
        }
        else{
            state->cursor.pos = 1;
        }
    }
    state->mark = state->cursor;
    
    OS_ReleaseScratch(scratch);
    
    _E_AssertEditorInvariants(state);
}

internal void
E_EditorDeleteRange(E_EditorState *state){
    E_Range range = E_GetClosedRange(state);
    switch (range.kind){
        case E_RangeKind_SingleTokenTextRange:
        {
            if (range.token > 0 && RangeSize(range.range) > 0){
                M_Arena *scratch = OS_GetScratch();
                STR_Hash *string_hash = APP_GetStringHash();
                
                E_TokenBuffer *buffer = state->buffer;
                C_Token *current_token = buffer->tokens + range.token - 1;
                String8 string = STR_Read(string_hash, current_token->string);
                String8 pre = StringPrefix(string, range.range.min);
                String8 post = StringSkip(string, range.range.max);
                String8 new_string = PushStringCat(scratch, pre, post);
                STR_Index new_string_indx = STR_Save(string_hash, new_string);
                current_token->string = new_string_indx;
                E_TokenBufferNotifyChange(buffer);
                
                OS_ReleaseScratch(scratch);
                
                state->cursor.pos = range.token;
                state->cursor.sub_pos = range.range.min;
                if (state->cursor.sub_pos == 0){
                    if (state->cursor.pos > 0){
                        state->cursor.pos -= 1;
                    }
                    E_CursorSetToEndOfToken(state, &state->cursor);
                }
                state->mark = state->cursor;
            }
        }break;
        
        case E_RangeKind_MultiTokenRange:
        {
            E_TokenBufferReplaceRange(state->buffer, range.range, 0, 0);
            
            state->cursor.pos = range.range.min;
            E_CursorSetToEndOfToken(state, &state->cursor);
            state->mark = state->cursor;
        }break;
    }
    
    _E_AssertEditorInvariants(state);
}

internal b32
_E_EditorMove(E_EditorState *state, Side side){
    b32 result = 0;
    if (side == Side_Min && state->cursor.pos > 0){
        state->cursor.pos -= 1;
        E_CursorSetToEndOfToken(state, &state->cursor);
        result = 1;
    }
    else if (side == Side_Max && state->cursor.pos < state->buffer->count){
        state->cursor.pos += 1;
        state->cursor.sub_pos = 1;
        result = 1;
    }
    
    _E_AssertEditorInvariants(state);
    return(result);
}

internal b32
E_EditorMoveSmall(E_EditorState *state, Side side){
    E_Cursor original = state->cursor;
    
    if (side == Side_Min){
        if (state->cursor.sub_pos > 1){
            state->cursor.sub_pos -= 1;
        }
        else{
            _E_EditorMove(state, Side_Min);
        }
    }
    else if (side == Side_Max && state->cursor.pos <= state->buffer->count){
        b32 small_step = 0;
        if (state->cursor.pos > 0){
            C_Token *token = state->buffer->tokens + state->cursor.pos - 1;
            String8 string = STR_Read(APP_GetStringHash(), token->string);
            if (state->cursor.sub_pos < string.size){
                state->cursor.sub_pos += 1;
                small_step = 1;
            }
        }
        if (!small_step){
            _E_EditorMove(state, Side_Max);
        }
    }
    
    state->mark = state->cursor;
    E_EditorUpdatePreferredXToCursor(state);
    
    _E_AssertEditorInvariants(state);
    
    return(!E_CursorEq(original, state->cursor));
}

internal b32
E_EditorMove(E_EditorState *state, Side side){
    E_Cursor original = state->cursor;
    
    _E_EditorMove(state, side);
    E_CursorSetToEndOfToken(state, &state->cursor);
    
    state->mark = state->cursor;
    E_EditorUpdatePreferredXToCursor(state);
    
    return(!E_CursorEq(original, state->cursor));
}

internal b32
E_EditorMoveSkipWhitespace(E_EditorState *state, Side side){
    E_Cursor original = state->cursor;
    
    for (;;){
        if (!_E_EditorMove(state, side)){
            break;
        }
        E_Rune *rune = E_GetRuneForPosition(state, state->cursor.pos);
        if (!(rune->kind == E_RuneKind_Space ||
              rune->kind == E_RuneKind_LineStart)){
            break;
        }
    }
    
    state->mark = state->cursor;
    E_EditorUpdatePreferredXToCursor(state);
    
    _E_AssertEditorInvariants(state);
    
    return(!E_CursorEq(original, state->cursor));
}

internal b32
E_EditorMoveWholeLine(E_EditorState *state, Side side){
    E_Cursor original = state->cursor;
    
    E_Rune *line = E_GetLineStartRune(state, state->cursor.pos);
    if (side == Side_Min){
        state->cursor.pos = line->cursor_pos;
        state->cursor.sub_pos = 1;
    }
    else{
        if (line->next != 0){
            state->cursor.pos = line->next->cursor_pos - 1;
        }
        else{
            state->cursor.pos = state->runes.count - 1;
        }
        E_CursorSetToEndOfToken(state, &state->cursor);
    }
    
    state->mark = state->cursor;
    E_EditorUpdatePreferredXToCursor(state);
    
    _E_AssertEditorInvariants(state);
    
    return(!E_CursorEq(original, state->cursor));
}

internal b32
E_EditorMoveVertical(E_EditorState *state, Side side){
    E_Cursor original = state->cursor;
    
    E_Rune *nearest = 0;
    E_Rune *line = E_GetLineStartRune(state, state->cursor.pos);
    if (side == Side_Min && line->prev != 0){
        nearest = E_GetNearestRuneOnLine(state, line->prev, state->preferred_x);
    }
    else if (side == Side_Max && line->next != 0){
        nearest = E_GetNearestRuneOnLine(state, line->next, state->preferred_x);
    }
    if (nearest != 0){
        state->cursor.pos = nearest->cursor_pos;
        state->cursor.sub_pos = E_GetNearestSubPosOnRune(state, nearest, state->preferred_x);
    }
    
    state->mark = state->cursor;
    
    _E_AssertEditorInvariants(state);
    
    return(!E_CursorEq(original, state->cursor));
}

internal b32
E_EditorMoveToXY(E_EditorState *state, v2 p){
    b32 result = 0;
    E_Rune *rune = E_GetRuneAtXY(state, p);
    if (rune != 0){
        result = 1;
        state->cursor.pos = rune->cursor_pos;
        state->cursor.sub_pos = E_GetNearestSubPosOnRune(state, rune, p.x);
    }
    state->preferred_x = p.x;
    
    state->mark = state->cursor;
    
    _E_AssertEditorInvariants(state);
    
    return(result);
}

internal void
E_EditorEnterFrom(E_EditorState *state, Dimension dimension, Side side, f32 v){
    if (dimension == Dimension_Y){
        E_Rune *line = 0;
        if (side == Side_Min){
            line = state->runes.first_line;
        }
        else if (side == Side_Max){
            line = state->runes.last_line;
        }
        if (line != 0){
            E_Rune *nearest = E_GetNearestRuneOnLine(state, line, v);
            if (nearest != 0){
                state->cursor.pos = nearest->cursor_pos;
                state->cursor.sub_pos = E_GetNearestSubPosOnRune(state, nearest, v);
            }
        }
        
        state->mark = state->cursor;
        state->preferred_x = v;
    }
    else{
        NotImplemented;
    }
    
    _E_AssertEditorInvariants(state);
}

internal void
E_EditorUpdateLayout(E_EditorState *state, R_Font *font){
    state->font = font;
    state->runes = E_LayoutTokenBuffer(APP_GetFrameArena(), font, state->buffer);
    _E_AssertEditorInvariants(state);
}

////////////////////////////////
// NOTE(allen): Input

internal b32
E_EditorHandleCharacterEvent(E_EditorState *state, OS_Event *event){
    b32 result = 0;
    
    if (E_HasRange(state)){
        E_EditorDeleteRange(state);
    }
    
    u64 character = event->character;
    if (state->text_field_mode != E_EditorFieldMode_None){
        E_EditorInsertCharacter(state, character);
    }
    else{
        switch (character){
            case ' ':
            {
                E_EditorInsertSpace(state);
            }break;
            case ';':
            {
                E_EditorInsertComment(state);
            }break;
            case '(':
            {
                E_EditorInsertTokenString(state, C_TokenKind_OpenParen, S8Zero());
            }break;
            case ')':
            {
                E_EditorInsertTokenString(state, C_TokenKind_CloseParen, S8Zero());
            }break;
            case '\'':
            {
                E_EditorInsertTokenString(state, C_TokenKind_Quote, S8Zero());
            }break;
            default:
            {
                E_EditorInsertCharacter(state, character);
            }break;
        }
    }
    
    OS_EatEvent(event);
    result = 1;
    
    return(result);
}

internal b32
E_EditorHandleKeyPressEvent(E_EditorState *state, OS_Event *event){
    i32 move_size = 0;
    if ((event->modifiers & KeyModifier_Ctrl) != 0){
        move_size = 1;
    }
    if (E_HasRangeWithTokenGranularity(state)){
        move_size = 1;
    }
    
    b32 selection = 0;
    if ((event->modifiers & KeyModifier_Shift) != 0){
        selection = 1;
    }
    
    b32 used_event = 0;
    
    switch (event->key){
        case Key_Enter:
        {
            if (state->text_field_mode == E_EditorFieldMode_None){
                used_event = E_EditorInsertTokenString(state, C_TokenKind_Newline, S8Zero());
            }
        }break;
        
        case Key_Backspace: case Key_Delete:
        {
            if (E_HasRange(state)){
                E_EditorDeleteRange(state);
            }
            else{
                Side side = Side_Min;
                if (event->key == Key_Delete){
                    side = Side_Max;
                }
                
                if (event->modifiers & KeyModifier_Alt){
                    E_EditorDelete(state, side);
                }
                else if (move_size == 0){
                    E_EditorDeleteSmall(state, side);
                }
                else if (move_size == 1){
                    E_Cursor mark = E_GetMark(state);
                    E_Rune *rune = E_GetRuneForPosition(state, state->cursor.pos);
                    if (!(rune->kind == E_RuneKind_Space ||
                          rune->kind == E_RuneKind_LineStart)){
                        if (side == Side_Max){
                            E_CursorSetToEndOfToken(state, &state->cursor);
                        }
                        else{
                            E_EditorMove(state, side);
                        }
                    }
                    else{
                        E_EditorMoveSkipWhitespace(state, side);
                    }
                    E_EditorSetMark(state, mark);
                    E_EditorDeleteRange(state);
                }
                
                used_event = 1;
            }
        }break;
        
        case Key_Left: case Key_Right:
        {
            Side side = Side_Min;
            if (event->key == Key_Right){
                side = Side_Max;
            }
            
            E_Cursor mark = E_GetMark(state);
            
            if (move_size == 0){
                used_event = E_EditorMoveSmall(state, side);
            }
            else if (move_size == 1){
                used_event = E_EditorMove(state, side);
            }
            
            if (selection){
                E_EditorSetMark(state, mark);
            }
        }break;
        
        case Key_Home: case Key_End:
        {
            Side side = Side_Min;
            if (event->key == Key_End){
                side = Side_Max;
            }
            
            E_Cursor mark = E_GetMark(state);
            
            used_event = E_EditorMoveWholeLine(state, side);
            
            if (selection){
                E_EditorSetMark(state, mark);
            }
        }break;
        
        case Key_Up: case Key_Down:
        {
            Side side = Side_Min;
            if (event->key == Key_Down){
                side = Side_Max;
            }
            
            E_Cursor mark = E_GetMark(state);
            
            used_event = E_EditorMoveVertical(state, side);
            
            if (selection){
                E_EditorSetMark(state, mark);
            }
        }break;
    }
    
    return(used_event);
}

internal b32
E_EditorHandleMousePressEvent(E_EditorState *state, OS_Event *event, v2 editor_to_screen){
    b32 result = 0;
    
    b32 selection = 0;
    if ((event->modifiers & KeyModifier_Shift) != 0){
        selection = 1;
    }
    
    v2 p_screen = event->position;
    v2 p_editor = V2Sub(p_screen, editor_to_screen);
    
    E_Cursor mark = E_GetMark(state);
    E_EditorMoveToXY(state, p_editor);
    if (selection){
        E_EditorSetMark(state, mark);
    }
    
    OS_EatEvent(event);
    result = 1;
    
    return(result);
}


////////////////////////////////
// NOTE(allen): Render

internal void
E_EditorRender(E_EditorState *state, R_Font *font, v2 base_p){
#define EDITOR_DEBUG_VISOR 0
#if EDITOR_DEBUG_VISOR
    // NOTE(allen): Debug visor for editor
    {
        R_Rect(MakeRect(base_p.x + state->preferred_x, 0.f, base_p.x + state->preferred_x + 1.f, 10000.f), cl_awful, 1.f);
    }
#endif
    
    b32 show_cursor = (APP_GetActiveEditor() == state);
    b32 show_range = (show_cursor && E_HasRange(state));
    E_Range range = E_GetClosedRange(state); 
    
    R_SelectFont(font);
    u64 rune_count = state->runes.count;
    E_Rune *rune = state->runes.vals;
    for (u64 i = 0; i < rune_count; i += 1, rune += 1){
        v2 p0 = V2Add(rune->rect.p0, base_p);
        v2 p1 = V2Add(rune->rect.p1, base_p);
        
        v3 color = v3(V3Expand(rune->color));
        f32 a = rune->color.a;
        
        if (show_range){
            f32 y_highlight_top = Lerp(p0.y, 0.775f, p1.y);
            Range y_range = MakeRange(y_highlight_top, p1.y);
            
            switch (range.kind)
            {
                case E_RangeKind_SingleTokenTextRange:
                {
                    if (rune->cursor_pos == range.token){
                        Range x_range;
                        v2 dim = R_StringDim(rune->text_scale, StringPrefix(rune->string, range.range.min));
                        x_range.min = p0.x + dim.x;
                        dim = R_StringDim(rune->text_scale, StringSubstring(rune->string, range.range));
                        x_range.max = x_range.min + dim.x;
                        
                        R_Rect(MakeRectRanges(x_range, y_range), color, a*0.5f);
                    }
                }break;
                
                case E_RangeKind_MultiTokenRange:
                {
                    if (RangeuContains(range.range, rune->cursor_pos - 1)){
                        Range x_range = MakeRange(p0.x, p1.x);
                        
                        R_Rect(MakeRectRanges(x_range, y_range), color, a*0.5f);
                    }
                }break;
            }
        }
        
        R_String(p0, rune->text_scale, rune->string, color, a);
        
        if (show_cursor){
            if (rune->cursor_pos == state->cursor.pos){
                switch (rune->kind){
                    default:
                    {
                        R_Rect(MakeRect(p1.x, p0.y, p1.x + 1.f, p1.y), color, a);
                        if (!show_range){
                            R_Rect(MakeRectVec(p0, p1), color, a*0.2f);
                        }
                    }break;
                    
                    case E_RuneKind_Label:
                    case E_RuneKind_Comment:
                    {
                        if (state->cursor.sub_pos >= rune->string.size){
                            R_Rect(MakeRect(p1.x, p0.y, p1.x + 1.f, p1.y), color, a);
                        }
                        else{
                            String8 pre_string = StringPrefix(rune->string, state->cursor.sub_pos);
                            f32 x = p0.x + R_StringDim(rune->text_scale, pre_string).x;
                            R_Rect(MakeRect(x, p0.y, x + 1.f, p1.y), color, a);
                        }
                        if (!show_range){
                            R_Rect(MakeRectVec(p0, p1), color, a*0.2f);
                        }
                    }break;
                    
                    case E_RuneKind_Space:
                    {
                        R_Rect(MakeRect(p1.x, p0.y, p1.x + 1.f, p1.y), color, a);
                    }break;
                }
            }
        }
        
#if EDITOR_DEBUG_VISOR
        {
            if (rune->cursor_pos == state->mark.pos){
                switch (rune->kind){
                    default:
                    {
                        R_Rect(MakeRect(p1.x, p0.y, p1.x + 1.f, p1.y), cl_green, 1.f);
                    }break;
                    
                    case E_RuneKind_Label:
                    case E_RuneKind_Comment:
                    {
                        if (state->mark.sub_pos >= rune->string.size){
                            R_Rect(MakeRect(p1.x, p0.y, p1.x + 1.f, p1.y), cl_green, 1.f);
                        }
                        else{
                            String8 pre_string = StringPrefix(rune->string, state->mark.sub_pos);
                            f32 x = p0.x + R_StringDim(rune->text_scale, pre_string).x;
                            R_Rect(MakeRect(x, p0.y, x + 1.f, p1.y), cl_green, a);
                        }
                    }break;
                }
            }
        }
#endif
    }
}

////////////////////////////////
// NOTE(allen): Test

internal void
_E_TokenBuffer_Test(void){
    E_TokenBuffer buffer = E_InitTokenBuffer();
    C_Token t1[] = { {1, 2}, {2, 4}, {3, 6}, };
    C_Token t2[] = { {100, 2}, {200, 8}, {300, 32}, {400, 64}, {800, 128}, };
    for (u64 i = 0; i < ArrayCount(t1); i += 1){
        u64 end = E_TokenBufferGetCount(&buffer);
        E_TokenBufferInsert(&buffer, end, t1[i]);
    }
    for (u64 i = 0; i < ArrayCount(t2); i += 1){
        u64 end = E_TokenBufferGetCount(&buffer);
        E_TokenBufferInsert(&buffer, end, t2[i]);
    }
    C_Token *r1 = E_TokenBufferReadRange(&buffer, MakeRangeu(0, 3));
    Assert(MemoryMatch(r1, t1, sizeof(t1)));
    C_Token *r2 = E_TokenBufferReadRange(&buffer, MakeRangeu(3, 8));
    Assert(MemoryMatch(r2, t2, sizeof(t2)));
    
    for (u64 i = 0; i < ArrayCount(t1); i += 1){
        E_TokenBufferDelete(&buffer, 0);
    }
    r2 = E_TokenBufferReadRange(&buffer, MakeRangeu(0, 5));
    Assert(MemoryMatch(r2, t2, sizeof(t2)));
    
    for (u64 i = 0; i < ArrayCount(t1); i += 1){
        E_TokenBufferModify(&buffer, i, t1[i]);
    }
    r1 = E_TokenBufferReadRange(&buffer, MakeRangeu(0, 3));
    Assert(MemoryMatch(r1, t1, sizeof(t1)));
    
    r2 = E_TokenBufferReadRange(&buffer, MakeRangeu(3, 5));
    Assert(MemoryMatch(r2, t2 + 3, 2*sizeof(*r2)));
}
