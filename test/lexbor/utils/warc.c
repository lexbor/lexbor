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

static lxb_status_t
content_end_cb(lxb_utils_warc_t *warc)
{
    return records_append(warc->ctx, (const lxb_char_t *) "$", 1);
}

TEST_BEGIN_ARGS(parse_chunks, const char *data, size_t split, size_t step,
                const char *need, const char *error)
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
    status = lxb_utils_warc_init(warc, header_cb, content_cb, content_end_cb,
                                 &records);
    test_eq(status, LXB_STATUS_OK);

    p = (const lxb_char_t *) data;
    end = p + split;
    last = p + len;

    while (p < last) {
        status = lxb_utils_warc_parse(warc, &p, end);
        if (status != LXB_STATUS_OK) {
            break;
        }

        test_eq(p, end);

        end = (step != 0 && (size_t) (last - end) > step) ? end + step : last;
    }

    if (error != NULL) {
        test_eq(status, LXB_STATUS_ABORTED);
        test_ne(warc->error, NULL);
        test_eq_str(warc->error, error);
    }
    else {
        test_eq(status, LXB_STATUS_OK);
        test_eq(lxb_utils_warc_parse_eof(warc), LXB_STATUS_OK);
    }

    test_eq_str_n(records.str.data, records.str.length, need, strlen(need));

    lxb_utils_warc_destroy(warc, true);
    lexbor_mraw_destroy(records.mraw, true);
}
TEST_END

TEST_BEGIN_ARGS(parse_splits, const char *data, const char *need,
                const char *error)
{
    size_t len = strlen(data);

    for (size_t split = 0; split <= len; split++) {
        TEST_CALL_ARGS(parse_chunks, data, split, 0, need, error);
    }

    TEST_CALL_ARGS(parse_chunks, data, 1, 1, need, error);
}
TEST_END

TEST_BEGIN_ARGS(parse_field, size_t name_len, size_t value_len, size_t split,
                lxb_status_t need, const char *error)
{
    size_t len;
    lxb_char_t *data, *p;
    lxb_status_t status;
    lxb_utils_warc_t *warc;
    const lxb_char_t *begin, *end;

    static const char head[] = "WARC/1.0\r\n";
    static const char tail[] = "\r\nContent-Length: 0\r\n\r\n\r\n\r\n";

    len = (sizeof(head) - 1) + name_len + 2 + value_len + (sizeof(tail) - 1);

    data = lexbor_malloc(len);
    test_ne(data, NULL);

    p = data;

    memcpy(p, head, sizeof(head) - 1);
    p += sizeof(head) - 1;

    memset(p, 'n', name_len);
    p += name_len;

    memcpy(p, ": ", 2);
    p += 2;

    memset(p, 'v', value_len);
    p += value_len;

    memcpy(p, tail, sizeof(tail) - 1);

    warc = lxb_utils_warc_create();
    status = lxb_utils_warc_init(warc, NULL, NULL, NULL, NULL);
    test_eq(status, LXB_STATUS_OK);

    begin = data;
    end = data + split;

    status = lxb_utils_warc_parse(warc, &begin, end);
    if (status == LXB_STATUS_OK) {
        end = data + len;
        status = lxb_utils_warc_parse(warc, &begin, end);
    }

    test_eq(status, need);

    if (error != NULL) {
        test_ne(warc->error, NULL);
        test_eq_str(warc->error, error);
    }
    else {
        test_eq(lxb_utils_warc_parse_eof(warc), LXB_STATUS_OK);
    }

    lxb_utils_warc_destroy(warc, true);
    lexbor_free(data);
}
TEST_END

TEST_BEGIN_ARGS(parse_file, const char *head, size_t pad, const char *tail,
                lxb_status_t need, const char *need_records, const char *error)
{
    FILE *fh;
    lxb_status_t status;
    records_t records;
    lxb_utils_warc_t *warc;

    fh = tmpfile();
    test_ne(fh, NULL);

    test_eq(fwrite(head, 1, strlen(head), fh), strlen(head));

    for (size_t i = 0; i < pad; i++) {
        test_eq(fputc('a', fh), 'a');
    }

    test_eq(fwrite(tail, 1, strlen(tail), fh), strlen(tail));
    rewind(fh);

    records.mraw = lexbor_mraw_create();
    test_eq(lexbor_mraw_init(records.mraw, 1024), LXB_STATUS_OK);

    lexbor_str_init(&records.str, records.mraw, 256);
    test_ne(records.str.data, NULL);

    warc = lxb_utils_warc_create();
    status = lxb_utils_warc_init(warc, header_cb, NULL, content_end_cb,
                                 &records);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_utils_warc_parse_file(warc, fh);
    test_eq(status, need);

    if (error != NULL) {
        test_ne(warc->error, NULL);
        test_eq_str(warc->error, error);
    }

    test_eq_str_n(records.str.data, records.str.length,
                  need_records, strlen(need_records));

    lxb_utils_warc_destroy(warc, true);
    lexbor_mraw_destroy(records.mraw, true);
    fclose(fh);
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
                   "[WARC-Type=\"response\"|Content-Length=4|4]ABCD$"
                   "[WARC-Type=metadata|Content-Length=2|2]xy$", NULL);
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
                   "ABCD$", NULL);
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
                   "[X-A=a\"b|X-B=\"a\\|Content-Length=4|4]ABCD$"
                   "[Content-Length=2|2]xy$", NULL);
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

TEST_BEGIN(version)
{
    const char *error = "Wrong warc version.";

    TEST_CALL_ARGS(parse_splits,
                   "WARC/1.0\r\nContent-Length: 2\r\n\r\nxy\r\n\r\n"
                   "warc/1.0\r\nContent-Length: 2\r\n\r\nzw\r\n\r\n",
                   "[Content-Length=2|2]xy$[Content-Length=2|2]zw$", NULL);

    TEST_CALL_ARGS(parse_splits, "WARC/1.0abc\r\n\r\n\r\n\r\n", "", error);
    TEST_CALL_ARGS(parse_splits, "WARC/1.00\r\n\r\n\r\n\r\n", "", error);
    TEST_CALL_ARGS(parse_splits, "WARC/1.x\r\n\r\n\r\n\r\n", "", error);
    TEST_CALL_ARGS(parse_splits, "WARC/1e0\r\n\r\n\r\n\r\n", "", error);
    TEST_CALL_ARGS(parse_splits, "WARC/+1.\r\n\r\n\r\n\r\n", "", error);
    TEST_CALL_ARGS(parse_splits, "WARC/1.1\r\n\r\n\r\n\r\n", "", error);
    TEST_CALL_ARGS(parse_splits, "WARC/1.0\n\r\n\r\n\r\n", "", error);
    TEST_CALL_ARGS(parse_splits, " WARC/1.0\r\n\r\n\r\n\r\n", "", error);
}
TEST_END

TEST_BEGIN(record_end)
{
    TEST_CALL_ARGS(parse_splits,
                   "WARC/1.0\r\n\r\n\r\n\r\n"
                   "WARC/1.0\r\nContent-Length: 1\r\n\r\na\r\n\r\n"
                   "WARC/1.0\r\n\r\n\r\n\r\n",
                   "[0]$[Content-Length=1|1]a$[0]$", NULL);

    TEST_CALL_ARGS(parse_splits,
                   "WARC/1.0\r\nContent-Length: 1\r\n\r\na\r\n\r\n"
                   "WARC/1.0\r\nContent-Length: 2\r\n\r\nbc\r\n\r\n",
                   "[Content-Length=1|1]a$[Content-Length=2|2]bc$", NULL);
}
TEST_END

TEST_BEGIN(file)
{
    TEST_CALL_ARGS(parse_file,
                   "WARC/1.0\r\nContent-Length: 9000\r\n\r\n", 9000,
                   "\r\n\r\nWARC/1.0\r\nContent-Length: 2\r\n\r\nxy\r\n\r\n",
                   LXB_STATUS_OK,
                   "[Content-Length=9000|9000]$[Content-Length=2|2]$", NULL);

    TEST_CALL_ARGS(parse_file,
                   "WARC/1.0\r\nContent-Length: 8154\r\n\r\n", 8154,
                   "\r\n\r\n", LXB_STATUS_OK,
                   "[Content-Length=8154|8154]$", NULL);
}
TEST_END

TEST_BEGIN(file_error)
{
    TEST_CALL_ARGS(parse_file,
                   "WARC/1.0\r\nContent-Length: 9000\r\n\r\n", 100, "",
                   LXB_STATUS_ABORTED, "[Content-Length=9000|9000]",
                   "Unexpected data termination.");

    TEST_CALL_ARGS(parse_file,
                   "WARC/1.0\r\nContent-Length: 9000\r\n\r\n", 8500, "",
                   LXB_STATUS_ABORTED, "[Content-Length=9000|9000]",
                   "Unexpected data termination.");

    TEST_CALL_ARGS(parse_file,
                   "WARC/2.0\r\nContent-Length: 9000\r\n\r\n", 9000,
                   "\r\n\r\n", LXB_STATUS_ABORTED, "",
                   "Wrong warc version.");

    TEST_CALL_ARGS(parse_file,
                   "WARC/1.0\r\nBad Name: a\r\nContent-Length: 9000\r\n\r\n",
                   9000, "\r\n\r\n", LXB_STATUS_ABORTED, "",
                   "Wrong header field name.");
}
TEST_END

TEST_BEGIN(field_name)
{
    const char *error = "Wrong header field name.";

    TEST_CALL_ARGS(parse_splits,
                   "WARC/1.0\r\nContent-Length: 0\r\n: a\r\n\r\n\r\n\r\n",
                   "", error);

    TEST_CALL_ARGS(parse_splits,
                   "WARC/1.0\r\n:\r\nContent-Length: 0\r\n\r\n\r\n\r\n",
                   "", error);

    TEST_CALL_ARGS(parse_splits,
                   "WARC/1.0\r\nX a: b\r\nContent-Length: 0\r\n\r\n\r\n\r\n",
                   "", error);

    TEST_CALL_ARGS(parse_splits,
                   "WARC/1.0\r\nX-A:\r\nContent-Length: 0\r\n\r\n\r\n\r\n",
                   "[X-A=|Content-Length=0|0]$", NULL);
}
TEST_END

TEST_BEGIN(field_name_limit)
{
    size_t max = 4096 * 4;
    const char *error = "Too large header field name.";

    TEST_CALL_ARGS(parse_field, max, 1, 0, LXB_STATUS_OK, NULL);
    TEST_CALL_ARGS(parse_field, max, 1, 4096, LXB_STATUS_OK, NULL);
    TEST_CALL_ARGS(parse_field, max, 1, 10 + max, LXB_STATUS_OK, NULL);

    TEST_CALL_ARGS(parse_field, max + 1, 1, 0, LXB_STATUS_ABORTED, error);
    TEST_CALL_ARGS(parse_field, max + 1, 1, 4096, LXB_STATUS_ABORTED, error);
    TEST_CALL_ARGS(parse_field, max + 1, 1, 10 + max + 1,
                   LXB_STATUS_ABORTED, error);
}
TEST_END

TEST_BEGIN(field_value_limit)
{
    size_t max = 4096 * 32;
    size_t cr = 10 + 1 + 2 + max + 1;
    const char *error = "Too large header field value.";

    TEST_CALL_ARGS(parse_field, 1, max, 0, LXB_STATUS_OK, NULL);
    TEST_CALL_ARGS(parse_field, 1, max, 4096, LXB_STATUS_OK, NULL);
    TEST_CALL_ARGS(parse_field, 1, max, cr - 1, LXB_STATUS_OK, NULL);
    TEST_CALL_ARGS(parse_field, 1, max, cr, LXB_STATUS_OK, NULL);

    TEST_CALL_ARGS(parse_field, 1, max + 1, 0, LXB_STATUS_ABORTED, error);
    TEST_CALL_ARGS(parse_field, 1, max + 1, 4096, LXB_STATUS_ABORTED, error);
    TEST_CALL_ARGS(parse_field, 1, max + 1, cr, LXB_STATUS_ABORTED, error);
    TEST_CALL_ARGS(parse_field, 1, max + 1, cr + 1, LXB_STATUS_ABORTED, error);
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
    TEST_ADD(field_name);
    TEST_ADD(field_name_limit);
    TEST_ADD(field_value_limit);
    TEST_ADD(version);
    TEST_ADD(record_end);
    TEST_ADD(file);
    TEST_ADD(file_error);

    TEST_RUN("lexbor/utils/warc");
    TEST_RELEASE();
}
