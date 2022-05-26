/*
 * Copyright (C) 2018-2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/css/syntax/syntax.h"
#include "lexbor/css/parser.h"

#include "lexbor/core/str.h"

#define LEXBOR_STR_RES_ANSI_REPLACEMENT_CHARACTER
#include "lexbor/core/str_res.h"


lxb_status_t
lxb_css_syntax_parse_list_rules(lxb_css_parser_t *parser,
                                const lxb_css_syntax_cb_list_rules_t *cb,
                                const lxb_char_t *data, size_t length,
                                void *ctx, bool top_level)
{
    lxb_status_t status;
    lxb_css_syntax_rule_t *rule;

    if (lxb_css_parser_is_running(parser)) {
        parser->status = LXB_STATUS_ERROR_WRONG_STAGE;
        return parser->status;
    }

    lxb_css_parser_clean(parser);

    lxb_css_parser_buffer_set(parser, data, length);

    rule = lxb_css_syntax_parser_list_rules_push(parser, NULL, NULL, cb,
                                                 ctx, top_level,
                                                 LXB_CSS_SYNTAX_TOKEN_UNDEF);
    if (rule == NULL) {
        status = parser->status;
        goto end;
    }

    parser->tkz->with_comment = false;
    parser->stage = LXB_CSS_PARSER_RUN;

    status = lxb_css_syntax_parser_run(parser);
    if (status != LXB_STATUS_OK) {
        /* Destroy StyleSheet. */
    }

end:

    parser->stage = LXB_CSS_PARSER_END;

    return status;
}

lxb_status_t
lxb_css_syntax_stack_expand(lxb_css_parser_t *parser, size_t count)
{
    uint8_t *begin;
    uintptr_t size, new_size;

    if ((parser->rules + count) >= parser->rules_end) {
        size = (parser->rules_end - parser->rules_begin);

        new_size = (count + 32) * sizeof(lxb_css_syntax_rule_t);

        begin = lexbor_realloc(parser->rules_begin, new_size + size);
        if (begin == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        parser->rules_begin = (lxb_css_syntax_rule_t *) begin;
        parser->rules_end = (lxb_css_syntax_rule_t *) (begin + new_size);
        parser->rules = (lxb_css_syntax_rule_t *) (begin + size) - 1;
    }

    return LXB_STATUS_OK;
}

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
