/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tree/insertion_mode.h"
#include "lexbor/html/tree/open_elements.h"
#include "lexbor/html/tree/active_formatting.h"


static bool
lxb_html_tree_insertion_mode_in_row_thtd(lxb_html_tree_t *tree,
                                         lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_row_tr_closed(lxb_html_tree_t *tree,
                                              lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_row_ct_open_closed(lxb_html_tree_t *tree,
                                                   lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_row_tbtfth_closed(lxb_html_tree_t *tree,
                                                  lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_row_bcht_closed(lxb_html_tree_t *tree,
                                                lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_row_anything_else(lxb_html_tree_t *tree,
                                                  lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_row_anything_else_closed(lxb_html_tree_t *tree,
                                                         lxb_html_token_t *token);

static void
lxb_html_tree_clear_stack_back_to_table_row(lxb_html_tree_t *tree);


#include "lexbor/html/tree/insertion_mode/in_row_res.h"


bool
lxb_html_tree_insertion_mode_in_row(lxb_html_tree_t *tree,
                                    lxb_html_token_t *token)
{
    if (token->tag_id >= LXB_TAG__LAST_ENTRY) {
        if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
            return lxb_html_tree_insertion_mode_in_row_anything_else_closed(tree, token);
        }

        return lxb_html_tree_insertion_mode_in_row_anything_else(tree, token);
    }

    if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
        return lxb_html_tree_insertion_mode_in_row_closed_res[token->tag_id](tree, token);
    }

    return lxb_html_tree_insertion_mode_in_row_res[token->tag_id](tree, token);
}

/*
 * "th", "td"
 */
static bool
lxb_html_tree_insertion_mode_in_row_thtd(lxb_html_tree_t *tree,
                                         lxb_html_token_t *token)
{
    lxb_html_element_t *element;

    lxb_html_tree_clear_stack_back_to_table_row(tree);

    element = lxb_html_tree_insert_html_element(tree, token);
    if (element == NULL) {
        tree->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

        return lxb_html_tree_process_abort(tree);
    }

    tree->mode = lxb_html_tree_insertion_mode_in_cell;

    tree->status = lxb_html_tree_active_formatting_push_marker(tree);
    if (tree->status != LXB_STATUS_OK) {
        return lxb_html_tree_process_abort(tree);
    }

    return true;
}

static bool
lxb_html_tree_insertion_mode_in_row_tr_closed(lxb_html_tree_t *tree,
                                              lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    node = lxb_html_tree_element_in_scope(tree, LXB_TAG_TR, LXB_NS_HTML,
                                          LXB_HTML_TAG_CATEGORY_SCOPE_TABLE);
    if (node == NULL) {
        lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNCLTO);

        return true;
    }

    lxb_html_tree_clear_stack_back_to_table_row(tree);
    lxb_html_tree_open_elements_pop(tree);

    tree->mode = lxb_html_tree_insertion_mode_in_table_body;

    return true;
}

/*
 * A start tag whose tag name is one of: "caption", "col", "colgroup", "tbody",
 * "tfoot", "thead", "tr"
 * An end tag whose tag name is "table"
 */
static bool
lxb_html_tree_insertion_mode_in_row_ct_open_closed(lxb_html_tree_t *tree,
                                                   lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    node = lxb_html_tree_element_in_scope(tree, LXB_TAG_TR, LXB_NS_HTML,
                                          LXB_HTML_TAG_CATEGORY_SCOPE_TABLE);
    if (node == NULL) {
        lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNTO);

        return true;
    }

    lxb_html_tree_clear_stack_back_to_table_row(tree);
    lxb_html_tree_open_elements_pop(tree);

    tree->mode = lxb_html_tree_insertion_mode_in_table_body;

    return false;
}

/*
 * "tbody", "tfoot", "thead"
 */
static bool
lxb_html_tree_insertion_mode_in_row_tbtfth_closed(lxb_html_tree_t *tree,
                                                  lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    node = lxb_html_tree_element_in_scope(tree, token->tag_id, LXB_NS_HTML,
                                          LXB_HTML_TAG_CATEGORY_SCOPE_TABLE);
    if (node == NULL) {
        lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNCLTO);

        return true;
    }

    node = lxb_html_tree_element_in_scope(tree, LXB_TAG_TR, LXB_NS_HTML,
                                          LXB_HTML_TAG_CATEGORY_SCOPE_TABLE);
    if (node == NULL) {
        return true;
    }

    lxb_html_tree_clear_stack_back_to_table_row(tree);
    lxb_html_tree_open_elements_pop(tree);

    tree->mode = lxb_html_tree_insertion_mode_in_table_body;

    return false;
}

/*
 * "body", "caption", "col", "colgroup", "html", "td", "th"
 */
static bool
lxb_html_tree_insertion_mode_in_row_bcht_closed(lxb_html_tree_t *tree,
                                                lxb_html_token_t *token)
{
    lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNCLTO);

    return true;
}

static bool
lxb_html_tree_insertion_mode_in_row_anything_else(lxb_html_tree_t *tree,
                                                  lxb_html_token_t *token)
{
    return lxb_html_tree_insertion_mode_in_table(tree, token);
}

static bool
lxb_html_tree_insertion_mode_in_row_anything_else_closed(lxb_html_tree_t *tree,
                                                         lxb_html_token_t *token)
{
    return lxb_html_tree_insertion_mode_in_row_anything_else(tree, token);
}

static void
lxb_html_tree_clear_stack_back_to_table_row(lxb_html_tree_t *tree)
{
    lxb_dom_node_t *current = lxb_html_tree_current_node(tree);

    while ((current->tag_id != LXB_TAG_TR
            && current->tag_id != LXB_TAG_TEMPLATE
            && current->tag_id != LXB_TAG_HTML)
           || current->ns != LXB_NS_HTML)
    {
        lxb_html_tree_open_elements_pop(tree);
        current = lxb_html_tree_current_node(tree);
    }
}
