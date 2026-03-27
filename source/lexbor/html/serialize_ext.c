/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/dom/interfaces/text.h"
#include "lexbor/dom/interfaces/comment.h"
#include "lexbor/dom/interfaces/processing_instruction.h"
#include "lexbor/dom/interfaces/document_type.h"

#include "lexbor/html/serialize_ext.h"
#include "lexbor/html/tree.h"
#include "lexbor/ns/ns.h"
#include "lexbor/html/interfaces/template_element.h"

#ifndef LEXBOR_DISABLE_INTERNAL_EXTERN
    LXB_EXTERN const unsigned char lexbor_tokenizer_chars_map[256];
#endif

#define lxb_html_serialize_ext_boundary_send(cb, node, data, len, ctx,         \
                                             level, is_close)                  \
    do {                                                                       \
        lxb_status_t status = (cb)((node), (data), (len), (ctx),               \
                                   (level), (is_close));                       \
        if (status != LXB_STATUS_OK) {                                         \
            return status;                                                     \
        }                                                                      \
    }                                                                          \
    while (0)

#define lxb_html_serialize_ext_name_send(cb, node, data, len, ctx, is_close)   \
    do {                                                                       \
        lxb_status_t status = (cb)((node), (data), (len), (ctx), (is_close));  \
        if (status != LXB_STATUS_OK) {                                         \
            return status;                                                     \
        }                                                                      \
    }                                                                          \
    while (0)

#define lxb_html_serialize_ext_attr_send(cb, node, attr, data, len, ctx)       \
    do {                                                                       \
        lxb_status_t status = (cb)((node), (attr), (data), (len), (ctx));      \
        if (status != LXB_STATUS_OK) {                                         \
            return status;                                                     \
        }                                                                      \
    }                                                                          \
    while (0)

#define lxb_html_serialize_ext_text_send(cb, node, data, len, ctx)             \
    do {                                                                       \
        lxb_status_t status = (cb)((node), (data), (len), (ctx));              \
        if (status != LXB_STATUS_OK) {                                         \
            return status;                                                     \
        }                                                                      \
    }                                                                          \
    while (0)

#define lxb_html_serialize_ext_send(cb, data, len, ctx)                        \
    do {                                                                       \
        lxb_status_t status = (cb)((data), (len), (ctx));                      \
        if (status != LXB_STATUS_OK) {                                         \
            return status;                                                     \
        }                                                                      \
    }                                                                          \
    while (0)

#define lxb_html_serialize_ext_indent_send(cb, ctx, indent, level, opt)        \
    do {                                                                       \
        if ((opt) & LXB_HTML_SERIALIZE_EXT_OPT_PRETTY) {                       \
            lxb_status_t status = lxb_html_serialize_ext_indent((cb), (ctx),   \
                                                                (indent),      \
                                                                (level));      \
            if (status != LXB_STATUS_OK) {                                     \
                return status;                                                 \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    while (0)

#define lxb_html_serialize_ext_newline_send(cb, ctx, level)                    \
    do {                                                                       \
        lxb_status_t status = (cb)((lxb_html_serialize_ext_nl_str.data),       \
                                   (lxb_html_serialize_ext_nl_str.length),     \
                                   (ctx), (level));                            \
        if (status != LXB_STATUS_OK) {                                         \
            return status;                                                     \
        }                                                                      \
    }                                                                          \
    while (0)


typedef struct {
    lexbor_str_t  *str;
    lexbor_mraw_t *mraw;
}
lxb_html_serialize_ext_ctx_t;

typedef struct {
    lexbor_str_t                 data;
    lxb_html_serialize_ext_opt_t opt;
}
lxb_html_serialize_ext_opt_str_t;

static const lexbor_str_t lxb_html_serialize_ext_ws_str = lexbor_str(" ");
static const lexbor_str_t lxb_html_serialize_ext_nl_str = lexbor_str("\n");
static const lexbor_str_t lxb_html_serialize_ext_is_str = lexbor_str("is");
static const lexbor_str_t lxb_html_serialize_ext_as_str = lexbor_str("=\"");
static const lexbor_str_t lxb_html_serialize_ext_qo_str = lexbor_str("\"");
static const lexbor_str_t lxb_html_serialize_ext_amp_str = lexbor_str("&amp;");
static const lexbor_str_t lxb_html_serialize_ext_nbsp_str = lexbor_str("&nbsp;");
static const lexbor_str_t lxb_html_serialize_ext_lt_str = lexbor_str("&lt;");
static const lexbor_str_t lxb_html_serialize_ext_gt_str = lexbor_str("&gt;");
static const lexbor_str_t lxb_html_serialize_ext_quot_str = lexbor_str("&quot;");
static const lexbor_str_t lxb_html_serialize_ext_dt_start_str = lexbor_str("<!DOCTYPE");
static const lexbor_str_t lxb_html_serialize_ext_start_str = lexbor_str("<");
static const lexbor_str_t lxb_html_serialize_ext_close_str = lexbor_str("</");
static const lexbor_str_t lxb_html_serialize_ext_tag_end_str = lexbor_str(">");
static const lexbor_str_t lxb_html_serialize_ext_empty_str = lexbor_str("");

static const lxb_html_serialize_ext_opt_str_t lxb_html_serialize_ext_opt_str[] = {
    { lexbor_str("Undefined"), LXB_HTML_SERIALIZE_EXT_OPT_UNDEF },
    { lexbor_str("Skip whitespace nodes"), LXB_HTML_SERIALIZE_EXT_OPT_SKIP_WS_NODES },
    { lexbor_str("Skip comments"), LXB_HTML_SERIALIZE_EXT_OPT_SKIP_COMMENT },
    { lexbor_str("Replace newline"), LXB_HTML_SERIALIZE_EXT_OPT_REPLACE_NEWLINE },
    { lexbor_str("Raw"), LXB_HTML_SERIALIZE_EXT_OPT_RAW },
    { lexbor_str("Without closing"), LXB_HTML_SERIALIZE_EXT_OPT_WITHOUT_CLOSING },
    { lexbor_str("Tag with namespace"), LXB_HTML_SERIALIZE_EXT_OPT_TAG_WITH_NS },
    { lexbor_str("Full doctype"), LXB_HTML_SERIALIZE_EXT_OPT_FULL_DOCTYPE },
    { lexbor_str("Pretty"), LXB_HTML_SERIALIZE_EXT_OPT_PRETTY }
};


/* Node (Element/Document). */
static lxb_status_t
lxb_html_serialize_ext_indent_str(const lxb_char_t *data, size_t length,
                                  void *ctx, size_t level);
static lxb_status_t
lxb_html_serialize_ext_begin_str(const lxb_dom_node_t *node,
                                 const lxb_char_t *data, size_t len,
                                 void *ctx, size_t level, bool is_close);
static lxb_status_t
lxb_html_serialize_ext_ns_str(const lxb_dom_node_t *node,
                              const lxb_char_t *data, size_t len,
                              void *ctx, bool is_close);
static lxb_status_t
lxb_html_serialize_ext_name_str(const lxb_dom_node_t *node,
                                const lxb_char_t *data, size_t len,
                                void *ctx, bool is_close);
static lxb_status_t
lxb_html_serialize_ext_end_str(const lxb_dom_node_t *node,
                               const lxb_char_t *data, size_t len,
                               void *ctx, size_t level, bool is_close);
/* Attribute. */
static lxb_status_t
lxb_html_serialize_ext_attr_ns_str(const lxb_dom_node_t *node,
                                   const lxb_dom_attr_t *attr,
                                   const lxb_char_t *data, size_t len,
                                   void *ctx);
static lxb_status_t
lxb_html_serialize_ext_attr_name_str(const lxb_dom_node_t *node,
                                     const lxb_dom_attr_t *attr,
                                     const lxb_char_t *data, size_t len,
                                     void *ctx);
static lxb_status_t
lxb_html_serialize_ext_attr_value_before_str(const lxb_dom_node_t *node,
                                             const lxb_dom_attr_t *attr,
                                             const lxb_char_t *data,
                                             size_t len, void *ctx);
static lxb_status_t
lxb_html_serialize_ext_attr_value_str(const lxb_dom_node_t *node,
                                      const lxb_dom_attr_t *attr,
                                      const lxb_char_t *data,
                                      size_t len, void *ctx);
static lxb_status_t
lxb_html_serialize_ext_attr_value_after_str(const lxb_dom_node_t *node,
                                            const lxb_dom_attr_t *attr,
                                            const lxb_char_t *data,
                                            size_t len, void *ctx);
static lxb_status_t
lxb_html_serialize_ext_attr_ws_str(const lxb_dom_node_t *node,
                                   const lxb_char_t *data, size_t len,
                                   void *ctx);
/* Text. */
static lxb_status_t
lxb_html_serialize_ext_text_begin_str(const lxb_dom_node_t *node,
                                      const lxb_char_t *data, size_t len,
                                      void *ctx, size_t level, bool is_close);
static lxb_status_t
lxb_html_serialize_ext_text_text_str(const lxb_dom_node_t *node,
                                     const lxb_char_t *data, size_t len,
                                     void *ctx);
static lxb_status_t
lxb_html_serialize_ext_text_end_str(const lxb_dom_node_t *node,
                                    const lxb_char_t *data, size_t len,
                                    void *ctx, size_t level, bool is_close);
/* Comment. */
static lxb_status_t
lxb_html_serialize_ext_comment_begin_str(const lxb_dom_node_t *node,
                                         const lxb_char_t *data, size_t len,
                                         void *ctx, size_t level, bool is_close);
static lxb_status_t
lxb_html_serialize_ext_comment_text_str(const lxb_dom_node_t *node,
                                        const lxb_char_t *data, size_t len,
                                        void *ctx);
static lxb_status_t
lxb_html_serialize_ext_comment_end_str(const lxb_dom_node_t *node,
                                       const lxb_char_t *data, size_t len,
                                       void *ctx, size_t level, bool is_close);
/* Processing instruction. */
static lxb_status_t
lxb_html_serialize_ext_pi_begin_str(const lxb_dom_node_t *node,
                                    const lxb_char_t *data, size_t len,
                                    void *ctx, size_t level, bool is_close);
static lxb_status_t
lxb_html_serialize_ext_pi_target_str(const lxb_dom_node_t *node,
                                     const lxb_char_t *data, size_t len,
                                     void *ctx);
static lxb_status_t
lxb_html_serialize_ext_pi_middle_str(const lxb_dom_node_t *node,
                                     const lxb_char_t *data, size_t len,
                                     void *ctx);
static lxb_status_t
lxb_html_serialize_ext_pi_text_str(const lxb_dom_node_t *node,
                                   const lxb_char_t *data, size_t len,
                                   void *ctx);
static lxb_status_t
lxb_html_serialize_ext_pi_end_str(const lxb_dom_node_t *node,
                                  const lxb_char_t *data, size_t len,
                                  void *ctx, size_t level, bool is_close);
/* Document Type. */
static lxb_status_t
lxb_html_serialize_ext_doctype_begin_str(const lxb_dom_node_t *node,
                                         const lxb_char_t *data, size_t len,
                                         void *ctx, size_t level, bool is_close);
static lxb_status_t
lxb_html_serialize_ext_doctype_name_str(const lxb_dom_node_t *node,
                                        const lxb_char_t *data, size_t len,
                                        void *ctx);
static lxb_status_t
lxb_html_serialize_ext_doctype_public_str(const lxb_dom_node_t *node,
                                          const lxb_char_t *data, size_t len,
                                          void *ctx);
static lxb_status_t
lxb_html_serialize_ext_doctype_system_str(const lxb_dom_node_t *node,
                                          const lxb_char_t *data, size_t len,
                                          void *ctx);
static lxb_status_t
lxb_html_serialize_ext_doctype_end_str(const lxb_dom_node_t *node,
                                       const lxb_char_t *data, size_t len,
                                       void *ctx, size_t level, bool is_close);
static lxb_status_t
lxb_html_serialize_ext_doctype_ws_str(const lxb_dom_node_t *node,
                                      const lxb_char_t *data, size_t len,
                                      void *ctx);
/* New Line. */
static lxb_status_t
lxb_html_serialize_ext_newline_str(const lxb_char_t *data, size_t len,
                                   void *ctx, size_t level);


static const lxb_html_serialize_ext_node_t lxb_html_serialize_ext_node = {
    .indent = lxb_html_serialize_ext_indent_str,
    .begin = lxb_html_serialize_ext_begin_str,
    .ns = lxb_html_serialize_ext_ns_str,
    .name = lxb_html_serialize_ext_name_str,
    .end = lxb_html_serialize_ext_end_str
};

static const lxb_html_serialize_ext_attr_t lxb_html_serialize_ext_attr = {
    .ns = lxb_html_serialize_ext_attr_ns_str,
    .name = lxb_html_serialize_ext_attr_name_str,
    .value_before = lxb_html_serialize_ext_attr_value_before_str,
    .value = lxb_html_serialize_ext_attr_value_str,
    .value_after = lxb_html_serialize_ext_attr_value_after_str,
    .ws = lxb_html_serialize_ext_attr_ws_str
};

static const lxb_html_serialize_ext_text_t lxb_html_serialize_ext_text = {
    .indent = lxb_html_serialize_ext_indent_str,
    .begin = lxb_html_serialize_ext_text_begin_str,
    .text = lxb_html_serialize_ext_text_text_str,
    .end = lxb_html_serialize_ext_text_end_str
};

static const lxb_html_serialize_ext_comment_t lxb_html_serialize_ext_comment = {
    .indent = lxb_html_serialize_ext_indent_str,
    .begin = lxb_html_serialize_ext_comment_begin_str,
    .text = lxb_html_serialize_ext_comment_text_str,
    .end = lxb_html_serialize_ext_comment_end_str
};

static const lxb_html_serialize_ext_processing_instruction_t lxb_html_serialize_ext_processing_instruction = {
    .indent = lxb_html_serialize_ext_indent_str,
    .begin = lxb_html_serialize_ext_pi_begin_str,
    .target = lxb_html_serialize_ext_pi_target_str,
    .middle = lxb_html_serialize_ext_pi_middle_str,
    .text = lxb_html_serialize_ext_pi_text_str,
    .end = lxb_html_serialize_ext_pi_end_str
};

static const lxb_html_serialize_ext_document_type_t lxb_html_serialize_ext_document_type = {
    .indent = lxb_html_serialize_ext_indent_str,
    .begin = lxb_html_serialize_ext_doctype_begin_str,
    .name = lxb_html_serialize_ext_doctype_name_str,
    .text_public = lxb_html_serialize_ext_doctype_public_str,
    .text_system = lxb_html_serialize_ext_doctype_system_str,
    .end = lxb_html_serialize_ext_doctype_end_str,
    .ws = lxb_html_serialize_ext_doctype_ws_str
};

static const lxb_html_serialize_ext_node_t lxb_html_serialize_ext_document = {
    .indent = lxb_html_serialize_ext_indent_str,
    .begin = lxb_html_serialize_ext_begin_str,
    .name = lxb_html_serialize_ext_name_str,
    .end = lxb_html_serialize_ext_end_str
};

static const lxb_html_serialize_ext_t lxb_html_serialize_ext_str = {
    .node = &lxb_html_serialize_ext_node,
    .attr = &lxb_html_serialize_ext_attr,
    .text = &lxb_html_serialize_ext_text,
    .comment = &lxb_html_serialize_ext_comment,
    .processing_instruction = &lxb_html_serialize_ext_processing_instruction,
    .document_type = &lxb_html_serialize_ext_document_type,
    .document = &lxb_html_serialize_ext_document,
    .newline = lxb_html_serialize_ext_newline_str
};


static lxb_status_t
lxb_html_serialize_ext_tree_cb_h(const lxb_dom_node_t *node,
                                 const lxb_html_serialize_ext_t *cb, void *ctx,
                                 lxb_html_serialize_ext_opt_t opt,
                                 const lexbor_str_t *indent, size_t level,
                                 bool with_current);

static lxb_status_t
lxb_html_serialize_ext_node_cb_h(const lxb_dom_node_t *node,
                                 const lxb_html_serialize_ext_t *cb, void *ctx,
                                 lxb_html_serialize_ext_opt_t opt,
                                 const lexbor_str_t *indent, size_t level);
static lxb_status_t
lxb_html_serialize_ext_element_cb_h(const lxb_dom_element_t *element,
                                    const lxb_html_serialize_ext_node_t *cb_node,
                                    const lxb_html_serialize_ext_attr_t *cb_attr,
                                    void *ctx, lxb_html_serialize_ext_opt_t opt,
                                    const lexbor_str_t *indent, size_t level,
                                    bool is_close);
static lxb_status_t
lxb_html_serialize_ext_element_wo_attr_cb_h(const lxb_dom_element_t *element,
                                            const lxb_html_serialize_ext_node_t *cb,
                                            void *ctx, lxb_html_serialize_ext_opt_t opt,
                                            const lexbor_str_t *indent,
                                            size_t level, bool is_close);
static lxb_status_t
lxb_html_serialize_ext_text_cb_h(const lxb_dom_text_t *text,
                                 const lxb_html_serialize_ext_text_t *cb,
                                 void *ctx, lxb_html_serialize_ext_opt_t opt,
                                 const lexbor_str_t *indent, size_t level);
static lxb_status_t
lxb_html_serialize_ext_comment_cb_h(const lxb_dom_comment_t *comment,
                                    const lxb_html_serialize_ext_comment_t *cb,
                                    void *ctx, lxb_html_serialize_ext_opt_t opt,
                                    const lexbor_str_t *indent, size_t level);
static lxb_status_t
lxb_html_serialize_ext_processing_instruction_cb_h(const lxb_dom_processing_instruction_t *pi,
                                                   const lxb_html_serialize_ext_processing_instruction_t *cb,
                                                   void *ctx, lxb_html_serialize_ext_opt_t opt,
                                                   const lexbor_str_t *indent, size_t level);
static lxb_status_t
lxb_html_serialize_ext_document_type_cb_h(const lxb_dom_document_type_t *doctype,
                                          const lxb_html_serialize_ext_document_type_t *cb,
                                          void *ctx, lxb_html_serialize_ext_opt_t opt,
                                          const lexbor_str_t *indent, size_t level);
static lxb_status_t
lxb_html_serialize_ext_document_type_full_cb_h(const lxb_dom_document_type_t *doctype,
                                               const lxb_html_serialize_ext_document_type_t *cb,
                                               void *ctx, lxb_html_serialize_ext_opt_t opt,
                                               const lexbor_str_t *indent, size_t level);
static lxb_status_t
lxb_html_serialize_ext_document_cb_h(const lxb_dom_document_t *document,
                                     const lxb_html_serialize_ext_node_t *cb,
                                     void *ctx, lxb_html_serialize_ext_opt_t opt,
                                     const lexbor_str_t *indent, size_t level);
static lxb_status_t
lxb_html_serialize_ext_walk(const lxb_dom_node_t *node,
                            const lxb_html_serialize_ext_t *cb, void *ctx,
                            const lexbor_str_t *indent,
                            lxb_html_serialize_ext_opt_t opt, size_t level);
static lxb_status_t
lxb_html_serialize_ext_escaping_attribute(const lxb_dom_node_t *node,
                                          const lxb_dom_attr_t *attr,
                                          const lxb_char_t *data, size_t len,
                                          lxb_html_serialize_ext_attr_cb_f cb,
                                          void *ctx);
static lxb_status_t
lxb_html_serialize_ext_escaping_string(const lxb_dom_node_t *node,
                                       const lxb_char_t *data, size_t len,
                                       lxb_html_serialize_ext_text_cb_f cb,
                                       void *ctx,
                                       lxb_html_serialize_ext_opt_t opt);
static lxb_status_t
lxb_html_serialize_ext_opt_string(const lxb_dom_node_t *node,
                                  const lxb_char_t *data, size_t len,
                                  lxb_html_serialize_ext_text_cb_f cb,
                                  void *ctx,
                                  lxb_html_serialize_ext_opt_t opt);
static lxb_status_t
lxb_html_serialize_ext_indent(lxb_html_serialize_ext_indent_cb_f cb, void *ctx,
                              const lexbor_str_t *indent, size_t level);
static lxb_status_t
lxb_html_serialize_ext_fragment(const lxb_dom_node_t *node,
                                const lxb_html_serialize_ext_node_t *cb,
                                void *ctx, lxb_html_serialize_ext_opt_t opt,
                                const lexbor_str_t *indent, size_t level);


lxb_status_t
lxb_html_serialize_ext_tree_cb(const lxb_dom_node_t *node,
                               const lxb_html_serialize_ext_t *cb, void *ctx,
                               lxb_html_serialize_ext_opt_t opt,
                               const lexbor_str_t *indent, bool with_current)
{
    return lxb_html_serialize_ext_tree_cb_h(node, cb, ctx, opt, indent, 0,
                                            with_current);
}

static lxb_status_t
lxb_html_serialize_ext_tree_cb_h(const lxb_dom_node_t *node,
                                 const lxb_html_serialize_ext_t *cb, void *ctx,
                                 lxb_html_serialize_ext_opt_t opt,
                                 const lexbor_str_t *indent, size_t level,
                                 bool with_current)
{
    lxb_status_t status;

    if (node->type == LXB_DOM_NODE_TYPE_DOCUMENT) {
        with_current = false;
    }

    if (with_current) {
        return lxb_html_serialize_ext_walk(node, cb, ctx, indent, opt, level);
    }
    else {
        node = node->first_child;

        while (node != NULL) {
            status = lxb_html_serialize_ext_walk(node, cb, ctx, indent,
                                                 opt, level);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            node = node->next;
        }
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_ext_tree_str(const lxb_dom_node_t *node, lexbor_str_t *str,
                                lxb_html_serialize_ext_opt_t opt,
                                const lexbor_str_t *indent, bool with_current)
{
    lxb_html_serialize_ext_ctx_t ctx;

    if (str->data == NULL) {
        lexbor_str_init(str, node->owner_document->text, 1024);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    ctx.str = str;
    ctx.mraw = node->owner_document->text;

    return lxb_html_serialize_ext_tree_cb_h(node,
                                            &lxb_html_serialize_ext_str,
                                            &ctx, opt, indent, 0,
                                            with_current);
}

lxb_status_t
lxb_html_serialize_ext_node_cb(const lxb_dom_node_t *node,
                               const lxb_html_serialize_ext_t *cb, void *ctx,
                               lxb_html_serialize_ext_opt_t opt,
                               const lexbor_str_t *indent)
{
    switch (node->type) {
        case LXB_DOM_NODE_TYPE_ELEMENT:
            return lxb_html_serialize_ext_element_cb(lxb_dom_interface_element(node),
                                                     cb->node, cb->attr, ctx, opt,
                                                     indent);
        case LXB_DOM_NODE_TYPE_TEXT:
            return lxb_html_serialize_ext_text_cb(lxb_dom_interface_text(node),
                                                  cb->text, ctx, opt, indent);
        case LXB_DOM_NODE_TYPE_COMMENT:
            return lxb_html_serialize_ext_comment_cb(lxb_dom_interface_comment(node),
                                                     cb->comment, ctx, opt, indent);

        case LXB_DOM_NODE_TYPE_PROCESSING_INSTRUCTION:
            return lxb_html_serialize_ext_processing_instruction_cb(lxb_dom_interface_processing_instruction(node),
                                                                    cb->processing_instruction, ctx, opt, indent);
        case LXB_DOM_NODE_TYPE_DOCUMENT_TYPE:
            if (!(opt & LXB_HTML_SERIALIZE_EXT_OPT_FULL_DOCTYPE)) {
                return lxb_html_serialize_ext_document_type_cb(lxb_dom_interface_document_type(node),
                                                               cb->document_type, ctx, opt, indent);
            }

            return lxb_html_serialize_ext_document_type_full_cb(lxb_dom_interface_document_type(node),
                                                                cb->document_type, ctx, opt, indent);
        case LXB_DOM_NODE_TYPE_DOCUMENT:
            return lxb_html_serialize_ext_document_cb_h(lxb_dom_interface_document(node),
                                                        cb->document, ctx, opt, indent, 0);
        default:
            break;
    }

    return LXB_STATUS_ERROR;
}

static lxb_status_t
lxb_html_serialize_ext_node_cb_h(const lxb_dom_node_t *node,
                                 const lxb_html_serialize_ext_t *cb, void *ctx,
                                 lxb_html_serialize_ext_opt_t opt,
                                 const lexbor_str_t *indent, size_t level)
{
    switch (node->type) {
        case LXB_DOM_NODE_TYPE_ELEMENT:
            return lxb_html_serialize_ext_element_cb_h(lxb_dom_interface_element(node),
                                                       cb->node, cb->attr, ctx, opt,
                                                       indent, level, false);
        case LXB_DOM_NODE_TYPE_TEXT:
            return lxb_html_serialize_ext_text_cb_h(lxb_dom_interface_text(node),
                                                    cb->text, ctx, opt, indent, level);
        case LXB_DOM_NODE_TYPE_COMMENT:
            return lxb_html_serialize_ext_comment_cb_h(lxb_dom_interface_comment(node),
                                                       cb->comment, ctx, opt, indent, level);

        case LXB_DOM_NODE_TYPE_PROCESSING_INSTRUCTION:
            return lxb_html_serialize_ext_processing_instruction_cb_h(lxb_dom_interface_processing_instruction(node),
                                                                      cb->processing_instruction, ctx, opt, indent, level);
        case LXB_DOM_NODE_TYPE_DOCUMENT_TYPE:
            if (!(opt & LXB_HTML_SERIALIZE_EXT_OPT_FULL_DOCTYPE)) {
                return lxb_html_serialize_ext_document_type_cb_h(lxb_dom_interface_document_type(node),
                                                                 cb->document_type, ctx, opt, indent, level);
            }

            return lxb_html_serialize_ext_document_type_full_cb_h(lxb_dom_interface_document_type(node),
                                                                  cb->document_type, ctx, opt, indent, level);
        case LXB_DOM_NODE_TYPE_DOCUMENT:
            return lxb_html_serialize_ext_document_cb_h(lxb_dom_interface_document(node),
                                                        cb->document, ctx, opt, indent, 0);
        default:
            break;
    }

    return LXB_STATUS_ERROR;
}

lxb_status_t
lxb_html_serialize_ext_node_str(const lxb_dom_node_t *node, lexbor_str_t *str,
                                lxb_html_serialize_ext_opt_t opt,
                                const lexbor_str_t *indent)
{
    lxb_status_t status;
    lxb_html_serialize_ext_ctx_t ctx;

    if (str->data == NULL) {
        lexbor_str_init(str, node->owner_document->text, 1024);

        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    ctx.str = str;
    ctx.mraw = node->owner_document->text;

    status = lxb_html_serialize_ext_node_cb_h(node,
                                              &lxb_html_serialize_ext_str,
                                              &ctx, opt, indent, 0);
    if (status != LXB_STATUS_OK) {
        if (status != LXB_STATUS_SKIPPED) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_ext_element_cb(const lxb_dom_element_t *element,
                                  const lxb_html_serialize_ext_node_t *cb_node,
                                  const lxb_html_serialize_ext_attr_t *cb_attr,
                                  void *ctx, lxb_html_serialize_ext_opt_t opt,
                                  const lexbor_str_t *indent)
{
    return lxb_html_serialize_ext_element_cb_h(element, cb_node, cb_attr, ctx,
                                               opt, indent, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_element_cb_h(const lxb_dom_element_t *element,
                                    const lxb_html_serialize_ext_node_t *cb_node,
                                    const lxb_html_serialize_ext_attr_t *cb_attr,
                                    void *ctx, lxb_html_serialize_ext_opt_t opt,
                                    const lexbor_str_t *indent, size_t level,
                                    bool is_close)
{
    lxb_status_t status;
    const lxb_dom_attr_t *attr;
    const lxb_dom_node_t *node;
    const lxb_char_t *tag_name;
    const lxb_ns_prefix_data_t *data;
    size_t len = 0;

    node = lxb_dom_interface_node(element);

    tag_name = lxb_dom_element_qualified_name(element, &len);
    if (tag_name == NULL) {
        return LXB_STATUS_ERROR;
    }

    lxb_html_serialize_ext_indent_send(cb_node->indent, ctx, indent,
                                       level, opt);

    if (!is_close) {
        lxb_html_serialize_ext_boundary_send(cb_node->begin, node,
                                             lxb_html_serialize_ext_start_str.data,
                                             lxb_html_serialize_ext_start_str.length,
                                             ctx, level, false);
    }
    else {
        lxb_html_serialize_ext_boundary_send(cb_node->begin, node,
                                             lxb_html_serialize_ext_close_str.data,
                                             lxb_html_serialize_ext_close_str.length,
                                             ctx, level, true);
    }

    if ((opt & LXB_HTML_SERIALIZE_EXT_OPT_TAG_WITH_NS)
        && node->ns != LXB_NS_HTML)
    {
        data = NULL;

        if (element->node.prefix != LXB_NS__UNDEF) {
            data = lxb_ns_prefix_data_by_id(node->owner_document->prefix,
                                            element->node.prefix);
        }
        else if (element->node.ns < LXB_NS__LAST_ENTRY) {
            data = lxb_ns_prefix_data_by_id(node->owner_document->prefix,
                                            element->node.ns);
        }

        if (data != NULL) {
            lxb_html_serialize_ext_name_send(cb_node->ns, node,
                                             lexbor_hash_entry_str(&data->entry),
                                             data->entry.length, ctx, is_close);
        }
    }

    lxb_html_serialize_ext_name_send(cb_node->name, node, tag_name, len,
                                     ctx, is_close);

    if (element->is_value != NULL && element->is_value->data != NULL) {
        attr = lxb_dom_element_attr_is_exist(element,
                                             (const lxb_char_t *) "is", 2);
        if (attr == NULL) {
            lxb_html_serialize_ext_text_send(cb_attr->ws, node,
                                             lxb_html_serialize_ext_ws_str.data,
                                             lxb_html_serialize_ext_ws_str.length,
                                             ctx);

            lxb_html_serialize_ext_attr_send(cb_attr->name, node, NULL,
                                             lxb_html_serialize_ext_is_str.data,
                                             lxb_html_serialize_ext_is_str.length, ctx);

            lxb_html_serialize_ext_attr_send(cb_attr->value_before, node, NULL,
                                             lxb_html_serialize_ext_as_str.data,
                                             lxb_html_serialize_ext_as_str.length, ctx);

            if (!(opt & LXB_HTML_SERIALIZE_EXT_OPT_RAW)) {
                status = lxb_html_serialize_ext_escaping_attribute(node, NULL,
                                                                   element->is_value->data,
                                                                   element->is_value->length,
                                                                   cb_attr->value, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }
            }
            else {
                lxb_html_serialize_ext_attr_send(cb_attr->value, node, NULL,
                                                 element->is_value->data,
                                                 element->is_value->length, ctx);
            }

            lxb_html_serialize_ext_attr_send(cb_attr->value_after, node, NULL,
                                             lxb_html_serialize_ext_qo_str.data,
                                             lxb_html_serialize_ext_qo_str.length,
                                             ctx);
        }
    }

    attr = element->first_attr;

    while (attr != NULL) {
        lxb_html_serialize_ext_text_send(cb_attr->ws, node,
                                         lxb_html_serialize_ext_ws_str.data,
                                         lxb_html_serialize_ext_ws_str.length,
                                         ctx);

        status = lxb_html_serialize_ext_attribute_cb(attr, cb_attr, ctx, opt);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        attr = attr->next;
    }

    lxb_html_serialize_ext_boundary_send(cb_node->end, node,
                                         lxb_html_serialize_ext_tag_end_str.data,
                                         lxb_html_serialize_ext_tag_end_str.length,
                                         ctx, level, is_close);
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_ext_element_wo_attr_cb(const lxb_dom_element_t *element,
                                          const lxb_html_serialize_ext_node_t *cb,
                                          void *ctx, lxb_html_serialize_ext_opt_t opt,
                                          const lexbor_str_t *indent)
{
    return lxb_html_serialize_ext_element_wo_attr_cb_h(element, cb, ctx, opt,
                                                       indent, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_element_wo_attr_cb_h(const lxb_dom_element_t *element,
                                            const lxb_html_serialize_ext_node_t *cb,
                                            void *ctx, lxb_html_serialize_ext_opt_t opt,
                                            const lexbor_str_t *indent,
                                            size_t level, bool is_close)
{
    const lxb_dom_node_t *node;
    const lxb_char_t *tag_name;
    size_t len = 0;

    node = lxb_dom_interface_node(element);

    tag_name = lxb_dom_element_qualified_name(element, &len);
    if (tag_name == NULL) {
        return LXB_STATUS_ERROR;
    }

    lxb_html_serialize_ext_indent_send(cb->indent, ctx, indent,
                                       level, opt);

    if (!is_close) {
        lxb_html_serialize_ext_boundary_send(cb->begin, node,
                                             lxb_html_serialize_ext_start_str.data,
                                             lxb_html_serialize_ext_start_str.length,
                                             ctx, level, false);
    }
    else {
        lxb_html_serialize_ext_boundary_send(cb->begin, node,
                                             lxb_html_serialize_ext_close_str.data,
                                             lxb_html_serialize_ext_close_str.length,
                                             ctx, level, true);
    }

    if ((opt & LXB_HTML_SERIALIZE_EXT_OPT_TAG_WITH_NS)
        && node->ns != LXB_NS_HTML)
    {
        const lxb_ns_prefix_data_t *ns_data = NULL;

        if (element->node.prefix != LXB_NS__UNDEF) {
            ns_data = lxb_ns_prefix_data_by_id(node->owner_document->prefix,
                                               element->node.prefix);
        }
        else if (element->node.ns < LXB_NS__LAST_ENTRY) {
            ns_data = lxb_ns_prefix_data_by_id(node->owner_document->prefix,
                                               element->node.ns);
        }

        if (ns_data != NULL) {
            lxb_html_serialize_ext_name_send(cb->ns, node,
                                             lexbor_hash_entry_str(&ns_data->entry),
                                             ns_data->entry.length, ctx, is_close);
        }
    }

    lxb_html_serialize_ext_name_send(cb->name, node, tag_name, len,
                                     ctx, is_close);

    lxb_html_serialize_ext_boundary_send(cb->end, node,
                                         lxb_html_serialize_ext_tag_end_str.data,
                                         lxb_html_serialize_ext_tag_end_str.length,
                                         ctx, level, is_close);
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_ext_attribute_cb(const lxb_dom_attr_t *attr,
                                    const lxb_html_serialize_ext_attr_t *ext_attr,
                                    void *ctx, lxb_html_serialize_ext_opt_t opt)
{
    size_t length;
    lxb_status_t status;
    const lxb_char_t *str;
    const lxb_dom_node_t *node;
    const lxb_dom_attr_data_t *data;

    static const lexbor_str_t xml_str = lexbor_str("xml");
    static const lexbor_str_t xlink_str = lexbor_str("xlink");
    static const lexbor_str_t xmlns_str = lexbor_str("xmlns");

    data = lxb_dom_attr_data_by_id(attr->node.owner_document->attrs,
                                   attr->node.local_name);
    if (data == NULL) {
        return LXB_STATUS_ERROR;
    }

    node = lxb_dom_interface_node(attr->owner);

    if (attr->node.ns == LXB_NS__UNDEF) {
        lxb_html_serialize_ext_attr_send(ext_attr->name, node, attr,
                                         lexbor_hash_entry_str(&data->entry),
                                         data->entry.length, ctx);
        goto value;
    }

    if (attr->node.ns == LXB_NS_XML) {
        lxb_html_serialize_ext_attr_send(ext_attr->ns, node, attr,
                                         xml_str.data, xml_str.length, ctx);

        lxb_html_serialize_ext_attr_send(ext_attr->name, node, attr,
                                         lexbor_hash_entry_str(&data->entry),
                                         data->entry.length, ctx);
        goto value;
    }

    if (attr->node.ns == LXB_NS_XMLNS)
    {
        if (attr->node.local_name == LXB_DOM_ATTR_XMLNS) {
            lxb_html_serialize_ext_attr_send(ext_attr->name, node, attr,
                                             xmlns_str.data, xmlns_str.length,
                                             ctx);
        }
        else {
            lxb_html_serialize_ext_attr_send(ext_attr->ns, node, attr,
                                             xmlns_str.data, xmlns_str.length,
                                             ctx);

            lxb_html_serialize_ext_attr_send(ext_attr->name, node, attr,
                                             lexbor_hash_entry_str(&data->entry),
                                             data->entry.length, ctx);
        }

        goto value;
    }

    if (attr->node.ns == LXB_NS_XLINK) {
        lxb_html_serialize_ext_attr_send(ext_attr->ns, node, attr,
                                         xlink_str.data, xlink_str.length,
                                         ctx);

        lxb_html_serialize_ext_attr_send(ext_attr->name, node, attr,
                                         lexbor_hash_entry_str(&data->entry),
                                         data->entry.length, ctx);
        goto value;
    }

    str = lxb_dom_attr_qualified_name(attr, &length);
    if (str == NULL) {
        return LXB_STATUS_ERROR;
    }

    lxb_html_serialize_ext_attr_send(ext_attr->name, node, attr,
                                     str, length, ctx);
value:

    lxb_html_serialize_ext_attr_send(ext_attr->value_before, node, attr,
                                     lxb_html_serialize_ext_as_str.data,
                                     lxb_html_serialize_ext_as_str.length, ctx);

    if (attr->value == NULL) {
        lxb_html_serialize_ext_attr_send(ext_attr->value_after, node, attr,
                                         lxb_html_serialize_ext_qo_str.data,
                                         lxb_html_serialize_ext_qo_str.length,
                                         ctx);
        return LXB_STATUS_OK;
    }

    if (!(opt & LXB_HTML_SERIALIZE_EXT_OPT_RAW)) {
        status = lxb_html_serialize_ext_escaping_attribute(node, attr,
                                                           attr->value->data,
                                                           attr->value->length,
                                                           ext_attr->value, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }
    else {
        lxb_html_serialize_ext_attr_send(ext_attr->value, node, attr,
                                         attr->value->data, attr->value->length,
                                         ctx);
    }

    lxb_html_serialize_ext_attr_send(ext_attr->value_after, node, attr,
                                     lxb_html_serialize_ext_qo_str.data,
                                     lxb_html_serialize_ext_qo_str.length,
                                     ctx);
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_ext_text_cb(const lxb_dom_text_t *text,
                               const lxb_html_serialize_ext_text_t *cb,
                               void *ctx, lxb_html_serialize_ext_opt_t opt,
                               const lexbor_str_t *indent)
{
    return lxb_html_serialize_ext_text_cb_h(text, cb, ctx, opt, indent, 0);
}

static lxb_status_t
lxb_html_serialize_ext_text_cb_h(const lxb_dom_text_t *text,
                                 const lxb_html_serialize_ext_text_t *cb,
                                 void *ctx, lxb_html_serialize_ext_opt_t opt,
                                 const lexbor_str_t *indent, size_t level)
{
    lxb_status_t status;
    const lxb_char_t *pos, *end;

    const lxb_dom_node_t *node = lxb_dom_interface_node(text);
    const lxb_dom_document_t *doc = node->owner_document;
    const lexbor_str_t *data = &text->char_data.data;

    if (opt & LXB_HTML_SERIALIZE_EXT_OPT_SKIP_WS_NODES) {
        pos = data->data;
        end = pos + data->length;

        while (pos != end) {
            if (lexbor_tokenizer_chars_map[ *pos ]
                != LEXBOR_STR_RES_MAP_CHAR_WHITESPACE)
            {
                break;
            }

            pos++;
        }

        if (pos >= end) {
            return LXB_STATUS_SKIPPED;
        }
    }

    lxb_html_serialize_ext_indent_send(cb->indent, ctx, indent, level, opt);
    lxb_html_serialize_ext_boundary_send(cb->begin, node,
                                         lxb_html_serialize_ext_empty_str.data,
                                         lxb_html_serialize_ext_empty_str.length,
                                         ctx, level, false);

    if (opt & LXB_HTML_SERIALIZE_EXT_OPT_PRETTY) {
        lxb_html_serialize_ext_text_send(cb->text, node,
                                         lxb_html_serialize_ext_qo_str.data,
                                         lxb_html_serialize_ext_qo_str.length,
                                         ctx);
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
                if (!(opt & LXB_HTML_SERIALIZE_EXT_OPT_REPLACE_NEWLINE)) {
                    lxb_html_serialize_ext_text_send(cb->text, node, data->data,
                                                     data->length, ctx);
                }
                else {
                    status = lxb_html_serialize_ext_opt_string(node, data->data,
                                                               data->length, cb->text,
                                                               ctx, opt);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }

                goto after;

            case LXB_TAG_NOSCRIPT:
                if (doc->scripting) {
                    if (!(opt & LXB_HTML_SERIALIZE_EXT_OPT_REPLACE_NEWLINE)) {
                        lxb_html_serialize_ext_text_send(cb->text, node, data->data,
                                                         data->length, ctx);
                    }
                    else {
                        status = lxb_html_serialize_ext_opt_string(node, data->data,
                                                                   data->length, cb->text,
                                                                   ctx, opt);
                        if (status != LXB_STATUS_OK) {
                            return status;
                        }
                    }

                    goto after;
                }

                break;

            default:
                break;
        }
    }

    if (!(opt & LXB_HTML_SERIALIZE_EXT_OPT_RAW)) {
        status = lxb_html_serialize_ext_escaping_string(node, data->data,
                                                        data->length, cb->text,
                                                        ctx, opt);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }
    else {
        if (!(opt & LXB_HTML_SERIALIZE_EXT_OPT_REPLACE_NEWLINE)) {
            lxb_html_serialize_ext_text_send(cb->text, node, data->data,
                                             data->length, ctx);
        }
        else {
            status = lxb_html_serialize_ext_opt_string(node, data->data,
                                                       data->length, cb->text,
                                                       ctx, opt);
            if (status != LXB_STATUS_OK) {
                return status;
            }
        }
    }

after:

    if (opt & LXB_HTML_SERIALIZE_EXT_OPT_PRETTY) {
        lxb_html_serialize_ext_text_send(cb->text, node,
                                         lxb_html_serialize_ext_qo_str.data,
                                         lxb_html_serialize_ext_qo_str.length,
                                         ctx);
    }

    lxb_html_serialize_ext_boundary_send(cb->end, node,
                                         lxb_html_serialize_ext_empty_str.data,
                                         lxb_html_serialize_ext_empty_str.length,
                                         ctx, level, false);
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_ext_comment_cb(const lxb_dom_comment_t *comment,
                                  const lxb_html_serialize_ext_comment_t *cb,
                                  void *ctx, lxb_html_serialize_ext_opt_t opt,
                                  const lexbor_str_t *indent)
{
    return lxb_html_serialize_ext_comment_cb_h(comment, cb, ctx, opt,
                                               indent, 0);
}

static lxb_status_t
lxb_html_serialize_ext_comment_cb_h(const lxb_dom_comment_t *comment,
                                    const lxb_html_serialize_ext_comment_t *cb,
                                    void *ctx, lxb_html_serialize_ext_opt_t opt,
                                    const lexbor_str_t *indent, size_t level)
{
    const lexbor_str_t *data = &comment->char_data.data;
    const lxb_dom_node_t *node = lxb_dom_interface_node(comment);

    static const lexbor_str_t comment_start_str = lexbor_str("<!--");
    static const lexbor_str_t comment_end_str = lexbor_str("-->");

    if (opt & LXB_HTML_SERIALIZE_EXT_OPT_SKIP_COMMENT) {
        return LXB_STATUS_SKIPPED;
    }

    lxb_html_serialize_ext_indent_send(cb->indent, ctx, indent, level, opt);

    lxb_html_serialize_ext_boundary_send(cb->begin, node, comment_start_str.data,
                                         comment_start_str.length, ctx, level, false);

    lxb_html_serialize_ext_text_send(cb->text, node, data->data, data->length,
                                     ctx);

    lxb_html_serialize_ext_boundary_send(cb->end, node, comment_end_str.data,
                                         comment_end_str.length, ctx, level, false);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_ext_processing_instruction_cb(const lxb_dom_processing_instruction_t *pi,
                                                 const lxb_html_serialize_ext_processing_instruction_t *cb,
                                                 void *ctx, lxb_html_serialize_ext_opt_t opt,
                                                 const lexbor_str_t *indent)
{
    return lxb_html_serialize_ext_processing_instruction_cb_h(pi, cb, ctx, opt,
                                                              indent, 0);
}

static lxb_status_t
lxb_html_serialize_ext_processing_instruction_cb_h(const lxb_dom_processing_instruction_t *pi,
                                                   const lxb_html_serialize_ext_processing_instruction_t *cb,
                                                   void *ctx, lxb_html_serialize_ext_opt_t opt,
                                                   const lexbor_str_t *indent, size_t level)
{
    const lexbor_str_t *data = &pi->char_data.data;
    const lxb_dom_node_t *node = lxb_dom_interface_node(pi);

    static const lexbor_str_t pi_start_str = lexbor_str("<?");
    static const lexbor_str_t pi_end_str = lexbor_str(">");

    lxb_html_serialize_ext_indent_send(cb->indent, ctx, indent, level, opt);

    lxb_html_serialize_ext_boundary_send(cb->begin, node, pi_start_str.data,
                                         pi_start_str.length, ctx, level, false);

    lxb_html_serialize_ext_text_send(cb->target, node, pi->target.data,
                                     pi->target.length, ctx);

    lxb_html_serialize_ext_text_send(cb->middle, node,
                                     lxb_html_serialize_ext_ws_str.data,
                                     lxb_html_serialize_ext_ws_str.length, ctx);

    lxb_html_serialize_ext_text_send(cb->text, node, data->data,
                                     data->length, ctx);

    lxb_html_serialize_ext_boundary_send(cb->end, node, pi_end_str.data,
                                         pi_end_str.length, ctx, level, false);

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_ext_document_type_cb(const lxb_dom_document_type_t *doctype,
                                        const lxb_html_serialize_ext_document_type_t *cb,
                                        void *ctx, lxb_html_serialize_ext_opt_t opt,
                                        const lexbor_str_t *indent)
{
    return lxb_html_serialize_ext_document_type_cb_h(doctype, cb, ctx, opt,
                                                     indent, 0);
}

static lxb_status_t
lxb_html_serialize_ext_document_type_cb_h(const lxb_dom_document_type_t *doctype,
                                          const lxb_html_serialize_ext_document_type_t *cb,
                                          void *ctx, lxb_html_serialize_ext_opt_t opt,
                                          const lexbor_str_t *indent, size_t level)
{
    size_t length;
    const lxb_char_t *name;
    const lxb_dom_node_t *node = lxb_dom_interface_node(doctype);

    lxb_html_serialize_ext_indent_send(cb->indent, ctx, indent, level, opt);

    lxb_html_serialize_ext_boundary_send(cb->begin, node,
                                         lxb_html_serialize_ext_dt_start_str.data,
                                         lxb_html_serialize_ext_dt_start_str.length,
                                         ctx, level, false);

    name = lxb_dom_document_type_name(doctype, &length);

    if (length != 0) {
        lxb_html_serialize_ext_text_send(cb->ws, node,
                                         lxb_html_serialize_ext_ws_str.data,
                                         lxb_html_serialize_ext_ws_str.length, ctx);

        lxb_html_serialize_ext_text_send(cb->name, node, name, length, ctx);
    }

    lxb_html_serialize_ext_boundary_send(cb->end, node,
                                         lxb_html_serialize_ext_tag_end_str.data,
                                         lxb_html_serialize_ext_tag_end_str.length,
                                         ctx, level, false);
    return LXB_STATUS_OK;
}

lxb_status_t
lxb_html_serialize_ext_document_type_full_cb(const lxb_dom_document_type_t *doctype,
                                             const lxb_html_serialize_ext_document_type_t *cb,
                                             void *ctx, lxb_html_serialize_ext_opt_t opt,
                                             const lexbor_str_t *indent)
{
    return lxb_html_serialize_ext_document_type_full_cb_h(doctype, cb, ctx, opt,
                                                          indent, 0);
}

static lxb_status_t
lxb_html_serialize_ext_document_type_full_cb_h(const lxb_dom_document_type_t *doctype,
                                               const lxb_html_serialize_ext_document_type_t *cb,
                                               void *ctx, lxb_html_serialize_ext_opt_t opt,
                                               const lexbor_str_t *indent, size_t level)
{
    size_t length;
    const lxb_char_t *name;
    const lxb_dom_node_t *node = lxb_dom_interface_node(doctype);

    lxb_html_serialize_ext_indent_send(cb->indent, ctx, indent, level, opt);

    lxb_html_serialize_ext_boundary_send(cb->begin, node,
                                         lxb_html_serialize_ext_dt_start_str.data,
                                         lxb_html_serialize_ext_dt_start_str.length,
                                         ctx, level, false);

    name = lxb_dom_document_type_name(doctype, &length);

    if (length != 0) {
        lxb_html_serialize_ext_text_send(cb->ws, node,
                                         lxb_html_serialize_ext_ws_str.data,
                                         lxb_html_serialize_ext_ws_str.length, ctx);

        lxb_html_serialize_ext_text_send(cb->name, node, name, length, ctx);
    }

    if (doctype->public_id.data != NULL && doctype->public_id.length != 0) {
        lxb_html_serialize_ext_text_send(cb->ws, node,
                                         lxb_html_serialize_ext_ws_str.data,
                                         lxb_html_serialize_ext_ws_str.length, ctx);

        lxb_html_serialize_ext_text_send(cb->text_public, node,
                                         doctype->public_id.data,
                                         doctype->public_id.length, ctx);
    }

    if (doctype->system_id.data != NULL && doctype->system_id.length != 0) {
        lxb_html_serialize_ext_text_send(cb->ws, node,
                                         lxb_html_serialize_ext_ws_str.data,
                                         lxb_html_serialize_ext_ws_str.length, ctx);

        lxb_html_serialize_ext_text_send(cb->text_system, node,
                                         doctype->system_id.data,
                                         doctype->system_id.length, ctx);
    }

    lxb_html_serialize_ext_boundary_send(cb->end, node,
                                         lxb_html_serialize_ext_tag_end_str.data,
                                         lxb_html_serialize_ext_tag_end_str.length,
                                         ctx, level, false);
    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_document_cb_h(const lxb_dom_document_t *document,
                                     const lxb_html_serialize_ext_node_t *cb,
                                     void *ctx, lxb_html_serialize_ext_opt_t opt,
                                     const lexbor_str_t *indent, size_t level)
{
    const lxb_dom_node_t *node = lxb_dom_interface_node(document);
    static const lexbor_str_t document_str = lexbor_str("#document");

    if (!(opt & LXB_HTML_SERIALIZE_EXT_OPT_PRETTY)) {
        return LXB_STATUS_OK;
    }

    lxb_html_serialize_ext_indent_send(cb->indent, ctx, indent, level, opt);

    lxb_html_serialize_ext_boundary_send(cb->begin, node,
                                         lxb_html_serialize_ext_start_str.data,
                                         lxb_html_serialize_ext_start_str.length,
                                         ctx, level, false);

    lxb_html_serialize_ext_name_send(cb->name, node,
                                     document_str.data,
                                     document_str.length,
                                     ctx, false);

    lxb_html_serialize_ext_boundary_send(cb->end, node,
                                         lxb_html_serialize_ext_tag_end_str.data,
                                         lxb_html_serialize_ext_tag_end_str.length,
                                         ctx, level, false);
    return LXB_STATUS_OK;
}

const lexbor_str_t *
lxb_html_serialize_ext_opt_to_str(lxb_html_serialize_ext_opt_t opt)
{
    size_t length;

    length = sizeof(lxb_html_serialize_ext_opt_str)
                / sizeof(lxb_html_serialize_ext_opt_str_t);

    for (size_t i = 0; i < length; i++) {
        if (lxb_html_serialize_ext_opt_str[i].opt == opt) {
            return &lxb_html_serialize_ext_opt_str[i].data;
        }
    }

    return NULL;
}

lxb_html_serialize_ext_opt_t
lxb_html_serialize_ext_str_to_opt(const lxb_char_t *data, size_t length)
{
    size_t len;
    const lxb_html_serialize_ext_opt_str_t *entry;

    len = sizeof(lxb_html_serialize_ext_opt_str)
            / sizeof(lxb_html_serialize_ext_opt_str_t);

    for (size_t i = 0; i < len; i++) {
        entry = &lxb_html_serialize_ext_opt_str[i];

        if (entry->data.length == length
            && lexbor_str_data_ncasecmp(entry->data.data, data, length))
        {
            return entry->opt;
        }
    }

    return LXB_HTML_SERIALIZE_EXT_OPT_UNDEF;
}

static lxb_status_t
lxb_html_serialize_ext_walk(const lxb_dom_node_t *node,
                            const lxb_html_serialize_ext_t *cb, void *ctx,
                            const lexbor_str_t *indent,
                            lxb_html_serialize_ext_opt_t opt, size_t level)
{
    bool skip_it;
    uint8_t offset;
    lxb_status_t status;
    const lxb_dom_node_t *root = node;

    while (node != NULL) {
        status = lxb_html_serialize_ext_node_cb_h(node, cb, ctx, opt,
                                                  indent, level);
        if (status != LXB_STATUS_OK) {
            if (status != LXB_STATUS_SKIPPED) {
                return status;
            }
        }

        if (opt & LXB_HTML_SERIALIZE_EXT_OPT_PRETTY
            && status != LXB_STATUS_SKIPPED)
        {
            lxb_html_serialize_ext_newline_send(cb->newline, ctx, level);
        }

        if (lxb_html_tree_node_is(node, LXB_TAG_TEMPLATE)) {
            lxb_html_template_element_t *temp;

            temp = lxb_html_interface_template(node);

            if (temp->content != NULL) {
                if (temp->content->node.first_child != NULL)
                {
                    offset = 1;

                    if (opt & LXB_HTML_SERIALIZE_EXT_OPT_PRETTY) {
                        offset += 1;

                        status = lxb_html_serialize_ext_fragment(node, cb->node,
                                                                 ctx, opt, indent,
                                                                 (level + 1));
                        if (status != LXB_STATUS_OK) {
                            return status;
                        }

                        lxb_html_serialize_ext_newline_send(cb->newline,
                                                            ctx, level);
                    }

                    status = lxb_html_serialize_ext_tree_cb_h(&temp->content->node,
                                                              cb, ctx, opt, indent,
                                                              (level + offset), false);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }
                }
            }
        }

        skip_it = lxb_html_node_is_void(node);

        if (skip_it == false && node->first_child != NULL) {
            level += 1;
            node = node->first_child;
        }
        else {
            while(node != root && node->next == NULL)
            {
                if (node->type == LXB_DOM_NODE_TYPE_ELEMENT
                    && lxb_html_node_is_void(node) == false
                    && !(opt & LXB_HTML_SERIALIZE_EXT_OPT_WITHOUT_CLOSING))
                {
                    status = lxb_html_serialize_ext_element_wo_attr_cb_h(lxb_dom_interface_element(node),
                                                                         cb->node, ctx, opt, indent, level, true);
                    if (status != LXB_STATUS_OK) {
                        return status;
                    }

                    if (opt & LXB_HTML_SERIALIZE_EXT_OPT_PRETTY) {
                        lxb_html_serialize_ext_newline_send(cb->newline,
                                                            ctx, level);
                    }
                }

                level -= 1;
                node = node->parent;
            }

            if (node->type == LXB_DOM_NODE_TYPE_ELEMENT
                && lxb_html_node_is_void(node) == false
                && !(opt & LXB_HTML_SERIALIZE_EXT_OPT_WITHOUT_CLOSING))
            {
                status = lxb_html_serialize_ext_element_wo_attr_cb_h(lxb_dom_interface_element(node),
                                                                     cb->node, ctx, opt, indent, level, true);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                if (opt & LXB_HTML_SERIALIZE_EXT_OPT_PRETTY) {
                    lxb_html_serialize_ext_newline_send(cb->newline,
                                                        ctx, level);
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
lxb_html_serialize_ext_escaping_attribute(const lxb_dom_node_t *node,
                                          const lxb_dom_attr_t *attr,
                                          const lxb_char_t *data, size_t len,
                                          lxb_html_serialize_ext_attr_cb_f cb,
                                          void *ctx)
{
    const lxb_char_t *pos = data;
    const lxb_char_t *end = data + len;

    while (data != end) {
        switch (*data) {
                /* U+0026 AMPERSAND (&) */
            case 0x26:
                if (pos != data) {
                    lxb_html_serialize_ext_attr_send(cb, node, attr, pos,
                                                     (data - pos), ctx);
                }

                lxb_html_serialize_ext_attr_send(cb, node, attr,
                                                 lxb_html_serialize_ext_amp_str.data,
                                                 lxb_html_serialize_ext_amp_str.length,
                                                 ctx);
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
                    lxb_html_serialize_ext_attr_send(cb, node, attr, pos,
                                                     (data - pos), ctx);
                }

                lxb_html_serialize_ext_attr_send(cb, node, attr,
                                                 lxb_html_serialize_ext_nbsp_str.data,
                                                 lxb_html_serialize_ext_nbsp_str.length,
                                                 ctx);
                data += 2;
                pos = data;

                break;

                /* U+003C LESS-THAN SIGN (<) */
            case 0x3C:
                if (pos != data) {
                    lxb_html_serialize_ext_attr_send(cb, node, attr, pos,
                                                     (data - pos), ctx);
                }

                lxb_html_serialize_ext_attr_send(cb, node, attr,
                                                 lxb_html_serialize_ext_lt_str.data,
                                                 lxb_html_serialize_ext_lt_str.length,
                                                 ctx);
                data++;
                pos = data;

                break;

                /* U+003E GREATER-THAN SIGN (>) */
            case 0x3E:
                if (pos != data) {
                    lxb_html_serialize_ext_attr_send(cb, node, attr, pos,
                                                     (data - pos), ctx);
                }

                lxb_html_serialize_ext_attr_send(cb, node, attr,
                                                 lxb_html_serialize_ext_gt_str.data,
                                                 lxb_html_serialize_ext_gt_str.length,
                                                 ctx);
                data++;
                pos = data;

                break;

                /* U+0022 QUOTATION MARK (") */
            case 0x22:
                if (pos != data) {
                    lxb_html_serialize_ext_attr_send(cb, node, attr, pos,
                                                     (data - pos), ctx);
                }

                lxb_html_serialize_ext_attr_send(cb, node, attr,
                                                 lxb_html_serialize_ext_quot_str.data,
                                                 lxb_html_serialize_ext_quot_str.length,
                                                 ctx);
                data++;
                pos = data;

                break;

            default:
                data++;

                break;
        }
    }

    if (pos != data) {
        lxb_html_serialize_ext_attr_send(cb, node, attr, pos,
                                         (data - pos), ctx);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_escaping_string(const lxb_dom_node_t *node,
                                       const lxb_char_t *data, size_t len,
                                       lxb_html_serialize_ext_text_cb_f cb,
                                       void *ctx,
                                       lxb_html_serialize_ext_opt_t opt)
{
    const lxb_char_t *pos = data;
    const lxb_char_t *end = data + len;

    while (data != end) {
        switch (*data) {
                /* U+0026 AMPERSAND (&) */
            case 0x26:
                if (pos != data) {
                    lxb_html_serialize_ext_text_send(cb, node, pos,
                                                     (data - pos), ctx);
                }

                lxb_html_serialize_ext_text_send(cb, node,
                                                 lxb_html_serialize_ext_amp_str.data,
                                                 lxb_html_serialize_ext_amp_str.length,
                                                 ctx);
                data += 1;
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
                    lxb_html_serialize_ext_text_send(cb, node, pos,
                                                     (data - pos), ctx);
                }

                lxb_html_serialize_ext_text_send(cb, node,
                                                 lxb_html_serialize_ext_nbsp_str.data,
                                                 lxb_html_serialize_ext_nbsp_str.length,
                                                 ctx);
                data += 2;
                pos = data;

                break;

                /* U+003C LESS-THAN SIGN (<) */
            case 0x3C:
                if (pos != data) {
                    lxb_html_serialize_ext_text_send(cb, node, pos,
                                                     (data - pos), ctx);
                }

                lxb_html_serialize_ext_text_send(cb, node,
                                                 lxb_html_serialize_ext_lt_str.data,
                                                 lxb_html_serialize_ext_lt_str.length,
                                                 ctx);
                data += 1;
                pos = data;

                break;

                /* U+003E GREATER-THAN SIGN (>) */
            case 0x3E:
                if (pos != data) {
                    lxb_html_serialize_ext_text_send(cb, node, pos,
                                                     (data - pos), ctx);
                }

                lxb_html_serialize_ext_text_send(cb, node,
                                                 lxb_html_serialize_ext_gt_str.data,
                                                 lxb_html_serialize_ext_gt_str.length,
                                                 ctx);
                data += 1;
                pos = data;

                break;

                /*
                 * U+000A LINE FEED (LF)
                 * U+000D CARRIAGE RETURN (CR)
                 */
            case 0x0A:
            case 0x0D:
                if (opt & LXB_HTML_SERIALIZE_EXT_OPT_REPLACE_NEWLINE) {
                    if (pos != data) {
                        lxb_html_serialize_ext_text_send(cb, node, pos,
                                                         (data - pos), ctx);
                    }

                    lxb_html_serialize_ext_text_send(cb, node,
                                                     lxb_html_serialize_ext_ws_str.data,
                                                     lxb_html_serialize_ext_ws_str.length,
                                                     ctx);
                    data += 1;
                    pos = data;

                    break;
                }
                /* Fall Through. */

            default:
                data += 1;
                break;
        }
    }

    if (pos != data) {
        lxb_html_serialize_ext_text_send(cb, node, pos, (data - pos), ctx);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_opt_string(const lxb_dom_node_t *node,
                                  const lxb_char_t *data, size_t len,
                                  lxb_html_serialize_ext_text_cb_f cb,
                                  void *ctx,
                                  lxb_html_serialize_ext_opt_t opt)
{
    const lxb_char_t *pos = data;
    const lxb_char_t *end = data + len;

    while (data != end) {
        switch (*data) {
                /*
                 * U+000A LINE FEED (LF)
                 * U+000D CARRIAGE RETURN (CR)
                 */
            case 0x0A:
            case 0x0D:
                if (opt & LXB_HTML_SERIALIZE_EXT_OPT_REPLACE_NEWLINE) {
                    if (pos != data) {
                        lxb_html_serialize_ext_text_send(cb, node, pos,
                                                         (data - pos), ctx);
                    }

                    lxb_html_serialize_ext_text_send(cb, node,
                                                     lxb_html_serialize_ext_ws_str.data,
                                                     lxb_html_serialize_ext_ws_str.length,
                                                     ctx);
                    data += 1;
                    pos = data;

                    break;
                }
                /* Fall Through. */

            default:
                data += 1;
                break;
        }
    }

    if (pos != data) {
        lxb_html_serialize_ext_text_send(cb, node, pos, (data - pos), ctx);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_indent(lxb_html_serialize_ext_indent_cb_f cb, void *ctx,
                              const lexbor_str_t *indent, size_t level)
{
    lxb_status_t status;
    static const lexbor_str_t default_indent = lexbor_str(" ");

    if (indent == NULL || indent->data == NULL || indent->length == 0) {
        indent = &default_indent;
    }

    for (size_t i = 0; i < level; i++) {
        status = cb(indent->data, indent->length, ctx, level);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_fragment(const lxb_dom_node_t *node,
                                const lxb_html_serialize_ext_node_t *cb,
                                void *ctx, lxb_html_serialize_ext_opt_t opt,
                                const lexbor_str_t *indent, size_t level)
{
    static const lexbor_str_t tag_name = lexbor_str("#document-fragment");

    lxb_html_serialize_ext_indent_send(cb->indent, ctx, indent, level, opt);

    lxb_html_serialize_ext_boundary_send(cb->begin, node,
                                         lxb_html_serialize_ext_start_str.data,
                                         lxb_html_serialize_ext_start_str.length,
                                         ctx, level, false);

    lxb_html_serialize_ext_name_send(cb->name, node,
                                     tag_name.data, tag_name.length,
                                     ctx, false);

    lxb_html_serialize_ext_boundary_send(cb->end, node,
                                         lxb_html_serialize_ext_tag_end_str.data,
                                         lxb_html_serialize_ext_tag_end_str.length,
                                         ctx, level, false);
    return LXB_STATUS_OK;
}

/*
 * Node (Element/Document).
 */
static lxb_status_t
lxb_html_serialize_ext_indent_str(const lxb_char_t *data, size_t length,
                                  void *ctx, size_t level)
{
    lxb_char_t *ret;
    lxb_html_serialize_ext_ctx_t *ext_ctx = ctx;

    ret = lexbor_str_append(ext_ctx->str, ext_ctx->mraw, data, length);
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_begin_str(const lxb_dom_node_t *node,
                                 const lxb_char_t *data, size_t len,
                                 void *ctx, size_t level, bool is_close)
{
    lxb_char_t *ret;
    lxb_html_serialize_ext_ctx_t *ext_ctx = ctx;

    ret = lexbor_str_append(ext_ctx->str, ext_ctx->mraw, data, len);
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_ns_str(const lxb_dom_node_t *node,
                              const lxb_char_t *data, size_t len,
                              void *ctx, bool is_close)
{
    lxb_char_t *ret;
    lxb_status_t status;
    lxb_html_serialize_ext_ctx_t *ext_ctx = ctx;

    status = lxb_html_serialize_ext_end_str(node, data, len, ctx,
                                            0, is_close);
    if (status != LXB_STATUS_OK) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    ret = lexbor_str_append_one(ext_ctx->str, ext_ctx->mraw, ':');
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_name_str(const lxb_dom_node_t *node,
                                const lxb_char_t *data, size_t len,
                                void *ctx, bool is_close)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx,
                                          0, is_close);
}

static lxb_status_t
lxb_html_serialize_ext_end_str(const lxb_dom_node_t *node,
                               const lxb_char_t *data, size_t len,
                               void *ctx, size_t level, bool is_close)
{
    lxb_char_t *ret;
    lxb_html_serialize_ext_ctx_t *ext_ctx = ctx;

    ret = lexbor_str_append(ext_ctx->str, ext_ctx->mraw, data, len);
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

/*
 * Attribute.
 */
static lxb_status_t
lxb_html_serialize_ext_attr_ns_str(const lxb_dom_node_t *node,
                                   const lxb_dom_attr_t *attr,
                                   const lxb_char_t *data, size_t len,
                                   void *ctx)
{
    lxb_char_t *ret;
    lxb_html_serialize_ext_ctx_t *ext_ctx = ctx;

    ret = lexbor_str_append(ext_ctx->str, ext_ctx->mraw, data, len);
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    ret = lexbor_str_append_one(ext_ctx->str, ext_ctx->mraw, ':');
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_attr_name_str(const lxb_dom_node_t *node,
                                     const lxb_dom_attr_t *attr,
                                     const lxb_char_t *data, size_t len,
                                     void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_attr_value_before_str(const lxb_dom_node_t *node,
                                             const lxb_dom_attr_t *attr,
                                             const lxb_char_t *data,
                                             size_t len, void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_attr_value_str(const lxb_dom_node_t *node,
                                      const lxb_dom_attr_t *attr,
                                      const lxb_char_t *data,
                                      size_t len, void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_attr_value_after_str(const lxb_dom_node_t *node,
                                            const lxb_dom_attr_t *attr,
                                            const lxb_char_t *data,
                                            size_t len, void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_attr_ws_str(const lxb_dom_node_t *node,
                                   const lxb_char_t *data, size_t len,
                                   void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

/* Text */
static lxb_status_t
lxb_html_serialize_ext_text_begin_str(const lxb_dom_node_t *node,
                                      const lxb_char_t *data, size_t len,
                                      void *ctx, size_t level, bool is_close)
{
    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_text_text_str(const lxb_dom_node_t *node,
                                     const lxb_char_t *data, size_t len,
                                     void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_text_end_str(const lxb_dom_node_t *node,
                                    const lxb_char_t *data, size_t len,
                                    void *ctx, size_t level, bool is_close)
{
    return LXB_STATUS_OK;
}

/*
 * Comment.
 */
static lxb_status_t
lxb_html_serialize_ext_comment_begin_str(const lxb_dom_node_t *node,
                                         const lxb_char_t *data, size_t len,
                                         void *ctx, size_t level, bool is_close)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_comment_text_str(const lxb_dom_node_t *node,
                                        const lxb_char_t *data, size_t len,
                                        void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_comment_end_str(const lxb_dom_node_t *node,
                                       const lxb_char_t *data, size_t len,
                                       void *ctx, size_t level, bool is_close)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

/*
 * Processing instruction.
 */
static lxb_status_t
lxb_html_serialize_ext_pi_begin_str(const lxb_dom_node_t *node,
                                    const lxb_char_t *data, size_t len,
                                    void *ctx, size_t level, bool is_close)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_pi_target_str(const lxb_dom_node_t *node,
                                     const lxb_char_t *data, size_t len,
                                     void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_pi_middle_str(const lxb_dom_node_t *node,
                                     const lxb_char_t *data, size_t len,
                                     void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_pi_text_str(const lxb_dom_node_t *node,
                                   const lxb_char_t *data, size_t len,
                                   void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_pi_end_str(const lxb_dom_node_t *node,
                                  const lxb_char_t *data, size_t len,
                                  void *ctx, size_t level, bool is_close)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

/*
 * Document type.
 */
static lxb_status_t
lxb_html_serialize_ext_doctype_begin_str(const lxb_dom_node_t *node,
                                         const lxb_char_t *data, size_t len,
                                         void *ctx, size_t level, bool is_close)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_doctype_name_str(const lxb_dom_node_t *node,
                                        const lxb_char_t *data, size_t len,
                                        void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_doctype_public_str(const lxb_dom_node_t *node,
                                          const lxb_char_t *data, size_t len,
                                          void *ctx)
{
    lxb_char_t *ret;
    lxb_html_serialize_ext_ctx_t *ext_ctx = ctx;

    static const lexbor_str_t public_str = lexbor_str("PUBLIC \"");

    ret = lexbor_str_append(ext_ctx->str, ext_ctx->mraw,
                            public_str.data, public_str.length);
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    ret = lexbor_str_append(ext_ctx->str, ext_ctx->mraw, data, len);
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    ret = lexbor_str_append_one(ext_ctx->str, ext_ctx->mraw, '"');
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_doctype_system_str(const lxb_dom_node_t *node,
                                          const lxb_char_t *data, size_t len,
                                          void *ctx)
{
    lxb_char_t *ret;
    lxb_dom_document_type_t *dtype = lxb_dom_interface_document_type(node);
    lxb_html_serialize_ext_ctx_t *ext_ctx = ctx;

    static const lexbor_str_t system_str = lexbor_str("SYSTEM \"");

    if (dtype->public_id.data == NULL) {
        ret = lexbor_str_append(ext_ctx->str, ext_ctx->mraw,
                                system_str.data, system_str.length);
    }
    else {
        ret = lexbor_str_append_one(ext_ctx->str, ext_ctx->mraw, '"');
    }

    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    ret = lexbor_str_append(ext_ctx->str, ext_ctx->mraw, data, len);
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    ret = lexbor_str_append_one(ext_ctx->str, ext_ctx->mraw, '"');
    if (ret == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
lxb_html_serialize_ext_doctype_end_str(const lxb_dom_node_t *node,
                                       const lxb_char_t *data, size_t len,
                                       void *ctx, size_t level, bool is_close)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

static lxb_status_t
lxb_html_serialize_ext_doctype_ws_str(const lxb_dom_node_t *node,
                                      const lxb_char_t *data, size_t len,
                                      void *ctx)
{
    return lxb_html_serialize_ext_end_str(node, data, len, ctx, 0, false);
}

/*
 * New Line.
 */
static lxb_status_t
lxb_html_serialize_ext_newline_str(const lxb_char_t *data, size_t len,
                                   void *ctx, size_t level)
{
    return lxb_html_serialize_ext_indent_str(data, len, ctx, level);
}
