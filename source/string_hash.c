////////////////////////////////
// NOTE(allen): Implementation

internal STR_Hash
STR_InitHash(void){
    STR_Hash hash = {0};
    hash.arena_ptrs = M_ArenaInitialize();
    hash.arena_strs = M_ArenaInitializeWithAlign(1);
    hash.str_ptrs = PushArray(&hash.arena_ptrs, String8, 1);
    hash.str_count = 1;
    hash.str_ptrs[0] = S8Lit("");
    return(hash);
}

internal STR_Index
STR_Save(STR_Hash *hash, String8 string){
    u32 count = hash->str_count;
    STR_Index result = count;
    String8 *str_ptr = hash->str_ptrs;
    for (u32 i = 0; i < count; i += 1, str_ptr += 1){
        if (StringMatch(*str_ptr, string)){
            result = i;
            break;
        }
    }
    if (result == count){
        String8 *new_str = PushArray(&hash->arena_ptrs, String8, 1);
        new_str->str = PushArray(&hash->arena_strs, u8, string.size);
        new_str->size = string.size;
        MemoryCopy(new_str->str, string.str, string.size);
        hash->str_count += 1;
    }
    return(result);
}

internal String8
STR_Read(STR_Hash *hash, STR_Index index){
    String8 result = {0};
    if (index < hash->str_count){
        result = hash->str_ptrs[index];
    }
    return(result);
}

internal String8
STR_NameFromPath(String8 path){
    String8 result = path;
    if (path.size > 0){
        u8 *first = path.str;
        u8 *last = path.str + path.size - 1;
        u8 *ptr = last;
        for (;ptr >= first; ptr -= 1){
            if (*ptr == '/' || *ptr == '\\'){
                break;
            }
        }
        if (ptr < last){
            result = S8Range(ptr + 1, last + 1);
        }
    }
    return(result);
}

internal String8
STR_NameWithoutExtension(String8 path){
    String8 result = path;
    if (path.size > 0){
        u8 *first = path.str;
        u8 *ptr = path.str + path.size - 1;
        for (;ptr >= first; ptr -= 1){
            if (*ptr == '.'){
                break;
            }
        }
        if (ptr > first){
            result = S8Range(first, ptr);
        }
    }
    return(result);
}

////////////////////////////////
// NOTE(allen): Test

internal void
_STR_Test(void){
    STR_Hash hash = STR_InitHash();
    STR_Index a = STR_Save(&hash, S8Lit("Foo"));
    STR_Index b = STR_Save(&hash, S8Lit("Bar"));
    STR_Index c = STR_Save(&hash, S8Lit("Jim"));
    STR_Index d = STR_Save(&hash, S8Lit("Jams"));
    Assert(STR_Save(&hash, S8Lit("Foo")) == a);
    Assert(STR_Save(&hash, S8Lit("Bar")) == b);
    Assert(STR_Save(&hash, S8Lit("Jim")) == c);
    Assert(STR_Save(&hash, S8Lit("Jams")) == d);
    Assert(StringMatch(STR_Read(&hash, a), S8Lit("Foo")));
    Assert(StringMatch(STR_Read(&hash, b), S8Lit("Bar")));
    Assert(StringMatch(STR_Read(&hash, c), S8Lit("Jim")));
    Assert(StringMatch(STR_Read(&hash, d), S8Lit("Jams")));
}

