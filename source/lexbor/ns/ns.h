/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_NS_H
#define LEXBOR_NS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/base.h"
#include "lexbor/core/shbst.h"
#include "lexbor/core/shs.h"

#include "lexbor/ns/const.h"


typedef struct lxb_ns_heap lxb_ns_heap_t;
typedef struct lxb_ns_data lxb_ns_data_t;


typedef const lxb_ns_data_t *
(*lxb_ns_data_by_id_f)(lxb_ns_heap_t *ns_heap, lxb_ns_id_t ns_id);

typedef const lxb_ns_data_t *
(*lxb_ns_data_by_link_f)(lxb_ns_heap_t *ns_heap,
                         const lxb_char_t *name, size_t len);


struct lxb_ns_data {
    const char  *name;
    const char  *name_lower;
    size_t      name_len;

    const char  *link;
    size_t      link_len;

    lxb_ns_id_t ns_id;
};

struct lxb_ns_heap {
    lexbor_shbst_t           *heap_link;
    lexbor_dobject_t         *data;

    lxb_ns_data_by_id_f      by_id;
    lxb_ns_data_by_link_f    by_link;
};


LXB_API lxb_ns_heap_t *
lxb_ns_heap_create(void);

LXB_API lxb_status_t
lxb_ns_heap_init(lxb_ns_heap_t *ns_heap, size_t table_size);

LXB_API void
lxb_ns_heap_clean(lxb_ns_heap_t *ns_heap);

LXB_API lxb_ns_heap_t *
lxb_ns_heap_destroy(lxb_ns_heap_t *ns_heap);


const lxb_ns_data_t *
lxb_ns_append(lxb_ns_heap_t *ns_heap, const lxb_char_t *link, size_t link_len,
              const lxb_char_t *name, size_t name_len);


LXB_API const lxb_char_t *
lxb_ns_name_by_id(lxb_ns_heap_t *ns_heap, lxb_ns_id_t ns_id, size_t *len);

LXB_API const lxb_char_t *
lxb_ns_lower_name_by_id(lxb_ns_heap_t *ns_heap, lxb_ns_id_t ns_id, size_t *len);

LXB_API const lxb_char_t *
lxb_ns_link_by_id(lxb_ns_heap_t *ns_heap, lxb_ns_id_t ns_id, size_t *len);

LXB_API lxb_ns_id_t
lxb_ns_id_by_link(lxb_ns_heap_t *ns_heap, const lxb_char_t *link, size_t len);


LXB_API const lxb_ns_data_t *
lxb_ns_data_by_id_default(lxb_ns_heap_t *ns_heap, lxb_ns_id_t ns_id);

LXB_API const lxb_ns_data_t *
lxb_ns_data_by_link_default(lxb_ns_heap_t *ns_heap,
                            const lxb_char_t *name, size_t len);


/*
 * Inline functions
 */
lxb_inline const lxb_ns_data_t *
lxb_ns_data_by_id(lxb_ns_heap_t *ns_heap, lxb_ns_id_t tag_id)
{
    return ns_heap->by_id(ns_heap, tag_id);
}

lxb_inline const lxb_ns_data_t *
lxb_ns_data_by_link(lxb_ns_heap_t *ns_heap, const lxb_char_t *link, size_t len)
{
    return ns_heap->by_link(ns_heap, link, len);
}

lxb_inline const lxb_ns_data_t *
lxb_ns_find_or_append(lxb_ns_heap_t *ns_heap,
                      const lxb_char_t *link, size_t link_len,
                      const lxb_char_t *name, size_t name_len)
{
    const lxb_ns_data_t *ns_data = lxb_ns_data_by_link(ns_heap, link, link_len);
    if (ns_data != NULL) {
        return ns_data;
    }

    return lxb_ns_append(ns_heap, link, link_len, name, name_len);
}

/*
 * No inline functions for ABI.
 */
const lxb_ns_data_t *
lxb_ns_data_by_id_noi(lxb_ns_heap_t *ns_heap, lxb_ns_id_t tag_id);

const lxb_ns_data_t *
lxb_ns_data_by_link_noi(lxb_ns_heap_t *ns_heap,
                        const lxb_char_t *link, size_t len);

const lxb_ns_data_t *
lxb_ns_find_or_append_noi(lxb_ns_heap_t *ns_heap,
                          const lxb_char_t *link, size_t link_len,
                          const lxb_char_t *name, size_t name_len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_NS_H */
