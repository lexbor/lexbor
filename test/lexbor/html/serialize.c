/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>

#include <unit/test.h>


typedef struct {
    const char *data;
    const char *result;
}
html_text_entry_t;


static const char *const raw_text_names[] = {
    "style", "script", "xmp", "iframe", "noembed", "noframes", "plaintext",
    "noscript"
};

static const html_text_entry_t html_text_entries[] = {
    {"<div><style>a&b<c></style></div>",
     "<div><style>a&b<c></style></div>"},
    {"<div><script>a&b<c></script></div>",
     "<div><script>a&b<c></script></div>"},
    {"<div><xmp>a&b<c></xmp></div>",
     "<div><xmp>a&b<c></xmp></div>"},
    {"<div><iframe>a&b<c></iframe></div>",
     "<div><iframe>a&b<c></iframe></div>"},
    {"<div><noembed>a&b<c></noembed></div>",
     "<div><noembed>a&b<c></noembed></div>"},
    {"<div><noframes>a&b<c></noframes></div>",
     "<div><noframes>a&b<c></noframes></div>"},
    {"<div><plaintext>a&b<c>",
     "<div><plaintext>a&b<c></plaintext></div>"},
    {"<div><noscript>a&b<c></noscript></div>",
     "<div><noscript>a&b<c></noscript></div>"}
};

static const char escaped_text[] = "&lt;img src=x onerror=alert(1)&gt;&amp;";
static const char pretty_text[] = "\"&lt;img src=x onerror=alert(1)&gt;"
                                  "&amp;\"\n";
static const char ext_pretty_text[] = "\"&lt;img src=x onerror=alert(1)&gt;"
                                      "&amp;\"";


static lxb_html_document_t *
document_parse(const char *html, size_t length, bool scripting)
{
    lxb_status_t status;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    if (document == NULL) {
        return NULL;
    }

    lxb_dom_document_scripting_set(lxb_dom_interface_document(document),
                                   scripting);

    status = lxb_html_document_parse(document, (const lxb_char_t *) html,
                                     length);
    if (status != LXB_STATUS_OK) {
        return lxb_html_document_destroy(document);
    }

    return document;
}

static lxb_dom_node_t *
document_body(lxb_html_document_t *document)
{
    return lxb_dom_interface_node(lxb_html_document_body_element(document));
}

static size_t
foreign_html(char *buf, size_t size, const char *root, const char *name)
{
    int length;

    length = snprintf(buf, size, "<%s><%s>%s</%s></%s>",
                      root, name, escaped_text, name, root);
    if (length < 0 || (size_t) length >= size) {
        return 0;
    }

    return (size_t) length;
}

static lxb_dom_node_t *
foreign_text_node(lxb_html_document_t *document)
{
    return document_body(document)->first_child->first_child->first_child;
}

TEST_BEGIN(text_node_without_parent)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_dom_text_t *text;
    lxb_html_document_t *document;

    static const lxb_char_t txt[] = "abc";
    static const size_t length = sizeof(txt) - 1;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    text = lxb_dom_document_create_text_node(&document->dom_document,
                                             txt, length);
    test_ne(text, NULL);

    status = lxb_html_serialize_str(lxb_dom_interface_node(text), &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, length);
    test_eq_str(str.data, txt);

    str.length = 0;

    status = lxb_html_serialize_pretty_str(lxb_dom_interface_node(text), 0, 0,
                                           &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, length + 3); /* "abc\n" */
    test_eq_str(str.data, "\"abc\"\n");

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN_ARGS(foreign_text, const char *root)
{
    size_t length;
    lxb_status_t status;
    lxb_dom_node_t *text;
    lxb_html_document_t *document, *reparsed;
    char html[128];

    for (size_t i = 0; i < sizeof(raw_text_names) / sizeof(char *); i++) {
        lexbor_str_t str = {0};
        lexbor_str_t restr = {0};

        length = foreign_html(html, sizeof(html), root, raw_text_names[i]);
        test_ne(length, 0);

        document = document_parse(html, length, true);
        test_ne(document, NULL);

        text = foreign_text_node(document);
        test_eq(text->type, LXB_DOM_NODE_TYPE_TEXT);

        status = lxb_html_serialize_deep_str(document_body(document), &str);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str_n(str.data, str.length, html, length);

        reparsed = document_parse((const char *) str.data, str.length, true);
        test_ne(reparsed, NULL);

        status = lxb_html_serialize_deep_str(document_body(reparsed), &restr);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str_n(restr.data, restr.length, html, length);

        str.length = 0;

        status = lxb_html_serialize_str(text, &str);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str(str.data, escaped_text);

        lxb_html_document_destroy(reparsed);
        lxb_html_document_destroy(document);
    }
}
TEST_END

TEST_BEGIN_ARGS(foreign_text_pretty, const char *root)
{
    size_t length;
    lxb_status_t status;
    lxb_dom_node_t *text;
    lxb_html_document_t *document;
    char html[128];

    for (size_t i = 0; i < sizeof(raw_text_names) / sizeof(char *); i++) {
        lexbor_str_t str = {0};

        length = foreign_html(html, sizeof(html), root, raw_text_names[i]);
        test_ne(length, 0);

        document = document_parse(html, length, true);
        test_ne(document, NULL);

        text = foreign_text_node(document);
        test_eq(text->type, LXB_DOM_NODE_TYPE_TEXT);

        status = lxb_html_serialize_pretty_str(text, 0, 0, &str);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str(str.data, pretty_text);

        lxb_html_document_destroy(document);
    }
}
TEST_END

TEST_BEGIN_ARGS(foreign_text_ext, const char *root)
{
    size_t length;
    lxb_status_t status;
    lxb_dom_node_t *body, *text;
    lxb_html_document_t *document;
    lxb_html_serialize_ext_opt_t opt;
    char html[128];

    for (size_t i = 0; i < sizeof(raw_text_names) / sizeof(char *); i++) {
        lexbor_str_t str = {0};

        length = foreign_html(html, sizeof(html), root, raw_text_names[i]);
        test_ne(length, 0);

        document = document_parse(html, length, true);
        test_ne(document, NULL);

        text = foreign_text_node(document);
        test_eq(text->type, LXB_DOM_NODE_TYPE_TEXT);

        body = document_body(document);
        opt = LXB_HTML_SERIALIZE_EXT_OPT_UNDEF;

        status = lxb_html_serialize_ext_tree_str(body, &str, opt, NULL, false);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str_n(str.data, str.length, html, length);

        str.length = 0;

        status = lxb_html_serialize_ext_node_str(text, &str, opt, NULL);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str(str.data, escaped_text);

        str.length = 0;
        opt = LXB_HTML_SERIALIZE_EXT_OPT_REPLACE_NEWLINE;

        status = lxb_html_serialize_ext_tree_str(body, &str, opt, NULL, false);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str_n(str.data, str.length, html, length);

        str.length = 0;
        opt = LXB_HTML_SERIALIZE_EXT_OPT_PRETTY;

        status = lxb_html_serialize_ext_node_str(text, &str, opt, NULL);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str(str.data, ext_pretty_text);

        lxb_html_document_destroy(document);
    }
}
TEST_END

TEST_BEGIN(svg_text)
{
    TEST_CALL_ARGS(foreign_text, "svg");
}
TEST_END

TEST_BEGIN(mathml_text)
{
    TEST_CALL_ARGS(foreign_text, "math");
}
TEST_END

TEST_BEGIN(svg_text_pretty)
{
    TEST_CALL_ARGS(foreign_text_pretty, "svg");
}
TEST_END

TEST_BEGIN(mathml_text_pretty)
{
    TEST_CALL_ARGS(foreign_text_pretty, "math");
}
TEST_END

TEST_BEGIN(svg_text_ext)
{
    TEST_CALL_ARGS(foreign_text_ext, "svg");
}
TEST_END

TEST_BEGIN(mathml_text_ext)
{
    TEST_CALL_ARGS(foreign_text_ext, "math");
}
TEST_END

TEST_BEGIN(null_namespace_text)
{
    int length;
    const char *name;
    lxb_status_t status;
    lxb_dom_text_t *text;
    lxb_dom_node_t *node;
    lxb_dom_element_t *element;
    lxb_dom_document_t *dom_document;
    lxb_html_document_t *document;
    lxb_html_serialize_ext_opt_t opt;
    char result[64];

    static const char data[] = "<b>&";

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_document = lxb_dom_interface_document(document);

    lxb_dom_document_scripting_set(dom_document, true);

    for (size_t i = 0; i < sizeof(raw_text_names) / sizeof(char *); i++) {
        lexbor_str_t str = {0};

        name = raw_text_names[i];

        length = snprintf(result, sizeof(result), "<%s>&lt;b&gt;&amp;</%s>",
                          name, name);
        test_eq(length > 0 && (size_t) length < sizeof(result), true);

        element = lxb_dom_element_create(dom_document,
                                         (const lxb_char_t *) name,
                                         strlen(name), NULL, 0, NULL, 0,
                                         NULL, 0, false);
        test_ne(element, NULL);
        test_eq(element->node.ns, LXB_NS__UNDEF);

        text = lxb_dom_document_create_text_node(dom_document,
                                                 (const lxb_char_t *) data,
                                                 sizeof(data) - 1);
        test_ne(text, NULL);

        node = lxb_dom_interface_node(element);

        lxb_dom_node_insert_child(node, lxb_dom_interface_node(text));

        status = lxb_html_serialize_tree_str(node, &str);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str(str.data, result);

        str.length = 0;
        opt = LXB_HTML_SERIALIZE_EXT_OPT_UNDEF;

        status = lxb_html_serialize_ext_tree_str(node, &str, opt, NULL, true);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str(str.data, result);

        str.length = 0;

        status = lxb_html_serialize_pretty_str(lxb_dom_interface_node(text),
                                               0, 0, &str);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str(str.data, "\"&lt;b&gt;&amp;\"\n");

        lxb_dom_node_destroy_deep(node);
    }

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(html_text)
{
    lxb_status_t status;
    lxb_dom_node_t *body;
    lxb_html_document_t *document;
    const html_text_entry_t *entry;
    lxb_html_serialize_ext_opt_t opt;

    for (size_t i = 0; i < sizeof(html_text_entries)
                           / sizeof(html_text_entry_t); i++)
    {
        lexbor_str_t str = {0};

        entry = &html_text_entries[i];

        document = document_parse(entry->data, strlen(entry->data), true);
        test_ne(document, NULL);

        body = document_body(document);

        status = lxb_html_serialize_deep_str(body, &str);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str(str.data, entry->result);

        str.length = 0;
        opt = LXB_HTML_SERIALIZE_EXT_OPT_UNDEF;

        status = lxb_html_serialize_ext_tree_str(body, &str, opt, NULL, false);
        test_eq(status, LXB_STATUS_OK);
        test_eq_str(str.data, entry->result);

        lxb_html_document_destroy(document);
    }
}
TEST_END

TEST_BEGIN(html_noscript_text_without_scripting)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_dom_node_t *body;
    lxb_html_document_t *document;

    static const char html[] = "<div><noscript>a&b</noscript></div>";
    static const char result[] = "<div><noscript>a&amp;b</noscript></div>";

    document = document_parse(html, sizeof(html) - 1, false);
    test_ne(document, NULL);

    body = document_body(document);

    status = lxb_html_serialize_deep_str(body, &str);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str(str.data, result);

    str.length = 0;

    status = lxb_html_serialize_ext_tree_str(body, &str,
                                             LXB_HTML_SERIALIZE_EXT_OPT_UNDEF,
                                             NULL, false);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str(str.data, result);

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(template_contents)
{
    lxb_status_t status;
    lxb_dom_node_t *node;
    lxb_html_document_t *document;
    lxb_html_serialize_ext_opt_t opt;
    lexbor_str_t str = {0};

    static const char html[] = "<div><template><b>a</b>c</template></div>";

    document = document_parse(html, sizeof(html) - 1, false);
    test_ne(document, NULL);

    node = document_body(document)->first_child->first_child;
    test_eq(node->local_name, LXB_TAG_TEMPLATE);
    test_eq(node->first_child, NULL);

    status = lxb_html_serialize_deep_str(node, &str);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str(str.data, "<b>a</b>c");

    str.length = 0;
    opt = LXB_HTML_SERIALIZE_EXT_OPT_UNDEF;

    status = lxb_html_serialize_ext_tree_str(node, &str, opt, NULL, false);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str(str.data, "<b>a</b>c");

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(template_children)
{
    lxb_status_t status;
    lxb_dom_text_t *text;
    lxb_dom_node_t *body, *node;
    lxb_dom_document_t *dom_document;
    lxb_html_document_t *document;
    lxb_html_serialize_ext_opt_t opt;
    lexbor_str_t str = {0};

    static const char html[] = "<div><template><b>a</b></template></div>";

    document = document_parse(html, sizeof(html) - 1, false);
    test_ne(document, NULL);

    dom_document = lxb_dom_interface_document(document);
    body = document_body(document);

    node = body->first_child->first_child;
    test_eq(node->local_name, LXB_TAG_TEMPLATE);

    text = lxb_dom_document_create_text_node(dom_document,
                                             (const lxb_char_t *) "x", 1);
    test_ne(text, NULL);

    lxb_dom_node_insert_child(node, lxb_dom_interface_node(text));

    status = lxb_html_serialize_deep_str(body, &str);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str(str.data, html);

    str.length = 0;

    status = lxb_html_serialize_deep_str(node, &str);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str(str.data, "<b>a</b>");

    str.length = 0;
    opt = LXB_HTML_SERIALIZE_EXT_OPT_UNDEF;

    status = lxb_html_serialize_ext_tree_str(body, &str, opt, NULL, false);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str(str.data, html);

    str.length = 0;

    status = lxb_html_serialize_ext_tree_str(node, &str, opt, NULL, false);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str(str.data, "<b>a</b>");

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(void_children)
{
    lxb_status_t status;
    lxb_dom_text_t *text;
    lxb_dom_node_t *body, *node;
    lxb_dom_document_t *dom_document;
    lxb_html_document_t *document;
    lxb_html_serialize_ext_opt_t opt;
    lexbor_str_t str = {0};

    static const char html[] = "<div><img></div>";

    document = document_parse(html, sizeof(html) - 1, false);
    test_ne(document, NULL);

    dom_document = lxb_dom_interface_document(document);
    body = document_body(document);

    node = body->first_child->first_child;
    test_eq(node->local_name, LXB_TAG_IMG);

    text = lxb_dom_document_create_text_node(dom_document,
                                             (const lxb_char_t *) "x", 1);
    test_ne(text, NULL);

    lxb_dom_node_insert_child(node, lxb_dom_interface_node(text));

    status = lxb_html_serialize_deep_str(body, &str);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str(str.data, html);

    str.length = 0;

    status = lxb_html_serialize_deep_str(node, &str);
    test_eq(status, LXB_STATUS_OK);
    test_eq(str.length, 0);

    str.length = 0;
    opt = LXB_HTML_SERIALIZE_EXT_OPT_UNDEF;

    status = lxb_html_serialize_ext_tree_str(body, &str, opt, NULL, false);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str(str.data, html);

    str.length = 0;

    status = lxb_html_serialize_ext_tree_str(node, &str, opt, NULL, false);
    test_eq(status, LXB_STATUS_OK);
    test_eq(str.length, 0);

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(deep_template_nesting)
{
    char *html;
    size_t i;
    lxb_status_t status;
    lxb_dom_node_t *tmpl;
    lxb_html_document_t *document;
    lexbor_str_t str = {0};
    lexbor_str_t ext = {0};

    static const char open[] = "<template>";
    static const size_t depth = 65536;

    html = lexbor_malloc(depth * (sizeof(open) - 1));
    test_ne(html, NULL);

    for (i = 0; i < depth; i++) {
        memcpy(html + i * (sizeof(open) - 1), open, sizeof(open) - 1);
    }

    document = document_parse(html, depth * (sizeof(open) - 1), false);
    test_ne(document, NULL);

    lexbor_free(html);

    tmpl = lxb_dom_interface_node(lxb_html_document_head_element(document));
    tmpl = tmpl->first_child;
    test_eq(tmpl->local_name, LXB_TAG_TEMPLATE);

    status = lxb_html_serialize_tree_str(tmpl, &str);
    test_eq(status, LXB_STATUS_OK);
    test_eq(str.length, depth * (sizeof("<template></template>") - 1));

    status = lxb_html_serialize_ext_tree_str(tmpl, &ext,
                                             LXB_HTML_SERIALIZE_EXT_OPT_UNDEF,
                                             NULL, true);
    test_eq(status, LXB_STATUS_OK);
    test_eq_str_n(ext.data, ext.length, str.data, str.length);

    str.length = 0;

    status = lxb_html_serialize_deep_str(tmpl, &str);
    test_eq(status, LXB_STATUS_OK);
    test_eq(str.length, (depth - 1) * (sizeof("<template></template>") - 1));

    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(text_node_without_parent);
    TEST_ADD(svg_text);
    TEST_ADD(mathml_text);
    TEST_ADD(svg_text_pretty);
    TEST_ADD(mathml_text_pretty);
    TEST_ADD(svg_text_ext);
    TEST_ADD(mathml_text_ext);
    TEST_ADD(null_namespace_text);
    TEST_ADD(html_text);
    TEST_ADD(html_noscript_text_without_scripting);
    TEST_ADD(template_contents);
    TEST_ADD(template_children);
    TEST_ADD(void_children);
    TEST_ADD(deep_template_nesting);

    TEST_RUN("lexbor/html/serialize");
    TEST_RELEASE();
}
