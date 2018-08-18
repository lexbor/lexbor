/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_STR_H
#define LEXBOR_STR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/core/base.h>
#include <lexbor/core/mraw.h>
#include <lexbor/core/utils.h>


#define lexbor_str_get(str, attr) str->attr
#define lexbor_str_set(str, attr) lexbor_str_get(str, attr)
#define lexbor_str_len(str) lexbor_str_get(str, length)


#define lexbor_str_check_size_arg_m(str, size, mraw, plus_len, return_fail)    \
    do {                                                                       \
        lxb_char_t *tmp;                                                       \
                                                                               \
        if (str->length > (SIZE_MAX - (plus_len)))                             \
            return (return_fail);                                              \
                                                                               \
        if ((str->length + (plus_len)) > (size)) {                             \
            tmp = lexbor_mraw_realloc(mraw, str->data,                         \
                                      (str->length + plus_len));               \
                                                                               \
            if (tmp == NULL) {                                                 \
                return (return_fail);                                          \
            }                                                                  \
                                                                               \
            str->data = tmp;                                                   \
        }                                                                      \
    }                                                                          \
    while (0)


typedef struct {
    lxb_char_t *data;
    size_t     length;
}
lexbor_str_t;


lexbor_str_t *
lexbor_str_create(void);

lxb_char_t *
lexbor_str_init(lexbor_str_t *str, lexbor_mraw_t *mraw, size_t size);

void
lexbor_str_clean(lexbor_str_t *str);

void
lexbor_str_clean_all(lexbor_str_t *str);

lexbor_str_t *
lexbor_str_destroy(lexbor_str_t *str, lexbor_mraw_t *mraw, bool destroy_obj);


lxb_char_t *
lexbor_str_realloc(lexbor_str_t *str, lexbor_mraw_t *mraw, size_t new_size);

lxb_char_t *
lexbor_str_check_size(lexbor_str_t *str, lexbor_mraw_t *mraw, size_t plus_len);

/* Append */
lxb_char_t *
lexbor_str_append(lexbor_str_t *str, lexbor_mraw_t *mraw,
                  const lxb_char_t *data, size_t length);

lxb_char_t *
lexbor_str_append_before(lexbor_str_t *str, lexbor_mraw_t *mraw,
                         const lxb_char_t *buff, size_t length);

lxb_char_t *
lexbor_str_append_one(lexbor_str_t *str, lexbor_mraw_t *mraw,
                      const lxb_char_t data);

lxb_char_t *
lexbor_str_append_lowercase(lexbor_str_t *str, lexbor_mraw_t *mraw,
                            const lxb_char_t *data, size_t length);

lxb_char_t *
lexbor_str_append_with_rep_null_chars(lexbor_str_t *str, lexbor_mraw_t *mraw,
                                      const lxb_char_t *buff, size_t length);

/* Other functions */
lxb_char_t *
lexbor_str_copy(lexbor_str_t *dest, const lexbor_str_t *target,
                lexbor_mraw_t *mraw);

void
lexbor_str_stay_only_whitespace(lexbor_str_t *target);

size_t
lexbor_str_crop_whitespace_from_begin(lexbor_str_t *target);

size_t
lexbor_str_whitespace_from_begin(lexbor_str_t *target);

size_t
lexbor_str_whitespace_from_end(lexbor_str_t *target);


/* Data utils */
/*
 * [in] first: must be null-terminated
 * [in] sec: no matter what data
 * [in] sec_size: size of the 'sec' buffer
 *
 * Function compare two lxb_char_t data until find '\0' in first arg.
 * Successfully if the function returned a pointer starting with '\0',
 * otherwise, if the data of the second buffer is insufficient function returned
 * position in first buffer.
 * If function returns NULL, the data are not equal.
 */
const lxb_char_t *
lexbor_str_data_ncasecmp_first(const lxb_char_t *first, const lxb_char_t *sec,
                               size_t sec_size);
bool
lexbor_str_data_ncasecmp(const lxb_char_t *first, const lxb_char_t *sec,
                         size_t size);
bool
lexbor_str_data_casecmp(const lxb_char_t *first, const lxb_char_t *sec);

bool
lexbor_str_data_ncmp(const lxb_char_t *first, const lxb_char_t *sec,
                     size_t size);

bool
lexbor_str_data_cmp(const lxb_char_t *first, const lxb_char_t *sec);

bool
lexbor_str_data_cmp_ws(const lxb_char_t *first, const lxb_char_t *sec);


/*
 * Inline functions
 */
lxb_inline lxb_char_t *
lexbor_str_data(lexbor_str_t *str)
{
    return str->data;
}

lxb_inline size_t
lexbor_str_length(lexbor_str_t *str)
{
    return str->length;
}

lxb_inline size_t
lexbor_str_size(lexbor_str_t *str)
{
    return lexbor_mraw_data_size(str->data);
}

lxb_inline void
lexbor_str_data_set(lexbor_str_t *str, lxb_char_t *data)
{
    str->data = data;
}

lxb_inline void
lexbor_str_length_set(lexbor_str_t *str, size_t length)
{
    str->length = length;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_STR_H */
