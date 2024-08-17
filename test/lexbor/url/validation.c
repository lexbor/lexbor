/*
 * Copyright (C) 2024 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */


#include <unit/test.h>
#include <lexbor/url/url.h>


typedef struct {
    lxb_char_t           *source;
    lxb_char_t           *result;
    lxb_url_error_type_t *errors;
}
test_url_t;

typedef struct {
    lexbor_str_t     str;
    lxb_url_parser_t *parser;
}
test_ctx_t;

static const test_url_t test_urls[] =
{
    {
        .source = (lxb_char_t *) "  http://lexbor.com/",
        .result = (lxb_char_t *) "http://lexbor.com/",
        .errors = (lxb_url_error_type_t[]) {
            LXB_URL_ERROR_TYPE_INVALID_URL_UNIT,
            LXB_URL_ERROR_TYPE__LAST_ENTRY
        },
    },
    {
        .source = (lxb_char_t *) "http://lexbor.com/    ",
        .result = (lxb_char_t *) "http://lexbor.com/",
        .errors = (lxb_url_error_type_t[]) {
            LXB_URL_ERROR_TYPE_INVALID_URL_UNIT,
            LXB_URL_ERROR_TYPE__LAST_ENTRY
        },
    },
    {
        .source = (lxb_char_t *) "http://lexb\nor.com/",
        .result = (lxb_char_t *) "http://lexbor.com/",
        .errors = (lxb_url_error_type_t[]) {
            LXB_URL_ERROR_TYPE_INVALID_URL_UNIT,
            LXB_URL_ERROR_TYPE__LAST_ENTRY
        },
    },
    {
        .source = (lxb_char_t *) "http://lexb\tor.com/",
        .result = (lxb_char_t *) "http://lexbor.com/",
        .errors = (lxb_url_error_type_t[]) {
            LXB_URL_ERROR_TYPE_INVALID_URL_UNIT,
            LXB_URL_ERROR_TYPE__LAST_ENTRY
        },
    },
    {
        .source = (lxb_char_t *) "http://lexb\ror.com/",
        .result = (lxb_char_t *) "http://lexbor.com/",
        .errors = (lxb_url_error_type_t[]) {
            LXB_URL_ERROR_TYPE_INVALID_URL_UNIT,
            LXB_URL_ERROR_TYPE__LAST_ENTRY
        },
    },
    {
        .source = (lxb_char_t *) "http://lexbor.com/path/to/world",
        .result = (lxb_char_t *) "http://lexbor.com/path/to/world",
        .errors = (lxb_url_error_type_t[]) {LXB_URL_ERROR_TYPE__LAST_ENTRY},
    },
    {
        .source = (lxb_char_t *) "http://lexbor.com/path/to/wo\u9FFFrld",
        .result = (lxb_char_t *) "http://lexbor.com/path/to/wo%E9%BF%BFrld",
        .errors = (lxb_url_error_type_t[]) {LXB_URL_ERROR_TYPE__LAST_ENTRY},
    },
    {
        .source = (lxb_char_t *) "http://lexbor.com/path/to/wo\U0007FFFErld",
        .result = (lxb_char_t *) "http://lexbor.com/path/to/wo%F1%BF%BF%BErld",
        .errors = (lxb_url_error_type_t[]) {
            LXB_URL_ERROR_TYPE_INVALID_URL_UNIT,
            LXB_URL_ERROR_TYPE__LAST_ENTRY
        },
    }
};


static lxb_status_t
serialize_cb(const lxb_char_t *data, size_t len, void *ctx)
{
    lxb_char_t *chr;
    test_ctx_t *context = ctx;

    chr = lexbor_str_append(&context->str, context->parser->mraw, data, len);

    return (chr != NULL) ? LXB_STATUS_OK : LXB_STATUS_ERROR_MEMORY_ALLOCATION;
}

static size_t
test_log_count(const test_url_t *turl)
{
    lxb_url_error_type_t *type;

    type = turl->errors;

    while (*type != LXB_URL_ERROR_TYPE__LAST_ENTRY) {
        type += 1;
    }

    return type - turl->errors;
}


TEST_BEGIN(validation)
{
    size_t len;
    lxb_url_t *url;
    lxb_status_t status;
    lxb_url_parser_t *parser;
    const test_url_t *turl;
    test_ctx_t context;
    lexbor_plog_t *log;
    lexbor_plog_entry_t *entry;

    parser = lxb_url_parser_create();
    status = lxb_url_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    context.parser = parser;

    lexbor_str_init(&context.str, parser->mraw, 1024);
    test_ne(context.str.data, NULL);

    for (size_t i = 0; i < sizeof(test_urls) / sizeof(test_url_t); i++) {
        turl = &test_urls[i];

        url = lxb_url_parse(parser, NULL, turl->source,
                            strlen((const char *) turl->source));
        test_ne(url, NULL);

        status = lxb_url_serialize(url, serialize_cb, &context, false);
        test_eq(status, LXB_STATUS_OK);

        test_eq_str(turl->result, context.str.data);
        lexbor_str_clean(&context.str);

        log = parser->log;
        len = test_log_count(turl);

        if (log == NULL) {
            if (len > 0) {
                TEST_FAILURE("Parser log is NULL, but we expecting "LEXBOR_FORMAT_Z,
                             len);
            }

            lxb_url_parser_clean(parser);
            continue;
        }

        test_eq(len, lexbor_plog_length(log));

        for (size_t l = 0; l < lexbor_plog_length(log); l++) {
            entry = lexbor_array_obj_get(&log->list, l);

            test_eq(entry->id, turl->errors[l]);
        }

        lxb_url_parser_clean(parser);
    }

    lxb_url_parser_memory_destroy(parser);
    lxb_url_parser_destroy(parser, true);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(validation);

    TEST_RUN("lexbor/url/validation");
    TEST_RELEASE();
}
