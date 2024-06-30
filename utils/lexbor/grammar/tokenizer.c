/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/grammar/tokenizer.h"
#include "lexbor/grammar/token.h"

#include "lexbor/core/conv.h"
#include "lexbor/core/utils.h"
#include "lexbor/dom/interfaces/element.h"
#include "lexbor/tag/tag.h"

lxb_status_t
lxb_html_parse_chunk_prepare(lxb_html_parser_t *parser,
                             lxb_grammar_document_t *document);

const lxb_tag_data_t *
lxb_tag_append(lexbor_hash_t *hash, lxb_tag_id_t tag_id,
               const lxb_char_t *name, size_t length);

static lxb_html_token_t *
lxb_grammar_tokenizer_html_token(lxb_html_tokenizer_t *html_tkz,
                                 lxb_html_token_t *token, void *ctx);

static const lxb_char_t *
lxb_grammar_tokenizer_state_data(lxb_grammar_tokenizer_t *tkz,
                                 lxb_html_token_t *token,
                                 const lxb_char_t *data, const lxb_char_t *end);

static lxb_status_t
lxb_grammar_tokenizer_data_copy(lxb_grammar_tokenizer_t *tkz,
                                const lxb_char_t *data, const lxb_char_t *end,
                                lxb_grammar_token_t *g_token);


lxb_grammar_tokenizer_t *
lxb_grammar_tokenizer_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_grammar_tokenizer_t));
}

lxb_status_t
lxb_grammar_tokenizer_init(lxb_grammar_tokenizer_t *tkz)
{
    lxb_status_t status;

    if (tkz == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    tkz->html_parser = lxb_html_parser_create();
    status = lxb_html_parser_init(tkz->html_parser);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    lxb_html_tokenizer_callback_token_done_set(tkz->html_parser->tkz,
                                         lxb_grammar_tokenizer_html_token, tkz);

    return LXB_STATUS_OK;
}

void
lxb_grammar_tokenizer_clean(lxb_grammar_tokenizer_t *tkz)
{
    lxb_html_parser_clean(tkz->html_parser);
}

lxb_grammar_tokenizer_t *
lxb_grammar_tokenizer_destroy(lxb_grammar_tokenizer_t *tkz, bool self_destroy)
{
    if (tkz == NULL) {
        return NULL;
    }

    tkz->html_parser = lxb_html_parser_destroy(tkz->html_parser);

    if (self_destroy) {
        return lexbor_free(tkz);
    }

    return tkz;
}

lxb_grammar_document_t *
lxb_grammar_tokenizer_process(lxb_grammar_tokenizer_t *tkz,
                              const lxb_char_t *data, size_t size)
{
    lexbor_array_t *tokens;
    lxb_grammar_document_t *document;
    const lxb_tag_data_t *tag;

    document = lxb_html_parse_chunk_begin(tkz->html_parser);
    if (document == NULL) {
        return NULL;
    }

    tkz->status = lxb_html_parse_chunk_prepare(tkz->html_parser, document);
    if (tkz->status != LXB_STATUS_OK) {
        return lxb_html_document_destroy(document);
    }

    /* Create ROOT tag */
    tag = lxb_tag_append(document->dom_document.tags, LXB_TAG__UNDEF,
                         (const lxb_char_t *) "root", 4);
    if (tag == NULL) {
        return lxb_html_document_destroy(document);
    }

    /* Create ARRAY for tokens */
    tokens = lexbor_mraw_alloc(lxb_html_document_mraw(document),
                               sizeof(lexbor_array_t));

    tkz->status = lexbor_array_init(tokens, 1024);
    if (tkz->status != LXB_STATUS_OK) {
        return lxb_html_document_destroy(document);
    }

    document->dom_document.user = tokens;

    tkz->state = lxb_grammar_tokenizer_state_data;

    /* Process parsing */
    tkz->status = lxb_html_parse_chunk_process(tkz->html_parser, data, size);
    if (tkz->status != LXB_STATUS_OK) {
        return lxb_html_document_destroy(document);
    }

    tkz->status = lxb_html_parse_chunk_end(tkz->html_parser);
    if (tkz->status != LXB_STATUS_OK) {
        return lxb_html_document_destroy(document);
    }

    return document;
}

static lxb_html_token_t *
lxb_grammar_tokenizer_html_token(lxb_html_tokenizer_t *html_tkz,
                                 lxb_html_token_t *token, void *ctx)
{
    lxb_status_t status;
    lexbor_array_t *tokens;
    lxb_html_element_t *element;
    lxb_grammar_token_t *grammar_token;
    lxb_grammar_tokenizer_t *tkz = ctx;
    lxb_html_tree_t *tree = tkz->html_parser->tree;

    tokens = tree->document->dom_document.user;

    if (token->tag_id == LXB_TAG__EM_COMMENT) {
        return token;
    }
    else if (token->tag_id == LXB_TAG__TEXT) {
        while (token->begin < token->end) {
            token->begin = tkz->state(tkz, token, token->begin, token->end);
        }

        if (tkz->status != LXB_STATUS_OK) {
            return NULL;
        }

        return token;
    }
    else if (token->tag_id == LXB_TAG__END_OF_FILE) {
        grammar_token = lxb_grammar_token_create(tkz, LXB_GRAMMAR_TOKEN_ELEMENT);
        if (grammar_token == NULL) {
            return NULL;
        }

        grammar_token->type = LXB_GRAMMAR_TOKEN_END_OF_FILE;

        status = lexbor_array_push(tokens, grammar_token);
        if (status != LXB_STATUS_OK) {
            return NULL;
        }

        return token;
    }

    element = lxb_html_tree_create_element_for_token(tree, token, LXB_NS_HTML);
    if (element == NULL) {
        return NULL;
    }

    grammar_token = lxb_grammar_token_create(tkz, LXB_GRAMMAR_TOKEN_ELEMENT);
    if (grammar_token == NULL) {
        return NULL;
    }

    grammar_token->u.node = lxb_dom_interface_node(element);

    status = lexbor_array_push(tokens, grammar_token);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    return token;
}

static const lxb_char_t *
lxb_grammar_tokenizer_state_data(lxb_grammar_tokenizer_t *tkz,
                                 lxb_html_token_t *token,
                                 const lxb_char_t *data, const lxb_char_t *end)
{
    bool have_minus;
    double num_start, num_stop;
    lxb_char_t ch;
    lxb_status_t status;
    lexbor_array_t *tokens;
    lxb_grammar_token_t *g_token;
    lxb_grammar_document_t *document;
    lxb_grammar_token_type_t type, exclude_tupe;
    const lxb_char_t *start;

    static const lexbor_str_t ex_sort = lexbor_str("sort");

    document = tkz->html_parser->tree->document;
    tokens = document->dom_document.user;

    while (data < end) {
        switch (*data) {
            /*
             * U+0009 CHARACTER TABULATION (tab)
             * U+000A LINE FEED (LF)
             * U+000C FORM FEED (FF)
             * U+000D CARRIAGE RETURN (CR)
             * U+0020 SPACE
             */
            case 0x09:
            case 0x0A:
            case 0x0C:
            case 0x0D:
            case 0x20:
                g_token = lxb_grammar_token_create(tkz,
                                                   LXB_GRAMMAR_TOKEN_WHITESPACE);
                if (g_token == NULL) {
                    goto failed;
                }

                for (start = data; data < end; data++) {
                    switch (*data) {
                        case ' ':
                        case '\t':
                        case '\f':
                            break;

                        case '\r':
                        case '\n':
                            g_token->flags |= LXB_GRAMMAR_TOKEN_FLAGS_NEWLINE;
                            break;

                        default:
                            goto process_chars;
                    }
                }

            process_chars:

                status = lxb_grammar_tokenizer_data_copy(tkz, start,
                                                         data, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                break;

            /* U+002A ASTERISK (*) */
            case 0x2A:
                g_token = lxb_grammar_token_create(tkz,
                                                   LXB_GRAMMAR_TOKEN_ASTERISK);
                if (g_token == NULL) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                data++;
                break;

            /* U+002B PLUS SIGN (+) */
            case 0x2B:
                g_token = lxb_grammar_token_create(tkz, LXB_GRAMMAR_TOKEN_PLUS);
                if (g_token == NULL) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                data++;
                break;

            /* U+0021 EXCLAMATION MARK (!) */
            case 0x21:
                g_token = lxb_grammar_token_create(tkz,
                                                 LXB_GRAMMAR_TOKEN_EXCLAMATION);
                if (g_token == NULL) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                data++;
                break;

            /* U+003F QUESTION MARK (?) */
            case 0x3F:
                g_token = lxb_grammar_token_create(tkz,
                                                   LXB_GRAMMAR_TOKEN_QUESTION);
                if (g_token == NULL) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                data++;
                break;

            /* U+005E CIRCUMFLEX ACCENT (^) */
            case 0x5E:
                data++;

                if (data + 2 > end) {
                    goto failed;
                }

                if (data[0] == 'W' && data[1] == 'S') {
                    data += 2;
                    exclude_tupe = LXB_GRAMMAR_TOKEN_EXCLUDE_WS;
                }
                else if (data + 4 <= end
                         && lexbor_str_data_ncasecmp(data, ex_sort.data,
                                                     ex_sort.length))
                {
                    data += 4;
                    exclude_tupe = LXB_GRAMMAR_TOKEN_EXCLUDE_SORT;
                }
                else {
                    goto failed;
                }

                g_token = lxb_grammar_token_create(tkz, exclude_tupe);
                if (g_token == NULL) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                break;

            /* U+005B LEFT SQUARE BRACKET ([) */
            case 0x5B:
                g_token = lxb_grammar_token_create(tkz,
                                                LXB_GRAMMAR_TOKEN_LEFT_BRACKET);
                if (g_token == NULL) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                data++;
                break;

            /* U+005D RIGHT SQUARE BRACKET (]) */
            case 0x5D:
                g_token = lxb_grammar_token_create(tkz,
                                               LXB_GRAMMAR_TOKEN_RIGHT_BRACKET);
                if (g_token == NULL) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                data++;
                break;

            /* U+007C BAR (|) */
            case 0x7C:
                data++;

                if (data >= end || *data != '|') {
                    g_token = lxb_grammar_token_create(tkz,
                                                       LXB_GRAMMAR_TOKEN_BAR);
                }
                else {
                    data++;

                    g_token = lxb_grammar_token_create(tkz,
                                                  LXB_GRAMMAR_TOKEN_DOUBLE_BAR);
                }

                if (g_token == NULL) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                break;

            /* U+0026 ampersand (&) */
            case 0x26:
                data++;

                if (data >= end || *data != '&') {
                    g_token = lxb_grammar_token_create(tkz,
                                                    LXB_GRAMMAR_TOKEN_UNQUOTED);
                }
                else {
                    data++;

                    g_token = lxb_grammar_token_create(tkz,
                                                       LXB_GRAMMAR_TOKEN_AND);
                }

                if (g_token == NULL) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                break;

            /* U+0023 NUMBER SIGN (#) */
            case 0x23:
                g_token = lxb_grammar_token_create(tkz, LXB_GRAMMAR_TOKEN_HASH);
                if (g_token == NULL) {
                    goto failed;
                }

                status = lxb_grammar_tokenizer_data_copy(tkz, data, data + 1,
                                                         g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                data++;
                break;

            /* U+007B LEFT CURLY BRACKET ({) */
            case 0x7B:
                data++;

                if (data >= end) {
                    goto failed;
                }

                num_stop = -1.0;

                if (*data >= '0' && *data <= '9') {
                    num_start = lexbor_conv_data_to_double(&data,
                                                           (end - data));
                }
                else {
                    goto failed;
                }

                if (*data == ',') {
                    data++;
                    type = LXB_GRAMMAR_TOKEN_RANGE;

                    if (data >= end) {
                        goto create_period;
                    }

                    if (*data >= '0' && *data <= '9') {
                        num_stop = lexbor_conv_data_to_double(&data,
                                                              (end - data));
                    }
                }
                else {
                    type = LXB_GRAMMAR_TOKEN_COUNT;
                }

                if (*data != '}') {
                    goto failed;
                }

                data++;

create_period:

                g_token = lxb_grammar_token_create(tkz, type);
                if (g_token == NULL) {
                    goto failed;
                }

                if (type == LXB_GRAMMAR_TOKEN_RANGE) {
                    g_token->u.period.start = (long) num_start;
                    g_token->u.period.stop = (long) num_stop;
                }
                else {
                    g_token->u.count = (long) num_start;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                break;

            /* U+003D EQUALS SIGN (=) */
            case 0x3D:
                g_token = lxb_grammar_token_create(tkz,
                                                   LXB_GRAMMAR_TOKEN_EQUALS);
                if (g_token == NULL) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                data++;
                break;

            /*
             * U+0022 QUOTATION MARK (")
             * U+0027 APOSTROPHE (')
             */
            case 0x22:
            case 0x27:
                ch = *data;
                data++;

                for (start = data; data < end; data++) {
                    if (*data == ch) {
                        break;
                    }
                    else if (*data == '\\') {
                        data++;
                    }
                }

                g_token = lxb_grammar_token_create(tkz,
                                                   LXB_GRAMMAR_TOKEN_STRING);
                if (g_token == NULL) {
                    goto failed;
                }

                status = lxb_grammar_tokenizer_data_copy(tkz, start, data,
                                                         g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                data++;
                break;

            default:
                if (*data == '-' || (*data >= '0' && *data <= '9')) {
                    if (*data == '-') {
                        data++;
                        have_minus = true;
                    }
                    else {
                        have_minus = false;
                    }

                    if (data >= end) {
                        goto failed;
                    }

                    g_token = lxb_grammar_token_create(tkz,
                                                      LXB_GRAMMAR_TOKEN_NUMBER);
                    if (g_token == NULL) {
                        goto failed;
                    }

                    g_token->u.num = lexbor_conv_data_to_double(&data,
                                                                (end - data));
                    if (have_minus) {
                        g_token->u.num = -g_token->u.num;
                    }

                    status = lexbor_array_push(tokens, g_token);
                    if (status != LXB_STATUS_OK) {
                        goto failed;
                    }

                    break;
                }

                if ((*data >= 'a' && *data <= 'z')
                    || (*data >= 'A' && *data <= 'Z')
                    || (*data >= 0x80))
                {
                    for (start = data; data < end; data++) {

                        if (   (*data < 'a' || *data > 'z')
                            && (*data < 'A' || *data > 'Z')
                            && (*data < '0' || *data > '9')
                            && *data != '_' && *data != '-'
                            && *data < 0x80)
                        {
                            break;
                        }
                    }

                    if (data < end && *data == '(') {
                        data++;
                    }

                    g_token = lxb_grammar_token_create(tkz,
                                                    LXB_GRAMMAR_TOKEN_UNQUOTED);
                }
                else {
                    start = data;

                    /* TODO: need add support UTF-8 code point */
                    data++;

                    g_token = lxb_grammar_token_create(tkz,
                                                       LXB_GRAMMAR_TOKEN_DELIM);
                }

                if (g_token == NULL) {
                    goto failed;
                }

                status = lxb_grammar_tokenizer_data_copy(tkz, start, data,
                                                         g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                status = lexbor_array_push(tokens, g_token);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }

                break;
        }
    }

    return data;

failed:

    tkz->status = LXB_STATUS_ERROR;

    return end;
}

static lxb_status_t
lxb_grammar_tokenizer_data_copy(lxb_grammar_tokenizer_t *tkz,
                                const lxb_char_t *data, const lxb_char_t *end,
                                lxb_grammar_token_t *g_token)
{
    const size_t length = end - data;

    (void) lexbor_str_init(&g_token->u.str, tkz->html_parser->tkz->mraw,
                           length);
    if (g_token->u.str.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    (void) lexbor_str_append(&g_token->u.str, tkz->html_parser->tkz->mraw,
                             data, length);

    return LXB_STATUS_OK;
}
