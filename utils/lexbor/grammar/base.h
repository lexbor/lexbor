/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_GRAMMAR_BASE_H
#define LEXBOR_GRAMMAR_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/base.h"

#include "lexbor/grammar/document.h"


#define LXB_GRAMMAR_VERSION_MAJOR 0
#define LXB_GRAMMAR_VERSION_MINOR 1
#define LXB_GRAMMAR_VERSION_PATCH 0

#define LXB_GRAMMAR_VERSION_STRING                                             \
    LEXBOR_STRINGIZE(LXB_GRAMMAR_VERSION_MAJOR) "."                            \
    LEXBOR_STRINGIZE(LXB_GRAMMAR_VERSION_MINOR) "."                            \
    LEXBOR_STRINGIZE(LXB_GRAMMAR_VERSION_PATCH)


#define lxb_grammar_cb_m(text, length)                                         \
    do {                                                                       \
        status = func((lxb_char_t *) (text), length, ctx);                     \
        if (status != LXB_STATUS_OK) {                                         \
            return status;                                                     \
        }                                                                      \
    }                                                                          \
    while (0)

#define lxb_grammar_indent_m(indent)                                           \
    do {                                                                       \
        for (size_t i = 0; i < indent; i++) {                                  \
            status = func((lxb_char_t *) "  ", 2, ctx);                        \
            if (status != LXB_STATUS_OK) {                                     \
                return status;                                                 \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    while (0)


typedef lxb_status_t
(*lxb_grammar_serialize_cb_f)(const lxb_char_t *data, size_t len, void *ctx);

typedef struct lxb_grammar_parser lxb_grammar_parser_t;
typedef struct lxb_grammar_tokenizer lxb_grammar_tokenizer_t;
typedef struct lxb_grammar_node lxb_grammar_node_t;
typedef struct lxb_grammar_tree lxb_grammar_tree_t;
typedef struct lxb_grammar_tree_scope lxb_grammar_tree_scope_t;
typedef struct lxb_grammar_tree_entry lxb_grammar_tree_entry_t;
typedef struct lxb_grammar_tree_repeat_entry lxb_grammar_tree_repeat_entry_t;
typedef struct lxb_grammar_tree_declaration lxb_grammar_tree_declaration_t;

typedef struct lxb_grammar_period {
    long start;
    long stop;
}
lxb_grammar_period_t;

typedef struct lxb_grammar_position {
    const lxb_char_t *newline_pos;
    size_t           newline_count;

    const lxb_char_t *pos;
    size_t           length;
}
lxb_grammar_position_t;

typedef enum {
    LXB_GRAMMAR_LOG_INFO = 0x00,
    LXB_GRAMMAR_LOG_WARNING,
    LXB_GRAMMAR_LOG_ERROR
}
lxb_grammar_log_level_t;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_GRAMMAR_BASE_H */
