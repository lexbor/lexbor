/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/style/dom/interfaces/document.h"
#include "lexbor/style/event.h"
#include "lexbor/style/dom/interfaces/element.h"


static const lexbor_hash_search_t  lxb_dom_document_css_customs_se = {
    .cmp = lexbor_str_data_ncasecmp,
    .hash = lexbor_hash_make_id
};

static const lexbor_hash_insert_t  lxb_dom_document_css_customs_in = {
    .copy = lexbor_hash_copy,
    .cmp = lexbor_str_data_ncasecmp,
    .hash = lexbor_hash_make_id
};

static const lxb_dom_document_node_cb_t lxb_dom_document_node_cbs =
{
    .insert = lxb_style_event_insert,
    .remove = lxb_style_event_remove,
    .destroy = lxb_style_event_destroy,
    .set_value = lxb_style_event_set_value
};


typedef struct {
    lexbor_hash_entry_t entry;
    uintptr_t           id;
}
lxb_dom_document_css_custom_entry_t;

typedef struct {
    lxb_dom_document_t              *doc;
    lxb_css_rule_declaration_list_t *list;
}
lxb_dom_document_remove_ctx_t;


static lxb_dom_document_css_custom_entry_t *
lxb_dom_document_css_customs_insert(lxb_dom_document_t *document,
                                    const lxb_char_t *key, size_t length);

static lxb_status_t
lxb_dom_document_style_remove_by_rule_cb(lxb_dom_node_t *node,
                                         lxb_css_selector_specificity_t spec,
                                         void *ctx);

static lxb_status_t
lxb_dom_document_style_remove_avl_cb(lexbor_avl_t *avl,
                                     lexbor_avl_node_t **root,
                                     lexbor_avl_node_t *node, void *ctx);

static lxb_status_t
lxb_dom_document_style_attach_cb(lxb_dom_node_t *node,
                                 lxb_css_selector_specificity_t spec, void *ctx);


lxb_status_t
lxb_dom_document_css_init(lxb_dom_document_t *document, bool init_events)
{
    lxb_status_t status;
    lxb_dom_document_css_t *css = document->css;

    if (css != NULL) {
        return LXB_STATUS_OK;
    }

    css = lexbor_calloc(1, sizeof(lxb_dom_document_css_t));
    if (css == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    document->css = css;

    css->memory = lxb_css_memory_create();
    status = lxb_css_memory_init(css->memory, 1024);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    css->css_selectors = lxb_css_selectors_create();
    status = lxb_css_selectors_init(css->css_selectors);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    css->parser = lxb_css_parser_create();
    status = lxb_css_parser_init(css->parser, NULL);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    lxb_css_parser_memory_set(css->parser, css->memory);
    lxb_css_parser_selectors_set(css->parser, css->css_selectors);

    css->selectors = lxb_selectors_create();
    status = lxb_selectors_init(css->selectors);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    css->styles = lexbor_avl_create();
    status = lexbor_avl_init(css->styles, 2048, sizeof(lxb_style_node_t));
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    css->stylesheets = lexbor_array_create();
    status = lexbor_array_init(css->stylesheets, 16);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    css->weak = lexbor_dobject_create();
    status = lexbor_dobject_init(css->weak, 2048,
                                 sizeof(lxb_style_weak_t));
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    status = lxb_dom_document_css_customs_init(document);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    if (init_events) {
        document->node_cb = &lxb_dom_document_node_cbs;
    }

    return LXB_STATUS_OK;

failed:

    lxb_dom_document_css_destroy(document);

    return status;
}

void
lxb_dom_document_css_destroy(lxb_dom_document_t *document)
{
    lxb_dom_document_css_t *css = document->css;

    if (document->css == NULL
        || lxb_dom_interface_node(document)->owner_document
        != lxb_dom_interface_document(document))
    {
        return;
    }

    css->memory = lxb_css_memory_destroy(css->memory, true);
    css->css_selectors = lxb_css_selectors_destroy(css->css_selectors, true);
    css->parser = lxb_css_parser_destroy(css->parser, true);
    css->selectors = lxb_selectors_destroy(css->selectors, true);
    css->styles = lexbor_avl_destroy(css->styles, true);
    css->stylesheets = lexbor_array_destroy(css->stylesheets, true);
    css->weak = lexbor_dobject_destroy(css->weak, true);

    lxb_dom_document_css_customs_destroy(document);

    document->css = lexbor_free(css);
}

void
lxb_dom_document_css_clean(lxb_dom_document_t *document)
{
    lxb_dom_document_css_t *css;

    if (lxb_dom_interface_node(document)->owner_document
        == lxb_dom_interface_document(document))
    {
        if (document->css == NULL) {
            return;
        }

        css = document->css;

        lxb_css_memory_clean(css->memory);
        lxb_css_selectors_clean(css->css_selectors);
        lxb_css_parser_clean(css->parser);
        lxb_selectors_clean(css->selectors);
        lexbor_avl_clean(css->styles);
        lexbor_array_clean(css->stylesheets);
        lexbor_dobject_clean(css->weak);
    }
}

lxb_status_t
lxb_dom_document_css_customs_init(lxb_dom_document_t *document)
{
    lxb_dom_document_css_t *css = document->css;

    css->customs_id = LXB_CSS_PROPERTY__LAST_ENTRY;

    css->customs = lexbor_hash_create();
    return lexbor_hash_init(css->customs, 512,
                            sizeof(lxb_dom_document_css_custom_entry_t));
}

void
lxb_dom_document_css_customs_destroy(lxb_dom_document_t *document)
{
    document->css->customs = lexbor_hash_destroy(document->css->customs, true);
}

uintptr_t
lxb_dom_document_css_customs_find_id(const lxb_dom_document_t *document,
                                     const lxb_char_t *key, size_t length)
{
    const lxb_dom_document_css_custom_entry_t *entry;

    entry = lexbor_hash_search(document->css->customs,
                               &lxb_dom_document_css_customs_se, key, length);

    return (entry != NULL) ? entry->id : 0;
}

static lxb_dom_document_css_custom_entry_t *
lxb_dom_document_css_customs_insert(lxb_dom_document_t *document,
                                    const lxb_char_t *key, size_t length)
{
    lxb_dom_document_css_custom_entry_t *entry;

    if (UINTPTR_MAX - document->css->customs_id == 0) {
        return NULL;
    }

    entry = lexbor_hash_insert(document->css->customs,
                               &lxb_dom_document_css_customs_in, key, length);
    if (entry == NULL) {
        return NULL;
    }

    entry->id = document->css->customs_id++;

    return entry;
}

uintptr_t
lxb_dom_document_css_customs_id(lxb_dom_document_t *document,
                                const lxb_char_t *key, size_t length)
{
    lxb_dom_document_css_custom_entry_t *entry;

    entry = lexbor_hash_search(document->css->customs,
                               &lxb_dom_document_css_customs_se, key, length);
    if (entry != NULL) {
        return entry->id;
    }

    entry = lxb_dom_document_css_customs_insert(document, key, length);
    if (entry == NULL) {
        return 0;
    }

    return entry->id;
}

lxb_status_t
lxb_dom_document_stylesheet_attach(lxb_dom_document_t *document,
                                   lxb_css_stylesheet_t *sst)
{
    lxb_status_t status;

    status = lexbor_array_push(document->css->stylesheets, sst);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return lxb_dom_document_stylesheet_apply(document, sst);
}

lxb_status_t
lxb_dom_document_stylesheet_apply(lxb_dom_document_t *document,
                                  lxb_css_stylesheet_t *sst)
{
    lxb_status_t status = LXB_STATUS_OK;
    lxb_css_rule_t *rule;
    lxb_css_rule_list_t *list;

    rule = sst->root;

    if (rule->type != LXB_CSS_RULE_LIST) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    list = lxb_css_rule_list(rule);
    rule = list->first;

    while (rule != NULL) {
        switch (rule->type) {
            case LXB_CSS_RULE_STYLE:
                status = lxb_dom_document_style_attach(document,
                                                       lxb_css_rule_style(rule));
                break;

            default:
                break;
        }

        if (status != LXB_STATUS_OK) {
            /* FIXME: what to do with an error? */
        }

        rule = rule->next;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_dom_document_stylesheet_add(lxb_dom_document_t *document,
                                lxb_css_stylesheet_t *sst)
{
    if (sst == NULL) {
        return LXB_STATUS_OK;
    }

    return lexbor_array_push(document->css->stylesheets, sst);
}

lxb_status_t
lxb_dom_document_stylesheet_remove(lxb_dom_document_t *document,
                                   lxb_css_stylesheet_t *sst)
{
    size_t i, length;
    lxb_status_t status = LXB_STATUS_OK;
    lxb_css_rule_t *rule;
    lxb_css_rule_list_t *list;
    lxb_css_stylesheet_t *sst_in;

    rule = sst->root;

    if (rule->type != LXB_CSS_RULE_LIST) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    list = lxb_css_rule_list(rule);
    rule = list->first;

    while (rule != NULL) {
        switch (rule->type) {
            case LXB_CSS_RULE_STYLE:
                status = lxb_dom_document_style_remove(document,
                                                       lxb_css_rule_style(rule));
                break;

            default:
                break;
        }

        if (status != LXB_STATUS_OK) {
            /* FIXME: what to do with an error? */
        }

        rule = rule->next;
    }

    length = lexbor_array_length(document->css->stylesheets);

    for (i = 0; i < length; i++) {
        sst_in = lexbor_array_get(document->css->stylesheets, i);

        if (sst_in == sst) {
            lexbor_array_delete(document->css->stylesheets, i, 1);
            length = lexbor_array_length(document->css->stylesheets);
        }
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_dom_document_element_styles_attach(lxb_dom_element_t *element)
{
    lxb_status_t status = LXB_STATUS_OK;
    lxb_css_rule_t *rule;
    lexbor_array_t *ssts;
    lxb_css_rule_list_t *list;
    lxb_css_stylesheet_t *sst;
    lxb_dom_document_t *document;

    document = lxb_dom_interface_node(element)->owner_document;
    ssts = document->css->stylesheets;

    for (size_t i = 0; i < lexbor_array_length(ssts); i++) {
        sst = lexbor_array_get(ssts, i);

        list = lxb_css_rule_list(sst->root);
        rule = list->first;

        while (rule != NULL) {
            switch (rule->type) {
                case LXB_CSS_RULE_STYLE:
                    status = lxb_dom_document_style_attach_by_element(document,
                                             element, lxb_css_rule_style(rule));
                    break;

                default:
                    break;
            }

            if (status != LXB_STATUS_OK) {
                /* FIXME: what to do with an error? */
            }

            rule = rule->next;
        }
    }

    return LXB_STATUS_OK;
}

void
lxb_dom_document_stylesheet_destroy_all(lxb_dom_document_t *document,
                                        bool destroy_memory)
{
    size_t length;
    lxb_css_stylesheet_t *sst;
    lxb_dom_document_css_t *css = document->css;

    length = lexbor_array_length(css->stylesheets);

    for (size_t i = 0; i < length; i++) {
        sst = lexbor_array_pop(css->stylesheets);

        (void) lxb_css_stylesheet_destroy(sst, destroy_memory);
    }
}

lxb_status_t
lxb_dom_document_style_attach(lxb_dom_document_t *document,
                              lxb_css_rule_style_t *style)
{
    lxb_dom_document_css_t *css = document->css;

    return lxb_selectors_find(css->selectors, lxb_dom_interface_node(document),
                              style->selector, lxb_dom_document_style_attach_cb, style);
}

lxb_status_t
lxb_dom_document_style_remove(lxb_dom_document_t *document,
                              lxb_css_rule_style_t *style)
{
    lxb_dom_document_css_t *css = document->css;

    return lxb_selectors_find(css->selectors, lxb_dom_interface_node(document),
                              style->selector,
                              lxb_dom_document_style_remove_by_rule_cb, style);
}

static lxb_status_t
lxb_dom_document_style_remove_by_rule_cb(lxb_dom_node_t *node,
                                         lxb_css_selector_specificity_t spec,
                                         void *ctx)
{
    lxb_dom_element_t *el;
    lxb_dom_document_t *doc;
    lxb_css_rule_style_t *style = ctx;
    lxb_dom_document_remove_ctx_t context;

    el = lxb_dom_interface_element(node);

    if (el->style == NULL) {
        return LXB_STATUS_OK;
    }

    doc = node->owner_document;

    context.doc = doc;
    context.list = style->declarations;

    return lexbor_avl_foreach(doc->css->styles, &el->style,
                              lxb_dom_document_style_remove_avl_cb, &context);
}

static lxb_status_t
lxb_dom_document_style_remove_avl_cb(lexbor_avl_t *avl,
                                     lexbor_avl_node_t **root,
                                     lexbor_avl_node_t *node, void *ctx)
{
    lxb_dom_document_remove_ctx_t *context = ctx;
    lxb_style_node_t *style = (lxb_style_node_t *) node;

    lxb_dom_element_style_remove_by_list(context->doc, root, style,
                                         context->list);
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_dom_document_style_attach_by_element(lxb_dom_document_t *document,
                                         lxb_dom_element_t *element,
                                         lxb_css_rule_style_t *style)
{
    lxb_dom_document_css_t *css = document->css;

    return lxb_selectors_match_node(css->selectors, lxb_dom_interface_node(element),
                                    style->selector, lxb_dom_document_style_attach_cb, style);
}

static lxb_status_t
lxb_dom_document_style_attach_cb(lxb_dom_node_t *node,
                                 lxb_css_selector_specificity_t spec, void *ctx)
{
    lxb_css_rule_style_t *style = ctx;

    if (style->declarations == NULL) {
        return LXB_STATUS_OK;
    }

    return lxb_dom_element_style_list_append(lxb_dom_interface_element(node),
                                             style->declarations, spec);
}

lxb_status_t
lxb_document_apply_stylesheets(lxb_dom_document_t *document)
{
    size_t i, length;
    lxb_status_t status;
    lxb_css_stylesheet_t *sst;

    if (document->css == NULL) {
        return LXB_STATUS_OK;
    }

    length = lexbor_array_length(document->css->stylesheets);

    for (i = 0; i < length; i++) {
        sst = lexbor_array_get(document->css->stylesheets, i);

        status = lxb_dom_document_stylesheet_apply(document, sst);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}
