/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/core/shs.h"

#include "lexbor/html/tag.h"

#define LXB_HTML_TAG_RES_DATA
#define LXB_HTML_TAG_RES_SHS_DATA
#include "lexbor/html/tag_res.h"

lxb_inline void
lxb_html_tag_data_set_default(lxb_html_tag_heap_t *tag_heap,
                              lxb_html_tag_data_t *data,
                              lexbor_shbst_entry_t *entry, size_t len);


lxb_html_tag_heap_t *
lxb_html_tag_heap_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_html_tag_heap_t));
}

lxb_status_t
lxb_html_tag_heap_init(lxb_html_tag_heap_t *tag_heap, size_t table_size)
{
    lxb_status_t status;

    if (tag_heap == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    if (table_size == 0) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    tag_heap->data = lexbor_dobject_create();
    status = lexbor_dobject_init(tag_heap->data, 128,
                                 sizeof(lxb_html_tag_data_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    lxb_html_tag_heap_ref(tag_heap);

    tag_heap->heap = lexbor_shbst_create();
    return lexbor_shbst_init(tag_heap->heap, table_size);
}

lxb_html_tag_heap_t *
lxb_html_tag_heap_ref(lxb_html_tag_heap_t *tag_heap)
{
    if (tag_heap == NULL) {
        return NULL;
    }

    tag_heap->ref_count++;
    return tag_heap;
}

lxb_html_tag_heap_t *
lxb_html_tag_heap_unref(lxb_html_tag_heap_t *tag_heap, bool self_destroy)
{
    if (tag_heap == NULL || tag_heap->ref_count == 0) {
        return NULL;
    }

    tag_heap->ref_count--;

    if (tag_heap->ref_count == 0) {
        lxb_html_tag_heap_destroy(tag_heap, self_destroy);
    }

    return NULL;
}

void
lxb_html_tag_heap_clean(lxb_html_tag_heap_t *tag_heap)
{
    if (tag_heap == NULL) {
        return;
    }

    if (tag_heap->ref_count == 1) {
        lexbor_dobject_clean(tag_heap->data);
        lexbor_shbst_clean(tag_heap->heap);
    }
}

lxb_html_tag_heap_t *
lxb_html_tag_heap_destroy(lxb_html_tag_heap_t *tag_heap, bool self_destroy)
{
    if (tag_heap == NULL) {
        return NULL;
    }

    tag_heap->data = lexbor_dobject_destroy(tag_heap->data, true);
    tag_heap->heap = lexbor_shbst_destroy(tag_heap->heap, true);

    if (self_destroy) {
        return lexbor_free(tag_heap);
    }

    return tag_heap;
}

lxb_inline void
lxb_html_tag_data_set_default(lxb_html_tag_heap_t *tag_heap,
                              lxb_html_tag_data_t *data,
                              lexbor_shbst_entry_t *entry, size_t len)
{
    data->name = entry->key;
    data->name_len = len;
    
    data->tag_id = LXB_HTML_TAG__LAST_ENTRY
    + (unsigned int) (lexbor_dobject_allocated(tag_heap->data) - 1);
    
    for (size_t i = 0; i < LXB_HTML_NS__LAST_ENTRY; i++) {
        data->cats[i] = LXB_HTML_TAG_CATEGORY_ORDINARY
        |LXB_HTML_TAG_CATEGORY_SCOPE_SELECT;
    }
    
    memset(data->interface, 0x00, sizeof(data->interface));
}

lxb_html_tag_data_t *
lxb_html_tag_append(lxb_html_tag_heap_t *tag_heap,
                    const lxb_char_t *name, size_t len)
{
    lxb_html_tag_data_t *data;
    lexbor_shbst_entry_t *entry;

    if (name == NULL || len == 0) {
        return NULL;
    }

    data = lexbor_dobject_calloc(tag_heap->data);
    if (data == NULL) {
        return NULL;
    }

    entry = lexbor_shbst_insert(tag_heap->heap, name, len, data);
    if (entry == NULL) {
        return NULL;
    }

    lxb_html_tag_data_set_default(tag_heap, data, entry, len);

    return data;
}

lxb_html_tag_data_t *
lxb_html_tag_append_wo_copy(lxb_html_tag_heap_t *tag_heap,
                            lxb_char_t *name, size_t len)
{
    lxb_html_tag_data_t *data;
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

    lxb_html_tag_data_set_default(tag_heap, data, entry, len);

    return data;
}

const lxb_html_tag_data_t *
lxb_html_tag_data_by_id(lxb_html_tag_heap_t *tag_heap, lxb_html_tag_id_t tag_id)
{
    if (tag_id >= LXB_HTML_TAG__LAST_ENTRY) {
        if (tag_heap == NULL) {
            return NULL;
        }

        return lexbor_dobject_by_absolute_position(tag_heap->data,
                                             tag_id - LXB_HTML_TAG__LAST_ENTRY);
    }

    return &lxb_html_tag_res_data[tag_id];
}

const lxb_html_tag_data_t *
lxb_html_tag_data_by_name(lxb_html_tag_heap_t *tag_heap,
                          const lxb_char_t *name, size_t len)
{
    lexbor_shbst_entry_t *hp_entry;
    const lexbor_shs_entry_t *entry;

    if (name == NULL || len == 0) {
        return NULL;
    }

    entry = lexbor_shs_entry_get_static(lxb_html_tag_res_shs_data, name, len);
    if (entry != NULL) {
        return (const lxb_html_tag_data_t *) entry->value;
    }

    if (tag_heap == NULL) {
        return NULL;
    }

    hp_entry = lexbor_shbst_search(tag_heap->heap, name, len, true);
    if (hp_entry == NULL) {
        return NULL;
    }

    return hp_entry->value;
}

const lxb_char_t *
lxb_html_tag_name_by_id(lxb_html_tag_heap_t *tag_heap,
                        lxb_html_tag_id_t tag_id, size_t *len)
{
    const lxb_html_tag_data_t *entry;

    if (tag_id >= LXB_HTML_TAG__LAST_ENTRY) {
        if (tag_heap == NULL) {
            return NULL;
        }

        entry = lexbor_dobject_by_absolute_position(tag_heap->data,
                                             tag_id - LXB_HTML_TAG__LAST_ENTRY);
        if (entry == NULL) {
            if (len != NULL) {
                *len = 0;
            }

            return NULL;
        }

        if (len != NULL) {
            *len = entry->name_len;
        }

        return entry->name;
    }

    if (len != NULL) {
        *len = lxb_html_tag_res_data[tag_id].name_len;
    }

    return (const lxb_char_t *) lxb_html_tag_res_data[tag_id].name;
}

lxb_html_tag_id_t
lxb_html_tag_id_by_name(lxb_html_tag_heap_t *tag_heap,
                        const lxb_char_t *name, size_t len)
{
    lexbor_shbst_entry_t *hp_entry;
    const lexbor_shs_entry_t *entry;

    if (name == NULL || len == 0) {
        return LXB_HTML_TAG__UNDEF;
    }

    entry = lexbor_shs_entry_get_static(lxb_html_tag_res_shs_data, name, len);
    if (entry != NULL) {
        return ((const lxb_html_tag_data_t *) entry->value)->tag_id;
    }

    if (tag_heap == NULL) {
        return LXB_HTML_TAG__UNDEF;
    }

    hp_entry = lexbor_shbst_search(tag_heap->heap, name, len, true);
    if (hp_entry == NULL) {
        return LXB_HTML_TAG__UNDEF;
    }

    return ((const lxb_html_tag_data_t *) hp_entry->value)->tag_id;
}

bool
lxb_html_tag_is_void(lxb_html_tag_id_t tag_id)
{
    switch (tag_id) {
        case LXB_HTML_TAG_AREA:
        case LXB_HTML_TAG_BASE:
        case LXB_HTML_TAG_BR:
        case LXB_HTML_TAG_COL:
        case LXB_HTML_TAG_EMBED:
        case LXB_HTML_TAG_HR:
        case LXB_HTML_TAG_IMG:
        case LXB_HTML_TAG_INPUT:
        case LXB_HTML_TAG_LINK:
        case LXB_HTML_TAG_META:
        case LXB_HTML_TAG_PARAM:
        case LXB_HTML_TAG_SOURCE:
        case LXB_HTML_TAG_TRACK:
        case LXB_HTML_TAG_WBR:
            return true;

        default:
            return false;
    }

    return false;
}
