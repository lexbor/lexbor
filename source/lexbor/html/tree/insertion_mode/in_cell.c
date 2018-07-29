/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/html/tree/insertion_mode.h"
#include "lexbor/html/tree/open_elements.h"
#include "lexbor/html/tree/active_formatting.h"


static bool
lxb_html_tree_insertion_mode_in_cell_tdth_closed(lxb_html_tree_t *tree,
                                                 lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_cell_ct(lxb_html_tree_t *tree,
                                        lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_cell_bch_closed(lxb_html_tree_t *tree,
                                                lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_cell_t_closed(lxb_html_tree_t *tree,
                                              lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_cell_anything_else(lxb_html_tree_t *tree,
                                                   lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_cell_anything_else_closed(lxb_html_tree_t *tree,
                                                          lxb_html_token_t *token);

static void
lxb_html_tree_close_cell(lxb_html_tree_t *tree, lxb_html_token_t *token);


#include "lexbor/html/tree/insertion_mode/in_cell_res.h"


bool
lxb_html_tree_insertion_mode_in_cell(lxb_html_tree_t *tree,
                                     lxb_html_token_t *token)
{
    if (token->tag_id >= LXB_HTML_TAG__LAST_ENTRY) {
        if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
            return lxb_html_tree_insertion_mode_in_cell_anything_else_closed(tree, token);
        }

        return lxb_html_tree_insertion_mode_in_cell_anything_else(tree, token);
    }

    if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
        return lxb_html_tree_insertion_mode_in_cell_closed_res[token->tag_id](tree, token);
    }

    return lxb_html_tree_insertion_mode_in_cell_res[token->tag_id](tree, token);
}

/*
 * "td", "th"
 */
static bool
lxb_html_tree_insertion_mode_in_cell_tdth_closed(lxb_html_tree_t *tree,
                                                 lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    node = lxb_html_tree_element_in_scope(tree, token->tag_id,
                                          LXB_HTML_NS_HTML,
                                          LXB_HTML_TAG_CATEGORY_SCOPE_TABLE);
    if (node == NULL) {
        lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNCLTO);

        return true;
    }

    lxb_html_tree_generate_implied_end_tags(tree, LXB_HTML_TAG__UNDEF,
                                            LXB_HTML_NS__UNDEF);

    node = lxb_html_tree_current_node(tree);

    if (lxb_html_tree_node_is(node, token->tag_id) == false) {
        lxb_html_tree_parse_error(tree, token,
                                  LXB_HTML_RULES_ERROR_MIELINOPELST);
    }

    lxb_html_tree_open_elements_pop_until_tag_id(tree, token->tag_id,
                                                 LXB_HTML_NS_HTML, true);

    lxb_html_tree_active_formatting_up_to_last_marker(tree);

    tree->mode = lxb_html_tree_insertion_mode_in_row;

    return true;
}

/*
 * "caption", "col", "colgroup", "tbody", "td", "tfoot", "th", "thead", "tr"
 */
static bool
lxb_html_tree_insertion_mode_in_cell_ct(lxb_html_tree_t *tree,
                                        lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    node = lxb_html_tree_element_in_scope_td_th(tree);
    if (node == NULL) {
        lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_MIELINSC);

        return true;
    }

    lxb_html_tree_close_cell(tree, token);

    return false;
}

/*
 * "body", "caption", "col", "colgroup", "html"
 */
static bool
lxb_html_tree_insertion_mode_in_cell_bch_closed(lxb_html_tree_t *tree,
                                                lxb_html_token_t *token)
{
    lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNCLTO);

    return true;
}

/*
 * "table", "tbody", "tfoot", "thead", "tr"
 */
static bool
lxb_html_tree_insertion_mode_in_cell_t_closed(lxb_html_tree_t *tree,
                                              lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    node = lxb_html_tree_element_in_scope(tree, token->tag_id,
                                          LXB_HTML_NS_HTML,
                                          LXB_HTML_TAG_CATEGORY_SCOPE_TABLE);
    if (node == NULL) {
        lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNCLTO);

        return true;
    }

    lxb_html_tree_close_cell(tree, token);

    return false;
}

static bool
lxb_html_tree_insertion_mode_in_cell_anything_else(lxb_html_tree_t *tree,
                                                   lxb_html_token_t *token)
{
    return lxb_html_tree_insertion_mode_in_body(tree, token);
}

static bool
lxb_html_tree_insertion_mode_in_cell_anything_else_closed(lxb_html_tree_t *tree,
                                                          lxb_html_token_t *token)
{
    return lxb_html_tree_insertion_mode_in_cell_anything_else(tree, token);
}

static void
lxb_html_tree_close_cell(lxb_html_tree_t *tree, lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    lxb_html_tree_generate_implied_end_tags(tree, LXB_HTML_TAG__UNDEF,
                                            LXB_HTML_NS__UNDEF);

    node = lxb_html_tree_current_node(tree);

    if (lxb_html_tree_node_is(node, LXB_HTML_TAG_TD) == false
        && lxb_html_tree_node_is(node, LXB_HTML_TAG_TH) == false)
    {
        lxb_html_tree_parse_error(tree, token,
                                  LXB_HTML_RULES_ERROR_MIELINOPELST);
    }

    lxb_html_tree_open_elements_pop_until_td_th(tree);
    lxb_html_tree_active_formatting_up_to_last_marker(tree);

    tree->mode = lxb_html_tree_insertion_mode_in_row;
}
