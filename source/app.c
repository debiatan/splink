#include "language_layer.h"
#include "app_memory.h"
#include "os.h"
#include "opengl.h"
#include "render.h"
#include "string_hash.h"
#include "ui.h"
#include "compute.h"
#include "token_buffer.h"
#include "definition.h"
#include "app.h"
#include "operations.h"

#include "language_layer.c"
#include "app_memory.c"
#include "os.c"

#include "app_core.c"
#include "string_hash.c"
#include "render.c"
#include "ui.c"
#include "compute.c"
#include "token_buffer.c"
#include "definition.c"
#include "operations.c"

////////////////////////////////

internal b32
APP_FuzzyMatch(String8 name, String8_List *filter_pattern){
    b32 result = 1;
    for (String8_Node *node = filter_pattern->first;
         node != 0;
         node = node->next){
        if (StringFindSubstringStart(name, node->string, StringMatchFlag_CaseInsensitive) == ~(u64)0){
            result = 0;
            break;
        }
    }
    return(result);
}

internal void
APP_TopLevelNavigate(Dimension dimension, Side side){
    if (dimension == Dimension_Y){
        E_EditorState *new_editor = vars->neighbor_editors[side];
        if (new_editor != 0){
            f32 x = vars->active_editor?vars->active_editor->preferred_x:0;
            E_EditorEnterFrom(new_editor, Dimension_Y, side^1, x);
            vars->active_editor = new_editor;
            MemoryZeroArray(vars->neighbor_editors);
        }
    }
    else{
        NotImplemented;
    }
}

internal void
APP_ApplyViewChange(E_View *view){
    if (vars->change_view != 0){
        E_ViewDeriveFromSource(view);
        vars->active_view = view;
    }
    vars->change_view = 0;
}

internal void
APP_SpaceHandleDeleteSignal(E_Space *space){
    space->delete_signal = 0;
    for (E_Definition *node = space->first_definition_ordered, *next = 0;
         node != 0;
         node = next){
        next = node->ordered_next;
        if (node->delete_me){
            _E_DeleteDefinition(node);
            space->dirty = 1;
        }
    }
}

internal void
APP_CloseSpace(E_Space *space){
    b32 deleted_active_view = 0;
    E_View *active_view = vars->active_view;
    for (E_View *view = vars->first_view, *next = 0;
         view != 0;
         view = next){
        next = view->next;
        if (view->space == space){
            if (view == active_view){
                deleted_active_view = 1;
            }
            E_DeleteView(view);
        }
    }
    
    E_DeleteSpace(space);
    
    if (deleted_active_view){
        vars->active_view = 0;
        if (vars->first_view != 0){
            APP_SignalViewChange(vars->first_view);
        }
    }
}

internal E_Space*
APP_ReloadSpace(E_Space *space, String8 serialized){
    E_Space *result = E_DeserializeSpace(serialized);
    
    if (result == 0){
        result = space;
    }
    else{
        E_View *active_view = vars->active_view;
        b32 deleted_active_view = 0;
        b32 effected_active_view = 0;
        
        for (E_View *view = vars->first_view, *next = 0;
             view != 0;
             view = next){
            next = view->next;
            if (view->space == space){
                E_Definition *definition = view->definition;
                E_Definition *replacement_definition = 0;
                if (definition != 0){
                    replacement_definition = E_GetDefinitionByID(result, definition->id, 0);
                }
                
                if (definition != 0 && replacement_definition == 0){
                    if (view == active_view){
                        deleted_active_view = 1;
                        effected_active_view = 1;
                    }
                    E_DeleteView(view);
                }
                else{
                    if (view == active_view){
                        effected_active_view = 1;
                    }
                    view->space = result;
                    view->definition = replacement_definition;
                }
            }
        }
        
        E_View *global_view = APP_GetGlobalView(result);
        if (global_view == 0){
            global_view = E_NewView();
            E_InitGlobalView(global_view, &vars->font, result);
        }
        if (deleted_active_view){
            active_view = global_view;
        }
        if (effected_active_view){
            vars->active_view = 0;
            APP_SignalViewChange(active_view);
        }
        
        E_DeleteSpace(space);
    }
    
    return(result);
}

////////////////////////////////

APP_PERMANENT_LOAD
{
    os = os_;
    
#if 1
    _STR_Test();
    _E_TokenBuffer_Test();
#endif
    
    {
        M_Arena arena_ = M_ArenaInitialize();
        vars = PushArray(&arena_, APP_Variables, 1);
        MemoryCopyStruct(&vars->arena_, &arena_);
    }
    M_Arena *arena = vars->arena = &vars->arena_;
    
    ////////////////////////////////
    // NOTE(allen): Init rendering
    
    R_Init(arena);
    R_InitFont(&vars->font, S8Lit("liberation-mono.ttf"), 20);
    R_SelectFont(&vars->font);
    
    ////////////////////////////////
    // NOTE(allen): Init splink engine
    
    for (u64 i = 0; i < ArrayCount(vars->frame_arena); i += 1){
        vars->frame_arena[i] = M_ArenaInitialize();
    }
    vars->string_hash = STR_InitHash();
    
    vars->cells = C_InitCellMemory();
    vars->static_bucket = C_InitCellBucket(&vars->cells);
    vars->statics = C_InitStatics(&vars->static_bucket);
    vars->global_defines_bucket = C_InitCellBucket(&vars->cells);
    vars->eval_bucket = C_InitCellBucket(&vars->cells);
    
    for (u64 i = 0; i < C_BuiltInIndex_COUNT; i += 1){
        vars->keyword_table[i] = STR_Save(&vars->string_hash, C_GetBuiltInKeyword(i));
    }
    
    ////////////////////////////////
    // NOTE(allen): UI State
    
    vars->lister_flags = ~0;
    vars->panel_filter_buffer = E_InitTextFieldTokenBuffer(&vars->panel_filter_memory);
    vars->panel_filter = E_InitEditorState(&vars->panel_filter_buffer, &vars->font);
}

APP_HOT_LOAD
{
    os = os_;
}

APP_HOT_UNLOAD {}

////////////////////////////////

APP_UPDATE
{
    ////////////////////////////////
    // NOTE(allen): Frame setup
    
    vars->frame_time = os->GetTime();
    vars->window_dim = v2(os->window_size.x, os->window_size.y);
    R_Begin(vars->window_dim, cl_black);
    
    M_ArenaClear(APP_GetFrameArena());
    
    ////////////////////////////////
    // NOTE(allen): Cleanup deleted definitions
    
    if (vars->active_view != 0){
        E_View *view = vars->active_view;
        E_Space *space = view->space;
        if (space->delete_signal){
            APP_SpaceHandleDeleteSignal(space);
            vars->active_view = 0;
            APP_SignalViewChange(view);
        }
    }
    
    for (E_Space *space = vars->first_space;
         space != 0;
         space = space->next){
        if (space->delete_signal){
            APP_SpaceHandleDeleteSignal(space);
        }
    }
    
    ////////////////////////////////
    // NOTE(allen): Cleanup closed tiles
    
    for (E_View *view = vars->first_view;
         view != 0;
         view = view->next){
        E_ViewCleanup(view);
    }
    
    ////////////////////////////////
    // NOTE(allen): Auto-Save
    
    {
        M_Arena *scratch = OS_GetScratch();
        M_Temp restore = M_BeginTemp(scratch);
        for (E_Space *space = vars->first_space_ordered;
             space != 0;
             space = space->ordered_next){
            if (space->dirty && space->save_path != 0){
                if (space->last_save_time + 3.f <= vars->frame_time){
                    space->last_save_time = vars->frame_time;
                    String8 path = STR_Read(APP_GetStringHash(), space->save_path);
                    String8 serialized = E_SerializeSpace(scratch, space);
                    space->dirty = os->SaveToFile(path, serialized.str, serialized.size);
                    M_EndTemp(restore);
                }
            }
        }
        OS_ReleaseScratch(scratch);
    }
    
    ////////////////////////////////
    // NOTE(allen): Set Mouse Position
    
    UI_Id main_mouse_layer = UI_IdV(APP_MouseLayer_Main);
    UI_Id floating_window_mouse_layer = UI_IdV(APP_MouseLayer_FloatingWindow);
    vars->active_mouse_layer = main_mouse_layer;
    
    MemoryZeroStruct(&vars->tool_tip_string);
    
    {
        OS_Event *event = 0;
        for (;OS_GetNextEvent(&event);){
            switch (event->type){
                case OS_EventType_MouseMove:
                {
                    vars->mouse_p = event->position;
                    OS_EatEvent(event);
                }break;
            }
        }
    }
    
    ////////////////////////////////
    // NOTE(allen): Floating Window Pre-Update
    
    vars->last_frame_owner_of_floating_window = vars->owner_of_floating_window;
    vars->last_frame_floating_window_ptr = vars->floating_window_ptr;
    vars->floating_window_ptr = 0;
    vars->floating_window_callback = 0;
    
    if (!UI_IdEq(vars->owner_of_floating_window, UI_IdZero())){
        APP_SetMouseLayer(floating_window_mouse_layer);
        vars->active_mouse_layer = floating_window_mouse_layer;
        if (UI_TryEatKeyPress(Key_Esc)){
            MemoryZeroStruct(&vars->owner_of_floating_window);
        }
        if (!UI_MouseInRect(vars->floating_window_last_frame_rect)){
            if (UI_TryEatLeftClick()){
                MemoryZeroStruct(&vars->owner_of_floating_window);
            }
        }
    }
    
    if (UI_IdEq(vars->owner_of_floating_window, UI_IdZero())){
        vars->active_mouse_layer = main_mouse_layer;
    }
    
    ////////////////////////////////
    // NOTE(allen): Top Level UI Layout
    
    APP_SetMouseLayer(main_mouse_layer);
    
    v2 space_dim = R_StringDimWithFont(&vars->font, 1.f, S8Lit(" "));
    f32 outline_t = 2.f;
    f32 bar_h = space_dim.y + outline_t*2.f;
    v2 btn_dim = {space_dim.x*3.f};
    btn_dim.y = btn_dim.x;
    
    Range y_free = MakeRange(0, vars->window_dim.y);
    Range x_free = MakeRange(0, vars->window_dim.x);
    
    Range panel_x = RangeSplit(&x_free, Side_Min, space_dim.x*21.f);
    x_free.min += outline_t;
    
    Range y_free_panel = y_free;
    Range tool_box_y = RangeSplit(&y_free_panel, Side_Min, btn_dim.y);
    Range lister_toggles_y = RangeSplit(&y_free_panel, Side_Min, btn_dim.y);
    y_free_panel.min += outline_t;
    Range filter_field_y = RangeSplit(&y_free_panel, Side_Min, bar_h);
    y_free_panel.min += outline_t;
    Range lister_y = y_free_panel;
    
    
    Range code_x = x_free;
    Range view_buttons_y = RangeSplit(&y_free, Side_Min, btn_dim.y);
    y_free_panel.min += outline_t;
    Range tabs_y = RangeSplit(&y_free, Side_Min, bar_h);
    Range code_y = y_free;
    
    
    ////////////////////////////////
    // NOTE(allen): Tool Box
    
    {
        Rect rect = MakeRectRanges(panel_x, tool_box_y);
        R_Rect(rect, cl_back_unimportant, 1.f);
        
        UI_ButtonCtx btn_ctx = UI_InitButtonCtx(rect, btn_dim, &vars->font, UI_IdV(APP_BtnCtx_ToolBox));
        btn_ctx.outline_t = outline_t;
        btn_ctx.enable_drop_down = 1;
        btn_ctx.enable_hot_keys = 1;
        
        UI_NextHotkey(&btn_ctx, Key_N, KeyModifier_Ctrl);
        UI_NextTooltip(&btn_ctx, S8Lit("new definition"));
        if (UI_Button(&btn_ctx, '+', 'n')){
            if (vars->active_view != 0){
                E_ViewCreateNewDefinition(vars->active_view);
            }
            else{
                E_Space *empty_space = E_NewSpace();
                E_View *global_view = E_NewView();
                E_InitGlobalView(global_view, &vars->font, empty_space);
                APP_SetActiveView(global_view);
                E_ViewCreateNewDefinition(global_view);
            }
        }
        
        UI_NextHotkey(&btn_ctx, Key_T, KeyModifier_Ctrl);
        UI_NextTooltip(&btn_ctx, S8Lit("new page"));
        if (UI_Button(&btn_ctx, '+', 't')){
            if (vars->active_view != 0){
                E_Definition *definition = E_ViewCreateNewDefinition(vars->active_view);
                E_View *view = E_NewView();
                E_InitPageView(view, vars->active_view->font, definition);
                APP_SignalViewChange(view);
            }
            else{
                E_Space *empty_space = E_NewSpace();
                E_View *global_view = E_NewView();
                E_InitGlobalView(global_view, &vars->font, empty_space);
                APP_SetActiveView(global_view);
                E_ViewCreateNewDefinition(global_view);
            }
        }
        
        String8 ext[2];
        ext[0] = S8Lit("Splink");
        ext[1] = S8Lit("splink");
        
        E_Space *space = APP_GetActiveSpace();
        UI_NextCondition(&btn_ctx, space != 0);
        UI_NextHotkey(&btn_ctx, Key_S, KeyModifier_Ctrl);
        UI_NextTooltip(&btn_ctx, S8Lit("set save path"));
        if (UI_Button(&btn_ctx, 'v', 'S')){
            M_Arena *scratch = OS_GetScratch();
            Assert(space != 0);
            String8 string = os->DialogueSavePath(scratch, ext);
            if (string.size > 0){
                if (!StringMatch(StringPostfix(string, 7),
                                 S8Lit(".splink"))){
                    string = PushStringCat(scratch, string, S8Lit(".splink"));
                }
                space->dirty = 1;
                space->last_save_time = 0.f;
                space->save_path = STR_Save(APP_GetStringHash(), string);
                E_SpaceUpdateValidation(space);
            }
            OS_ReleaseScratch(scratch);
        }
        
        UI_NextHotkey(&btn_ctx, Key_O, KeyModifier_Ctrl);
        UI_NextTooltip(&btn_ctx, S8Lit("open space"));
        if (UI_Button(&btn_ctx, '^', 'O')){
            M_Arena *scratch = OS_GetScratch();
            String8 string = os->DialogueLoadPath(scratch, ext);
            if (string.size > 0){
                String8 serialized = {0};
                os->LoadEntireFile(scratch, string, (void**)&serialized.str, &serialized.size);
                
                STR_Index path = STR_Save(APP_GetStringHash(), string);
                E_Space *new_space = E_GetSpaceByPath(path, 0);
                if (new_space != 0){
                    new_space = APP_ReloadSpace(new_space, serialized);
                }
                else{
                    new_space = E_DeserializeSpace(serialized);
                    E_View *global_view = E_NewView();
                    E_InitGlobalView(global_view, &vars->font, new_space);
                    APP_SetActiveView(global_view);
                }
                if (new_space->init_error){
                    new_space->save_path = 0;
                }
                else{
                    new_space->save_path = path;
                }
                E_SpaceUpdateValidation(new_space);
            }
            OS_ReleaseScratch(scratch);
        }
        
        UI_NextHotkey(&btn_ctx, Key_N, KeyModifier_Ctrl|KeyModifier_Shift);
        UI_NextTooltip(&btn_ctx, S8Lit("new space"));
        if (UI_Button(&btn_ctx, '^', 'N')){
            E_Space *empty_space = E_NewSpace();
            E_View *global_view = E_NewView();
            E_InitGlobalView(global_view, &vars->font, empty_space);
            APP_SetActiveView(global_view);
        }
    }
    
    ////////////////////////////////
    // NOTE(allen): Panel Buttons
    
    {
        Rect rect = MakeRectRanges(panel_x, lister_toggles_y);
        R_Rect(rect, cl_back_unimportant, 1.f);
        
        UI_ButtonCtx btn_ctx = UI_InitButtonCtx(rect, btn_dim, &vars->font, UI_IdV(APP_BtnCtx_ListerOptions));
        btn_ctx.outline_t = outline_t;
        btn_ctx.enable_drop_down = 1;
        btn_ctx.enable_hot_keys = 1;
        
        UI_NextActive(&btn_ctx, vars->lister_flags & APP_ListerFlag_Spaces);
        UI_NextHotkey(&btn_ctx, Key_1, KeyModifier_Ctrl);
        UI_NextTooltip(&btn_ctx, S8Lit("list spaces"));
        if (UI_Button(&btn_ctx, 'L', 's')){
            vars->lister_flags ^= APP_ListerFlag_Spaces;
        }
        
        UI_NextActive(&btn_ctx, vars->lister_flags & APP_ListerFlag_Pages);
        UI_NextHotkey(&btn_ctx, Key_2, KeyModifier_Ctrl);
        UI_NextTooltip(&btn_ctx, S8Lit("list pages"));
        if (UI_Button(&btn_ctx, 'L', 'p')){
            vars->lister_flags ^= APP_ListerFlag_Pages;
        }
        
        UI_NextActive(&btn_ctx, vars->lister_flags & APP_ListerFlag_Invalids);
        UI_NextHotkey(&btn_ctx, Key_3, KeyModifier_Ctrl);
        UI_NextTooltip(&btn_ctx, S8Lit("list invalid definitions"));
        if (UI_Button(&btn_ctx, 'L', 'e')){
            vars->lister_flags ^= APP_ListerFlag_Invalids;
        }
        
        UI_NextActive(&btn_ctx, vars->lister_flags & APP_ListerFlag_Definitions);
        UI_NextHotkey(&btn_ctx, Key_4, KeyModifier_Ctrl);
        UI_NextTooltip(&btn_ctx, S8Lit("list definitions"));
        if (UI_Button(&btn_ctx, 'L', 'd')){
            vars->lister_flags ^= APP_ListerFlag_Definitions;
        }
        
        UI_NextActive(&btn_ctx, vars->lister_flags & APP_ListerFlag_Tests);
        UI_NextHotkey(&btn_ctx, Key_5, KeyModifier_Ctrl);
        UI_NextTooltip(&btn_ctx, S8Lit("list tests"));
        if (UI_Button(&btn_ctx, 'L', 't')){
            vars->lister_flags ^= APP_ListerFlag_Tests;
        }
        
    }
    
    ////////////////////////////////
    // NOTE(allen): Panel Text Field
    
    {
        Rect rect = MakeRectRanges(panel_x, filter_field_y);
        Rect inner = RectShrink(rect, outline_t);
        
        UI_ColorProfile *cl = UI_ColorsDefault();
        UI_ActionLevel actlvl = UI_ActionLevel_None;
        if (UI_MouseInRect(rect)){
            actlvl = UI_ActionLevel_Hover;
            OS_Event *event = 0;
            if (UI_TryGetLeftClick(&event)){
                APP_SetActiveEditor(&vars->panel_filter);
                E_EditorHandleMousePressEvent(&vars->panel_filter, event, inner.p0);
            }
        }
        else if (vars->active_editor == &vars->panel_filter){
            actlvl = UI_ActionLevel_Active;
        }
        
        R_Rect(rect, cl->back[actlvl], 1.f);
        R_RectOutline(rect, outline_t, cl->outline[actlvl], 1.f);
        
        Rect clip_restore = R_PushClip(inner);
        
        E_EditorUpdateLayout(&vars->panel_filter, &vars->font);
        E_EditorRender(&vars->panel_filter, &vars->font, inner.p0);
        
        if (vars->active_editor != &vars->panel_filter && vars->panel_filter.buffer->count == 0){
            R_String(inner.p0, 1.f, S8Lit("filter"), cl->front[0], 0.15f);
        }
        
        R_SetClip(clip_restore);
    }
    
    ////////////////////////////////
    // NOTE(allen): Panel Definitions List
    
    {
        M_Arena *scratch = OS_GetScratch();
        
        STR_Hash *string_hash = APP_GetStringHash();
        Rect rect = MakeRectRanges(panel_x, lister_y);
        R_Rect(rect, cl_back_unimportant, 1.f);
        
        Rect clip_restore = R_PushClip(rect);
        Rect scrolled_rect = rect;
        scrolled_rect.y0 -= vars->panel_scroll_y;
        
        char split_chars[] = " *";
        String8 filter_key = {0};
        if (vars->panel_filter_buffer.count > 0){
            filter_key = STR_Read(string_hash, vars->panel_filter_memory.string);
        }
        String8_List filter_pattern = StringSplit(scratch, filter_key, (u8*)split_chars, ArrayCount(split_chars));
        StringListRemoveEmpties(&filter_pattern);
        
        UI_ButtonCtx btn_ctx = UI_InitButtonCtx(scrolled_rect, v2(RangeSize(panel_x), bar_h), &vars->font, UI_IdV(APP_BtnCtx_Lister));
        btn_ctx.outline_t = outline_t;
        
        UI_ColorProfile *cl_prof[2];
        cl_prof[0] = UI_ColorsProblem();
        cl_prof[1] = UI_ColorsDefault();
        
        // NOTE(allen): Spaces
        if (vars->lister_flags & APP_ListerFlag_Spaces){
            UI_NextCondition(&btn_ctx, 0);
            UI_ButtonLabel(&btn_ctx, S8Lit("<Spaces>"));
            
            E_Space *first[2];
            first[0] = vars->first_invalid_space;
            first[1] = vars->first_space;
            
            for (u64 i = 0; i < 2; i += 1){
                UI_SetColorProfile(&btn_ctx, cl_prof[i]);
                for (E_Space *space = first[i];
                     space != 0;
                     space = space->next){
                    String8 path = STR_Read(string_hash, space->save_path);
                    String8 name = STR_NameFromPath(path);
                    if (APP_FuzzyMatch(name, &filter_pattern)){
                        if (UI_ButtonLabel(&btn_ctx, name)){
                            E_View *view = APP_GetGlobalView(space);
                            if (view == 0){
                                view = E_NewView();
                                E_InitGlobalView(view, &vars->font, space);
                            }
                            APP_SignalViewChange(view);
                        }
                    }
                }
            }
        }
        
        // NOTE(allen): Various types of definitions
        E_Space *space = APP_GetActiveSpace();
        if (space != 0){
            typedef struct APP_DefinitionGroup APP_DefinitionGroup;
            struct APP_DefinitionGroup{
                char *label;
                APP_ListerFlags lister_flag;
                E_DefinitionFlags filter_positive;
                E_DefinitionFlags filter_negative;
                UI_ColorProfile *cl;
            };
            
            APP_DefinitionGroup groups[4] = {
                {"<Pages>"      , APP_ListerFlag_Pages      ,
                    E_DefinitionFlag_Page, 0,
                    UI_ColorsPage(), },
                
                {"<Invalids>"   , APP_ListerFlag_Invalids   ,
                    E_DefinitionFlag_Invalid, E_DefinitionFlag_Page|E_DefinitionFlag_Test,
                    UI_ColorsProblem(), },
                
                {"<Definitions>", APP_ListerFlag_Definitions,
                    0, E_DefinitionFlag_Page|E_DefinitionFlag_Invalid|E_DefinitionFlag_Test,
                    UI_ColorsDefault(), },
                
                {"<Tests>"      , APP_ListerFlag_Tests      ,
                    E_DefinitionFlag_Test, E_DefinitionFlag_Page,
                    UI_ColorsTest(), },
            };
            
            APP_DefinitionGroup *group = groups;
            for (u64 j = 0; j < ArrayCount(groups); j += 1, group += 1){
                if (vars->lister_flags & group->lister_flag){
                    Assert((group->filter_positive & group->filter_negative) == 0);
                    
                    E_DefinitionFlags filter_positive = group->filter_positive;
                    E_DefinitionFlags filter_negative = group->filter_negative;
                    
                    UI_SetColorProfile(&btn_ctx, group->cl);
                    UI_NextCondition(&btn_ctx, 0);
                    UI_ButtonLabel(&btn_ctx, String8FromCString(group->label));
                    
                    for (E_Definition *node = space->first_definition_ordered;
                         node != 0;
                         node = node->ordered_next){
                        if ((node->flags & filter_positive) != filter_positive){
                            continue;
                        }
                        if ((node->flags & filter_negative) != 0){
                            continue;
                        }
                        
                        String8 name = E_DefinitionName(node);
                        if (!APP_FuzzyMatch(name, &filter_pattern)){
                            continue;
                        }
                        
                        if (name.size == 0){
                            name = S8Lit("<unnamed>");
                        }
                        if (UI_ButtonLabel(&btn_ctx, name)){
                            switch (group->lister_flag){
                                case APP_ListerFlag_Pages:
                                {
                                    OP_ViewPageFromDefinition(node);
                                }break;
                                
                                default:
                                {
                                    OP_LookAtDefinition(node);
                                }break;
                            }
                        }
                    }
                }
            }
            
            // NOTE(allen): Scrolling
            if (UI_MouseInRect(rect)){
                v2 delta = {0};
                if (UI_TryEatScroll(&delta)){
                    vars->panel_scroll_y -= delta.y;
                }
            }
            
            f32 h = UI_ButtonCtxGetTraversedY(&btn_ctx);;
            f32 max_scroll = h - RangeSize(lister_y)*0.5f;
            max_scroll = ClampBot(0.f, max_scroll);
            vars->panel_scroll_y = Clamp(0.f, vars->panel_scroll_y, max_scroll);
            
        }
        
        R_SetClip(clip_restore);
        OS_ReleaseScratch(scratch);
    }
    
    ////////////////////////////////
    // NOTE(allen): View Buttons
    
    {
        Rect rect = MakeRectRanges(code_x, view_buttons_y);
        R_Rect(rect, cl_back_unimportant, 1.f);
        
        UI_ButtonCtx btn_ctx = UI_InitButtonCtx(rect, btn_dim, &vars->font, UI_IdV(APP_BtnCtx_ViewButtons));
        btn_ctx.outline_t = outline_t;
        btn_ctx.enable_drop_down = 1;
        btn_ctx.enable_hot_keys = 1;
        
        UI_SetColorProfile(&btn_ctx, UI_ColorsProblem());
        UI_SetRedText(&btn_ctx.cl);
        
        UI_NextActive(&btn_ctx, vars->active_view != 0);
        UI_NextHotkey(&btn_ctx, Key_K, KeyModifier_Ctrl|KeyModifier_Shift);
        UI_NextTooltip(&btn_ctx, S8Lit("close current tab"));
        if (UI_Button(&btn_ctx, 'X', 't')){
            E_View *view = vars->active_view;
            E_View *switch_to_view = view->next;
            if (switch_to_view == 0){
                switch_to_view = view->prev;
            }
            E_DeleteView(view);
            
            vars->active_view = 0;
            if (switch_to_view != 0){
                APP_SignalViewChange(switch_to_view);
            }
        }
        
        UI_NextActive(&btn_ctx, APP_GetActiveSpace() != 0);
        UI_NextTooltip(&btn_ctx, S8Lit("close current splink file"));
        if (UI_Button(&btn_ctx, 'X', 'f')){
            E_Space *space = APP_GetActiveSpace();
            APP_CloseSpace(space);
        }
        
    }
    
    ////////////////////////////////
    // NOTE(allen): Page Tabs
    
    {
        E_View *view = vars->active_view;
        if (UI_TryEatKeyPressModified(Key_Tab, KeyModifier_Ctrl)){
            APP_SignalViewChange((view != 0 && view->next != 0)?
                                 view->next:vars->first_view);
        }
        if (UI_TryEatKeyPressModified(Key_Tab, KeyModifier_Ctrl|KeyModifier_Shift)){
            APP_SignalViewChange((view != 0 && view->prev != 0)?
                                 view->prev:vars->last_view);
        }
    }
    
    {
        Rect rect = MakeRectRanges(code_x, tabs_y);
        R_Rect(rect, cl_back_unimportant, 1.f);
        
        f32 w = R_StringDimWithFont(&vars->font, 1.f, S8Lit(" ")).x*10.f;
        UI_ButtonCtx btn_ctx = UI_InitButtonCtx(rect, v2(w, RangeSize(tabs_y)), &vars->font, UI_IdV(APP_BtnCtx_Tabs));
        btn_ctx.outline_t = outline_t;
        btn_ctx.text_scale = 0.70f;
        btn_ctx.enable_flexible_x_advance = 1;
        
        UI_SetColorProfile(&btn_ctx, UI_ColorsTabs());
        
        for (E_View *view = vars->first_view;
             view != 0;
             view = view->next){
            if (view == vars->active_view){
                UI_NextActive(&btn_ctx, 1);
            }
            String8 name = E_ViewGetName(view);
            if (UI_ButtonLabel(&btn_ctx, name)){
                APP_SignalViewChange(view);
            }
        }
    }
    
    ////////////////////////////////
    // NOTE(allen): Update token editor layouts
    
    if (vars->active_view != 0){
        MemoryZeroArray(vars->neighbor_editors);
        E_EditorState *prev_editor = 0;
        b32 save_next = 0;
        E_EditorState *active_editor = vars->active_editor;
        
        for (E_Tile *tile = vars->active_view->first_tile;
             tile != 0;
             tile = tile->next){
            E_TileUpdateLayout(tile, &vars->font);
            
            if (active_editor != 0){
                E_EditorState *editor = tile->editors;
                for (u64 i = 0; i < ArrayCount(tile->editors); i += 1, editor += 1){
                    if (save_next){
                        vars->neighbor_editors[Side_Max] = editor;
                        save_next = 0;
                        active_editor = 0;
                        break;
                    }
                    if (active_editor == editor){
                        save_next = 1;
                        vars->neighbor_editors[Side_Min] = prev_editor;
                    }
                    prev_editor = editor;
                }
            }
        }
    }
    
    ////////////////////////////////
    // NOTE(allen): Handle events
    
    {
        OS_Event *event = 0;
        for (;OS_GetNextEvent(&event);){
            switch (event->type){
                case OS_EventType_CharacterInput:
                {
                    // NOTE(allen): Does the active editor use this key?
                    if (vars->active_editor != 0 && E_EditorHandleCharacterEvent(vars->active_editor, event)){
                        goto finished_event;
                    }
                }break;
                
                case OS_EventType_KeyPress:
                {
                    // NOTE(allen): Universal rules
                    if (event->key == Key_Esc){
                        vars->active_editor = 0;
                        OS_EatEvent(event);
                        goto finished_event;
                    }
                    
                    
                    if (vars->active_editor != 0){
                        // NOTE(allen): Editor override rules
                        if (((event->modifiers & KeyModifier_Shift) != 0)){
                            if (event->key == Key_Enter){
                                APP_TopLevelNavigate(Dimension_Y, Side_Max);
                                OS_EatEvent(event);
                                goto finished_event;
                            }
                        }
                        
                        // NOTE(allen): Does the active editor use this key?
                        if (E_EditorHandleKeyPressEvent(vars->active_editor, event)){
                            OS_EatEvent(event);
                            goto finished_event;
                        }
                        
                        // NOTE(allen): Navigate between active editors
                        if (event->key == Key_Up || event->key == Key_Down){
                            Side side = Side_Min;
                            if (event->key == Key_Down){
                                side = Side_Max;
                            }
                            APP_TopLevelNavigate(Dimension_Y, side);
                            
                            OS_EatEvent(event);
                            goto finished_event;
                        }
                    }
                }break;
                
                case OS_EventType_MouseMove:
                {
                    vars->mouse_p = event->position;
                }break;
            }
            
            finished_event:;
        }
    }
    
    ////////////////////////////////
    // NOTE(allen): Check for change notifications
    
    if (vars->active_view != 0){
        M_Arena *scratch = OS_GetScratch();
        M_Temp restore = M_BeginTemp(scratch);
        E_View *view = vars->active_view;
        
        for (E_Space *space = vars->first_space_ordered;
             space != 0;
             space = space->ordered_next){
            for (E_Definition *node = space->first_definition_ordered;
                 node != 0;
                 node = node->ordered_next){
                // NOTE(allen): Check for a change
                b32 has_change = 0;
                {
                    E_TokenBuffer *buffer = node->buffers;
                    for (u64 i = 0; i < ArrayCount(node->buffers); i += 1, buffer += 1){
                        if (E_TokenBufferHasChange(buffer)){
                            has_change = 1;
                        }
                    }
                }
                
                if (has_change){
                    // NOTE(allen): State Update
                    if (E_TokenBufferHasChange(&node->name)){
                        E_DefinitionUpdateValidation(node);
                    }
                    
                    // NOTE(allen): Collect tiles that view this definition
                    E_Tile **tile_ptrs = PushArray(scratch, E_Tile*, 0);
                    for (E_Tile *tile = view->first_tile;
                         tile != 0;
                         tile = tile->next){
                        if (tile->definition == node){
                            E_Tile **new_tile_ptr = PushArray(scratch, E_Tile*, 1);
                            *new_tile_ptr = tile;
                        }
                    }
                    u64 tile_count = (PushArray(scratch, E_Tile*, 0) - tile_ptrs);
                    
                    // NOTE(allen): Update Buffers
                    u64 *cursor_array = PushArray(scratch, u64, tile_count);
                    E_TokenBuffer *buffer = node->buffers;
                    for (u64 i = 0; i < ArrayCount(node->buffers); i += 1, buffer += 1){
                        if (E_TokenBufferHasChange(buffer)){
                            for (u64 j = 0; j < tile_count; j += 1){
                                cursor_array[j] = tile_ptrs[j]->editors[i].cursor.pos;
                            }
                            E_TokenBufferScrub(buffer, cursor_array, tile_count);
                            E_TokenBufferChangeHandled(buffer);
                            
                            // NOTE(allen): Update Editors
                            for (u64 j = 0; j < tile_count; j += 1){
                                E_EditorState *editor = &tile_ptrs[j]->editors[i];
                                E_EditorUpdateLayout(editor, tile_ptrs[j]->font);
                                editor->mark = editor->cursor;
                                E_EditorUpdatePreferredXToCursor(editor);
                            }
                        }
                    }
                    
                    // NOTE(allen): Compute Update
                    E_DefinitionUpdateCompute(node);
                    
                    // NOTE(allen): Space Dirty
                    space->dirty = 1;
                    M_EndTemp(restore);
                }
            }
            
        }
        
        OS_ReleaseScratch(scratch);
    }
    
    ////////////////////////////////
    // NOTE(allen): Update space validations from available identifiers
    
    for (u64 lim = 0; vars->identifier_available_count > 0 && lim < 2; lim += 1){
        u64 count = vars->identifier_available_count;
        vars->identifier_available_count = 0;
        
        if (count <= ArrayCount(vars->identifier_available)){
            // NOTE(allen): Grab a copy of available so that E_SpaceUpdateValidation can
            // signal more available identifiers.
            STR_Index available[ArrayCount(vars->identifier_available)];
            MemoryCopy(available, vars->identifier_available, sizeof(*available)*count);
            
            for (u64 i = 0; i < count; i += 1){
                STR_Index check = available[i];
                E_Space *space = E_GetInvalidSpace(check, 0);
                if (space != 0){
                    E_SpaceUpdateValidation(space);
                }
            }
        }
        else{
            for (E_Space *space = vars->first_invalid_space;
                 space != 0;
                 space = space->next){
                E_SpaceUpdateValidation(space);
            }
        }
    }
    
    ////////////////////////////////
    // NOTE(allen): Update definition validations from available identifiers
    
    for (E_Space *space = vars->first_space_ordered;
         space != 0;
         space = space->ordered_next){
        for (u64 lim = 0; space->identifier_available_count > 0 && lim < 2; lim += 1){
            u64 count = space->identifier_available_count;
            space->identifier_available_count = 0;
            
            if (count <= ArrayCount(space->identifier_available)){
                // NOTE(allen): Grab a copy of available so that E_DefinitionUpdateValidation can
                // signal more available identifiers.
                STR_Index available[ArrayCount(space->identifier_available)];
                MemoryCopy(available, space->identifier_available, sizeof(*available)*count);
                
                for (u64 i = 0; i < count; i += 1){
                    STR_Index check = available[i];
                    E_Definition *definition = E_GetInvalidDefinition(space, check, 0);
                    if (definition != 0){
                        E_DefinitionUpdateValidation(definition);
                    }
                }
            }
            else{
                for (E_Definition *node = space->first_invalid_definition;
                     node != 0;
                     node = node->next){
                    E_DefinitionUpdateValidation(node);
                }
            }
        }
    }
    
    ////////////////////////////////
    // NOTE(allen): Init defers & Populate global defines
    
    {
        STR_Hash *string_hash = APP_GetStringHash();
        M_Arena *scratch = OS_GetScratch();
        C_CellBucket *bucket = &vars->global_defines_bucket;
        C_ClearBucket(bucket);
        
        C_Cell *spaces_env = vars->statics.env;
        for (E_Space *space = vars->first_space_ordered;
             space != 0;
             space = space->ordered_next){
            if (!space->invalid){
                String8 name = E_SpaceName(space);
                String8 space_identifier = PushStringCat(scratch, S8Lit("$"), name);
                STR_Index space_name = STR_Save(string_hash, space_identifier);
                C_Cell *space_cell = C_NewSpaceCell(bucket, space);
                spaces_env = C_ExtendEnvironment(bucket, space_name, space_cell, spaces_env);
            }
        }
        vars->spaces_env = spaces_env;
        
        for (E_Space *space = vars->first_space_ordered;
             space != 0;
             space = space->ordered_next){
            C_Cell *env = spaces_env;
            C_Cell *id_env = vars->statics.Nil;
            C_Cell *id_list = 0;
            C_ListBuilder builder = C_InitListBuilder(id_list);
            for (E_Definition *node = space->first_definition_ordered;
                 node != 0;
                 node = node->ordered_next){
                MemoryZeroStruct(&node->deferred);
                node->deferred.user_ptr = node;
                C_Cell *defer_cell = C_NewDeferredCell(bucket, &node->deferred, node->body_cell);
                C_Cell *id_cell = C_NewIdCell(bucket, node->id);
                
                if (!(node->flags & E_DefinitionFlag_Invalid) && E_DefinitionCanEval(node)){
                    STR_Index name = C_GetIdentifierFromCell(node->name_cell);
                    env = C_ExtendEnvironment(bucket, name, defer_cell, env);
                }
                id_env = C_ExtendEnvironment(bucket, node->id, defer_cell, id_env);
                C_ListBuilderPush(bucket, builder, id_cell);
            }
            C_ListBuilderTerminate(&vars->statics, builder);
            
            space->defines_env = env;
            space->id_env = id_env;
            space->id_list = id_list;
            
            for (E_Definition *node = space->first_definition_ordered;
                 node != 0;
                 node = node->ordered_next){
                node->deferred.env = env;
            }
        }
        
        OS_ReleaseScratch(scratch);
    }
    
    ////////////////////////////////
    // NOTE(allen): Render and update editor
    
    {
        C_ClearBucket(&vars->eval_bucket);
        
        Rect rect = MakeRectRanges(code_x, code_y);
        R_Rect(rect, cl_back_editor, 1.f);
        
        E_Tile *snap_to_tile = vars->snap_to_tile;
        b32 found_snap_y = 0;
        f32 snap_y = 0;
        
        E_View *view = vars->active_view;
        if (view != 0){
            Rect restore_clip = R_PushClip(rect);
            
            // NOTE(allen): Height pass
            E_Tile *first_tile_crossover = 0;
            f32 first_tile_crossover_y = 0;
            
            f32 top = rect.y0 - view->scroll_y;
            f32 y = top;
            for (E_Tile *tile = view->first_tile;
                 tile != 0;
                 tile = tile->next){
                f32 h = E_TileUIGetHeight(view, tile, y, code_x);
                if (tile == snap_to_tile){
                    found_snap_y = 1;
                    snap_y = y + h*0.5f;
                }
                y += h;
                if (first_tile_crossover == 0 && y > rect.y0){
                    first_tile_crossover = tile;
                    first_tile_crossover_y = y - h;
                }
            }
            
            f32 h = y - top;
            
            // NOTE(allen): Scrolling
            if (UI_MouseInRect(rect)){
                v2 delta = {0};
                if (UI_TryEatScroll(&delta)){
                    view->scroll_y -= delta.y;
                }
            }
            if (found_snap_y){
                f32 snap_y_document_space = snap_y - top;
                
                // align the snap with 33% of the way down the view
                //  solve for scroll_y in: scroll_y + 0.33*view_h = snap_y
                view->scroll_y = snap_y_document_space - 0.33f*RangeSize(code_y);
            }
            
            f32 max_scroll = h - RangeSize(code_y)*0.5f;
            max_scroll = ClampBot(0.f, max_scroll);
            view->scroll_y = Clamp(0.f, view->scroll_y, max_scroll);
            
            // NOTE(allen): Action pass
            y = first_tile_crossover_y;
            for (E_Tile *tile = first_tile_crossover;
                 tile != 0;
                 tile = tile->next){
                f32 h = E_TileUIUpdateAndRender(view, tile, y, code_x);
                y += h;
                if (y >= rect.y1 + 500.f){
                    break;
                }
            }
            
            R_SetClip(restore_clip);
        }
        
        vars->snap_to_tile = 0;
    }
    
    ////////////////////////////////
    // NOTE(allen): Delayed view change
    
    APP_ApplyViewChange(vars->change_view);
    
    ////////////////////////////////
    // NOTE(allen): Floating Window Post-Update
    
    if (!UI_IdEq(vars->owner_of_floating_window, UI_IdZero())){
        APP_SetMouseLayer(floating_window_mouse_layer);
        if (vars->floating_window_callback != 0){
            R_Rect(MakeRect(0, 0, V2Expand(vars->window_dim)), cl_gray(0.1f), 0.25f);
            APP_FloatingWindowResult result = {0};
            vars->floating_window_callback(vars->floating_window_ptr, &result);
            vars->floating_window_last_frame_rect = result.rect;
        }
        else{
            MemoryZeroStruct(&vars->owner_of_floating_window);
        }
    }
    
    ////////////////////////////////
    // NOTE(allen): Tool Tip
    
    if (vars->tool_tip_string.size > 0){
        f32 padding = 2.f;
        f32 y_skirt[2] = {6.f, 18.f};
        f32 dbl_padding = padding*2.f;
        
        String8 str = vars->tool_tip_string;
        
        R_SelectFont(&vars->font);
        v2 str_dim = R_StringDim(1.f, str);
        v2 str_dim_padded = V2Add(str_dim, v2(dbl_padding, dbl_padding));
        
        f32 y_base = vars->mouse_p.y + y_skirt[Side_Max];
        Range y = MakeRange(y_base, y_base + str_dim_padded.y);
        if (y.max > vars->window_dim.y){
            y_base = vars->mouse_p.y - y_skirt[Side_Min];
            y = MakeRange(y_base, y_base - str_dim_padded.y);
        }
        
        Range x;
        // solve for x in: (x + 0.2*w = mx)
        x.min = vars->mouse_p.x - 0.2f*str_dim_padded.x;
        x.min = ClampBot(0.f, x.min);
        x.max = x.min + str_dim_padded.x;
        
        Rect rect = MakeRectRanges(x, y);
        R_Rect(rect, cl_gray(0.1f), 0.66f);
        
        v2 p = V2Add(rect.p0, v2(padding, padding));
        R_String(p, 1.f, str, cl_white, 0.66f);
    }
    
    ////////////////////////////////
    // NOTE(allen): End the frame
    
    vars->frame_indx += 1;
    
    {
        // NOTE(allen): Why does the layer even keep events from previous frames?
        // Anyway, just clear those out.
        OS_Event *event = 0;
        for (;OS_GetNextEvent(&event);){
            OS_EatEvent(event);
        }
    }
    
    R_End();
    
    AssertIff(vars->active_view == 0, vars->first_view == 0);
    _E_AssertGlobalEngineInvariants();
}

