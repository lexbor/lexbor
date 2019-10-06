/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <math.h>
#include <float.h>

#include "lexbor/core/conv.h"
#include "lexbor/core/dtoa.h"
#include "lexbor/core/strtod.h"


size_t
lexbor_conv_float_to_data(double num, lxb_char_t *buf, size_t len)
{
    return lexbor_dtoa(num, buf, len);
}

double
lexbor_conv_data_to_double(const lxb_char_t **start, size_t len)
{
    int exponent, exp, insignf;
    lxb_char_t c, *pos;
    bool minus;
    const lxb_char_t  *e, *p, *last, *end;
    lxb_char_t data[128];

    end = *start + len;

    exponent = 0;
    insignf = 0;

    pos = data;
    last = data + sizeof(data);

    for (p = *start; p < end; p++) {
        /* Values less than '0' become >= 208. */
        c = *p - '0';

        if (c > 9) {
            break;
        }

        if (pos < last) {
            *pos++ = *p;
        }
        else {
            insignf++;
        }
    }

    /* Do not emit a '.', but adjust the exponent instead. */
    if (p < end && *p == '.') {

        for (p++; p < end; p++) {
            /* Values less than '0' become >= 208. */
            c = *p - '0';

            if (c > 9) {
                break;
            }

            if (pos < last) {
                *pos++ = *p;
                exponent--;
            }
            else {
                /* Ignore insignificant digits in the fractional part. */
            }
        }
    }

    e = p + 1;

    if (e < end && (*p == 'e' || *p == 'E')) {
        minus = 0;

        if (e + 1 < end) {
            if (*e == '-') {
                e++;
                minus = 1;
            }
            else if (*e == '+') {
                e++;
            }
        }

        /* Values less than '0' become >= 208. */
        c = *e - '0';

        if (c <= 9) {
            exp = c;

            for (p = e + 1; p < end; p++) {
                /* Values less than '0' become >= 208. */
                c = *p - '0';

                if (c > 9) {
                    break;
                }

                exp = exp * 10 + c;
            }

            exponent += minus ? -exp : exp;
        }
    }

    *start = p;

    exponent += insignf;

    return lexbor_strtod_internal(data, pos - data, exponent);
}
