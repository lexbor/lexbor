/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/style.h"
#include "lexbor/html/interfaces/element.h"
#include "lexbor/html/interfaces/document.h"


lxb_html_element_t *
lxb_html_element_interface_create(lxb_html_document_t *document)
{
    lxb_html_element_t *element;

    element = lexbor_mraw_calloc(document->dom_document.mraw,
                                 sizeof(lxb_html_element_t));
    if (element == NULL) {
        return NULL;
    }

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = lxb_html_document_original_ref(document);
    node->type = LXB_DOM_NODE_TYPE_ELEMENT;

    return element;
}

lxb_html_element_t *
lxb_html_element_interface_destroy(lxb_html_element_t *element)
{
    return lexbor_mraw_free(
                lxb_dom_interface_node(element)->owner_document->mraw, element);
}

lxb_html_element_t *
lxb_html_element_inner_html_set(lxb_html_element_t *element,
                                const lxb_char_t *html, size_t size)
{
    lxb_dom_node_t *node, *child;
    lxb_dom_node_t *root = lxb_dom_interface_node(element);
    lxb_html_document_t *doc = lxb_html_interface_document(root->owner_document);

    node = lxb_html_document_parse_fragment(doc, &element->element, html, size);
    if (node == NULL) {
        return NULL;
    }

    while (root->first_child != NULL) {
        lxb_dom_node_destroy_deep(root->first_child);
    }

    while (node->first_child != NULL) {
        child = node->first_child;

        lxb_dom_node_remove(child);
        lxb_dom_node_insert_child(root, child);
    }

    lxb_dom_node_destroy(node);

    return lxb_html_interface_element(root);
}

const lxb_css_rule_declaration_t *
lxb_html_element_style_by_name(lxb_html_element_t *element,
                               const lxb_char_t *name, size_t size)
{
    uintptr_t id;
    lxb_html_style_node_t *node;
    lxb_dom_document_t *ddoc = lxb_dom_interface_node(element)->owner_document;
    lxb_html_document_t *doc = lxb_html_interface_document(ddoc);
    const lxb_css_entry_data_t *data;

    data = lxb_css_property_by_name(name, size);

    if (data == NULL) {
        id = lxb_html_document_css_customs_find_id(doc, name, size);
        if (id == 0) {
            return NULL;
        }
    }
    else {
        id = data->unique;
    }

    node = (void *) lexbor_avl_search(doc->css.styles, element->style, id);

    return (node != NULL) ? node->entry.value : NULL;
}

const lxb_css_rule_declaration_t *
lxb_html_element_style_by_id(lxb_html_element_t *element, uintptr_t id)
{
    lxb_html_style_node_t *node;

    lxb_dom_document_t *ddoc = lxb_dom_interface_node(element)->owner_document;
    lxb_html_document_t *doc = lxb_html_interface_document(ddoc);

    node = (void *) lexbor_avl_search(doc->css.styles, element->style, id);
    if (node == NULL) {
        return NULL;
    }

    return node->entry.value;
}

lxb_status_t
lxb_html_element_style_parse(lxb_html_element_t *element,
                             const lxb_char_t *style, size_t size)
{
    lxb_css_rule_declaration_list_t *list;

    lxb_dom_document_t *ddoc = lxb_dom_interface_node(element)->owner_document;
    lxb_html_document_t *doc = lxb_html_interface_document(ddoc);
    lxb_html_document_css_t *css = &doc->css;

    list = lxb_css_declaration_list_parse(css->parser, css->memory,
                                          style, size);
    if (list == NULL) {
        return css->parser->status;
    }

    return lxb_html_element_style_list_append(element, list,
                                              lxb_css_selector_sp_up_s(0));
}

lxb_status_t
lxb_html_element_style_append(lxb_html_element_t *element,
                              lxb_css_rule_declaration_t *declr,
                              lxb_css_selector_specificity_t spec)
{
    uintptr_t id;
    lexbor_str_t *name;
    lxb_html_style_node_t *node;

    lxb_dom_document_t *ddoc = lxb_dom_interface_node(element)->owner_document;
    lxb_html_document_t *doc = lxb_html_interface_document(ddoc);
    lxb_html_document_css_t *css = &doc->css;

    id = declr->type;

    lxb_css_selector_sp_set_i(spec, declr->important);

    if (id == LXB_CSS_PROPERTY__CUSTOM) {
        name = &declr->u.custom->name;

        id = lxb_html_document_css_customs_id(doc, name->data,
                                              name->length);
        if (id == 0) {
            /* FIXME: what to do with an error? */
            return LXB_STATUS_ERROR;
        }
    }

    node = (void *) lexbor_avl_search(css->styles, element->style, id);
    if (node != NULL) {
        if (spec >= node->sp) {
            node->entry.value = declr;
            node->sp = spec;
        }

        return LXB_STATUS_OK;
    }

    node = (void *) lexbor_avl_insert(css->styles, &element->style,
                                      id, declr);
    if (node == NULL) {
        /* FIXME: what to do with an error? */
        return LXB_STATUS_ERROR;
    }

    node->sp = spec;

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_element_style_list_append(lxb_html_element_t *element,
                                   lxb_css_rule_declaration_list_t *list,
                                   lxb_css_selector_specificity_t spec)
{
    lxb_status_t status;
    lxb_css_rule_t *rule;
    lxb_css_rule_declaration_t *declr;

    rule = list->first;

    while (rule != NULL) {
        if (rule->type != LXB_CSS_RULE_DECLARATION) {
            goto next;
        }

        declr = lxb_css_rule_declaration(rule);

        status = lxb_html_element_style_append(element, declr, spec);
        if (status != LXB_STATUS_OK) {
            /* FIXME: what to do with an error? */
        }

    next:

        rule = rule->next;
    }

    return LXB_STATUS_OK;
}
