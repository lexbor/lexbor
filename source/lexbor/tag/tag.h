/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_TAG_H
#define LEXBOR_TAG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/shbst.h"
#include "lexbor/core/shs.h"
#include "lexbor/core/dobject.h"
#include "lexbor/core/str.h"

#include "lexbor/tag/const.h"


typedef struct lxb_tag_data lxb_tag_data_t;
typedef struct lxb_tag_heap lxb_tag_heap_t;


typedef const lxb_tag_data_t *
(*lxb_tag_data_by_id_f)(lxb_tag_heap_t *tag_heap, lxb_tag_id_t tag_id);

typedef const lxb_tag_data_t *
(*lxb_tag_data_by_name_f)(lxb_tag_heap_t *tag_heap,
                          const lxb_char_t *name, size_t len);


struct lxb_tag_data {
    const lxb_char_t *name;
    const lxb_char_t *name_upper;
    size_t           name_len;

    lxb_tag_id_t     tag_id;
};

struct lxb_tag_heap {
    lexbor_shbst_t         *heap;
    lexbor_dobject_t       *data;

    lxb_tag_data_by_id_f   by_id;
    lxb_tag_data_by_name_f by_name;
};


LXB_API lxb_tag_heap_t *
lxb_tag_heap_create(void);

LXB_API lxb_status_t
lxb_tag_heap_init(lxb_tag_heap_t *tag_heap, size_t table_size);

LXB_API void
lxb_tag_heap_clean(lxb_tag_heap_t *tag_heap);

LXB_API lxb_tag_heap_t *
lxb_tag_heap_destroy(lxb_tag_heap_t *tag_heap);


LXB_API const lxb_tag_data_t *
lxb_tag_append(lxb_tag_heap_t *tag_heap, const lxb_char_t *name, size_t len);

/*
 * Append a tag without name copying.
 * Name should always be created only using the local 'mraw'.
 * For get local 'mraw' use 'lxb_tag_heap_mraw' function.
 */
LXB_API const lxb_tag_data_t *
lxb_tag_append_wo_copy(lxb_tag_heap_t *tag_heap, lxb_char_t *name, size_t len);


/*
 * Inline functions
 */
lxb_inline const lxb_tag_data_t *
lxb_tag_data_by_id(lxb_tag_heap_t *tag_heap, lxb_tag_id_t tag_id)
{
    return tag_heap->by_id(tag_heap, tag_id);
}

lxb_inline const lxb_tag_data_t *
lxb_tag_data_by_name(lxb_tag_heap_t *tag_heap,
                     const lxb_char_t *name, size_t len)
{
    return tag_heap->by_name(tag_heap, name, len);
}

lxb_inline const lxb_char_t *
lxb_tag_name_by_id(lxb_tag_heap_t *tag_heap, lxb_tag_id_t tag_id, size_t *len)
{
    const lxb_tag_data_t *data = tag_heap->by_id(tag_heap, tag_id);
    if (data == NULL) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len != NULL) {
        *len = data->name_len;
    }

    return data->name;
}

lxb_inline const lxb_char_t *
lxb_tag_name_upper_by_id(lxb_tag_heap_t *tag_heap,
                         lxb_tag_id_t tag_id, size_t *len)
{
    const lxb_tag_data_t *data = tag_heap->by_id(tag_heap, tag_id);
    if (data == NULL) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len != NULL) {
        *len = data->name_len;
    }

    return data->name_upper;
}

lxb_inline lxb_tag_id_t
lxb_tag_id_by_name(lxb_tag_heap_t *tag_heap, const lxb_char_t *name, size_t len)
{
    const lxb_tag_data_t *data = tag_heap->by_name(tag_heap, name, len);
    if (data == NULL) {
        return LXB_TAG__UNDEF;
    }

    return data->tag_id;
}

lxb_inline lexbor_mraw_t *
lxb_tag_heap_mraw(lxb_tag_heap_t *tag_heap)
{
    return lexbor_shbst_keys(tag_heap->heap);
}

lxb_inline const lxb_tag_data_t *
lxb_tag_find_or_append(lxb_tag_heap_t *tag_heap,
                       const lxb_char_t *name, size_t len)
{
    const lxb_tag_data_t *tag_data;

    tag_data = lxb_tag_data_by_name(tag_heap, name, len);
    if (tag_data != NULL) {
        return tag_data;
    }

    return lxb_tag_append(tag_heap, name, len);
}

/*
 * No inline functions for ABI.
 */
const lxb_tag_data_t *
lxb_tag_data_by_id_noi(lxb_tag_heap_t *tag_heap, lxb_tag_id_t tag_id);

const lxb_tag_data_t *
lxb_tag_data_by_name_noi(lxb_tag_heap_t *tag_heap,
                         const lxb_char_t *name, size_t len);

const lxb_char_t *
lxb_tag_name_by_id_noi(lxb_tag_heap_t *tag_heap, lxb_tag_id_t tag_id,
                       size_t *len);

const lxb_char_t *
lxb_tag_name_upper_by_id_noi(lxb_tag_heap_t *tag_heap,
                             lxb_tag_id_t tag_id, size_t *len);

lxb_tag_id_t
lxb_tag_id_by_name_noi(lxb_tag_heap_t *tag_heap,
                       const lxb_char_t *name, size_t len);

lexbor_mraw_t *
lxb_tag_heap_mraw_noi(lxb_tag_heap_t *tag_heap);

const lxb_tag_data_t *
lxb_tag_find_or_append_noi(lxb_tag_heap_t *tag_heap,
                           const lxb_char_t *name, size_t len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_TAG_H */
