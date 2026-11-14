/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/utils/warc.h>


typedef struct {
    lexbor_mraw_t *mraw;
    lexbor_str_t  str;
}
records_t;


static lxb_status_t
records_append(records_t *records, const lxb_char_t *data, size_t len)
{
    if (lexbor_str_append(&records->str, records->mraw, data, len) == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
header_cb(lxb_utils_warc_t *warc)
{
    int len;
    lxb_utils_warc_field_t *field;
    records_t *records = warc->ctx;
    char buf[64];

    if (records_append(records, (const lxb_char_t *) "[", 1)) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (size_t i = 0; i < lexbor_array_obj_length(warc->fields); i++) {
        field = lexbor_array_obj_get(warc->fields, i);

        if (records_append(records, field->name.data, field->name.length)
            || records_append(records, (const lxb_char_t *) "=", 1)
            || records_append(records, field->value.data, field->value.length)
            || records_append(records, (const lxb_char_t *) "|", 1))
        {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    len = snprintf(buf, sizeof(buf), LEXBOR_FORMAT_Z "]",
                   warc->content_length);

    return records_append(records, (const lxb_char_t *) buf, len);
}

static lxb_status_t
content_cb(lxb_utils_warc_t *warc, const lxb_char_t *data,
           const lxb_char_t *end)
{
    return records_append(warc->ctx, data, end - data);
}

TEST_BEGIN_ARGS(parse_chunks, const char *data, size_t split, size_t step,
                const char *need)
{
    size_t len;
    lxb_status_t status;
    records_t records;
    lxb_utils_warc_t *warc;
    const lxb_char_t *p, *end, *last;

    len = strlen(data);

    records.mraw = lexbor_mraw_create();
    test_eq(lexbor_mraw_init(records.mraw, 1024), LXB_STATUS_OK);

    lexbor_str_init(&records.str, records.mraw, 256);
    test_ne(records.str.data, NULL);

    warc = lxb_utils_warc_create();
    status = lxb_utils_warc_init(warc, header_cb, content_cb, NULL, &records);
    test_eq(status, LXB_STATUS_OK);

    p = (const lxb_char_t *) data;
    end = p + split;
    last = p + len;

    while (p < last) {
        status = lxb_utils_warc_parse(warc, &p, end);
        test_eq(status, LXB_STATUS_OK);
        test_eq(p, end);

        end = (step != 0 && (size_t) (last - end) > step) ? end + step : last;
    }

    test_eq_str_n(records.str.data, records.str.length, need, strlen(need));

    lxb_utils_warc_destroy(warc, true);
    lexbor_mraw_destroy(records.mraw, true);
}
TEST_END

TEST_BEGIN_ARGS(parse_splits, const char *data, const char *need)
{
    size_t len = strlen(data);

    for (size_t split = 0; split <= len; split++) {
        TEST_CALL_ARGS(parse_chunks, data, split, 0, need);
    }

    TEST_CALL_ARGS(parse_chunks, data, 1, 1, need);
}
TEST_END

TEST_BEGIN(quoted_value)
{
    TEST_CALL_ARGS(parse_splits,
                   "WARC/1.0\r\n"
                   "WARC-Type: \"response\"\r\n"
                   "Content-Length: 4\r\n"
                   "\r\n"
                   "ABCD\r\n\r\n"
                   "WARC/1.0\r\n"
                   "WARC-Type: metadata\r\n"
                   "Content-Length: 2\r\n"
                   "\r\n"
                   "xy\r\n\r\n",
                   "[WARC-Type=\"response\"|Content-Length=4|4]ABCD"
                   "[WARC-Type=metadata|Content-Length=2|2]xy");
}
TEST_END

TEST_BEGIN(quoted_pair)
{
    TEST_CALL_ARGS(parse_splits,
                   "WARC/1.0\r\n"
                   "X-A: \"a\\\"b\" c\r\n"
                   "X-B: \"a\\\\\" b\r\n"
                   "Content-Length: 4\r\n"
                   "\r\n"
                   "ABCD\r\n\r\n",
                   "[X-A=\"a\\\"b\" c|X-B=\"a\\\\\" b|Content-Length=4|4]"
                   "ABCD");
}
TEST_END

TEST_BEGIN(unclosed_quote)
{
    TEST_CALL_ARGS(parse_splits,
                   "WARC/1.0\r\n"
                   "X-A: a\"b\r\n"
                   "X-B: \"a\\\r\n"
                   "Content-Length: 4\r\n"
                   "\r\n"
                   "ABCD\r\n\r\n"
                   "WARC/1.0\r\n"
                   "Content-Length: 2\r\n"
                   "\r\n"
                   "xy\r\n\r\n",
                   "[X-A=a\"b|X-B=\"a\\|Content-Length=4|4]ABCD"
                   "[Content-Length=2|2]xy");
}
TEST_END

TEST_BEGIN(quoted_value_limit)
{
    size_t len;
    lxb_char_t *data;
    lxb_status_t status;
    lxb_utils_warc_t *warc;
    const lxb_char_t *p, *end, *last;

    static const char head[] = "WARC/1.0\r\nX-A: \"";

    len = (sizeof(head) - 1) + (4096 * 64);

    data = lexbor_malloc(len);
    test_ne(data, NULL);

    memcpy(data, head, sizeof(head) - 1);
    memset(data + (sizeof(head) - 1), 'a', len - (sizeof(head) - 1));

    warc = lxb_utils_warc_create();
    status = lxb_utils_warc_init(warc, NULL, NULL, NULL, NULL);
    test_eq(status, LXB_STATUS_OK);

    p = data;
    last = data + len;

    do {
        end = ((size_t) (last - p) > 4096) ? p + 4096 : last;
        status = lxb_utils_warc_parse(warc, &p, end);
    }
    while (status == LXB_STATUS_OK && p < last);

    test_eq(status, LXB_STATUS_ABORTED);

    lxb_utils_warc_destroy(warc, true);
    lexbor_free(data);
}
TEST_END

int
main(int argc, const char * argv[])
{
    (void) argc;
    (void) argv;

    TEST_INIT();

    TEST_ADD(quoted_value);
    TEST_ADD(quoted_pair);
    TEST_ADD(unclosed_quote);
    TEST_ADD(quoted_value_limit);

    TEST_RUN("lexbor/utils/warc");
    TEST_RELEASE();
}
