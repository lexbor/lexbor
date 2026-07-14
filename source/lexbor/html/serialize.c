/*
 * Copyright (C) 2018-2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/dom/interfaces/text.h"
#include "lexbor/dom/interfaces/comment.h"
#include "lexbor/dom/interfaces/processing_instruction.h"
#include "lexbor/dom/interfaces/document_type.h"

#include "lexbor/html/serialize.h"
#include "lexbor/html/tree.h"
#include "lexbor/ns/ns.h"
#include "lexbor/html/interfaces/template_element.h"

#ifndef LEXBOR_DISABLE_INTERNAL_EXTERN
    LXB_EXTERN const unsigned char lexbor_tokenizer_chars_map[256];
#endif

#define lxb_html_serialize_send(data, len, ctx)                                \
    do {                                                                       \
        status = cb((const lxb_char_t *) data, len, ctx);                      \
        if (status != LXB_STATUS_OK) {                                         \
            return status;                                                     \
        }                                                                      \
    }                                                                          \
    while (0)

#define lxb_html_serialize_send_indent(count, ctx)                             \
    do {                                                                       \
        for (size_t i = 0; i < count; i++) {                                   \
            lxb_html_serialize_send("  ", 2, ctx);                             \
        }                                                                      \
    }                                                                          \
    while (0)


typedef struct {
    lexbor_str_t  *str;
    lexbor_mraw_t *mraw;
}
lxb_html_serialize_ctx_t;

typedef struct {
    lxb_dom_attr_t *attr;
    size_t          offset;
    size_t          length;
}
lxb_html_serialize_attr_entry_t;


static lxb_status_t
lxb_html_serialize_str_callback(const lxb_char_t *data, size_t len, void *ctx);

static lxb_status_t
lxb_html_serialize_node_cb(lxb_dom_node_t *node,
                           lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_element_cb(lxb_dom_element_t *element,
                              lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_element_closed_cb(lxb_dom_element_t *element,
                                     lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_text_cb(lxb_dom_text_t *text,
                           lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_comment_cb(lxb_dom_comment_t *comment,
                              lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_processing_instruction_cb(lxb_dom_processing_instruction_t *pi,
                                             lxb_html_serialize_cb_f cb,
                                             void *ctx);

static lxb_status_t
lxb_html_serialize_document_type_cb(lxb_dom_document_type_t *doctype,
                                    lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_document_type_full_cb(lxb_dom_document_type_t *doctype,
                                         lxb_html_serialize_opt_t opt,
                                         lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_document_cb(lxb_dom_document_t *document,
                               lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_send_escaping_attribute_string(const lxb_char_t *data,
                                                  size_t len,
                                                  lxb_html_serialize_cb_f cb,
                                                  void *ctx);

static lxb_status_t
lxb_html_serialize_send_escaping_string(const lxb_char_t *data, size_t len,
                                        lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_attribute_cb(lxb_dom_attr_t *attr,
                                lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_pretty_node_cb(lxb_dom_node_t *node,
                                  lxb_html_serialize_opt_t opt, size_t deep,
                                  lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_pretty_element_cb(lxb_dom_element_t *element,
                                     lxb_html_serialize_opt_t opt, size_t indent,
                                     lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_pretty_attribute_cb(lxb_dom_attr_t *attr,
                                       lxb_html_serialize_opt_t opt, bool has_raw,
                                       lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_pretty_attributes_sorted(lxb_dom_element_t *element,
                                            lxb_html_serialize_opt_t opt,
                                            size_t indent,
                                            lxb_html_serialize_cb_f cb,
                                            void *ctx);

static size_t
lxb_html_serialize_attr_name_size(const lxb_dom_attr_t *attr);

static size_t
lxb_html_serialize_attr_name_build(const lxb_dom_attr_t *attr,
                                   lxb_char_t *buf, size_t cap);

static int
lxb_html_serialize_attr_entry_cmp(const lxb_html_serialize_attr_entry_t *a,
                                  const lxb_html_serialize_attr_entry_t *b,
                                  const lxb_char_t *names);

static void
lxb_html_serialize_attr_sort(lxb_html_serialize_attr_entry_t *entries,
                             size_t n, const lxb_char_t *names);

static lxb_status_t
lxb_html_serialize_pretty_text_cb(lxb_dom_text_t *text,
                                  lxb_html_serialize_opt_t opt, size_t indent,
                                  lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_pretty_comment_cb(lxb_dom_comment_t *comment,
                                     size_t indent, bool with_indent,
                                     lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_pretty_document_cb(lxb_dom_document_t *document,
                                      lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_pretty_send_escaping_string(const lxb_char_t *data, size_t len,
                                               size_t indent, bool with_indent,
                                               lxb_html_serialize_cb_f cb, void *ctx);

static lxb_status_t
lxb_html_serialize_pretty_send_string(const lxb_char_t *data, size_t len,
                                      size_t indent, bool with_indent,
                                      lxb_html_serialize_cb_f cb, void *ctx);


lxb_status_t
lxb_html_serialize_cb(lxb_dom_node_t *node,
                      lxb_html_serialize_cb_f cb, void *ctx)
{
    switch (node->type) {
        case LXB_DOM_NODE_TYPE_ELEMENT:
            return lxb_html_serialize_element_cb(lxb_dom_interface_element(node),
                                                 cb, ctx);

        case LXB_DOM_NODE_TYPE_TEXT:
            return lxb_html_serialize_text_cb(lxb_dom_interface_text(node),
                                              cb, ctx);

        case LXB_DOM_NODE_TYPE_COMMENT:
            return lxb_html_serialize_comment_cb(lxb_dom_interface_comment(node),
                                                 cb, ctx);

        case LXB_DOM_NODE_TYPE_PROCESSING_INSTRUCTION:
            return lxb_html_serialize_processing_instruction_cb(lxb_dom_interface_processing_instruction(node),
                                                                cb, ctx);

        case LXB_DOM_NODE_TYPE_DOCUMENT_TYPE:
            return lxb_html_serialize_document_type_cb(lxb_dom_interface_document_type(node),
                                                       cb, ctx);

        case LXB_DOM_NODE_TYPE_DOCUMENT:
            return lxb_html_serialize_document_cb(lxb_dom_interface_document(node),
                                                  cb, ctx);

        default:
            break;
    }

    return LXB_STATUS_ERROR;
}

lxb_status_t
lxb_html_serialize_str(lxb_dom_node_t *node, lexbor_str_t *str)
{
    lxb_html_serialize_ctx_t ctx;

    if (str->data == NULL) {
        lexbor_str_init(str, node->owner_document->text, 1024);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    ctx.str = str;
    ctx.mraw = node->owner_document->text;

    return lxb_html_serialize_cb(node, lxb_html_serialize_str_callback, &ctx);
}

static lxb_status_t
lxb_html_serialize_str_callback(const lxb_char_t *data, size_t len, void *ctx)
{
    lxb_char_t *ret;
    lxb_html_serialize_ctx_t *s_ctx = ctx;

    ret = lexbor_str_append(s_ctx->str, s_ctx->mraw, data, len);
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_deep_cb(lxb_dom_node_t *node,
                           lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;

    node = node->first_child;

    while (node != NULL) {
        status = lxb_html_serialize_node_cb(node, cb, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        node = node->next;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_deep_str(lxb_dom_node_t *node, lexbor_str_t *str)
{
    lxb_html_serialize_ctx_t ctx;

    if (str->data == NULL) {
        lexbor_str_init(str, node->owner_document->text, 1024);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    ctx.str = str;
    ctx.mraw = node->owner_document->text;

    return lxb_html_serialize_deep_cb(node,
                                      lxb_html_serialize_str_callback, &ctx);
}

static lxb_status_t
lxb_html_serialize_node_cb(lxb_dom_node_t *node,
                           lxb_html_serialize_cb_f cb, void *ctx)
{
    bool skip_it;
    lxb_status_t status;
    lxb_dom_node_t *root = node;

    while (node != NULL) {
        status = lxb_html_serialize_cb(node, cb, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        if (lxb_html_tree_node_is(node, LXB_TAG_TEMPLATE)) {
            lxb_html_template_element_t *temp;

            temp = lxb_html_interface_template(node);

            if (temp->content != NULL) {
                if (temp->content->node.first_child != NULL)
                {
                    status = lxb_html_serialize_deep_cb(&temp->content->node,
                                                        cb, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }
            }
        }

        skip_it = lxb_html_node_is_void(node);

        if (skip_it == false && node->first_child != NULL) {
            node = node->first_child;
        }
        else {
            while(node != root && node->next == NULL)
            {
                if (node->type == LXB_DOM_NODE_TYPE_ELEMENT
                    && lxb_html_node_is_void(node) == false)
                {
                    status = lxb_html_serialize_element_closed_cb(lxb_dom_interface_element(node),
                                                                  cb, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }

                node = node->parent;
            }

            if (node->type == LXB_DOM_NODE_TYPE_ELEMENT
                && lxb_html_node_is_void(node) == false)
            {
                status = lxb_html_serialize_element_closed_cb(lxb_dom_interface_element(node),
                                                              cb, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }
            }

            if (node == root) {
                break;
            }

            node = node->next;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_element_cb(lxb_dom_element_t *element,
                              lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    const lxb_char_t *tag_name;
    size_t len = 0;

    lxb_dom_attr_t *attr;

    tag_name = lxb_dom_element_qualified_name(element, &len);
    if (tag_name == NULL) {
        return LXB_STATUS_ERROR;
    }

    lxb_html_serialize_send("<", 1, ctx);
    lxb_html_serialize_send(tag_name, len, ctx);

    if (element->is_value != NULL && element->is_value->data != NULL) {
        attr = lxb_dom_element_attr_is_exist(element,
                                             (const lxb_char_t *) "is", 2);
        if (attr == NULL) {
            lxb_html_serialize_send(" is=\"", 5, ctx);

            status = lxb_html_serialize_send_escaping_attribute_string(element->is_value->data,
                                                                       element->is_value->length,
                                                                       cb, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            lxb_html_serialize_send("\"", 1, ctx);
        }
    }

    attr = element->first_attr;

    while (attr != NULL) {
        lxb_html_serialize_send(" ", 1, ctx);

        status = lxb_html_serialize_attribute_cb(attr, cb, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        attr = attr->next;
    }

    lxb_html_serialize_send(">", 1, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_element_closed_cb(lxb_dom_element_t *element,
                                     lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    const lxb_char_t *tag_name;
    size_t len = 0;

    tag_name = lxb_dom_element_qualified_name(element, &len);
    if (tag_name == NULL) {
        return LXB_STATUS_ERROR;
    }

    lxb_html_serialize_send("</", 2, ctx);
    lxb_html_serialize_send(tag_name, len, ctx);
    lxb_html_serialize_send(">", 1, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_text_cb(lxb_dom_text_t *text,
                           lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;

    lxb_dom_node_t *node = lxb_dom_interface_node(text);
    lxb_dom_document_t *doc = node->owner_document;
    lexbor_str_t *data = &text->char_data.data;

    if (node->parent != NULL) {
        switch (node->parent->local_name) {
            case LXB_TAG_STYLE:
            case LXB_TAG_SCRIPT:
            case LXB_TAG_XMP:
            case LXB_TAG_IFRAME:
            case LXB_TAG_NOEMBED:
            case LXB_TAG_NOFRAMES:
            case LXB_TAG_PLAINTEXT:
                lxb_html_serialize_send(data->data, data->length, ctx);

                return LXB_STATUS_OK;

            case LXB_TAG_NOSCRIPT:
                if (doc->scripting) {
                    lxb_html_serialize_send(data->data, data->length, ctx);

                    return LXB_STATUS_OK;
                }

                break;

            default:
                break;
        }
    }

    return lxb_html_serialize_send_escaping_string(data->data, data->length,
                                                   cb, ctx);
}

static lxb_status_t
lxb_html_serialize_comment_cb(lxb_dom_comment_t *comment,
                              lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    lexbor_str_t *data = &comment->char_data.data;

    lxb_html_serialize_send("<!--", 4, ctx);
    lxb_html_serialize_send(data->data, data->length, ctx);
    lxb_html_serialize_send("-->", 3, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_processing_instruction_cb(lxb_dom_processing_instruction_t *pi,
                                             lxb_html_serialize_cb_f cb,
                                             void *ctx)
{
    lxb_status_t status;
    lexbor_str_t *data = &pi->char_data.data;

    lxb_html_serialize_send("<?", 2, ctx);
    lxb_html_serialize_send(pi->target.data, pi->target.length, ctx);
    lxb_html_serialize_send(" ", 1, ctx);
    lxb_html_serialize_send(data->data, data->length, ctx);
    lxb_html_serialize_send("?>", 2, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_document_type_cb(lxb_dom_document_type_t *doctype,
                                    lxb_html_serialize_cb_f cb, void *ctx)
{
    size_t length;
    const lxb_char_t *name;
    lxb_status_t status;

    lxb_html_serialize_send("<!DOCTYPE", 9, ctx);
    lxb_html_serialize_send(" ", 1, ctx);

    name = lxb_dom_document_type_name(doctype, &length);

    if (length != 0) {
        lxb_html_serialize_send(name, length, ctx);
    }

    lxb_html_serialize_send(">", 1, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_document_type_full_cb(lxb_dom_document_type_t *doctype,
                                         lxb_html_serialize_opt_t opt,
                                         lxb_html_serialize_cb_f cb, void *ctx)
{
    bool have_pub, have_sys;
    size_t length;
    const lxb_char_t *name;
    lxb_status_t status;

    lxb_html_serialize_send("<!DOCTYPE", 9, ctx);
    lxb_html_serialize_send(" ", 1, ctx);

    name = lxb_dom_document_type_name(doctype, &length);

    if (length != 0) {
        lxb_html_serialize_send(name, length, ctx);
    }

    have_pub = doctype->public_id.length != 0;
    have_sys = doctype->system_id.length != 0;

    if (opt & LXB_HTML_SERIALIZE_OPT_HTML5TEST) {
        /*
         * html5lib-tests format: when either PUBLIC or SYSTEM identifier
         * is present, emit both slots. A missing identifier is shown as "".
         */
        if (have_pub || have_sys) {
            lxb_html_serialize_send(" \"", 2, ctx);

            if (have_pub) {
                lxb_html_serialize_send(doctype->public_id.data,
                                        doctype->public_id.length, ctx);
            }

            lxb_html_serialize_send("\" \"", 3, ctx);

            if (have_sys) {
                lxb_html_serialize_send(doctype->system_id.data,
                                        doctype->system_id.length, ctx);
            }

            lxb_html_serialize_send("\"", 1, ctx);
        }
    }
    else {
        if (have_pub) {
            lxb_html_serialize_send(" PUBLIC \"", 9, ctx);

            lxb_html_serialize_send(doctype->public_id.data,
                                    doctype->public_id.length, ctx);

            lxb_html_serialize_send("\"", 1, ctx);
        }

        if (have_sys) {
            if (!have_pub) {
                lxb_html_serialize_send(" SYSTEM", 7, ctx);
            }

            lxb_html_serialize_send(" \"", 2, ctx);

            lxb_html_serialize_send(doctype->system_id.data,
                                    doctype->system_id.length, ctx);

            lxb_html_serialize_send("\"", 1, ctx);
        }
    }

    lxb_html_serialize_send(">", 1, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_document_cb(lxb_dom_document_t *document,
                               lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;

    lxb_html_serialize_send("<#document>", 11, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_send_escaping_attribute_string(const lxb_char_t *data,
                                                  size_t len,
                                                  lxb_html_serialize_cb_f cb,
                                                  void *ctx)
{
    lxb_status_t status;
    const lxb_char_t *pos = data;
    const lxb_char_t *end = data + len;

    while (data != end) {
        switch (*data) {
            /* U+0026 AMPERSAND (&) */
            case 0x26:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&amp;", 5, ctx);

                data++;
                pos = data;

                break;

            /* {0xC2, 0xA0} NO-BREAK SPACE */
            case 0xC2:
                data += 1;
                if (data == end) {
                    break;
                }

                if (*data != 0xA0) {
                    continue;
                }

                data -= 1;

                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&nbsp;", 6, ctx);

                data += 2;
                pos = data;

                break;

            /* U+003C LESS-THAN SIGN (<) */
            case 0x3C:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&lt;", 4, ctx);

                data++;
                pos = data;

                break;

            /* U+003E GREATER-THAN SIGN (>) */
            case 0x3E:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&gt;", 4, ctx);

                data++;
                pos = data;

                break;

            /* U+0022 QUOTATION MARK (") */
            case 0x22:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&quot;", 6, ctx);

                data++;
                pos = data;

                break;

            default:
                data++;

                break;
        }
    }

    if (pos != data) {
        lxb_html_serialize_send(pos, (data - pos), ctx);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_send_escaping_string(const lxb_char_t *data, size_t len,
                                        lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    const lxb_char_t *pos = data;
    const lxb_char_t *end = data + len;

    while (data != end) {
        switch (*data) {
            /* U+0026 AMPERSAND (&) */
            case 0x26:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&amp;", 5, ctx);

                data++;
                pos = data;

                break;

            /* {0xC2, 0xA0} NO-BREAK SPACE */
            case 0xC2:
                data += 1;
                if (data == end) {
                    break;
                }

                if (*data != 0xA0) {
                    continue;
                }

                data -= 1;

                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&nbsp;", 6, ctx);

                data += 2;
                pos = data;

                break;

            /* U+003C LESS-THAN SIGN (<) */
            case 0x3C:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&lt;", 4, ctx);

                data++;
                pos = data;

                break;

            /* U+003E GREATER-THAN SIGN (>) */
            case 0x3E:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&gt;", 4, ctx);

                data++;
                pos = data;

                break;

            default:
                data++;

                break;
        }
    }

    if (pos != data) {
        lxb_html_serialize_send(pos, (data - pos), ctx);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_attribute_cb(lxb_dom_attr_t *attr,
                                lxb_html_serialize_cb_f cb, void *ctx)
{
    size_t length;
    lxb_status_t status;
    const lxb_char_t *str;
    const lxb_dom_attr_data_t *data;

    data = lxb_dom_attr_data_by_id(attr->node.owner_document->attrs,
                                   attr->node.local_name);
    if (data == NULL) {
        return LXB_STATUS_ERROR;
    }

    if (attr->node.ns == LXB_NS__UNDEF) {
        lxb_html_serialize_send(lexbor_hash_entry_str(&data->entry),
                                data->entry.length, ctx);
        goto value;
    }

    if (attr->node.ns == LXB_NS_XML) {
        lxb_html_serialize_send((const lxb_char_t *) "xml:", 4, ctx);
        lxb_html_serialize_send(lexbor_hash_entry_str(&data->entry),
                                data->entry.length, ctx);

        goto value;
    }

    if (attr->node.ns == LXB_NS_XMLNS)
    {
        if (data->entry.length == 5
            && lexbor_str_data_cmp(lexbor_hash_entry_str(&data->entry),
                                   (const lxb_char_t *) "xmlns"))
        {
            lxb_html_serialize_send((const lxb_char_t *) "xmlns", 5, ctx);
        }
        else {
            lxb_html_serialize_send((const lxb_char_t *) "xmlns:", 6, ctx);
            lxb_html_serialize_send(lexbor_hash_entry_str(&data->entry),
                                    data->entry.length, ctx);
        }

        goto value;
    }

    if (attr->node.ns == LXB_NS_XLINK) {
        lxb_html_serialize_send((const lxb_char_t *) "xlink:", 6, ctx);
        lxb_html_serialize_send(lexbor_hash_entry_str(&data->entry),
                                data->entry.length, ctx);

        goto value;
    }

    str = lxb_dom_attr_qualified_name(attr, &length);
    if (str == NULL) {
        return LXB_STATUS_ERROR;
    }

    lxb_html_serialize_send(str, length, ctx);

value:

    if (attr->value == NULL) {
        lxb_html_serialize_send("=\"\"", 3, ctx);
        return LXB_STATUS_OK;
    }

    lxb_html_serialize_send("=\"", 2, ctx);

    status = lxb_html_serialize_send_escaping_attribute_string(attr->value->data,
                                                               attr->value->length,
                                                               cb, ctx);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    lxb_html_serialize_send("\"", 1, ctx);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_pretty_cb(lxb_dom_node_t *node,
                             lxb_html_serialize_opt_t opt, size_t indent,
                             lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;

    if (opt & LXB_HTML_SERIALIZE_OPT_HTML5TEST) {
        opt |= LXB_HTML_SERIALIZE_OPT_WITHOUT_CLOSING
               | LXB_HTML_SERIALIZE_OPT_TAG_WITH_NS
               | LXB_HTML_SERIALIZE_OPT_WITHOUT_TEXT_INDENT
               | LXB_HTML_SERIALIZE_OPT_FULL_DOCTYPE
               | LXB_HTML_SERIALIZE_OPT_RAW;
    }

    switch (node->type) {
        case LXB_DOM_NODE_TYPE_ELEMENT:
            lxb_html_serialize_send_indent(indent, ctx);

            status = lxb_html_serialize_pretty_element_cb(lxb_dom_interface_element(node),
                                                          opt, indent, cb, ctx);

            break;

        case LXB_DOM_NODE_TYPE_TEXT:
            return lxb_html_serialize_pretty_text_cb(lxb_dom_interface_text(node),
                                                     opt, indent, cb, ctx);

        case LXB_DOM_NODE_TYPE_COMMENT: {
            bool with_indent;

            if (opt & LXB_HTML_SERIALIZE_OPT_SKIP_COMMENT) {
                return LXB_STATUS_OK;
            }

            with_indent = (opt & LXB_HTML_SERIALIZE_OPT_WITHOUT_TEXT_INDENT) == 0;

            status = lxb_html_serialize_pretty_comment_cb(lxb_dom_interface_comment(node),
                                                          indent, with_indent, cb, ctx);

            break;
        }

        case LXB_DOM_NODE_TYPE_PROCESSING_INSTRUCTION:
            lxb_html_serialize_send_indent(indent, ctx);

            status = lxb_html_serialize_processing_instruction_cb(lxb_dom_interface_processing_instruction(node),
                                                                  cb, ctx);

            break;

        case LXB_DOM_NODE_TYPE_DOCUMENT_TYPE:
            lxb_html_serialize_send_indent(indent, ctx);

            if (opt & LXB_HTML_SERIALIZE_OPT_FULL_DOCTYPE) {
                status = lxb_html_serialize_document_type_full_cb(lxb_dom_interface_document_type(node),
                                                                  opt, cb, ctx);
            }
            else {
                status = lxb_html_serialize_document_type_cb(lxb_dom_interface_document_type(node),
                                                             cb, ctx);
            }

            break;

        case LXB_DOM_NODE_TYPE_DOCUMENT:
            lxb_html_serialize_send_indent(indent, ctx);

            status = lxb_html_serialize_pretty_document_cb(lxb_dom_interface_document(node),
                                                           cb, ctx);

            break;

        default:
            return LXB_STATUS_ERROR;
    }

    if (status != LXB_STATUS_OK) {
        return status;
    }

    lxb_html_serialize_send("\n", 1, ctx);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_pretty_str(lxb_dom_node_t *node,
                              lxb_html_serialize_opt_t opt, size_t indent,
                              lexbor_str_t *str)
{
    lxb_html_serialize_ctx_t ctx;

    if (str->data == NULL) {
        lexbor_str_init(str, node->owner_document->text, 1024);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    ctx.str = str;
    ctx.mraw = node->owner_document->text;

    return lxb_html_serialize_pretty_cb(node, opt, indent,
                                        lxb_html_serialize_str_callback, &ctx);
}

lxb_status_t
lxb_html_serialize_pretty_deep_cb(lxb_dom_node_t *node,
                                  lxb_html_serialize_opt_t opt, size_t indent,
                                  lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;

    if (opt & LXB_HTML_SERIALIZE_OPT_HTML5TEST) {
        opt |= LXB_HTML_SERIALIZE_OPT_WITHOUT_CLOSING
               | LXB_HTML_SERIALIZE_OPT_TAG_WITH_NS
               | LXB_HTML_SERIALIZE_OPT_WITHOUT_TEXT_INDENT
               | LXB_HTML_SERIALIZE_OPT_FULL_DOCTYPE
               | LXB_HTML_SERIALIZE_OPT_RAW;
    }

    node = node->first_child;

    while (node != NULL) {
        status = lxb_html_serialize_pretty_node_cb(node, opt, indent, cb, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        node = node->next;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_pretty_deep_str(lxb_dom_node_t *node,
                                   lxb_html_serialize_opt_t opt, size_t indent,
                                   lexbor_str_t *str)
{
    lxb_html_serialize_ctx_t ctx;

    if (str->data == NULL) {
        lexbor_str_init(str, node->owner_document->text, 1024);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    ctx.str = str;
    ctx.mraw = node->owner_document->text;

    return lxb_html_serialize_pretty_deep_cb(node, opt, indent,
                                             lxb_html_serialize_str_callback,
                                             &ctx);
}

static lxb_status_t
lxb_html_serialize_pretty_node_cb(lxb_dom_node_t *node,
                                  lxb_html_serialize_opt_t opt, size_t deep,
                                  lxb_html_serialize_cb_f cb, void *ctx)
{
    bool skip_it;
    lxb_status_t status;
    lxb_dom_node_t *root = node;

    while (node != NULL) {
        status = lxb_html_serialize_pretty_cb(node, opt, deep, cb, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        if (lxb_html_tree_node_is(node, LXB_TAG_TEMPLATE)) {
            lxb_html_template_element_t *temp;

            temp = lxb_html_interface_template(node);

            if (opt & LXB_HTML_SERIALIZE_OPT_HTML5TEST) {
                lxb_html_serialize_send_indent((deep + 1), ctx);
                lxb_html_serialize_send("content", 7, ctx);
                lxb_html_serialize_send("\n", 1, ctx);
            }

            if (temp->content != NULL) {
                if (temp->content->node.first_child != NULL)
                {
                    if ((opt & LXB_HTML_SERIALIZE_OPT_HTML5TEST) == 0) {
                        lxb_html_serialize_send_indent((deep + 1), ctx);
                        lxb_html_serialize_send("#document-fragment", 18, ctx);
                        lxb_html_serialize_send("\n", 1, ctx);
                    }

                    status = lxb_html_serialize_pretty_deep_cb(&temp->content->node,
                                                               opt, (deep + 2),
                                                               cb, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }
            }
        }

        skip_it = lxb_html_node_is_void(node);

        if (skip_it == false && node->first_child != NULL) {
            deep++;

            node = node->first_child;
        }
        else {
            while(node != root && node->next == NULL)
            {
                if (node->type == LXB_DOM_NODE_TYPE_ELEMENT
                    && lxb_html_node_is_void(node) == false)
                {
                    if ((opt & LXB_HTML_SERIALIZE_OPT_WITHOUT_CLOSING) == 0) {
                        lxb_html_serialize_send_indent(deep, ctx);

                        status = lxb_html_serialize_element_closed_cb(lxb_dom_interface_element(node),
                                                                      cb, ctx);
                        if (status != LXB_STATUS_OK) {
                            return status;
                        }

                        lxb_html_serialize_send("\n", 1, ctx);
                    }
                }

                deep--;

                node = node->parent;
            }

            if (node->type == LXB_DOM_NODE_TYPE_ELEMENT
                && lxb_html_node_is_void(node) == false)
            {
                if ((opt & LXB_HTML_SERIALIZE_OPT_WITHOUT_CLOSING) == 0) {
                    lxb_html_serialize_send_indent(deep, ctx);

                    status = lxb_html_serialize_element_closed_cb(lxb_dom_interface_element(node),
                                                                  cb, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }

                    lxb_html_serialize_send("\n", 1, ctx);
                }
            }

            if (node == root) {
                break;
            }

            node = node->next;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_pretty_element_cb(lxb_dom_element_t *element,
                                     lxb_html_serialize_opt_t opt, size_t indent,
                                     lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    const lxb_char_t *tag_name;
    size_t len = 0;

    lxb_dom_attr_t *attr;
    lxb_dom_node_t *node = lxb_dom_interface_node(element);

    tag_name = lxb_dom_element_qualified_name(element, &len);
    if (tag_name == NULL) {
        return LXB_STATUS_ERROR;
    }

    lxb_html_serialize_send("<", 1, ctx);

    if (element->node.ns != LXB_NS_HTML
        && opt & LXB_HTML_SERIALIZE_OPT_TAG_WITH_NS)
    {
        const lxb_ns_prefix_data_t *data = NULL;

        if (element->node.prefix != LXB_NS__UNDEF) {
            data = lxb_ns_prefix_data_by_id(node->owner_document->prefix,
                                            element->node.prefix);
        }
        else if (element->node.ns < LXB_NS__LAST_ENTRY) {
            data = lxb_ns_prefix_data_by_id(node->owner_document->prefix,
                                             element->node.ns);
        }

        if (data != NULL) {
            lxb_html_serialize_send(lexbor_hash_entry_str(&data->entry),
                                    data->entry.length, ctx);

            if (opt & LXB_HTML_SERIALIZE_OPT_HTML5TEST) {
                lxb_html_serialize_send(" ", 1, ctx);
            }
            else {
                lxb_html_serialize_send(":", 1, ctx);
            }
        }
    }

    lxb_html_serialize_send(tag_name, len, ctx);

    if (element->is_value != NULL && element->is_value->data != NULL) {
        attr = lxb_dom_element_attr_is_exist(element,
                                             (const lxb_char_t *) "is", 2);
        if (attr == NULL) {
            lxb_html_serialize_send(" is=\"", 5, ctx);

            if (opt & LXB_HTML_SERIALIZE_OPT_RAW) {
                lxb_html_serialize_send(element->is_value->data,
                                        element->is_value->length, ctx);
            }
            else {
                status = lxb_html_serialize_send_escaping_attribute_string(element->is_value->data,
                                                                           element->is_value->length,
                                                                           cb, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }
            }

            lxb_html_serialize_send("\"", 1, ctx);
        }
    }

    if (opt & LXB_HTML_SERIALIZE_OPT_HTML5TEST) {
        lxb_html_serialize_send(">", 1, ctx);

        status = lxb_html_serialize_pretty_attributes_sorted(element, opt,
                                                             indent, cb, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }
    else {
        attr = element->first_attr;

        while (attr != NULL) {
            lxb_html_serialize_send(" ", 1, ctx);

            status = lxb_html_serialize_pretty_attribute_cb(attr, opt,
                                                            (opt & LXB_HTML_SERIALIZE_OPT_RAW),
                                                            cb, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            attr = attr->next;
        }

        lxb_html_serialize_send(">", 1, ctx);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_pretty_attribute_cb(lxb_dom_attr_t *attr,
                                       lxb_html_serialize_opt_t opt, bool has_raw,
                                       lxb_html_serialize_cb_f cb, void *ctx)
{
    size_t length;
    lxb_status_t status;
    const lxb_char_t *str;
    const lxb_dom_attr_data_t *data;
    lxb_char_t spliter;

    if (opt & LXB_HTML_SERIALIZE_OPT_HTML5TEST) {
        spliter = ' ';
    }
    else {
        spliter = ':';
    }

    data = lxb_dom_attr_data_by_id(attr->node.owner_document->attrs,
                                   attr->node.local_name);
    if (data == NULL) {
        return LXB_STATUS_ERROR;
    }

    if (attr->node.ns == LXB_NS__UNDEF) {
        lxb_html_serialize_send(lexbor_hash_entry_str(&data->entry),
                                data->entry.length, ctx);
        goto value;
    }

    if (attr->node.ns == LXB_NS_XML) {
        lxb_html_serialize_send((const lxb_char_t *) "xml", 3, ctx);
        lxb_html_serialize_send(&spliter, 1, ctx);
        lxb_html_serialize_send(lexbor_hash_entry_str(&data->entry),
                                data->entry.length, ctx);

        goto value;
    }

    if (attr->node.ns == LXB_NS_XMLNS)
    {
        if (data->entry.length == 5
            && lexbor_str_data_cmp(lexbor_hash_entry_str(&data->entry),
                                   (const lxb_char_t *) "xmlns"))
        {
            lxb_html_serialize_send((const lxb_char_t *) "xmlns", 5, ctx);
        }
        else {
            lxb_html_serialize_send((const lxb_char_t *) "xmlns", 5, ctx);
            lxb_html_serialize_send(&spliter, 1, ctx);
            lxb_html_serialize_send(lexbor_hash_entry_str(&data->entry),
                                    data->entry.length, ctx);
        }

        goto value;
    }

    if (attr->node.ns == LXB_NS_XLINK) {
        lxb_html_serialize_send((const lxb_char_t *) "xlink", 5, ctx);
        lxb_html_serialize_send(&spliter, 1, ctx);
        lxb_html_serialize_send(lexbor_hash_entry_str(&data->entry),
                                data->entry.length, ctx);

        goto value;
    }

    str = lxb_dom_attr_qualified_name(attr, &length);
    if (str == NULL) {
        return LXB_STATUS_ERROR;
    }

    lxb_html_serialize_send(str, length, ctx);

value:

    if (attr->value == NULL) {
        lxb_html_serialize_send("=\"\"", 3, ctx);
        return LXB_STATUS_OK;
    }

    lxb_html_serialize_send("=\"", 2, ctx);

    if (has_raw) {
        lxb_html_serialize_send(attr->value->data, attr->value->length, ctx);
    }
    else {
        status = lxb_html_serialize_send_escaping_attribute_string(attr->value->data,
                                                                   attr->value->length,
                                                                   cb, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    lxb_html_serialize_send("\"", 1, ctx);

    return LXB_STATUS_OK;
}

static size_t
lxb_html_serialize_attr_name_build(const lxb_dom_attr_t *attr,
                                   lxb_char_t *buf, size_t cap)
{
    size_t length, xmlns_len, pos;
    const lxb_char_t *str;
    const lexbor_str_t *ns;
    const lxb_dom_attr_data_t *data;

    static const lexbor_str_t str_xml = lexbor_str("xml ");
    static const lexbor_str_t str_xmlns = lexbor_str("xmlns ");
    static const lexbor_str_t str_xlink = lexbor_str("xlink ");

    data = lxb_dom_attr_data_by_id(attr->node.owner_document->attrs,
                                   attr->node.local_name);
    if (data == NULL) {
        return 0;
    }

    pos = 0;
    str = lexbor_hash_entry_str(&data->entry);
    length = data->entry.length;

    switch (attr->node.ns) {
        case LXB_NS_XML:
            if (str_xml.length + length > cap) {
                return 0;
            }

            ns = &str_xml;
            goto done;

        case LXB_NS_XMLNS:
            xmlns_len = str_xmlns.length - 1;

            if (length == xmlns_len
                && lexbor_str_data_ncmp(str, str_xmlns.data, xmlns_len))
            {
                if (xmlns_len > cap) {
                    return 0;
                }

                memcpy(buf, str_xmlns.data, xmlns_len);
                return xmlns_len;
            }

            if (str_xmlns.length + length > cap) {
                return 0;
            }

            ns = &str_xmlns;
            goto done;

        case LXB_NS_XLINK:
            if (str_xlink.length + length > cap) {
                return 0;
            }

            ns = &str_xlink;
            goto done;

        case LXB_NS__UNDEF:
            if (length > cap) {
                return 0;
            }

            memcpy(buf, str, length);
            return length;

        default:
            if (attr->qualified_name != 0) {
                data = lxb_dom_attr_data_by_id(attr->node.owner_document->attrs,
                                               attr->qualified_name);
                if (data == NULL) {
                    return 0;
                }

                str = lexbor_hash_entry_str(&data->entry);
                length = data->entry.length;
            }

            if (length > cap) {
                return 0;
            }

            memcpy(buf, str, length);
            return length;
    }

done:

    memcpy(buf, ns->data, ns->length);
    pos = ns->length;

    memcpy(buf + pos, str, length);
    pos += length;

    return pos;
}

static size_t
lxb_html_serialize_attr_name_size(const lxb_dom_attr_t *attr)
{
    size_t length;
    const lxb_dom_attr_data_t *data;

    static const lexbor_str_t str_xml = lexbor_str("xml ");
    static const lexbor_str_t str_xmlns = lexbor_str("xmlns ");
    static const lexbor_str_t str_xlink = lexbor_str("xlink ");

    data = lxb_dom_attr_data_by_id(attr->node.owner_document->attrs,
                                   attr->node.local_name);
    if (data == NULL) {
        return 0;
    }

    length = data->entry.length;

    switch (attr->node.ns) {
        case LXB_NS_XML:
            return str_xml.length + length;

        case LXB_NS_XMLNS:
            if (length == str_xmlns.length - 1
                && lexbor_str_data_ncmp(lexbor_hash_entry_str(&data->entry),
                                        str_xmlns.data, str_xmlns.length - 1))
            {
                return str_xmlns.length - 1;
            }

            return str_xmlns.length + length;

        case LXB_NS_XLINK:
            return str_xlink.length + length;

        case LXB_NS__UNDEF:
            return length;

        default:
            if (attr->qualified_name != 0) {
                data = lxb_dom_attr_data_by_id(attr->node.owner_document->attrs,
                                               attr->qualified_name);
                if (data == NULL) {
                    return 0;
                }

                length = data->entry.length;
            }

            return length;
    }
}

static int
lxb_html_serialize_attr_entry_cmp(const lxb_html_serialize_attr_entry_t *a,
                                  const lxb_html_serialize_attr_entry_t *b,
                                  const lxb_char_t *names)
{
    int c;
    size_t min;

    min = (a->length < b->length) ? a->length : b->length;

    c = memcmp(names + a->offset, names + b->offset, min);
    if (c != 0) {
        return c;
    }

    if (a->length < b->length) return -1;
    if (a->length > b->length) return 1;
    return 0;
}

static void
lxb_html_serialize_attr_sort(lxb_html_serialize_attr_entry_t *entries,
                             size_t n, const lxb_char_t *names)
{
    size_t i, j;
    lxb_html_serialize_attr_entry_t cur;

    for (i = 1; i < n; i++) {
        cur = entries[i];
        j = i;

        while (j > 0
               && lxb_html_serialize_attr_entry_cmp(&entries[j - 1], &cur,
                                                    names) > 0)
        {
            entries[j] = entries[j - 1];
            j--;
        }

        entries[j] = cur;
    }
}

static lxb_status_t
lxb_html_serialize_pretty_attributes_sorted(lxb_dom_element_t *element,
                                            lxb_html_serialize_opt_t opt,
                                            size_t indent,
                                            lxb_html_serialize_cb_f cb,
                                            void *ctx)
{
    size_t i, k, off, len, count, total;
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    lxb_html_serialize_attr_entry_t *entries;
    lxb_char_t *names;
    lxb_char_t stack_names[256];
    lxb_html_serialize_attr_entry_t stack_entries[16];

    count = 0;
    total = 0;
    entries = stack_entries;
    names = stack_names;

    for (attr = element->first_attr; attr != NULL; attr = attr->next) {
        count += 1;
        total += lxb_html_serialize_attr_name_size(attr);
    }

    if (count == 0) {
        return LXB_STATUS_OK;
    }

    if (count > sizeof(stack_entries) / sizeof(stack_entries[0])) {
        entries = lexbor_malloc(count * sizeof(lxb_html_serialize_attr_entry_t));
        if (entries == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    if (total > sizeof(stack_names)) {
        names = lexbor_malloc(total);
        if (names == NULL) {
            if (entries != stack_entries) {
                lexbor_free(entries);
            }

            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    i = 0;
    off = 0;
    for (attr = element->first_attr; attr != NULL; attr = attr->next) {
        len = lxb_html_serialize_attr_name_build(attr, names + off,
                                                 total - off);
        entries[i].attr = attr;
        entries[i].offset = off;
        entries[i].length = len;

        off += len;
        i += 1;
    }

    lxb_html_serialize_attr_sort(entries, count, names);

    status = LXB_STATUS_OK;

    for (i = 0; i < count; i++) {
        status = cb((const lxb_char_t *) "\n", 1, ctx);
        if (status != LXB_STATUS_OK) {
            goto done;
        }

        for (k = 0; k < indent + 1; k++) {
            status = cb((const lxb_char_t *) "  ", 2, ctx);
            if (status != LXB_STATUS_OK) {
                goto done;
            }
        }

        status = lxb_html_serialize_pretty_attribute_cb(entries[i].attr, opt,
                                            (opt & LXB_HTML_SERIALIZE_OPT_RAW),
                                            cb, ctx);
        if (status != LXB_STATUS_OK) {
            goto done;
        }
    }

done:

    if (names != stack_names) {
        lexbor_free(names);
    }

    if (entries != stack_entries) {
        lexbor_free(entries);
    }

    return status;
}

static lxb_status_t
lxb_html_serialize_pretty_text_cb(lxb_dom_text_t *text,
                                  lxb_html_serialize_opt_t opt, size_t indent,
                                  lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    lxb_dom_node_t *node = lxb_dom_interface_node(text);
    lxb_dom_document_t *doc = node->owner_document;
    lexbor_str_t *data = &text->char_data.data;

    bool with_indent = (opt & LXB_HTML_SERIALIZE_OPT_WITHOUT_TEXT_INDENT) == 0;

    if (opt & LXB_HTML_SERIALIZE_OPT_SKIP_WS_NODES) {
        const lxb_char_t *pos = data->data;
        const lxb_char_t *end = pos + data->length;

        while (pos != end) {
            if (lexbor_tokenizer_chars_map[ *pos ]
                != LEXBOR_STR_RES_MAP_CHAR_WHITESPACE)
            {
                break;
            }

            pos++;
        }

        if (pos >= end) {
            return LXB_STATUS_OK;
        }
    }

    if (node->parent != NULL) {
        switch (node->parent->local_name) {
            case LXB_TAG_STYLE:
            case LXB_TAG_SCRIPT:
            case LXB_TAG_XMP:
            case LXB_TAG_IFRAME:
            case LXB_TAG_NOEMBED:
            case LXB_TAG_NOFRAMES:
            case LXB_TAG_PLAINTEXT:
                status = lxb_html_serialize_pretty_send_string(data->data,
                                                               data->length,
                                                               indent,
                                                               with_indent,
                                                               cb, ctx);
                goto end;

            case LXB_TAG_NOSCRIPT:
                if (doc->scripting) {
                    status = lxb_html_serialize_pretty_send_string(data->data,
                                                                   data->length,
                                                                   indent,
                                                                   with_indent,
                                                                   cb, ctx);
                    goto end;
                }

                break;

            default:
                break;
        }
    }

    if (opt & LXB_HTML_SERIALIZE_OPT_RAW) {
        status = lxb_html_serialize_pretty_send_string(data->data, data->length,
                                                       indent, with_indent,
                                                       cb, ctx);
    }
    else {
        status = lxb_html_serialize_pretty_send_escaping_string(data->data,
                                                                data->length,
                                                                indent,
                                                                with_indent,
                                                                cb, ctx);
    }

end:

    if (status != LXB_STATUS_OK) {
        return status;
    }

    lxb_html_serialize_send("\n", 1, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_pretty_comment_cb(lxb_dom_comment_t *comment,
                                     size_t indent, bool with_indent,
                                     lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;

    lxb_html_serialize_send_indent(indent, ctx);
    lxb_html_serialize_send("<!-- ", 5, ctx);

    if (with_indent) {
        const lxb_char_t *data = comment->char_data.data.data;
        const lxb_char_t *pos = data;
        const lxb_char_t *end = pos + comment->char_data.data.length;

        while (data != end) {
            /*
             * U+000A LINE FEED (LF)
             * U+000D CARRIAGE RETURN (CR)
             */
            if (*data == 0x0A || *data == 0x0D) {
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send(data, 1, ctx);
                lxb_html_serialize_send_indent(indent, ctx);

                data++;
                pos = data;
            }
            else {
                data++;
            }
        }

        if (pos != data) {
            lxb_html_serialize_send(pos, (data - pos), ctx);
        }
    }
    else {
        lxb_html_serialize_send(comment->char_data.data.data,
                                comment->char_data.data.length, ctx);
    }

    lxb_html_serialize_send(" -->", 4, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_pretty_document_cb(lxb_dom_document_t *document,
                                      lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;

    lxb_html_serialize_send("#document", 9, ctx);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_tree_cb(lxb_dom_node_t *node,
                           lxb_html_serialize_cb_f cb, void *ctx)
{
    /* For a document we must serialize all children without document node. */
    if (node->local_name == LXB_TAG__DOCUMENT) {
        node = node->first_child;

        while (node != NULL) {
            lxb_status_t status = lxb_html_serialize_node_cb(node, cb, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            node = node->next;
        }

        return LXB_STATUS_OK;
    }

    return lxb_html_serialize_node_cb(node, cb, ctx);
}

lxb_status_t
lxb_html_serialize_tree_str(lxb_dom_node_t *node, lexbor_str_t *str)
{
    lxb_html_serialize_ctx_t ctx;

    if (str->data == NULL) {
        lexbor_str_init(str, node->owner_document->text, 1024);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    ctx.str = str;
    ctx.mraw = node->owner_document->text;

    return lxb_html_serialize_tree_cb(node, lxb_html_serialize_str_callback, &ctx);
}

lxb_status_t
lxb_html_serialize_pretty_tree_cb(lxb_dom_node_t *node,
                                  lxb_html_serialize_opt_t opt, size_t indent,
                                  lxb_html_serialize_cb_f cb, void *ctx)
{
    if (opt & LXB_HTML_SERIALIZE_OPT_HTML5TEST) {
        opt |= LXB_HTML_SERIALIZE_OPT_WITHOUT_CLOSING
               | LXB_HTML_SERIALIZE_OPT_TAG_WITH_NS
               | LXB_HTML_SERIALIZE_OPT_WITHOUT_TEXT_INDENT
               | LXB_HTML_SERIALIZE_OPT_FULL_DOCTYPE
               | LXB_HTML_SERIALIZE_OPT_RAW;
    }

    /* For a document we must serialize all children without document node. */
    if (node->local_name == LXB_TAG__DOCUMENT) {
        node = node->first_child;

        while (node != NULL) {
            lxb_status_t status = lxb_html_serialize_pretty_node_cb(node, opt,
                                                               indent, cb, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            node = node->next;
        }

        return LXB_STATUS_OK;
    }

    return lxb_html_serialize_pretty_node_cb(node, opt, indent, cb, ctx);
}

lxb_status_t
lxb_html_serialize_pretty_tree_str(lxb_dom_node_t *node,
                                   lxb_html_serialize_opt_t opt, size_t indent,
                                   lexbor_str_t *str)
{
    lxb_html_serialize_ctx_t ctx;

    if (str->data == NULL) {
        lexbor_str_init(str, node->owner_document->text, 1024);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    ctx.str = str;
    ctx.mraw = node->owner_document->text;

    return lxb_html_serialize_pretty_tree_cb(node, opt, indent,
                                             lxb_html_serialize_str_callback,
                                             &ctx);
}

static lxb_status_t
lxb_html_serialize_pretty_send_escaping_string(const lxb_char_t *data, size_t len,
                                               size_t indent, bool with_indent,
                                               lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    const lxb_char_t *pos = data;
    const lxb_char_t *end = data + len;

    lxb_html_serialize_send_indent(indent, ctx);
    lxb_html_serialize_send("\"", 1, ctx);

    while (data != end) {
        switch (*data) {
            /* U+0026 AMPERSAND (&) */
            case 0x26:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&amp;", 5, ctx);

                data++;
                pos = data;

                break;

            /* {0xC2, 0xA0} NO-BREAK SPACE */
            case 0xC2:
                data += 1;
                if (data == end) {
                    break;
                }

                if (*data != 0xA0) {
                    continue;
                }

                data -= 1;

                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&nbsp;", 6, ctx);

                data += 2;
                pos = data;

                break;

            /* U+003C LESS-THAN SIGN (<) */
            case 0x3C:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&lt;", 4, ctx);

                data++;
                pos = data;

                break;

            /* U+003E GREATER-THAN SIGN (>) */
            case 0x3E:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&gt;", 4, ctx);

                data++;
                pos = data;

                break;

            /*
             * U+000A LINE FEED (LF)
             * U+000D CARRIAGE RETURN (CR)
             */
            case 0x0A:
            case 0x0D:
                if (with_indent) {
                    if (pos != data) {
                        lxb_html_serialize_send(pos, (data - pos), ctx);
                    }

                    lxb_html_serialize_send("\n", 1, ctx);
                    lxb_html_serialize_send_indent(indent, ctx);

                    data++;
                    pos = data;

                    break;
                }
                /* fall through */

            default:
                data++;

                break;
        }
    }

    if (pos != data) {
        lxb_html_serialize_send(pos, (data - pos), ctx);
    }

    lxb_html_serialize_send("\"", 1, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_pretty_send_string(const lxb_char_t *data, size_t len,
                                      size_t indent, bool with_indent,
                                      lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;

    lxb_html_serialize_send_indent(indent, ctx);
    lxb_html_serialize_send("\"", 1, ctx);

    if (with_indent) {
        const lxb_char_t *pos = data;
        const lxb_char_t *end = data + len;

        while (data != end) {
            /*
             * U+000A LINE FEED (LF)
             * U+000D CARRIAGE RETURN (CR)
             */
            if (*data == 0x0A || *data == 0x0D) {
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send(data, 1, ctx);
                lxb_html_serialize_send_indent(indent, ctx);

                data++;
                pos = data;
            }
            else {
                data++;
            }
        }

        if (pos != data) {
            lxb_html_serialize_send(pos, (data - pos), ctx);
        }
    }
    else {
        lxb_html_serialize_send(data, len, ctx);
    }

    lxb_html_serialize_send("\"", 1, ctx);

    return LXB_STATUS_OK;
}
