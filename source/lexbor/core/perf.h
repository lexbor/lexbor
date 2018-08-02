/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_PERF_H
#define LEXBOR_PERF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/base.h"


#ifdef LEXBOR_WITH_PERF


void *
lexbor_perf_create(void);

void
lexbor_perf_clean(void *perf);

void
lexbor_perf_destroy(void *perf);

lxb_status_t
lexbor_perf_begin(void *perf);

lxb_status_t
lexbor_perf_end(void *perf);

double
lexbor_perf_in_sec(void *perf);

unsigned long long
lexbor_perf_clock(void);

unsigned long long
lexbor_perf_frequency(void);


#endif /* LEXBOR_WITH_PERF */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_PERF_H */
