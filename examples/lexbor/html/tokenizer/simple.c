/*
 * Copyright (C) 2019-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tokenizer.h"


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
    char qo;
    lxb_html_token_attr_t *attr;

    /* Last token, end of parsing */
    if (token->tag_id == LXB_TAG__END_OF_FILE) {
        return token;
    }

    /* Text token */
    if (token->tag_id == LXB_TAG__TEXT) {
        printf("%.*s", (int) (token->end - token->begin), token->begin);

        return token;
    }

    /* Tag name */
    if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
        printf("</%.*s", (int) (token->end - token->begin), token->begin);
    }
    else {
        printf("<%.*s", (int) (token->end - token->begin), token->begin);
    }

    /* Attributes */
    attr = token->attr_first;

    while (attr != NULL) {
        /* Name */
        printf(" %.*s", (int) (attr->name_end - attr->name_begin),
               attr->name_begin);

        /* Value */
        if (attr->value_begin) {
            /* Get original quote */
            qo = (char) *(attr->value_begin - 1);

            /* Attribute have no quote */
            if (qo == '=') {
                printf("=%.*s", (int) (attr->value_end - attr->value_begin),
                       attr->value_begin);
            }
            else {
                printf("=%c%.*s%c", qo, (int) (attr->value_end - attr->value_begin),
                       attr->value_begin, qo);
            }
        }

        attr = attr->next;
    }

    printf(">");

    return token;
}

int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_html_tokenizer_t *tkz;

    const lxb_char_t data[] = "<div a='b' enabled> &copy; Hi<span c=\"d\" e=f>"
                              " my </span>friend</div>";

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

    printf("\n");

    lxb_html_tokenizer_destroy(tkz);

    return 0;
}
