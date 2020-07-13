////////////////////////////////
// NOTE(allen): App core

global APP_Variables *vars = 0;

internal M_Arena*
APP_GetFrameArena(void){
    return(&vars->frame_arena[vars->frame_indx&1]);
}

internal STR_Hash*
APP_GetStringHash(void){
    return(&vars->string_hash);
}

internal C_CellMemory*
APP_GetCellMemory(void){
    return(&vars->cells);
}

internal E_EditorState*
APP_GetActiveEditor(void){
    return(vars->active_editor);
}

internal E_Tile*
APP_GetActiveTile(void){
    return(vars->active_tile);
}

internal void
APP_SetActiveView(E_View *view){
    if (vars->active_view != view){
        vars->active_tile = 0;
        vars->active_editor = 0;
    }
    vars->active_view = view;
    Assert(vars->active_view != 0);
}

internal void
APP_SetActiveEditor(E_EditorState *editor){
    if (vars->active_editor != editor){
        vars->active_tile = 0;
    }
    vars->active_editor = editor;
}

internal void
APP_SetActiveTile(E_Tile *tile){
    if (vars->active_tile != tile){
        b32 editor_in_tile = 0;
        if (tile != 0){
            for (u64 i = 0; i < ArrayCount(tile->editors); i += 1){
                if (vars->active_editor == &tile->editors[i]){
                    editor_in_tile = 1;
                    break;
                }
            }
        }
        if (!editor_in_tile){
            vars->active_editor = 0;
        }
    }
    vars->active_tile = tile;
}

internal C_Statics*
APP_GetStatics(void){
    return(&vars->statics);
}

internal C_CellBucket*
APP_GetEvalBucket(void){
    return(&vars->eval_bucket);
}

internal void
APP_SignalSnapToTile(E_Tile *tile){
    vars->snap_to_tile = tile;
}

internal void
APP_SignalViewChange(E_View *view){
    if (view != 0){
        vars->change_view = view;
    }
}

internal E_View*
APP_GetGlobalView(E_Space *space){
    E_View *result = 0;
    for (E_View *view = vars->first_view;
         view != 0;
         view = view->next){
        if (view->kind == E_ViewKind_Global && view->space == space){
            result = view;
            break;
        }
    }
    return(result);
}

internal E_View*
APP_GetPageView(E_Definition *definition){
    E_View *result = 0;
    for (E_View *view = vars->first_view;
         view != 0;
         view = view->next){
        if (view->kind == E_ViewKind_Page && view->definition == definition){
            result = view;
            break;
        }
    }
    return(result);
}

internal E_Space*
APP_GetActiveSpace(void){
    E_Space *result = 0;
    E_View *view = vars->active_view;
    if (view != 0){
        result = view->space;
        Assert(result != 0);
    }
    return(result);
}

internal void
APP_SignalSpaceNameAvailable(STR_Index name){
    if (vars->identifier_available_count < ArrayCount(vars->identifier_available)){
        vars->identifier_available[vars->identifier_available_count] = name;
    }
    vars->identifier_available_count += 1;
}

internal UI_Id
APP_OwnerOfFloatingWindow(void){
    return(vars->owner_of_floating_window);
}

internal void
APP_TakeOwnershipOfFloatingWindow(UI_Id id){
    vars->owner_of_floating_window = id;
}

internal void
APP_ZeroOwnershipOfFloatingWindow(void){
    MemoryZeroStruct(&vars->owner_of_floating_window);
}

internal void*
APP_GetFloatingWindowPtr(void){
    return(vars->floating_window_ptr);
}

internal void*
APP_GetLastFrameFloatingWindowPtr(void){
    void *result = 0;
    if (UI_IdEq(vars->last_frame_owner_of_floating_window, vars->owner_of_floating_window)){
        result = vars->last_frame_floating_window_ptr;
    }
    return(result);
}

internal void
APP_SetFloatingWindowPtrAndCallback(void *ptr, APP_FloatingWindowCallbackType *callback){
    vars->floating_window_ptr = ptr;
    vars->floating_window_callback = callback;
}

internal b32
APP_MouseIsActive(void){
    return(UI_IdEq(vars->active_mouse_layer, vars->current_mouse_layer));
}

internal void
APP_SetMouseLayer(UI_Id id){
    vars->current_mouse_layer = id;
}

internal void
APP_SetToolTip(String8 string){
    vars->tool_tip_string = string;
}
