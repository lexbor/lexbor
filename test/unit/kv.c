/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "unit/kv.h"


const lxb_char_t *
unit_kv_state_begin(unit_kv_t *kv, const lxb_char_t *data, const lxb_char_t *end);

unit_kv_token_t *
unit_kv_rules_begin(unit_kv_t *kv, unit_kv_token_t *token, void *ctx);

static lxb_char_t *
unit_kv_position_as_string(unit_kv_t *kv, const char *desc,
                           lexbor_str_t *str, const lxb_char_t *begin,
                           const lxb_char_t *line_begin, size_t line_count);

static lxb_char_t *
unit_kv_fragment_as_string(unit_kv_t *kv,
                           lexbor_str_t *str, const lxb_char_t *begin);


const lxb_char_t *unit_kv_eof = (const lxb_char_t *) "\x00";


unit_kv_t *
unit_kv_create(void)
{
    return lexbor_calloc(1, sizeof(unit_kv_t));
}

lxb_status_t
unit_kv_init(unit_kv_t *kv, size_t sp_size)
{
    lxb_status_t status;

    if (kv == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    kv->map = lexbor_bst_map_create();
    status = lexbor_bst_map_init(kv->map, 4096 * 16);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    kv->objs = lexbor_dobject_create();
    status = lexbor_dobject_init(kv->objs, 512, sizeof(unit_kv_value_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    kv->mraw = lexbor_bst_map_mraw(kv->map);

    kv->status = LXB_STATUS_OK;

    kv->line_count = 0;
    kv->line_begin = NULL;
    kv->error_pos = NULL;
    kv->filename = NULL;

    lexbor_str_init(&kv->var_name, kv->mraw, 1);
    if (kv->var_name.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
unit_kv_clean(unit_kv_t *kv)
{
    kv->status = LXB_STATUS_OK;

    if (kv->filename != NULL) {
        kv->filename = lexbor_free(kv->filename);

        if (kv->begin != NULL) {
            kv->begin = lexbor_free((lxb_char_t *) kv->begin);
        }
    }

    kv->line_count = 0;
    kv->line_begin = NULL;
    kv->error_pos = NULL;

    lexbor_str_clean(&kv->var_name);

    lexbor_bst_map_clean(kv->map);
    lexbor_dobject_clean(kv->objs);

    lexbor_str_init(&kv->var_name, kv->mraw, 1);
    if (kv->var_name.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

unit_kv_t *
unit_kv_destroy(unit_kv_t *kv, bool self_destroy)
{
    if (kv == NULL) {
        return NULL;
    }

    if (kv->filename != NULL) {
        kv->filename = lexbor_free(kv->filename);

        if (kv->begin != NULL) {
            kv->begin = lexbor_free((lxb_char_t *) kv->begin);
        }
    }

    kv->mraw = NULL;
    kv->map = lexbor_bst_map_destroy(kv->map, true);
    kv->objs = lexbor_dobject_destroy(kv->objs, true);

    if (self_destroy) {
        return lexbor_free(kv);
    }

    return kv;
}

lxb_status_t
unit_kv_parse(unit_kv_t *kv, const lxb_char_t *data, size_t size)
{
    const lxb_char_t *end = data + size;
    const lxb_char_t *eof_end = unit_kv_eof + 1UL;

    kv->token = lexbor_calloc(1, sizeof(unit_kv_token_t));
    if (kv->token == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    if (kv->filename != NULL) {
        kv->filename = lexbor_free(kv->filename);

        if (kv->begin != NULL) {
            lexbor_free((lxb_char_t *) kv->begin);
        }
    }

    kv->state = unit_kv_state_begin;
    kv->rules = unit_kv_rules_begin;

    kv->status = LXB_STATUS_OK;
    kv->line_count = 1;
    kv->line_begin = data;
    kv->error_pos = NULL;
    kv->is_eof = false;
    kv->bad_token = NULL;
    kv->begin = data;
    kv->end = end;

    while (data < end) {
        data = kv->state(kv, data, end);
    }

    if (kv->status != LXB_STATUS_OK) {
        goto done;
    }

    /* Send fake EOF char */
    kv->is_eof = true;

    while (kv->state(kv, unit_kv_eof, eof_end) < eof_end) {
        /* empty loop */
    }

    kv->is_eof = false;

    if (kv->status != LXB_STATUS_OK && kv->token != NULL) {
        kv->token->type = UNIT_KV_TOKEN_TYPE_EOF;

        kv->token = kv->rules(kv, kv->token, kv->rules_ctx);

        if (kv->token == NULL) {
            if (kv->status == LXB_STATUS_OK) {
                kv->status = LXB_STATUS_ERROR;
            }

            goto done;
        }
    }

done:

    if (kv->token != NULL) {
        kv->token = lexbor_free(kv->token);
    }

    return kv->status;
}

lxb_status_t
unit_kv_parse_file(unit_kv_t *kv, const lxb_char_t *filepath)
{
    FILE *fh;
    long size;
    char *data;
    size_t nread;
    lxb_char_t *dup_fp;
    lxb_status_t status;

    /* Try to read file */
    fh = fopen((const char *) filepath, "rb");
    if (fh == NULL) {
        kv->status = LXB_STATUS_ERROR;
        return kv->status;
    }

    if (fseek(fh, 0L, SEEK_END) != 0) {
        goto error;
    }

    size = ftell(fh);
    if (size < 0) {
        goto error;
    }

    if (fseek(fh, 0L, SEEK_SET) != 0) {
        goto error;
    }

    data = lexbor_malloc(size + 1);
    if (data == NULL) {
        fclose(fh);

        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    nread = fread(data, 1, size, fh);
    if (nread != (size_t) size) {
        goto error;
    }

    data[size] = '\0';

    fclose(fh);

    /* Copy filepath */
    nread = strlen((const char *) filepath) + 1;

    dup_fp = lexbor_malloc(nread);
    if (dup_fp == NULL) {
        fclose(fh);
        lexbor_free(data);

        kv->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        return kv->status;
    }

    memcpy(dup_fp, filepath, nread);

    /* Parse */
    status = unit_kv_parse(kv, (const lxb_char_t *) data, (size_t) size);

    kv->filename = dup_fp;

    return status;

error:

    fclose(fh);

    kv->status = LXB_STATUS_ERROR;

    return kv->status;
}

static lxb_char_t *
unit_kv_position_as_string(unit_kv_t *kv, const char *desc,
                           lexbor_str_t *str, const lxb_char_t *begin,
                           const lxb_char_t *line_begin, size_t line_count)
{
    lxb_char_t *data;
    char buffer[128];

    data = lexbor_str_append(str, kv->mraw,
                             (const lxb_char_t *) desc, strlen(desc));
    if (data == NULL) {
        return NULL;
    }

    data = (lxb_char_t *) " in line ";
    data = lexbor_str_append(str, kv->mraw,
                             data, strlen((const char *) data));
    if (data == NULL) {
        return NULL;
    }

    sprintf(buffer, LEXBOR_FORMAT_Z, line_count);
    data = lexbor_str_append(str, kv->mraw,
                             (const lxb_char_t *) buffer, strlen(buffer));
    if (data == NULL) {
        return NULL;
    }

    data = (lxb_char_t *) " and position ";
    data = lexbor_str_append(str, kv->mraw,
                             (const lxb_char_t *) data,
                             strlen((const char *) data));
    if (data == NULL) {
        return NULL;
    }

    sprintf(buffer, LEXBOR_FORMAT_Z, (begin - line_begin));
    data = lexbor_str_append(str, kv->mraw,
                             (const lxb_char_t *) buffer, strlen(buffer));
    if (data == NULL) {
        return NULL;
    }

    return data;
}

static lxb_char_t *
unit_kv_fragment_as_string(unit_kv_t *kv,
                           lexbor_str_t *str, const lxb_char_t *begin)
{
    size_t len;
    lxb_char_t *data;

    data = (lxb_char_t *) "Begin fragment:\n";
    data = lexbor_str_append(str, kv->mraw,
                             data, strlen((const char *) data));
    if (data == NULL) {
        return NULL;
    }

    if ((kv->end - begin) < 20) {
        len = kv->end - begin;
        data = lexbor_str_append(str, kv->mraw, begin, len);

        if (data == NULL) {
            return NULL;
        }
    }
    else {
        len = 20;

        data = lexbor_str_append(str, kv->mraw, begin, len);
        if (data == NULL) {
            return NULL;
        }

        data = (lxb_char_t *) "...";
        data = lexbor_str_append(str, kv->mraw,
                                 data, strlen((const char *) data));
        if (data == NULL) {
            return NULL;
        }
    }

    return data;
}

lexbor_str_t
unit_kv_value_position_as_string(unit_kv_t *kv, unit_kv_value_t *value)
{
    lexbor_str_t str = {0};
    lxb_char_t *data;

    data = lexbor_str_init(&str, kv->mraw, 1024);
    if (data == NULL) {
        return str;
    }

    data = unit_kv_position_as_string(kv, "Value", &str, value->pos.begin,
                                      value->pos.line_begin,
                                      value->pos.line_count);
    if (data == NULL) {
        return str;
    }

    return str;
}

lexbor_str_t
unit_kv_value_fragment_as_string(unit_kv_t *kv, unit_kv_value_t *value)
{
    lexbor_str_t str = {0};
    lxb_char_t *data;

    data = lexbor_str_init(&str, kv->mraw, 1024);
    if (data == NULL) {
        return str;
    }

    data = unit_kv_fragment_as_string(kv, &str, value->pos.begin);
    if (data == NULL) {
        return str;
    }

    return str;
}

lexbor_str_t
unit_kv_parse_error_as_string(unit_kv_t *kv)
{
    lxb_char_t *data;
    unit_kv_token_t *token;
    lexbor_str_t str = {0};

    if (kv->status == LXB_STATUS_OK) {
        return str;
    }

    data = lexbor_str_init(&str, kv->mraw, 1024);
    if (data == NULL) {
        return str;
    }

    if (kv->status == LXB_STATUS_ERROR) {
        data = (lxb_char_t *) "Error";

        lexbor_str_append(&str, kv->mraw, data, strlen((const char *) data));

        return str;
    }

    if (kv->status == LXB_STATUS_ERROR) {
        data = (lxb_char_t *) "Failed to allocate memory";

        lexbor_str_append(&str, kv->mraw, data, strlen((const char *) data));

        return str;
    }

    if (kv->status == LXB_STATUS_ERROR_UNEXPECTED_RESULT) {
        token = kv->bad_token;

        if (token->type == UNIT_KV_TOKEN_TYPE_EOF) {
            data = (lxb_char_t *) "Unexpected END OF FILE";
            data = lexbor_str_append(&str, kv->mraw,
                                     data, strlen((const char *) data));
            return str;
        }

        data = unit_kv_position_as_string(kv, "Unexpected token",
                                          &str, token->pos.begin,
                                          token->pos.line_begin,
                                          token->pos.line_count);
        if (data == NULL) {
            return str;
        }

        data = (lxb_char_t *) "\n";
        data = lexbor_str_append(&str, kv->mraw,
                                 data, strlen((const char *) data));
        if (data == NULL) {
            return str;
        }

        unit_kv_fragment_as_string(kv, &str, token->pos.begin);

        return str;
    }

    if (kv->status != LXB_STATUS_ERROR_UNEXPECTED_DATA) {
        data = (lxb_char_t *) "Unknown error";
        lexbor_str_append(&str, kv->mraw, data, strlen((const char *) data));

        return str;
    }

    if (kv->error_pos == unit_kv_eof) {
        data = (lxb_char_t *) "Unexpected END OF FILE";
        data = lexbor_str_append(&str, kv->mraw,
                                 data, strlen((const char *) data));
        return str;
    }

    data = unit_kv_position_as_string(kv, "Unexpected character",
                                      &str, kv->error_pos,
                                      kv->line_begin, kv->line_count);
    if (data == NULL) {
        return str;
    }

    return str;
}

unit_kv_value_t *
unit_kv_hash_value(unit_kv_value_t *value, const lxb_char_t *key, size_t len)
{
    lexbor_bst_map_entry_t *entry;

    entry = lexbor_bst_map_search(value->kv->map, value->value.hash_root,
                                  key, len);
    if (entry == NULL) {
        return NULL;
    }

    return entry->value;
}

unit_kv_value_t *
unit_kv_array_append(unit_kv_value_t *array, unit_kv_value_t *value)
{
    unit_kv_value_t **list;
    unit_kv_array_t *ary = &array->value.array;

    if (ary->list == NULL) {
        ary->list = lexbor_mraw_calloc(array->kv->mraw,
                                       sizeof(unit_kv_value_t *));
        if (ary->list == NULL) {
            return NULL;
        }
    }
    else {
        list = lexbor_mraw_realloc(array->kv->mraw, ary->list,
                                   lexbor_mraw_data_size(ary->list)
                                   + sizeof(unit_kv_value_t *));
        if (list == NULL) {
            return NULL;
        }

        ary->list = list;
    }

    ary->list[ ary->length ] = value;
    ary->length++;

    return value;
}

void
unit_kv_string_destroy(unit_kv_t *kv, lexbor_str_t *str, bool self_destroy)
{
    if (str == NULL) {
        return;
    }

    lexbor_str_destroy(str, kv->mraw, self_destroy);
}
