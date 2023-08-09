/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/css.h>


lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    return LXB_STATUS_OK;
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t length)
{
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;

    /* Create CSS parser. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Parse. */

    sst = lxb_css_stylesheet_parse(parser, data, length);
    if (sst == NULL) {
        goto done;
    }

    /* Serialization. */

    status = lxb_css_rule_serialize(sst->root, callback, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

done:

    /* Destroy resources for CSS Parser. */
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_css_stylesheet_destroy(sst, true);

    return EXIT_SUCCESS;
}
