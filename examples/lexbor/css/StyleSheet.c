/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "base.h"

#include <lexbor/core/fs.h>
#include <lexbor/css/css.h>


lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, data);

    return LXB_STATUS_OK;
}

int
main(int argc, const char *argv[])
{
    size_t css_len;
    lxb_char_t *css;
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    const lxb_char_t *fl;

    if (argc != 2) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "\tStyleSheet <file>\n");
        FAILED("Invalid number of arguments");
    }

    fl = (const lxb_char_t *) argv[1];

    css = lexbor_fs_file_easy_read(fl, &css_len);
    if (css == NULL) {
        FAILED("Failed to read CSS file");
    }

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to create CSS Parser");
    }

    sst = lxb_css_stylesheet_parse(parser, css, css_len);

    (void) lexbor_free(css);
    (void) lxb_css_parser_destroy(parser, true);

    if (sst == NULL) {
        FAILED("Failed to parse CSS");
    }

    status = lxb_css_rule_serialize(sst->root, callback, NULL);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to serialize StyleSheet");
    }

    printf("\n");

    (void) lxb_css_stylesheet_destroy(sst, true);

    return EXIT_SUCCESS;
}
