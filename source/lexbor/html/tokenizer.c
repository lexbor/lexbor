/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tokenizer.h"
#include "lexbor/html/tokenizer/state.h"
#include "lexbor/html/tokenizer/state_rcdata.h"
#include "lexbor/html/tokenizer/state_rawtext.h"
#include "lexbor/html/tokenizer/state_script.h"
#include "lexbor/html/tree.h"
#include "lexbor/html/in.h"

#define LXB_HTML_TAG_RES_DATA
#define LXB_HTML_TAG_RES_SHS_DATA
#include "lexbor/html/tag_res.h"


const lxb_char_t *lxb_html_tokenizer_eof = (const lxb_char_t *) "\x00";


static void
lxb_html_tokenizer_erase_incoming(lxb_html_tokenizer_t *tkz);

static lxb_status_t
lxb_html_tokenizer_chunk_process(lxb_html_tokenizer_t *tkz,
                                 const lxb_char_t *data, size_t size);

static lxb_html_token_t *
lxb_html_tokenizer_token_done(lxb_html_tokenizer_t *tkz,
                              lxb_html_token_t *token, void *ctx);

static const lxb_char_t *
lxb_html_tokenizer_change_incoming_eof(lxb_html_tokenizer_t *tkz,
                                       const lxb_char_t *pos);


lxb_html_tokenizer_t *
lxb_html_tokenizer_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_html_tokenizer_t));
}

lxb_status_t
lxb_html_tokenizer_init(lxb_html_tokenizer_t *tkz)
{
    lxb_status_t status;

    if (tkz == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    /* mraw for templary strings or structures */
    tkz->mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(tkz->mraw, 1024);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Init Token */
    tkz->token = NULL;

    tkz->dobj_token = lexbor_dobject_create();
    status = lexbor_dobject_init(tkz->dobj_token,
                                 4096, sizeof(lxb_html_token_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Init Token Attributes */
    tkz->dobj_token_attr = lexbor_dobject_create();
    status = lexbor_dobject_init(tkz->dobj_token_attr, 4096,
                                 sizeof(lxb_html_token_attr_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Incoming */
    tkz->incoming = lexbor_in_create();
    status = lexbor_in_init(tkz->incoming, 32);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    tkz->incoming_node = NULL;

    /* Parse errors */
    tkz->parse_errors = lexbor_array_obj_create();
    status = lexbor_array_obj_init(tkz->parse_errors, 16,
                                   sizeof(lxb_html_tokenizer_error_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    tkz->tree = NULL;
    tkz->tags = NULL;

    tkz->state = lxb_html_tokenizer_state_data_before;
    tkz->state_return = NULL;

    tkz->callback_token_done = lxb_html_tokenizer_token_done;
    tkz->callback_token_ctx = NULL;

    tkz->is_eof = false;
    tkz->status = LXB_STATUS_OK;

    tkz->base = NULL;
    tkz->ref_count = 1;

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_tokenizer_inherit(lxb_html_tokenizer_t *tkz_to,
                           lxb_html_tokenizer_t *tkz_from)
{
    lxb_status_t status;

    tkz_to->tags = tkz_from->tags;
    tkz_to->mraw = tkz_from->mraw;

    /* Token and Attributes */
    tkz_to->token = NULL;

    tkz_to->dobj_token = tkz_from->dobj_token;
    tkz_to->dobj_token_attr = tkz_from->dobj_token_attr;

    /* Incoming */
    tkz_to->incoming = tkz_from->incoming;
    tkz_to->incoming_node = NULL;

    /* Parse errors */
    tkz_to->parse_errors = lexbor_array_obj_create();
    status = lexbor_array_obj_init(tkz_to->parse_errors, 16,
                                   sizeof(lxb_html_tokenizer_error_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    tkz_to->state = lxb_html_tokenizer_state_data_before;
    tkz_to->state_return = NULL;

    tkz_to->callback_token_done = lxb_html_tokenizer_token_done;
    tkz_to->callback_token_ctx = NULL;

    tkz_to->is_eof = false;
    tkz_to->status = LXB_STATUS_OK;

    tkz_to->base = tkz_from;
    tkz_to->ref_count = 1;

    return LXB_STATUS_OK;
}

lxb_html_tokenizer_t *
lxb_html_tokenizer_ref(lxb_html_tokenizer_t *tkz)
{
    if (tkz == NULL) {
        return NULL;
    }

    if (tkz->base != NULL) {
        return lxb_html_tokenizer_ref(tkz->base);
    }

    tkz->ref_count++;

    return tkz;
}

lxb_html_tokenizer_t *
lxb_html_tokenizer_unref(lxb_html_tokenizer_t *tkz)
{
    if (tkz == NULL || tkz->ref_count == 0) {
        return NULL;
    }

    if (tkz->base != NULL) {
        tkz->base = lxb_html_tokenizer_unref(tkz->base);
    }

    tkz->ref_count--;

    if (tkz->ref_count == 0) {
        lxb_html_tokenizer_destroy(tkz);
    }

    return NULL;
}

void
lxb_html_tokenizer_clean(lxb_html_tokenizer_t *tkz)
{
    tkz->tree = NULL;

    tkz->state = lxb_html_tokenizer_state_data_before;
    tkz->state_return = NULL;

    tkz->is_eof = false;
    tkz->status = LXB_STATUS_OK;

    lexbor_mraw_clean(tkz->mraw);
    lexbor_dobject_clean(tkz->dobj_token);
    lexbor_dobject_clean(tkz->dobj_token_attr);

    lxb_html_tokenizer_erase_incoming(tkz);
    lexbor_in_clean(tkz->incoming);

    lexbor_array_obj_clean(tkz->parse_errors);

    tkz->incoming_first = NULL;
    tkz->incoming_node = NULL;
}

lxb_html_tokenizer_t *
lxb_html_tokenizer_destroy(lxb_html_tokenizer_t *tkz)
{
    if (tkz == NULL) {
        return NULL;
    }

    lxb_html_tokenizer_erase_incoming(tkz);

    if (tkz->base == NULL) {
        tkz->mraw = lexbor_mraw_destroy(tkz->mraw, true);
        tkz->dobj_token = lexbor_dobject_destroy(tkz->dobj_token, true);
        tkz->dobj_token_attr = lexbor_dobject_destroy(tkz->dobj_token_attr,
                                                      true);
        tkz->incoming = lexbor_in_destroy(tkz->incoming, true);
    }

    tkz->parse_errors = lexbor_array_obj_destroy(tkz->parse_errors, true);

    return lexbor_free(tkz);
}

lxb_status_t
lxb_html_tokenizer_tags_make(lxb_html_tokenizer_t *tkz, size_t table_size)
{
    tkz->tags = lexbor_hash_create();
    return lexbor_hash_init(tkz->tags, table_size, sizeof(lxb_tag_data_t));
}

void
lxb_html_tokenizer_tags_destroy(lxb_html_tokenizer_t *tkz)
{
    tkz->tags = lexbor_hash_destroy(tkz->tags, true);
}

static void
lxb_html_tokenizer_erase_incoming(lxb_html_tokenizer_t *tkz)
{
    lexbor_in_node_t *next_node;

    while (tkz->incoming_first != NULL)
    {
        if (tkz->incoming_first->opt & LEXBOR_IN_OPT_ALLOC) {
            lexbor_free((lxb_char_t *) tkz->incoming_first->begin);
        }

        next_node = tkz->incoming_first->next;

        lexbor_in_node_destroy(tkz->incoming, tkz->incoming_first, true);

        tkz->incoming_first = next_node;
    }
}

lxb_status_t
lxb_html_tokenizer_begin(lxb_html_tokenizer_t *tkz)
{
    tkz->token = lxb_html_token_create(tkz->dobj_token);
    if (tkz->token == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_tokenizer_chunk(lxb_html_tokenizer_t *tkz, const lxb_char_t *data,
                         size_t size)
{
    lexbor_in_node_t *next_node;

    if (tkz->opt & LXB_HTML_TOKENIZER_OPT_WO_COPY) {
        tkz->incoming_node = lexbor_in_node_make(tkz->incoming, tkz->incoming_node,
                                                 data, size);
        if (tkz->incoming_node == NULL) {
            goto failed_in;
        }

        if (tkz->incoming_first == NULL) {
            tkz->incoming_first = tkz->incoming_node;
        }

        lxb_html_tokenizer_chunk_process(tkz, data, size);
    }
    else {
        lxb_char_t *copied;

        copied = lexbor_malloc(sizeof(lxb_char_t) * size);
        if (copied == NULL) {
            goto failed_in;
        }

        memcpy(copied, data, sizeof(lxb_char_t) * size);

        tkz->incoming_node = lexbor_in_node_make(tkz->incoming, tkz->incoming_node,
                                                 copied, size);
        if (tkz->incoming_node == NULL) {
            lexbor_free(copied);

            goto failed_in;
        }

        if (tkz->incoming_first == NULL) {
            tkz->incoming_first = tkz->incoming_node;
        }

        tkz->incoming_node->opt = LEXBOR_IN_OPT_ALLOC;

        lxb_html_tokenizer_chunk_process(tkz, copied, size);
    }

    if (tkz->opt & LXB_HTML_TOKENIZER_OPT_WO_IN_DESTROY) {
        return tkz->status;
    }

    if (tkz->status != LXB_STATUS_OK) {
        lxb_html_tokenizer_erase_incoming(tkz);

        return tkz->status;
    }

    if (tkz->token->begin != NULL) {
        while (tkz->incoming_first != NULL &&
               lexbor_in_segment(tkz->incoming_first, tkz->token->begin) == false)
        {
            if (tkz->incoming_first->opt & LEXBOR_IN_OPT_ALLOC) {
                lexbor_free((lxb_char_t *) tkz->incoming_first->begin);
            }

            next_node = tkz->incoming_first->next;

            lexbor_in_node_destroy(tkz->incoming, tkz->incoming_first, true);

            tkz->incoming_first = next_node;
        }

        if (tkz->incoming_first != NULL) {
            tkz->incoming_first->prev = NULL;
        }
        else {
            tkz->incoming_node = NULL;
        }
    }

    return tkz->status;

failed_in:

    if ((tkz->opt & LXB_HTML_TOKENIZER_OPT_WO_IN_DESTROY) == 0) {
        lxb_html_tokenizer_erase_incoming(tkz);
    }

    return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
}

static lxb_status_t
lxb_html_tokenizer_chunk_process(lxb_html_tokenizer_t *tkz,
                                 const lxb_char_t *data, size_t size)
{
    lexbor_in_node_t *in_node;
    const lxb_char_t *end = data + size;

    tkz->is_eof = false;
    tkz->status = LXB_STATUS_OK;

    while (data < end) {
        data = tkz->state(tkz, data, end);
    }

    /*
     * If some state change current incoming buffer,
     * then we need to parse all next buffers again.
     */
    if (tkz->incoming_node->next != NULL) {

reuse:

        in_node = tkz->incoming_node;
        data = in_node->use;

        for (;;) {
            while (data < in_node->end) {
                data = tkz->state(tkz, data, in_node->end);
            }

            if (in_node != tkz->incoming_node) {
                goto reuse;
            }

            in_node->use = in_node->end;

            if (in_node->next == NULL) {
                break;
            }

            in_node = in_node->next;
            tkz->incoming_node = in_node;

            data = in_node->begin;
        }
    }

    tkz->incoming_node->use = end;

    return tkz->status;
}

lxb_status_t
lxb_html_tokenizer_end(lxb_html_tokenizer_t *tkz)
{
    lexbor_in_node_t *in_node;
    const lxb_char_t *data, *end;

    tkz->reuse = false;
    tkz->status = LXB_STATUS_OK;

    /*
     * Send a fake EOF data (not added in to incoming buffer chain)
     * If some state change incoming buffer,
     * then we need parse again all buffers after current position
     * and try again send fake EOF.
     */
    do {
        data = lxb_html_tokenizer_eof;
        end = lxb_html_tokenizer_eof + 1UL;

        tkz->is_eof = true;

        while (tkz->state(tkz, data, end) < end) {
            /* empty loop */
        }

        /* TODO: need simplify and get rid of copy-paste */
        if (tkz->reuse) {

reuse:

            in_node = tkz->incoming_node;

            tkz->is_eof = false;
            data = in_node->use;

            for (;;) {
                while (data < in_node->end) {
                    data = tkz->state(tkz, data, in_node->end);
                }

                if (in_node != tkz->incoming_node) {
                    goto reuse;
                }

                in_node->use = in_node->end;

                if (in_node->next == NULL) {
                    break;
                }

                in_node = in_node->next;
                tkz->incoming_node = in_node;

                data = in_node->begin;
            }

            tkz->reuse = false;
        }
        else {
            break;
        }
    }
    while (1);

    tkz->is_eof = false;

    if (tkz->status != LXB_STATUS_OK) {
        return tkz->status;
    }

    /* Emit fake token: END OF FILE */
    lxb_html_token_clean(tkz->token);

    tkz->token->tag_id = LXB_TAG__END_OF_FILE;

    tkz->token = tkz->callback_token_done(tkz, tkz->token,
                                          tkz->callback_token_ctx);

    if (tkz->token == NULL && tkz->status == LXB_STATUS_OK) {
        tkz->status = LXB_STATUS_ERROR;
    }

    return tkz->status;
}

static lxb_html_token_t *
lxb_html_tokenizer_token_done(lxb_html_tokenizer_t *tkz,
                              lxb_html_token_t *token, void *ctx)
{
    return token;
}

const lxb_char_t *
lxb_html_tokenizer_change_incoming(lxb_html_tokenizer_t *tkz,
                                   const lxb_char_t *pos)
{
    if (tkz->is_eof) {
        return lxb_html_tokenizer_change_incoming_eof(tkz, pos);
    }

    if (lexbor_in_segment(tkz->incoming_node, pos)) {
        tkz->incoming_node->use = pos;

        return pos;
    }

    lexbor_in_node_t *node = tkz->incoming_node;
    tkz->incoming_node = lexbor_in_node_find(tkz->incoming_node, pos);

    if (tkz->incoming_node == NULL) {
        tkz->status = LXB_STATUS_ERROR;
        tkz->incoming_node = node;

        return tkz->incoming_node->end;
    }

    tkz->incoming_node->use = pos;

    return node->end;
}

static const lxb_char_t *
lxb_html_tokenizer_change_incoming_eof(lxb_html_tokenizer_t *tkz,
                                       const lxb_char_t *pos)
{
    if (pos == lxb_html_tokenizer_eof) {
        return pos;
    }

    tkz->reuse = true;

    if (lexbor_in_segment(tkz->incoming_node, pos)) {
        tkz->incoming_node->use = pos;

        return lxb_html_tokenizer_eof + 1;
    }

    lexbor_in_node_t *node = tkz->incoming_node;
    tkz->incoming_node = lexbor_in_node_find(tkz->incoming_node, pos);

    if (tkz->incoming_node == NULL) {
        tkz->reuse = false;

        tkz->status = LXB_STATUS_ERROR;
        tkz->incoming_node = node;

        return lxb_html_tokenizer_eof + 1;
    }

    tkz->incoming_node->use = pos;

    return lxb_html_tokenizer_eof + 1;
}

lxb_ns_id_t
lxb_html_tokenizer_current_namespace(lxb_html_tokenizer_t *tkz)
{
    if (tkz->tree == NULL) {
        return LXB_NS__UNDEF;
    }

    lxb_dom_node_t *node = lxb_html_tree_adjusted_current_node(tkz->tree);

    if (node == NULL) {
        return LXB_NS__UNDEF;
    }

    return node->ns;
}

void
lxb_html_tokenizer_set_state_by_tag(lxb_html_tokenizer_t *tkz, bool scripting,
                                    lxb_tag_id_t tag_id, lxb_ns_id_t ns)
{
    if (ns != LXB_NS_HTML) {
        tkz->state = lxb_html_tokenizer_state_data_before;

        return;
    }

    switch (tag_id) {
        case LXB_TAG_TITLE:
        case LXB_TAG_TEXTAREA:
            tkz->tmp_tag_id = tag_id;
            tkz->state = lxb_html_tokenizer_state_rcdata_before;

            break;

        case LXB_TAG_STYLE:
        case LXB_TAG_XMP:
        case LXB_TAG_IFRAME:
        case LXB_TAG_NOEMBED:
        case LXB_TAG_NOFRAMES:
            tkz->tmp_tag_id = tag_id;
            tkz->state = lxb_html_tokenizer_state_rawtext_before;

            break;

        case LXB_TAG_SCRIPT:
            tkz->tmp_tag_id = tag_id;
            tkz->state = lxb_html_tokenizer_state_script_data_before;

            break;

        case LXB_TAG_NOSCRIPT:
            if (scripting) {
                tkz->tmp_tag_id = tag_id;
                tkz->state = lxb_html_tokenizer_state_rawtext_before;

                return;
            }

            tkz->state = lxb_html_tokenizer_state_data_before;

            break;

        case LXB_TAG_PLAINTEXT:
            tkz->state = lxb_html_tokenizer_state_plaintext_before;

            break;

        default:
            break;
    }
}

/*
 * No inline functions for ABI.
 */
void
lxb_html_tokenizer_status_set_noi(lxb_html_tokenizer_t *tkz,
                                  lxb_status_t status)
{
    lxb_html_tokenizer_status_set(tkz, status);
}

void
lxb_html_tokenizer_opt_set_noi(lxb_html_tokenizer_t *tkz,
                               lxb_html_tokenizer_opt_t opt)
{
    lxb_html_tokenizer_opt_set(tkz, opt);
}

lxb_html_tokenizer_opt_t
lxb_html_tokenizer_opt_noi(lxb_html_tokenizer_t *tkz)
{
    return lxb_html_tokenizer_opt(tkz);
}

void
lxb_html_tokenizer_callback_token_done_set_noi(lxb_html_tokenizer_t *tkz,
                                               lxb_html_tokenizer_token_f call_func,
                                               void *ctx)
{
    lxb_html_tokenizer_callback_token_done_set(tkz, call_func, ctx);
}

void *
lxb_html_tokenizer_callback_token_done_ctx_noi(lxb_html_tokenizer_t *tkz)
{
    return lxb_html_tokenizer_callback_token_done_ctx(tkz);
}

void
lxb_html_tokenizer_state_set_noi(lxb_html_tokenizer_t *tkz,
                                 lxb_html_tokenizer_state_f state)
{
    lxb_html_tokenizer_state_set(tkz, state);
}

void
lxb_html_tokenizer_tmp_tag_id_set_noi(lxb_html_tokenizer_t *tkz,
                                      lxb_tag_id_t tag_id)
{
    lxb_html_tokenizer_tmp_tag_id_set(tkz, tag_id);
}

lxb_html_tree_t *
lxb_html_tokenizer_tree_noi(lxb_html_tokenizer_t *tkz)
{
    return lxb_html_tokenizer_tree(tkz);
}

void
lxb_html_tokenizer_tree_set_noi(lxb_html_tokenizer_t *tkz,
                                lxb_html_tree_t *tree)
{
    lxb_html_tokenizer_tree_set(tkz, tree);
}

lexbor_mraw_t *
lxb_html_tokenizer_mraw_noi(lxb_html_tokenizer_t *tkz)
{
    return lxb_html_tokenizer_mraw(tkz);
}

lexbor_hash_t *
lxb_html_tokenizer_tags_noi(lxb_html_tokenizer_t *tkz)
{
    return lxb_html_tokenizer_tags(tkz);
}
