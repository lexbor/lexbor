/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Extended HTML Serialization API.
 * 
 * This is a new, flexible implementation of HTML serialization.  It produces
 * the same spec-compliant output as the original serialize.h interface
 * (see lxb_html_serialize_tree_str, lxb_html_serialize_cb, etc.), but gives
 * the caller fine-grained control over every part of the output through
 * per-node-type callback structures.
 *
 * The core serialization algorithm follows the WHATWG HTML Living Standard:
 *     https://html.spec.whatwg.org/multipage/parsing.html#serialising-html-fragments
 *
 * The callback mechanism allows applications to intercept and
 * transform individual tokens (tag edges, names, attribute parts, text chunks,
 * indentation, etc.) without reimplementing the serialization logic.
 *
 * This approach makes it easy to highlight HTML code or customize it to suit
 * the user's needs.
 * 
 * NOTE:
 * The lxb_html_serialize_ext_opt_t flags (SKIP_WS_NODES, SKIP_COMMENT,
 * PRETTY, RAW, etc.) are intentional deviations from the specification.
 * They exist for debugging, pretty-printing, and human-readable HTML output.
 * When any of these flags is set, the result may differ from what the spec
 * prescribes.  For strict spec-compliant serialization, use
 * LXB_HTML_SERIALIZE_EXT_OPT_UNDEF (no flags).
 * 
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_HTML_SERIALIZE_EXT_H
#define LEXBOR_HTML_SERIALIZE_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/str.h"
#include "lexbor/dom/interfaces/element.h"
#include "lexbor/html/base.h"


typedef unsigned lxb_html_serialize_ext_opt_t;

enum lxb_html_serialize_ext_opt {
    LXB_HTML_SERIALIZE_EXT_OPT_UNDEF               = 0x00,

    /*
     * Skip whitespace-only text nodes during serialization.
     *
     * Useful for pretty-printing or when you want to ignore insignificant
     * whitespace between elements.
     */
    LXB_HTML_SERIALIZE_EXT_OPT_SKIP_WS_NODES       = 1 << 0,

    /*
     * Skip comment nodes during serialization.
     *
     * When set, comment nodes (<!-- ... -->) will be omitted from the output.
     */
    LXB_HTML_SERIALIZE_EXT_OPT_SKIP_COMMENT        = 1 << 1,

    /*
     * Replace newline characters in text content with spaces.
     *
     * Useful for producing single-line output from multi-line text nodes.
     */
    LXB_HTML_SERIALIZE_EXT_OPT_REPLACE_NEWLINE     = 1 << 2,

    /*
     * Serialize text content without HTML entity escaping.
     *
     * By default, characters like &, <, >, " are escaped to their entity
     * equivalents (&amp;, &lt;, etc.).  With this option, the raw text is
     * passed through as-is.
     */
    LXB_HTML_SERIALIZE_EXT_OPT_RAW                 = 1 << 3,

    /*
     * Omit closing tags for elements.
     *
     * Only the opening tag (with attributes) will be serialized.
     * Child nodes are still serialized, but no </tag> is emitted.
     */
    LXB_HTML_SERIALIZE_EXT_OPT_WITHOUT_CLOSING     = 1 << 4,

    /*
     * Include namespace prefix in tag names.
     *
     * For example, an SVG element will be serialized as <svg:rect> instead
     * of just <rect>.  The namespace prefix callback (ns) in
     * lxb_html_serialize_ext_node_t will be called for each element.
     */
    LXB_HTML_SERIALIZE_EXT_OPT_TAG_WITH_NS         = 1 << 5,

    /*
     * Serialize DOCTYPE with full PUBLIC/SYSTEM identifiers.
     *
     * By default, DOCTYPE is serialized in short form: <!DOCTYPE html>.
     * With this option, the full form is used:
     *     <!DOCTYPE html PUBLIC "..." SYSTEM "...">
     */
    LXB_HTML_SERIALIZE_EXT_OPT_FULL_DOCTYPE        = 1 << 6,

    /*
     * Enable pretty-printing with indentation and newlines.
     *
     * When set, the serializer inserts newlines between nodes and calls
     * the indent callback to add indentation based on the nesting depth.
     * The indent string is specified via the "indent" parameter
     * of the serialization functions.
     */
    LXB_HTML_SERIALIZE_EXT_OPT_PRETTY              = 1 << 7
};

typedef lxb_status_t
(*lxb_html_serialize_ext_node_cb_f)(const lxb_dom_node_t *node,
                                    void *ctx);
typedef lxb_status_t
(*lxb_html_serialize_ext_edge_cb_f)(const lxb_dom_node_t *node,
                                    const lxb_char_t *data, size_t len,
                                    void *ctx, bool is_close);
typedef lxb_status_t
(*lxb_html_serialize_ext_name_cb_f)(const lxb_dom_node_t *node,
                                    const lxb_char_t *data, size_t len,
                                    void *ctx, bool is_close);
typedef lxb_status_t
(*lxb_html_serialize_ext_attr_cb_f)(const lxb_dom_node_t *node,
                                    const lxb_dom_attr_t *attr,
                                    const lxb_char_t *data, size_t len,
                                    void *ctx);
typedef lxb_status_t
(*lxb_html_serialize_ext_text_cb_f)(const lxb_dom_node_t *node,
                                    const lxb_char_t *data, size_t len,
                                    void *ctx);
typedef lxb_status_t
(*lxb_html_serialize_ext_cb_f)(const lxb_char_t *data, size_t len, void *ctx);

typedef struct {
    /*
     * Called before the element begins.
     * It can be called multiple times.
     * This callback for print indent.
     */
    lxb_html_serialize_ext_cb_f        indent;
    /*
     * Called once before the element begins. Before <tag...>.
     * This callback must print the "<" character.
     * For closing tag, the callback will be called with is_close = true,
     * and it must print the "</" characters.
     */
    lxb_html_serialize_ext_edge_cb_f   before;
    /*
     * Called once after the element ends. After <tag...>.
     * This callback must print the ">" character.
     * For closing tag, the callback will be called with is_close = true,
     * and it must print the ">" character.
     */
    lxb_html_serialize_ext_edge_cb_f   after;

    /*
     * Called once for the namespace prefix of the element.
     * This callback must print the namespace prefix followed by ":" character.
     * For example, for element "<svg>" with namespace "SVG", the callback
     * will be called with data = "svg" and len = 3.
     * You must print data and ":" character.
     * Only for LXB_HTML_SERIALIZE_EXT_OPT_TAG_WITH_NS option.
     * if namespace prefix is not present, this callback will not be called.
     */
    lxb_html_serialize_ext_name_cb_f   ns;
    /*
     * Called once for the tag name.
     * This callback must print the tag name.
     * For closing tag, the callback will be called with is_close = true,
     * and it must print the tag name.
     */
    lxb_html_serialize_ext_name_cb_f   name;
}
lxb_html_serialize_ext_node_t;

typedef struct {
    /*
     * Called once for the namespace prefix of the attribute.
     * This callback must print the namespace prefix followed by ":" character.
     * For example, for attribute "xlink:href", the callback will be called
     * with data = "xlink" and len = 5.
     * if namespace prefix is not present, this callback will not be called.
     */
    lxb_html_serialize_ext_attr_cb_f ns;
    /*
     * Called once for the attribute name.
     * This callback must print the attribute name.
     * const lxb_dom_attr_t *attr can be NULL.
     */
    lxb_html_serialize_ext_attr_cb_f name;
    /*
     * Called once before the attribute value.
     * This callback must print '="' characters.
     * const lxb_dom_attr_t *attr can be NULL.
     */
    lxb_html_serialize_ext_attr_cb_f value_before;
    /*
     * Called for the attribute value. It can be called multiple times.
     * This callback must print the attribute value.
     * const lxb_dom_attr_t *attr can be NULL.
     */
    lxb_html_serialize_ext_attr_cb_f value;
    /*
     * Called once after the attribute value.
     * This callback must print '"' character.
     * const lxb_dom_attr_t *attr can be NULL.
     */
    lxb_html_serialize_ext_attr_cb_f value_after;
    /*
     * Called once for whitespace before the attribute name.
     * This callback must print a single space character.
     */
    lxb_html_serialize_ext_text_cb_f ws;
}
lxb_html_serialize_ext_attr_t;

typedef struct {
    /*
     * Called before the text begins.
     * It can be called multiple times.
     * This callback for print indent.
     */
    lxb_html_serialize_ext_cb_f      indent;
    /*
     * Called once before the text node begins.
     */
    lxb_html_serialize_ext_node_cb_f before;
    /*
     * Called for the text node data. It can be called multiple times.
     * This callback must print the text node data.
     */
    lxb_html_serialize_ext_text_cb_f text;
    /*
     * Called once after the text node ends.
     */
    lxb_html_serialize_ext_node_cb_f after;
}
lxb_html_serialize_ext_text_t;

typedef struct {
    /*
     * Called before the comment begins.
     * It can be called multiple times.
     * This callback for print indent.
     */
    lxb_html_serialize_ext_cb_f      indent;
    /*
     * Called once with "<!--" string.
     */
    lxb_html_serialize_ext_text_cb_f begin;
    /*
     * Called for the comment data. It can be called multiple times.
     */
    lxb_html_serialize_ext_text_cb_f text;
    /*
     * Called once with "-->" string.
     */
    lxb_html_serialize_ext_text_cb_f end;
}
lxb_html_serialize_ext_comment_t;

typedef struct {
    /*
     * Called before the comment begins.
     * It can be called multiple times.
     * This callback for print indent.
     */
    lxb_html_serialize_ext_cb_f      indent;
    /*
     * Called once with "<?" string.
     */
    lxb_html_serialize_ext_text_cb_f begin;
    /*
     * Called for the processing instruction target.
     * It can be called multiple times.
     */
    lxb_html_serialize_ext_text_cb_f target;
    /*
     * Called once with " " string.
     * Called after "target" and before "text".
     */
    lxb_html_serialize_ext_text_cb_f middle;
    /*
     * Called for the processing instruction data.
     * It can be called multiple times.
     */
    lxb_html_serialize_ext_text_cb_f text;
    /*
     * Called once with ">" string.
     */
    lxb_html_serialize_ext_text_cb_f end;
}
lxb_html_serialize_ext_processing_instruction_t;

typedef struct {
    /*
     * Called before the comment begins.
     * It can be called multiple times.
     * This callback for print indent.
     */
    lxb_html_serialize_ext_cb_f      indent;
    /*
     * Called once with "<!DOCTYPE" string.
     */
    lxb_html_serialize_ext_text_cb_f begin;
    /*
     * Called once for the document type name.
     */
    lxb_html_serialize_ext_text_cb_f name;
    /*
     * Called once for the document type public identifier.
     * User must print 'PUBLIC "' string before the public identifier value,
     * and '"' character after the public identifier value.
     */
    lxb_html_serialize_ext_text_cb_f text_public;
    /*
     * Called once for the document type system identifier.
     * User must print 'SYSTEM "' string before the system identifier value,
     * and '"' character after the system identifier value.
     */
    lxb_html_serialize_ext_text_cb_f text_system;
    /*
     * Called once with ">" string.
     */
    lxb_html_serialize_ext_text_cb_f end;
    /*
     * Called for whitespace before name, public identifier,
     * and system identifier.
     * This callback must print a single space character.
     */
    lxb_html_serialize_ext_text_cb_f ws;

}
lxb_html_serialize_ext_document_type_t;

/*
 * Main callbacks structure for extended serialization.
 *
 * Contains callback groups for each DOM node type.  Any callback field can be
 * set to NULL, in which case the default behavior (append text to string)
 * is used.  This allows you to override only the parts you need.
 *
 * Usage:
 *     lxb_html_serialize_ext_t ext = {0};
 *     ext.node.name = my_tag_name_cb;       // Custom tag name handler.
 *     ext.attr.value = my_attr_value_cb;    // Custom attribute value handler.
 *     // All other callbacks will use defaults.
 *
 *     lxb_html_serialize_ext_tree_cb(root, &ext, my_ctx,
 *                                    LXB_HTML_SERIALIZE_EXT_OPT_UNDEF,
 *                                    NULL, false);
 */
typedef struct {
    /* Callbacks for element and document opening/closing tags. */
    const lxb_html_serialize_ext_node_t                   *node;

    /* Callbacks for attribute serialization. */
    const lxb_html_serialize_ext_attr_t                   *attr;

    /* Callbacks for text node serialization. */
    const lxb_html_serialize_ext_text_t                   *text;

    /* Callbacks for comment node serialization. */
    const lxb_html_serialize_ext_comment_t                *comment;

    /* Callbacks for processing instruction (<?...>) serialization. */
    const lxb_html_serialize_ext_processing_instruction_t *processing_instruction;

    /* Callbacks for document type (<!DOCTYPE ...>) serialization. */
    const lxb_html_serialize_ext_document_type_t          *document_type;

    /* Callbacks for document node serialization (same structure as element). */
    const lxb_html_serialize_ext_node_t                   *document;

    /*
     * Called for newline character.
     * Only for LXB_HTML_SERIALIZE_EXT_OPT_PRETTY option.
     */
    lxb_html_serialize_ext_cb_f                           newline;
}
lxb_html_serialize_ext_t;


/*
 * Serialize a DOM tree with user-defined callbacks.
 *
 * Walks the tree starting from the given node and calls the callbacks
 * for each part of the serialization (tags, attributes, text, comments, etc.).
 *
 * @param[in] node. The root node to start serialization from.
 * @param[in] cb. Callbacks for each node type.
 * @param[in] ctx. User context passed to all callbacks.
 * @param[in] opt. Serialization options (bitmask).
 * @param[in] indent. Indent string for pretty-printing.
 *     Used only with LXB_HTML_SERIALIZE_EXT_OPT_PRETTY. Can be NULL.
 * @param[in] with_current. If true, the root node itself is included
 *     in the output. If false, only its children are serialized.
 *     Ignored for document nodes (always serializes children only).
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_tree_cb(const lxb_dom_node_t *node,
                               const lxb_html_serialize_ext_t *cb, void *ctx,
                               lxb_html_serialize_ext_opt_t opt,
                               const lexbor_str_t *indent, bool with_current);

/*
 * Serialize a DOM tree into a string.
 *
 * Convenience wrapper that uses default callbacks and appends the result
 * to the provided lexbor_str_t. If str->data is NULL, the string will be
 * initialized automatically using the document's memory allocator.
 *
 * @param[in] node. The root node to start serialization from.
 * @param[in, out] str. Output string. Can be uninitialized (data = NULL),
 *     in which case it will be allocated.
 * @param[in] opt. Serialization options (bitmask).
 * @param[in] indent. Indent string for pretty-printing.
 *     Used only with LXB_HTML_SERIALIZE_EXT_OPT_PRETTY. Can be NULL.
 * @param[in] with_current. If true, the root node itself is included
 *     in the output. If false, only its children are serialized.
 *     Ignored for document nodes (always serializes children only).
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_tree_str(const lxb_dom_node_t *node, lexbor_str_t *str,
                                lxb_html_serialize_ext_opt_t opt,
                                const lexbor_str_t *indent, bool with_current);

/*
 * Serialize a single DOM node with user-defined callbacks.
 *
 * Dispatches the node to the appropriate type-specific serializer
 * (element, text, comment, processing instruction, document type, or document)
 * and calls the corresponding callbacks.
 *
 * Does not walk child nodes. Only the node itself is serialized.
 *
 * @param[in] node. The node to serialize.
 * @param[in] cb. Callbacks for each node type.
 * @param[in] ctx. User context passed to all callbacks.
 * @param[in] opt. Serialization options (bitmask).
 * @param[in] indent. Indent string for pretty-printing. Can be NULL.
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_node_cb(const lxb_dom_node_t *node,
                               const lxb_html_serialize_ext_t *cb, void *ctx,
                               lxb_html_serialize_ext_opt_t opt,
                               const lexbor_str_t *indent);

/*
 * Serialize a single DOM node into a string.
 *
 * Convenience wrapper that uses default callbacks and appends the result
 * to the provided lexbor_str_t. If str->data is NULL, the string will be
 * initialized automatically.
 *
 * Does not walk child nodes.
 *
 * @param[in] node. The node to serialize.
 * @param[in, out] str. Output string. Can be uninitialized (data = NULL).
 * @param[in] opt. Serialization options (bitmask).
 * @param[in] indent. Indent string for pretty-printing. Can be NULL.
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_node_str(const lxb_dom_node_t *node, lexbor_str_t *str,
                                lxb_html_serialize_ext_opt_t opt,
                                const lexbor_str_t *indent);

/*
 * Serialize an element's opening tag with attributes using callbacks.
 *
 * Produces the opening tag only: edge tokens, namespace prefix,
 * tag name, and all attributes. Does not serialize children or closing tag.
 *
 * Callback call order:
 *     indent -> before("<") -> ns("svg") -> name("rect") ->
 *     [for each attr: ws(" ") -> ns("xlink") -> name("href") ->
 *      value_before("=\"") -> value("...") -> value_after("\"")] ->
 *     after(">")
 *
 * @param[in] element. The element to serialize.
 * @param[in] cb_node. Element callbacks (edge, name, ns).
 * @param[in] cb_attr. Attribute callbacks.
 * @param[in] ctx. User context.
 * @param[in] opt. Serialization options (bitmask).
 * @param[in] indent. Indent string. Can be NULL.
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_element_cb(const lxb_dom_element_t *element,
                                  const lxb_html_serialize_ext_node_t *cb_node,
                                  const lxb_html_serialize_ext_attr_t *cb_attr,
                                  void *ctx, lxb_html_serialize_ext_opt_t opt,
                                  const lexbor_str_t *indent);

/*
 * Serialize an element tag without attributes using callbacks.
 *
 * Produces only the tag structure: edge tokens, namespace prefix,
 * and tag name. No attributes are serialized.
 * Used internally for closing tags during tree serialization.
 *
 * @param[in] element. The element to serialize.
 * @param[in] cb. Element callbacks (edge, name, ns).
 * @param[in] ctx. User context.
 * @param[in] opt. Serialization options (bitmask).
 * @param[in] indent. Indent string. Can be NULL.
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_element_wo_attr_cb(const lxb_dom_element_t *element,
                                          const lxb_html_serialize_ext_node_t *cb,
                                          void *ctx, lxb_html_serialize_ext_opt_t opt,
                                          const lexbor_str_t *indent);

/*
 * Serialize a single attribute using callbacks.
 *
 * Callback call order:
 *     ns("xlink") -> name("href") -> value_before("=\"") ->
 *     value("...") -> value_after("\"")
 *
 * The value_before and value_after callbacks are always called,
 * even when the attribute has no value (value callback is skipped).
 *
 * @param[in] attr. The attribute to serialize.
 * @param[in] ext_attr. Attribute callbacks.
 * @param[in] ctx. User context.
 * @param[in] opt. Serialization options (bitmask).
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_attribute_cb(const lxb_dom_attr_t *attr,
                                    const lxb_html_serialize_ext_attr_t *ext_attr,
                                    void *ctx, lxb_html_serialize_ext_opt_t opt);

/*
 * Serialize a text node using callbacks.
 *
 * Text content is escaped according to the HTML spec unless
 * LXB_HTML_SERIALIZE_EXT_OPT_RAW is set.
 *
 * Callback call order: indent -> before -> text (may be called multiple
 * times when escaping splits the content) -> after.
 *
 * @param[in] text. The text node to serialize.
 * @param[in] cb. Text callbacks.
 * @param[in] ctx. User context.
 * @param[in] opt. Serialization options (bitmask).
 * @param[in] indent. Indent string. Can be NULL.
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_text_cb(const lxb_dom_text_t *text,
                               const lxb_html_serialize_ext_text_t *cb,
                               void *ctx, lxb_html_serialize_ext_opt_t opt,
                               const lexbor_str_t *indent);

/*
 * Serialize a comment node using callbacks.
 *
 * Callback call order: indent -> begin("<!--") -> text -> end("-->").
 *
 * @param[in] comment. The comment node to serialize.
 * @param[in] cb. Comment callbacks.
 * @param[in] ctx. User context.
 * @param[in] opt. Serialization options (bitmask).
 * @param[in] indent. Indent string. Can be NULL.
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_comment_cb(const lxb_dom_comment_t *comment,
                                  const lxb_html_serialize_ext_comment_t *cb,
                                  void *ctx, lxb_html_serialize_ext_opt_t opt,
                                  const lexbor_str_t *indent);

/*
 * Serialize a processing instruction node using callbacks.
 *
 * Callback call order:
 *     indent -> begin("<?") -> target -> middle(" ") -> text -> end(">").
 *
 * @param[in] pi. The processing instruction node to serialize.
 * @param[in] cb. Processing instruction callbacks.
 * @param[in] ctx. User context.
 * @param[in] opt. Serialization options (bitmask).
 * @param[in] indent. Indent string. Can be NULL.
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_processing_instruction_cb(const lxb_dom_processing_instruction_t *pi,
                                                 const lxb_html_serialize_ext_processing_instruction_t *cb,
                                                 void *ctx, lxb_html_serialize_ext_opt_t opt,
                                                 const lexbor_str_t *indent);

/*
 * Serialize a document type node in short form using callbacks.
 *
 * Produces: <!DOCTYPE name>
 *
 * Callback call order:
 *     indent -> begin("<!DOCTYPE") -> ws(" ") -> name -> end(">").
 *
 * @param[in] doctype. The document type node.
 * @param[in] cb. Document type callbacks.
 * @param[in] ctx. User context.
 * @param[in] opt. Serialization options (bitmask).
 * @param[in] indent. Indent string. Can be NULL.
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_document_type_cb(const lxb_dom_document_type_t *doctype,
                                        const lxb_html_serialize_ext_document_type_t *cb,
                                        void *ctx, lxb_html_serialize_ext_opt_t opt,
                                        const lexbor_str_t *indent);

/*
 * Serialize a document type node in full form using callbacks.
 *
 * Produces: <!DOCTYPE name PUBLIC "pubid" "sysid">
 *
 * Callback call order:
 *     indent -> begin("<!DOCTYPE") -> ws(" ") -> name ->
 *     [ws(" ") -> text_public] -> [ws(" ") -> text_system] -> end(">").
 *
 * The text_public and text_system callbacks are only called if the
 * corresponding identifiers are present in the document type.
 *
 * @param[in] doctype. The document type node.
 * @param[in] cb. Document type callbacks.
 * @param[in] ctx. User context.
 * @param[in] opt. Serialization options (bitmask).
 * @param[in] indent. Indent string. Can be NULL.
 *
 * @return LXB_STATUS_OK on success, or an error status.
 */
LXB_API lxb_status_t
lxb_html_serialize_ext_document_type_full_cb(const lxb_dom_document_type_t *doctype,
                                             const lxb_html_serialize_ext_document_type_t *cb,
                                             void *ctx, lxb_html_serialize_ext_opt_t opt,
                                             const lexbor_str_t *indent);

LXB_API const lexbor_str_t *
lxb_html_serialize_ext_opt_to_str(lxb_html_serialize_ext_opt_t opt);

LXB_API lxb_html_serialize_ext_opt_t
lxb_html_serialize_ext_str_to_opt(const lxb_char_t *data, size_t length);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_SERIALIZE_EXT_H */
