/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/core/shs.h"

#define LEXBOR_STR_RES_MAP_LOWERCASE
#include "lexbor/core/str_res.h"

#include "lexbor/ns/ns.h"

#define LXB_NS_RES_DATA
#define LXB_NS_RES_SHS_LINK_DATA
#include "lexbor/ns/res.h"


lxb_inline lxb_status_t
lxb_ns_data_set_default(lxb_ns_heap_t *ns_heap, lxb_ns_data_t *data,
                        lexbor_shbst_entry_t *entry, size_t link_len,
                        const lxb_char_t *name, size_t name_len);


lxb_ns_heap_t *
lxb_ns_heap_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_ns_heap_t));
}

lxb_status_t
lxb_ns_heap_init(lxb_ns_heap_t *ns_heap, size_t table_size)
{
    lxb_status_t status;

    if (ns_heap == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    if (table_size == 0) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    ns_heap->data = lexbor_dobject_create();
    status = lexbor_dobject_init(ns_heap->data, 128, sizeof(lxb_ns_data_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    lxb_ns_heap_ref(ns_heap);

    /* For links */
    ns_heap->heap_link = lexbor_shbst_create();
    status = lexbor_shbst_init(ns_heap->heap_link, table_size);

    if (status != LXB_STATUS_OK) {
        return status;
    }

    ns_heap->by_id = lxb_ns_data_by_id_default;
    ns_heap->by_link = lxb_ns_data_by_link_default;

    return LXB_STATUS_OK;
}

lxb_ns_heap_t *
lxb_ns_heap_ref(lxb_ns_heap_t *ns_heap)
{
    if (ns_heap == NULL) {
        return NULL;
    }

    ns_heap->ref_count++;

    return ns_heap;
}

lxb_ns_heap_t *
lxb_ns_heap_unref(lxb_ns_heap_t *ns_heap)
{
    if (ns_heap == NULL || ns_heap->ref_count == 0) {
        return NULL;
    }

    ns_heap->ref_count--;

    if (ns_heap->ref_count == 0) {
        lxb_ns_heap_destroy(ns_heap);
    }

    return NULL;
}

void
lxb_ns_heap_clean(lxb_ns_heap_t *ns_heap)
{
    if (ns_heap == NULL) {
        return;
    }

    lexbor_dobject_clean(ns_heap->data);
    lexbor_shbst_clean(ns_heap->heap_link);
}

lxb_ns_heap_t *
lxb_ns_heap_destroy(lxb_ns_heap_t *ns_heap)
{
    if (ns_heap == NULL) {
        return NULL;
    }

    ns_heap->data = lexbor_dobject_destroy(ns_heap->data, true);
    ns_heap->heap_link = lexbor_shbst_destroy(ns_heap->heap_link, true);

    return lexbor_free(ns_heap);
}

lxb_inline lxb_status_t
lxb_ns_data_set_default(lxb_ns_heap_t *ns_heap, lxb_ns_data_t *data,
                        lexbor_shbst_entry_t *entry, size_t link_len,
                        const lxb_char_t *name, size_t name_len)
{
    data->link = (const char *) entry->key;
    data->link_len = link_len;
    data->name_len = name_len;

    data->ns_id = LXB_NS__LAST_ENTRY
        + (unsigned int) (lexbor_dobject_allocated(ns_heap->data) - 1);

    data->name = lexbor_mraw_calloc(lexbor_shbst_keys(ns_heap->heap_link),
                                    sizeof(lxb_char_t) * (data->name_len + 1));
    if (data->name == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    data->name_lower = lexbor_mraw_calloc(lexbor_shbst_keys(ns_heap->heap_link),
                                          sizeof(lxb_char_t) * (data->name_len + 1));
    if (data->name_lower == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    if (data->name_len == 0) {
        return LXB_STATUS_OK;
    }

    memcpy((lxb_char_t *) data->name, name, sizeof(lxb_char_t) * name_len);

    for (size_t i = 0; i < name_len; i++) {
        ((lxb_char_t *) data->name_lower)[i] = lexbor_str_res_map_lowercase[ name[i] ];
    }

    return LXB_STATUS_OK;
}

const lxb_ns_data_t *
lxb_ns_append(lxb_ns_heap_t *ns_heap, const lxb_char_t *link, size_t link_len,
              const lxb_char_t *name, size_t name_len)
{
    lxb_status_t status;
    lxb_ns_data_t *data;
    lexbor_shbst_entry_t *entry;

    if (link == NULL || link_len == 0) {
        return NULL;
    }

    data = lexbor_dobject_calloc(ns_heap->data);
    if (data == NULL) {
        return NULL;
    }

    entry = lexbor_shbst_insert(ns_heap->heap_link, link, link_len, data);
    if (entry == NULL) {
        lexbor_dobject_free(ns_heap->data, data);

        return NULL;
    }

    status = lxb_ns_data_set_default(ns_heap, data, entry, link_len,
                                     name, name_len);
    if (status != LXB_STATUS_OK) {
        lexbor_dobject_free(ns_heap->data, data);
    }

    return data;
}

const lxb_char_t *
lxb_ns_name_by_id(lxb_ns_heap_t *ns_heap, lxb_ns_id_t ns_id, size_t *len)
{
    const lxb_ns_data_t *entry;

    entry = ns_heap->by_id(ns_heap, ns_id);
    if (entry == NULL) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len != NULL) {
        *len = entry->name_len;
    }

    return (const lxb_char_t *) entry->name;
}

const lxb_char_t *
lxb_ns_lower_name_by_id(lxb_ns_heap_t *ns_heap, lxb_ns_id_t ns_id, size_t *len)
{
    const lxb_ns_data_t *entry;

    entry = ns_heap->by_id(ns_heap, ns_id);
    if (entry == NULL) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len != NULL) {
        *len = entry->name_len;
    }

    return (const lxb_char_t *) entry->name_lower;
}

const lxb_char_t *
lxb_ns_link_by_id(lxb_ns_heap_t *ns_heap, lxb_ns_id_t ns_id, size_t *len)
{
    const lxb_ns_data_t *entry;

    entry = ns_heap->by_id(ns_heap, ns_id);
    if (entry == NULL) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    if (len != NULL) {
        *len = entry->link_len;
    }

    return (const lxb_char_t *) entry->link;
}

lxb_ns_id_t
lxb_ns_id_by_link(lxb_ns_heap_t *ns_heap, const lxb_char_t *link, size_t len)
{
    const lxb_ns_data_t *entry;

    entry = ns_heap->by_link(ns_heap, link, len);
    if (entry == NULL) {
        return LXB_NS__UNDEF;
    }

    return entry->ns_id;
}

const lxb_ns_data_t *
lxb_ns_data_by_id_default(lxb_ns_heap_t *ns_heap, lxb_ns_id_t ns_id)
{
    if (ns_id >= LXB_NS__LAST_ENTRY) {
        return lexbor_dobject_by_absolute_position(ns_heap->data,
                                                   ns_id - LXB_NS__LAST_ENTRY);
    }

    return &lxb_ns_res_data[ns_id];
}

const lxb_ns_data_t *
lxb_ns_data_by_link_default(lxb_ns_heap_t *ns_heap,
                            const lxb_char_t *name, size_t len)
{
    const lexbor_shbst_entry_t *hp_entry;
    const lexbor_shs_entry_t *entry;

    entry = lexbor_shs_entry_get_static(lxb_ns_res_shs_link_data, name, len);
    if (entry != NULL) {
        return (const lxb_ns_data_t *) entry->value;
    }

    hp_entry = lexbor_shbst_search(ns_heap->heap_link, name, len, true);
    if (hp_entry == NULL) {
        return NULL;
    }

    return hp_entry->value;
}
