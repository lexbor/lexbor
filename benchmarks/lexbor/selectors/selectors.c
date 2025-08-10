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


typedef struct {
    const lexbor_str_t *html;
    const lexbor_str_t *selector;
}
bm_ctx_t;


static const lexbor_str_t bm_selectors[] =
{
    lexbor_str("div"),
    lexbor_str("div span"),
    lexbor_str("p ~ p"),
    lexbor_str("p + p"),
    lexbor_str("div > p"),
    lexbor_str("div > div"),
    lexbor_str("div p:not(#p-5) a"),
    lexbor_str("div:has(a) a"),
    lexbor_str("div p:nth-child(n+2)"),
    lexbor_str("div p:nth-child(n+2 of div > p)"),
};

/*
lxb_status_t
callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}
*/

static lxb_status_t
find_callback(lxb_dom_node_t *node, lxb_css_selector_specificity_t spec,
              void *ctx)
{
    unsigned *count = ctx;

    (*count)++;
/*
    printf("%u) ", *count);
    (void) lxb_html_serialize_cb(node, callback, NULL);
    printf("\n");
*/
    return LXB_STATUS_OK;
}

BENCHMARK_BEGIN(css_small, context)
    unsigned count;
    bm_ctx_t *ctx;
    lxb_status_t status;
    const lexbor_str_t *slctr, *html;
    lxb_dom_node_t *body;
    lxb_selectors_t *selectors;
    lxb_html_document_t *document;
    lxb_css_parser_t *parser;
    lxb_css_selector_list_t *list;

    ctx = context;
    html = ctx->html;
    slctr = ctx->selector;
    count = 0;

    /* Create HTML Document. */
    document = lxb_html_document_create();
    status = lxb_html_document_parse(document, html->data, html->length);
    test_eq(status, LXB_STATUS_OK);

    /* Create CSS parser. */
    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    /* Selectors. */
    selectors = lxb_selectors_create();
    status = lxb_selectors_init(selectors);
    test_eq(status, LXB_STATUS_OK);

    lxb_selectors_opt_set(selectors, LXB_SELECTORS_OPT_MATCH_FIRST);

    /* Parse selectors. */
    list = lxb_css_selectors_parse(parser, slctr->data, slctr->length);
    test_eq(parser->status, LXB_STATUS_OK);

    body = lxb_dom_interface_node(document);

BENCHMARK_CODE
    status = lxb_selectors_find(selectors, body, list, find_callback, &count);
    test_eq(status, LXB_STATUS_OK);
BENCHMARK_CODE_END

    /* Destroy Selectors object. */
    lxb_selectors_destroy(selectors, true);

    /* Destroy resources for CSS Parser. */
    lxb_css_parser_destroy(parser, true);

    /* Destroy all object for all CSS Selector List. */
    lxb_css_selector_list_destroy_memory(list);

    /* Destroy HTML Document. */
    lxb_html_document_destroy(document);
BENCHMARK_END

int
main(int argc, const char * argv[])
{
    bm_ctx_t context;
    lexbor_str_t html;

    if (argc != 2) {
        printf("Usage:\n\tselectors path_to_html_file.html\n");
        return EXIT_FAILURE;
    }

    html.data = lexbor_fs_file_easy_read((const lxb_char_t *) argv[1], &html.length);
    test_ne(html.data, NULL);

    context.html = &html;

    BENCHMARK_INIT;

    for (size_t i = 0; i < sizeof(bm_selectors) / sizeof(lexbor_str_t); i++) {
        context.selector = &bm_selectors[i];
        BENCHMARK_ADD(css_small, bm_selectors[i].data, 10000, &context);
    }

    lexbor_free(html.data);

    return EXIT_SUCCESS;
}
