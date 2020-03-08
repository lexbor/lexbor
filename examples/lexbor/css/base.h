/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_EXAMPLES_BASE_H
#define LEXBOR_EXAMPLES_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

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


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_EXAMPLES_BASE_H */
