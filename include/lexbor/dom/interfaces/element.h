/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_DOM_ELEMENT_H
#define LEXBOR_DOM_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/core/str.h>

#include <lexbor/dom/interfaces/document.h>
#include <lexbor/dom/interfaces/node.h>
#include <lexbor/dom/qualified_name.h>

#include <lexbor/tag/tag.h>


typedef enum {
    LXB_DOM_ELEMENT_CUSTOM_STATE_UNDEFINED      = 0x00,
    LXB_DOM_ELEMENT_CUSTOM_STATE_FAILED         = 0x01,
    LXB_DOM_ELEMENT_CUSTOM_STATE_UNCUSTOMIZED   = 0x02,
    LXB_DOM_ELEMENT_CUSTOM_STATE_CUSTOM         = 0x03
}
lxb_dom_element_custom_state_t;

struct lxb_dom_element {
    lxb_dom_node_t                 node;

    lexbor_str_t                   *is_value;
    lxb_dom_qualified_name_t       *qualified_name;

    lxb_dom_attr_t                 *first_attr;
    lxb_dom_attr_t                 *last_attr;

    lxb_dom_element_custom_state_t custom_state;
};


LXB_API lxb_dom_element_t *
lxb_dom_element_interface_create(lxb_dom_document_t *document);

LXB_API lxb_dom_element_t *
lxb_dom_element_interface_destroy(lxb_dom_element_t *element);

LXB_API lxb_dom_element_t *
lxb_dom_element_create(lxb_dom_document_t *document,
                       const lxb_char_t *local_name, size_t lname_len,
                       const lxb_char_t *ns_name, size_t ns_len,
                       const lxb_char_t *prefix, size_t prefix_len,
                       const lxb_char_t *is, size_t is_len,
                       bool sync_custom, bool lowercase);

lxb_dom_attr_t *
lxb_dom_element_set_attribute(lxb_dom_element_t *element,
                              const lxb_char_t *qualified_name, size_t qn_len,
                              const lxb_char_t *value, size_t value_len,
                              lxb_dom_exception_code_t *exception_code);

LXB_API bool
lxb_dom_element_compare(lxb_dom_element_t *first, lxb_dom_element_t *second);

LXB_API lxb_dom_attr_t *
lxb_dom_element_attr_is_exist(lxb_dom_element_t *element,
                              const lxb_char_t *qualified_name, size_t len);

LXB_API void
lxb_dom_element_attr_append(lxb_dom_element_t *element, lxb_dom_attr_t *attr);

LXB_API lxb_status_t
lxb_dom_element_qualified_name_set(lxb_dom_element_t *element,
                                   const lxb_char_t *prefix, unsigned int prefix_len,
                                   const lxb_char_t *lname, unsigned int lname_len);

lxb_status_t
lxb_dom_element_is_set(lxb_dom_element_t *element,
                       const lxb_char_t *is, size_t is_len);


/*
 * Inline functions
 */
lxb_inline const lxb_char_t *
lxb_dom_element_qualified_name(lxb_dom_element_t *element, size_t *len)
{
    if (element->qualified_name != NULL) {
        return lxb_dom_qualified_name(element->qualified_name, len);
    }

    return lxb_tag_name_by_id(lxb_dom_interface_node(element)->owner_document->tags,
                              lxb_dom_interface_node(element)->tag_id, len);
}

lxb_inline const lxb_char_t *
lxb_dom_element_qualified_name_upper(lxb_dom_element_t *element, size_t *len)
{
    if (element->qualified_name != NULL) {
        return lxb_dom_qualified_name_upper(lxb_dom_interface_node(element)->owner_document,
                                            element->qualified_name, len);
    }

    return lxb_tag_name_upper_by_id(lxb_dom_interface_node(element)->owner_document->tags,
                                    lxb_dom_interface_node(element)->tag_id, len);
}

lxb_inline const lxb_char_t *
lxb_dom_element_local_name(lxb_dom_element_t *element, size_t *len)
{
    if (element->qualified_name != NULL) {
        return lxb_dom_qualified_name_local_name(element->qualified_name, len);
    }

    return lxb_tag_name_by_id(lxb_dom_interface_node(element)->owner_document->tags,
                              lxb_dom_interface_node(element)->tag_id, len);
}

lxb_inline const lxb_char_t *
lxb_dom_element_prefix(lxb_dom_element_t *element, size_t *len)
{
    if (element->qualified_name == NULL) {
        if (len != NULL) {
            *len = 0;
        }

        return NULL;
    }

    return lxb_dom_qualified_name_prefix(element->qualified_name, len);
}

lxb_inline const lxb_char_t *
lxb_dom_element_tag_name(lxb_dom_element_t *element, size_t *len)
{
    lxb_dom_document_t *doc = lxb_dom_interface_node(element)->owner_document;

    if (element->node.ns != LXB_NS_HTML
        || doc->type != LXB_DOM_DOCUMENT_DTYPE_HTML)
    {
        lxb_dom_element_qualified_name(element, len);
    }

    return lxb_dom_element_qualified_name_upper(element, len);
}

lxb_inline bool
lxb_dom_element_is_custom(lxb_dom_element_t *element)
{
    return element->custom_state & LXB_DOM_ELEMENT_CUSTOM_STATE_CUSTOM;
}

lxb_inline bool
lxb_dom_element_custom_is_defined(lxb_dom_element_t *element)
{
    return element->custom_state & LXB_DOM_ELEMENT_CUSTOM_STATE_CUSTOM
        || element->custom_state & LXB_DOM_ELEMENT_CUSTOM_STATE_UNCUSTOMIZED;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_DOM_ELEMENT_H */
