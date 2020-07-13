////////////////////////////////
// NOTE(allen): Shhh compiler be quite

internal STR_Index E_DefinitionNameIndex(E_Definition *definition);
internal String8   E_DefinitionName(E_Definition *definition);
internal E_Definition* _E_CreateNewDefinitionWithContents(E_Space *space, E_DefinitionFlags flags,
                                                          STR_Index name, C_TokenArray body, C_Id id);
internal void _E_DeleteDefinition(E_Definition *definition);

////////////////////////////////
// NOTE(allen): Space

internal String8
E_SpaceName(E_Space *space){
    STR_Hash *string_hash = APP_GetStringHash();
    String8 path = STR_Read(string_hash, space->save_path);
    String8 file_name = STR_NameFromPath(path);
    String8 name = STR_NameWithoutExtension(file_name);
    return(name);
}

internal STR_Index
E_SpaceNameIndex(E_Space *space){
    STR_Hash *string_hash = APP_GetStringHash();
    String8 path = STR_Read(string_hash, space->save_path);
    String8 name = STR_NameFromPath(path);
    STR_Index name_index = STR_Save(string_hash, name);
    return(name_index);
}

internal E_Space*
E_NewSpace(void){
    E_Space *result = vars->free_space;
    if (result != 0){
        SLLStackPop(vars->free_space);
    }
    else{
        result = PushArray(vars->arena, E_Space, 1);
    }
    MemoryZeroStruct(result);
    
    DLLPushBack_NP(vars->first_space_ordered, vars->last_space_ordered, result, ordered_next, ordered_prev);
    DLLPushBack(vars->first_invalid_space, vars->last_invalid_space, result);
    result->invalid = 1;
    
    result->id_counter = 1;
    
    return(result);
}

internal void
_E_SpaceFreeAllDefinitions(E_Space *space){
    for (E_Definition *node = space->first_definition, *next = 0;
         node != 0;
         node = next){
        next = node->next;
        _E_DeleteDefinition(node);
    }
}

internal void
E_DeleteSpace(E_Space *space){
    _E_SpaceFreeAllDefinitions(space);
    DLLRemove_NP(vars->first_space_ordered, vars->last_space_ordered, space, ordered_next, ordered_prev);
    if (space->invalid){
        DLLRemove(vars->first_invalid_space, vars->last_invalid_space, space);
    }
    else{
        STR_Index name = E_SpaceNameIndex(space);
        APP_SignalSpaceNameAvailable(name);
        DLLRemove(vars->first_space, vars->last_space, space);
    }
    SLLStackPush(vars->free_space, space);
}

// TODO(allen): vvvv names

internal C_Cell*
E_GetGlobalEnvironment(E_Space *space){
    return(space->defines_env);
}

internal C_Cell*
E_GetGlobalIdEnvironment(E_Space *space){
    return(space->id_env);
}

internal C_Id
E_GetUniqueCompId(E_Space *space){
    C_Id result = space->id_counter;
    space->id_counter += 1;
    return(result);
}

internal void
E_SignalIdentifierAvailable(E_Space *space, STR_Index index){
    if (space->identifier_available_count < ArrayCount(space->identifier_available)){
        space->identifier_available[space->identifier_available_count] = index;
    }
    space->identifier_available_count += 1;
}

internal E_Space*
_E_GetSpace(STR_Index name, E_Space *skip, E_Space *first){
    E_Space *result = 0;
    for (E_Space *space = first;
         space != 0;
         space = space->next){
        if (space == skip){
            continue;
        }
        STR_Index space_name = E_SpaceNameIndex(space);
        if (space_name == name){
            result = space;
            break;
        }
    }
    return(result);
}

internal E_Space*
E_GetSpace(STR_Index name, E_Space *skip){
    E_Space *result = _E_GetSpace(name, skip, vars->first_space);
    return(result);
}

internal E_Space*
E_GetInvalidSpace(STR_Index name, E_Space *skip){
    E_Space *result = _E_GetSpace(name, skip, vars->first_invalid_space);
    return(result);
}

internal E_Space*
E_GetSpaceByPath(STR_Index path, E_Space *skip){
    E_Space *result = 0;
    for (E_Space *space = vars->first_space_ordered;
         space != 0;
         space = space->ordered_next){
        if (space == skip){
            continue;
        }
        if (space->save_path == path){
            result = space;
            break;
        }
    }
    return(result);
}

internal E_Definition*
_E_GetDefinition(STR_Index name, E_Definition *skip, E_Definition *first){
    E_Definition *result = 0;
    for (E_Definition *node = first;
         node != 0;
         node = node->next){
        if (node == skip){
            continue;
        }
        STR_Index node_name = E_DefinitionNameIndex(node);
        if (node_name == name){
            result = node;
            break;
        }
    }
    return(result);
}

internal E_Definition*
E_GetDefinition(E_Space *space, STR_Index name, E_Definition *skip){
    E_Definition *result = _E_GetDefinition(name, skip, space->first_definition);
    return(result);
}

internal E_Definition*
E_GetInvalidDefinition(E_Space *space, STR_Index name, E_Definition *skip){
    E_Definition *result = _E_GetDefinition(name, skip, space->first_invalid_definition);
    return(result);
}

internal E_Definition*
E_GetDefinitionByID(E_Space *space, C_Id id, E_Definition *skip){
    E_Definition *result = 0;
    for (E_Definition *node = space->first_definition_ordered;
         node != 0;
         node = node->ordered_next){
        if (node == skip){
            continue;
        }
        if (id == node->id){
            result = node;
            break;
        }
    }
    return(result);
}

internal void
_E_SpaceSetValid(E_Space *space){
    if (space->invalid){
        DLLRemove(vars->first_invalid_space, vars->last_invalid_space, space);
        DLLPushBack(vars->first_space, vars->last_space, space);
        space->invalid = 0;
    }
} 

internal void
_E_SpaceSetInvalid(E_Space *space){
    if (!space->invalid){
        DLLRemove(vars->first_space, vars->last_space, space);
        DLLPushBack(vars->first_invalid_space, vars->last_invalid_space, space);
        space->invalid = 1;
    }
}

internal void
E_SpaceUpdateValidation(E_Space *space){
    b32 should_be_valid = 0;
    STR_Index name = E_SpaceNameIndex(space);
    if (name != 0){
        E_Space *existing_conflict = E_GetSpace(name, space);
        if (existing_conflict == 0){
            should_be_valid = 1;
        }
    }
    
    if (!space->invalid){
        if (name != space->last_name){
            APP_SignalSpaceNameAvailable(space->last_name);
        }
    }
    space->last_name = name;
    
    if (should_be_valid){
        _E_SpaceSetValid(space);
    }
    else{
        _E_SpaceSetInvalid(space);
    }
}

internal String8
E_SerializeSpace(M_Arena *arena, E_Space *space){
    String8_List out = {0};
    
    u8 null_byte = 0;
    String8 null = S8(&null_byte, 1);
    
    for (E_Definition *node = space->first_definition_ordered;
         node != 0;
         node = node->ordered_next){
        // NOTE(allen): id
        StringListPushF(arena, &out, "%llu", node->id);
        StringListPush(arena, &out, null);
        
        // NOTE(allen): flags
        if (node->flags & E_DefinitionFlag_Invalid){
            StringListPush(arena, &out, S8Lit("I"));
        }
        if (node->flags & E_DefinitionFlag_Page){
            StringListPush(arena, &out, S8Lit("P"));
        }
        if (node->flags & E_DefinitionFlag_Test){
            StringListPush(arena, &out, S8Lit("T"));
        }
        StringListPush(arena, &out, null);
        
        // NOTE(allen): name
        StringListPush(arena, &out, E_DefinitionName(node));
        StringListPush(arena, &out, null);
        
        // NOTE(allen): body
        C_StringizeTokensStream(arena, node->body.tokens, node->body.count, &out);
        StringListPush(arena, &out, null);
    }
    
    return(StringListJoin(arena, &out, 0));
}

internal E_Space*
E_DeserializeSpace(String8 data){
    STR_Hash *string_hash = APP_GetStringHash();
    M_Arena *scratch = OS_GetScratch();
    
    E_Space *space = E_NewSpace();
    
    u8 null_byte = 0;
    String8_List parts = StringSplit(scratch, data, &null_byte, 1);
    
    u64 max_id = 0;
    for (String8_Node *node = parts.first;
         node != 0;){
        String8_Node *id_node = node;
        String8_Node *flags_node = ((id_node != 0)?id_node->next:0);
        String8_Node *name_node = ((flags_node != 0)?flags_node->next:0);
        String8_Node *body_node = ((name_node != 0)?name_node->next:0);
        node = ((body_node != 0)?body_node->next:0);
        
        if (body_node != 0){
            u64 id = GetFirstIntegerFromString(id_node->string);
            if (id == 0){
                space->init_error = 1;
                continue;
            }
            E_Definition *duplicate = E_GetDefinitionByID(space, id, 0);
            if (duplicate != 0){
                space->init_error = 1;
                continue;
            }
            
            String8 name_string = name_node->string;
            if (name_string.size > 0 && !C_ValidLabel(name_string)){
                name_string.size = 0;
            }
            
            E_DefinitionFlags flags = 0;
            {
                u64 size = flags_node->string.size;
                u8 *ptr = flags_node->string.str;
                for (u64 i = 0; i < size; i += 1, ptr += 1){
                    if (*ptr == 'I'){
                        flags |= E_DefinitionFlag_Invalid;
                    }
                    if (*ptr == 'P'){
                        flags |= E_DefinitionFlag_Page;
                    }
                    if (*ptr == 'T'){
                        flags |= E_DefinitionFlag_Test;
                    }
                }
            }
            
            STR_Index name = STR_Save(string_hash, name_node->string);
            C_TokenArray tokens = C_Lex(scratch, body_node->string);
            _E_CreateNewDefinitionWithContents(space, flags, name, tokens, id);
            
            max_id = Max(max_id, id);
        }
    }
    if (max_id < (~(u64)0) - (~(u32)0)){
        space->id_counter = max_id + 1;
    }
    else{
        space->init_error = 1;
    }
    space->dirty = 0;
    
    OS_ReleaseScratch(scratch);
    
    return(space);
}

////////////////////////////////
// NOTE(allen): Definitions

internal E_Definition*
_E_AllocateDefinition(void){
    E_Definition *result = vars->free_definition;
    if (result != 0){
        SLLStackPop(vars->free_definition);
    }
    else{
        result = PushArray(vars->arena, E_Definition, 1);
    }
    MemoryZeroStruct(result);
    return(result);
}

internal void
_E_FreeDefinition(E_Definition *definition){
    SLLStackPush(vars->free_definition, definition);
}

internal STR_Index
E_DefinitionNameIndex(E_Definition *definition){
    STR_Index result = 0;
    if (definition->name.count > 0){
        result = definition->name.tokens[0].string;
    }
    return(result);
}

internal String8
E_DefinitionName(E_Definition *definition){
    String8 result = STR_Read(APP_GetStringHash(), E_DefinitionNameIndex(definition));
    return(result);
}

internal b32
E_DefinitionCanEval(E_Definition *definition){
    return(!definition->invalid_name_field &&
           definition->name_cell != 0 &&
           definition->body_cell != 0);
}

internal E_Definition*
_E_CreateNewDefinitionWithContents(E_Space *space, E_DefinitionFlags flags,
                                   STR_Index name, C_TokenArray body, C_Id id){
    E_Definition *definition = _E_AllocateDefinition();
    
    DLLPushBack_NP(space->first_definition_ordered, space->last_definition_ordered, definition, ordered_next, ordered_prev);
    if (flags & E_DefinitionFlag_Invalid){
        DLLPushBack(space->first_invalid_definition, space->last_invalid_definition, definition);
    }
    else{
        DLLPushBack(space->first_definition, space->last_definition, definition);
    }
    definition->flags = flags;
    definition->space = space;
    
    definition->name = E_InitTextFieldTokenBuffer(&definition->name_token_buffer);
    if (name != 0){
        C_Token new_token = { C_TokenKind_Label, name };
        E_TokenBufferReplaceRange(&definition->name, MakeRangeu(0, 0), &new_token, 1);
    }
    
    definition->body = E_InitTokenBuffer();
    if (body.count > 0){
        E_TokenBufferReplaceRange(&definition->body, MakeRangeu(0, 0), body.vals, body.count);
    }
    
    definition->bucket = C_InitCellBucket(APP_GetCellMemory());
    definition->id = id;
    
    space->dirty = 1;
    
    return(definition);
}

internal E_Definition*
_E_CreateNewDefinition(E_Space *space){
    C_TokenArray empty = {0};
    E_Definition *definition = _E_CreateNewDefinitionWithContents(space, 1, 0, empty, E_GetUniqueCompId(space));
    return(definition);
}

internal void
_E_DeleteDefinition(E_Definition *definition){
    E_Space *space = definition->space;
    E_ReleaseTokenBuffer(&definition->body);
    if (definition->flags & E_DefinitionFlag_Invalid){
        DLLRemove(space->first_invalid_definition, space->last_invalid_definition, definition);
    }
    else{
        STR_Index name = E_DefinitionNameIndex(definition);
        E_SignalIdentifierAvailable(definition->space, name);
        DLLRemove(space->first_definition, space->last_definition, definition);
    }
    DLLRemove_NP(space->first_definition_ordered, space->last_definition_ordered, definition, ordered_next, ordered_prev);
    C_ClearBucket(&definition->bucket);
    _E_FreeDefinition(definition);
}

internal void
_E_DefinitionSetValid(E_Definition *definition){
    if (definition->flags & E_DefinitionFlag_Invalid){
        E_Space *space = definition->space;
        DLLRemove(space->first_invalid_definition, space->last_invalid_definition, definition);
        DLLPushBack(space->first_definition, space->last_definition, definition);
        definition->flags ^= E_DefinitionFlag_Invalid;
    }
} 

internal void
_E_DefinitionSetInvalid(E_Definition *definition){
    if (!(definition->flags & E_DefinitionFlag_Invalid)){
        E_Space *space = definition->space;
        DLLRemove(space->first_definition, space->last_definition, definition);
        DLLPushBack(space->first_invalid_definition, space->last_invalid_definition, definition);
        definition->flags ^= E_DefinitionFlag_Invalid;
    }
}

internal void
E_DefinitionUpdateValidation(E_Definition *definition){
    b32 should_be_valid = 0;
    STR_Index name = E_DefinitionNameIndex(definition);
    if (name != 0){
        E_Definition *existing_conflict = E_GetDefinition(definition->space, name, definition);
        if (existing_conflict == 0){
            should_be_valid = 1;
        }
    }
    
    if (!(definition->flags & E_DefinitionFlag_Invalid)){
        if (name != definition->last_name){
            E_SignalIdentifierAvailable(definition->space, definition->last_name);
        }
    }
    definition->last_name = name;
    
    if (should_be_valid){
        _E_DefinitionSetValid(definition);
    }
    else{
        _E_DefinitionSetInvalid(definition);
    }
} 

internal void
E_DefinitionUpdateCompute(E_Definition *definition){
    C_Statics *statics = APP_GetStatics();
    C_CellBucket *bucket = &definition->bucket;
    C_ClearBucket(bucket);
    
    u64 error_count = 0;
    u64 error_space_count = ArrayCount(definition->errors);
    C_ParseError *error_space = definition->errors;
    for (u64 i = 0; i < ArrayCount(definition->buffers); i += 1){
        u64 new_error_count = 0;
        definition->cells[i] = C_Parse(statics, bucket, definition->buffers[i].tokens, definition->buffers[i].count,
                                       error_space, ArrayCount(definition->errors) - error_count, &new_error_count);
        error_count += new_error_count;
    }
    definition->error_count = error_count;
    
    STR_Index name_indx = C_GetIdentifierFromCell(definition->name_cell);
    definition->invalid_name_field = (name_indx == 0);
}

internal C_Cell*
E_DefinitionEvaluate(E_Definition *definition){
    C_EvalCtx eval_ctx = C_InitEvalCtx(APP_GetStatics(), APP_GetEvalBucket(), E_GetGlobalIdEnvironment(definition->space));
    C_Cell *cell = 0;
    if (definition->body_cell != 0){
        C_Cell *env = E_GetGlobalEnvironment(definition->space);
        cell = C_EvaluateDeferred(&eval_ctx, &definition->deferred, &env);
    }
    return(cell);
}

#if 1
internal void
_E_AssertSpaceInvariants(E_Space *space){
    u64 id = 0;
    for (E_Definition *node = space->first_definition_ordered;
         node != 0;
         node = node->ordered_next){
        Assert(id < node->id);
        id = node->id;
        
        E_Definition *scan = space->first_definition;
        if (node->flags & E_DefinitionFlag_Invalid){
            scan = space->first_invalid_definition;
        }
        b32 found = 0;
        for (;scan != 0;
             scan = scan->next){
            if (scan == node){
                found = 1;
                break;
            }
        }
        Assert(found);
    }
}

internal void
_E_AssertGlobalEngineInvariants(void){
    for (E_Space *space = vars->first_space_ordered;
         space != 0;
         space = space->ordered_next){
        _E_AssertSpaceInvariants(space);
    }
}

#else
#define _E_AssertSpaceInvariants(S)
#define _E_AssertGlobalEngineInvariants()
#endif

////////////////////////////////
// NOTE(allen): Tiles

internal E_Tile*
E_AllocateTile(void){
    E_Tile *result = vars->free_tile;
    if (result != 0){
        SLLStackPop(vars->free_tile);
    }
    else{
        result = PushArray(vars->arena, E_Tile, 1);
    }
    MemoryZeroStruct(result);
    return(result);
}

internal void
E_FreeTile(E_Tile *tile){
    SLLStackPush(vars->free_tile, tile);
}

internal void
E_TileInit(E_Tile *tile, E_Definition *definition, R_Font *font){
    tile->definition = definition;
    for (u64 i = 0; i < ArrayCount(tile->editors); i += 1){
        tile->editors[i] = E_InitEditorState(&definition->buffers[i], font);
        E_EditorUpdateLayout(&tile->editors[i], font);
    }
    E_EditorSetFieldMode(&tile->name, E_EditorFieldMode_CodeIdentifier);
    tile->font = font;
}

internal void
E_TileUpdateLayout(E_Tile *tile, R_Font *font){
    tile->font = font;
    for (u64 i = 0; i < ArrayCount(tile->editors); i += 1){
        E_EditorUpdateLayout(&tile->editors[i], font);
    }
}

typedef enum{
    E_TileUIPass_Height,
    E_TileUIPass_Action,
} E_TileUIPass;

internal f32
_E_TileUI(E_View *view, E_Tile *tile, f32 y, Range x, E_TileUIPass pass){
    E_Definition *definition = tile->definition;
    E_EditorState *active = APP_GetActiveEditor();
    
    b32 show_parse = 0;
    
    
    // NOTE(allen): Layout
    String8 empty_field_label[2];
    empty_field_label[0] = S8Lit("name");
    empty_field_label[1] = S8Lit("body");
    
    R_SelectFont(tile->font);
    v2 space_dim = R_StringDim(1.f, S8Lit(" "));
    
    f32 outline_t = 2.f;
    v2 btn_dim = V2Add(space_dim, v2(outline_t*4.f, outline_t*4.f));
    
    Range inner_x = RangeShrink(x, outline_t);
    Range edit_x = inner_x;
    Range button_x = RangeSplit(&edit_x, Side_Min, btn_dim.x);
    
    f32 top = y;
    y += outline_t;
    Range code_y[ArrayCount(tile->editors)];
    {
        E_EditorState *editor = tile->editors;
        for (u64 i = 0; i < ArrayCount(tile->editors); i += 1, editor += 1){
            code_y[i] = MakeRange(y, y + editor->runes.dim.y);
            y = code_y[i].max + outline_t;
        }
    }
    
    show_parse = (show_parse && (definition->body_cell != 0));
    Range parse_y = {0};
    if (show_parse){
        parse_y = MakeRange(y, y + space_dim.y);
        y = parse_y.max + outline_t;
    }
    
    Range eval_y = {0};
    eval_y = MakeRange(y, y + space_dim.y);
    y = eval_y.max + outline_t;
    
    Range inner_y = MakeRange(top + outline_t, y - outline_t);
    Rect rect = MakeRectRanges(x, MakeRange(top, y));
    
    
    if (pass == E_TileUIPass_Action){
        // NOTE(allen): Eval Update
        C_Cell *evaluated = E_DefinitionEvaluate(definition);
        
        // NOTE(allen): Color Profile
        UI_ColorProfile *cl = UI_ColorsDefault();
        if (definition->flags & E_DefinitionFlag_Page){
            cl = UI_ColorsPage();
        }
        else if (definition->flags & E_DefinitionFlag_Test){
            cl = UI_ColorsTest();
        }
        else if (definition->flags & E_DefinitionFlag_Invalid){
            cl = UI_ColorsProblem();
        }
        
        for (u64 i = 0; i < ArrayCount(tile->editors); i += 1){
            if (active == &tile->editors[i]){
                APP_SetActiveTile(tile);
                break;
            }
        }
        
        UI_ActionLevel actlvl = UI_ActionLevel_None;
        if (UI_MouseInRect(rect)){
            actlvl = UI_ActionLevel_Hover;
        }
        else{
            if (APP_GetActiveTile() == tile){
                actlvl = UI_ActionLevel_Active;
            }
        }
        
        // NOTE(allen): Buttons
        {
            Rect box = MakeRectRanges(button_x, inner_y);
            R_Rect(box, cl->outline[UI_ActionLevel_None], 1.f);
            
            UI_ButtonCtx btn_ctx = UI_InitButtonCtx(box, btn_dim, tile->font, UI_IdP(tile));
            btn_ctx.outline_t = outline_t;
            btn_ctx.enable_drop_down = 1;
            btn_ctx.enable_hot_keys = (actlvl == UI_ActionLevel_Active);
            UI_SetColorProfile(&btn_ctx, cl);
            
            if (E_ViewWillCloseDefinition(view)){
                UI_NextHotkey(&btn_ctx, Key_X, KeyModifier_Alt);
                UI_NextTooltip(&btn_ctx, S8Lit("close"));
                UI_SetRedText(&btn_ctx.cl);
                if (UI_Button(&btn_ctx, 'x', ' ')){
                    tile->free_me = 1;
                    view->free_children = 1;
                }
                UI_SetColorProfile(&btn_ctx, cl);
            }
            
            STR_Index name = E_DefinitionNameIndex(definition);
            UI_NextHotkey(&btn_ctx, Key_V, KeyModifier_Alt);
            UI_NextTooltip(&btn_ctx, S8Lit("force validate"));
            UI_NextCondition(&btn_ctx, ((definition->flags & E_DefinitionFlag_Invalid) && name != 0));
            if (UI_Button(&btn_ctx, '!', 'v')){
                E_Definition *existing_conflict = E_GetDefinition(definition->space, name, definition);
                if (existing_conflict != 0){
                    _E_DefinitionSetInvalid(existing_conflict);
                }
                _E_DefinitionSetValid(definition);
            }
            
            UI_NextHotkey(&btn_ctx, Key_I, KeyModifier_Alt);
            UI_NextTooltip(&btn_ctx, S8Lit("view page"));
            if (UI_Button(&btn_ctx, 'I', 'p')){
                definition->flags |= E_DefinitionFlag_Page;
                OP_ViewPageFromDefinition(definition);
            }
            
            UI_NextTooltip(&btn_ctx, S8Lit("toggle test mode"));
            if (UI_Button(&btn_ctx, 'T', 't')){
                definition->flags ^= E_DefinitionFlag_Test;
            }
            
            UI_NextTooltip(&btn_ctx, S8Lit("toggle page mode"));
            if (UI_Button(&btn_ctx, 'P', 't')){
                definition->flags ^= E_DefinitionFlag_Page;
            }
            
            {
                UI_SetRedText(&btn_ctx.cl);
                UI_NextTooltip(&btn_ctx, S8Lit("delete definition"));
                if (UI_Button(&btn_ctx, '-', ' ')){
                    definition->delete_me = 1;
                    definition->space->delete_signal = 1;
                }
                UI_SetColorProfile(&btn_ctx, cl);
            }
        }
        
        
        // NOTE(allen): Code
        E_EditorState *editor = tile->editors;
        for (u64 i = 0; i < ArrayCount(tile->editors); i += 1, editor += 1){
            Rect box = MakeRectRanges(edit_x, code_y[i]);
            Rect clip_restore = R_PushClip(box);
            
            if (active == editor){
                R_Rect(box, cl->back[UI_ActionLevel_Hover], 0.3);
            }
            
            if (UI_MouseInRect(box)){
                R_Rect(box, cl->back[UI_ActionLevel_Hover], 1.f);
                OS_Event *event = 0;
                if (UI_TryGetLeftClick(&event)){
                    APP_SetActiveEditor(editor);
                    E_EditorHandleMousePressEvent(editor, event, box.p0);
                }
            }
            
            E_EditorRender(editor, tile->font, box.p0);
            
            if (active != editor && editor->buffer->count == 0){
                R_String(box.p0, 1.f, empty_field_label[i], cl->front[0], 0.15f);
            }
            R_SetClip(clip_restore);
        }
        
        
        // NOTE(allen): Fallback Clicks
        if (UI_MouseInRect(rect)){
            if (UI_TryEatLeftClick()){
                APP_SetActiveTile(tile);
            }
        }
        
        // NOTE(allen): Parse/Eval Render
        M_Arena *scratch = OS_GetScratch();
        if (show_parse){
            String8 string = C_StringizeCell(scratch, definition->body_cell);
            if (string.size > 0){
                R_String(v2(edit_x.min, parse_y.min), 1.f, string, cl->front[0], 0.5f);
            }
            else{
                R_String(v2(edit_x.min, parse_y.min), 1.f, S8Lit("parse"), cl->front[0], 0.15f);
            }
        }
        
        b32 eval_fallback = 1;
        if (evaluated != 0){
            String8 string = C_StringizeCell(scratch, evaluated);
            if (string.size > 0){
                R_String(v2(edit_x.min, eval_y.min), 1.f, string, cl->front[0], 0.5f);
                eval_fallback = 0;
            }
        }
        if (eval_fallback){
            R_String(v2(edit_x.min, eval_y.min), 1.f, S8Lit("eval"), cl->front[0], 0.15f);
        }
        OS_ReleaseScratch(scratch);
        
        
        // NOTE(allen): Outline
        R_RectOutline(rect, outline_t, cl->outline[actlvl], 1.f);
    }
    
    return(rect.y1 - rect.y0);
}

internal f32
E_TileUIGetHeight(E_View *view, E_Tile *tile, f32 y, Range x){
    return(_E_TileUI(view, tile, y, x, E_TileUIPass_Height));
}

internal f32
E_TileUIUpdateAndRender(E_View *view, E_Tile *tile, f32 y, Range x){
    return(_E_TileUI(view, tile, y, x, E_TileUIPass_Action));
}

internal E_Tile*
E_GetTileForDefinition(E_Tile *first, E_Definition *definition){
    E_Tile *result = 0;
    for (E_Tile *tile = first;
         tile != 0;
         tile = tile->next){
        if (tile->definition == definition){
            result = tile;
            break;
        }
    }
    return(result);
}

////////////////////////////////
// NOTE(allen): Views

internal E_View*
E_NewView(void){
    E_View *result = vars->free_view;
    if (result != 0){
        SLLStackPop(vars->free_view);
    }
    else{
        result = PushArray(vars->arena, E_View, 1);
    }
    MemoryZeroStruct(result);
    DLLPushBack(vars->first_view, vars->last_view, result);
    return(result);
}

internal void
E_DeleteView(E_View *view){
    DLLRemove(vars->first_view, vars->last_view, view);
    SLLStackPush(vars->free_view, view);
}

internal void
_E_ViewFreeAllTiles(E_View *view){
    for (E_Tile *tile = view->first_tile, *next = 0;
         tile != 0;
         tile = next){
        next = tile->next;
        if (tile != 0){
            E_FreeTile(tile);
        }
    }
    view->first_tile = view->last_tile = 0;
}

internal void
E_ViewDeriveFromSource(E_View *view){
    if (view->derive_from_source != 0){
        _E_ViewFreeAllTiles(view);
        view->derive_from_source(view, 0, 0);
    }
}

internal E_Definition*
E_ViewCreateNewDefinition(E_View *view){
    E_Definition *definition = 0;
    if (view->create_new_definition != 0){
        view->create_new_definition(view, 0, (E_ViewCallbackOut*)&definition);
    }
    return(definition);
}

internal void
E_ViewLookAtDefinition(E_View *view, E_Definition *definition){
    if (view->look_at_definition != 0){
        E_ViewCallbackIn in = {0};
        in.definition = definition;
        view->look_at_definition(view, &in, 0);
    }
}

internal b32
E_ViewWillLookAtDefinition(E_View *view){
    return(view->look_at_definition != 0);
}

internal void
E_ViewCleanup(E_View *view){
    if (view->cleanup != 0){
        view->cleanup(view, 0, 0);
    }
}

internal b32
E_ViewWillCloseDefinition(E_View *view){
    return(view->cleanup != 0);
}

internal void
E_ViewShutdown(E_View *view){
    if (view->shutdown != 0){
        view->shutdown(view, 0, 0);
    }
    _E_ViewFreeAllTiles(view);
    MemoryZeroStruct(view);
}

internal String8
E_ViewGetName(E_View *view){
    String8 result = view->name;
    if (view->get_name != 0){
        view->get_name(view, 0, (E_ViewCallbackOut*)&result);
    }
    return(result);
}

internal E_Tile*
E_ViewGetTileForDefinition(E_View *view, E_Definition *definition){
    E_Tile *result = E_GetTileForDefinition(view->first_tile, definition);
    return(result);
}

internal E_Tile*
_E_ViewInsertDefinition(E_View *view, E_Definition *definition){
    E_Tile *tile = E_AllocateTile();
    DLLPushBack(view->first_tile, view->last_tile, tile);
    E_TileInit(tile, definition, view->font);
    return(tile);
}

internal void
_E_ScrollToTile(E_View *view, E_Tile *tile){
    // TODO(allen): Scroll to it
}

internal E_Tile*
_E_ViewInsertOrScrollTo(E_View *view, E_Definition *definition, b32 *is_new_out){
    E_Tile *tile = E_ViewGetTileForDefinition(view, definition);
    if (tile != 0){
        _E_ScrollToTile(view, tile);
        *is_new_out = 0;
    }
    else{
        tile = _E_ViewInsertDefinition(view, definition);
        *is_new_out = 1;
    }
    return(tile);
}

internal E_Definition*
_E_ViewNewDefinitionAndActivate(E_View *view){
    E_Definition *definition = _E_CreateNewDefinition(view->space);
    b32 ignore;
    E_Tile *tile = _E_ViewInsertDefinition(view, definition);
    APP_SetActiveEditor(&tile->name);
    return(definition);
}

internal void
_E_ViewCleanup(E_View *view){
    if (view->free_children){
        view->free_children = 0;
        for (E_Tile *tile = view->first_tile, *next = 0;
             tile != 0;
             tile = next){
            Assert(!tile->definition->delete_me);
            next = tile->next;
            if (tile->free_me){
                DLLRemove(view->first_tile, view->last_tile, tile);
                E_FreeTile(tile);
                view->state_ahead_of_source = 1;
            }
        }
    }
}

////////////////////////////////
// NOTE(allen): Global view

internal
VIEW_CALLBACK_SIG(E_GlobalView_DeriveFromSource){
    Assert(view->first_tile == 0);
    for (E_Definition *node = view->space->first_definition_ordered;
         node != 0;
         node = node->ordered_next){
        _E_ViewInsertDefinition(view, node);
    }
}

internal
VIEW_CALLBACK_SIG(E_GlobalView_CreateNewDefinition){
    out->definition = _E_ViewNewDefinitionAndActivate(view);
}

internal
VIEW_CALLBACK_SIG(E_GlobalView_LookAtDefinition){
    E_Tile *tile = E_ViewGetTileForDefinition(view, in->definition);
    Assert(tile != 0);
    _E_ScrollToTile(view, tile);
    APP_SetActiveEditor(&tile->name);
}

internal
VIEW_CALLBACK_SIG(E_GlobalView_GetName){
    String8 path = STR_Read(APP_GetStringHash(), view->space->save_path);
    out->name = STR_NameFromPath(path);
    if (out->name.size == 0){
        out->name = S8Lit("no path!");
    }
}

internal void
E_InitGlobalView(E_View *view, R_Font *font, E_Space *space){
    view->kind = E_ViewKind_Global;
    view->derive_from_source = E_GlobalView_DeriveFromSource;
    view->create_new_definition = E_GlobalView_CreateNewDefinition;
    view->look_at_definition = E_GlobalView_LookAtDefinition;
    view->get_name = E_GlobalView_GetName;
    view->font = font;
    view->space = space;
    E_GlobalView_DeriveFromSource(view, 0, 0);
}

////////////////////////////////
// NOTE(allen): Page view

internal
VIEW_CALLBACK_SIG(E_PageView_DeriveFromSource){
    Assert(view->first_tile == 0);
    E_Definition *definition = view->definition;
    C_Cell *cell = E_DefinitionEvaluate(definition);
    C_Statics *statics = APP_GetStatics();
    for (C_Cell *node = cell;
         !C_IsNil(node) && node->kind == C_CellKind_Constructed;
         node = node->next){
        C_Cell *first = node->first_child;
        if (first->kind == C_CellKind_Id){
            C_Cell *env = E_GetGlobalIdEnvironment(definition->space);
            C_Cell *resolved = C_EnvironmentResolve(statics, env, first->id);
            if (resolved != 0 && resolved->kind == C_CellKind_Deferred){
                E_Definition *resolved_definition = (E_Definition*)resolved->deferred->user_ptr;
                _E_ViewInsertDefinition(view, resolved_definition);
            }
        }
    }
    view->state_ahead_of_source = 0;
}

internal
VIEW_CALLBACK_SIG(E_PageView_CreateNewDefinition){
    out->definition = _E_ViewNewDefinitionAndActivate(view);
    view->state_ahead_of_source = 1;
}

internal
VIEW_CALLBACK_SIG(E_PageView_LookAtDefinition){
    b32 is_new = 0;
    E_Tile *tile = _E_ViewInsertOrScrollTo(view, in->definition, &is_new);
    APP_SetActiveEditor(&tile->name);
    if (is_new){
        view->state_ahead_of_source = 1;
    }
}

internal
VIEW_CALLBACK_SIG(E_PageView_Cleanup){
    _E_ViewCleanup(view);
    
    if (view->state_ahead_of_source){
        view->state_ahead_of_source = 0;
        
        STR_Hash *string_hash = APP_GetStringHash();
        M_Arena *scratch = OS_GetScratch();
        C_Token *tokens = PushArray(scratch, C_Token, 0);
        
        {
            C_Token *token = PushArray(scratch, C_Token, 1);
            token->kind = C_TokenKind_Quote;
        }
        {
            C_Token *token = PushArray(scratch, C_Token, 1);
            token->kind = C_TokenKind_OpenParen;
        }
        
        u64 counter = 0;
        for (E_Tile *tile = view->first_tile;
             tile != 0;
             tile = tile->next, counter += 1){
            {
                C_Token *token = PushArray(scratch, C_Token, 1);
                token->kind = C_TokenKind_Label;
                M_Temp temp = M_BeginTemp(scratch);
                String8 string = PushStringF(scratch, "#%llu", tile->definition->id);
                token->string = STR_Save(string_hash, string);
                M_EndTemp(temp);
            }
            if (tile->next != 0){
                C_Token *token = PushArray(scratch, C_Token, 1);
                token->kind = ((counter%8 < 7)?C_TokenKind_Space:C_TokenKind_Newline);
            }
        }
        
        {
            C_Token *token = PushArray(scratch, C_Token, 1);
            token->kind = C_TokenKind_CloseParen;
        }
        
        C_Token *opl = PushArray(scratch, C_Token, 0);
        E_TokenBufferReplaceRange(&view->definition->body, MakeRangeu(0, view->definition->body.count), tokens, (opl - tokens));
        
        OS_ReleaseScratch(scratch);
    }
}

internal
VIEW_CALLBACK_SIG(E_PageView_GetName){
    E_Definition *definition = view->definition;
    String8 name = E_DefinitionName(definition);
    out->name = PushStringF(APP_GetFrameArena(), "%.*s", StringExpand(name));
}

internal void
E_InitPageView(E_View *view, R_Font *font, E_Definition *definition){
    view->kind = E_ViewKind_Page;
    view->derive_from_source = E_PageView_DeriveFromSource;
    view->create_new_definition = E_PageView_CreateNewDefinition;
    view->look_at_definition = E_PageView_LookAtDefinition;
    view->cleanup = E_PageView_Cleanup;
    view->get_name = E_PageView_GetName;
    view->font = font;
    view->space = definition->space;
    view->definition = definition;
    E_PageView_DeriveFromSource(view, 0, 0);
}


