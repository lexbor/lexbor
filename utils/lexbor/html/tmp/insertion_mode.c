/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tree/insertion_mode.h"


static bool
lxb_html_tree_insertion_mode_%%NAME%%_anything_else(lxb_html_tree_t *tree,
                                                         lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_%%NAME%%_anything_else_closed(lxb_html_tree_t *tree,
                                                                lxb_html_token_t *token);


#include "lexbor/html/tree/insertion_mode/%%NAME%%_res.h"


bool
lxb_html_tree_insertion_mode_%%NAME%%(lxb_html_tree_t *tree,
                                           lxb_html_token_t *token)
{
    if (token->tag_id >= LXB_HTML_TAG__LAST_ENTRY) {
        if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
            return lxb_html_tree_insertion_mode_%%NAME%%_anything_else_closed(tree, token);
        }

        return lxb_html_tree_insertion_mode_%%NAME%%_anything_else(tree, token);
    }

    if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
        return lxb_html_tree_insertion_mode_%%NAME%%_closed_res[token->tag_id](tree, token);
    }

    return lxb_html_tree_insertion_mode_%%NAME%%_res[token->tag_id](tree, token);
}

static bool
lxb_html_tree_insertion_mode_%%NAME%%_anything_else(lxb_html_tree_t *tree,
                                                      lxb_html_token_t *token)
{
    return true;
}

static bool
lxb_html_tree_insertion_mode_%%NAME%%_anything_else_closed(lxb_html_tree_t *tree,
                                                             lxb_html_token_t *token)
{
    return true;
}
