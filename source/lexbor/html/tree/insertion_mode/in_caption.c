/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/html/tree/insertion_mode.h"
#include "lexbor/html/tree/open_elements.h"
#include "lexbor/html/tree/active_formatting.h"


static bool
lxb_html_tree_insertion_mode_in_caption_caption_closed(lxb_html_tree_t *tree,
                                                       lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_caption_ct_open_closed(lxb_html_tree_t *tree,
                                                       lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_caption_bcht_closed(lxb_html_tree_t *tree,
                                                    lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_caption_anything_else(lxb_html_tree_t *tree,
                                                      lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_caption_anything_else_closed(lxb_html_tree_t *tree,
                                                             lxb_html_token_t *token);


#include "lexbor/html/tree/insertion_mode/in_caption_res.h"


bool
lxb_html_tree_insertion_mode_in_caption(lxb_html_tree_t *tree,
                                        lxb_html_token_t *token)
{
    if (token->tag_id >= LXB_TAG__LAST_ENTRY) {
        if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
            return lxb_html_tree_insertion_mode_in_caption_anything_else_closed(tree, token);
        }

        return lxb_html_tree_insertion_mode_in_caption_anything_else(tree, token);
    }

    if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
        return lxb_html_tree_insertion_mode_in_caption_closed_res[token->tag_id](tree, token);
    }

    return lxb_html_tree_insertion_mode_in_caption_res[token->tag_id](tree, token);
}

static bool
lxb_html_tree_insertion_mode_in_caption_caption_closed(lxb_html_tree_t *tree,
                                                       lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    node = lxb_html_tree_element_in_scope(tree, LXB_TAG_CAPTION, LXB_NS_HTML,
                                          LXB_TAG_CATEGORY_SCOPE_TABLE);
    if (node == NULL) {
        lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_MIELINSC);

        return true;
    }

    lxb_html_tree_generate_implied_end_tags(tree, LXB_TAG__UNDEF,
                                            LXB_NS__UNDEF);

    node = lxb_html_tree_current_node(tree);

    if (lxb_html_tree_node_is(node, LXB_TAG_CAPTION) == false) {
        lxb_html_tree_parse_error(tree, token,
                                  LXB_HTML_RULES_ERROR_UNELINOPELST);
    }

    lxb_html_tree_open_elements_pop_until_tag_id(tree, LXB_TAG_CAPTION,
                                                 LXB_NS_HTML, true);

    lxb_html_tree_active_formatting_up_to_last_marker(tree);

    tree->mode = lxb_html_tree_insertion_mode_in_table;

    return true;
}

/*
 * A start tag whose tag name is one of: "caption", "col", "colgroup", "tbody",
 * "td", "tfoot", "th", "thead", "tr"
 * An end tag whose tag name is "table"
 */
static bool
lxb_html_tree_insertion_mode_in_caption_ct_open_closed(lxb_html_tree_t *tree,
                                                       lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    node = lxb_html_tree_element_in_scope(tree, LXB_TAG_CAPTION, LXB_NS_HTML,
                                          LXB_TAG_CATEGORY_SCOPE_TABLE);
    if (node == NULL) {
        lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_MIELINSC);

        return true;
    }

    lxb_html_tree_generate_implied_end_tags(tree, LXB_TAG__UNDEF,
                                            LXB_NS__UNDEF);

    node = lxb_html_tree_current_node(tree);

    if (lxb_html_tree_node_is(node, LXB_TAG_CAPTION) == false) {
        lxb_html_tree_parse_error(tree, token,
                                  LXB_HTML_RULES_ERROR_UNELINOPELST);
    }

    lxb_html_tree_open_elements_pop_until_tag_id(tree, LXB_TAG_CAPTION,
                                                 LXB_NS_HTML, true);

    lxb_html_tree_active_formatting_up_to_last_marker(tree);

    tree->mode = lxb_html_tree_insertion_mode_in_table;

    return false;
}

/*
 * "body", "col", "colgroup", "html", "tbody", "td", "tfoot", "th", "thead",
 * "tr"
 */
static bool
lxb_html_tree_insertion_mode_in_caption_bcht_closed(lxb_html_tree_t *tree,
                                                    lxb_html_token_t *token)
{
    lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNCLTO);

    return true;
}

static bool
lxb_html_tree_insertion_mode_in_caption_anything_else(lxb_html_tree_t *tree,
                                                      lxb_html_token_t *token)
{
    return lxb_html_tree_insertion_mode_in_body(tree, token);
}

static bool
lxb_html_tree_insertion_mode_in_caption_anything_else_closed(lxb_html_tree_t *tree,
                                                             lxb_html_token_t *token)
{
    return lxb_html_tree_insertion_mode_in_caption_anything_else(tree, token);
}
