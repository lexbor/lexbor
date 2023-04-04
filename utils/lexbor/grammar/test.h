/*
 * Copyright (C) 2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_GRAMMAR_TEST_H
#define LEXBOR_GRAMMAR_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/grammar/base.h"


typedef lxb_status_t
(*utils_lxb_grammar_test_cb_f)(const lxb_char_t *name, size_t name_len,
                               const lxb_char_t *value, size_t value_len,
                               const lxb_char_t *ordered, size_t ordered_len,
                               bool last, bool bad, void *ctx);


LXB_API lxb_status_t
utils_lxb_grammar_test(const lxb_char_t *grammar, const size_t length,
                       lexbor_serialize_cb_f begin,
                       utils_lxb_grammar_test_cb_f cb,
                       lexbor_serialize_cb_f end, void *ctx);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_GRAMMAR_TEST_H */
