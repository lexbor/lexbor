/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#define LEXBOR_TOKENIZER_CHARS_MAP
#include "lexbor/core/str_res.h"

#include "lexbor/html/tree/insertion_mode.h"
#include "lexbor/html/tree/open_elements.h"
#include "lexbor/html/interfaces/element.h"


static bool
lxb_html_tree_insertion_mode_foreign_content_text(lxb_html_tree_t *tree,
                                                  lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_foreign_content_comment(lxb_html_tree_t *tree,
                                                     lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_foreign_content_doctype(lxb_html_tree_t *tree,
                                                     lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_foreign_content_all(lxb_html_tree_t *tree,
                                                 lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_foreign_content_script_closed(lxb_html_tree_t *tree,
                                                           lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_foreign_content_text(lxb_html_tree_t *tree,
                                                  lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_foreign_content_anything_else(lxb_html_tree_t *tree,
                                                           lxb_html_token_t *token);

static bool
lxb_html_tree_insertion_mode_foreign_content_anything_else_closed(lxb_html_tree_t *tree,
                                                                  lxb_html_token_t *token);


#include "lexbor/html/tree/insertion_mode/foreign_content_res.h"


bool
lxb_html_tree_insertion_mode_foreign_content(lxb_html_tree_t *tree,
                                             lxb_html_token_t *token)
{
    if (token->tag_id >= LXB_TAG__LAST_ENTRY) {
        if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
            return lxb_html_tree_insertion_mode_foreign_content_anything_else_closed(tree,
                                                                                     token);
        }

        return lxb_html_tree_insertion_mode_foreign_content_anything_else(tree,
                                                                          token);
    }

    if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
        return lxb_html_tree_insertion_mode_foreign_content_closed_res[token->tag_id](tree,
                                                                                      token);
    }

    return lxb_html_tree_insertion_mode_foreign_content_res[token->tag_id](tree,
                                                                           token);
}

static bool
lxb_html_tree_insertion_mode_foreign_content_text(lxb_html_tree_t *tree,
                                                  lxb_html_token_t *token)
{
    if (token->type & LXB_HTML_TOKEN_TYPE_NULL) {
        lxb_html_tree_parse_error(tree, token,
                                  LXB_HTML_RULES_ERROR_NUCH);
    }

    lexbor_str_t str = {0};
    lxb_html_parser_char_t pc = {0};

    pc.mraw = tree->document->mem->text;
    pc.state = lxb_html_parser_char_data;
    pc.replace_null = true;

    tree->status = lxb_html_parser_char_process(&pc, &str, token->in_begin,
                                                token->begin, token->end);
    if (tree->status != LXB_STATUS_OK) {
        return lxb_html_tree_process_abort(tree);
    }

    /* Can be zero only if all NULL are gone */
    if (str.length == 0) {
        lexbor_str_destroy(&str, tree->document->mem->text, false);

        return true;
    }

    tree->status = lxb_html_tree_insert_character_for_data(tree, &str, NULL);
    if (tree->status != LXB_STATUS_OK) {
        return lxb_html_tree_process_abort(tree);
    }

    if (tree->frameset_ok) {
        const lxb_char_t *pos = str.data;
        const lxb_char_t *end = str.data + str.length;
        const lxb_char_t *rep = (const lxb_char_t *) "\xEF\xBF\xBD";

        while (pos != end)
        {
            /* Need skip U+FFFD REPLACEMENT CHARACTER */
            if (*pos == *rep) {
                if ((end - pos) < 3) {
                    tree->frameset_ok = false;

                    break;
                }

                if (memcmp(pos, rep, sizeof(lxb_char_t) * 3) != 0) {
                    tree->frameset_ok = false;

                    break;
                }

                pos = pos + 3;

                continue;
            }

            if (lexbor_tokenizer_chars_map[*pos]
                != LEXBOR_STR_RES_MAP_CHAR_WHITESPACE)
            {
                tree->frameset_ok = false;

                break;
            }

            pos++;
        }
    }

    return true;
}

static bool
lxb_html_tree_insertion_mode_foreign_content_comment(lxb_html_tree_t *tree,
                                                     lxb_html_token_t *token)
{
    lxb_dom_comment_t *comment;

    comment = lxb_html_tree_insert_comment(tree, token, NULL);
    if (comment == NULL) {
        tree->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

        return lxb_html_tree_process_abort(tree);
    }

    return true;
}

static bool
lxb_html_tree_insertion_mode_foreign_content_doctype(lxb_html_tree_t *tree,
                                                     lxb_html_token_t *token)
{
    lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_DOTOFOCOMO);

    return true;
}

/*
 * "b", "big", "blockquote", "body", "br", "center", "code", "dd", "div", "dl",
 * "dt", "em", "embed", "h1", "h2", "h3", "h4", "h5", "h6", "head", "hr", "i",
 * "img", "li", "listing", "menu", "meta", "nobr", "ol", "p", "pre", "ruby",
 * "s", "small", "span", "strong", "strike", "sub", "sup", "table", "tt", "u",
 * "ul", "var"
 * "font", if the token has any attributes named "color", "face", or "size"
 */
static bool
lxb_html_tree_insertion_mode_foreign_content_all(lxb_html_tree_t *tree,
                                                 lxb_html_token_t *token)
{
    lxb_dom_node_t *node;

    if (token->tag_id == LXB_TAG_FONT) {
        lexbor_str_t str = {0};
        lxb_html_token_attr_t *attr = token->attr_first;

        while (attr != NULL) {
            str.length = 0;

            tree->status = lxb_html_token_attr_make_name(attr, &str,
                                                         tree->document->mem->text);
            if (tree->status != LXB_STATUS_OK) {
                lexbor_str_destroy(&str, tree->document->mem->text, false);

                return lxb_html_tree_process_abort(tree);
            }

            if (str.length == 5
                && lexbor_str_data_cmp((const lxb_char_t *) "color", str.data))
            {
                lexbor_str_destroy(&str, tree->document->mem->text, false);

                goto go_next;
            }

            if (str.length == 4
                && (lexbor_str_data_cmp((const lxb_char_t *) "face", str.data)
                    || lexbor_str_data_cmp((const lxb_char_t *) "size", str.data)))
            {
                lexbor_str_destroy(&str, tree->document->mem->text, false);

                goto go_next;
            }

            attr = attr->next;
        }

        lexbor_str_destroy(&str, tree->document->mem->text, false);

        return lxb_html_tree_insertion_mode_foreign_content_anything_else(tree,
                                                                          token);
    }

go_next:

    lxb_html_tree_parse_error(tree, token, LXB_HTML_RULES_ERROR_UNTO);

    if (tree->fragment != NULL) {
        return lxb_html_tree_insertion_mode_foreign_content_anything_else(tree,
                                                                          token);
    }

    do {
        lxb_html_tree_open_elements_pop(tree);

        node = lxb_html_tree_current_node(tree);
    }
    while (node &&
           !(lxb_html_tree_mathml_text_integration_point(node)
            || lxb_html_tree_html_integration_point(node)
            || node->ns == LXB_NS_HTML));

    return false;
}

/*
 * TODO: Need to process script
 */
static bool
lxb_html_tree_insertion_mode_foreign_content_script_closed(lxb_html_tree_t *tree,
                                                           lxb_html_token_t *token)
{
    lxb_dom_node_t *node = lxb_html_tree_current_node(tree);

    if (node->tag_id != LXB_TAG_SCRIPT || node->ns != LXB_NS_SVG) {
        return lxb_html_tree_insertion_mode_foreign_content_anything_else_closed(tree,
                                                                                 token);
    }

    lxb_html_tree_open_elements_pop(tree);

    return true;
}

static bool
lxb_html_tree_insertion_mode_foreign_content_anything_else(lxb_html_tree_t *tree,
                                                           lxb_html_token_t *token)
{
    lxb_html_element_t *element;
    const lxb_html_tag_fixname_t *fixname_svg;
    lxb_dom_node_t *node = lxb_html_tree_adjusted_current_node(tree);

    if (node->ns == LXB_NS_MATH) {
        tree->before_append_attr = lxb_html_tree_adjust_attributes_mathml;
    }
    else if (node->ns == LXB_NS_SVG) {
        tree->before_append_attr = lxb_html_tree_adjust_attributes_svg;
    }

    element = lxb_html_tree_insert_foreign_element(tree, token, node->ns);
    if (element == NULL) {
        tree->before_append_attr = NULL;
        tree->status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;

        return lxb_html_tree_process_abort(tree);
    }

    if (node->ns == LXB_NS_SVG) {
        fixname_svg = lxb_html_tag_fixname_svg(element->element.node.tag_id);
        if (fixname_svg != NULL) {
            lxb_dom_element_qualified_name_set(&element->element, NULL, 0,
                                               fixname_svg->name,
                                               (size_t) fixname_svg->len);
        }
    }

    tree->before_append_attr = NULL;

    if ((token->type & LXB_HTML_TOKEN_TYPE_CLOSE_SELF) == 0) {
        return true;
    }

    node = lxb_html_tree_current_node(tree);

    if (token->tag_id == LXB_TAG_SCRIPT && node->ns == LXB_NS_SVG) {
        lxb_html_tree_acknowledge_token_self_closing(tree, token);
        return lxb_html_tree_insertion_mode_foreign_content_script_closed(tree, token);
    }
    else {
        lxb_html_tree_open_elements_pop(tree);
        lxb_html_tree_acknowledge_token_self_closing(tree, token);
    }

    return true;
}

static bool
lxb_html_tree_insertion_mode_foreign_content_anything_else_closed(lxb_html_tree_t *tree,
                                                                  lxb_html_token_t *token)
{
    if (tree->open_elements->length == 0) {
        return tree->mode(tree, token);
    }

    lxb_dom_node_t **list = (lxb_dom_node_t **) tree->open_elements->list;

    size_t idx = tree->open_elements->length - 1;

    if (idx > 0 && list[idx]->tag_id != token->tag_id) {
        lxb_html_tree_parse_error(tree, token,
                                  LXB_HTML_RULES_ERROR_UNELINOPELST);
    }

    while (idx != 0) {
        if (list[idx]->tag_id == token->tag_id) {
            lxb_html_tree_open_elements_pop_until_node(tree, list[idx], true);

            return true;
        }

        idx--;

        if (list[idx]->ns == LXB_NS_HTML) {
            break;
        }
    }

    return tree->mode(tree, token);
}
