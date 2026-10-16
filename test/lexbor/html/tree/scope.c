/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/html/tree.h>
#include <lexbor/html/tree/open_elements.h>


static lxb_html_tree_t *
scope_tree_create(lxb_html_tokenizer_t **tkz);

static lxb_dom_node_t
scope_node(lxb_tag_id_t tag_id, lxb_ns_id_t ns);

static void
scope_tree_destroy(lxb_html_tree_t *tree, lxb_html_tokenizer_t *tkz);


TEST_BEGIN(tbody_thead_tfoot_scope)
{
    lxb_html_tokenizer_t *tkz;
    lxb_html_tree_t *tree = scope_tree_create(&tkz);

    lxb_dom_node_t html = scope_node(LXB_TAG_HTML, LXB_NS_HTML);
    lxb_dom_node_t tbody = scope_node(LXB_TAG_TBODY, LXB_NS_HTML);
    lxb_dom_node_t svg_table = scope_node(LXB_TAG_TABLE, LXB_NS_SVG);
    lxb_dom_node_t html_table = scope_node(LXB_TAG_TABLE, LXB_NS_HTML);

    test_eq(lxb_html_tree_open_elements_push(tree, &html), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &tbody), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &svg_table), LXB_STATUS_OK);

    test_eq(lxb_html_tree_element_in_scope_tbody_thead_tfoot(tree), &tbody);

    lxb_html_tree_clean(tree);

    test_eq(lxb_html_tree_open_elements_push(tree, &html), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &tbody), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &html_table), LXB_STATUS_OK);

    test_eq(lxb_html_tree_element_in_scope_tbody_thead_tfoot(tree), NULL);

    scope_tree_destroy(tree, tkz);
}
TEST_END

TEST_BEGIN(td_th_scope)
{
    lxb_html_tokenizer_t *tkz;
    lxb_html_tree_t *tree = scope_tree_create(&tkz);

    lxb_dom_node_t html = scope_node(LXB_TAG_HTML, LXB_NS_HTML);
    lxb_dom_node_t td = scope_node(LXB_TAG_TD, LXB_NS_HTML);
    lxb_dom_node_t svg_table = scope_node(LXB_TAG_TABLE, LXB_NS_SVG);
    lxb_dom_node_t html_table = scope_node(LXB_TAG_TABLE, LXB_NS_HTML);

    test_eq(lxb_html_tree_open_elements_push(tree, &html), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &td), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &svg_table), LXB_STATUS_OK);

    test_eq(lxb_html_tree_element_in_scope_td_th(tree), &td);

    lxb_html_tree_clean(tree);

    test_eq(lxb_html_tree_open_elements_push(tree, &html), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &td), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &html_table), LXB_STATUS_OK);

    test_eq(lxb_html_tree_element_in_scope_td_th(tree), NULL);

    scope_tree_destroy(tree, tkz);
}
TEST_END

TEST_BEGIN(option_optgroup_scope)
{
    lxb_html_tokenizer_t *tkz;
    lxb_html_tree_t *tree = scope_tree_create(&tkz);

    lxb_dom_node_t html = scope_node(LXB_TAG_HTML, LXB_NS_HTML);
    lxb_dom_node_t option = scope_node(LXB_TAG_OPTION, LXB_NS_HTML);
    lxb_dom_node_t math_mi = scope_node(LXB_TAG_MI, LXB_NS_MATH);
    lxb_dom_node_t svg_path = scope_node(LXB_TAG_PATH, LXB_NS_SVG);

    test_eq(lxb_html_tree_open_elements_push(tree, &html), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &option), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &svg_path), LXB_STATUS_OK);

    test_eq(lxb_html_tree_element_in_scope_option_optgroup(tree), &option);

    lxb_html_tree_clean(tree);

    test_eq(lxb_html_tree_open_elements_push(tree, &html), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &option), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &math_mi), LXB_STATUS_OK);

    test_eq(lxb_html_tree_element_in_scope_option_optgroup(tree), NULL);

    scope_tree_destroy(tree, tkz);
}
TEST_END

TEST_BEGIN(p_button_scope)
{
    lxb_html_tokenizer_t *tkz;
    lxb_html_tree_t *tree = scope_tree_create(&tkz);

    lxb_dom_node_t html = scope_node(LXB_TAG_HTML, LXB_NS_HTML);
    lxb_dom_node_t p = scope_node(LXB_TAG_P, LXB_NS_HTML);
    lxb_dom_node_t svg_p = scope_node(LXB_TAG_P, LXB_NS_SVG);
    lxb_dom_node_t div = scope_node(LXB_TAG_DIV, LXB_NS_HTML);
    lxb_dom_node_t button = scope_node(LXB_TAG_BUTTON, LXB_NS_HTML);
    lxb_dom_node_t svg_button = scope_node(LXB_TAG_BUTTON, LXB_NS_SVG);
    lxb_dom_node_t foreign_object = scope_node(LXB_TAG_FOREIGNOBJECT,
                                               LXB_NS_SVG);

    test_eq(lxb_html_tree_open_elements_push(tree, &html), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &div), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &div), LXB_STATUS_OK);
    test_eq(lxb_html_tree_element_in_scope(tree, LXB_TAG_P, LXB_NS_HTML,
                                           LXB_HTML_TAG_CATEGORY_SCOPE_BUTTON),
            NULL);

    /* A p inserted below the current node. */
    test_eq(lxb_html_tree_open_elements_insert_after(tree, &p, 0),
            LXB_STATUS_OK);
    test_eq(lxb_html_tree_element_in_scope(tree, LXB_TAG_P, LXB_NS_HTML,
                                           LXB_HTML_TAG_CATEGORY_SCOPE_BUTTON),
            &p);

    /* A button above the p hides it until the button is popped. */
    test_eq(lxb_html_tree_open_elements_push(tree, &button), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &svg_button), LXB_STATUS_OK);
    test_eq(lxb_html_tree_element_in_scope(tree, LXB_TAG_P, LXB_NS_HTML,
                                           LXB_HTML_TAG_CATEGORY_SCOPE_BUTTON),
            NULL);

    /* The test nodes have no owner document for the pop callbacks. */
    lexbor_array_pop(tree->open_elements);
    lexbor_array_pop(tree->open_elements);
    test_eq(lxb_html_tree_element_in_scope(tree, LXB_TAG_P, LXB_NS_HTML,
                                           LXB_HTML_TAG_CATEGORY_SCOPE_BUTTON),
            &p);

    /* A button removed from below the current node. */
    test_eq(lxb_html_tree_open_elements_push(tree, &button), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &div), LXB_STATUS_OK);
    test_eq(lxb_html_tree_element_in_scope(tree, LXB_TAG_P, LXB_NS_HTML,
                                           LXB_HTML_TAG_CATEGORY_SCOPE_BUTTON),
            NULL);

    lxb_html_tree_open_elements_remove_by_node(tree, &button);
    test_eq(lxb_html_tree_element_in_scope(tree, LXB_TAG_P, LXB_NS_HTML,
                                           LXB_HTML_TAG_CATEGORY_SCOPE_BUTTON),
            &p);

    lxb_html_tree_open_elements_remove_by_node(tree, &p);
    test_eq(lxb_html_tree_element_in_scope(tree, LXB_TAG_P, LXB_NS_HTML,
                                           LXB_HTML_TAG_CATEGORY_SCOPE_BUTTON),
            NULL);

    /*
     * Foreign elements: a p in SVG is not an HTML p, and foreignObject is a
     * scope boundary.
     */
    lxb_html_tree_clean(tree);

    test_eq(lxb_html_tree_open_elements_push(tree, &html), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &svg_p), LXB_STATUS_OK);
    test_eq(lxb_html_tree_element_in_scope(tree, LXB_TAG_P, LXB_NS_HTML,
                                           LXB_HTML_TAG_CATEGORY_SCOPE_BUTTON),
            NULL);

    test_eq(lxb_html_tree_open_elements_push(tree, &p), LXB_STATUS_OK);
    test_eq(lxb_html_tree_open_elements_push(tree, &foreign_object),
            LXB_STATUS_OK);
    test_eq(lxb_html_tree_element_in_scope(tree, LXB_TAG_P, LXB_NS_HTML,
                                           LXB_HTML_TAG_CATEGORY_SCOPE_BUTTON),
            NULL);

    scope_tree_destroy(tree, tkz);
}
TEST_END

static lxb_html_tree_t *
scope_tree_create(lxb_html_tokenizer_t **tkz)
{
    lxb_status_t status;
    lxb_html_tree_t *tree;

    *tkz = lxb_html_tokenizer_create();
    if (*tkz == NULL) {
        TEST_FAILURE("Failed to create tokenizer");
    }

    status = lxb_html_tokenizer_init(*tkz);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to init tokenizer");
    }

    tree = lxb_html_tree_create();
    if (tree == NULL) {
        TEST_FAILURE("Failed to create tree");
    }

    status = lxb_html_tree_init(tree, *tkz);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to init tree");
    }

    return tree;
}

static lxb_dom_node_t
scope_node(lxb_tag_id_t tag_id, lxb_ns_id_t ns)
{
    lxb_dom_node_t node = {0};

    node.local_name = tag_id;
    node.ns = ns;

    return node;
}

static void
scope_tree_destroy(lxb_html_tree_t *tree, lxb_html_tokenizer_t *tkz)
{
    lxb_html_tokenizer_unref(tkz);
    lxb_html_tree_unref(tree);
}

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(tbody_thead_tfoot_scope);
    TEST_ADD(td_th_scope);
    TEST_ADD(option_optgroup_scope);
    TEST_ADD(p_button_scope);

    TEST_RUN("lexbor/html/tree/scope");
    TEST_RELEASE();
}
