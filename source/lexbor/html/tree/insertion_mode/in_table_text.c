/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/html/tree/insertion_mode.h"
#include "lexbor/html/token.h"


bool
lxb_html_tree_insertion_mode_in_table_text(lxb_html_tree_t *tree,
                                           lxb_html_token_t *token)
{
    lxb_status_t status;
    lxb_html_token_t *token_text;
    lexbor_array_obj_t *pt_list = tree->pending_table.text_tokens;

    if (token->tag_id == LXB_HTML_TAG__TEXT) {
        if (token->type & LXB_HTML_TOKEN_TYPE_NULL) {
            lxb_html_tree_parse_error(tree, token,
                                      LXB_HTML_RULES_ERROR_NUCH);
        }

        token_text = lexbor_array_obj_push(pt_list);
        if (token_text == NULL) {
            tree->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

            return lxb_html_tree_process_abort(tree);
        }

        *token_text = *token;

        /*
         * The lxb_html_token_data_skip_ws_begin function
         * can change token->begin and token->in_begin value.
         */
        status = lxb_html_token_data_skip_ws_begin(token_text);
        if (status != LXB_STATUS_OK) {
            return lxb_html_tree_process_abort(tree);
        }

        if (token_text->begin != token_text->end) {
            if (!tree->pending_table.have_non_ws) {
                tree->pending_table.have_non_ws = true;
            }
        }

        /* Set back all token params */
        *token_text = *token;

        return true;
    }

    if (tree->pending_table.have_non_ws) {
        lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_CHINTATE);

        for (size_t i = 0; i < lexbor_array_obj_length(pt_list); i++) {
            token_text = lexbor_array_obj_get(pt_list, i);

            lxb_html_tree_insertion_mode_in_table_anything_else(tree,
                                                                token_text);
            if (tree->status != LXB_STATUS_OK) {
                return lxb_html_tree_process_abort(tree);
            }
        }
    }
    else {
        lexbor_str_t str = {0};
        lxb_html_parser_char_t pc = {0};

        pc.drop_null = true;

        for (size_t i = 0; i < lexbor_array_obj_length(pt_list); i++) {
            token_text = lexbor_array_obj_get(pt_list, i);

            str.data = NULL;

            tree->status = lxb_html_token_parse_data(token_text, &pc, &str,
                                                     tree->document->mem->text);
            if (tree->status != LXB_STATUS_OK) {
                return lxb_html_tree_process_abort(tree);
            }

            if (str.length == 0) {
                lexbor_str_destroy(&str, tree->document->mem->text, false);

                continue;
            }

            tree->status = lxb_html_tree_insert_character_for_data(tree, &str,
                                                                   NULL);
            if (tree->status != LXB_STATUS_OK) {
                return lxb_html_tree_process_abort(tree);
            }
        }
    }

    tree->mode = tree->original_mode;

    return false;
}
