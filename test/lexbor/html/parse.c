/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/parser.h>
#include <lexbor/html/serialize.h>
#include <lexbor/html/interfaces/document.h>

#include <unit/test.h>


TEST_BEGIN(document)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_document_t *document;

    lxb_char_t data[] = "<html><head></head><body><sometag><p><button>"
                        "</button></p></sometag></body></html>";
    size_t len = sizeof(data) - 1;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_serialize_tree_str(lxb_dom_interface_node(document),
                                         &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, len);

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(document_three)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_document_t *document;

    lxb_char_t data[] = "<html><head></head><body><sometag><p><button>"
                        "</button></p></sometag></body></html>";
    size_t len = sizeof(data) - 1;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_serialize_tree_str(lxb_dom_interface_node(document),
                                         &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, len);

    lxb_html_document_destroy(document);
}
TEST_END


TEST_BEGIN(document_chunk)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_document_t *document;

    lxb_char_t data[] = "<html><head></head><body><sometag><p><button>"
                        "</button></p></sometag></body></html>";
    size_t len = sizeof(data) - 1;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse_chunk_begin(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse_chunk(document, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse_chunk_end(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_serialize_tree_str(lxb_dom_interface_node(document),
                                         &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, len);

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(document_chunk_three)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_document_t *document;

    lxb_char_t data[] = "<html><head></head><body><sometag><p><button>"
                        "</button></p></sometag></body></html>";
    size_t len = sizeof(data) - 1;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    /* One */
    status = lxb_html_document_parse_chunk_begin(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse_chunk(document, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse_chunk_end(document);
    test_eq(status, LXB_STATUS_OK);

    /* Two */
    status = lxb_html_document_parse_chunk_begin(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse_chunk(document, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse_chunk_end(document);
    test_eq(status, LXB_STATUS_OK);

    /* Three */
    status = lxb_html_document_parse_chunk_begin(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse_chunk(document, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse_chunk_end(document);
    test_eq(status, LXB_STATUS_OK);

    /* Serialization */
    status = lxb_html_serialize_tree_str(lxb_dom_interface_node(document),
                                         &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, len);

    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(parser)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;

    lxb_char_t data[] = "<html><head></head><body><sometag><p><button>"
                        "</button></p></sometag></body></html>";
    size_t len = sizeof(data) - 1;

    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);
    test_eq(status, LXB_STATUS_OK);

    document = lxb_html_parse(parser, data, len);
    test_ne(document, NULL);

    status = lxb_html_serialize_tree_str(lxb_dom_interface_node(document),
                                         &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, len);

    lxb_html_parser_destroy(parser);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(parser_three)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;

    lxb_char_t data[] = "<html><head></head><body><sometag><p><button>"
                        "</button></p></sometag></body></html>";
    size_t len = sizeof(data) - 1;

    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);
    test_eq(status, LXB_STATUS_OK);

    document = lxb_html_parse(parser, data, len);
    test_ne(document, NULL);
    lxb_html_document_destroy(document);

    document = lxb_html_parse(parser, data, len);
    test_ne(document, NULL);
    lxb_html_document_destroy(document);

    document = lxb_html_parse(parser, data, len);
    test_ne(document, NULL);

    status = lxb_html_serialize_tree_str(lxb_dom_interface_node(document),
                                         &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, len);

    lxb_html_parser_destroy(parser);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(parser_chunk)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;

    lxb_char_t data[] = "<html><head></head><body><sometag><p><button>"
                        "</button></p></sometag></body></html>";
    size_t len = sizeof(data) - 1;

    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);
    test_eq(status, LXB_STATUS_OK);

    document = lxb_html_parse_chunk_begin(parser);
    test_ne(document, NULL);

    status = lxb_html_parse_chunk_process(parser, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_parse_chunk_end(parser);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_serialize_tree_str(lxb_dom_interface_node(document),
                                         &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, len);

    lxb_html_parser_destroy(parser);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(parser_chunk_three)
{
    lxb_status_t status;
    lexbor_str_t str = {0};
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;

    lxb_char_t data[] = "<html><head></head><body><sometag><p><button>"
                        "</button></p></sometag></body></html>";
    size_t len = sizeof(data) - 1;

    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);
    test_eq(status, LXB_STATUS_OK);

    /* One */
    document = lxb_html_parse_chunk_begin(parser);
    test_ne(document, NULL);

    status = lxb_html_parse_chunk_process(parser, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_parse_chunk_end(parser);
    test_eq(status, LXB_STATUS_OK);

    lxb_html_document_destroy(document);

    /* Two */
    document = lxb_html_parse_chunk_begin(parser);
    test_ne(document, NULL);

    status = lxb_html_parse_chunk_process(parser, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_parse_chunk_end(parser);
    test_eq(status, LXB_STATUS_OK);

    lxb_html_document_destroy(document);

    /* Three */
    document = lxb_html_parse_chunk_begin(parser);
    test_ne(document, NULL);

    status = lxb_html_parse_chunk_process(parser, data, len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_parse_chunk_end(parser);
    test_eq(status, LXB_STATUS_OK);

    /* Serialization */
    status = lxb_html_serialize_tree_str(lxb_dom_interface_node(document),
                                         &str);
    test_eq(status, LXB_STATUS_OK);

    test_eq(str.length, len);

    lxb_html_parser_destroy(parser);
    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(document);
    TEST_ADD(document_three);
    TEST_ADD(document_chunk);
    TEST_ADD(document_chunk_three);

    TEST_ADD(parser);
    TEST_ADD(parser_three);
    TEST_ADD(parser_chunk);
    TEST_ADD(parser_chunk_three);

    TEST_RUN("lexbor/html/parse");
    TEST_RELEASE();
}
