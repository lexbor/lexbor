/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/style/dom/interfaces/element.h"
#include "lexbor/style/style.h"
#include "lexbor/style/dom/interfaces/document.h"
#include "lexbor/dom/interfaces/document.h"
#include "lexbor/core/avl.h"


typedef struct {
    lxb_dom_element_t          *element;
    lxb_dom_element_style_cb_f cb;
    void                       *ctx;
    bool                       weak;
}
lxb_dom_element_style_walk_ctx_t;

typedef struct {
    lexbor_str_t  *str;
    lexbor_mraw_t *mraw;
}
lxb_dom_element_style_ctx_t;


static lxb_status_t
lxb_style_document_cb(lxb_dom_node_t *node,
                      lxb_css_selector_specificity_t spec, void *ctx);

static lxb_status_t
lxb_dom_element_style_walk_cb(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                              lexbor_avl_node_t *node, void *ctx);

static lxb_status_t
lxb_dom_element_style_serialize_cb(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                                   lexbor_avl_node_t *node, void *ctx);

static lxb_status_t
lxb_dom_element_style_serialize_str_cb(const lxb_char_t *data,
                                       size_t len, void *ctx);


const lxb_css_rule_declaration_t *
lxb_dom_element_style_by_name(const lxb_dom_element_t *element,
                              const lxb_char_t *name, size_t size)
{
    uintptr_t id;
    lxb_style_node_t *node;
    const lxb_dom_document_t *doc = lxb_dom_element_document(element);

    id = lxb_style_id_by_name(doc, name, size);
    if (id == LXB_CSS_PROPERTY__UNDEF) {
        return NULL;
    }

    node = (void *) lexbor_avl_search(doc->css->styles, element->style, id);

    return (node != NULL) ? node->entry.value : NULL;
}

const lxb_css_rule_declaration_t *
lxb_dom_element_style_by_id(const lxb_dom_element_t *element, uintptr_t id)
{
    const lxb_style_node_t *node;

    node = lxb_dom_element_style_node_by_id(element, id);
    if (node == NULL) {
        return NULL;
    }

    return node->entry.value;
}

const lxb_style_node_t *
lxb_dom_element_style_node_by_id(const lxb_dom_element_t *element, uintptr_t id)
{
    lxb_dom_document_t *doc = lxb_dom_element_document(element);

    return (lxb_style_node_t *) lexbor_avl_search(doc->css->styles,
                                                  element->style, id);
}

const lxb_style_node_t *
lxb_dom_element_style_node_by_name(const lxb_dom_element_t *element,
                                   const lxb_char_t *name, size_t size)
{
    uintptr_t id;
    lxb_dom_document_t *doc = lxb_dom_element_document(element);

    id = lxb_style_id_by_name(doc, name, size);
    if (id == LXB_CSS_PROPERTY__UNDEF) {
        return NULL;
    }

    return (lxb_style_node_t *) lexbor_avl_search(doc->css->styles,
                                                  element->style, id);
}

const void *
lxb_dom_element_css_property_by_id(const lxb_dom_element_t *element,
                                   uintptr_t id)
{
    lxb_css_rule_declaration_t *declr;
    const lxb_style_node_t *node;

    node = lxb_dom_element_style_node_by_id(element, id);
    if (node == NULL) {
        return lxb_css_property_initial_by_id(id);
    }

    declr = node->entry.value;

    return declr->u.user;
}

lxb_status_t
lxb_dom_element_style_attach_exists(lxb_dom_element_t *element)
{
    lxb_status_t status = LXB_STATUS_OK;
    lxb_css_rule_t *rule;
    lexbor_array_t *ssts;
    lxb_css_rule_list_t *list;
    lxb_css_stylesheet_t *sst;
    lxb_dom_document_t *document;

    document = lxb_dom_element_document(element);
    ssts = document->css->stylesheets;

    for (size_t i = 0; i < lexbor_array_length(ssts); i++) {
        sst = lexbor_array_get(ssts, i);

        list = lxb_css_rule_list(sst->root);
        rule = list->first;

        while (rule != NULL) {
            switch (rule->type) {
                case LXB_CSS_RULE_STYLE:
                    status = lxb_dom_element_style_attach(element,
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
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_dom_element_style_attach(lxb_dom_element_t *element,
                             lxb_css_rule_style_t *style)
{
    lxb_dom_document_css_t *css = lxb_dom_element_document(element)->css;

    return lxb_selectors_find(css->selectors, lxb_dom_interface_node(element),
                              style->selector, lxb_style_document_cb, style);
}

static lxb_status_t
lxb_style_document_cb(lxb_dom_node_t *node,
                      lxb_css_selector_specificity_t spec, void *ctx)
{
    lxb_css_rule_style_t *style = ctx;

    return lxb_dom_element_style_list_append(lxb_dom_interface_element(node),
                                         style->declarations, spec);
}

lxb_status_t
lxb_dom_element_style_list_append(lxb_dom_element_t *element,
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

        status = lxb_dom_element_style_append(element, declr, spec);
        if (status != LXB_STATUS_OK) {
            /* FIXME: what to do with an error? */
        }

    next:

        rule = rule->next;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_dom_element_style_append(lxb_dom_element_t *element,
                             lxb_css_rule_declaration_t *declr,
                             lxb_css_selector_specificity_t spec)
{
    uintptr_t id;
    lxb_status_t status;
    lexbor_str_t *name;
    lxb_style_node_t *node;

    lxb_dom_document_t *doc = lxb_dom_interface_node(element)->owner_document;
    lxb_dom_document_css_t *css = doc->css;

    id = declr->type;

    lxb_css_selector_sp_set_i(spec, declr->important);

    if (id == LXB_CSS_PROPERTY__CUSTOM) {
        name = &declr->u.custom->name;

        id = lxb_dom_document_css_customs_id(doc, name->data, name->length);
        if (id == 0) {
            /* FIXME: what to do with an error? */
            return LXB_STATUS_ERROR;
        }
    }

    node = (void *) lexbor_avl_search(css->styles, element->style, id);
    if (node != NULL) {
        if (spec < node->sp) {
            return lxb_dom_element_style_weak_append(doc, node, declr, spec);
        }

        status = lxb_dom_element_style_weak_append(doc, node,
                                                   node->entry.value, node->sp);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        lxb_css_rule_ref_dec(node->entry.value);

        node->entry.value = declr;
        node->sp = spec;

        return LXB_STATUS_OK;
    }

    node = (void *) lexbor_avl_insert(css->styles, &element->style, id, declr);
    if (node == NULL) {
        /* FIXME: what to do with an error? */
        return LXB_STATUS_ERROR;
    }

    node->sp = spec;

    return lxb_css_rule_ref_inc(lxb_css_rule(declr));
}

lxb_status_t
lxb_dom_element_style_weak_append(lxb_dom_document_t *doc,
                                  lxb_style_node_t *node,
                                  lxb_css_rule_declaration_t *declr,
                                  lxb_css_selector_specificity_t spec)
{
    lxb_style_weak_t *weak, *prev, *new_weak;

    new_weak = lexbor_dobject_alloc(doc->css->weak);
    if (new_weak == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    new_weak->value = declr;
    new_weak->sp = spec;

    if (node->weak == NULL) {
        node->weak = new_weak;
        new_weak->next = NULL;

        goto done;
    }

    weak = node->weak;

    if (weak->sp <= spec) {
        node->weak = new_weak;
        new_weak->next = weak;

        goto done;
    }

    prev = weak;
    weak = weak->next;

    while (weak != NULL) {
        if (weak->sp <= spec) {
            prev->next = new_weak;
            new_weak->next = weak;

            goto done;
        }

        prev = weak;
        weak = weak->next;
    }

    prev->next = new_weak;
    new_weak->next = NULL;

done:

    return lxb_css_rule_ref_inc(lxb_css_rule(declr));
}

lxb_status_t
lxb_dom_element_style_walk(lxb_dom_element_t *element,
                           lxb_dom_element_style_cb_f cb,
                           void *ctx, bool with_weak)
{
    lxb_dom_element_style_walk_ctx_t walk;

    walk.element = element;
    walk.cb = cb;
    walk.ctx = ctx;
    walk.weak = with_weak;

    return lexbor_avl_foreach(NULL, &element->style,
                              lxb_dom_element_style_walk_cb, &walk);
}

static lxb_status_t
lxb_dom_element_style_walk_cb(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                              lexbor_avl_node_t *node, void *ctx)
{
    lxb_status_t status;
    lxb_style_weak_t *weak;
    lxb_style_node_t *style;
    lxb_dom_element_style_walk_ctx_t *walk = ctx;

    style = (lxb_style_node_t *) node;

    status = walk->cb(walk->element, node->value, walk->ctx, style->sp, false);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    weak = style->weak;

    while (weak != NULL) {
        status = walk->cb(walk->element, weak->value, walk->ctx,
                          weak->sp, true);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        weak = weak->next;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_dom_element_style_parse(lxb_dom_element_t *element,
                            const lxb_char_t *style, size_t size)
{
    lxb_css_rule_declaration_list_t *list;

    lxb_dom_document_t *doc = lxb_dom_element_document(element);
    lxb_dom_document_css_t *css = doc->css;

    list = lxb_css_declaration_list_parse(css->parser, css->memory,
                                          style, size);
    if (list == NULL) {
        return css->parser->status;
    }

    element->list = list;

    return lxb_dom_element_style_list_append(element, list,
                                             lxb_css_selector_sp_up_s(0));
}

void
lxb_dom_element_style_remove_by_name(lxb_dom_element_t *element,
                                     const lxb_char_t *name, size_t size)
{
    uintptr_t id;
    lxb_dom_document_t *doc = lxb_dom_element_document(element);

    id = lxb_style_id_by_name(doc, name, size);
    if (id == LXB_CSS_PROPERTY__UNDEF) {
        return;
    }

    lxb_dom_element_style_remove_by_id(element, id);
}

void
lxb_dom_element_style_remove_by_id(lxb_dom_element_t *element, uintptr_t id)
{
    lxb_style_node_t *node;
    lxb_dom_document_t *doc = lxb_dom_element_document(element);

    node = (lxb_style_node_t *) lexbor_avl_search(doc->css->styles,
                                                  element->style, id);
    if (node != NULL) {
        lxb_dom_element_style_remove_all(doc, &element->style, node);
    }
}

lxb_style_node_t *
lxb_dom_element_style_remove_all_not(lxb_dom_document_t *doc,
                                     lexbor_avl_node_t **root,
                                     lxb_style_node_t *style, bool bs)
{
    lxb_style_weak_t *weak, *prev, *next;

    weak = style->weak;
    prev = NULL;

    while (weak != NULL) {
        next = weak->next;

        if (lxb_css_selector_sp_s(weak->sp) == bs) {
            lxb_css_rule_ref_dec_destroy(weak->value);
            lexbor_dobject_free(doc->css->weak, weak);

            if (prev != NULL) {
                prev->next = next;
            }
            else {
                style->weak = next;
            }
        }
        else {
            prev = weak;
        }

        weak = next;
    }

    if (lxb_css_selector_sp_s(style->sp) != bs) {
        return style;
    }

    lxb_css_rule_ref_dec_destroy(style->entry.value);

    if (style->weak == NULL) {
        lexbor_avl_remove_by_node(doc->css->styles, root,
                                  (lexbor_avl_node_t *) style);
        return NULL;
    }

    weak = style->weak;

    style->entry.value = weak->value;
    style->sp = weak->sp;
    style->weak = weak->next;

    lexbor_dobject_free(doc->css->weak, weak);

    return style;
}

lxb_style_node_t *
lxb_dom_element_style_remove_all(lxb_dom_document_t *doc, lexbor_avl_node_t **root,
                                 lxb_style_node_t *style)
{
    lxb_style_weak_t *weak, *next;

    weak = style->weak;

    while (weak != NULL) {
        next = weak->next;

        lxb_css_rule_ref_dec_destroy(weak->value);
        lexbor_dobject_free(doc->css->weak, weak);

        weak = next;
    }

    lxb_css_rule_ref_dec_destroy(style->entry.value);
    lexbor_avl_remove_by_node(doc->css->styles, root,
                              (lexbor_avl_node_t *) style);
    return NULL;
}

lxb_style_node_t *
lxb_dom_element_style_remove_by_list(lxb_dom_document_t *doc,
                                     lexbor_avl_node_t **root,
                                     lxb_style_node_t *style,
                                     lxb_css_rule_declaration_list_t *list)
{
    lxb_style_weak_t *weak, *prev, *next;

    weak = style->weak;
    prev = NULL;

    while (weak != NULL) {
        next = weak->next;

        if (((lxb_css_rule_declaration_t *) weak->value)->rule.parent
            == (lxb_css_rule_t *) list)
        {
            lxb_css_rule_ref_dec_destroy(weak->value);
            lexbor_dobject_free(doc->css->weak, weak);

            if (prev != NULL) {
                prev->next = next;
            }
            else {
                style->weak = next;
            }
        }
        else {
            prev = weak;
        }

        weak = next;
    }

    if (((lxb_css_rule_declaration_t *) style->entry.value)->rule.parent
        != (lxb_css_rule_t *) list)
    {
        return style;
    }

    lxb_css_rule_ref_dec_destroy(style->entry.value);

    if (style->weak == NULL) {
        lexbor_avl_remove_by_node(doc->css->styles, root,
                                  (lexbor_avl_node_t *) style);
        return NULL;
    }

    weak = style->weak;

    style->entry.value = weak->value;
    style->sp = weak->sp;
    style->weak = weak->next;

    lexbor_dobject_free(doc->css->weak, weak);

    return style;
}

lxb_status_t
lxb_dom_element_style_serialize(lxb_dom_element_t *element,
                                lxb_dom_element_style_opt_t opt,
                                lexbor_serialize_cb_f cb, void *ctx)
{
    lexbor_serialize_ctx_t context;

    context.cb = cb;
    context.ctx = ctx;
    context.opt = opt;
    context.count = 0;

    return lexbor_avl_foreach(NULL, &element->style,
                              lxb_dom_element_style_serialize_cb, &context);
}

static lxb_status_t
lxb_dom_element_style_serialize_cb(lexbor_avl_t *avl, lexbor_avl_node_t **root,
                                   lexbor_avl_node_t *node, void *ctx)
{
    lxb_status_t status;
    lexbor_serialize_ctx_t *context = ctx;

    static const lexbor_str_t splt = lexbor_str("; ");

    if (context->count > 0) {
        lexbor_serialize_write(context->cb, splt.data, splt.length,
                               context->ctx, status);
    }

    context->count = 1;

    return lxb_css_rule_serialize(node->value, context->cb, context->ctx);
}

lxb_status_t
lxb_dom_element_style_serialize_str(lxb_dom_element_t *element, lexbor_str_t *str,
                                    lxb_dom_element_style_opt_t opt)
{
    lxb_dom_document_t *doc;
    lxb_dom_element_style_ctx_t context;

    doc = lxb_dom_interface_node(element)->owner_document;

    if (str->data == NULL) {
        lexbor_str_init(str, doc->text, 1024);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    context.str = str;
    context.mraw = doc->text;

    return lxb_dom_element_style_serialize(element, opt,
                                           lxb_dom_element_style_serialize_str_cb,
                                           &context);
}

static lxb_status_t
lxb_dom_element_style_serialize_str_cb(const lxb_char_t *data,
                                       size_t len, void *ctx)
{
    lxb_char_t *ret;
    lxb_dom_element_style_ctx_t *context = ctx;

    ret = lexbor_str_append(context->str, context->mraw, data, len);
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}
