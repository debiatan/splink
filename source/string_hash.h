/* date = July 6th 2020 6:18 am */

#ifndef STRING_HASH_H
#define STRING_HASH_H

typedef u32 STR_Index;

typedef struct STR_Hash STR_Hash;
struct STR_Hash{
    M_Arena arena_ptrs;
    M_Arena arena_strs;
    String8 *str_ptrs;
    u32 str_count;
};

////////////////////////////////

internal STR_Hash STR_InitHash(void);
internal STR_Index STR_Save(STR_Hash *hash, String8 string);
internal String8 STR_Read(STR_Hash *hash, STR_Index index);

#endif //STRING_HASH_H
