/*
 * Copyright (C) 2019-2020 Alexander Borisov
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
    const lxb_char_t *tag, *name;
    lxb_html_token_attr_t *attr;

    attr = token->attr_first;

    /* Skip all #text or without attributes tokens */
    if (token->tag_id == LXB_TAG__TEXT || attr == NULL) {
        return token;
    }

    tag = lxb_tag_name_by_id(lxb_html_tokenizer_tags(tkz), token->tag_id, NULL);
    if (tag == NULL) {
        FAILED("Failed to get token name");
    }

    printf("\"%s\" attributes:\n", tag);

    while (attr != NULL) {
        name = lxb_html_token_attr_name(attr, NULL);

        if (name != NULL) {
            printf("    Name: %s; ", name);
        }
        else {
            /* This can only happen for the DOCTYPE token. */

            printf("    Name: <NOT SET>; \n");
        }

        if (attr->value != NULL) {
            printf("Value: %.*s\n", (int) attr->value_size, attr->value);
        }
        else {
            printf("Value: <NOT SET>\n");
        }

        attr = attr->next;
    }

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

    lxb_html_tokenizer_destroy(tkz);

    return 0;
}
