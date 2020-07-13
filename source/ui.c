////////////////////////////////
// NOTE(allen): Colors

global v3 cl_black = {0.f, 0.f, 0.f};
global v3 cl_white = {1.f, 1.f, 1.f};
#define cl_gray(f) v3((f), (f), (f))

global v3 cl_red    = {1.f, 0.f, 0.f};
global v3 cl_yellow = {1.f, 1.f, 0.f};
global v3 cl_green  = {0.f, 1.f, 0.f};
global v3 cl_cyan   = {0.f, 1.f, 1.f};
global v3 cl_blue   = {0.f, 0.f, 1.f};
global v3 cl_awful  = {1.f, 0.f, 1.f};

////////////////////////////////

global v3 cl_back_editor      = {0.15f, 0.15f, 0.15f};

global v3 cl_back_unimportant = {0.00f, 0.00f, 0.00f};
global v3 cl_back             = {0.21f, 0.21f, 0.21f};
global v3 cl_back_flash       = {0.50f, 0.50f, 0.50f};

global v3 cl_outline_unimportant = {0.15f, 0.30f, 0.15f};
global v3 cl_outline_important   = {0.09f, 0.50f, 0.09f};
global v3 cl_outline             = {0.00f, 0.70f, 0.00f};
global v3 cl_outline_flash       = {0.50f, 0.85f, 0.50f};

global v3 cl_button_text       = {1.f, 1.f, 0.8f};
global v3 cl_button_text_flash = {0.f, 0.f, 0.2f};


global v3 cl_tab_outline_unimportant = {0.15f, 0.15f, 0.15f};
global v3 cl_tab_outline             = {0.50f, 0.50f, 0.50f};


global v3 cl_err_back_unimportant = {0.100f, 0.00f, 0.00f};
global v3 cl_err_back             = {0.299f, 0.21f, 0.21};
global v3 cl_err_back_flash       = {0.550f, 0.50f, 0.50f};

global v3 cl_err_outline_unimportant = {0.30f, 0.195f, 0.15f};
global v3 cl_err_outline_important   = {0.50f, 0.213f, 0.09f};
global v3 cl_err_outline             = {0.70f, 0.210f, 0.00f};
global v3 cl_err_outline_flash       = {0.85f, 0.605f, 0.50f};


global v3 cl_page_back_unimportant = {0.100f, 0.000f, 0.00f};
global v3 cl_page_back             = {0.299f, 0.299f, 0.21f};
global v3 cl_page_back_flash       = {0.550f, 0.550f, 0.50f};

global v3 cl_page_outline_unimportant = {0.30f, 0.30f, 0.15f};
global v3 cl_page_outline_important   = {0.50f, 0.50f, 0.09f};
global v3 cl_page_outline             = {0.70f, 0.70f, 0.00f};
global v3 cl_page_outline_flash       = {0.85f, 0.85f, 0.50f};


global v3 cl_test_back_unimportant = {0.050f, 0.000f, 0.050f};
global v3 cl_test_back             = {0.150f, 0.105f, 0.150f};
global v3 cl_test_back_flash       = {0.275f, 0.250f, 0.275f};

global v3 cl_test_outline_unimportant = {0.200f, 0.130f, 0.200f};
global v3 cl_test_outline_important   = {0.250f, 0.106f, 0.250f};
global v3 cl_test_outline             = {0.350f, 0.105f, 0.350f};
global v3 cl_test_outline_flash       = {0.425f, 0.303f, 0.425f};


global v3 cl_menu_bar = {0.70f, 0.70f, 0.70f};
global v3 cl_error    = {1.00f, 0.00f, 0.00f};
global v3 cl_text     = {1.00f, 1.00f, 1.00f};
global v3 cl_comment  = {0.50f, 0.50f, 0.65f};
global v3 cl_keyword  = {0.95f, 0.99f, 0.50f};

////////////////////////////////

internal UI_ColorProfile*
UI_ColorsDefault(void){
    local_persist UI_ColorProfile result = {0};
    local_persist b32 first = 1;
    if (first){
        first = 0;
        result.back[0] = cl_back_unimportant;
        result.back[1] = cl_back;
        result.back[2] = cl_back;
        result.back[3] = cl_back_flash;
        result.outline[0] = cl_outline_unimportant;
        result.outline[1] = cl_outline_important;
        result.outline[2] = cl_outline;
        result.outline[3] = cl_outline_flash;
        result.front[0] = cl_button_text;
        result.front[1] = cl_button_text;
        result.front[2] = cl_button_text;
        result.front[3] = cl_button_text_flash;
    }
    return(&result);
}

internal UI_ColorProfile*
UI_ColorsProblem(void){
    local_persist UI_ColorProfile result = {0};
    local_persist b32 first = 1;
    if (first){
        first = 0;
        result.back[0] = cl_err_back_unimportant;
        result.back[1] = cl_err_back;
        result.back[2] = cl_err_back;
        result.back[3] = cl_err_back_flash;
        result.outline[0] = cl_err_outline_unimportant;
        result.outline[1] = cl_err_outline_important;
        result.outline[2] = cl_err_outline;
        result.outline[3] = cl_err_outline_flash;
        result.front[0] = cl_button_text;
        result.front[1] = cl_button_text;
        result.front[2] = cl_button_text;
        result.front[3] = cl_button_text_flash;
    }
    return(&result);
}

internal UI_ColorProfile*
UI_ColorsTabs(void){
    local_persist UI_ColorProfile result = {0};
    local_persist b32 first = 1;
    if (first){
        first = 0;
        result.back[0] = cl_back_unimportant;
        result.back[1] = cl_back_editor;
        result.back[2] = cl_back;
        result.back[3] = cl_back;
        result.outline[0] = cl_tab_outline_unimportant;
        result.outline[1] = cl_back_editor;
        result.outline[2] = cl_tab_outline;
        result.outline[3] = cl_tab_outline;
        for (u64 i = 0; i < (u64)UI_ActionLevel_COUNT; i += 1){
            result.front[i] = cl_button_text;
        }
    }
    return(&result);
}

internal UI_ColorProfile*
UI_ColorsPage(void){
    local_persist UI_ColorProfile result = {0};
    local_persist b32 first = 1;
    if (first){
        first = 0;
        result.back[0] = cl_page_back_unimportant;
        result.back[1] = cl_page_back;
        result.back[2] = cl_page_back;
        result.back[3] = cl_page_back_flash;
        result.outline[0] = cl_page_outline_unimportant;
        result.outline[1] = cl_page_outline_important;
        result.outline[2] = cl_page_outline;
        result.outline[3] = cl_page_outline_flash;
        result.front[0] = cl_button_text;
        result.front[1] = cl_button_text;
        result.front[2] = cl_button_text;
        result.front[3] = cl_button_text_flash;
    }
    return(&result);
}

internal UI_ColorProfile*
UI_ColorsTest(void){
    local_persist UI_ColorProfile result = {0};
    local_persist b32 first = 1;
    if (first){
        first = 0;
        result.back[0] = cl_test_back_unimportant;
        result.back[1] = cl_test_back;
        result.back[2] = cl_test_back;
        result.back[3] = cl_test_back_flash;
        result.outline[0] = cl_test_outline_unimportant;
        result.outline[1] = cl_test_outline_important;
        result.outline[2] = cl_test_outline;
        result.outline[3] = cl_test_outline_flash;
        result.front[0] = cl_button_text;
        result.front[1] = cl_button_text;
        result.front[2] = cl_button_text;
        result.front[3] = cl_button_text_flash;
    }
    return(&result);
}

internal void
UI_SetRedText(UI_ColorProfile *cl){
    for (u64 i = 0; i < (u64)UI_ActionLevel_COUNT; i += 1){
        cl->front[i] = cl_red;
    }
}

internal v3
UI_Grayified(v3 color){
    v3 hsv = RGBToHSV(color);
    hsv.y *= 0.5f;
    hsv.z *= 0.4f;
    v3 result = HSVToRGB(hsv);
    return(result);
}

////////////////////////////////
// NOTE(allen): Event Helpers

internal b32
UI_MouseInRect(Rect rect){
    b32 result = 0;
    if (APP_MouseIsActive()){
        Rect clipped = RectIntersect(rect, R_GetClip());
        result = RectContains(clipped, vars->mouse_p);
    }
    return(result);
}

internal b32
UI_TryGetLeftClick(OS_Event **event_out){
    b32 result = 0;
    OS_Event *event = 0;
    for (;OS_GetNextEvent(&event);){
        if (event->type == OS_EventType_MousePress && event->mouse_button == MouseButton_Left){
            *event_out = event;
            result = 1;
        }
    }
    return(result);
}

internal b32
UI_TryEatLeftClick(void){
    OS_Event *event = 0;
    b32 result = UI_TryGetLeftClick(&event);
    if (result){
        OS_EatEvent(event);
    }
    return(result);
}

internal b32
UI_TryGetKeyPress(OS_Event **event_out, Key key){
    b32 result = 0;
    OS_Event *event = 0;
    for (;OS_GetNextEvent(&event);){
        if (event->type == OS_EventType_KeyPress && event->key == key){
            *event_out = event;
            result = 1;
        }
    }
    return(result);
}

internal b32
UI_TryEatKeyPress(Key key){
    OS_Event *event = 0;
    b32 result = UI_TryGetKeyPress(&event, key);
    if (result){
        OS_EatEvent(event);
    }
    return(result);
}

internal b32
UI_TryGetKeyPressModified(OS_Event **event_out, Key key, KeyModifiers mods){
    b32 result = 0;
    OS_Event *event = 0;
    for (;OS_GetNextEvent(&event);){
        if (event->type == OS_EventType_KeyPress && event->key == key && event->modifiers == mods){
            *event_out = event;
            result = 1;
        }
    }
    return(result);
}

internal b32
UI_TryEatKeyPressModified(Key key, KeyModifiers mods){
    OS_Event *event = 0;
    b32 result = UI_TryGetKeyPressModified(&event, key, mods);
    if (result){
        OS_EatEvent(event);
    }
    return(result);
}

internal b32
UI_TryGetScroll(OS_Event **event_out){
    b32 result = 0;
    OS_Event *event = 0;
    for (;OS_GetNextEvent(&event);){
        if (event->type == OS_EventType_MouseScroll){
            *event_out = event;
            result = 1;
        }
    }
    return(result);
}

internal b32
UI_TryEatScroll(v2 *scroll_out){
    OS_Event *event = 0;
    b32 result = UI_TryGetScroll(&event);
    if (result){
        *scroll_out = event->scroll;
        OS_EatEvent(event);
    }
    return(result);
}

internal String8
UI_StringizeKeyModified(M_Arena *arena, Key key, KeyModifiers mods){
    String8_List list = {0};
    if (mods & KeyModifier_Ctrl){
        StringListPush(arena, &list, S8Lit("ctrl-"));
    }
    if (mods & KeyModifier_Shift){
        StringListPush(arena, &list, S8Lit("shift-"));
    }
    if (mods & KeyModifier_Alt){
        StringListPush(arena, &list, S8Lit("alt-"));
    }
    String8 key_name = KeyName(key);
    StringListPush(arena, &list, key_name);
    return(StringListJoin(arena, &list, 0));
}

////////////////////////////////
// NOTE(allen): UI ID

internal b32
UI_IdEq(UI_Id a, UI_Id b){
    return(MemoryMatchStruct(&a, &b));
}

internal UI_Id
UI_IdZero(void){
    UI_Id id = {0};
    return(id);
}

internal UI_Id
UI_IdV(u64 v){
    UI_Id id;
    id.v1 = v;
    return(id);
}

internal UI_Id
UI_IdP(void *p){
    UI_Id id;
    id.p1 = p;
    return(id);
}

////////////////////////////////
// NOTE(allen): Button placer

internal void
UI_SetColorProfile(UI_ButtonCtx *ctx, UI_ColorProfile *profile){
    MemoryCopyStruct(&ctx->cl, profile);
}

internal UI_ButtonCtx
UI_InitButtonCtx(Rect rect, v2 btn_dim, R_Font *font, UI_Id id){
    UI_ButtonCtx result = {0};
    
    v2 rect_dim = RectGetDim(rect);
    if (rect_dim.x >= btn_dim.x && rect_dim.y >= btn_dim.y){
        result.font = font;
        result.id = id;
        result.button_id.v1 = 1;
        result.rect = rect;
        result.p = rect.p0;
        result.btn_dim = btn_dim;
        result.has_room = 1;
        
        result.condition = 1;
        result.text_scale = 1;
        
        UI_SetColorProfile(&result, UI_ColorsDefault());
    }
    
    return(result);
}

internal f32
UI_ButtonCtxGetTraversedY(UI_ButtonCtx *ctx){
    return(ctx->p.y - ctx->rect.y0);
}

internal void
UI_NextCondition(UI_ButtonCtx *ctx, b32 condition){
    ctx->condition = condition;
}

internal void
UI_NextHotkey(UI_ButtonCtx *ctx, Key key, KeyModifiers mods){
    ctx->has_hot_key = 1;
    ctx->hot_key = key;
    ctx->hot_key_mods = mods;
}

internal void
UI_NextTooltip(UI_ButtonCtx *ctx, String8 string){
    ctx->tool_tip = string;
}

internal void
UI_NextActive(UI_ButtonCtx *ctx, b32 active){
    ctx->active = active;
}

internal void
_UI_ButtonHotkey(UI_ButtonCtx *ctx, b32 *result_out){
    if (ctx->enable_hot_keys && ctx->has_hot_key && ctx->condition){
        if (UI_TryEatKeyPressModified(ctx->hot_key, ctx->hot_key_mods)){
            *result_out = 1;
            APP_ZeroOwnershipOfFloatingWindow();
        }
    }
}

internal f32
_UI_ButtonGetXAdvance(UI_ButtonCtx *ctx){
    f32 advance = 0;
    if (ctx->enable_flexible_x_advance){
        advance = ClampTop(ctx->this_button_x_advance, ctx->btn_dim.x);
    }
    else{
        advance = ctx->btn_dim.x;
    }
    return(advance);
}

internal b32
_UI_ButtonGetNextP(UI_ButtonCtx *ctx, v2 p, v2 *out){
    b32 result = 1;
    v2 dim = ctx->btn_dim;
    p.x += _UI_ButtonGetXAdvance(ctx);
    if (p.x + dim.x > ctx->rect.x1){
        p.x = ctx->rect.x0;
        p.y += dim.y;
        if (p.y + dim.y > ctx->rect.y1){
            result = 0;
        }
    }
    *out = p;
    return(result);
}

internal b32
_UI_ButtonNextWillFit(UI_ButtonCtx *ctx){
    b32 result = 1;
    v2 p = ctx->p;
    if (!_UI_ButtonGetNextP(ctx, p, &p)){
        result = 0;
    }
    return(result);
}

internal Rect
_UI_ButtonPre(UI_ButtonCtx *ctx, UI_ActionLevel *action_level_out, b32 *result_out){
    // NOTE(allen): Drop down check
    ctx->do_drop_down = (ctx->enable_drop_down && !_UI_ButtonNextWillFit(ctx));
    if (ctx->do_drop_down){
        ctx->restore.condition = ctx->condition;
        ctx->restore.active = ctx->active;
        ctx->restore.has_hot_key = ctx->has_hot_key;
        ctx->restore.tool_tip = ctx->tool_tip;
        ctx->condition = 1;
        ctx->active = 0;
        ctx->has_hot_key = 0;
        ctx->tool_tip = S8Lit("more");
        ctx->has_room = 0;
    }
    
    // NOTE(allen): Layout
    f32 right = ctx->p.x + _UI_ButtonGetXAdvance(ctx);;
    Rect rect = MakeRect(V2Expand(ctx->p), right, ctx->p.y + ctx->btn_dim.y);
    
    // NOTE(allen): Mouse
    UI_ActionLevel action_level = UI_ActionLevel_None;
    if (ctx->active){
        action_level = UI_ActionLevel_Active;
    }
    ctx->did_tool_tip = 0;
    if (UI_MouseInRect(rect)){
        if (ctx->condition){
            action_level = UI_ActionLevel_Hover;
            if (UI_TryEatLeftClick()){
                *result_out = 1;
                action_level = UI_ActionLevel_Flash;
            }
        }
        
        // NOTE(allen): Tool tip
        {
            M_Arena *scratch = OS_GetScratch();
            String8_List tool_tip = {0};
            StringListPush(scratch, &tool_tip, ctx->tool_tip);
            if (ctx->has_hot_key){
                StringListPush(scratch, &tool_tip, S8Lit(" ["));
                String8 key_str = UI_StringizeKeyModified(scratch, ctx->hot_key, ctx->hot_key_mods);
                StringListPush(scratch, &tool_tip, key_str);
                StringListPush(scratch, &tool_tip, S8Lit("]"));
            }
            String8 tool_tip_str = StringListJoin(APP_GetFrameArena(), &tool_tip, 0);
            APP_SetToolTip(tool_tip_str);
            OS_ReleaseScratch(scratch);
            
            if (tool_tip_str.size > 0){
                ctx->did_tool_tip = 1;
            }
        }
    }
    *action_level_out = action_level;
    
    // NOTE(allen): Render back
    R_SelectFont(ctx->font);
    v3 back = ctx->cl.back[action_level];
    v3 outline = ctx->cl.outline[action_level];
    if (!ctx->condition){
        back = UI_Grayified(back);
        outline = UI_Grayified(outline);
    }
    R_Rect(rect, back, 1.f);
    R_RectOutline(rect, ctx->outline_t, outline, 1.f);
    
    return(rect);
}

internal void
_UI_ButtonPost(UI_ButtonCtx *ctx){
    if (!_UI_ButtonGetNextP(ctx, ctx->p, &ctx->p)){
        ctx->has_room = 0;
    }
    
    if (ctx->do_drop_down){
        ctx->condition = ctx->restore.condition;
        ctx->active = ctx->restore.active;
        ctx->has_hot_key = ctx->restore.has_hot_key;
        ctx->tool_tip = ctx->restore.tool_tip;
    }
}

internal void
_UI_ButtonEatExtendedParameters(UI_ButtonCtx *ctx){
    ctx->condition = 1;
    ctx->active = 0;
    ctx->has_hot_key = 0;
    ctx->tool_tip.size = 0;
}

internal void
_UI_DropDownButtonClick(UI_ButtonCtx *ctx, b32 click_result){
    if (ctx->enable_hot_keys){
        if (UI_TryEatKeyPressModified(Key_ForwardSlash, KeyModifier_Ctrl)){
            APP_TakeOwnershipOfFloatingWindow(ctx->id);
        }
    }
    if (click_result){
        APP_TakeOwnershipOfFloatingWindow(ctx->id);
    }
}

internal void
UI_ButtonDropdownCallback(void *ptr, APP_FloatingWindowResult *result);

internal UI_ButtonRecord*
_UI_DropDownButtonSaveRecord(UI_ButtonCtx *ctx, Rect rect, b32 *result_out){
    M_Arena *arena = APP_GetFrameArena();
    
    UI_ButtonDropdown *last_frame_dropdown = (UI_ButtonDropdown*)APP_GetLastFrameFloatingWindowPtr();
    if (last_frame_dropdown != 0){
        UI_ButtonRecord *activated = last_frame_dropdown->activated;
        if (activated != 0 && UI_IdEq(activated->id, ctx->button_id)){
            *result_out = 1;
            APP_ZeroOwnershipOfFloatingWindow();
        }
    }
    
    UI_ButtonDropdown *dropdown = (UI_ButtonDropdown*)APP_GetFloatingWindowPtr();
    if (dropdown == 0){
        dropdown = PushArrayZero(arena, UI_ButtonDropdown, 1);
        dropdown->font = ctx->font;
        dropdown->btn_dim = ctx->btn_dim;
        dropdown->source_rect = rect;
        APP_SetFloatingWindowPtrAndCallback(dropdown, UI_ButtonDropdownCallback);
    }
    
    UI_ButtonRecord *record = PushArray(arena, UI_ButtonRecord, 1);
    record->condition = ctx->condition;
    record->has_hot_key = ctx->has_hot_key;
    record->hot_key = ctx->hot_key;
    record->hot_key_mods = ctx->hot_key_mods;
    record->outline_t = ctx->outline_t;
    record->text_scale = ctx->text_scale;
    MemoryCopyStruct(&record->cl, &ctx->cl);
    record->tool_tip = ctx->tool_tip;
    record->id = ctx->button_id;
    
    SLLQueuePush(dropdown->first, dropdown->last, record);
    dropdown->count += 1;
    
    return(record);
}

internal b32
UI_Button(UI_ButtonCtx *ctx, u8 major, u8 minor){
    b32 result = 0;
    
    _UI_ButtonHotkey(ctx, &result);
    
    b32 did_button = ctx->has_room;
    Rect rect = {0};
    if (ctx->has_room){
        b32 click_result = 0;
        UI_ActionLevel action_level;
        rect = _UI_ButtonPre(ctx, &action_level, &click_result);
        
        
        
        String8 on_screen_major = S8(&major, 1);
        String8 on_screen_minor = S8(&minor, 1);
        v3 text_color = ctx->cl.front[action_level];
        
        if (!ctx->condition){
            text_color = UI_Grayified(text_color);
        }
        
        if (ctx->do_drop_down){
            _UI_DropDownButtonClick(ctx, click_result);
            on_screen_major = S8Lit("/");
            on_screen_minor = S8Lit(" ");
            text_color = cl_button_text;
            did_button = 0;
        }
        else{
            if (click_result){
                result = 1;
            }
        }
        
        f32 major_scale = ctx->text_scale;
        f32 minor_scale = ctx->text_scale*0.65f;
        
        v2 major_dim = R_StringDim(major_scale, on_screen_major);
        v2 minor_dim = R_StringDim(minor_scale, on_screen_minor);
        
        if (on_screen_minor.str[0] == ' '){
            minor_dim.x = 0.f;
            minor_dim.y = 0.f;
        }
        
        v2 major_p = {0};
        v2 minor_p = {0};
        minor_p.x = major_p.x + 0.8f*major_dim.x;
        // align 0.7 of the way down major with 0.5 of the way down minor
        //  solve for minor_y in (major_y + 0.7*major_h = minor_y + 0.5*minor_h)
        minor_p.y = major_p.y + 0.7f*major_dim.y - 0.5f*minor_dim.y;
        
        v2 combined_dim;
        combined_dim.x = Max(major_p.x + major_dim.x, minor_p.x + minor_dim.x);
        combined_dim.y = Max(major_p.y + major_dim.y, minor_p.y + minor_dim.y);
        
        v2 combined_half_dim = V2Mul(combined_dim, 0.5f);
        v2 center = RectGetCenter(rect);
        
        major_p = V2Sub(center, combined_half_dim);
        minor_p = V2Add(major_p, minor_p);
        
        major_p.x = f32Floor(major_p.x);
        major_p.y = f32Floor(major_p.y);
        minor_p.x = f32Floor(minor_p.x);
        minor_p.y = f32Floor(minor_p.y);
        
        R_String(major_p, major_scale, on_screen_major, text_color, 1.f);
        R_String(minor_p, minor_scale, on_screen_minor, text_color, 1.f);
        
        
        
        _UI_ButtonPost(ctx);
    }
    
    if (ctx->enable_drop_down && !did_button){
        if (UI_IdEq(APP_OwnerOfFloatingWindow(), ctx->id)){
            b32 drop_result = 0;
            UI_ButtonRecord *record = _UI_DropDownButtonSaveRecord(ctx, rect, &drop_result);
            if (drop_result){
                result = 1;
            }
            record->kind = UI_ButtonKind_Icon;
            record->major = major;
            record->minor = minor;
        }
    }
    
    ctx->button_id.v1 += 1;
    _UI_ButtonEatExtendedParameters(ctx);
    
    return(result);
}

internal b32
UI_ButtonLabel(UI_ButtonCtx *ctx, String8 string){
    b32 result = 0;
    
    _UI_ButtonHotkey(ctx, &result);
    
    b32 did_button = ctx->has_room;
    Rect rect = {0};
    if (ctx->has_room){
        v2 dim = R_StringDim(ctx->text_scale, string);
        if (ctx->enable_flexible_x_advance){
            ctx->this_button_x_advance = dim.x + ctx->outline_t*2.f;
        }
        
        
        
        b32 click_result = 0;
        UI_ActionLevel action_level;
        rect = _UI_ButtonPre(ctx, &action_level, &click_result);
        b32 did_tool_tip = ctx->did_tool_tip;
        
        
        v3 text_color = ctx->cl.front[action_level];
        if (!ctx->condition){
            text_color = UI_Grayified(text_color);
        }
        
        String8 on_screen_string = string;
        if (ctx->do_drop_down){
            _UI_DropDownButtonClick(ctx, click_result);
            on_screen_string = S8Lit("* more *");
            text_color = cl_button_text;
            did_button = 0;
        }
        else{
            if (click_result){
                result = 1;
            }
        }
        
        Rect inner = RectShrink(rect, ctx->outline_t);
        
        v2 p = {0};
        p.x = inner.x0;
        
        // vertically align center of text with center of box
        //  solve for y in: y + 0.5*h = center.y
        p.y = RectGetCenter(inner).y - 0.5f*dim.y;
        
        R_StringCapped(p, inner.x1, ctx->text_scale, on_screen_string, text_color, 1.f);
        
        if ((action_level == UI_ActionLevel_Hover ||
             action_level == UI_ActionLevel_Flash) &&
            did_button && !did_tool_tip &&
            dim.x > (inner.x1 - inner.x0)){
            APP_SetToolTip(on_screen_string);
        }
        
        
        
        _UI_ButtonPost(ctx);
    }
    
    if (ctx->enable_drop_down && !did_button){
        if (UI_IdEq(APP_OwnerOfFloatingWindow(), ctx->id)){
            b32 drop_result = 0;
            UI_ButtonRecord *record = _UI_DropDownButtonSaveRecord(ctx, rect, &drop_result);
            if (drop_result){
                result = 1;
            }
            record->kind = UI_ButtonKind_Label;
            record->string = string;
        }
    }
    
    ctx->button_id.v1 += 1;
    _UI_ButtonEatExtendedParameters(ctx);
    
    return(result);
}

internal void
UI_ButtonDropdownCallback(void *ptr, APP_FloatingWindowResult *result){
    UI_ButtonDropdown *dropdown = (UI_ButtonDropdown*)ptr;
    
    // NOTE(allen): Select drop down placement
    f32 best_area = NegInf32();
    v2 best_p = {0};
    v2 best_dim = {0};
    Side best_cast_dir[2] = {0};
    
    Rect win = MakeRect(0, 0, V2Expand(vars->window_dim));
    Rect src = dropdown->source_rect;
    
    for (u32 x_it = 0; x_it < 2; x_it += 1){
        u32 x_side = x_it^1;
        for (u32 y_side = 0; y_side < 2; y_side += 1){
            v2 p = v2(src.p[x_side].x, src.p[y_side].y);
            Range x = MakeRange(p.x, win.p[x_side].x);
            Range y = MakeRange(p.y, win.p[y_side^1].y);
            v2 dim = v2(RangeSize(x), RangeSize(y));
            f32 area = dim.x*dim.y;
            if (area > best_area){
                best_area = area;
                best_p = p;
                best_dim = dim;
                best_cast_dir[Dimension_X] = x_side;
                best_cast_dir[Dimension_Y] = y_side^1;
            }
        }
    }
    
    
    // NOTE(allen): Fit the drop down to a nice aesthetic rectangle
    v2 weight = V2Hadamard(v2(1.f, SmallGolden32), dropdown->btn_dim);
    v2 dim = v2(1, 1);
    f32 count = (f32)dropdown->count;
    for (;dim.x*dim.y < count;){
        f32 scores[2];
        for (u32 i = 0; i < 2; i += 1){
            dim.v[i] += 1;
            scores[i] = V2Dot(dim, weight)/(dim.x*dim.y);
            dim.v[i] -= 1;
        }
        if (scores[0] <= scores[1]){
            dim.v[0] += 1;
        }
        else{
            dim.v[1] += 1;
        }
    }
    
    v2 pixel_dim = V2Hadamard(dim, dropdown->btn_dim);
    
    Range x = MakeRange(best_p.x, best_p.x + SignOfSide(best_cast_dir[Dimension_X])*pixel_dim.x);
    Range y = MakeRange(best_p.y, best_p.y + SignOfSide(best_cast_dir[Dimension_Y])*pixel_dim.y);
    Rect rect = MakeRectRanges(x, y);
    Rect outer = RectGrow(rect, 4.f);
    Rect inner = RectShrink(outer, 2.f);
    
    
    // NOTE(allen): Place buttons
    R_RectOutline(outer, 2.f, cl_white, 1.f);
    R_Rect(inner, cl_black, 1.f);
    
    inner.p0 = rect.p0;
    
    UI_ButtonCtx btn_ctx = UI_InitButtonCtx(inner, dropdown->btn_dim, dropdown->font, UI_IdV(0));
    
    for (UI_ButtonRecord *node = dropdown->first;
         node != 0;
         node = node->next){
        MemoryCopyStruct(&btn_ctx.cl, &node->cl);
        btn_ctx.outline_t = node->outline_t;
        btn_ctx.text_scale = node->text_scale;
        UI_NextCondition(&btn_ctx, node->condition);
        if (node->has_hot_key){
            UI_NextHotkey(&btn_ctx, node->hot_key, node->hot_key_mods);
        }
        UI_NextTooltip(&btn_ctx, node->tool_tip);
        if (node->kind == UI_ButtonKind_Icon){
            if (UI_Button(&btn_ctx, node->major, node->minor)){
                dropdown->activated = node;
            }
        }
        else if (node->kind == UI_ButtonKind_Label){
            if (UI_ButtonLabel(&btn_ctx, node->string)){
                dropdown->activated = node;
            }
        }
    }
    
    result->rect = outer;
}
