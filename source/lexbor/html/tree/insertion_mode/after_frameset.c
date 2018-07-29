/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/html/tree/insertion_mode.h"


bool
lxb_html_tree_insertion_mode_after_frameset(lxb_html_tree_t *tree,
                                            lxb_html_token_t *token)
{
    switch (token->tag_id) {
        case LXB_HTML_TAG__EM_COMMENT: {
            lxb_dom_comment_t *comment;

            comment = lxb_html_tree_insert_comment(tree, token, NULL);
            if (comment == NULL) {
                return lxb_html_tree_process_abort(tree);
            }

            break;
        }

        case LXB_HTML_TAG__EM_DOCTYPE:
            lxb_html_tree_parse_error(tree, token,
                                      LXB_HTML_RULES_ERROR_DOTOAFFRMO);
            break;

        case LXB_HTML_TAG_HTML:
            if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
                tree->mode = lxb_html_tree_insertion_mode_after_after_frameset;

                return true;
            }

            return lxb_html_tree_insertion_mode_in_body(tree, token);

        case LXB_HTML_TAG_NOFRAMES:
            return lxb_html_tree_insertion_mode_in_head(tree, token);

        case LXB_HTML_TAG__END_OF_FILE: {
            tree->status = lxb_html_tree_stop_parsing(tree);
            if (tree->status != LXB_STATUS_OK) {
                return lxb_html_tree_process_abort(tree);
            }

            break;
        }

        case LXB_HTML_TAG__TEXT: {
            lxb_html_token_t ws_token = {0};

            tree->status = lxb_html_token_data_split_ws_begin(token, &ws_token);
            if (tree->status != LXB_STATUS_OK) {
                return lxb_html_tree_process_abort(tree);
            }

            if (ws_token.begin != ws_token.end) {
                tree->status = lxb_html_tree_insert_character(tree, &ws_token,
                                                              NULL);
                if (tree->status != LXB_STATUS_OK) {
                    return lxb_html_tree_process_abort(tree);
                }
            }

            if (token->begin == token->end) {
                return true;
            }
        }
        /* fall through */

        default:
            lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNTO);

            break;
    }

    return true;
}
