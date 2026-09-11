/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/dom/dom.h>
#include <lexbor/html/html.h>


TEST_BEGIN_ARGS(test_check_data, lxb_dom_character_data_t *ch_data,
                const lxb_char_t *data, size_t length)
{
    test_ne(ch_data->data.data, NULL);

    /* Check room for the terminator even when ASan is disabled. */
    test_eq(lexbor_str_size(&ch_data->data) > length, true);
    test_eq_str_n(ch_data->data.data, ch_data->data.length, data, length);
    test_eq(ch_data->data.data[length], 0x00);
}
TEST_END

TEST_BEGIN(replace_without_data)
{
    lxb_status_t status;
    lxb_html_document_t *document;
    lxb_dom_character_data_t *ch_data;

    static const lexbor_str_t values[] = {
        lexbor_str(""),
        lexbor_str("New data")
    };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        document = lxb_html_document_create();
        test_ne(document, NULL);

        ch_data = lxb_dom_character_data_interface_create(
                                       lxb_dom_interface_document(document));
        test_ne(ch_data, NULL);
        test_eq(ch_data->data.data, NULL);

        status = lxb_dom_character_data_replace(ch_data, values[i].data,
                                                values[i].length, 0, 0);
        test_eq(status, LXB_STATUS_OK);

        TEST_CALL_ARGS(test_check_data, ch_data,
                       values[i].data, values[i].length);

        lxb_dom_character_data_interface_destroy(ch_data);
        lxb_html_document_destroy(document);
    }
}
TEST_END

TEST_BEGIN(replace_empty)
{
    lxb_status_t status;
    lxb_html_document_t *document;
    lxb_dom_text_t *text;
    lxb_dom_character_data_t *ch_data;

    static const lexbor_str_t initial = lexbor_str("Old data");
    static const lexbor_str_t empty = lexbor_str("");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    text = lxb_dom_document_create_text_node(
                 lxb_dom_interface_document(document),
                 initial.data, initial.length);
    test_ne(text, NULL);
    ch_data = lxb_dom_interface_character_data(text);

    status = lxb_dom_character_data_replace(ch_data, empty.data, 0, 0, 0);
    test_eq(status, LXB_STATUS_OK);

    TEST_CALL_ARGS(test_check_data, ch_data, empty.data, 0);

    lxb_dom_document_destroy_interface(text);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(replace_capacity_boundaries)
{
    size_t capacity, length;
    lxb_char_t *data;
    lxb_status_t status;
    lxb_html_document_t *document;
    lxb_dom_text_t *text;
    lxb_dom_character_data_t *ch_data;

    static const lexbor_str_t initial = lexbor_str("Old");

    /* Use a fresh buffer for capacity - 1, capacity, and capacity + 1. */
    for (size_t i = 0; i < 3; i++) {
        document = lxb_html_document_create();
        test_ne(document, NULL);

        text = lxb_dom_document_create_text_node(
                     lxb_dom_interface_document(document),
                     initial.data, initial.length);
        test_ne(text, NULL);
        ch_data = lxb_dom_interface_character_data(text);

        capacity = lexbor_str_size(&ch_data->data);
        length = capacity - 1 + i;

        data = lexbor_malloc(length);
        test_ne(data, NULL);
        memset(data, 'x', length);

        status = lxb_dom_character_data_replace(ch_data, data, length, 0, 0);
        test_eq(status, LXB_STATUS_OK);

        TEST_CALL_ARGS(test_check_data, ch_data, data, length);

        lexbor_free(data);
        lxb_dom_document_destroy_interface(text);
        lxb_html_document_destroy(document);
    }
}
TEST_END

TEST_BEGIN(text_content_set_at_capacity)
{
    size_t length, neighbor_capacity;
    lxb_char_t *data;
    lxb_status_t status;
    lxb_html_document_t *document;
    lxb_dom_document_t *dom_doc;
    lxb_dom_node_t *node;
    lxb_dom_text_t *neighbor;
    lxb_dom_character_data_t *ch_data;

    static const lexbor_str_t initial = lexbor_str("Old");
    static const lexbor_str_t target = lexbor_str("target");
    static const lexbor_str_t neighbor_data = lexbor_str("Neighbor");
    static const lxb_dom_node_type_t types[] = {
        LXB_DOM_NODE_TYPE_TEXT,
        LXB_DOM_NODE_TYPE_COMMENT,
        LXB_DOM_NODE_TYPE_PROCESSING_INSTRUCTION
    };

    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        document = lxb_html_document_create();
        test_ne(document, NULL);
        dom_doc = lxb_dom_interface_document(document);

        switch (types[i]) {
            case LXB_DOM_NODE_TYPE_TEXT:
                node = lxb_dom_interface_node(lxb_dom_document_create_text_node(
                                    dom_doc, initial.data, initial.length));
                break;

            case LXB_DOM_NODE_TYPE_COMMENT:
                node = lxb_dom_interface_node(lxb_dom_document_create_comment(
                                    dom_doc, initial.data, initial.length));
                break;

            default:
                node = lxb_dom_interface_node(
                    lxb_dom_document_create_processing_instruction(dom_doc,
                        target.data, target.length,
                        initial.data, initial.length));
                break;
        }

        test_ne(node, NULL);
        ch_data = lxb_dom_interface_character_data(node);
        length = lexbor_str_size(&ch_data->data);

        /* Keep another allocation after the buffer that must grow. */
        neighbor = lxb_dom_document_create_text_node(dom_doc,
                            neighbor_data.data, neighbor_data.length);
        test_ne(neighbor, NULL);
        neighbor_capacity = lexbor_str_size(&neighbor->char_data.data);

        data = lexbor_malloc(length);
        test_ne(data, NULL);
        memset(data, 'x', length);

        status = lxb_dom_node_text_content_set(node, data, length);
        test_eq(status, LXB_STATUS_OK);

        TEST_CALL_ARGS(test_check_data, ch_data, data, length);
        TEST_CALL_ARGS(test_check_data, &neighbor->char_data,
                       neighbor_data.data, neighbor_data.length);
        test_eq_size(lexbor_str_size(&neighbor->char_data.data),
                     neighbor_capacity);

        lexbor_free(data);
        lxb_dom_document_destroy_interface(node);
        lxb_dom_document_destroy_interface(neighbor);
        lxb_html_document_destroy(document);
    }
}
TEST_END


int
main(int argc, const char *argv[])
{
    TEST_INIT();

    TEST_ADD(replace_without_data);
    TEST_ADD(replace_empty);
    TEST_ADD(replace_capacity_boundaries);
    TEST_ADD(text_content_set_at_capacity);

    TEST_RUN("lexbor/dom/character_data");
    TEST_RELEASE();
}
