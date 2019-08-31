/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tree/insertion_mode.h"
#include "lexbor/html/tree/open_elements.h"
#include "lexbor/html/tree/active_formatting.h"
#include "lexbor/html/tree/template_insertion.h"


static bool
lxb_html_tree_insertion_mode_in_template_text_comment_doctype(lxb_html_tree_t *tree,
                                                              lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_template_blmnst_open_closed(lxb_html_tree_t *tree,
                                                            lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_template_ct(lxb_html_tree_t *tree,
                                            lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_template_col(lxb_html_tree_t *tree,
                                             lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_template_tr(lxb_html_tree_t *tree,
                                            lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_template_tdth(lxb_html_tree_t *tree,
                                              lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_template_end_of_file(lxb_html_tree_t *tree,
                                                     lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_template_anything_else(lxb_html_tree_t *tree,
                                                       lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_in_template_anything_else_closed(lxb_html_tree_t *tree,
                                                              lxb_html_token_t *token);


#include "lexbor/html/tree/insertion_mode/in_template_res.h"


bool
lxb_html_tree_insertion_mode_in_template(lxb_html_tree_t *tree,
                                         lxb_html_token_t *token)
{
    if (token->tag_id >= LXB_TAG__LAST_ENTRY) {
        if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
            return lxb_html_tree_insertion_mode_in_template_anything_else_closed(tree,
                                                                                 token);
        }

        return lxb_html_tree_insertion_mode_in_template_anything_else(tree,
                                                                      token);
    }

    if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
        return lxb_html_tree_insertion_mode_in_template_closed_res[token->tag_id](tree,
                                                                                  token);
    }

    return lxb_html_tree_insertion_mode_in_template_res[token->tag_id](tree,
                                                                       token);
}

/*
 * A character token
 * A comment token
 * A DOCTYPE token
 */
static bool
lxb_html_tree_insertion_mode_in_template_text_comment_doctype(lxb_html_tree_t *tree,
                                                              lxb_html_token_t *token)
{
    return lxb_html_tree_insertion_mode_in_body(tree, token);
}

/*
 * A start tag whose tag name is one of: "base", "basefont", "bgsound", "link",
 * "meta", "noframes", "script", "style", "template", "title"
 * An end tag whose tag name is "template"
 */
static bool
lxb_html_tree_insertion_mode_in_template_blmnst_open_closed(lxb_html_tree_t *tree,
                                                            lxb_html_token_t *token)
{
    return lxb_html_tree_insertion_mode_in_head(tree, token);
}

/*
 * "caption", "colgroup", "tbody", "tfoot", "thead"
 */
static bool
lxb_html_tree_insertion_mode_in_template_ct(lxb_html_tree_t *tree,
                                            lxb_html_token_t *token)
{
    lxb_html_tree_template_insertion_pop(tree);

    tree->status = lxb_html_tree_template_insertion_push(tree,
                                                         lxb_html_tree_insertion_mode_in_table);
    if (tree->status != LXB_STATUS_OK) {
        return lxb_html_tree_process_abort(tree);
    }

    tree->mode = lxb_html_tree_insertion_mode_in_table;

    return false;
}

static bool
lxb_html_tree_insertion_mode_in_template_col(lxb_html_tree_t *tree,
                                             lxb_html_token_t *token)
{
    lxb_html_tree_template_insertion_pop(tree);

    tree->status = lxb_html_tree_template_insertion_push(tree,
                                                         lxb_html_tree_insertion_mode_in_column_group);
    if (tree->status != LXB_STATUS_OK) {
        return lxb_html_tree_process_abort(tree);
    }

    tree->mode = lxb_html_tree_insertion_mode_in_column_group;

    return false;
}

static bool
lxb_html_tree_insertion_mode_in_template_tr(lxb_html_tree_t *tree,
                                            lxb_html_token_t *token)
{
    lxb_html_tree_template_insertion_pop(tree);

    tree->status = lxb_html_tree_template_insertion_push(tree,
                                                         lxb_html_tree_insertion_mode_in_table_body);
    if (tree->status != LXB_STATUS_OK) {
        return lxb_html_tree_process_abort(tree);
    }

    tree->mode = lxb_html_tree_insertion_mode_in_table_body;

    return false;
}

/*
 * "td", "th"
 */
static bool
lxb_html_tree_insertion_mode_in_template_tdth(lxb_html_tree_t *tree,
                                              lxb_html_token_t *token)
{
    lxb_html_tree_template_insertion_pop(tree);

    tree->status = lxb_html_tree_template_insertion_push(tree,
                                                         lxb_html_tree_insertion_mode_in_row);
    if (tree->status != LXB_STATUS_OK) {
        return lxb_html_tree_process_abort(tree);
    }

    tree->mode = lxb_html_tree_insertion_mode_in_row;

    return false;
}

static bool
lxb_html_tree_insertion_mode_in_template_end_of_file(lxb_html_tree_t *tree,
                                                     lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    node = lxb_html_tree_open_elements_find(tree, LXB_TAG_TEMPLATE, LXB_NS_HTML,
                                            NULL);
    if (node == NULL) {
        tree->status =  lxb_html_tree_stop_parsing(tree);
        if (tree->status != LXB_STATUS_OK) {
            return lxb_html_tree_process_abort(tree);
        }

        return true;
    }

    lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNENOFFI);

    lxb_html_tree_open_elements_pop_until_tag_id(tree, LXB_TAG_TEMPLATE,
                                                 LXB_NS_HTML, true);

    lxb_html_tree_active_formatting_up_to_last_marker(tree);
    lxb_html_tree_template_insertion_pop(tree);
    lxb_html_tree_reset_insertion_mode_appropriately(tree);

    return false;
}

static bool
lxb_html_tree_insertion_mode_in_template_anything_else(lxb_html_tree_t *tree,
                                                       lxb_html_token_t *token)
{
    lxb_html_tree_template_insertion_pop(tree);

    tree->status = lxb_html_tree_template_insertion_push(tree,
                                                         lxb_html_tree_insertion_mode_in_body);
    if (tree->status != LXB_STATUS_OK) {
        return lxb_html_tree_process_abort(tree);
    }

    tree->mode = lxb_html_tree_insertion_mode_in_body;

    return false;
}

static bool
lxb_html_tree_insertion_mode_in_template_anything_else_closed(lxb_html_tree_t *tree,
                                                              lxb_html_token_t *token)
{
    lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNCLTO);

    return true;
}
