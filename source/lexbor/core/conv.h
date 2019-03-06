/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_CONV_H
#define LEXBOR_CONV_H

#ifdef __cplusplus
extern "C" {
#endif


#include "lexbor/core/base.h"


LXB_API size_t
lexbor_conv_float_to_data(double num, lxb_char_t *buf, size_t len);

LXB_API double
lexbor_conv_data_to_double(const lxb_char_t **start, size_t len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_CONV_H */
