/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
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

#include "lexbor/tag/tag.h"
#include "lexbor/ns/ns.h"


typedef struct lxb_html_tokenizer lxb_html_tokenizer_t;
typedef unsigned int lxb_html_tokenizer_opt_t;


/* State */
typedef const lxb_char_t *
(*lxb_html_tokenizer_state_f)(lxb_html_tokenizer_t *tkz,
                              const lxb_char_t *data, const lxb_char_t *end);

typedef lxb_html_token_t *
(*lxb_html_tokenizer_token_f)(lxb_html_tokenizer_t *tkz,
                              lxb_html_token_t *token, void *ctx);


enum lxb_html_tokenizer_opt {
    LXB_HTML_TOKENIZER_OPT_UNDEF         = 0x00,

    /*
     * Without copying input buffer.
     * The user himself monitors the safety of buffers until the end of parsing.
     */
    LXB_HTML_TOKENIZER_OPT_WO_COPY       = 0x01,

    /*
     * During parsing, incoming buffers will not be destroyed.
     * By default, when the incoming buffer is no longer needed,
     * it is destroyed.
     */
    LXB_HTML_TOKENIZER_OPT_WO_IN_DESTROY = 0x02
};

struct lxb_html_tokenizer {
    lxb_html_tokenizer_state_f       state;
    lxb_html_tokenizer_state_f       state_return;

    lxb_html_tokenizer_token_f       callback_token_done;
    void                             *callback_token_ctx;

    lxb_tag_heap_t                   *tag_heap;

    /* For a temp strings and other templary data */
    lexbor_mraw_t                    *mraw;

    /* Current process token */
    lxb_html_token_t                 *token;

    /* Memory for token and attr */
    lexbor_dobject_t                 *dobj_token;
    lexbor_dobject_t                 *dobj_token_attr;

    /* Incoming Buffer and current process buffer */
    lexbor_in_t                      *incoming;
    lexbor_in_node_t                 *incoming_first;
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
    lxb_tag_id_t                     tmp_tag_id;

    /* Entities */
    const lexbor_sbst_entry_static_t *entity;
    const lxb_char_t                 *entity_pos;
    const lxb_char_t                 *entity_value;

    /* Process */
    lxb_html_tokenizer_opt_t         opt;
    lxb_status_t                     status;
    bool                             is_eof;
    bool                             reuse;

    lxb_html_tokenizer_t             *base;
    size_t                           ref_count;
};


#include "lexbor/html/tokenizer/error.h"


extern const lxb_char_t *lxb_html_tokenizer_eof;

LXB_API lxb_html_tokenizer_t *
lxb_html_tokenizer_create(void);

LXB_API lxb_status_t
lxb_html_tokenizer_init(lxb_html_tokenizer_t *tkz);

LXB_API lxb_status_t
lxb_html_tokenizer_inherit(lxb_html_tokenizer_t *tkz_to,
                           lxb_html_tokenizer_t *tkz_from);

LXB_API lxb_html_tokenizer_t *
lxb_html_tokenizer_ref(lxb_html_tokenizer_t *tkz);

LXB_API lxb_html_tokenizer_t *
lxb_html_tokenizer_unref(lxb_html_tokenizer_t *tkz);

LXB_API void
lxb_html_tokenizer_clean(lxb_html_tokenizer_t *tkz);

LXB_API lxb_html_tokenizer_t *
lxb_html_tokenizer_destroy(lxb_html_tokenizer_t *tkz);

LXB_API lxb_status_t
lxb_html_tokenizer_tag_heap_make(lxb_html_tokenizer_t *tkz, size_t table_size);

LXB_API void
lxb_html_tokenizer_tag_heap_destroy(lxb_html_tokenizer_t *tkz);

LXB_API lxb_status_t
lxb_html_tokenizer_begin(lxb_html_tokenizer_t *tkz);

LXB_API lxb_status_t
lxb_html_tokenizer_chunk(lxb_html_tokenizer_t *tkz,
                         const lxb_char_t *data, size_t size);

LXB_API lxb_status_t
lxb_html_tokenizer_end(lxb_html_tokenizer_t *tkz);


LXB_API const lxb_char_t *
lxb_html_tokenizer_change_incoming(lxb_html_tokenizer_t *tkz,
                                   const lxb_char_t *pos);

LXB_API lxb_ns_id_t
lxb_html_tokenizer_current_namespace(lxb_html_tokenizer_t *tkz);

LXB_API void
lxb_html_tokenizer_set_state_by_tag(lxb_html_tokenizer_t *tkz, bool scripting,
                                    lxb_tag_id_t tag_id, lxb_ns_id_t ns);


/*
 * Inline functions
 */
lxb_inline void
lxb_html_tokenizer_status_set(lxb_html_tokenizer_t *tkz, lxb_status_t status)
{
    tkz->status = status;
}

lxb_inline void
lxb_html_tokenizer_opt_set(lxb_html_tokenizer_t *tkz,
                           lxb_html_tokenizer_opt_t opt)
{
    tkz->opt = opt;
}

lxb_inline lxb_html_tokenizer_opt_t
lxb_html_tokenizer_opt(lxb_html_tokenizer_t *tkz)
{
    return tkz->opt;
}

lxb_inline void
lxb_html_tokenizer_tag_heap_set(lxb_html_tokenizer_t *tkz,
                                lxb_tag_heap_t *tag_heap)
{
    tkz->tag_heap = tag_heap;
}

lxb_inline lxb_tag_heap_t *
lxb_html_tokenizer_tag_heap(lxb_html_tokenizer_t *tkz)
{
    return tkz->tag_heap;
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
                                  lxb_tag_id_t tag_id)
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

lxb_inline lexbor_mraw_t *
lxb_html_tokenizer_mraw(lxb_html_tokenizer_t *tkz)
{
    return tkz->mraw;
}

/*
 * No inline functions for ABI.
 */
void
lxb_html_tokenizer_status_set_noi(lxb_html_tokenizer_t *tkz,
                                  lxb_status_t status);

void
lxb_html_tokenizer_opt_set_noi(lxb_html_tokenizer_t *tkz,
                               lxb_html_tokenizer_opt_t opt);

lxb_html_tokenizer_opt_t
lxb_html_tokenizer_opt_noi(lxb_html_tokenizer_t *tkz);

void
lxb_html_tokenizer_tag_heap_set_noi(lxb_html_tokenizer_t *tkz,
                                    lxb_tag_heap_t *tag_heap);

lxb_tag_heap_t *
lxb_html_tokenizer_tag_heap_noi(lxb_html_tokenizer_t *tkz);

void
lxb_html_tokenizer_ns_heap_set_noi(lxb_html_tokenizer_t *tkz,
                                   lxb_ns_heap_t *ns_heap);

lxb_ns_heap_t *
lxb_html_tokenizer_ns_heap_noi(lxb_html_tokenizer_t *tkz);

void
lxb_html_tokenizer_callback_token_done_set_noi(lxb_html_tokenizer_t *tkz,
                                               lxb_html_tokenizer_token_f call_func,
                                               void *ctx);

void *
lxb_html_tokenizer_callback_token_done_ctx_noi(lxb_html_tokenizer_t *tkz);

void
lxb_html_tokenizer_state_set_noi(lxb_html_tokenizer_t *tkz,
                                 lxb_html_tokenizer_state_f state);

void
lxb_html_tokenizer_tmp_tag_id_set_noi(lxb_html_tokenizer_t *tkz,
                                      lxb_tag_id_t tag_id);

lxb_html_tree_t *
lxb_html_tokenizer_tree_noi(lxb_html_tokenizer_t *tkz);

void
lxb_html_tokenizer_tree_set_noi(lxb_html_tokenizer_t *tkz,
                                lxb_html_tree_t *tree);

lexbor_mraw_t *
lxb_html_tokenizer_mraw_noi(lxb_html_tokenizer_t *tkz);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_TOKENIZER_H */
