/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/core/core.h>
#include <lexbor/html/html.h>
#include <lexbor/css/css.h>
#include <lexbor/selectors/selectors.h>
#include <lexbor/style/style.h>

#include <emscripten.h>


typedef struct {
    lexbor_array_t *nodes;
}
wasm_selectors_ctx_t;


/*
 * HTML Serialize.
 */
#define wasm_document_str_append(wasm, data, length)                           \
    do {                                                                       \
        lxb_status_t status;                                                   \
        status = wasm_document_serialize_append((wasm),                        \
                                                (const lxb_char_t *) (data),   \
                                                (length));                     \
        if (status != LXB_STATUS_OK) {                                         \
            return status;                                                     \
        }                                                                      \
    }                                                                          \
    while (0)

#define wasm_document_str_append_return(wasm, data, length)                    \
    wasm_document_serialize_append((wasm), (const lxb_char_t *) (data), (length))


typedef struct {
    lexbor_mraw_t *mraw;
    lexbor_str_t  str;
    int           last_level;
    bool          as_tree;
}
wasm_serialize_ctx_t;


/* Selectors. */
static lxb_status_t
wasm_selectors_node_callback(lxb_dom_node_t *node,
                             lxb_css_selector_specificity_t spec, void *ctx);

/* Node (Element/Document). */
static lxb_status_t
wasm_serialize_cb_indent(const lxb_char_t *data, size_t length,
                         void *ctx, size_t level);
static lxb_status_t
wasm_serialize_cb_begin(const lxb_dom_node_t *node,
                        const lxb_char_t *data, size_t len,
                        void *ctx, size_t level, bool is_close);
static lxb_status_t
wasm_serialize_cb_ns(const lxb_dom_node_t *node,
                     const lxb_char_t *data, size_t len,
                     void *ctx, bool is_close);
static lxb_status_t
wasm_serialize_cb_name(const lxb_dom_node_t *node,
                       const lxb_char_t *data, size_t len,
                       void *ctx, bool is_close);
static lxb_status_t
wasm_serialize_cb_end(const lxb_dom_node_t *node,
                      const lxb_char_t *data, size_t len,
                      void *ctx, size_t level, bool is_close);
/* Attribute. */
static lxb_status_t
wasm_serialize_cb_attr_ns(const lxb_dom_node_t *node,
                          const lxb_dom_attr_t *attr,
                          const lxb_char_t *data, size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_attr_name(const lxb_dom_node_t *node,
                            const lxb_dom_attr_t *attr,
                            const lxb_char_t *data, size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_attr_value_before(const lxb_dom_node_t *node,
                                    const lxb_dom_attr_t *attr,
                                    const lxb_char_t *data,
                                    size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_attr_value(const lxb_dom_node_t *node,
                             const lxb_dom_attr_t *attr,
                             const lxb_char_t *data,
                             size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_attr_value_after(const lxb_dom_node_t *node,
                                   const lxb_dom_attr_t *attr,
                                   const lxb_char_t *data,
                                   size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_attr_ws(const lxb_dom_node_t *node,
                          const lxb_char_t *data, size_t len, void *ctx);
/* Text. */
static lxb_status_t
wasm_serialize_cb_text_begin(const lxb_dom_node_t *node,
                             const lxb_char_t *data, size_t len,
                             void *ctx, size_t level, bool is_close);
static lxb_status_t
wasm_serialize_cb_text_text(const lxb_dom_node_t *node,
                            const lxb_char_t *data, size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_text_end(const lxb_dom_node_t *node,
                           const lxb_char_t *data, size_t len,
                           void *ctx, size_t level, bool is_close);
/* Comment. */
static lxb_status_t
wasm_serialize_cb_comment_begin(const lxb_dom_node_t *node,
                                const lxb_char_t *data, size_t len,
                                void *ctx, size_t level, bool is_close);
static lxb_status_t
wasm_serialize_cb_comment_text(const lxb_dom_node_t *node,
                               const lxb_char_t *data, size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_comment_end(const lxb_dom_node_t *node,
                              const lxb_char_t *data, size_t len,
                              void *ctx, size_t level, bool is_close);
/* Processing instruction. */
static lxb_status_t
wasm_serialize_cb_pi_begin(const lxb_dom_node_t *node,
                           const lxb_char_t *data, size_t len,
                           void *ctx, size_t level, bool is_close);
static lxb_status_t
wasm_serialize_cb_pi_target(const lxb_dom_node_t *node,
                            const lxb_char_t *data, size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_pi_middle(const lxb_dom_node_t *node,
                            const lxb_char_t *data, size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_pi_text(const lxb_dom_node_t *node,
                          const lxb_char_t *data, size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_pi_end(const lxb_dom_node_t *node,
                         const lxb_char_t *data, size_t len,
                         void *ctx, size_t level, bool is_close);
/* Document Type. */
static lxb_status_t
wasm_serialize_cb_doctype_begin(const lxb_dom_node_t *node,
                                const lxb_char_t *data, size_t len,
                                void *ctx, size_t level, bool is_close);
static lxb_status_t
wasm_serialize_cb_doctype_name(const lxb_dom_node_t *node,
                               const lxb_char_t *data, size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_doctype_public(const lxb_dom_node_t *node,
                                 const lxb_char_t *data, size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_doctype_system(const lxb_dom_node_t *node,
                                 const lxb_char_t *data, size_t len, void *ctx);
static lxb_status_t
wasm_serialize_cb_doctype_end(const lxb_dom_node_t *node,
                              const lxb_char_t *data, size_t len,
                              void *ctx, size_t level, bool is_close);
static lxb_status_t
wasm_serialize_cb_doctype_ws(const lxb_dom_node_t *node,
                             const lxb_char_t *data, size_t len, void *ctx);
/* New Line. */
static lxb_status_t
wasm_serialize_cb_newline(const lxb_char_t *data, size_t len,
                          void *ctx, size_t level);


static const lxb_html_serialize_ext_node_t wasm_serialize_cb_node = {
    .indent = wasm_serialize_cb_indent,
    .begin = wasm_serialize_cb_begin,
    .ns = wasm_serialize_cb_ns,
    .name = wasm_serialize_cb_name,
    .end = wasm_serialize_cb_end
};

static const lxb_html_serialize_ext_attr_t wasm_serialize_cb_attr = {
    .ns = wasm_serialize_cb_attr_ns,
    .name = wasm_serialize_cb_attr_name,
    .value_before = wasm_serialize_cb_attr_value_before,
    .value = wasm_serialize_cb_attr_value,
    .value_after = wasm_serialize_cb_attr_value_after,
    .ws = wasm_serialize_cb_attr_ws
};

static const lxb_html_serialize_ext_text_t wasm_serialize_cb_text = {
    .indent = wasm_serialize_cb_indent,
    .begin = wasm_serialize_cb_text_begin,
    .text = wasm_serialize_cb_text_text,
    .end = wasm_serialize_cb_text_end
};

static const lxb_html_serialize_ext_comment_t wasm_serialize_cb_comment = {
    .indent = wasm_serialize_cb_indent,
    .begin = wasm_serialize_cb_comment_begin,
    .text = wasm_serialize_cb_comment_text,
    .end = wasm_serialize_cb_comment_end
};

static const lxb_html_serialize_ext_processing_instruction_t wasm_serialize_cb_processing_instruction = {
    .indent = wasm_serialize_cb_indent,
    .begin = wasm_serialize_cb_pi_begin,
    .target = wasm_serialize_cb_pi_target,
    .middle = wasm_serialize_cb_pi_middle,
    .text = wasm_serialize_cb_pi_text,
    .end = wasm_serialize_cb_pi_end
};

static const lxb_html_serialize_ext_document_type_t wasm_serialize_cb_document_type = {
    .indent = wasm_serialize_cb_indent,
    .begin = wasm_serialize_cb_doctype_begin,
    .name = wasm_serialize_cb_doctype_name,
    .text_public = wasm_serialize_cb_doctype_public,
    .text_system = wasm_serialize_cb_doctype_system,
    .end = wasm_serialize_cb_doctype_end,
    .ws = wasm_serialize_cb_doctype_ws
};

static const lxb_html_serialize_ext_node_t wasm_serialize_cb_document = {
    .indent = wasm_serialize_cb_indent,
    .begin = wasm_serialize_cb_begin,
    .end = wasm_serialize_cb_end,
    .name = wasm_serialize_cb_name
};

static const lxb_html_serialize_ext_t wasm_serialize_cb = {
    .node = &wasm_serialize_cb_node,
    .attr = &wasm_serialize_cb_attr,
    .text = &wasm_serialize_cb_text,
    .comment = &wasm_serialize_cb_comment,
    .processing_instruction = &wasm_serialize_cb_processing_instruction,
    .document_type = &wasm_serialize_cb_document_type,
    .document = &wasm_serialize_cb_document,
    .newline = wasm_serialize_cb_newline
};


/*
 * HTML.
 */
/*
 * Document.
 */
EMSCRIPTEN_KEEPALIVE lxb_html_document_t *
wasm_document_create(void)
{
    return lxb_html_document_create();
}

EMSCRIPTEN_KEEPALIVE void
wasm_document_destroy(lxb_html_document_t *document)
{
    if (document != NULL) {
        lxb_html_document_destroy(document);
    }
}

EMSCRIPTEN_KEEPALIVE lxb_status_t
wasm_document_parse(lxb_html_document_t *document,
                    const lxb_char_t *html, size_t length)
{
    if (document == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    return lxb_html_document_parse(document, html, length);
}

EMSCRIPTEN_KEEPALIVE void
wasm_document_destroy_string(lxb_html_document_t *document,
                             lxb_char_t *data)
{
    if (document != NULL) {
        lxb_dom_document_destroy_text(lxb_dom_interface_node(document)->owner_document,
                                      data);
    }
}

/*
 * Document Settings.
 */
EMSCRIPTEN_KEEPALIVE void
wasm_document_opt_set(lxb_html_document_t *document,
                      lxb_dom_document_opt_t opt)
{
    if (document != NULL) {
        lxb_html_document_dom_opt_set(document, opt);
    }
}

EMSCRIPTEN_KEEPALIVE lxb_dom_document_opt_t
wasm_document_opt_get(const lxb_html_document_t *document)
{
    if (document == NULL) {
        return LXB_DOM_DOCUMENT_OPT_UNDEF;
    }

    return lxb_html_document_dom_opt(document);
}

EMSCRIPTEN_KEEPALIVE void
wasm_document_scripting_set(lxb_html_document_t *document, bool scripting)
{
    if (document != NULL) {
        lxb_html_document_scripting_set(document, scripting);
    }
}

/*
 * Node.
 */
EMSCRIPTEN_KEEPALIVE void
wasm_node_destroy_string(lxb_dom_node_t *node, lxb_char_t *data)
{
    if (node != NULL) {
        lxb_dom_document_destroy_text(node->owner_document, data);
    }
}

/*
 * Document Style.
 */
EMSCRIPTEN_KEEPALIVE lxb_status_t
wasm_document_style_init(lxb_html_document_t *document)
{
    return lxb_style_init(document);
}

EMSCRIPTEN_KEEPALIVE void
wasm_document_style_destroy(lxb_html_document_t *document)
{
    lxb_style_destroy(document);
}

/*
 * CSS.
 */
EMSCRIPTEN_KEEPALIVE lxb_css_parser_t *
wasm_css_parser_create(void)
{
    lxb_status_t status;
    lxb_css_parser_t *parser;

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    status = lxb_css_parser_selectors_init(parser);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    return parser;

failed:

    lxb_css_parser_destroy(parser, true);

    return NULL;
}

EMSCRIPTEN_KEEPALIVE void
wasm_css_parser_destroy(lxb_css_parser_t *parser)
{
    lxb_css_parser_selectors_destroy(parser);
    lxb_css_parser_destroy(parser, true);
}

EMSCRIPTEN_KEEPALIVE lxb_css_selector_list_t *
wasm_css_parse_selectors(lxb_css_parser_t *parser,
                         const lxb_char_t *selector, size_t length)
{
    lxb_css_selector_list_t *list;

    list = lxb_css_selectors_parse(parser, selector, length);
    if (list == NULL) {
        return NULL;
    }

    return list;
}

EMSCRIPTEN_KEEPALIVE void
wasm_css_selector_list_destroy(lxb_css_selector_list_t *list)
{
    lxb_css_selector_list_destroy(list);
}

EMSCRIPTEN_KEEPALIVE void
wasm_css_memory_destroy(lxb_css_parser_t *parser)
{
    lxb_css_memory_destroy(parser->memory, true);
}

/*
 * Selectors.
 */
EMSCRIPTEN_KEEPALIVE lxb_selectors_t *
wasm_selectors_create(void)
{
    lxb_status_t status;
    lxb_selectors_t *selectors;

    selectors = lxb_selectors_create();
    status = lxb_selectors_init(selectors);
    if (status != LXB_STATUS_OK) {
        lxb_selectors_destroy(selectors, true);
        return NULL;
    }

    lxb_selectors_opt_set(selectors,
                          LXB_SELECTORS_OPT_MATCH_FIRST);
    return selectors;
}

EMSCRIPTEN_KEEPALIVE void
wasm_selectors_destroy(lxb_selectors_t *selectors)
{
    lxb_selectors_destroy(selectors, true);
}

EMSCRIPTEN_KEEPALIVE wasm_selectors_ctx_t *
wasm_selectors_context_create(void)
{
    lxb_status_t status;
    wasm_selectors_ctx_t *context;

    context = lexbor_calloc(1, sizeof(wasm_selectors_ctx_t));
    if (context == NULL) {
        return NULL;
    }

    context->nodes = lexbor_array_create();
    status = lexbor_array_init(context->nodes, 128);
    if (status != LXB_STATUS_OK) {
        lexbor_array_destroy(context->nodes, true);
        lexbor_free(context);
    }

    return context;
}

EMSCRIPTEN_KEEPALIVE void
wasm_selectors_context_clean(wasm_selectors_ctx_t *context)
{
    if (context != NULL) {
        lexbor_array_clean(context->nodes);
    }
}

EMSCRIPTEN_KEEPALIVE void
wasm_selectors_context_destroy(wasm_selectors_ctx_t *context)
{
    if (context->nodes != NULL) {
        lexbor_array_destroy(context->nodes, true);
    }

    lexbor_free(context);
}

EMSCRIPTEN_KEEPALIVE size_t
wasm_selectors_context_count(wasm_selectors_ctx_t *context)
{
    if (context == NULL) {
        return 0;
    }

    return lexbor_array_length(context->nodes);
}

EMSCRIPTEN_KEEPALIVE lxb_dom_node_t *
wasm_selectors_context_get_node(wasm_selectors_ctx_t *context, size_t idx)
{
    if (context == NULL) {
        return NULL;
    }

    return lexbor_array_get(context->nodes, idx);
}

EMSCRIPTEN_KEEPALIVE lxb_status_t
wasm_selectors_match(lxb_selectors_t *selectors, lxb_css_selector_list_t *list,
                     lxb_dom_node_t *root, wasm_selectors_ctx_t *context)
{
    if (selectors == NULL || list == NULL || root == NULL || context == NULL) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    return lxb_selectors_find(selectors, root, list,
                              wasm_selectors_node_callback, context);
}

static lxb_status_t
wasm_selectors_node_callback(lxb_dom_node_t *node,
                             lxb_css_selector_specificity_t spec, void *ctx)
{
    wasm_selectors_ctx_t *context = ctx;

    return lexbor_array_push(context->nodes, node);
}

/*
 * Document Serialize.
 */
EMSCRIPTEN_KEEPALIVE lxb_char_t *
wasm_document_serialize(const lxb_dom_node_t *node,
                        lxb_html_serialize_ext_opt_t opt, bool as_tree)
{
    lxb_status_t status;
    wasm_serialize_ctx_t wasm;

    wasm.mraw = node->owner_document->mraw;
    wasm.last_level = -1;
    wasm.as_tree = as_tree;

    (void) lexbor_str_init(&wasm.str, wasm.mraw, 4096);
    if (wasm.str.data == NULL) {
        return NULL;
    }

    status = lxb_html_serialize_ext_tree_cb(node, &wasm_serialize_cb,
                                            &wasm, opt, NULL, true);
    if (status != LXB_STATUS_OK) {
        lxb_dom_document_destroy_text(node->owner_document, wasm.str.data);
        return NULL;
    }

    return wasm.str.data;
}

EMSCRIPTEN_KEEPALIVE const lxb_char_t *
wasm_document_serialize_opt_to_str(lxb_html_serialize_ext_opt_t opt)
{
    const lexbor_str_t *str;

    str = lxb_html_serialize_ext_opt_to_str(opt);
    if (str == NULL) {
        return NULL;
    }

    return str->data;
}

static lxb_status_t
wasm_document_serialize_append(wasm_serialize_ctx_t *wasm,
                               const lxb_char_t *data, size_t length)
{
    lxb_char_t *ret = lexbor_str_append(&wasm->str, wasm->mraw, data, length);

    return (ret != NULL) ? LXB_STATUS_OK : LXB_STATUS_ERROR_MEMORY_ALLOCATION;
}

/*
 * Append data with HTML escaping: &, <, >, ".
 */
static lxb_status_t
buf_append_escaped(wasm_serialize_ctx_t *wasm,
                   const lxb_char_t *data, size_t len)
{
    const lxb_char_t *pos = data;
    const lxb_char_t *end = data + len;

    static const lexbor_str_t amp_str = lexbor_str("&amp;");
    static const lexbor_str_t lt_str = lexbor_str("&lt;");
    static const lexbor_str_t gt_str = lexbor_str("&gt;");
    static const lexbor_str_t quot_str = lexbor_str("&quot;");

    while (data != end) {
        switch (*data) {
            case '&':
                if (pos != data) {
                    wasm_document_str_append(wasm, pos, (size_t) (data - pos));
                }
                wasm_document_str_append(wasm, amp_str.data, amp_str.length);

                data += 1;
                pos = data;
                break;

            case '<':
                if (pos != data) {
                    wasm_document_str_append(wasm, pos, (size_t) (data - pos));
                }
                wasm_document_str_append(wasm, lt_str.data, lt_str.length);

                data += 1;
                pos = data;
                break;

            case '>':
                if (pos != data) {
                    wasm_document_str_append(wasm, pos, (size_t) (data - pos));
                }
                wasm_document_str_append(wasm, gt_str.data, gt_str.length);

                data += 1;
                pos = data;
                break;

            case '"':
                if (pos != data) {
                    wasm_document_str_append(wasm, pos, (size_t) (data - pos));
                }
                wasm_document_str_append(wasm, quot_str.data, quot_str.length);

                data += 1;
                pos = data;
                break;

            default:
                data += 1;
                break;
        }
    }

    if (pos != data) {
        wasm_document_str_append(wasm, pos, (size_t) (data - pos));
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
wasm_document_serialize_span(wasm_serialize_ctx_t *wasm,
                             const lxb_char_t *mclass, size_t mlen)
{
    static const lexbor_str_t span_open_str = lexbor_str("<span");
    static const lexbor_str_t class_str = lexbor_str(" class=\"");
    static const lexbor_str_t qo_str = lexbor_str("\"");
    static const lexbor_str_t cl_str = lexbor_str(">");

    /* <span */
    wasm_document_str_append(wasm, span_open_str.data, span_open_str.length);

    /* class */
    if (mclass != NULL && mlen > 0) {
        wasm_document_str_append(wasm, class_str.data, class_str.length);
        wasm_document_str_append(wasm, mclass, mlen);
        wasm_document_str_append(wasm, qo_str.data, qo_str.length);
    }

    /* > */
    return wasm_document_str_append_return(wasm, cl_str.data, cl_str.length);
}

static lxb_status_t
wasm_document_serialize_span_close(wasm_serialize_ctx_t *wasm)
{
    lxb_char_t *ret;

    static const lexbor_str_t span_close_str = lexbor_str("</span>");

    ret = lexbor_str_append(&wasm->str, wasm->mraw,
                            span_close_str.data, span_close_str.length);

    return (ret != NULL) ? LXB_STATUS_OK : LXB_STATUS_ERROR_MEMORY_ALLOCATION;
}

static lxb_status_t
wasm_document_serialize_ul(wasm_serialize_ctx_t *wasm,
                           const lxb_char_t *mclass, size_t mlen)
{
    static const lexbor_str_t li_open_str = lexbor_str("<ul");
    static const lexbor_str_t class_str = lexbor_str(" class=\"");
    static const lexbor_str_t qo_str = lexbor_str("\"");
    static const lexbor_str_t cl_str = lexbor_str(">");

    if (wasm->as_tree) {
        /* <ul */
        wasm_document_str_append(wasm, li_open_str.data, li_open_str.length);

        /* class */
        if (mclass != NULL && mlen > 0) {
            wasm_document_str_append(wasm, class_str.data, class_str.length);
            wasm_document_str_append(wasm, mclass, mlen);
            wasm_document_str_append(wasm, qo_str.data, qo_str.length);
        }


        /* > */
        wasm_document_str_append(wasm, cl_str.data, cl_str.length);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
wasm_document_serialize_ul_close(wasm_serialize_ctx_t *wasm)
{
    lxb_char_t *ret;

    static const lexbor_str_t li_close_str = lexbor_str("</ul>");

    ret = lexbor_str_append(&wasm->str, wasm->mraw,
                            li_close_str.data, li_close_str.length);

    return (ret != NULL) ? LXB_STATUS_OK : LXB_STATUS_ERROR_MEMORY_ALLOCATION;
}

static lxb_status_t
wasm_document_serialize_li_span(wasm_serialize_ctx_t *wasm,
                                const lxb_dom_node_t *node,
                                const lxb_char_t *mclass, size_t mlen,
                                bool is_close)
{
    size_t length;
    lxb_char_t buf[128];

    static const lexbor_str_t li_open_str = lexbor_str("<li");
    static const lexbor_str_t span_open_str = lexbor_str("<span");
    static const lexbor_str_t id_open_str = lexbor_str(" id=\"o-");
    static const lexbor_str_t id_close_str = lexbor_str(" id=\"c-");
    static const lexbor_str_t class_str = lexbor_str(" class=\"");
    static const lexbor_str_t qo_str = lexbor_str("\"");
    static const lexbor_str_t cl_str = lexbor_str(">");
    static const lexbor_str_t arrow_str = lexbor_str("<span></span>");

    if (wasm->as_tree) {
        /* <li */
        wasm_document_str_append(wasm, li_open_str.data, li_open_str.length);
    }
    else {
        wasm_document_str_append(wasm, span_open_str.data, span_open_str.length);
    }

    /* id */
    if (!is_close) {
        wasm_document_str_append(wasm, id_open_str.data, id_open_str.length);
    }
    else {
        wasm_document_str_append(wasm, id_close_str.data, id_close_str.length);
    }

    length = lexbor_conv_int64_to_data((int64_t) (uintptr_t) node,
                                       buf, sizeof(buf));
    if (length == 0) {
        return LXB_STATUS_ERROR;
    }

    wasm_document_str_append(wasm, buf, length);
    wasm_document_str_append(wasm, qo_str.data, qo_str.length);

    /* class */
    if (mclass != NULL && mlen > 0) {
        wasm_document_str_append(wasm, class_str.data, class_str.length);
        wasm_document_str_append(wasm, mclass, mlen);
        wasm_document_str_append(wasm, qo_str.data, qo_str.length);
    }

    /* > */
    wasm_document_str_append(wasm, cl_str.data, cl_str.length);

    if (wasm->as_tree) {
        wasm_document_str_append(wasm, arrow_str.data, arrow_str.length);
        return wasm_document_serialize_span(wasm, mclass, mlen);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
wasm_document_serialize_li_close(wasm_serialize_ctx_t *wasm)
{
    lxb_char_t *ret;

    static const lexbor_str_t li_close_str = lexbor_str("</li>");

    if (!wasm->as_tree) {
        return LXB_STATUS_OK;
    }

    ret = lexbor_str_append(&wasm->str, wasm->mraw,
                            li_close_str.data, li_close_str.length);

    return (ret != NULL) ? LXB_STATUS_OK : LXB_STATUS_ERROR_MEMORY_ALLOCATION;
}

static lxb_status_t
wasm_document_serialize_level(wasm_serialize_ctx_t *wasm, size_t level)
{
    lxb_status_t status;

    if (!wasm->as_tree) {
        goto done;
    }

    if (wasm->last_level < (int) level) {
        status = wasm_document_serialize_ul(wasm, NULL, 0);
    }
    else if (wasm->last_level > (int) level) {
        status = wasm_document_serialize_li_close(wasm);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        status = wasm_document_serialize_ul_close(wasm);
    }
    else {
        status = wasm_document_serialize_li_close(wasm);
    }

    if (status != LXB_STATUS_OK) {
        return status;
    }

done:

    wasm->last_level = (int) level;

    return LXB_STATUS_OK;
}

/*
 * Node (Element/Document).
 */
static lxb_status_t
wasm_serialize_cb_indent(const lxb_char_t *data, size_t length,
                         void *ctx, size_t level)
{
    return LXB_STATUS_OK;
}

static lxb_status_t
wasm_serialize_cb_begin(const lxb_dom_node_t *node,
                        const lxb_char_t *data, size_t len,
                        void *ctx, size_t level, bool is_close)
{
    lxb_status_t status;
    wasm_serialize_ctx_t *wasm = ctx;

    static const lexbor_str_t element_str = lexbor_str("element");

    status = wasm_document_serialize_level(wasm, level);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = wasm_document_serialize_li_span(wasm, node, element_str.data,
                                             element_str.length, is_close);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return buf_append_escaped(ctx, data, len);
}

static lxb_status_t
wasm_serialize_cb_ns(const lxb_dom_node_t *node,
                     const lxb_char_t *data, size_t len,
                     void *ctx, bool is_close)
{
    lxb_status_t status;
    wasm_serialize_ctx_t *wasm = ctx;

    static const lexbor_str_t ns_str = lexbor_str("ns");

    status = wasm_document_serialize_span(wasm, ns_str.data, ns_str.length);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = buf_append_escaped(wasm, data, len);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = wasm_document_serialize_span_close(wasm);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return wasm_document_str_append_return(ctx, ":", 1);
}

static lxb_status_t
wasm_serialize_cb_name(const lxb_dom_node_t *node,
                       const lxb_char_t *data, size_t len,
                       void *ctx, bool is_close)
{
    lxb_status_t status;
    const lxb_dom_element_t *el;
    wasm_serialize_ctx_t *wasm = ctx;

    static const lexbor_str_t name_str = lexbor_str("name");
    static const lexbor_str_t attr_str = lexbor_str("attrs");

    status = wasm_document_serialize_span(wasm, name_str.data, name_str.length);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = buf_append_escaped(wasm, data, len);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = wasm_document_serialize_span_close(wasm);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (node->type != LXB_DOM_NODE_TYPE_ELEMENT) {
        return LXB_STATUS_OK;
    }

    el = lxb_dom_interface_element(node);

    if (el->first_attr == NULL) {
        return LXB_STATUS_OK;
    }

    return wasm_document_serialize_span(ctx, attr_str.data, attr_str.length);
}

static lxb_status_t
wasm_serialize_cb_end(const lxb_dom_node_t *node,
                      const lxb_char_t *data, size_t len,
                      void *ctx, size_t level, bool is_close)
{
    lxb_status_t status;
    const lxb_dom_element_t *el;
    wasm_serialize_ctx_t *wasm = ctx;

    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        el = lxb_dom_interface_element(node);

        if (el->first_attr != NULL) {
            status = wasm_document_serialize_span_close(wasm);
            if (status != LXB_STATUS_OK) {
                return status;
            }
        }
    }

    status = buf_append_escaped(wasm, data, len);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return wasm_document_serialize_span_close(wasm);
}

/*
 * Attribute.
 */
static lxb_status_t
wasm_serialize_cb_attr_ns(const lxb_dom_node_t *node,
                          const lxb_dom_attr_t *attr,
                          const lxb_char_t *data, size_t len, void *ctx)
{
    return wasm_serialize_cb_ns(node, data, len, ctx, false);
}

static lxb_status_t
wasm_serialize_cb_attr_name(const lxb_dom_node_t *node,
                            const lxb_dom_attr_t *attr,
                            const lxb_char_t *data, size_t len, void *ctx)
{
    lxb_status_t status;
    wasm_serialize_ctx_t *wasm = ctx;

    static const lexbor_str_t name_str = lexbor_str("name");

    status = wasm_document_serialize_span(wasm, name_str.data, name_str.length);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = buf_append_escaped(wasm, data, len);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return wasm_document_serialize_span_close(wasm);
}

static lxb_status_t
wasm_serialize_cb_attr_value_before(const lxb_dom_node_t *node,
                                    const lxb_dom_attr_t *attr,
                                    const lxb_char_t *data,
                                    size_t len, void *ctx)
{
    lxb_status_t status;

    static const lexbor_str_t value_str = lexbor_str("value");

    status = buf_append_escaped(ctx, data, len);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return wasm_document_serialize_span(ctx, value_str.data, value_str.length);
}

static lxb_status_t
wasm_serialize_cb_attr_value(const lxb_dom_node_t *node,
                             const lxb_dom_attr_t *attr,
                             const lxb_char_t *data,
                             size_t len, void *ctx)
{
    return buf_append_escaped(ctx, data, len);
}

static lxb_status_t
wasm_serialize_cb_attr_value_after(const lxb_dom_node_t *node,
                                   const lxb_dom_attr_t *attr,
                                   const lxb_char_t *data,
                                   size_t len, void *ctx)
{
    lxb_status_t status;

    status = wasm_document_serialize_span_close(ctx);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return wasm_document_str_append_return(ctx, data, len);
}

static lxb_status_t
wasm_serialize_cb_attr_ws(const lxb_dom_node_t *node,
                          const lxb_char_t *data, size_t len, void *ctx)
{
    return wasm_document_str_append_return(ctx, data, len);
}

/* Text */
static lxb_status_t
wasm_serialize_cb_text_begin(const lxb_dom_node_t *node,
                             const lxb_char_t *data, size_t len,
                             void *ctx, size_t level, bool is_close)
{
    lxb_status_t status;
    wasm_serialize_ctx_t *wasm = ctx;

    static const lexbor_str_t text_str = lexbor_str("text");
    static const lexbor_str_t qo_str = lexbor_str("\"");

    status = wasm_document_serialize_level(wasm, level);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = wasm_document_serialize_li_span(wasm, node, text_str.data,
                                             text_str.length, is_close);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (wasm->as_tree) {
        return wasm_document_str_append_return(ctx, qo_str.data, qo_str.length);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
wasm_serialize_cb_text_text(const lxb_dom_node_t *node,
                            const lxb_char_t *data, size_t len, void *ctx)
{
    return buf_append_escaped(ctx, data, len);
}

static lxb_status_t
wasm_serialize_cb_text_end(const lxb_dom_node_t *node,
                           const lxb_char_t *data, size_t len,
                           void *ctx, size_t level, bool is_close)
{
    wasm_serialize_ctx_t *wasm = ctx;

    static const lexbor_str_t qo_str = lexbor_str("\"");

    if (wasm->as_tree) {
        wasm_document_str_append(ctx, qo_str.data, qo_str.length);
    }

    return wasm_document_serialize_span_close(ctx);
}

/*
 * Comment.
 */
static lxb_status_t
wasm_serialize_cb_comment_begin(const lxb_dom_node_t *node,
                                const lxb_char_t *data, size_t len,
                                void *ctx, size_t level, bool is_close)
{
    lxb_status_t status;
    wasm_serialize_ctx_t *wasm = ctx;

    static const lexbor_str_t comment_str = lexbor_str("comment");

    status = wasm_document_serialize_level(wasm, level);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = wasm_document_serialize_li_span(wasm, node, comment_str.data,
                                             comment_str.length, is_close);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return buf_append_escaped(ctx, data, len);
}

static lxb_status_t
wasm_serialize_cb_comment_text(const lxb_dom_node_t *node,
                               const lxb_char_t *data, size_t len,
                               void *ctx)
{
    return buf_append_escaped(ctx, data, len);
}

static lxb_status_t
wasm_serialize_cb_comment_end(const lxb_dom_node_t *node,
                              const lxb_char_t *data, size_t len,
                              void *ctx, size_t level, bool is_close)
{
    lxb_status_t status = buf_append_escaped(ctx, data, len);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return wasm_document_serialize_span_close(ctx);
}

/*
 * Processing instruction.
 */
static lxb_status_t
wasm_serialize_cb_pi_begin(const lxb_dom_node_t *node,
                           const lxb_char_t *data, size_t len,
                           void *ctx, size_t level, bool is_close)
{
    lxb_status_t status;
    wasm_serialize_ctx_t *wasm = ctx;

    static const lexbor_str_t pi_str = lexbor_str("proc-inst");

    status = wasm_document_serialize_level(wasm, level);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = wasm_document_serialize_li_span(wasm, node, pi_str.data,
                                             pi_str.length, is_close);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return buf_append_escaped(ctx, data, len);
}

static lxb_status_t
wasm_serialize_cb_pi_target(const lxb_dom_node_t *node,
                            const lxb_char_t *data, size_t len,
                            void *ctx)
{
    return buf_append_escaped(ctx, data, len);
}

static lxb_status_t
wasm_serialize_cb_pi_middle(const lxb_dom_node_t *node,
                            const lxb_char_t *data, size_t len,
                            void *ctx)
{
    return wasm_document_str_append_return(ctx, data, len);
}

static lxb_status_t
wasm_serialize_cb_pi_text(const lxb_dom_node_t *node,
                          const lxb_char_t *data, size_t len,
                          void *ctx)
{
    return buf_append_escaped(ctx, data, len);
}

static lxb_status_t
wasm_serialize_cb_pi_end(const lxb_dom_node_t *node,
                         const lxb_char_t *data, size_t len,
                         void *ctx, size_t level, bool is_close)
{
    lxb_status_t status = buf_append_escaped(ctx, data, len);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return wasm_document_serialize_span_close(ctx);
}

/*
 * Document type.
 */
static lxb_status_t
wasm_serialize_cb_doctype_begin(const lxb_dom_node_t *node,
                                const lxb_char_t *data, size_t len,
                                void *ctx, size_t level, bool is_close)
{
    lxb_status_t status;
    wasm_serialize_ctx_t *wasm = ctx;

    static const lexbor_str_t doctype_str = lexbor_str("doctype");

    status = wasm_document_serialize_level(wasm, level);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = wasm_document_serialize_li_span(wasm, node, doctype_str.data,
                                             doctype_str.length, is_close);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return buf_append_escaped(ctx, data, len);
}

static lxb_status_t
wasm_serialize_cb_doctype_name(const lxb_dom_node_t *node,
                               const lxb_char_t *data, size_t len,
                               void *ctx)
{
    return wasm_serialize_cb_name(node, data, len, ctx, false);
}

static lxb_status_t
wasm_serialize_cb_doctype_public(const lxb_dom_node_t *node,
                                 const lxb_char_t *data, size_t len,
                                 void *ctx)
{
    lxb_status_t status;
    wasm_serialize_ctx_t *wasm = ctx;

    static const lexbor_str_t public_lower_str = lexbor_str("public");
    static const lexbor_str_t public_str = lexbor_str("PUBLIC");
    static const lexbor_str_t ws_str = lexbor_str(" \"");

    /* span with class=public */
    status = wasm_document_serialize_span(wasm, public_lower_str.data,
                                          public_lower_str.length);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    wasm_document_str_append(ctx, public_str.data, public_str.length);

    status = wasm_document_serialize_span_close(wasm);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* "<PUBLIC ID>" */
    wasm_document_str_append(ctx, ws_str.data, ws_str.length);

    status = buf_append_escaped(wasm, data, len);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return wasm_document_str_append_return(ctx, "\"", 1);
}

static lxb_status_t
wasm_serialize_cb_doctype_system(const lxb_dom_node_t *node,
                                 const lxb_char_t *data, size_t len,
                                 void *ctx)
{
    lxb_status_t status;
    wasm_serialize_ctx_t *wasm = ctx;
    lxb_dom_document_type_t *dtype = lxb_dom_interface_document_type(node);

    static const lexbor_str_t system_lower_str = lexbor_str("system");
    static const lexbor_str_t system_str = lexbor_str("SYSTEM");
    static const lexbor_str_t ws_str = lexbor_str(" ");
    static const lexbor_str_t qo_str = lexbor_str("\"");

    /* span with class=system */
    if (dtype->public_id.data == NULL) {
        status = wasm_document_serialize_span(wasm, system_lower_str.data,
                                              system_lower_str.length);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        wasm_document_str_append(ctx, system_str.data, system_str.length);

        status = wasm_document_serialize_span_close(wasm);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        wasm_document_str_append(ctx, ws_str.data, ws_str.length);
    }

    /* "<SYSTEM ID>" */
    wasm_document_str_append(ctx, qo_str.data, qo_str.length);

    status = buf_append_escaped(wasm, data, len);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return wasm_document_str_append_return(ctx, "\"", 1);
}

static lxb_status_t
wasm_serialize_cb_doctype_end(const lxb_dom_node_t *node,
                              const lxb_char_t *data, size_t len,
                              void *ctx, size_t level, bool is_close)
{
    lxb_status_t status = buf_append_escaped(ctx, data, len);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return wasm_document_serialize_span_close(ctx);
}

static lxb_status_t
wasm_serialize_cb_doctype_ws(const lxb_dom_node_t *node,
                             const lxb_char_t *data, size_t len,
                             void *ctx)
{
    return wasm_document_str_append_return(ctx, data, len);
}

/*
 * New Line.
 */
static lxb_status_t
wasm_serialize_cb_newline(const lxb_char_t *data, size_t len,
                          void *ctx, size_t level)
{
    return LXB_STATUS_OK;
}
