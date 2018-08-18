/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_IN_H
#define LEXBOR_IN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/core/base.h>
#include <lexbor/core/dobject.h>


typedef struct lexbor_in_node lexbor_in_node_t;

typedef enum {
    LEXBOR_IN_OPT_UNDEF    = 0x00,
    LEXBOR_IN_OPT_READONLY = 0x01,
    LEXBOR_IN_OPT_DONE     = 0x02,
    LEXBOR_IN_OPT_FAKE     = 0x04
}
lexbor_in_opt_t;

typedef struct {
    lexbor_dobject_t *nodes;
}
lexbor_in_t;

struct lexbor_in_node {
    size_t            offset;
    lexbor_in_opt_t   opt;

    const lxb_char_t  *begin;
    const lxb_char_t  *end;
    const lxb_char_t  *use;

    lexbor_in_node_t  *next;
    lexbor_in_node_t  *prev;

    lexbor_in_t       *incoming;
};


lexbor_in_t *
lexbor_in_create(void);

lxb_status_t
lexbor_in_init(lexbor_in_t *incoming, size_t chunk_size);

void
lexbor_in_clean(lexbor_in_t *incoming);

lexbor_in_t *
lexbor_in_destroy(lexbor_in_t *incoming, bool self_destroy);


lexbor_in_node_t *
lexbor_in_node_make(lexbor_in_t *incoming, lexbor_in_node_t *last_node,
                    const lxb_char_t *buf, size_t buf_size);

void
lexbor_in_node_clean(lexbor_in_node_t *node);

lexbor_in_node_t *
lexbor_in_node_destroy(lexbor_in_t *incoming,
                       lexbor_in_node_t *node, bool self_destroy);


lexbor_in_node_t *
lexbor_in_node_split(lexbor_in_node_t *node, const lxb_char_t *pos);

lexbor_in_node_t *
lexbor_in_node_find(lexbor_in_node_t *node, const lxb_char_t *pos);

/**
 * Get position by `offset`.
 * If position outside of nodes return `begin` position of first node
 * in nodes chain.
 */
const lxb_char_t *
lexbor_in_node_pos_up(lexbor_in_node_t *node, lexbor_in_node_t **return_node,
                      const lxb_char_t *pos, size_t offset);

/**
 * Get position by `offset`.
 * If position outside of nodes return `end`
 * position of last node in nodes chain.
 */
const lxb_char_t *
lexbor_in_node_pos_down(lexbor_in_node_t *node, lexbor_in_node_t **return_node,
                        const lxb_char_t *pos, size_t offset);

/*
 * Inline functions
 */
lxb_inline const lxb_char_t *
lexbor_in_node_begin(const lexbor_in_node_t *node)
{
    return node->begin;
}

lxb_inline const lxb_char_t *
lexbor_in_node_end(const lexbor_in_node_t *node)
{
    return node->end;
}

lxb_inline size_t
lexbor_in_node_offset(const lexbor_in_node_t *node)
{
    return node->offset;
}

lxb_inline lexbor_in_node_t *
lexbor_in_node_next(const lexbor_in_node_t *node)
{
    return node->next;
}

lxb_inline lexbor_in_node_t *
lexbor_in_node_prev(const lexbor_in_node_t *node)
{
    return node->prev;
}

lxb_inline lexbor_in_t *
lexbor_in_node_in(const lexbor_in_node_t *node)
{
    return node->incoming;
}

lxb_inline bool
lexbor_in_segment(const lexbor_in_node_t *node, const lxb_char_t *data)
{
    return node->begin <= data && node->end >= data;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_IN_H */
