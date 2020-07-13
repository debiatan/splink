/* date = July 2nd 2020 11:52 pm */

#ifndef APP_H
#define APP_H

////////////////////////////////
// NOTE(allen): Floating Window

typedef struct APP_FloatingWindowResult APP_FloatingWindowResult;
struct APP_FloatingWindowResult{
    Rect rect;
};

typedef void APP_FloatingWindowCallbackType(void *ptr, APP_FloatingWindowResult *result);


////////////////////////////////
// NOTE(allen): Variables

typedef enum{
    APP_MouseLayer_Null,
    APP_MouseLayer_Main,
    APP_MouseLayer_FloatingWindow,
} APP_MouseLayer;

typedef enum{
    APP_BtnCtx_Null,
    APP_BtnCtx_ToolBox,
    APP_BtnCtx_ListerOptions,
    APP_BtnCtx_Lister,
    APP_BtnCtx_ViewButtons,
    APP_BtnCtx_Tabs,
} APP_MouseLayer;

typedef u32 APP_ListerFlags;
enum{
    APP_ListerFlag_Spaces      = (1 << 0),
    APP_ListerFlag_Pages       = (1 << 1),
    APP_ListerFlag_Tests       = (1 << 2),
    APP_ListerFlag_Invalids    = (1 << 3),
    APP_ListerFlag_Definitions = (1 << 4),
};

typedef struct APP_Variables APP_Variables;
struct APP_Variables{
    M_Arena arena_;
    M_Arena *arena;
    
    R_Font font;
    v2 mouse_p;
    v2 window_dim;
    f32 frame_time;
    
    ////////////////////////////////
    // NOTE(allen): Engine State
    
    u64 frame_indx;
    M_Arena frame_arena[2];
    
    STR_Hash string_hash;
    
    STR_Index keyword_table[C_BuiltInIndex_COUNT];
    C_CellMemory cells;
    C_CellBucket static_bucket;
    C_Statics statics;
    C_CellBucket global_defines_bucket;
    C_CellBucket eval_bucket;
    C_Cell *spaces_env;
    
    E_Definition *free_definition;
    E_Space *free_space;
    
    E_Space *first_space_ordered;
    E_Space *last_space_ordered;
    E_Space *first_space;
    E_Space *last_space;
    E_Space *first_invalid_space;
    E_Space *last_invalid_space;
    
    u64 identifier_available_count;
    STR_Index identifier_available[8];
    
    E_Tile *free_tile;
    
    E_View *free_view;
    E_View *first_view;
    E_View *last_view;
    
    E_View *active_view;
    E_EditorState *active_editor;
    E_Tile *active_tile;
    
    E_View *change_view;
    E_Tile *snap_to_tile;
    
    
    ////////////////////////////////
    // NOTE(allen): UI
    
    UI_Id active_mouse_layer;
    UI_Id current_mouse_layer;
    
    UI_Id owner_of_floating_window;
    void *floating_window_ptr;
    APP_FloatingWindowCallbackType *floating_window_callback;
    Rect floating_window_last_frame_rect;
    
    UI_Id last_frame_owner_of_floating_window;
    void *last_frame_floating_window_ptr;
    
    String8 tool_tip_string;
    
    APP_ListerFlags lister_flags;
    C_Token panel_filter_memory;
    E_TokenBuffer panel_filter_buffer;
    E_EditorState panel_filter;
    f32 panel_scroll_y;
    
    ////////////////////////////////
    // NOTE(allen): Frame Data
    
    E_EditorState *neighbor_editors[2];
};

#endif //APP_H
