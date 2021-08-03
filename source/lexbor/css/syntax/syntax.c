/*
 * Copyright (C) 2018-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/css/syntax/base.h"
#include "lexbor/css/syntax/tokenizer.h"

#include "lexbor/core/str.h"

#define LEXBOR_STR_RES_ANSI_REPLACEMENT_CHARACTER
#include "lexbor/core/str_res.h"


void
lxb_css_syntax_codepoint_to_ascii(lxb_css_syntax_tokenizer_t *tkz,
                                  lxb_codepoint_t cp)
{
    /*
     * Zero, or is for a surrogate, or is greater than
     * the maximum allowed code point (tkz->num > 0x10FFFF).
     */
    if (cp == 0 || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
        memcpy(tkz->pos, lexbor_str_res_ansi_replacement_character, 3);

        tkz->pos += 3;
        *tkz->pos = '\0';

        return;
    }

    lxb_char_t *data = tkz->pos;

    if (cp <= 0x0000007F) {
        /* 0xxxxxxx */
        data[0] = (lxb_char_t) cp;

        tkz->pos += 1;
    }
    else if (cp <= 0x000007FF) {
        /* 110xxxxx 10xxxxxx */
        data[0] = (char)(0xC0 | (cp >> 6  ));
        data[1] = (char)(0x80 | (cp & 0x3F));

        tkz->pos += 2;
    }
    else if (cp <= 0x0000FFFF) {
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        data[0] = (char)(0xE0 | ((cp >> 12)));
        data[1] = (char)(0x80 | ((cp >> 6 ) & 0x3F));
        data[2] = (char)(0x80 | ( cp & 0x3F));

        tkz->pos += 3;
    }
    else if (cp <= 0x001FFFFF) {
        /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        data[0] = (char)(0xF0 | ( cp >> 18));
        data[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        data[2] = (char)(0x80 | ((cp >> 6 ) & 0x3F));
        data[3] = (char)(0x80 | ( cp & 0x3F));

        tkz->pos += 4;
    }

    *tkz->pos = '\0';
}
