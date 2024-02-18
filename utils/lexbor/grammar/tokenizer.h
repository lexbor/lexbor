/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_GRAMMAR_TOKENIZER_H
#define LEXBOR_GRAMMAR_TOKENIZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/grammar/base.h"
#include "lexbor/grammar/token.h"

#include "lexbor/html/parser.h"
#include "lexbor/html/token.h"


typedef const lxb_char_t *
(*lxb_grammar_tokenizer_state_f)(lxb_grammar_tokenizer_t *tkz,
                                 lxb_html_token_t *token,
                                 const lxb_char_t *data, const lxb_char_t *end);


struct lxb_grammar_tokenizer {
    lxb_grammar_tokenizer_state_f state;

    lxb_html_parser_t             *html_parser;

    lxb_status_t                  status;
};


LXB_API lxb_grammar_tokenizer_t *
lxb_grammar_tokenizer_create(void);

LXB_API lxb_status_t
lxb_grammar_tokenizer_init(lxb_grammar_tokenizer_t *tkz);

LXB_API void
lxb_grammar_tokenizer_clean(lxb_grammar_tokenizer_t *tkz);

LXB_API lxb_grammar_tokenizer_t *
lxb_grammar_tokenizer_destroy(lxb_grammar_tokenizer_t *tkz, bool self_destroy);

LXB_API lxb_grammar_document_t *
lxb_grammar_tokenizer_process(lxb_grammar_tokenizer_t *tkz,
                              const lxb_char_t *data, size_t size);

/*
 * Inline functions
 */
lxb_inline void
lxb_grammar_tokenizer_state_set(lxb_grammar_tokenizer_t *tkz,
                                lxb_grammar_tokenizer_state_f state)
{
    tkz->state = state;
}

lxb_inline lexbor_array_t *
lxb_grammar_tokenizer_tokens(lxb_grammar_document_t *document)
{
    return document->dom_document.user;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_GRAMMAR_TOKENIZER_H */
