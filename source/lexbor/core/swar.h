/*
 * Copyright (C) 2024 Alexander Borisov
 *
 * Author: Niels Dossche <nielsdos@php.net>
 */

#ifndef LEXBOR_SWAR_H
#define LEXBOR_SWAR_H

#ifdef __cplusplus
extern "C" {
#endif


#include "lexbor/core/base.h"


/* Based on techniques from https://graphics.stanford.edu/~seander/bithacks.html */
#define SWAR_ONES (~((size_t) 0) / 0xFF)
#define SWAR_REPEAT(x) (SWAR_ONES * (x))
#define SWAR_HAS_ZERO(v) (((v) - SWAR_ONES) & ~(v) & SWAR_REPEAT(0x80))
#define SWAR_IS_LITTLE_ENDIAN (*(unsigned char *)&(uint16_t){1})


/* When handling hot loops that search for a set of characters,
 * this function can be used to quickly move the data pointer much
 * closer to the first occurrence of such a character. */
lxb_inline const lxb_char_t *lxb_swar_seek4(const lxb_char_t *data,
                                            const lxb_char_t *end,
                                            lxb_char_t c1, lxb_char_t c2,
                                            lxb_char_t c3, lxb_char_t c4)
{
    if (SWAR_IS_LITTLE_ENDIAN) {
        while (data + sizeof(size_t) <= end) {
            size_t bytes;
            memcpy(&bytes, data, sizeof(size_t));

            size_t t1 = bytes ^ SWAR_REPEAT(c1);
            size_t t2 = bytes ^ SWAR_REPEAT(c2);
            size_t t3 = bytes ^ SWAR_REPEAT(c3);
            size_t t4 = bytes ^ SWAR_REPEAT(c4);
            size_t matches = SWAR_HAS_ZERO(t1) | SWAR_HAS_ZERO(t2)
                           | SWAR_HAS_ZERO(t3) | SWAR_HAS_ZERO(t4);

            if (matches) {
                data += ((((matches - 1) & SWAR_ONES) * SWAR_ONES)
                        >> (sizeof(size_t) * 8 - 8)) - 1;
                break;
            } else {
                data += sizeof(size_t);
            }
        }
    }

    return data;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_SWAR_H */

