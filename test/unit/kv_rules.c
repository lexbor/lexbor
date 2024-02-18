/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "unit/kv.h"


unit_kv_token_t *
unit_kv_rules_begin(unit_kv_t *kv, unit_kv_token_t *token, void *ctx);

static unit_kv_value_t *
unit_kv_rules_value_append(unit_kv_t *kv);

static unit_kv_value_t *
unit_kv_rules_value_append_hash(unit_kv_t *kv, unit_kv_token_t *token);

static unit_kv_value_t *
unit_kv_rules_value_append_array(unit_kv_t *kv, unit_kv_token_t *token);

lxb_status_t
unit_kv_rules_check_parent(unit_kv_t *kv);

static unit_kv_token_t *
unit_kv_rules_hash_begin(unit_kv_t *kv, unit_kv_token_t *token, void *ctx);

static unit_kv_token_t *
unit_kv_rules_hash_name(unit_kv_t *kv, unit_kv_token_t *token, void *ctx);

static unit_kv_token_t *
unit_kv_rules_hash_value(unit_kv_t *kv, unit_kv_token_t *token, void *ctx);

static unit_kv_token_t *
unit_kv_rules_hash_after_value(unit_kv_t *kv, unit_kv_token_t *token, void *ctx);

static unit_kv_token_t *
unit_kv_rules_array_begin(unit_kv_t *kv, unit_kv_token_t *token, void *ctx);

static unit_kv_token_t *
unit_kv_rules_array_after_value(unit_kv_t *kv, unit_kv_token_t *token, void *ctx);

static unit_kv_token_t *
unit_kv_rules_emit_all(unit_kv_t *kv, unit_kv_token_t *token, void *ctx);

static lxb_status_t
unit_kv_rules_check_chars(unit_kv_token_t *token, unit_kv_value_t *value);


static unit_kv_value_t *
unit_kv_rules_value_append(unit_kv_t *kv)
{
    unit_kv_value_t *value = lexbor_dobject_calloc(kv->objs);
    if (value == NULL) {
        return NULL;
    }

    value->kv = kv;

    if (kv->value == NULL) {
        kv->value = value;
        return value;
    }

    value->parent = kv->value;
    kv->value = value;

    return value;
}

static unit_kv_value_t *
unit_kv_rules_value_append_hash(unit_kv_t *kv, unit_kv_token_t *token)
{
    unit_kv_value_t *value = unit_kv_rules_value_append(kv);

    value->type = UNIT_KV_VALUE_TYPE_HASH;
    value->pos = token->pos;

    return value;
}

static unit_kv_value_t *
unit_kv_rules_value_append_array(unit_kv_t *kv, unit_kv_token_t *token)
{
    unit_kv_value_t *value = unit_kv_rules_value_append(kv);

    value->type = UNIT_KV_VALUE_TYPE_ARRAY;
    value->pos = token->pos;

    return value;
}

lxb_status_t
unit_kv_rules_check_parent(unit_kv_t *kv)
{
    if (kv->value->parent == NULL) {
        kv->rules = unit_kv_rules_emit_all;
        return LXB_STATUS_OK;
    }

    kv->value = kv->value->parent;

    if (kv->value->type == UNIT_KV_VALUE_TYPE_HASH) {
        kv->rules = unit_kv_rules_hash_after_value;
    }
    else if (kv->value->type == UNIT_KV_VALUE_TYPE_ARRAY) {
        kv->rules = unit_kv_rules_array_after_value;
    }
    else {
        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}

unit_kv_token_t *
unit_kv_rules_begin(unit_kv_t *kv, unit_kv_token_t *token, void *ctx)
{
    switch (token->type) {
        case UNIT_KV_TOKEN_TYPE_LEFT_CURLY_BRACKET:
            if (unit_kv_rules_value_append_hash(kv, token) == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            kv->rules = unit_kv_rules_hash_begin;

            break;

        case UNIT_KV_TOKEN_TYPE_LEFT_SQUARE_BRACKET:
            if (unit_kv_rules_value_append_array(kv, token) == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            kv->rules = unit_kv_rules_array_begin;

            break;

        case UNIT_KV_TOKEN_TYPE_EOF:
            break;

        default:
            kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
            kv->bad_token = token;

            return NULL;
    }

    return token;
}

/*
 * Hash
 */
static unit_kv_token_t *
unit_kv_rules_hash_begin(unit_kv_t *kv, unit_kv_token_t *token, void *ctx)
{
    switch (token->type) {
        case UNIT_KV_TOKEN_TYPE_RIGHT_CURLY_BRACKET:
            if (unit_kv_rules_check_parent(kv) != LXB_STATUS_OK) {
                kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
                kv->bad_token = token;

                return NULL;
            }

            break;

        case UNIT_KV_TOKEN_TYPE_STRING:
            kv->key_name = token->value.str;
            kv->rules = unit_kv_rules_hash_name;

            break;

        case UNIT_KV_TOKEN_TYPE_EOF:
            /* fall through */

        default:
            kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
            kv->bad_token = token;

            return NULL;
    }

    return token;
}

static unit_kv_token_t *
unit_kv_rules_hash_name(unit_kv_t *kv, unit_kv_token_t *token, void *ctx)
{
    switch (token->type) {
        case UNIT_KV_TOKEN_TYPE_COLON:
            kv->rules = unit_kv_rules_hash_value;

            break;

        case UNIT_KV_TOKEN_TYPE_EOF:
            /* fall through */

        default:
            kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
            kv->bad_token = token;

            return NULL;
    }

    return token;
}

static unit_kv_token_t *
unit_kv_rules_hash_value(unit_kv_t *kv, unit_kv_token_t *token, void *ctx)
{
    unit_kv_value_t *value;
    lexbor_bst_map_entry_t *hash_entry;

    switch (token->type) {
        case UNIT_KV_TOKEN_TYPE_LEFT_CURLY_BRACKET:
            hash_entry = lexbor_bst_map_insert_not_exists(kv->map,
                                                      &kv->value->value.hash_root,
                                                      kv->key_name.data,
                                                      kv->key_name.length);
            if (hash_entry == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            hash_entry->value = unit_kv_rules_value_append_hash(kv, token);
            if (hash_entry->value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            kv->rules = unit_kv_rules_hash_begin;

            break;

        case UNIT_KV_TOKEN_TYPE_LEFT_SQUARE_BRACKET:
            hash_entry = lexbor_bst_map_insert_not_exists(kv->map,
                                                      &kv->value->value.hash_root,
                                                      kv->key_name.data,
                                                      kv->key_name.length);
            if (hash_entry == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            hash_entry->value = unit_kv_rules_value_append_array(kv, token);
            if (hash_entry->value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            kv->rules = unit_kv_rules_array_begin;

            break;

        case UNIT_KV_TOKEN_TYPE_STRING:
        case UNIT_KV_TOKEN_TYPE_DATA:
            hash_entry = lexbor_bst_map_insert_not_exists(kv->map,
                                                      &kv->value->value.hash_root,
                                                      kv->key_name.data,
                                                      kv->key_name.length);
            if (hash_entry == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            value = lexbor_dobject_calloc(kv->objs);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            value->type = UNIT_KV_VALUE_TYPE_STRING;
            value->kv = kv;
            value->pos = token->pos;
            value->value.str = token->value.str;

            hash_entry->value = value;

            kv->rules = unit_kv_rules_hash_after_value;

            break;

        case UNIT_KV_TOKEN_TYPE_NUMBER:
            hash_entry = lexbor_bst_map_insert_not_exists(kv->map,
                                                      &kv->value->value.hash_root,
                                                      kv->key_name.data,
                                                      kv->key_name.length);
            if (hash_entry == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            value = lexbor_dobject_calloc(kv->objs);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            value->type = UNIT_KV_VALUE_TYPE_NUMBER;
            value->kv = kv;
            value->pos = token->pos;
            value->value.num = token->value.num;

            hash_entry->value = value;

            kv->rules = unit_kv_rules_hash_after_value;

            break;

        case UNIT_KV_TOKEN_TYPE_CHARS:
            value = lexbor_dobject_calloc(kv->objs);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            if (unit_kv_rules_check_chars(token, value) == LXB_STATUS_ERROR) {
                value->type = UNIT_KV_VALUE_TYPE_CHARS;
                value->value.str = token->value.str;
            }

            hash_entry = lexbor_bst_map_insert_not_exists(kv->map,
                                                      &kv->value->value.hash_root,
                                                      kv->key_name.data,
                                                      kv->key_name.length);
            if (hash_entry == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            value->kv = kv;
            value->pos = token->pos;

            hash_entry->value = value;

            kv->rules = unit_kv_rules_hash_after_value;

            break;

        case UNIT_KV_TOKEN_TYPE_EOF:
            /* fall through */

        default:
            kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
            kv->bad_token = token;

            return NULL;
    }

    return token;
}

static unit_kv_token_t *
unit_kv_rules_hash_after_value(unit_kv_t *kv, unit_kv_token_t *token, void *ctx)
{
    switch (token->type) {
        case UNIT_KV_TOKEN_TYPE_RIGHT_CURLY_BRACKET:
            if (unit_kv_rules_check_parent(kv) != LXB_STATUS_OK) {
                kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
                kv->bad_token = token;

                return NULL;
            }

            break;

        case UNIT_KV_TOKEN_TYPE_COMMA:
            kv->rules = unit_kv_rules_hash_begin;

            break;

        case UNIT_KV_TOKEN_TYPE_EOF:
            /* fall through */

        default:
            kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
            kv->bad_token = token;

            return NULL;
    }

    return token;
}

/*
 * Array
 */
static unit_kv_token_t *
unit_kv_rules_array_begin(unit_kv_t *kv, unit_kv_token_t *token, void *ctx)
{
    unit_kv_value_t *value, *array;

    switch (token->type) {
        case UNIT_KV_TOKEN_TYPE_LEFT_CURLY_BRACKET:
            array = kv->value;

            value = unit_kv_rules_value_append_hash(kv, token);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            value = unit_kv_array_append(array, value);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            kv->rules = unit_kv_rules_hash_begin;

            break;

        case UNIT_KV_TOKEN_TYPE_LEFT_SQUARE_BRACKET:
            array = kv->value;

            value = unit_kv_rules_value_append_array(kv, token);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            value = unit_kv_array_append(array, value);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            break;

        case UNIT_KV_TOKEN_TYPE_RIGHT_SQUARE_BRACKET:
            if (unit_kv_rules_check_parent(kv) != LXB_STATUS_OK) {
                kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
                kv->bad_token = token;

                return NULL;
            }

            break;

        case UNIT_KV_TOKEN_TYPE_STRING:
        case UNIT_KV_TOKEN_TYPE_DATA:
            array = kv->value;

            value = lexbor_dobject_calloc(kv->objs);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            value->type = UNIT_KV_VALUE_TYPE_STRING;
            value->kv = kv;
            value->pos = token->pos;
            value->value.str = token->value.str;

            value = unit_kv_array_append(array, value);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            kv->rules = unit_kv_rules_array_after_value;

            break;

        case UNIT_KV_TOKEN_TYPE_NUMBER:
            array = kv->value;

            value = lexbor_dobject_calloc(kv->objs);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            value->type = UNIT_KV_VALUE_TYPE_NUMBER;
            value->kv = kv;
            value->pos = token->pos;
            value->value.num = token->value.num;

            value = unit_kv_array_append(array, value);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            kv->rules = unit_kv_rules_array_after_value;

            break;

        case UNIT_KV_TOKEN_TYPE_CHARS:
            array = kv->value;

            value = lexbor_dobject_calloc(kv->objs);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            if (unit_kv_rules_check_chars(token, value) == LXB_STATUS_ERROR) {
                value->type = UNIT_KV_VALUE_TYPE_CHARS;
                value->value.str = token->value.str;
            }

            value->kv = kv;
            value->pos = token->pos;
            value->value.num = token->value.num;

            value = unit_kv_array_append(array, value);
            if (value == NULL) {
                kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                return NULL;
            }

            kv->rules = unit_kv_rules_array_after_value;

            break;

        case UNIT_KV_TOKEN_TYPE_EOF:
            /* fall through */

        default:
            kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
            kv->bad_token = token;

            return NULL;
    }

    return token;
}

static unit_kv_token_t *
unit_kv_rules_array_after_value(unit_kv_t *kv, unit_kv_token_t *token, void *ctx)
{
    switch (token->type) {
        case UNIT_KV_TOKEN_TYPE_RIGHT_SQUARE_BRACKET:
            if (unit_kv_rules_check_parent(kv) != LXB_STATUS_OK) {
                kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
                kv->bad_token = token;

                return NULL;
            }

            break;

        case UNIT_KV_TOKEN_TYPE_COMMA:
            kv->rules = unit_kv_rules_array_begin;

            break;

        case UNIT_KV_TOKEN_TYPE_EOF:
            /* fall through */

        default:
            kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
            kv->bad_token = token;

            return NULL;
    }

    return token;
}

/*
 * Sys
 */
static unit_kv_token_t *
unit_kv_rules_emit_all(unit_kv_t *kv, unit_kv_token_t *token, void *ctx)
{
    if (token->type == UNIT_KV_TOKEN_TYPE_EOF) {
        return token;
    }

    kv->status = LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    kv->bad_token = token;

    return NULL;
}

static lxb_status_t
unit_kv_rules_check_chars(unit_kv_token_t *token, unit_kv_value_t *value)
{
    if (token->value.str.length == 4) {
        if (lexbor_str_data_cmp(token->value.str.data, (lxb_char_t *) "true"))
        {
            value->type = UNIT_KV_VALUE_TYPE_TRUE;
        }
        else if (lexbor_str_data_cmp(token->value.str.data,
                                     (lxb_char_t *) "null"))
        {
            value->type = UNIT_KV_VALUE_TYPE_NULL;
        }
        else {
            return LXB_STATUS_ERROR;
        }
    }
    else if (token->value.str.length == 5
             && lexbor_str_data_cmp(token->value.str.data,
                                    (lxb_char_t *) "false"))
    {
        value->type = UNIT_KV_VALUE_TYPE_FALSE;
    }
    else {
        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}
