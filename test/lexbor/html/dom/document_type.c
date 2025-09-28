/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>
#include <lexbor/dom/interfaces/document_type.h>

#include <unit/test.h>

TEST_BEGIN(create_only_name)
{
    size_t length;
    const lxb_char_t *name;
    lxb_dom_document_t *domdoc;
    lxb_html_document_t *document;
    lxb_dom_document_type_t *doctype;

    static const lexbor_str_t name_str = lexbor_str("lexbor");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    domdoc = lxb_dom_interface_document(document);

    doctype = lxb_dom_document_type_create(domdoc, name_str.data,
                                           name_str.length, NULL, 0, NULL, 0,
                                           NULL);
    test_ne(doctype, NULL);

    name = lxb_dom_document_type_name(doctype, &length);
    test_ne(name, NULL);
    test_eq_str_n(name, length, name_str.data, name_str.length);

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(create_only_name_html)
{
    size_t length;
    const lxb_char_t *name;
    lxb_dom_document_t *domdoc;
    lxb_html_document_t *document;
    lxb_dom_document_type_t *doctype;

    static const lexbor_str_t name_str = lexbor_str("html");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    domdoc = lxb_dom_interface_document(document);

    doctype = lxb_dom_document_type_create(domdoc, name_str.data,
                                           name_str.length, NULL, 0, NULL, 0,
                                           NULL);
    test_ne(doctype, NULL);

    name = lxb_dom_document_type_name(doctype, &length);
    test_ne(name, NULL);
    test_eq_str_n(name, length, name_str.data, name_str.length);

    name = lxb_dom_document_type_public_id(doctype, &length);
    test_eq(name, NULL);

    name = lxb_dom_document_type_system_id(doctype, &length);
    test_eq(name, NULL);

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(create_full)
{
    size_t length;
    const lxb_char_t *name;
    lxb_dom_document_t *domdoc;
    lxb_html_document_t *document;
    lxb_dom_document_type_t *doctype;

    static const lexbor_str_t name_str = lexbor_str("lexbor");
    static const lexbor_str_t public_str = lexbor_str("-//W3C//DTD HTML 4.01 Transitional//EN");
    static const lexbor_str_t system_str = lexbor_str("http://www.w3.org/TR/html4/loose.dtd");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    domdoc = lxb_dom_interface_document(document);

    doctype = lxb_dom_document_type_create(domdoc,
                                           name_str.data, name_str.length,
                                           public_str.data, public_str.length,
                                           system_str.data, system_str.length,
                                           NULL);
    test_ne(doctype, NULL);

    name = lxb_dom_document_type_name(doctype, &length);
    test_ne(name, NULL);
    test_eq_str_n(name, length, name_str.data, name_str.length);

    name = lxb_dom_document_type_public_id(doctype, &length);
    test_ne(name, NULL);
    test_eq_str_n(name, length, public_str.data, public_str.length);

    name = lxb_dom_document_type_system_id(doctype, &length);
    test_ne(name, NULL);
    test_eq_str_n(name, length, system_str.data, system_str.length);

    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(create_only_name);
    TEST_ADD(create_only_name_html);
    TEST_ADD(create_full);

    TEST_RUN("lexbor/html/dom/document_type");
    TEST_RELEASE();
}
