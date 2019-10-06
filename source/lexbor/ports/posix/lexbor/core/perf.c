/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/core/perf.h"


#ifdef LEXBOR_WITH_PERF

#ifdef LEXBOR_OS_DARWIN
    #include <sys/sysctl.h>
#elif LEXBOR_OS_LINUX
#endif


static unsigned long long
lexbor_perf_clock(void);

static unsigned long long
lexbor_perf_frequency(void);


typedef struct lexbor_perf {
    unsigned long long start;
    unsigned long long end;
    unsigned long long freq;
}
lexbor_perf_t;


void *
lexbor_perf_create(void)
{
    lexbor_perf_t *perf = lexbor_calloc(1, sizeof(lexbor_perf_t));
    if (perf == NULL) {
        return NULL;
    }

    perf->freq = lexbor_perf_frequency();

    return perf;
}

void
lexbor_perf_clean(void *perf)
{
    memset(perf, 0, sizeof(lexbor_perf_t));
}

void
lexbor_perf_destroy(void *perf)
{
    if (perf != NULL) {
        lexbor_free(perf);
    }
}

lxb_status_t
lexbor_perf_begin(void *perf)
{
    ((lexbor_perf_t *) (perf))->start = lexbor_perf_clock();

    return LXB_STATUS_OK;
}

lxb_status_t
lexbor_perf_end(void *perf)
{
    ((lexbor_perf_t *) (perf))->end = lexbor_perf_clock();

    return LXB_STATUS_OK;
}

double
lexbor_perf_in_sec(void *perf)
{
    lexbor_perf_t *obj_perf = (lexbor_perf_t *) perf;

    if (obj_perf->freq != 0) {
        return ((double) (obj_perf->end - obj_perf->start)
                / (double)obj_perf->freq);
    }

    return 0.0f;
}

static unsigned long long
lexbor_perf_clock(void)
{
    unsigned long long x;

     /*
      * cpuid serializes any out-of-order prefetches
      * before executing rdtsc (clobbers ebx, ecx, edx).
      */
    __asm__ volatile (
                      "cpuid\n\t"
                      "rdtsc\n\t"
                      "shl $32, %%rdx\n\t"
                      "or %%rdx, %%rax"
                      : "=a" (x)
                      :
                      : "rdx", "ebx", "ecx");

    return x;
}

static unsigned long long
lexbor_perf_frequency(void)
{
    unsigned long long freq = 0;

#if defined(LEXBOR_OS_DARWIN) && defined(CTL_HW) && defined(HW_CPU_FREQ)

    /* OSX kernel: sysctl(CTL_HW | HW_CPU_FREQ) */
    size_t len = sizeof(freq);
    int mib[2] = {CTL_HW, HW_CPU_FREQ};

    if(sysctl(mib, 2, &freq, &len, NULL, 0)) {
        return 0;
    }

    return freq;

#elif defined(LEXBOR_OS_LINUX)

    char buf[1024] = {0};
    double fval = 0.0;

    /* Use procfs on linux */
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        return 0;
    }

    /* Find 'CPU MHz :' */
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (sscanf(buf, "cpu MHz : %lf\n", &fval) == 1) {
            freq = (unsigned long long)(fval * 1000000ull);

            break;
        }
    }

    fclose(fp);

    return freq;

#else

    return freq;

#endif /* LEXBOR_OS_DARWIN || LEXBOR_OS_LINUX */
}

#endif /* LEXBOR_WITH_PERF */
