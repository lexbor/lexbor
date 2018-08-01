/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include "lexbor/dom/interfaces/text.h"
#include "lexbor/dom/interfaces/comment.h"
#include "lexbor/dom/interfaces/processing_instruction.h"
#include "lexbor/dom/interfaces/document_type.h"

#include "lexbor/html/serialize.h"
#include "lexbor/html/tree.h"
#include "lexbor/html/ns.h"
#include "lexbor/html/interfaces/template_element.h"

#define LEXBOR_TOKENIZER_CHARS_MAP
#include "lexbor/core/str_res.h"


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


static lxb_status_t
lxb_html_serialize_str_callback(const lxb_char_t *data, size_t len, void *ctx);

static lxb_status_t
lxb_html_serialize_node_cb(lxb_dom_node_t *node,
                           lxb_html_serialize_cb_f cb, void *ctx);

lxb_inline lxb_status_t
lxb_html_serialize_node_is_void(lxb_dom_node_t *node);

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
lxb_html_serialize_attribute_cb(lxb_dom_element_attr_t *attr, bool has_raw,
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
lxb_html_serialize_tree_cb(lxb_dom_node_t *node,
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

    return lxb_html_serialize_tree_cb(node,
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

        if (node->tag_id == LXB_HTML_TAG_TEMPLATE) {
            lxb_html_template_element_t *temp;

            temp = lxb_html_interface_template(node);

            if (temp->content != NULL) {
                if (temp->content->node.first_child != NULL)
                {
                    status = lxb_html_serialize_tree_cb(&temp->content->node,
                                                        cb, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }
            }
        }

        skip_it = lxb_html_serialize_node_is_void(node);

        if (skip_it == false && node->first_child != NULL) {
            node = node->first_child;
        }
        else {
            while(node != root && node->next == NULL)
            {
                if (node->type == LXB_DOM_NODE_TYPE_ELEMENT
                    && lxb_html_serialize_node_is_void(node) == false)
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
                && lxb_html_serialize_node_is_void(node) == false)
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

lxb_inline lxb_status_t
lxb_html_serialize_node_is_void(lxb_dom_node_t *node)
{
    if (node->ns != LXB_HTML_NS_HTML) {
        return false;
    }

    switch (node->tag_id) {
        case LXB_HTML_TAG_AREA:
        case LXB_HTML_TAG_BASE:
        case LXB_HTML_TAG_BASEFONT:
        case LXB_HTML_TAG_BGSOUND:
        case LXB_HTML_TAG_BR:
        case LXB_HTML_TAG_COL:
        case LXB_HTML_TAG_EMBED:
        case LXB_HTML_TAG_FRAME:
        case LXB_HTML_TAG_HR:
        case LXB_HTML_TAG_IMG:
        case LXB_HTML_TAG_INPUT:
        case LXB_HTML_TAG_KEYGEN:
        case LXB_HTML_TAG_LINK:
        case LXB_HTML_TAG_META:
        case LXB_HTML_TAG_PARAM:
        case LXB_HTML_TAG_SOURCE:
        case LXB_HTML_TAG_TRACK:
        case LXB_HTML_TAG_WBR:
            return true;

        default:
            return false;
    }

    return false;
}

static lxb_status_t
lxb_html_serialize_element_cb(lxb_dom_element_t *element,
                              lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    const lxb_char_t *tag_name;
    size_t len = 0;

    lxb_dom_element_attr_t *attr;
    lxb_dom_node_t *node = lxb_dom_interface_node(element);
    lxb_dom_document_t *doc = element->node.owner_document;

    if (node->ns == LXB_HTML_NS_HTML
        || node->ns == LXB_HTML_NS_MATH
        || node->ns == LXB_HTML_NS_SVG)
    {
        tag_name = lxb_html_tag_name_by_id(doc->tags, node->tag_id, node->ns,
                                           &len);
    }
    else {
        /* FIXIT: need to fix */
        tag_name = lxb_html_tag_name_by_id(doc->tags, node->tag_id, node->ns,
                                           &len);
    }

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

        status = lxb_html_serialize_attribute_cb(attr, false, cb, ctx);
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

    lxb_dom_node_t *node = lxb_dom_interface_node(element);
    lxb_dom_document_t *doc = element->node.owner_document;

    if (node->ns == LXB_HTML_NS_HTML
        || node->ns == LXB_HTML_NS_MATH
        || node->ns == LXB_HTML_NS_SVG)
    {
        tag_name = lxb_html_tag_name_by_id(doc->tags, node->tag_id, node->ns,
                                           &len);
    }
    else {
        /* FIXIT: need to fix */
        tag_name = lxb_html_tag_name_by_id(doc->tags, node->tag_id, node->ns,
                                           &len);
    }

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

    switch (node->parent->tag_id) {
        case LXB_HTML_TAG_STYLE:
        case LXB_HTML_TAG_SCRIPT:
        case LXB_HTML_TAG_XMP:
        case LXB_HTML_TAG_IFRAME:
        case LXB_HTML_TAG_NOEMBED:
        case LXB_HTML_TAG_NOFRAMES:
        case LXB_HTML_TAG_PLAINTEXT:
            lxb_html_serialize_send(data->data, data->length, ctx);

            return LXB_STATUS_OK;

        case LXB_HTML_TAG_NOSCRIPT:
            if (doc->scripting) {
                lxb_html_serialize_send(data->data, data->length, ctx);

                return LXB_STATUS_OK;
            }

            break;

        default:
            break;
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
    lxb_html_serialize_send(">", 1, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_document_type_cb(lxb_dom_document_type_t *doctype,
                                    lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;

    lxb_html_serialize_send("<!DOCTYPE", 9, ctx);
    lxb_html_serialize_send(" ", 1, ctx);

    if (doctype->name.data != NULL && doctype->name.length != 0) {
        lxb_html_serialize_send(doctype->name.data, doctype->name.length, ctx);
    }

    lxb_html_serialize_send(">", 1, ctx);

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_document_type_full_cb(lxb_dom_document_type_t *doctype,
                                         lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;
    
    lxb_html_serialize_send("<!DOCTYPE", 9, ctx);
    lxb_html_serialize_send(" ", 1, ctx);
    
    if (doctype->name.data != NULL && doctype->name.length != 0) {
        lxb_html_serialize_send(doctype->name.data, doctype->name.length, ctx);
    }

    if (doctype->public_id.data != NULL && doctype->public_id.length != 0) {
        lxb_html_serialize_send(" PUBLIC ", 8, ctx);
        lxb_html_serialize_send("\"", 1, ctx);

        lxb_html_serialize_send(doctype->public_id.data,
                                doctype->public_id.length, ctx);

        lxb_html_serialize_send("\"", 1, ctx);
    }

    if (doctype->system_id.data != NULL && doctype->system_id.length != 0) {
        if (doctype->public_id.length == 0) {
            lxb_html_serialize_send(" SYSTEM", 7, ctx);
        }

        lxb_html_serialize_send(" \"", 2, ctx);

        lxb_html_serialize_send(doctype->system_id.data,
                                doctype->system_id.length, ctx);

        lxb_html_serialize_send("\"", 1, ctx);
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

            /* U+00A0 NO-BREAK SPACE */
            case 0xA0:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&nbsp;", 6, ctx);

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

            /* U+00A0 NO-BREAK SPACE */
            case 0xA0:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&nbsp;", 6, ctx);

                data++;
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
lxb_html_serialize_attribute_cb(lxb_dom_element_attr_t *attr, bool has_raw,
                                lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;

    if (attr->ns == LXB_HTML_NS__UNDEF) {
        lxb_html_serialize_send(attr->local_name.data, attr->local_name.length,
                                ctx);

        goto value;
    }

    if (attr->ns == LXB_HTML_NS_XML) {
        lxb_html_serialize_send((const lxb_char_t *) "xml:", 4, ctx);
        lxb_html_serialize_send(attr->local_name.data, attr->local_name.length,
                                ctx);

        goto value;
    }

    if (attr->ns == LXB_HTML_NS_XMLNS)
    {
        if (attr->local_name.length == 5
            && lexbor_str_data_cmp(attr->local_name.data,
                                   (const lxb_char_t *) "xmlns"))
        {
            lxb_html_serialize_send((const lxb_char_t *) "xmlns", 5, ctx);
        }
        else {
            lxb_html_serialize_send((const lxb_char_t *) "xmlns:", 5, ctx);
            lxb_html_serialize_send(attr->local_name.data, attr->local_name.length,
                                    ctx);
        }

        goto value;
    }

    if (attr->ns == LXB_HTML_NS_XLINK) {
        lxb_html_serialize_send((const lxb_char_t *) "xlink:", 6, ctx);
        lxb_html_serialize_send(attr->local_name.data, attr->local_name.length,
                                ctx);

        goto value;
    }

    lxb_html_serialize_send(attr->name.data, attr->name.length, ctx);

value:

    if (attr->value.data == NULL) {
        return LXB_STATUS_OK;
    }

    lxb_html_serialize_send("=\"", 2, ctx);

    if (has_raw) {
        lxb_html_serialize_send(attr->value.data, attr->value.length, ctx);
    }
    else {
        status = lxb_html_serialize_send_escaping_attribute_string(attr->value.data,
                                                                   attr->value.length,
                                                                   cb, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }
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

    switch (node->type) {
        case LXB_DOM_NODE_TYPE_ELEMENT:
            lxb_html_serialize_send_indent(indent, ctx);

            status = lxb_html_serialize_pretty_element_cb(lxb_dom_interface_element(node),
                                                          opt, indent, cb, ctx);

            break;

        case LXB_DOM_NODE_TYPE_TEXT:
            status = lxb_html_serialize_pretty_text_cb(lxb_dom_interface_text(node),
                                                       opt, indent, cb, ctx);

            return LXB_STATUS_OK;

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
                                                             cb, ctx);
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
lxb_html_serialize_pretty_tree_cb(lxb_dom_node_t *node,
                                  lxb_html_serialize_opt_t opt, size_t indent,
                                  lxb_html_serialize_cb_f cb, void *ctx)
{
    lxb_status_t status;

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

        if (lxb_html_tree_node_is(node, LXB_HTML_TAG_TEMPLATE)) {
            lxb_html_template_element_t *temp;

            temp = lxb_html_interface_template(node);

            if (temp->content != NULL) {
                if (temp->content->node.first_child != NULL)
                {
                    lxb_html_serialize_send_indent((deep + 1), ctx);
                    lxb_html_serialize_send("#document-fragment", 18, ctx);
                    lxb_html_serialize_send("\n", 1, ctx);

                    status = lxb_html_serialize_pretty_tree_cb(&temp->content->node,
                                                               opt, (deep + 2),
                                                               cb, ctx);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }
            }
        }

        skip_it = lxb_html_serialize_node_is_void(node);

        if (skip_it == false && node->first_child != NULL) {
            deep++;

            node = node->first_child;
        }
        else {
            while(node != root && node->next == NULL)
            {
                if (node->type == LXB_DOM_NODE_TYPE_ELEMENT
                    && lxb_html_serialize_node_is_void(node) == false)
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
                && lxb_html_serialize_node_is_void(node) == false)
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

    lxb_dom_element_attr_t *attr;
    lxb_dom_node_t *node = lxb_dom_interface_node(element);
    lxb_dom_document_t *doc = element->node.owner_document;

    tag_name = lxb_html_tag_name_by_id(doc->tags, node->tag_id, node->ns, &len);
    if (tag_name == NULL) {
        return LXB_STATUS_ERROR;
    }

    lxb_html_serialize_send("<", 1, ctx);

    if (element->node.ns != LXB_HTML_NS_HTML
        && opt & LXB_HTML_SERIALIZE_OPT_TAG_WITH_NS)
    {
        size_t ns_len;
        const lxb_char_t *ns_name;

        ns_name = lxb_html_ns_lower_name_by_id(element->node.ns, &ns_len);
        if (ns_name == NULL) {
            return LXB_STATUS_ERROR;
        }

        lxb_html_serialize_send(ns_name, ns_len, ctx);
        lxb_html_serialize_send(":", 1, ctx);
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

    attr = element->first_attr;

    while (attr != NULL) {
        lxb_html_serialize_send(" ", 1, ctx);

        status = lxb_html_serialize_attribute_cb(attr,
                                                 (opt & LXB_HTML_SERIALIZE_OPT_RAW),
                                                 cb, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        attr = attr->next;
    }

    lxb_html_serialize_send(">", 1, ctx);

    return LXB_STATUS_OK;
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

        return LXB_STATUS_OK;
    }

    switch (node->parent->tag_id) {
        case LXB_HTML_TAG_STYLE:
        case LXB_HTML_TAG_SCRIPT:
        case LXB_HTML_TAG_XMP:
        case LXB_HTML_TAG_IFRAME:
        case LXB_HTML_TAG_NOEMBED:
        case LXB_HTML_TAG_NOFRAMES:
        case LXB_HTML_TAG_PLAINTEXT:
            status = lxb_html_serialize_pretty_send_string(data->data,
                                                           data->length, indent,
                                                           with_indent,
                                                           cb, ctx);
            goto end;

        case LXB_HTML_TAG_NOSCRIPT:
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

            /* U+00A0 NO-BREAK SPACE */
            case 0xA0:
                if (pos != data) {
                    lxb_html_serialize_send(pos, (data - pos), ctx);
                }

                lxb_html_serialize_send("&nbsp;", 6, ctx);

                data++;
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
