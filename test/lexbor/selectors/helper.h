/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_TEST_SELECTORS_HELPER_H
#define LEXBOR_TEST_SELECTORS_HELPER_H

#include <lexbor/html/html.h>
#include <lexbor/css/css.h>
#include <lexbor/selectors/selectors.h>

#include <unit/test.h>


typedef struct {
    const char *id;
    size_t      selector;
}
test_selector_expected_t;

typedef struct {
    const char               *html;
    const char               *selector;
    test_selector_expected_t expected[16];
}
test_selector_case_t;

typedef struct {
    lxb_dom_node_t                 *node;
    const lxb_css_selector_list_t  *list;
    lxb_css_selector_specificity_t specificity;
}
test_selector_match_t;

typedef struct {
    lxb_selectors_t       *selectors;
    test_selector_match_t matches[16];
    size_t                length;
    lxb_status_t          status;
}
test_selector_ctx_t;


static lxb_status_t
test_selector_callback(lxb_dom_node_t *node,
                       lxb_css_selector_specificity_t spec, void *ctx)
{
    test_selector_match_t *match;
    test_selector_ctx_t *context = ctx;

    if (context->length == sizeof(context->matches)
                           / sizeof(test_selector_match_t))
    {
        return LXB_STATUS_ERROR_SMALL_BUFFER;
    }

    match = &context->matches[context->length++];
    match->node = node;
    match->list = lxb_selectors_selector(context->selectors);
    match->specificity = spec;

    if (context->length == 2) {
        return context->status;
    }

    return LXB_STATUS_OK;
}

static bool
test_selector_in_root(lxb_dom_node_t *node, lxb_dom_node_t *root,
                       lxb_selectors_opt_t options, bool match_node)
{
    if (node == root) {
        return match_node || (options & LXB_SELECTORS_OPT_MATCH_ROOT) != 0;
    }

    if (match_node) {
        return false;
    }

    while (node->parent != NULL) {
        node = node->parent;

        if (node == root) {
            return true;
        }
    }

    return false;
}

TEST_BEGIN_ARGS(test_selector_check, lxb_selectors_t *selectors,
                lxb_dom_node_t *root, const lxb_css_selector_list_t *list,
                const test_selector_match_t *expected, size_t length,
                lxb_selectors_opt_t options, lxb_status_t control,
                bool match_node)
{
    size_t count;
    lxb_status_t status, need_status;
    test_selector_ctx_t ctx;
    test_selector_match_t matches[16];

    count = 0;

    for (size_t i = 0; i < length; i++) {
        if (!test_selector_in_root(expected[i].node, root, options,
                                   match_node))
        {
            continue;
        }

        if ((options & LXB_SELECTORS_OPT_MATCH_FIRST) != 0 && count != 0
            && matches[count - 1].node == expected[i].node)
        {
            continue;
        }

        matches[count++] = expected[i];
    }

    lxb_selectors_opt_set(selectors, options);

    /* Reuse the same object after normal, stopped and failed calls. */
    for (unsigned reuse = 0; reuse < 2; reuse++) {
        ctx.selectors = selectors;
        ctx.length = 0;
        ctx.status = reuse == 0 ? control : LXB_STATUS_OK;
        need_status = LXB_STATUS_OK;
        length = count;

        if (ctx.status != LXB_STATUS_OK && length >= 2) {
            length = 2;
            need_status = ctx.status;

            if (!match_node && need_status == LXB_STATUS_STOP) {
                need_status = LXB_STATUS_OK;
            }
        }

        if (match_node) {
            status = lxb_selectors_match_node(selectors, root, list,
                                              test_selector_callback, &ctx);
        }
        else {
            status = lxb_selectors_find(selectors, root, list,
                                        test_selector_callback, &ctx);
        }

        test_eq(status, need_status);
        test_eq(ctx.length, length);

        for (size_t i = 0; i < length; i++) {
            test_eq(ctx.matches[i].node, matches[i].node);
            test_eq(ctx.matches[i].list, matches[i].list);
            test_eq(ctx.matches[i].specificity, matches[i].specificity);
        }
    }
}
TEST_END

TEST_BEGIN_ARGS(test_selector_case, const test_selector_case_t *entry)
{
    size_t count, length, id_length;
    const lxb_char_t *id;
    lxb_status_t status;
    lxb_dom_node_t *node, *root, *nodes[128];
    lxb_selectors_t *selectors;
    lxb_css_parser_t *parser;
    lxb_css_selector_list_t *list;
    lxb_html_document_t *document;
    const lxb_css_selector_list_t *selected;
    test_selector_match_t expected[16];

    static const lxb_selectors_opt_t options[] = {
        LXB_SELECTORS_OPT_DEFAULT,
        LXB_SELECTORS_OPT_MATCH_ROOT,
        LXB_SELECTORS_OPT_MATCH_FIRST,
        LXB_SELECTORS_OPT_MATCH_ROOT | LXB_SELECTORS_OPT_MATCH_FIRST
    };

    static const lxb_status_t controls[] = {
        LXB_STATUS_OK, LXB_STATUS_STOP, LXB_STATUS_ERROR
    };

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document,
                                     (const lxb_char_t *) entry->html,
                                     strlen(entry->html));
    test_eq(status, LXB_STATUS_OK);

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    list = lxb_css_selectors_parse(parser,
                                   (const lxb_char_t *) entry->selector,
                                   strlen(entry->selector));
    test_ne(list, NULL);
    test_eq(parser->status, LXB_STATUS_OK);

    selectors = lxb_selectors_create();
    status = lxb_selectors_init(selectors);
    test_eq(status, LXB_STATUS_OK);

    root = lxb_dom_interface_node(document);
    node = root;
    count = 0;

    do {
        test_eq(count < sizeof(nodes) / sizeof(lxb_dom_node_t *), true);
        nodes[count++] = node;

        if (node->first_child != NULL) {
            node = node->first_child;
            continue;
        }

        while (node != root && node->next == NULL) {
            node = node->parent;
        }

        if (node == root) {
            break;
        }

        node = node->next;
    }
    while (node != NULL);

    length = 0;

    while (length < sizeof(entry->expected) / sizeof(test_selector_expected_t)
           && entry->expected[length].id != NULL)
    {
        expected[length].node = NULL;

        for (size_t i = 0; i < count; i++) {
            if (nodes[i]->type != LXB_DOM_NODE_TYPE_ELEMENT) {
                continue;
            }

            id = lxb_dom_element_id(lxb_dom_interface_element(nodes[i]),
                                    &id_length);
            if (id != NULL
                && id_length == strlen(entry->expected[length].id)
                && memcmp(id, entry->expected[length].id, id_length) == 0)
            {
                expected[length].node = nodes[i];
                break;
            }
        }

        test_ne(expected[length].node, NULL);

        selected = list;

        for (size_t i = 0; i < entry->expected[length].selector; i++) {
            test_ne(selected, NULL);
            selected = selected->next;
        }

        test_ne(selected, NULL);
        expected[length].list = selected;
        expected[length].specificity = selected->specificity;
        length++;
    }

    for (size_t o = 0; o < sizeof(options) / sizeof(lxb_selectors_opt_t); o++) {
        for (size_t c = 0; c < sizeof(controls) / sizeof(lxb_status_t); c++) {
            for (size_t n = 0; n < count; n++) {
                if (nodes[n]->type == LXB_DOM_NODE_TYPE_ELEMENT
                    || nodes[n]->type == LXB_DOM_NODE_TYPE_DOCUMENT)
                {
                    TEST_STACK_PUSH();
                    return_val = test_selector_check(my_test, selectors,
                                                     nodes[n], list, expected,
                                                     length, options[o],
                                                     controls[c], false);
                    TEST_STACK_POP();
                    if (my_test->error) {
                        goto done;
                    }
                }

                TEST_STACK_PUSH();
                return_val = test_selector_check(my_test, selectors, nodes[n],
                                                 list, expected, length,
                                                 options[o], controls[c], true);
                TEST_STACK_POP();
                if (my_test->error) {
                    goto done;
                }
            }
        }
    }

done:

    if (my_test->error) {
        TEST_PRINTLN("Selector: %s", entry->selector);
        TEST_PRINTLN("HTML: %s", entry->html);
    }

    (void) lxb_selectors_destroy(selectors, true);
    (void) lxb_css_parser_destroy(parser, true);
    lxb_css_selector_list_destroy_memory(list);
    lxb_html_document_destroy(document);
}
TEST_END

#endif /* LEXBOR_TEST_SELECTORS_HELPER_H */
