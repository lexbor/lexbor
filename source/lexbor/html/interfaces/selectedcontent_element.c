/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/dom/interfaces/document_fragment.h"
#include "lexbor/html/interfaces/selectedcontent_element.h"
#include "lexbor/html/interfaces/select_element.h"
#include "lexbor/html/interfaces/option_element.h"
#include "lexbor/html/interfaces/document.h"


lxb_html_selectedcontent_element_t *
lxb_html_selectedcontent_element_interface_create(lxb_html_document_t *document)
{
    lxb_html_selectedcontent_element_t *element;

    element = lexbor_mraw_calloc(document->dom_document.mraw,
                                 sizeof(lxb_html_selectedcontent_element_t));
    if (element == NULL) {
        return NULL;
    }

    /* disabled default is false. */

    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    node->owner_document = lxb_html_document_original_ref(document);
    node->type = LXB_DOM_NODE_TYPE_ELEMENT;

    return element;
}

lxb_html_selectedcontent_element_t *
lxb_html_selectedcontent_element_interface_destroy(lxb_html_selectedcontent_element_t *selectedcontent_element)
{
    (void) lxb_dom_node_interface_destroy(lxb_dom_interface_node(selectedcontent_element));
    return NULL;
}

lxb_dom_exception_code_t
lxb_html_selectedcontent_clone_option(lxb_html_selectedcontent_element_t *sc,
                                      lxb_html_option_element_t *option)
{
    lxb_dom_node_t *node, *child, *opt_node;
    lxb_dom_document_fragment_t fragment;

    memset(&fragment, 0x00, sizeof(lxb_dom_document_fragment_t));

    opt_node = lxb_dom_interface_node(option);

    fragment.node.type = LXB_DOM_NODE_TYPE_DOCUMENT_FRAGMENT;
    fragment.node.owner_document = opt_node->owner_document;

    node = opt_node->first_child;

    while (node != NULL) {
        child = lxb_dom_node_clone(node, true);
        if (child == NULL) {
            return LXB_DOM_EXCEPTION_ERR;
        }

        lxb_dom_node_insert_child_wo_events(lxb_dom_interface_node(&fragment),
                                            child);
        node = node->next;
    }

    return lxb_dom_node_replace_all_spec(lxb_dom_interface_node(sc),
                                         lxb_dom_interface_node(&fragment));
}

lxb_status_t
lxb_html_selectedcontent_insert_steps(lxb_dom_node_t *inserted_node)
{
    lxb_html_selectedcontent_element_t *sc;
    lxb_html_select_element_t *select;

    sc = lxb_html_interface_selectedcontent(inserted_node);

    /* Check if parent is a <select> element without `multiple` attribute. */

    if (inserted_node->parent == NULL
        || inserted_node->parent->local_name != LXB_TAG_SELECT
        || inserted_node->parent->ns != LXB_NS_HTML)
    {
        sc->disabled = true;
        return LXB_STATUS_OK;
    }

    select = lxb_html_interface_select(inserted_node->parent);

    sc->disabled = false;

    (void) lxb_html_select_selectedness_setting_algorithm(select);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_selectedcontent_remove_steps(lxb_dom_node_t *removed_node,
                                      lxb_dom_node_t *old_parent)
{
    lxb_html_selectedcontent_element_t *sc;

    sc = lxb_html_interface_selectedcontent(removed_node);
    sc->disabled = true;

    return LXB_STATUS_OK;
}
