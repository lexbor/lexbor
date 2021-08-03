/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/css.h>

lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}

int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_selector_list_t *list;

    static const lxb_char_t indent[] = "    ";
    static const size_t indent_length = sizeof(indent) / sizeof(lxb_char_t) - 1;

    static const lxb_char_t slctrs[] = ":has(div, :not(as, 1%, .class), #hash)";

    /* Create parser. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Parse and get the log. */

    printf("Parse: %s\n", (const char *) slctrs);

    list = lxb_css_selectors_parse(parser, slctrs,
                                   sizeof(slctrs) / sizeof(lxb_char_t) - 1);
    if (parser->status != LXB_STATUS_OK) {
        printf("Something went wrong\n");
        return EXIT_FAILURE;
    }

    /* Selector List Serialization. */

    printf("Result: ");
    (void) lxb_css_selector_serialize_list(list, callback, NULL);
    printf("\n");

    if (lxb_css_log_length(lxb_css_parser_log(parser)) != 0) {
        printf("Log:\n");

        (void) lxb_css_log_serialize(parser->log, callback, NULL,
                                     indent, indent_length);
        printf("\n");
    }

    /* Destroy resources for Parser. */
    (void) lxb_css_parser_destroy(parser, true);

    /* Destroy all Selector List memory. */
    lxb_css_selector_list_destroy_memory(list);

    return EXIT_SUCCESS;
}
