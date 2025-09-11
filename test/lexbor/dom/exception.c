/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/dom/exception.h>
#include <lexbor/html/html.h>


TEST_BEGIN(create_by_name)
{
    lxb_dom_exception_t *exception;
    lxb_dom_document_t *dom_doc;
    lxb_html_document_t *document;

    static const lexbor_str_t msg = lexbor_str("Simple error");
    static const lexbor_str_t name = lexbor_str("MyError");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_doc = lxb_html_document_original_ref(document);

    /* Create Exception. */

    exception = lxb_dom_exception_create(dom_doc, msg.data, msg.length,
                                         name.data, name.length);
    test_ne(exception, NULL);

    test_eq_str_n(exception->message.data, exception->message.length,
                  msg.data, msg.length);

    test_eq_str_n(exception->name.data, exception->name.length,
                  name.data, name.length);

    /* Destroy all. */

    lxb_dom_exception_destroy(exception);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(create_by_default)
{
    lxb_dom_exception_t *exception;
    lxb_dom_document_t *dom_doc;
    lxb_html_document_t *document;
    const lexbor_str_t *msg, *name;

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_doc = lxb_html_document_original_ref(document);

    /* Create Exception. */

    exception = lxb_dom_exception_create(dom_doc, NULL, 0, NULL, 0);
    test_ne(exception, NULL);

    msg = lxb_dom_exception_message_by_code(LXB_DOM_EXCEPTION_ERR);
    name = lxb_dom_exception_name_by_code(LXB_DOM_EXCEPTION_ERR);

    test_eq_str_n(exception->message.data, exception->message.length,
                  msg->data, msg->length);

    test_eq_str_n(exception->name.data, exception->name.length,
                  name->data, name->length);

    /* Destroy all. */

    lxb_dom_exception_destroy(exception);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(create_by_code)
{
    lxb_dom_exception_t *exception;
    lxb_dom_document_t *dom_doc;
    lxb_html_document_t *document;
    const lexbor_str_t *name;

    static const lexbor_str_t msg = lexbor_str("Simple error");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_doc = lxb_html_document_original_ref(document);

    /* Create Exception. */

    exception = lxb_dom_exception_create_by_code(dom_doc, msg.data, msg.length,
                                                 LXB_DOM_EXCEPTION_NETWORK_ERR);
    test_ne(exception, NULL);

    name = lxb_dom_exception_name_by_code(LXB_DOM_EXCEPTION_NETWORK_ERR);

    test_eq_str_n(exception->message.data, exception->message.length,
                  msg.data, msg.length);

    test_eq_str_n(exception->name.data, exception->name.length,
                  name->data, name->length);

    /* Destroy all. */

    lxb_dom_exception_destroy(exception);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(create_using_reserved_name)
{
    lxb_dom_exception_t *exception;
    lxb_dom_document_t *dom_doc;
    lxb_html_document_t *document;
    const lexbor_str_t *n_msg, *n_name;

    static const lexbor_str_t name = lexbor_str("InvalidStateError");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_doc = lxb_html_document_original_ref(document);

    /* Create Exception. */

    exception = lxb_dom_exception_create(dom_doc, NULL, 0,
                                         name.data, name.length);
    test_ne(exception, NULL);

    n_msg = lxb_dom_exception_message_by_code(LXB_DOM_EXCEPTION_INVALID_STATE_ERR);
    n_name = lxb_dom_exception_name_by_code(LXB_DOM_EXCEPTION_INVALID_STATE_ERR);

    test_eq_str_n(exception->message.data, exception->message.length,
                  n_msg->data, n_msg->length);

    test_eq_str_n(exception->name.data, exception->name.length,
                  n_name->data, n_name->length);

    /* Destroy all. */

    lxb_dom_exception_destroy(exception);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(last_error)
{
    lxb_dom_exception_t *exception;
    lxb_dom_document_t *dom_doc;
    lxb_html_document_t *document;
    const lexbor_str_t *n_msg, *n_name;

    static const lexbor_str_t name = lexbor_str("OptOutError");

    /* Create HTML Document. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    dom_doc = lxb_html_document_original_ref(document);

    /* Create Exception. */

    exception = lxb_dom_exception_create(dom_doc, NULL, 0,
                                         name.data, name.length);
    test_ne(exception, NULL);

    n_msg = lxb_dom_exception_message_by_code(LXB_DOM_EXCEPTION_OPT_OUT_ERR);
    n_name = lxb_dom_exception_name_by_code(LXB_DOM_EXCEPTION_OPT_OUT_ERR);

    test_eq_str_n(exception->message.data, exception->message.length,
                  n_msg->data, n_msg->length);

    test_eq_str_n(exception->name.data, exception->name.length,
                  n_name->data, n_name->length);

    /* Destroy all. */

    lxb_dom_exception_destroy(exception);
    lxb_html_document_destroy(document);
}
TEST_END


int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(create_by_name);
    TEST_ADD(create_by_default);
    TEST_ADD(create_by_code);
    TEST_ADD(create_using_reserved_name);
    TEST_ADD(last_error);

    TEST_RUN("lexbor/dom/exception");
    TEST_RELEASE();
}
