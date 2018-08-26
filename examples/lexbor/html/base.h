/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_EXAMPLES_BASE_H
#define LEXBOR_EXAMPLES_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/core/types.h>
#include <lexbor/html/serialize.h>


#define FAILED(...)                                                            \
    do {                                                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
    while (0)

#define PRINT(...)                                                             \
    do {                                                                       \
        fprintf(stdout, __VA_ARGS__);                                          \
        fprintf(stdout, "\n");                                                 \
    }                                                                          \
    while (0)


lxb_inline lxb_status_t
serializer_callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_EXAMPLES_BASE_H */
