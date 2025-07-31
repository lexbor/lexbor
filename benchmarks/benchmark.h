/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_BENCHMARK_H
#define LEXBOR_BENCHMARK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/core/conv.h>
#include <lexbor/core/perf.h>


#define BENCHMARK_FAILURE(...)                                                \
    do {                                                                      \
        fprintf(stderr, __VA_ARGS__);                                         \
        fprintf(stderr, "\n");                                                \
        exit(EXIT_FAILURE);                                                   \
    }                                                                         \
    while (0)

#define BENCHMARK_INIT                                                        \
    lxb_char_t buf[128];                                                      \
    char *buf_p = (char *) buf;                                               \
    setbuf(stdout, NULL)

#define BENCHMARK_ADD(_name_, _caption_, _repeat_, _context_)                 \
    do {                                                                      \
        printf("Run: %s; Repeat: %d; ", (const char *) _caption_, _repeat_);  \
                                                                              \
        double res = benchmark_ ##_name_(_repeat_, _context_);                \
        size_t len = lexbor_conv_float_to_data(res, buf, sizeof(buf) - 1);    \
        buf[len] = '\0';                                                      \
                                                                              \
        char *p = strchr(buf_p, '.');                                         \
        if (p != NULL) {                                                      \
            size_t n = p - buf_p + 1;                                         \
            if (len - n > 5) {                                                \
                len = n + 5;                                                  \
            }                                                                 \
        }                                                                     \
        printf("Result: %.*s sec\n", (int) len, buf_p);                       \
    }                                                                         \
    while (false)

#define BENCHMARK_BEGIN(_name_, _context_)                                    \
static double                                                                 \
benchmark_ ## _name_(size_t _repeat, void *_context_)                         \
{                                                                             \
    double _mean;                                                             \
    void *perf = lexbor_perf_create();

#define BENCHMARK_CODE                                                        \
    _mean = 0;                                                                \
                                                                              \
    for (size_t _n = 0; _n < 5; _n++) {                                       \
        lexbor_perf_begin(perf);                                              \
                                                                              \
        for (size_t _i = 0; _i < _repeat; _i++) {

#define BENCHMARK_CODE_END                                                    \
        }                                                                     \
                                                                              \
        lexbor_perf_end(perf);                                                \
        _mean += lexbor_perf_in_sec(perf);                                    \
    }

#define BENCHMARK_END                                                         \
    lexbor_perf_destroy(perf);                                                \
                                                                              \
    return _mean / 5.0f;                                                      \
}

#define test_eq(have, need)                                                   \
    do {                                                                      \
        if ((have) != (need)) {                                               \
            BENCHMARK_FAILURE("Failure\n%s:%d:%s\n",                          \
                              __FILE__, __LINE__, __func__);                  \
        }                                                                     \
    }                                                                         \
    while (0)

#define test_ne(have, need)                                                   \
    do {                                                                      \
        if ((have) == (need)) {                                               \
            BENCHMARK_FAILURE("Failure\n%s:%d:%s\n",                          \
                              __FILE__, __LINE__, __func__);                  \
        }                                                                     \
    }                                                                         \
    while (0)


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_BENCHMARK_H */
