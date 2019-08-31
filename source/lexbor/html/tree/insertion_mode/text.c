/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/tree/insertion_mode.h"
#include "lexbor/html/tree/open_elements.h"


bool
lxb_html_tree_insertion_mode_text(lxb_html_tree_t *tree,
                                  lxb_html_token_t *token)
{
    switch (token->tag_id) {
        case LXB_TAG__TEXT: {
            tree->status = lxb_html_tree_insert_character(tree, token, NULL);
            if (tree->status != LXB_STATUS_OK) {
                return lxb_html_tree_process_abort(tree);
            }

            break;
        }

        case LXB_TAG__END_OF_FILE: {
            lxb_dom_node_t *node;

            lxb_html_tree_parse_error(tree, token,
                                      LXB_HTML_RULES_ERROR_UNENOFFI);

            node = lxb_html_tree_current_node(tree);

            if (lxb_html_tree_node_is(node, LXB_TAG_SCRIPT)) {
                /* TODO: mark the script element as "already started" */
            }

            lxb_html_tree_open_elements_pop(tree);

            tree->mode = tree->original_mode;

            return false;
        }

        /* TODO: need to implement */
        case LXB_TAG_SCRIPT:
            lxb_html_tree_open_elements_pop(tree);

            tree->mode = tree->original_mode;

            break;

        default:
            lxb_html_tree_open_elements_pop(tree);

            tree->mode = tree->original_mode;

            break;
    }

    return true;
}
