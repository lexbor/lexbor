/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef UNIT_KV_H
#define UNIT_KV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <lexbor/core/bst_map.h>
#include <lexbor/core/dobject.h>
#include <lexbor/core/str.h>


typedef struct unit_kv unit_kv_t;
typedef struct unit_kv_value unit_kv_value_t;
typedef struct unit_kv_token unit_kv_token_t;

typedef const lxb_char_t *(*unit_kv_state_f)(unit_kv_t *kv,
                                             const lxb_char_t *data,
                                             const lxb_char_t *end);

typedef unit_kv_token_t *(*unit_kv_rules_f)(unit_kv_t *kv,
                                            unit_kv_token_t *token,
                                            void *ctx);

typedef enum {
    UNIT_KV_VALUE_TYPE_UNDEF  = 0x00,
    UNIT_KV_VALUE_TYPE_STRING = 0x01,
    UNIT_KV_VALUE_TYPE_ARRAY  = 0x02,
    UNIT_KV_VALUE_TYPE_HASH   = 0x03,
    UNIT_KV_VALUE_TYPE_CHARS  = 0x04,
    UNIT_KV_VALUE_TYPE_NUMBER = 0x05,
    UNIT_KV_VALUE_TYPE_TRUE   = 0x06,
    UNIT_KV_VALUE_TYPE_FALSE  = 0x07,
    UNIT_KV_VALUE_TYPE_NULL   = 0x08
}
unit_kv_value_type_t;

typedef enum {
    UNIT_KV_TOKEN_TYPE_UNDEF                = 0x00,
    UNIT_KV_TOKEN_TYPE_LEFT_CURLY_BRACKET   = 0x01, /* { */
    UNIT_KV_TOKEN_TYPE_RIGHT_CURLY_BRACKET  = 0x02, /* } */
    UNIT_KV_TOKEN_TYPE_LEFT_SQUARE_BRACKET  = 0x03, /* [ */
    UNIT_KV_TOKEN_TYPE_RIGHT_SQUARE_BRACKET = 0x04, /* ] */
    UNIT_KV_TOKEN_TYPE_COLON                = 0x05, /* : */
    UNIT_KV_TOKEN_TYPE_COMMA                = 0x06, /* , */
    UNIT_KV_TOKEN_TYPE_STRING               = 0x07,
    UNIT_KV_TOKEN_TYPE_CHARS                = 0x08,
    UNIT_KV_TOKEN_TYPE_NUMBER               = 0x09,
    UNIT_KV_TOKEN_TYPE_DATA                 = 0x0A,
    UNIT_KV_TOKEN_TYPE_EOF                  = 0x0B
}
unit_kv_token_type_t;

typedef struct {
    bool is_float;

    union {
        double f;
        long   l;
    }
    value;
}
unit_kv_number_t;

typedef struct {
    unit_kv_value_t **list;
    size_t length;
}
unit_kv_array_t;

typedef struct {
    const lxb_char_t *begin;
    const lxb_char_t *line_begin;
    size_t           line_count;
}
unit_kv_position_t;

struct unit_kv_token {
    unit_kv_token_type_t type;
    unit_kv_position_t   pos;

    union {
        lexbor_str_t       str;
        unit_kv_number_t   num;
    }
    value;
};

struct unit_kv_value {
    unit_kv_value_type_t type;
    unit_kv_value_t      *parent;

    unit_kv_t            *kv;
    unit_kv_position_t   pos;

    union {
        lexbor_str_t       str;
        unit_kv_number_t   num;
        unit_kv_array_t    array;
        lexbor_bst_entry_t *hash_root;
    }
    value;
};

struct unit_kv {
    unit_kv_state_f    state;
    unit_kv_state_f    state_return;

    unit_kv_token_t    *token;
    unit_kv_value_t    *value;

    unit_kv_rules_f    rules;
    void               *rules_ctx;

    lexbor_dobject_t   *objs;
    lexbor_mraw_t      *mraw;
    lexbor_bst_map_t   *map;

    lxb_status_t       status;
    bool               is_eof;

    short              count;
    unsigned int       num;

    bool               num_negative;
    int                num_digits;
    int                num_decimals;
    int                num_exponent;

    size_t             line_count;
    const lxb_char_t   *line_begin;
    const lxb_char_t   *error_pos;

    lxb_char_t         *filename;
    const lxb_char_t   *begin;
    const lxb_char_t   *end;

    lxb_char_t         data_skip;
    size_t             data_skip_count;

    unit_kv_token_t    *bad_token;

    lexbor_str_t       var_name;
    lexbor_str_t       key_name;
};


LXB_API unit_kv_t *
unit_kv_create(void);

LXB_API lxb_status_t
unit_kv_init(unit_kv_t *kv, size_t sp_size);

LXB_API lxb_status_t
unit_kv_clean(unit_kv_t *kv);

LXB_API unit_kv_t *
unit_kv_destroy(unit_kv_t *kv, bool self_destroy);


LXB_API lxb_status_t
unit_kv_parse(unit_kv_t *kv, const lxb_char_t *data, size_t size);

LXB_API lxb_status_t
unit_kv_parse_file(unit_kv_t *kv, const lxb_char_t *filepath);


LXB_API unit_kv_value_t *
unit_kv_hash_value(unit_kv_value_t *value, const lxb_char_t *key, size_t len);

LXB_API unit_kv_value_t *
unit_kv_array_append(unit_kv_value_t *array, unit_kv_value_t *value);

LXB_API lexbor_str_t
unit_kv_parse_error_as_string(unit_kv_t *kv);

LXB_API lexbor_str_t
unit_kv_value_position_as_string(unit_kv_t *kv, unit_kv_value_t *value);

LXB_API lexbor_str_t
unit_kv_value_fragment_as_string(unit_kv_t *kv, unit_kv_value_t *value);

LXB_API void
unit_kv_string_destroy(unit_kv_t *kv, lexbor_str_t *str, bool self_destroy);


/*
 * Inline functions
 */
lxb_inline unit_kv_value_t *
unit_kv_value(unit_kv_t *kv)
{
    return kv->value;
}

lxb_inline bool
unit_kv_is_string(unit_kv_value_t *value)
{
    return value->type == UNIT_KV_VALUE_TYPE_STRING;
}

lxb_inline lexbor_str_t *
unit_kv_string(unit_kv_value_t *value)
{
    return &value->value.str;
}

lxb_inline bool
unit_kv_is_chars(unit_kv_value_t *value)
{
    return value->type == UNIT_KV_VALUE_TYPE_CHARS;
}

lxb_inline lexbor_str_t *
unit_kv_chars(unit_kv_value_t *value)
{
    return &value->value.str;
}

lxb_inline bool
unit_kv_is_number(unit_kv_value_t *value)
{
    return value->type == UNIT_KV_VALUE_TYPE_NUMBER;
}

lxb_inline unit_kv_number_t *
unit_kv_number(unit_kv_value_t *value)
{
    return &value->value.num;
}

lxb_inline bool
unit_kv_is_hash(unit_kv_value_t *value)
{
    return value->type == UNIT_KV_VALUE_TYPE_HASH;
}

lxb_inline lexbor_bst_entry_t *
unit_kv_hash(unit_kv_value_t *value)
{
    return value->value.hash_root;
}

lxb_inline bool
unit_kv_is_array(unit_kv_value_t *value)
{
    return value->type == UNIT_KV_VALUE_TYPE_ARRAY;
}

lxb_inline unit_kv_array_t *
unit_kv_array(unit_kv_value_t *value)
{
    return &value->value.array;
}

lxb_inline bool
unit_kv_is_bool(unit_kv_value_t *value)
{
    return value->type == UNIT_KV_VALUE_TYPE_TRUE
        || value->type == UNIT_KV_VALUE_TYPE_FALSE;
}

lxb_inline bool
unit_kv_bool(unit_kv_value_t *value)
{
    return value->type == UNIT_KV_VALUE_TYPE_TRUE;
}

lxb_inline bool
unit_kv_is_null(unit_kv_value_t *value)
{
    return value->type == UNIT_KV_VALUE_TYPE_NULL;
}

lxb_inline unit_kv_value_t *
unit_kv_hash_value_nolen(unit_kv_value_t *value, const lxb_char_t *key)
{
    return unit_kv_hash_value(value, key, strlen((const char *) key));
}

lxb_inline unit_kv_value_t *
unit_kv_hash_value_nolen_c(unit_kv_value_t *value, const char *key)
{
    return unit_kv_hash_value(value, (const lxb_char_t *) key, strlen(key));
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UNIT_KV_H */
