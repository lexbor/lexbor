/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/tag/tag.h"

#define LXB_TAG_RES_DATA_DEFAULT
#define LXB_TAG_RES_SHS_DATA_DEFAULT
#include "lexbor/tag/res.h"


lxb_inline void
lxb_tag_data_set_attr(lxb_tag_heap_t *tag_heap, lxb_tag_data_t *data,
                      lexbor_shbst_entry_t *entry, size_t len);

static const lxb_tag_data_t *
lxb_tag_data_by_id_default_cb(lxb_tag_heap_t *tag_heap, lxb_tag_id_t tag_id);

static const lxb_tag_data_t *
lxb_tag_data_by_name_default_cb(lxb_tag_heap_t *tag_heap,
                                const lxb_char_t *name, size_t len);


lxb_tag_heap_t *
lxb_tag_heap_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_tag_heap_t));
}

lxb_status_t
lxb_tag_heap_init(lxb_tag_heap_t *tag_heap, size_t table_size)
{
    lxb_status_t status;

    if (tag_heap == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    if (table_size == 0) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    tag_heap->data = lexbor_dobject_create();
    status = lexbor_dobject_init(tag_heap->data, 128, sizeof(lxb_tag_data_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    tag_heap->by_id = lxb_tag_data_by_id_default_cb;
    tag_heap->by_name = lxb_tag_data_by_name_default_cb;

    tag_heap->heap = lexbor_shbst_create();

    return lexbor_shbst_init(tag_heap->heap, table_size);
}

void
lxb_tag_heap_clean(lxb_tag_heap_t *tag_heap)
{
    if (tag_heap == NULL) {
        return;
    }

    lexbor_dobject_clean(tag_heap->data);
    lexbor_shbst_clean(tag_heap->heap);
}

lxb_tag_heap_t *
lxb_tag_heap_destroy(lxb_tag_heap_t *tag_heap)
{
    if (tag_heap == NULL) {
        return NULL;
    }

    tag_heap->data = lexbor_dobject_destroy(tag_heap->data, true);
    tag_heap->heap = lexbor_shbst_destroy(tag_heap->heap, true);

    return lexbor_free(tag_heap);
}

lxb_inline void
lxb_tag_data_set_attr(lxb_tag_heap_t *tag_heap, lxb_tag_data_t *data,
                      lexbor_shbst_entry_t *entry, size_t len)
{
    data->name = entry->key;
    data->name_len = len;

    data->tag_id = LXB_TAG__LAST_ENTRY
        + (unsigned int) (lexbor_dobject_allocated(tag_heap->data) - 1);
}

const lxb_tag_data_t *
lxb_tag_append(lxb_tag_heap_t *tag_heap, const lxb_char_t *name, size_t len)
{
    lxb_tag_data_t *data;
    lexbor_shbst_entry_t *entry;

    if (name == NULL || len == 0) {
        return NULL;
    }

    data = lexbor_dobject_calloc(tag_heap->data);
    if (data == NULL) {
        return NULL;
    }

    entry = lexbor_shbst_insert_lowercase(tag_heap->heap, name, len, data);
    if (entry == NULL) {
        return NULL;
    }

    lxb_tag_data_set_attr(tag_heap, data, entry, len);

    return data;
}

const lxb_tag_data_t *
lxb_tag_append_wo_copy(lxb_tag_heap_t *tag_heap, lxb_char_t *name, size_t len)
{
    lxb_tag_data_t *data;
    lexbor_shbst_entry_t *entry;

    if (name == NULL || len == 0) {
        return NULL;
    }

    data = lexbor_dobject_calloc(tag_heap->data);
    if (data == NULL) {
        return NULL;
    }

    entry = lexbor_shbst_insert_wo_copy(tag_heap->heap, name, len, data);
    if (entry == NULL) {
        return NULL;
    }

    lxb_tag_data_set_attr(tag_heap, data, entry, len);

    return data;
}

static const lxb_tag_data_t *
lxb_tag_data_by_id_default_cb(lxb_tag_heap_t *tag_heap, lxb_tag_id_t tag_id)
{
    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        return lexbor_dobject_by_absolute_position(tag_heap->data,
                                                   tag_id - LXB_TAG__LAST_ENTRY);
    }

    return &lxb_tag_res_data_default[tag_id];
}

static const lxb_tag_data_t *
lxb_tag_data_by_name_default_cb(lxb_tag_heap_t *tag_heap,
                                const lxb_char_t *name, size_t len)
{
    const lexbor_shbst_entry_t *hp_entry;
    const lexbor_shs_entry_t *entry;

    if (name == NULL || len == 0) {
        return NULL;
    }

    entry = lexbor_shs_entry_get_static(lxb_tag_res_shs_data_default,
                                        name, len);
    if (entry != NULL) {
        return (const lxb_tag_data_t *) entry->value;
    }

    hp_entry = lexbor_shbst_search(tag_heap->heap, name, len, true);
    if (hp_entry == NULL) {
        return NULL;
    }

    return hp_entry->value;
}

/*
 * No inline functions for ABI.
 */
const lxb_tag_data_t *
lxb_tag_data_by_id_noi(lxb_tag_heap_t *tag_heap, lxb_tag_id_t tag_id)
{
    return lxb_tag_data_by_id(tag_heap, tag_id);
}

const lxb_tag_data_t *
lxb_tag_data_by_name_noi(lxb_tag_heap_t *tag_heap,
                         const lxb_char_t *name, size_t len)
{
    return lxb_tag_data_by_name(tag_heap, name, len);
}


const lxb_char_t *
lxb_tag_name_by_id_noi(lxb_tag_heap_t *tag_heap, lxb_tag_id_t tag_id,
                       size_t *len)
{
    return lxb_tag_name_by_id(tag_heap, tag_id, len);
}

const lxb_char_t *
lxb_tag_name_upper_by_id_noi(lxb_tag_heap_t *tag_heap,
                             lxb_tag_id_t tag_id, size_t *len)
{
    return lxb_tag_name_upper_by_id(tag_heap, tag_id, len);
}

lxb_tag_id_t
lxb_tag_id_by_name_noi(lxb_tag_heap_t *tag_heap,
                       const lxb_char_t *name, size_t len)
{
    return lxb_tag_id_by_name(tag_heap, name, len);
}

lexbor_mraw_t *
lxb_tag_heap_mraw_noi(lxb_tag_heap_t *tag_heap)
{
    return lxb_tag_heap_mraw(tag_heap);
}

const lxb_tag_data_t *
lxb_tag_find_or_append_noi(lxb_tag_heap_t *tag_heap,
                           const lxb_char_t *name, size_t len)
{
    return lxb_tag_find_or_append(tag_heap, name, len);
}
