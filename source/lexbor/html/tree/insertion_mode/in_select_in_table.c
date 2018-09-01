/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/html/tree/insertion_mode.h"
#include "lexbor/html/tree/open_elements.h"


static bool
lxb_html_tree_insertion_mode_in_select_in_table_ct(lxb_html_tree_t *tree,
                                                   lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_select_in_table_ct_closed(lxb_html_tree_t *tree,
                                                          lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_select_in_table_anything_else(lxb_html_tree_t *tree,
                                                              lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_select_in_table_anything_else_closed(lxb_html_tree_t *tree,
                                                                     lxb_html_token_t *token);


#include "lexbor/html/tree/insertion_mode/in_select_in_table_res.h"


bool
lxb_html_tree_insertion_mode_in_select_in_table(lxb_html_tree_t *tree,
                                                lxb_html_token_t *token)
{
    if (token->tag_id >= LXB_TAG__LAST_ENTRY) {
        if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
            return lxb_html_tree_insertion_mode_in_select_in_table_anything_else_closed(tree,
                                                                                        token);
        }

        return lxb_html_tree_insertion_mode_in_select_in_table_anything_else(tree,
                                                                             token);
    }

    if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
        return lxb_html_tree_insertion_mode_in_select_in_table_closed_res[token->tag_id](tree,
                                                                                         token);
    }

    return lxb_html_tree_insertion_mode_in_select_in_table_res[token->tag_id](tree,
                                                                              token);
}

/*
 * "caption", "table", "tbody", "tfoot", "thead", "tr", "td", "th"
 */
static bool
lxb_html_tree_insertion_mode_in_select_in_table_ct(lxb_html_tree_t *tree,
                                                   lxb_html_token_t *token)
{
    lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNTO);

    lxb_html_tree_open_elements_pop_until_tag_id(tree, LXB_TAG_SELECT,
                                                 LXB_NS_HTML, true);

    lxb_html_tree_reset_insertion_mode_appropriately(tree);

    return false;
}

/*
 * "caption", "table", "tbody", "tfoot", "thead", "tr", "td", "th"
 */
static bool
lxb_html_tree_insertion_mode_in_select_in_table_ct_closed(lxb_html_tree_t *tree,
                                                          lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNCLTO);

    node = lxb_html_tree_element_in_scope(tree, token->tag_id, LXB_NS_HTML,
                                          LXB_HTML_TAG_CATEGORY_SCOPE_TABLE);
    if (node == NULL) {
        return true;
    }

    lxb_html_tree_open_elements_pop_until_tag_id(tree, LXB_TAG_SELECT,
                                                 LXB_NS_HTML, true);

    lxb_html_tree_reset_insertion_mode_appropriately(tree);

    return false;
}

static bool
lxb_html_tree_insertion_mode_in_select_in_table_anything_else(lxb_html_tree_t *tree,
                                                              lxb_html_token_t *token)
{
    return lxb_html_tree_insertion_mode_in_select(tree, token);
}

static bool
lxb_html_tree_insertion_mode_in_select_in_table_anything_else_closed(lxb_html_tree_t *tree,
                                                                     lxb_html_token_t *token)
{
    return lxb_html_tree_insertion_mode_in_select_in_table_anything_else(tree,
                                                                         token);
}

