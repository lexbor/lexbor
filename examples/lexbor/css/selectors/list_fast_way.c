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
    unsigned i;
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_selectors_t *selectors;

    static const lxb_char_t indent[] = "    ";
    static const size_t indent_length = sizeof(indent) / sizeof(lxb_char_t) - 1;

    static const char *slctrs[] =
    {
        ":not()",
        "div #hash [refs=i]",
        "div.class",
        "div + .class > #hash ~ :not(a) || [ref=some]:has(:not(div))",
        ":has(div, :not(as, 1%, .class), #hash)",
        ":has(:has(:has(:has(div))))",
        NULL
    };

    lxb_css_selector_list_t *lists[sizeof(slctrs) / sizeof(const char *)];

    /* Create parser. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL, NULL);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /* Create selectors. */

    selectors = lxb_css_selectors_create();
    status = lxb_css_selectors_init(selectors, 32);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    /*
     * This is important! It is necessary to bind the Selectors object
     * to the parser, otherwise a Selectors object (lxb_css_selectors_t)
     * will be created for each parsing.
     */
    lxb_css_parser_selectors_set(parser, selectors);

    /* Parse and get the log. */

    printf("Parse selectors:\n");

    i = 0;

    do {
        printf("%u) %s -- ", i + 1, slctrs[i]);

        lists[i] = lxb_css_selectors_parse(parser, (const lxb_char_t *) slctrs[i],
                                           strlen(slctrs[i]));
        if (parser->status != LXB_STATUS_OK) {
            printf("failed\n");

            (void) lxb_css_log_serialize(parser->log, callback, NULL,
                                         indent, indent_length);
            printf("\n");
        }
        else {
            if (lxb_css_log_length(lxb_css_parser_log(parser)) != 0) {
                printf("warning\n");

                (void) lxb_css_log_serialize(parser->log, callback, NULL,
                                             indent, indent_length);
                printf("\n");
            }
            else {
                printf("ok\n");
            }
        }
    }
    while (slctrs[++i] != NULL);

    /* Destroy resources for Parser and Selectors. */

    (void) lxb_css_selectors_destroy(selectors, false, true);
    (void) lxb_css_parser_destroy(parser, true);

    /* Selectors List Serialization. */

    i = 0;

    printf("\nResult:\n");

    do {
        printf("%u) ", i + 1);

        if (lists[i] != NULL) {
            (void) lxb_css_selector_serialize_list(lists[i], callback, NULL);

            printf("\n");
        }
        else {
            printf("-Empty result-\n");
        }
    }
    while (slctrs[++i] != NULL);

    /*
     * Destroy all Selector List memory.
     * Use any exist Selector List.
     */
    lxb_css_selector_list_destroy_memory(lists[1]);


    return EXIT_SUCCESS;
}
