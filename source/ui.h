/* date = July 7th 2020 4:09 pm */

#ifndef UI_H
#define UI_H

typedef enum{
    UI_ActionLevel_None,
    UI_ActionLevel_Active,
    UI_ActionLevel_Hover,
    UI_ActionLevel_Flash,
    UI_ActionLevel_COUNT,
} UI_ActionLevel;

typedef struct UI_ColorProfile UI_ColorProfile;
struct UI_ColorProfile{
    v3 back[UI_ActionLevel_COUNT];
    v3 outline[UI_ActionLevel_COUNT];
    v3 front[UI_ActionLevel_COUNT];
};

typedef union UI_Id UI_Id;
union UI_Id{
    struct{
        u64 v1;
    };
    struct{
        void *p1;
    };
    u64 v[1];
    void *p[1];
};

typedef struct UI_ButtonCtxParamRestore UI_ButtonCtxParamRestore;
struct UI_ButtonCtxParamRestore{
    b32 condition;
    b32 active;
    b32 has_hot_key;
    String8 tool_tip;
};

typedef struct UI_ButtonCtx UI_ButtonCtx;
struct UI_ButtonCtx{
    // NOTE(allen): State
    R_Font *font;
    UI_Id id;
    UI_Id button_id;
    Rect rect;
    v2 p;
    v2 btn_dim;
    
    UI_ButtonCtxParamRestore restore;
    b8 do_drop_down;
    b8 did_tool_tip;
    
    b8 has_room;
    
    // NOTE(allen): Extended parameters
    b8 condition;
    b8 active;
    
    b8 has_hot_key;
    Key hot_key;
    KeyModifiers hot_key_mods;
    String8 tool_tip;
    f32 this_button_x_advance;
    
    // NOTE(allen): Settings
    UI_ColorProfile cl;
    f32 outline_t;
    f32 text_scale;
    
    b8 enable_drop_down;
    b8 enable_hot_keys;
    b8 enable_flexible_x_advance;
};

typedef enum{
    UI_ButtonKind_Icon,
    UI_ButtonKind_Label,
} UI_ButtonKind;

typedef struct UI_ButtonRecord UI_ButtonRecord;
struct UI_ButtonRecord{
    UI_ButtonRecord *next;
    String8 string;
    UI_ButtonKind kind;
    u8 major;
    u8 minor;
    b8 condition;
    b8 has_hot_key;
    Key hot_key;
    KeyModifiers hot_key_mods;
    f32 outline_t;
    f32 text_scale;
    UI_ColorProfile cl;
    String8 tool_tip;
    UI_Id id;
};

typedef struct UI_ButtonDropdown UI_ButtonDropdown;
struct UI_ButtonDropdown{
    UI_ButtonRecord *first;
    UI_ButtonRecord *last;
    u64 count;
    
    R_Font *font;
    v2 btn_dim;
    Rect source_rect;
    
    UI_ButtonRecord *activated;
};

#endif //UI_H
