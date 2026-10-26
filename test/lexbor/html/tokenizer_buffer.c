/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Dave Protasowski <david.protasowski@shopify.com>
 */

#include <lexbor/dom/interfaces/character_data.h>
#include <lexbor/html/parser.h>
#include <lexbor/html/interfaces/document.h>

#include <unit/test.h>


static void *
test_realloc_fail(void *dst, size_t size)
{
    (void) dst;
    (void) size;

    return NULL;
}


TEST_BEGIN_ARGS(chunked_script, bool closed)
{
    size_t i, j, capacity, length, growths = 0;
    lxb_status_t status;
    lxb_char_t *data;
    lxb_char_t chunk[4096];
    lxb_dom_node_t *script, *text;
    lxb_dom_character_data_t *ch_data;
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;
    lxb_html_tokenizer_t *tkz;

    length = 256 * sizeof(chunk);
    data = lexbor_malloc(length);
    test_ne(data, NULL);

    parser = lxb_html_parser_create();
    test_ne(parser, NULL);
    status = lxb_html_parser_init(parser);
    test_eq(status, LXB_STATUS_OK);

    document = lxb_html_parse_chunk_begin(parser);
    test_ne(document, NULL);
    tkz = lxb_html_parser_tokenizer(parser);

    status = lxb_html_parse_chunk_process(parser,
                                          (const lxb_char_t *) "<script>", 8);
    test_eq(status, LXB_STATUS_OK);

    for (i = 0; i < 256; i++) {
        for (j = 0; j < sizeof(chunk); j++) {
            chunk[j] = (lxb_char_t) ('a' + (i + j) % 26);
        }

        chunk[0] = (lxb_char_t) ('a' + i / 16);
        chunk[1] = (lxb_char_t) ('a' + i % 16);
        memcpy(data + i * sizeof(chunk), chunk, sizeof(chunk));

        capacity = tkz->end - tkz->start;
        status = lxb_html_parse_chunk_process(parser, chunk, sizeof(chunk));
        test_eq(status, LXB_STATUS_OK);

        if ((size_t) (tkz->end - tkz->start) != capacity) {
            growths++;
        }
    }

    if (closed) {
        status = lxb_html_parse_chunk_process(parser,
                                              (const lxb_char_t *) "</script>",
                                              9);
        test_eq(status, LXB_STATUS_OK);
    }

    status = lxb_html_parse_chunk_end(parser);
    test_eq(status, LXB_STATUS_OK);
    test_ne(document->head, NULL);
    script = lxb_dom_interface_node(document->head)->first_child;
    test_ne(script, NULL);
    test_eq(script->local_name, LXB_TAG_SCRIPT);
    text = script->first_child;
    test_ne(text, NULL);
    test_eq(text->type, LXB_DOM_NODE_TYPE_TEXT);
    test_eq(text->next, NULL);
    ch_data = lxb_dom_interface_character_data(text);
    test_eq_size(ch_data->data.length, length);
    test_eq(memcmp(ch_data->data.data, data, length), 0);

    lexbor_free(data);
    lxb_html_document_destroy(document);
    lxb_html_parser_destroy(parser);

    test_eq(growths > 0 && growths <= 10, true);
}
TEST_END

TEST_BEGIN(chunked_script_eof)
{
    TEST_CALL_ARGS(chunked_script, false);
}
TEST_END

TEST_BEGIN(chunked_script_closed)
{
    TEST_CALL_ARGS(chunked_script, true);
}
TEST_END

TEST_BEGIN(large_append)
{
    size_t capacity, length;
    lxb_char_t *data;
    lxb_status_t status;
    lxb_html_tokenizer_t *tkz;
    const lxb_char_t prefix[] = "retained prefix";

    tkz = lxb_html_tokenizer_create();
    test_ne(tkz, NULL);
    status = lxb_html_tokenizer_init(tkz);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_tokenizer_temp_append(tkz, prefix, sizeof(prefix) - 1);
    test_eq(status, LXB_STATUS_OK);
    capacity = tkz->end - tkz->start;
    length = capacity * 3;
    data = lexbor_malloc(length);
    test_ne(data, NULL);
    memset(data, 'y', length);

    status = lxb_html_tokenizer_temp_append(tkz, data, length);
    test_eq(status, LXB_STATUS_OK);
    test_eq_size((tkz->pos - tkz->start), sizeof(prefix) - 1 + length);
    test_eq((size_t) (tkz->end - tkz->start) >= sizeof(prefix) - 1 + length,
            true);
    test_eq(memcmp(tkz->start, prefix, sizeof(prefix) - 1), 0);
    test_eq(memcmp(tkz->start + sizeof(prefix) - 1, data, length), 0);

    lexbor_free(data);
    lxb_html_tokenizer_destroy(tkz);
}
TEST_END

TEST_BEGIN(overflow)
{
    lxb_char_t *start, *pos;
    const lxb_char_t *end;
    lxb_status_t status;
    lxb_html_tokenizer_t *tkz;
    const lxb_char_t data[] = "retained contents";

    tkz = lxb_html_tokenizer_create();
    test_ne(tkz, NULL);
    status = lxb_html_tokenizer_init(tkz);
    test_eq(status, LXB_STATUS_OK);
    status = lxb_html_tokenizer_temp_append(tkz, data, sizeof(data) - 1);
    test_eq(status, LXB_STATUS_OK);

    start = tkz->start;
    pos = tkz->pos;
    end = tkz->end;
    status = lxb_html_tokenizer_temp_realloc(tkz, SIZE_MAX);
    test_eq(status, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    test_eq(tkz->status, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    test_eq(tkz->start, start);
    test_eq(tkz->pos, pos);
    test_eq(tkz->end, end);
    test_eq(memcmp(tkz->start, data, sizeof(data) - 1), 0);

    lxb_html_tokenizer_destroy(tkz);
}
TEST_END

TEST_BEGIN(allocation_failure)
{
    size_t capacity;
    lxb_char_t *start, *pos;
    const lxb_char_t *end;
    lxb_status_t status, setup_status, restore_status;
    lxb_html_tokenizer_t *tkz;
    const lxb_char_t data[] = "retained contents";

    tkz = lxb_html_tokenizer_create();
    test_ne(tkz, NULL);
    status = lxb_html_tokenizer_init(tkz);
    test_eq(status, LXB_STATUS_OK);
    status = lxb_html_tokenizer_temp_append(tkz, data, sizeof(data) - 1);
    test_eq(status, LXB_STATUS_OK);

    start = tkz->start;
    pos = tkz->pos;
    end = tkz->end;
    capacity = end - start;

    setup_status = lexbor_memory_setup(malloc, test_realloc_fail, calloc, free);
    status = lxb_html_tokenizer_temp_realloc(tkz, 1);
    restore_status = lexbor_memory_setup(malloc, realloc, calloc, free);

    test_eq(setup_status, LXB_STATUS_OK);
    test_eq(restore_status, LXB_STATUS_OK);
    test_eq(status, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    test_eq(tkz->status, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    test_eq(tkz->start, start);
    test_eq(tkz->pos, pos);
    test_eq(tkz->end, end);
    test_eq(memcmp(tkz->start, data, sizeof(data) - 1), 0);

    lxb_html_tokenizer_clean(tkz);
    test_eq(tkz->status, LXB_STATUS_OK);
    test_eq(tkz->pos, tkz->start);

    status = lxb_html_tokenizer_temp_append(tkz, data, sizeof(data) - 1);
    test_eq(status, LXB_STATUS_OK);
    status = lxb_html_tokenizer_temp_realloc(tkz, 1);
    test_eq(status, LXB_STATUS_OK);
    test_eq(tkz->status, LXB_STATUS_OK);
    test_eq_size((tkz->pos - tkz->start), sizeof(data) - 1);
    test_eq((size_t) (tkz->end - tkz->start) > capacity, true);
    test_eq(memcmp(tkz->start, data, sizeof(data) - 1), 0);

    lxb_html_tokenizer_destroy(tkz);
}
TEST_END

int
main(int argc, const char *argv[])
{
    TEST_INIT();

    TEST_ADD(chunked_script_eof);
    TEST_ADD(chunked_script_closed);
    TEST_ADD(large_append);
    TEST_ADD(overflow);
    TEST_ADD(allocation_failure);

    TEST_RUN("lexbor/html/tokenizer_buffer");
    TEST_RELEASE();
}
