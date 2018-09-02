/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/core/str.h"

#define LEXBOR_STR_RES_ANSI_REPLACEMENT_CHARACTER
#define LEXBOR_STR_RES_MAP_LOWERCASE
#define LEXBOR_STR_RES_MAP_UPPERCASE
#include "lexbor/core/str_res.h"


lexbor_str_t *
lexbor_str_create(void)
{
    return lexbor_calloc(1, sizeof(lexbor_str_t));
}

lxb_char_t *
lexbor_str_init(lexbor_str_t *str, lexbor_mraw_t *mraw, size_t size)
{
    if (str == NULL) {
        return NULL;
    }

    str->data = lexbor_mraw_alloc(mraw, (size + 1));
    str->length = 0;

    if (str->data != NULL) {
        *str->data = '\0';
    }

    return str->data;
}

void
lexbor_str_clean(lexbor_str_t *str)
{
    str->length = 0;
}

void
lexbor_str_clean_all(lexbor_str_t *str)
{
    memset(str, 0, sizeof(lexbor_str_t));
}

lexbor_str_t *
lexbor_str_destroy(lexbor_str_t *str, lexbor_mraw_t *mraw, bool destroy_obj)
{
    if (str == NULL) {
        return NULL;
    }

    if (str->data != NULL) {
        str->data = lexbor_mraw_free(mraw, str->data);
    }

    if (destroy_obj) {
        return lexbor_free(str);
    }

    return str;
}

lxb_char_t *
lexbor_str_realloc(lexbor_str_t *str, lexbor_mraw_t *mraw, size_t new_size)
{
    lxb_char_t *tmp = lexbor_mraw_realloc(mraw, str->data, new_size);
    if (tmp == NULL) {
        return NULL;
    }

    str->data = tmp;

    return tmp;
}

lxb_char_t *
lexbor_str_check_size(lexbor_str_t *str, lexbor_mraw_t *mraw, size_t plus_len)
{
    lxb_char_t *tmp;

    if (str->length > (SIZE_MAX - plus_len)) {
        return NULL;
    }

    if ((str->length + plus_len) <= lexbor_str_size(str)) {
        return str->data;
    }

    tmp = lexbor_mraw_realloc(mraw, str->data, (str->length + plus_len));
    if (tmp == NULL) {
        return NULL;
    }

    str->data = tmp;

    return tmp;
}

/* Append API */
lxb_char_t *
lexbor_str_append(lexbor_str_t *str, lexbor_mraw_t *mraw,
                  const lxb_char_t *buff, size_t length)
{
    lxb_char_t *data_begin;

    lexbor_str_check_size_arg_m(str, lexbor_str_size(str),
                                mraw, (length + 1), NULL);

    data_begin = &str->data[str->length];
    memcpy(data_begin, buff, sizeof(lxb_char_t) * length);

    str->length += length;
    str->data[str->length] = '\0';

    return data_begin;
}

lxb_char_t *
lexbor_str_append_before(lexbor_str_t *str, lexbor_mraw_t *mraw,
                         const lxb_char_t *buff, size_t length)
{
    lxb_char_t *data_begin;

    lexbor_str_check_size_arg_m(str, lexbor_str_size(str),
                                mraw, (length + 1), NULL);

    data_begin = &str->data[str->length];

    memmove(&str->data[length], str->data, sizeof(lxb_char_t) * str->length);
    memcpy(str->data, buff, sizeof(lxb_char_t) * length);

    str->length += length;
    str->data[str->length] = '\0';

    return data_begin;
}

lxb_char_t *
lexbor_str_append_one(lexbor_str_t *str, lexbor_mraw_t *mraw,
                      const lxb_char_t data)
{
    lexbor_str_check_size_arg_m(str, lexbor_str_size(str), mraw, 2, NULL);

    str->data[str->length] = data;

    str->length += 1;
    str->data[str->length] = '\0';

    return &str->data[(str->length - 1)];
}

lxb_char_t *
lexbor_str_append_lowercase(lexbor_str_t *str, lexbor_mraw_t *mraw,
                            const lxb_char_t *data, size_t length)
{
    size_t i;
    lxb_char_t *data_begin;

    lexbor_str_check_size_arg_m(str, lexbor_str_size(str),
                                mraw, (length + 1), NULL);

    data_begin = &str->data[str->length];

    for (i = 0; i < length; i++) {
        data_begin[i] = lexbor_str_res_map_lowercase[ data[i] ];
    }

    data_begin[i] = '\0';
    str->length += length;

    return data_begin;
}

lxb_char_t *
lexbor_str_append_with_rep_null_chars(lexbor_str_t *str, lexbor_mraw_t *mraw,
                                      const lxb_char_t *buff, size_t length)
{
    const lxb_char_t *pos, *res, *end;
    size_t current_len = str->length;

    lexbor_str_check_size_arg_m(str, lexbor_str_size(str),
                                mraw, (length + 1), NULL);
    end = buff + length;

    while (buff != end) {
        pos = memchr(buff, '\0', sizeof(lxb_char_t) * (end - buff));
        if (pos == NULL) {
            break;
        }

        res = lexbor_str_append(str, mraw, buff, (pos - buff));
        if (res == NULL) {
            return NULL;
        }

        res = lexbor_str_append(str, mraw,
                         lexbor_str_res_ansi_replacement_character,
                         sizeof(lexbor_str_res_ansi_replacement_character) - 1);
        if (res == NULL) {
            return NULL;
        }

        buff = pos + 1;
    }

    if (buff != end) {
        res = lexbor_str_append(str, mraw, buff, (end - buff));
        if (res == NULL) {
            return NULL;
        }
    }

    return &str->data[current_len];
}

lxb_char_t *
lexbor_str_copy(lexbor_str_t *dest, const lexbor_str_t *target,
                lexbor_mraw_t *mraw)
{
    if (target->data == NULL) {
        return NULL;
    }

    if (dest->data == NULL) {
        lexbor_str_init(dest, mraw, target->length);

        if (dest->data == NULL) {
            return NULL;
        }
    }

    return lexbor_str_append(dest, mraw, target->data, target->length);
}

void
lexbor_str_stay_only_whitespace(lexbor_str_t *target)
{
    size_t i, pos = 0;
    lxb_char_t *data = target->data;

    for (i = 0; i < target->length; i++) {
        if (lexbor_utils_whitespace(data[i], ==, ||)) {
            data[pos] = data[i];
            pos++;
        }
    }

    target->length = pos;
}

size_t
lexbor_str_crop_whitespace_from_begin(lexbor_str_t *target)
{
    size_t i;
    lxb_char_t *data = target->data;

    for (i = 0; i < target->length; i++) {
        if (lexbor_utils_whitespace(data[i], !=, &&)) {
            break;
        }
    }

    if (i != 0 && i != target->length) {
        memmove(target->data, &target->data[i], (target->length - i));
    }

    target->length -= i;
    return i;
}

size_t
lexbor_str_whitespace_from_begin(lexbor_str_t *target)
{
    size_t i;
    lxb_char_t *data = target->data;

    for (i = 0; i < target->length; i++) {
        if (lexbor_utils_whitespace(data[i], !=, &&)) {
            break;
        }
    }

    return i;
}

size_t
lexbor_str_whitespace_from_end(lexbor_str_t *target)
{
    size_t i = target->length;
    lxb_char_t *data = target->data;

    while (i) {
        i--;

        if (lexbor_utils_whitespace(data[i], !=, &&)) {
            return target->length - (i + 1);
        }
    }

    return 0;
}

/*
 * Data utils
 */
const lxb_char_t *
lexbor_str_data_ncasecmp_first(const lxb_char_t *first, const lxb_char_t *sec,
                               size_t sec_size)
{
    size_t i;

    for (i = 0; i < sec_size; i++) {
        if (first[i] == '\0') {
            return &first[i];
        }

        if (lexbor_str_res_map_lowercase[ first[i] ]
            != lexbor_str_res_map_lowercase[ sec[i] ])
        {
            return NULL;
        }
    }

    return &first[i];
}

bool
lexbor_str_data_ncasecmp(const lxb_char_t *first, const lxb_char_t *sec,
                         size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (lexbor_str_res_map_lowercase[ first[i] ]
            != lexbor_str_res_map_lowercase[ sec[i] ])
        {
            return false;
        }
    }

    return true;
}

bool
lexbor_str_data_casecmp(const lxb_char_t *first, const lxb_char_t *sec)
{
    for (;;) {
        if (lexbor_str_res_map_lowercase[*first]
            != lexbor_str_res_map_lowercase[*sec])
        {
            return false;
        }

        if (*first == '\0') {
            return true;
        }

        first++;
        sec++;
    }
}

bool
lexbor_str_data_ncmp(const lxb_char_t *first, const lxb_char_t *sec,
                     size_t size)
{
    if (memcmp(first, sec, sizeof(lxb_char_t) * size) == 0) {
        return true;
    }

    return false;
}

bool
lexbor_str_data_cmp(const lxb_char_t *first, const lxb_char_t *sec)
{
    for (;;) {
        if (*first != *sec) {
            return false;
        }

        if (*first == '\0') {
            return true;
        }

        first++;
        sec++;
    }
}

bool
lexbor_str_data_cmp_ws(const lxb_char_t *first, const lxb_char_t *sec)
{
    for (;;) {
        if (*first != *sec) {
            return false;
        }

        if (lexbor_utils_whitespace(*first, ==, ||) || *first == '\0') {
            return true;
        }

        first++;
        sec++;
    }
}

void
lexbor_str_data_to_lowercase(lxb_char_t *to, const lxb_char_t *from, size_t len)
{
    while (len) {
        len--;

        to[len] = lexbor_str_res_map_lowercase[ from[len] ];
    }
}

void
lexbor_str_data_to_uppercase(lxb_char_t *to, const lxb_char_t *from, size_t len)
{
    while (len) {
        len--;
        
        to[len] = lexbor_str_res_map_uppercase[ from[len] ];
    }
}
