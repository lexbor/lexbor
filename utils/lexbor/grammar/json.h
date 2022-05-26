/*
 * Copyright (C) 2020-2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_GRAMMAR_JSON_H
#define LEXBOR_GRAMMAR_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/grammar/base.h"
#include "lexbor/core/avl.h"


typedef struct {
    lexbor_avl_t      avl;
    lexbor_avl_node_t *scope_root;
}
lxb_grammar_json_t;


LXB_API lxb_status_t
lxb_grammar_json_init(lxb_grammar_json_t *json);

LXB_API lxb_grammar_json_t *
lxb_grammar_json_destroy(lxb_grammar_json_t *json, bool self_destroy);

LXB_API lxb_status_t
lxb_grammar_json_ast(const lxb_grammar_node_t *root, size_t indent,
                     lxb_grammar_serialize_cb_f func, void *ctx);

LXB_API lxb_status_t
lxb_grammar_json_ast_node_attributes(const lxb_grammar_node_t *node, size_t indent,
                                    lxb_grammar_serialize_cb_f func, void *ctx);

LXB_API lxb_status_t
lxb_grammar_json_ast_node_value(const lxb_grammar_node_t *node,
                                lxb_grammar_serialize_cb_f func, void *ctx);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_GRAMMAR_JSON_H */
