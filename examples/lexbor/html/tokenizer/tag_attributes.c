/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tokenizer.h"
#include "lexbor/html/token_attr.h"


#define FAILED(...)                                                            \
    do {                                                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
    while (0)


static lxb_html_token_t *
token_callback(lxb_html_tokenizer_t *tkz, lxb_html_token_t *token, void *ctx)
{
    lxb_status_t status;
    lexbor_mraw_t *mraw;
    const lxb_char_t *tag;
    lexbor_str_t name = {0};
    lexbor_str_t value = {0};
    lxb_html_parser_char_t pc = {0};

    lxb_html_token_attr_t *attr = token->attr_first;
    lexbor_hash_t *tags = lxb_html_tokenizer_tags(tkz);

    /* Skip all #text or without attributes tokens */
    if (token->tag_id == LXB_HTML_TOKEN_TYPE_TEXT || attr == NULL) {
        return token;
    }

    mraw = lxb_html_tokenizer_mraw(tkz);

    if (token->tag_id == LXB_TAG__UNDEF) {
        token->tag_id = lxb_html_token_tag_id_from_data(tags, token, mraw);
        if (token->tag_id == LXB_TAG__UNDEF) {
            lxb_html_tokenizer_status_set(tkz, LXB_STATUS_ERROR);
            return NULL;
        }
    }

    tag = lxb_tag_name_by_id(tags, token->tag_id, NULL);
    if (tag == NULL) {
        FAILED("Failed to get token name");
    }

    printf("\"%s\" attributes:\n", tag);

    while (attr != NULL) {
        name.length = 0;
        value.length = 0;

        status = lxb_html_token_attr_parse(attr, &pc, &name, &value, mraw);
        if (status != LXB_STATUS_OK) {
            FAILED("Failed to parse token attributes");
        }

        printf("    Name: %s; ", name.data);

        if (value.data != NULL) {
            printf("Value: %s\n", value.data);
        }
        else {
            printf("Value: <NOT SET>\n");
        }

        attr = attr->next;
    }

    lexbor_str_destroy(&name, mraw, false);
    lexbor_str_destroy(&value, mraw, false);

    return token;
}

int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_html_tokenizer_t *tkz;

    const lxb_char_t data[] = "<div id=one-id class=silent ref='some &copy; a'>"
                              "<option-one enabled>"
                              "<option-two enabled='&#81'>"
                              "</div>";

    printf("HTML:\n%s\n\n", (char *) data);
    printf("Result:\n");

    tkz = lxb_html_tokenizer_create();
    status = lxb_html_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to create tokenizer object");
    }

    status = lxb_html_tokenizer_tags_make(tkz, 64);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to create tokenizer tags");
    }

    /* Without copying input buffer */
    lxb_html_tokenizer_opt_set(tkz, LXB_HTML_TOKENIZER_OPT_WO_COPY);
    /* Set callback for token */
    lxb_html_tokenizer_callback_token_done_set(tkz, token_callback, NULL);

    status = lxb_html_tokenizer_begin(tkz);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to prepare tokenizer object for parsing");
    }

    status = lxb_html_tokenizer_chunk(tkz, data, (sizeof(data) - 1));
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to parse the html data");
    }

    status = lxb_html_tokenizer_end(tkz);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to ending of parsing the html data");
    }

    lxb_html_tokenizer_tags_destroy(tkz);
    lxb_html_tokenizer_destroy(tkz);

    return 0;
}
