/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_HTML_TOKENIZER_H
#define LEXBOR_HTML_TOKENIZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/in.h"
#include "lexbor/core/sbst.h"
#include "lexbor/core/array_obj.h"

#include "lexbor/html/base.h"
#include "lexbor/html/token.h"
#include "lexbor/html/tag.h"


typedef struct lxb_html_tokenizer lxb_html_tokenizer_t;


/* State */
typedef const lxb_char_t *
(*lxb_html_tokenizer_state_f)(lxb_html_tokenizer_t *tkz,
                              const lxb_char_t *data, const lxb_char_t *end);

typedef lxb_html_token_t *
(*lxb_html_tokenizer_token_f)(lxb_html_tokenizer_t *tkz,
                              lxb_html_token_t *token, void *ctx);


struct lxb_html_tokenizer {
    lxb_html_tokenizer_state_f       state;
    lxb_html_tokenizer_state_f       state_return;

    lxb_html_tokenizer_token_f       callback_token_done;
    void                             *callback_token_ctx;

    /*
     * A tokenizer create an lxb_html_tag_heap_t object if not it eq NULL
     */
    lxb_html_tag_heap_t              *tag_heap_ref;

    /* For a temp strings and other templary data */
    lexbor_mraw_t                    *mraw;

    /* Current process token */
    lxb_html_token_t                 *token;

    /* Memory for token and attr */
    lexbor_dobject_t                 *dobj_token;
    lexbor_dobject_t                 *dobj_token_attr;

    /* Incoming Buffer and current process buffer */
    lexbor_in_t                      *incoming;
    lexbor_in_node_t                 *incoming_node;

    /* Parse error */
    lexbor_array_obj_t               *parse_errors;

    /*
     * Leak abstractions.
     * The only place where the specification causes mixing Tree Builder
     * and Tokenizer. We kill all beauty.
     * Current Tree parser. This is not ref (not ref count).
     */
    lxb_html_tree_t                  *tree;

    /* Temp */
    const lxb_char_t                 *markup;
    lexbor_in_node_t                 *tmp_incoming_node;
    lxb_html_tag_id_t                tmp_tag_id;

    /* Entities */
    const lexbor_sbst_entry_static_t *entity;
    const lxb_char_t                 *entity_pos;
    const lxb_char_t                 *entity_value;

    /* Process */
    lxb_status_t                     status;
    bool                             is_eof;

    lxb_html_tokenizer_t             *base;
    size_t                           ref_count;
};


#include "lexbor/html/tokenizer/error.h"


extern const lxb_char_t *lxb_html_tokenizer_eof;

lxb_html_tokenizer_t *
lxb_html_tokenizer_create(void);

lxb_status_t
lxb_html_tokenizer_init(lxb_html_tokenizer_t *tkz);

lxb_status_t
lxb_html_tokenizer_inherit(lxb_html_tokenizer_t *tkz_to,
                           lxb_html_tokenizer_t *tkz_from);

lxb_html_tokenizer_t *
lxb_html_tokenizer_ref(lxb_html_tokenizer_t *tkz);

lxb_html_tokenizer_t *
lxb_html_tokenizer_unref(lxb_html_tokenizer_t *tkz, bool self_destroy);

void
lxb_html_tokenizer_clean(lxb_html_tokenizer_t *tkz);

lxb_html_tokenizer_t *
lxb_html_tokenizer_destroy(lxb_html_tokenizer_t *tkz, bool self_destroy);


lxb_status_t
lxb_html_tokenizer_begin(lxb_html_tokenizer_t *tkz);

lxb_status_t
lxb_html_tokenizer_chunk(lxb_html_tokenizer_t *tkz,
                         const lxb_char_t *data, size_t size);

lxb_status_t
lxb_html_tokenizer_end(lxb_html_tokenizer_t *tkz);


const lxb_char_t *
lxb_html_tokenizer_change_incoming(lxb_html_tokenizer_t *tkz,
                                   const lxb_char_t *pos);

lxb_html_ns_id_t
lxb_html_tokenizer_current_namespace(lxb_html_tokenizer_t *tkz);

void
lxb_html_tokenizer_set_state_by_tag(lxb_html_tokenizer_t *tkz, bool scripting,
                                    lxb_html_tag_id_t tag_id,
                                    lxb_html_ns_id_t ns);


/*
 * Inline functions
 */
lxb_inline void
lxb_html_tokenizer_status_set(lxb_html_tokenizer_t *tkz, lxb_status_t status)
{
    tkz->status = status;
}

lxb_inline void
lxb_html_tokenizer_tag_heap_set(lxb_html_tokenizer_t *tkz,
                                lxb_html_tag_heap_t *tag_heap)
{
    tkz->tag_heap_ref = lxb_html_tag_heap_ref(tag_heap);
}

lxb_inline void
lxb_html_tokenizer_callback_token_done_set(lxb_html_tokenizer_t *tkz,
                                           lxb_html_tokenizer_token_f call_func,
                                           void *ctx)
{
    tkz->callback_token_done = call_func;
    tkz->callback_token_ctx = ctx;
}

lxb_inline void *
lxb_html_tokenizer_callback_token_done_ctx(lxb_html_tokenizer_t *tkz)
{
    return tkz->callback_token_ctx;
}

lxb_inline void
lxb_html_tokenizer_state_set(lxb_html_tokenizer_t *tkz,
                             lxb_html_tokenizer_state_f state)
{
    tkz->state = state;
}

lxb_inline void
lxb_html_tokenizer_tmp_tag_id_set(lxb_html_tokenizer_t *tkz,
                                  lxb_html_tag_id_t tag_id)
{
    tkz->tmp_tag_id = tag_id;
}

lxb_inline lxb_html_tree_t *
lxb_html_tokenizer_tree(lxb_html_tokenizer_t *tkz)
{
    return tkz->tree;
}

lxb_inline void
lxb_html_tokenizer_tree_set(lxb_html_tokenizer_t *tkz, lxb_html_tree_t *tree)
{
    tkz->tree = tree;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_TOKENIZER_H */
