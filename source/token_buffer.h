/* date = July 6th 2020 6:35 am */

#ifndef TOKEN_BUFFER_H
#define TOKEN_BUFFER_H

typedef struct E_TokenBuffer E_TokenBuffer;
struct E_TokenBuffer{
    M_Arena arena;
    C_Token *tokens;
    u64 count;
    u64 max;
    b32 dirty;
    b32 text_field_mode;
};

typedef u32 E_RuneKind;
enum{
    E_RuneKind_Error,
    E_RuneKind_Space,
    E_RuneKind_LineStart,
    E_RuneKind_Symbol,
    E_RuneKind_Label,
    E_RuneKind_Comment,
};

typedef struct E_Rune E_Rune;
struct E_Rune{
    u64 cursor_pos;
    E_Rune *next;
    E_Rune *prev;
    String8 string;
    Rect rect;
    v4 color;
    E_RuneKind kind;
    f32 text_scale;
};

typedef struct E_RuneLayout E_RuneLayout;
struct E_RuneLayout{
    E_Rune *vals;
    u64 count;
    E_Rune *first_line;
    E_Rune *last_line;
    E_Rune *first_error;
    E_Rune *last_error;
    v2 dim;
};

typedef struct E_LayoutIndent E_LayoutIndent;
struct E_LayoutIndent{
    E_LayoutIndent *next;
    f32 pre_indent;
    f32 post_indent;
};

typedef struct E_LayoutCtx E_LayoutCtx;
struct E_LayoutCtx{
    // NOTE(allen): state
    M_Arena *arena;
    M_Arena *temp_arena;
    E_RuneLayout *layout;
    u64 cursor_pos;
    v2 p;
    
    E_LayoutIndent *free_indent;
    E_LayoutIndent *indent;
    
    // NOTE(allen): settings
    R_Font *font;
    f32 text_scale;
    f32 initial_padding;
    v2 space_dim;
    v4 cl_whitespace;
    v4 cl_error;
};

typedef struct E_Cursor E_Cursor;
struct E_Cursor{
    u64 pos;
    u64 sub_pos;
};

typedef enum
{
    E_RangeKind_SingleTokenTextRange,
    E_RangeKind_MultiTokenRange,
} E_RangeKind;

typedef struct E_Range E_Range;
struct E_Range
{
    E_RangeKind kind;
    Rangeu range;
    u64 token;
};

typedef enum{
    E_EditorFieldMode_None,
    E_EditorFieldMode_AnyString,
    E_EditorFieldMode_CodeIdentifier,
} E_EditorFieldMode;

typedef struct E_EditorState E_EditorState;
struct E_EditorState{
    E_TokenBuffer *buffer;
    R_Font *font;
    E_RuneLayout runes;
    E_Cursor cursor;
    E_Cursor mark;
    f32 preferred_x;
    E_EditorFieldMode text_field_mode;
};

#endif //TOKEN_BUFFER_H
