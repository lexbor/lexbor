/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/syntax/tokenizer.h>

#include "../base.h"


static lxb_css_syntax_token_t *
token_callback_done(lxb_css_syntax_tokenizer_t *tkz,
                    lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
serialization(const lxb_char_t *data, size_t len, void *ctx);


int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_css_syntax_tokenizer_t *tkz;

    static const lxb_char_t html[][64] = {
        ":root {\n",
        "  --blue: #007", "bff;\n",
        "}\n",
        "html {\n",
        "  font-family: sans-serif;\n",
        "  line-h", "eight: 1", ".15;\n",
        "}\n",
        "\0"
    };

    /* Initialization */
    tkz = lxb_css_syntax_tokenizer_create();
    status = lxb_css_syntax_tokenizer_init(tkz);

    if (status != LXB_STATUS_OK) {
        FAILED("Failed to create CSS:Syntax parser");
    }

    /* Sets callback for ready tokens */
    lxb_css_syntax_tokenizer_token_cb_set(tkz, token_callback_done, NULL);

    /* Parse CSS */
    status = lxb_css_syntax_tokenizer_begin(tkz);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to parse CSS");
    }

    for (size_t i = 0; html[i][0] != '\0'; i++) {
        status = lxb_css_syntax_tokenizer_chunk(tkz, html[i],
                                                strlen((const char *) html[i]));
        if (status != LXB_STATUS_OK) {
            FAILED("Failed to parse CSS chunk");
        }
    }

    status = lxb_css_syntax_tokenizer_end(tkz);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to parse CSS");
    }

    /* Destroy tokenizer */
    lxb_css_syntax_tokenizer_destroy(tkz);

    return EXIT_SUCCESS;
}

static lxb_css_syntax_token_t *
token_callback_done(lxb_css_syntax_tokenizer_t *tkz,
                    lxb_css_syntax_token_t *token, void *ctx)
{
    if (lxb_css_syntax_token_type(token) == LXB_CSS_SYNTAX_TOKEN_COMMENT) {
        return token;
    }

    lxb_status_t status = lxb_css_syntax_tokenizer_make_data(tkz, token);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    status = lxb_css_syntax_token_serialize_cb(token, serialization, NULL);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    return token;
}

static lxb_status_t
serialization(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, data);

    return LXB_STATUS_OK;
}
