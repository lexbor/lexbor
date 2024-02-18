/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/html/interfaces/element.h>
#include <lexbor/html/parser.h>


TEST_BEGIN(tags)
{
    size_t length;
    lxb_tag_id_t tag_id;
    lxb_status_t status;
    const lxb_char_t *name;
    lxb_html_element_t *element;
    lxb_html_document_t *document;

    static const lxb_char_t html[] = "<div>a</div>";
    size_t html_len = sizeof(html) - 1;

    /* Initialization */
    document = lxb_html_document_create();
    test_ne(document, NULL);

    /* Parse HTML */
    status = lxb_html_document_parse(document, html, html_len);
    test_eq(status, LXB_STATUS_OK);

    /* 1 */
    element = lxb_html_document_create_element(document, (lxb_char_t *) "div",
                                               3, NULL);
    test_ne(element, NULL);

    test_eq(lxb_html_element_tag_id(element), LXB_TAG_DIV);

    /* 2 */
    element = lxb_html_document_create_element(document, (lxb_char_t *) "DiV",
                                               3, NULL);
    test_ne(element, NULL);

    test_eq(lxb_html_element_tag_id(element), LXB_TAG_DIV);

    name = lxb_tag_name_by_id(lxb_html_document_tags(document),
                              lxb_html_element_tag_id(element), &length);
    test_ne(name, NULL);
    test_eq_str(name, "div");

    /* 3 */
    element = lxb_html_document_create_element(document, (lxb_char_t *) "hoho",
                                               4, NULL);
    test_ne(element, NULL);

    test_gt(lxb_html_element_tag_id(element), LXB_TAG__LAST_ENTRY);

    name = lxb_tag_name_by_id(lxb_html_document_tags(document),
                              lxb_html_element_tag_id(element), &length);
    test_ne(name, NULL);
    test_eq_str(name, "hoho");

    /* 4 */
    tag_id = lxb_html_element_tag_id(element);

    element = lxb_html_document_create_element(document, (lxb_char_t *) "hoho",
                                               4, NULL);
    test_ne(element, NULL);
    test_eq(lxb_html_element_tag_id(element), tag_id);

    /* 5 */
    element = lxb_html_document_create_element(document, (lxb_char_t *) "hOHo",
                                               4, NULL);
    test_ne(element, NULL);
    test_eq(lxb_html_element_tag_id(element), tag_id);

    /* Destroy document */
    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(tags);

    TEST_RUN("lexbor/html/tags");
    TEST_RELEASE();
}
