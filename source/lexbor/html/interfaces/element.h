/*
 * Copyright (C) 2018-2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_ELEMENT_H
#define LEXBOR_HTML_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/html/interface.h"
#include "lexbor/dom/interfaces/element.h"


struct lxb_html_element {
    lxb_dom_element_t element;
};


LXB_API lxb_html_element_t *
lxb_html_element_interface_create(lxb_html_document_t *document);

LXB_API lxb_html_element_t *
lxb_html_element_interface_destroy(lxb_html_element_t *element);


LXB_API lxb_html_element_t *
lxb_html_element_inner_html_set(lxb_html_element_t *element,
                                const lxb_char_t *html, size_t size);

/*
 * Unknown Element steps.
 */
LXB_API lxb_status_t
lxb_html_element_inserted_unknown_steps(lxb_dom_node_t *inserted_node);

LXB_API lxb_status_t
lxb_html_element_removed_unknown_steps(lxb_dom_node_t *removed_node,
                                       lxb_dom_node_t *old_parent);
LXB_API lxb_status_t
lxb_html_element_moved_unknown_steps(lxb_dom_node_t *moved_node,
                                     lxb_dom_node_t *old_parent);
LXB_API lxb_status_t
lxb_html_element_destroy_unknown_steps(lxb_dom_node_t *node);

LXB_API lxb_status_t
lxb_html_element_children_changed_unknown_steps(lxb_dom_node_t *parent);

LXB_API lxb_status_t
lxb_html_element_connected_unknown_steps(lxb_dom_node_t *connected_node);

LXB_API lxb_status_t
lxb_html_element_attr_change(lxb_dom_element_t *element,
                             lxb_dom_attr_id_t local_name,
                             const lxb_char_t *old_value, size_t old_len,
                             const lxb_char_t *value, size_t value_len,
                             lxb_ns_id_t ns);

LXB_API lxb_status_t
lxb_html_element_attr_append(lxb_dom_element_t *element,
                             lxb_dom_attr_id_t local_name,
                             const lxb_char_t *old_value, size_t old_len,
                             const lxb_char_t *value, size_t value_len,
                             lxb_ns_id_t ns);

LXB_API lxb_status_t
lxb_html_element_attr_remove(lxb_dom_element_t *element,
                             lxb_dom_attr_id_t local_name,
                             const lxb_char_t *old_value, size_t old_len,
                             const lxb_char_t *value, size_t value_len,
                             lxb_ns_id_t ns);

LXB_API lxb_status_t
lxb_html_element_attr_replace(lxb_dom_element_t *element,
                              lxb_dom_attr_id_t local_name,
                              const lxb_char_t *old_value, size_t old_len,
                              const lxb_char_t *value, size_t value_len,
                              lxb_ns_id_t ns);


/*
 * Inline functions
 */
lxb_inline lxb_tag_id_t
lxb_html_element_tag_id(lxb_html_element_t *element)
{
    return lxb_dom_interface_node(element)->local_name;
}

lxb_inline lxb_ns_id_t
lxb_html_element_ns_id(lxb_html_element_t *element)
{
    return lxb_dom_interface_node(element)->ns;
}

lxb_inline void
lxb_html_element_insert_before(lxb_html_element_t *dst, lxb_html_element_t *src)
{
    lxb_dom_node_insert_before(lxb_dom_interface_node(dst),
                               lxb_dom_interface_node(src));
}

lxb_inline void
lxb_html_element_insert_after(lxb_html_element_t *dst, lxb_html_element_t *src)
{
    lxb_dom_node_insert_after(lxb_dom_interface_node(dst),
                              lxb_dom_interface_node(src));
}

lxb_inline void
lxb_html_element_insert_child(lxb_html_element_t *dst, lxb_html_element_t *src)
{
    lxb_dom_node_insert_child(lxb_dom_interface_node(dst),
                              lxb_dom_interface_node(src));
}

lxb_inline void
lxb_html_element_remove(lxb_html_element_t *element)
{
    lxb_dom_node_remove(lxb_dom_interface_node(element));
}

lxb_inline lxb_html_document_t *
lxb_html_element_document(lxb_html_element_t *element)
{
    return lxb_html_interface_document(lxb_dom_interface_node(element)->owner_document);
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_ELEMENT_H */
