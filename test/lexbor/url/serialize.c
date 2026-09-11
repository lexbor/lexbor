/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>
#include <lexbor/url/url.h>


typedef struct {
    lexbor_str_t input;
    lexbor_str_t fragment;
}
serialize_entry_t;

typedef struct {
    lxb_char_t   data[128];
    size_t       length;
    size_t       calls;
    lxb_status_t status;
}
serialize_ctx_t;


static const serialize_entry_t entries[] = {
    {
        lexbor_str("https://example.org/"),
        {NULL, 0}
    },
    {
        lexbor_str("https://example.org/#"),
        lexbor_str("")
    },
    {
        lexbor_str("https://example.org/#frag"),
        lexbor_str("frag")
    },
    {
        lexbor_str("https://example.org/?"),
        {NULL, 0}
    },
    {
        lexbor_str("https://example.org/?#"),
        lexbor_str("")
    },
    {
        lexbor_str("https://example.org/?#frag"),
        lexbor_str("frag")
    },
    {
        lexbor_str("https://example.org/?q=1"),
        {NULL, 0}
    },
    {
        lexbor_str("https://example.org/?q=1#"),
        lexbor_str("")
    },
    {
        lexbor_str("https://example.org/?q=1#frag"),
        lexbor_str("frag")
    },
    {
        lexbor_str("https://example.org/#a b"),
        lexbor_str("a%20b")
    },
    {
        lexbor_str("https://example.org/#a%20b"),
        lexbor_str("a%20b")
    },
    {
        lexbor_str("https://example.org/#\xC3\xA9?x#y"),
        lexbor_str("%C3%A9?x#y")
    }
};


static lxb_status_t
serialize_callback(const lxb_char_t *data, size_t length, void *ctx)
{
    serialize_ctx_t *context = ctx;

    context->calls++;

    if (data == NULL || length > sizeof(context->data) - context->length) {
        return LXB_STATUS_ERROR;
    }

    memcpy(context->data + context->length, data, length);
    context->length += length;

    return context->status;
}

TEST_BEGIN_ARGS(test_serialize_fragment, const serialize_entry_t *entry,
                lxb_status_t callback_status)
{
    size_t calls;
    lxb_status_t status;
    lxb_url_t *url;
    lxb_url_parser_t parser;
    serialize_ctx_t context = {0};

    TEST_PRINTLN("URL: %.*s", (int) entry->input.length, entry->input.data);

    status = lxb_url_parser_init(&parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    url = lxb_url_parse(&parser, NULL, entry->input.data, entry->input.length);
    test_ne(url, NULL);

    context.status = callback_status;
    status = lxb_url_serialize_fragment(url, serialize_callback, &context);

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);

    /* An empty fragment calls the callback; an absent fragment does not. */
    calls = (entry->fragment.data != NULL) ? 1 : 0;
    test_eq_size(context.calls, calls);
    test_eq(status, calls != 0 ? callback_status : LXB_STATUS_OK);

    if (entry->fragment.data != NULL) {
        test_eq_str_n(context.data, context.length,
                      entry->fragment.data, entry->fragment.length);
    }
    else {
        test_eq_size(context.length, 0);
    }
}
TEST_END

TEST_BEGIN(serialize_fragment)
{
    for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
        TEST_CALL_ARGS(test_serialize_fragment, &entries[i], LXB_STATUS_OK);
    }
}
TEST_END

TEST_BEGIN(serialize_fragment_callback_error)
{
    for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
        TEST_CALL_ARGS(test_serialize_fragment, &entries[i],
                       LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }
}
TEST_END


int
main(int argc, const char *argv[])
{
    TEST_INIT();

    TEST_ADD(serialize_fragment);
    TEST_ADD(serialize_fragment_callback_error);

    TEST_RUN("lexbor/url/serialize");
    TEST_RELEASE();
}
