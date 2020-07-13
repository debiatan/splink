/* date = July 7th 2020 7:47 am */

#ifndef DEFINITION_H
#define DEFINITION_H

typedef u32 E_DefinitionFlags;
enum{
    E_DefinitionFlag_Invalid = (1 << 0),
    E_DefinitionFlag_Page = (1 << 1),
    E_DefinitionFlag_Test = (1 << 2),
};

typedef struct E_Definition E_Definition;
struct E_Definition{
    // NOTE(allen): State
    
    E_Definition *ordered_next;
    E_Definition *ordered_prev;
    E_Definition *next;
    E_Definition *prev;
    struct E_Space *space;
    
    E_DefinitionFlags flags;
    STR_Index last_name;
    C_Id id;
    b32 delete_me;
    
    // NOTE(allen): Contents
    
    C_Token name_token_buffer;
    union{
        struct{
            E_TokenBuffer name;
            E_TokenBuffer body;
        };
        E_TokenBuffer buffers[2];
    };
    
    // NOTE(allen): Compute
    
    b32 invalid_name_field;
    C_CellBucket bucket;
    u64 error_count;
    C_ParseError errors[20];
    union{
        struct{
            C_Cell *name_cell;
            C_Cell *body_cell;
        };
        C_Cell *cells[2];
    };
    
    C_Deferred deferred;
};

typedef struct E_Space E_Space;
struct E_Space{
    E_Space *ordered_next;
    E_Space *ordered_prev;
    E_Space *next;
    E_Space *prev;
    
    b32 invalid;
    STR_Index last_name;
    
    C_Cell *defines_env;
    C_Cell *id_env;
    C_Cell *id_list;
    u64 id_counter;
    
    E_Definition *first_definition;
    E_Definition *last_definition;
    E_Definition *first_invalid_definition;
    E_Definition *last_invalid_definition;
    E_Definition *first_definition_ordered;
    E_Definition *last_definition_ordered;
    
    b32 delete_signal;
    
    b32 init_error;
    b32 dirty;
    f32 last_save_time;
    STR_Index save_path;
    
    u64 identifier_available_count;
    STR_Index identifier_available[8];
};

////////////////////////////////

typedef struct E_Tile E_Tile;
struct E_Tile{
    E_Tile *next;
    E_Tile *prev;
    
    struct E_TileCache *tile_cache;
    b32 free_me;
    
    E_Definition *definition;
    union{
        struct{
            E_EditorState name;
            E_EditorState body;
        };
        E_EditorState editors[2];
    };
    
    R_Font *font;
};

typedef struct E_ViewCallbackIn E_ViewCallbackIn;
struct E_ViewCallbackIn{
    E_Definition *definition;
};

typedef union E_ViewCallbackOut E_ViewCallbackOut;
union E_ViewCallbackOut{
    E_Definition *definition;
    String8 name;
};

#define VIEW_CALLBACK_SIG(name) void name(struct E_View *view, E_ViewCallbackIn *in, E_ViewCallbackOut *out)
typedef VIEW_CALLBACK_SIG(E_ViewCallback);

/* NOTE(allen): View v-table
** derive_from_source (void) (void)
** create_new_definition (void) (out->definition)
** look_at_definition (in->definition) (void)
** cleanup (void) (void)
** shutdown (void) (void)
** get_name (void) (out->name)
*/

typedef enum{
    E_ViewKind_Null,
    E_ViewKind_Global,
    E_ViewKind_Page,
} E_ViewKind;

typedef struct E_View E_View;
struct E_View{
    E_ViewKind kind;
    
    E_View *prev;
    E_View *next;
    
    // NOTE(allen): v-table
    E_ViewCallback *derive_from_source;
    E_ViewCallback *create_new_definition;
    E_ViewCallback *look_at_definition;
    E_ViewCallback *cleanup;
    E_ViewCallback *shutdown;
    E_ViewCallback *get_name;
    
    // NOTE(allen): Tiles
    R_Font *font;
    E_Tile *first_tile;
    E_Tile *last_tile;
    b32 free_children;
    f32 scroll_y;
    
    // NOTE(allen): Variables
    E_Space *space;
    String8 name;
    E_Definition *definition;
    b32 state_ahead_of_source;
};

////////////////////////////////

internal E_View* E_NewView(void);
internal void    E_DeleteView(E_View *view);

internal void          E_ViewDeriveFromSource(E_View *view);
internal E_Definition* E_ViewCreateNewDefinition(E_View *view);
internal void          E_ViewLookAtDefinition(E_View *view, E_Definition *definition);
internal b32           E_ViewWillLookAtDefinition(E_View *view);
internal void          E_ViewCleanup(E_View *view);
internal b32           E_ViewWillCloseDefinition(E_View *view);
internal void          E_ViewShutdown(E_View *view);
internal String8       E_ViewGetName(E_View *view);

internal void E_InitGlobalView(E_View *view, R_Font *font, E_Space *space);
internal void E_InitPageView(E_View *view, R_Font *font, E_Definition *definition);

#endif //DEFINITION_H
