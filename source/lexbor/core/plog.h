/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_PLOG_H
#define LEXBOR_PLOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/array_obj.h"


typedef struct {
    const lxb_char_t *data;
    void             *context;
    unsigned         id;
}
lexbor_plog_entry_t;

typedef struct {
    lexbor_array_obj_t list;
}
lexbor_plog_t;


LXB_API lxb_status_t
lexbor_plog_init(lexbor_plog_t *plog, size_t init_size, size_t struct_size);

LXB_API lexbor_plog_t *
lexbor_plog_destroy(lexbor_plog_t *plog, bool self_destroy);


/*
 * Inline functions
 */
lxb_inline lexbor_plog_t *
lexbor_plog_create(void)
{
    return lexbor_calloc(1, sizeof(lexbor_plog_t));
}

lxb_inline void
lexbor_plog_clean(lexbor_plog_t *plog)
{
    lexbor_array_obj_clean(&plog->list);
}

lxb_inline void *
lexbor_plog_push(lexbor_plog_t *plog, const lxb_char_t *data, void *ctx,
                 unsigned id)
{
    if (plog == NULL) {
        return NULL;
    }

    lexbor_plog_entry_t *entry = (lexbor_plog_entry_t *) lexbor_array_obj_push(&plog->list);
    if (entry == NULL) {
        return NULL;
    }

    entry->data = data;
    entry->context = ctx;
    entry->id = id;

    return (void *) entry;
}

lxb_inline size_t
lexbor_plog_length(lexbor_plog_t *plog)
{
    return lexbor_array_obj_length(&plog->list);
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_PLOG_H */

