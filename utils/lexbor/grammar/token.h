/*
 * Copyright (C) 2019-2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_GRAMMAR_TOKEN_H
#define LEXBOR_GRAMMAR_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/grammar/base.h"

#include "lexbor/core/str.h"
#include "lexbor/dom/interfaces/node.h"


typedef enum {
    LXB_GRAMMAR_TOKEN_UNDEF = 0x00,
    LXB_GRAMMAR_TOKEN_WHITESPACE,    /* \n\t\r\f */
    LXB_GRAMMAR_TOKEN_ELEMENT,       /* <some val="data"> */
    LXB_GRAMMAR_TOKEN_EQUALS,        /* = */
    LXB_GRAMMAR_TOKEN_STRING,        /* "abc" or 'abc' */
    LXB_GRAMMAR_TOKEN_NUMBER,        /* 123 */
    LXB_GRAMMAR_TOKEN_UNQUOTED,      /* abc */
    LXB_GRAMMAR_TOKEN_DELIM,         /* one char, like a '%'. not a-zA-Z, 0-9 */
    LXB_GRAMMAR_TOKEN_ASTERISK,      /* * */
    LXB_GRAMMAR_TOKEN_PLUS,          /* + */
    LXB_GRAMMAR_TOKEN_EXCLAMATION,   /* ! */
    LXB_GRAMMAR_TOKEN_QUESTION,      /* ? */
    LXB_GRAMMAR_TOKEN_HASH,          /* # */
    LXB_GRAMMAR_TOKEN_EXCLUDE_WS,    /* ^WS */
    LXB_GRAMMAR_TOKEN_EXCLUDE_SORT,  /* ^SORT */
    LXB_GRAMMAR_TOKEN_COUNT,         /* {10} */
    LXB_GRAMMAR_TOKEN_RANGE,         /* {1,2} */
    LXB_GRAMMAR_TOKEN_BAR,           /* | */
    LXB_GRAMMAR_TOKEN_DOUBLE_BAR,    /* || */
    LXB_GRAMMAR_TOKEN_AND,           /* && */
    LXB_GRAMMAR_TOKEN_LEFT_BRACKET,  /* [ */
    LXB_GRAMMAR_TOKEN_RIGHT_BRACKET, /* ] */
    LXB_GRAMMAR_TOKEN_END_OF_FILE
}
lxb_grammar_token_type_t;

typedef enum {
    LXB_GRAMMAR_TOKEN_FLAGS_UNDEF   = 0x00,
    LXB_GRAMMAR_TOKEN_FLAGS_NEWLINE = 0x01
}
lxb_grammar_token_flags_t;

typedef struct lxb_grammar_token {
    lxb_grammar_token_type_t type;
    int                      flags;

    union lxb_grammar_token_u {
        double                     num;
        long                       count;
        lexbor_str_t               str;
        lxb_dom_node_t             *node;
        lxb_grammar_period_t       period;
    }
    u;

    lxb_grammar_position_t   position;
}
lxb_grammar_token_t;


LXB_API lxb_grammar_token_t *
lxb_grammar_token_create(lxb_grammar_tokenizer_t *tkz,
                         lxb_grammar_token_type_t type);

LXB_API lxb_grammar_token_t *
lxb_grammar_token_destroy(lxb_grammar_tokenizer_t *tkz,
                          lxb_grammar_token_t *token);

LXB_API lxb_status_t
lxb_grammar_token_serialize(lxb_grammar_token_t *token,
                            lxb_grammar_serialize_cb_f func, void *ctx);

LXB_API const lxb_char_t *
lxb_grammar_token_name(lxb_grammar_token_t *token, size_t *len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_GRAMMAR_TOKEN_H */
