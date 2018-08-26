/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/tag/tag.h"


lxb_inline void
lxb_tag_data_set_default(lxb_tag_heap_t *tag_heap, lxb_tag_data_t *data,
                         lexbor_shbst_entry_t *entry, size_t len);


lxb_tag_heap_t *
lxb_tag_heap_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_tag_heap_t));
}

lxb_status_t
lxb_tag_heap_init(lxb_tag_heap_t *tag_heap, size_t table_size,
                  const lxb_tag_data_t *static_data,
                  const lexbor_shs_entry_t *static_heap,
                  lxb_tag_category_t default_cats)
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

    lxb_tag_heap_ref(tag_heap);

    tag_heap->heap = lexbor_shbst_create();
    tag_heap->static_data = static_data;
    tag_heap->static_heap = static_heap;
    tag_heap->default_cats = default_cats;

    return lexbor_shbst_init(tag_heap->heap, table_size);
}

lxb_tag_heap_t *
lxb_tag_heap_ref(lxb_tag_heap_t *tag_heap)
{
    if (tag_heap == NULL) {
        return NULL;
    }

    tag_heap->ref_count++;

    return tag_heap;
}

lxb_tag_heap_t *
lxb_tag_heap_unref(lxb_tag_heap_t *tag_heap, bool self_destroy)
{
    if (tag_heap == NULL || tag_heap->ref_count == 0) {
        return NULL;
    }

    tag_heap->ref_count--;

    if (tag_heap->ref_count == 0) {
        lxb_tag_heap_destroy(tag_heap, self_destroy);
    }

    return NULL;
}

void
lxb_tag_heap_clean(lxb_tag_heap_t *tag_heap)
{
    if (tag_heap == NULL) {
        return;
    }

    if (tag_heap->ref_count == 1) {
        lexbor_dobject_clean(tag_heap->data);
        lexbor_shbst_clean(tag_heap->heap);
    }
}

lxb_tag_heap_t *
lxb_tag_heap_destroy(lxb_tag_heap_t *tag_heap, bool self_destroy)
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
lxb_tag_data_set_default(lxb_tag_heap_t *tag_heap, lxb_tag_data_t *data,
                         lexbor_shbst_entry_t *entry, size_t len)
{
    data->name = entry->key;
    data->name_len = len;

    data->tag_id = LXB_TAG__LAST_ENTRY
        + (unsigned int) (lexbor_dobject_allocated(tag_heap->data) - 1);

    for (size_t i = 0; i < LXB_NS__LAST_ENTRY; i++) {
        data->cats[i] = tag_heap->default_cats;
    }

    memset(data->interface, 0x00, sizeof(data->interface));
}

lxb_tag_data_t *
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

    lxb_tag_data_set_default(tag_heap, data, entry, len);

    return data;
}

lxb_tag_data_t *
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

    lxb_tag_data_set_default(tag_heap, data, entry, len);

    return data;
}

const lxb_tag_data_t *
lxb_tag_data_by_id(lxb_tag_heap_t *tag_heap, lxb_tag_id_t tag_id)
{
    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        return lexbor_dobject_by_absolute_position(tag_heap->data,
                                                   tag_id - LXB_TAG__LAST_ENTRY);
    }

    if (tag_heap->static_data == NULL) {
        return NULL;
    }

    return &tag_heap->static_data[tag_id];
}

const lxb_tag_data_t *
lxb_tag_data_by_name(lxb_tag_heap_t *tag_heap,
                     const lxb_char_t *name, size_t len)
{
    lexbor_shbst_entry_t *hp_entry;
    const lexbor_shs_entry_t *entry;

    if (name == NULL || len == 0) {
        return NULL;
    }

    if (tag_heap->static_heap != NULL) {
        entry = lexbor_shs_entry_get_static(tag_heap->static_heap, name, len);
        if (entry != NULL) {
            return (const lxb_tag_data_t *) entry->value;
        }
    }

    hp_entry = lexbor_shbst_search(tag_heap->heap, name, len, true);
    if (hp_entry == NULL) {
        return NULL;
    }

    return hp_entry->value;
}

const lxb_char_t *
lxb_tag_name_by_id(lxb_tag_heap_t *tag_heap,
                   lxb_tag_id_t tag_id, lxb_ns_id_t ns, size_t *len)
{
    const lxb_tag_data_t *entry;

    if (tag_id >= LXB_TAG__LAST_ENTRY) {
        entry = lexbor_dobject_by_absolute_position(tag_heap->data,
                                                    tag_id - LXB_TAG__LAST_ENTRY);
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

    if (tag_heap->static_data == NULL) {
        return NULL;
    }

    if (tag_heap->static_data[tag_id].fixname[ns] != NULL) {
        if (len != NULL) {
            *len = tag_heap->static_data[tag_id].fixname[ns]->length;
        }

        return tag_heap->static_data[tag_id].fixname[ns]->data;
    }

    if (len != NULL) {
        *len = tag_heap->static_data[tag_id].name_len;
    }

    return (const lxb_char_t *) tag_heap->static_data[tag_id].name;
}

lxb_tag_id_t
lxb_tag_id_by_name(lxb_tag_heap_t *tag_heap, const lxb_char_t *name, size_t len)
{
    lexbor_shbst_entry_t *hp_entry;
    const lexbor_shs_entry_t *entry;

    if (name == NULL || len == 0) {
        return LXB_TAG__UNDEF;
    }

    if (tag_heap->static_heap != NULL) {
        entry = lexbor_shs_entry_get_static(tag_heap->static_heap, name, len);
        if (entry != NULL) {
            return ((const lxb_tag_data_t *) entry->value)->tag_id;
        }
    }

    hp_entry = lexbor_shbst_search(tag_heap->heap, name, len, true);
    if (hp_entry == NULL) {
        return LXB_TAG__UNDEF;
    }

    return ((const lxb_tag_data_t *) hp_entry->value)->tag_id;
}

bool
lxb_tag_is_void(lxb_tag_id_t tag_id)
{
    switch (tag_id) {
        case LXB_TAG_AREA:
        case LXB_TAG_BASE:
        case LXB_TAG_BR:
        case LXB_TAG_COL:
        case LXB_TAG_EMBED:
        case LXB_TAG_HR:
        case LXB_TAG_IMG:
        case LXB_TAG_INPUT:
        case LXB_TAG_LINK:
        case LXB_TAG_META:
        case LXB_TAG_PARAM:
        case LXB_TAG_SOURCE:
        case LXB_TAG_TRACK:
        case LXB_TAG_WBR:
            return true;

        default:
            return false;
    }

    return false;
}
