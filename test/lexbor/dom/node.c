/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/html/html.h>


typedef struct {
    const lexbor_str_t str;
    bool               is_null;
}
test_id_data_t;


static const lexbor_str_t html = lexbor_str("<div id='div-1'></div>"
                                            "<div id='div-2'></div>");


TEST_BEGIN(insert_before)
{
    lexbor_status_t status;
    lxb_dom_exception_code_t code;
    lxb_html_document_t *document;
    lexbor_str_t str;
    lxb_dom_node_t *body;
    lxb_html_element_t *p_el;
    lxb_html_body_element_t *element;

    static const lexbor_str_t p_str = lexbor_str("p");
    static const lexbor_str_t result = lexbor_str("<div id=\"div-1\"></div>"
                                                  "<p></p>"
                                                  "<div id=\"div-2\"></div>");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    element = lxb_html_document_body_element(document);
    body = lxb_dom_interface_node(element);

    /* Create <p> Element. */

    p_el = lxb_html_document_create_element(document, p_str.data,
                                            p_str.length, NULL);
    test_ne(p_el, NULL);

    /* Insert. */

    code = lxb_dom_node_insert_before_spec(body, lxb_dom_interface_node(p_el),
                                           body->first_child->next);
    test_eq(code, LXB_DOM_EXCEPTION_OK);

    /* Check. */

    str.data = NULL;
    str.length = 0;

    status = lxb_html_serialize_deep_str(body, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(str.data, str.length, result.data, result.length);

    /* Destroy all. */

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(append_child)
{
    lexbor_status_t status;
    lxb_dom_exception_code_t code;
    lxb_html_document_t *document;
    lexbor_str_t str;
    lxb_dom_node_t *body;
    lxb_html_element_t *p_el;
    lxb_html_body_element_t *element;

    static const lexbor_str_t p_str = lexbor_str("p");
    static const lexbor_str_t result = lexbor_str("<div id=\"div-1\"></div>"
                                                  "<div id=\"div-2\"></div>"
                                                  "<p></p>");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    element = lxb_html_document_body_element(document);
    body = lxb_dom_interface_node(element);

    /* Create <p> Element. */

    p_el = lxb_html_document_create_element(document, p_str.data,
                                            p_str.length, NULL);
    test_ne(p_el, NULL);

    /* Append. */

    code = lxb_dom_node_append_child(body, lxb_dom_interface_node(p_el));
    test_eq(code, LXB_DOM_EXCEPTION_OK);

    /* Check. */

    str.data = NULL;
    str.length = 0;

    status = lxb_html_serialize_deep_str(body, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(str.data, str.length, result.data, result.length);

    /* Destroy all. */

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(append_child_document_fragment)
{
    lexbor_status_t status;
    lxb_dom_exception_code_t code;
    lxb_html_document_t *document;
    lexbor_str_t str;
    lxb_dom_node_t *body, *frag_node;
    lxb_dom_document_t *dom_doc;
    lxb_html_element_t *p_el, *a_el;
    lxb_html_body_element_t *element;
    lxb_dom_document_fragment_t *fragment;

    static const lexbor_str_t p_str = lexbor_str("p");
    static const lexbor_str_t a_str = lexbor_str("a");
    static const lexbor_str_t result = lexbor_str("<div id=\"div-1\"></div>"
                                                  "<div id=\"div-2\"></div>"
                                                  "<p></p><a></a>");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    element = lxb_html_document_body_element(document);
    body = lxb_dom_interface_node(element);

    /* Create DocumentFragment. */

    dom_doc = lxb_dom_interface_document(document);

    fragment = lxb_dom_document_fragment_interface_create(dom_doc);
    test_ne(fragment, NULL);

    /* Create <p> and <a> Element. */

    frag_node = lxb_dom_interface_node(fragment);

    p_el = lxb_html_document_create_element(document, p_str.data,
                                            p_str.length, NULL);
    test_ne(p_el, NULL);

    a_el = lxb_html_document_create_element(document, a_str.data,
                                            a_str.length, NULL);
    test_ne(a_el, NULL);

    code = lxb_dom_node_append_child(frag_node, lxb_dom_interface_node(p_el));
    test_eq(code, LXB_DOM_EXCEPTION_OK);

    code = lxb_dom_node_append_child(frag_node, lxb_dom_interface_node(a_el));
    test_eq(code, LXB_DOM_EXCEPTION_OK);

    /* Append DocumentFragment. */

    code = lxb_dom_node_append_child(body, frag_node);
    test_eq(code, LXB_DOM_EXCEPTION_OK);

    /* Check. */

    str.data = NULL;
    str.length = 0;

    status = lxb_html_serialize_deep_str(body, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(str.data, str.length, result.data, result.length);

    /* Destroy all. */

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(replace_child)
{
    lexbor_status_t status;
    lxb_dom_exception_code_t code;
    lxb_html_document_t *document;
    lexbor_str_t str;
    lxb_dom_node_t *body;
    lxb_html_element_t *p_el;
    lxb_html_body_element_t *element;

    static const lexbor_str_t p_str = lexbor_str("p");
    static const lexbor_str_t result = lexbor_str("<p></p>"
                                                  "<div id=\"div-2\"></div>");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    element = lxb_html_document_body_element(document);
    body = lxb_dom_interface_node(element);

    /* Create <p> Element. */

    p_el = lxb_html_document_create_element(document, p_str.data,
                                            p_str.length, NULL);
    test_ne(p_el, NULL);

    /* Replace. */

    code = lxb_dom_node_replace_child(body, lxb_dom_interface_node(p_el),
                                      body->first_child);
    test_eq(code, LXB_DOM_EXCEPTION_OK);

    /* Check. */

    str.data = NULL;
    str.length = 0;

    status = lxb_html_serialize_deep_str(body, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(str.data, str.length, result.data, result.length);

    /* Destroy all. */

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(replace_child_document_fragment)
{
    lexbor_status_t status;
    lxb_dom_exception_code_t code;
    lxb_html_document_t *document;
    lexbor_str_t str;
    lxb_dom_node_t *body, *frag_node;
    lxb_dom_document_t *dom_doc;
    lxb_html_element_t *p_el, *a_el;
    lxb_html_body_element_t *element;
    lxb_dom_document_fragment_t *fragment;

    static const lexbor_str_t p_str = lexbor_str("p");
    static const lexbor_str_t a_str = lexbor_str("a");
    static const lexbor_str_t result = lexbor_str("<p></p><a></a>"
                                                  "<div id=\"div-2\"></div>");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    element = lxb_html_document_body_element(document);
    body = lxb_dom_interface_node(element);

    /* Create DocumentFragment. */

    dom_doc = lxb_dom_interface_document(document);

    fragment = lxb_dom_document_fragment_interface_create(dom_doc);
    test_ne(fragment, NULL);

    /* Create <p> and <a> Element. */

    frag_node = lxb_dom_interface_node(fragment);

    p_el = lxb_html_document_create_element(document, p_str.data,
                                            p_str.length, NULL);
    test_ne(p_el, NULL);

    a_el = lxb_html_document_create_element(document, a_str.data,
                                            a_str.length, NULL);
    test_ne(a_el, NULL);

    code = lxb_dom_node_append_child(frag_node, lxb_dom_interface_node(p_el));
    test_eq(code, LXB_DOM_EXCEPTION_OK);

    code = lxb_dom_node_append_child(frag_node, lxb_dom_interface_node(a_el));
    test_eq(code, LXB_DOM_EXCEPTION_OK);

    /* Replace by DocumentFragment. */

    code = lxb_dom_node_replace_child(body, frag_node, body->first_child);
    test_eq(code, LXB_DOM_EXCEPTION_OK);

    /* Check. */

    str.data = NULL;
    str.length = 0;

    status = lxb_html_serialize_deep_str(body, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(str.data, str.length, result.data, result.length);

    /* Destroy all. */

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(remove_child)
{
    lexbor_status_t status;
    lxb_dom_exception_code_t code;
    lxb_html_document_t *document;
    lexbor_str_t str;
    lxb_dom_node_t *body;
    lxb_html_body_element_t *element;

    static const lexbor_str_t result = lexbor_str("<div id=\"div-2\"></div>");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html.data, html.length);
    test_eq(status, LXB_STATUS_OK);

    element = lxb_html_document_body_element(document);
    body = lxb_dom_interface_node(element);

    /* Replace. */

    code = lxb_dom_node_remove_child(body, body->first_child);
    test_eq(code, LXB_DOM_EXCEPTION_OK);

    /* Check. */

    str.data = NULL;
    str.length = 0;

    status = lxb_html_serialize_deep_str(body, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(str.data, str.length, result.data, result.length);

    /* Destroy all. */

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN_ARGS(replace_all_with, const char *data, const char *id,
                const char *result)
{
    lexbor_str_t str = {0};
    lxb_status_t status;
    lxb_dom_node_t *body, *parent, *node;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, (const lxb_char_t *) data,
                                     strlen(data));
    test_eq(status, LXB_STATUS_OK);

    body = lxb_dom_interface_node(lxb_html_document_body_element(document));
    parent = body->first_child;

    node = lxb_dom_node_by_id(body, (const lxb_char_t *) id, strlen(id));
    test_ne(node, NULL);

    status = lxb_dom_node_replace_all(parent, node);
    test_eq(status, LXB_STATUS_OK);

    test_eq(node->parent, parent);
    test_eq(node->prev, NULL);
    test_eq(node->next, NULL);
    test_eq(parent->first_child, node);
    test_eq(parent->last_child, node);

    status = lxb_html_serialize_deep_str(body, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str(str.data, result);

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(replace_all_with_child)
{
    TEST_CALL_ARGS(replace_all_with,
                   "<div><i>a</i><span id=n>b<b>c</b></span><i>d</i></div>"
                   "<p>e</p>",
                   "n",
                   "<div><span id=\"n\">b<b>c</b></span></div><p>e</p>");
}
TEST_END

TEST_BEGIN(replace_all_with_descendant)
{
    TEST_CALL_ARGS(replace_all_with,
                   "<div><i>a</i><p>b<span id=n>c<b>d</b></span>e</p></div>"
                   "<p>f</p>",
                   "n",
                   "<div><span id=\"n\">c<b>d</b></span></div><p>f</p>");
}
TEST_END

TEST_BEGIN(replace_all_with_child_of_other)
{
    lexbor_str_t str = {0};
    lxb_status_t status;
    lxb_dom_node_t *body, *div, *p, *span;
    lxb_html_document_t *document;

    static const lexbor_str_t data = lexbor_str("<div><i>a</i></div>"
                                                "<p>b<span>c</span>d</p>");

    static const lexbor_str_t result = lexbor_str("<div><span>c</span></div>"
                                                  "<p>bd</p>");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, data.data, data.length);
    test_eq(status, LXB_STATUS_OK);

    body = lxb_dom_interface_node(lxb_html_document_body_element(document));
    div = body->first_child;
    p = div->next;
    span = p->first_child->next;

    status = lxb_dom_node_replace_all(div, span);
    test_eq(status, LXB_STATUS_OK);

    test_eq(span->parent, div);
    test_eq(div->first_child, span);
    test_eq(div->last_child, span);

    test_eq(p->first_child->next, p->last_child);
    test_eq(p->last_child->prev, p->first_child);

    status = lxb_html_serialize_deep_str(body, &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(str.data, str.length, result.data, result.length);

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(get_by_id)
{
    size_t len;
    lexbor_status_t status;
    lxb_html_document_t *document;
    lxb_dom_node_t *body, *node;
    lxb_html_body_element_t *element;

    static const lexbor_str_t html_str = lexbor_str("<div id='idDIV'></div>");
    static const test_id_data_t id_data[] =
    {
        {lexbor_str("idDIV"), false},
        {lexbor_str(" idDIV"), true},
        {lexbor_str("idDIV "), true},
        {lexbor_str(" dDIV"), true},
        {lexbor_str("idDI "), true},
        {lexbor_str("iddiv"), true}
    };

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html_str.data, html_str.length);
    test_eq(status, LXB_STATUS_OK);

    element = lxb_html_document_body_element(document);
    body = lxb_dom_interface_node(element);

    /* Find. */

    len = sizeof(id_data) / sizeof(test_id_data_t);

    for (size_t i = 0; i < len; i++) {
        node = lxb_dom_node_by_id(body,
                                  id_data[i].str.data, id_data[i].str.length);
        if (id_data[i].is_null) {
            test_eq(node, NULL);
        }
        else {
            test_eq(node, body->first_child);
        }
    }

    /* Destroy all. */

    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(insert_before);
    TEST_ADD(append_child);
    TEST_ADD(append_child_document_fragment);
    TEST_ADD(replace_child);
    TEST_ADD(replace_child_document_fragment);
    TEST_ADD(remove_child);
    TEST_ADD(replace_all_with_child);
    TEST_ADD(replace_all_with_descendant);
    TEST_ADD(replace_all_with_child_of_other);
    TEST_ADD(get_by_id);

    TEST_RUN("lexbor/dom/node");
    TEST_RELEASE();
}
