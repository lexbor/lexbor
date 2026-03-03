/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "benchmark.h"

#include <lexbor/core/fs.h>
#include <lexbor/selectors/selectors.h>
#include <lexbor/html/html.h>
#include <lexbor/css/css.h>


const char *
get_file_name(const char *path);


static lxb_html_token_t *
token_callback(lxb_html_tokenizer_t *tkz, lxb_html_token_t *token, void *ctx)
{
    return token;
}

BENCHMARK_BEGIN(input_validation, context)
    lxb_status_t status;
    lexbor_str_t *html;
    lxb_html_tokenizer_t *tkz;

    html = context;

    /* Create. */
    tkz = lxb_html_tokenizer_create();
    status = lxb_html_tokenizer_init(tkz);
    test_eq(status, LXB_STATUS_OK);

    /* Set callback for token. */
    lxb_html_tokenizer_callback_token_done_set(tkz, token_callback, NULL);

    /* Enable input validation. */
    lxb_html_tokenizer_input_validation_set(tkz, true);

    /* Parse. */
    status = lxb_html_tokenizer_begin(tkz);
    test_eq(status, LXB_STATUS_OK);

BENCHMARK_CODE
    status = lxb_html_tokenizer_chunk(tkz, html->data, html->length);
    test_eq(status, LXB_STATUS_OK);
BENCHMARK_CODE_END

    /* Destroy. */
    lxb_html_tokenizer_destroy(tkz);
BENCHMARK_END

int
main(int argc, const char * argv[])
{
    lexbor_str_t html;

    if (argc < 2) {
        printf("Usage:\n\tinput_validation path_to_html_file.html [...]\n");
        return EXIT_FAILURE;
    }

    BENCHMARK_INIT;

    for (int i = 1; i < argc; i++) {
        html.data = lexbor_fs_file_easy_read((const lxb_char_t *) argv[i],
                                             &html.length);
        test_ne(html.data, NULL);

        BENCHMARK_ADD(input_validation, get_file_name(argv[i]), 1000, &html);

        lexbor_free(html.data);
    }

    return EXIT_SUCCESS;
}

const char *
get_file_name(const char *path)
{
    size_t len = strlen(path);
    const char *file_name;

    for (file_name = path + len; file_name > path; file_name--) {
        if (*file_name == '/') {
            file_name += 1;

            if (*file_name == '\0') {
                file_name = path;
            }

            break;
        }
    }

    return file_name;
}
