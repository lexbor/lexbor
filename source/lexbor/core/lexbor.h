/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_H
#define LEXBOR_H

#ifdef __cplusplus
extern "C" {
#endif


void *
lexbor_malloc(size_t size);

void *
lexbor_realloc(void *dst, size_t size);

void *
lexbor_calloc(size_t num, size_t size);

void *
lexbor_free(void *dst);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_H */

