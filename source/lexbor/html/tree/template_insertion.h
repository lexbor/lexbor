/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_HTML_TEMPLATE_INSERTION_H
#define LEXBOR_HTML_TEMPLATE_INSERTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/array.h"

#include "lexbor/html/tree.h"


/*
 * Inline functions
 */
lxb_inline lxb_html_tree_insertion_mode_f
lxb_html_tree_template_insertion_current(lxb_html_tree_t *tree)
{
    if (tree->template_insertion_modes->length == 0) {
        return NULL;
    }

    return (lxb_html_tree_insertion_mode_f) tree->template_insertion_modes->list
        [ (tree->template_insertion_modes->length - 1) ];
}

lxb_inline lxb_html_tree_insertion_mode_f
lxb_html_tree_template_insertion_first(lxb_html_tree_t *tree)
{
    return (lxb_html_tree_insertion_mode_f)
        lexbor_array_get(tree->template_insertion_modes, 0);
}

lxb_inline lxb_html_tree_insertion_mode_f
lxb_html_tree_template_insertion_get(lxb_html_tree_t *tree, size_t idx)
{
    return (lxb_html_tree_insertion_mode_f)
        lexbor_array_get(tree->template_insertion_modes, idx);
}

lxb_inline lxb_status_t
lxb_html_tree_template_insertion_push(lxb_html_tree_t *tree,
                                      lxb_html_tree_insertion_mode_f mode)
{
    return lexbor_array_push(tree->template_insertion_modes, (void *) mode);
}

lxb_inline lxb_html_tree_insertion_mode_f
lxb_html_tree_template_insertion_pop(lxb_html_tree_t *tree)
{
    return (lxb_html_tree_insertion_mode_f)
        lexbor_array_pop(tree->template_insertion_modes);
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_TEMPLATE_INSERTION_H */

