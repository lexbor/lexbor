/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>
#include <lexbor/url/url.h>


typedef struct {
    lexbor_str_t query;
    lexbor_str_t result;
    bool         sort;
}
test_params_t;


static const test_params_t test_params[] =
{
    {
        .query = lexbor_str(""),
        .result = lexbor_str(""),
        .sort = false
    },
    {
        .query = lexbor_str("xyz=789&abc=123"),
        .result = lexbor_str("xyz=789&abc=123"),
        .sort = false
    },
    {
        .query = lexbor_str("xyz=789&abc=123"),
        .result = lexbor_str("abc=123&xyz=789"),
        .sort = true
    },
    {
        .query = lexbor_str("?xyz=789&abc=123"),
        .result = lexbor_str("xyz=789&abc=123"),
        .sort = false
    },
    {
        .query = lexbor_str("xyz789&abc=123"),
        .result = lexbor_str("xyz789=&abc=123"),
        .sort = false
    },
    {
        .query = lexbor_str("xyz=&abc=123"),
        .result = lexbor_str("xyz=&abc=123"),
        .sort = false
    },
    {
        .query = lexbor_str("xyz789abc=123"),
        .result = lexbor_str("xyz789abc=123"),
        .sort = false
    },
    {
        .query = lexbor_str("xyz"),
        .result = lexbor_str("xyz="),
        .sort = false
    },
    {
        .query = lexbor_str("xyz&abc"),
        .result = lexbor_str("xyz=&abc="),
        .sort = false
    },
    {
        .query = lexbor_str("xyz&abc"),
        .result = lexbor_str("abc=&xyz="),
        .sort = true
    },
    {
        .query = lexbor_str("xyz=789&123=abc"),
        .result = lexbor_str("123=abc&xyz=789"),
        .sort = true
    },
    {
        .query = lexbor_str("xyz=7+8+9"),
        .result = lexbor_str("xyz=7+8+9"),
        .sort = false
    },
    {
        .query = lexbor_str("xyz=7 8 9"),
        .result = lexbor_str("xyz=7+8+9"),
        .sort = false
    },
    {
        .query = lexbor_str("xyz=спасибо"),
        .result = lexbor_str("xyz=%D1%81%D0%BF%D0%B0%D1%81%D0%B8%D0%B1%D0%BE"),
        .sort = false
    },
    {
        .query = {.data = NULL, .length = 0},
        .result = lexbor_str(""),
        .sort = true
    },
};


static lxb_status_t
serialize_cb(const lxb_char_t *data, size_t len, void *ctx)
{
    lexbor_str_t *str = ctx;

    str->length = len;
    str->data = lexbor_malloc(len + 1);
    if (str->data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    memcpy(str->data, data, len);

    str->data[len] = '\0';

    return LXB_STATUS_OK;
}

TEST_BEGIN(params_init)
{
    lxb_status_t status;
    lexbor_str_t str;
    lexbor_mraw_t *mraw;
    const test_params_t *param;
    lxb_url_search_params_t *sp;

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    for (size_t i = 0; i < sizeof(test_params) / sizeof(test_params_t); i++) {
        param = &test_params[i];

        sp = lxb_url_search_params_init(mraw,
                                        param->query.data, param->query.length);
        test_ne(sp, NULL);

        if (param->sort) {
            lxb_url_search_params_sort(sp);
        }

        str.data = NULL;

        status = lxb_url_search_params_serialize(sp, serialize_cb, &str);
        test_eq(status, LXB_STATUS_OK);

        if (param->result.data == NULL) {
            test_ne(str.data, NULL);
            test_eq_str(str.data, "");
        }
        else {
            test_ne(str.data, NULL);
            test_eq_str(str.data, param->result.data);
        }

        str.data = lexbor_free(str.data);
    }

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_null_init_append)
{
    lxb_status_t status;
    lexbor_str_t str;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;
    lxb_url_search_entry_t *entry;

    static const lexbor_str_t name = lexbor_str("abc");
    static const lexbor_str_t value = lexbor_str("xyz");
    static const lexbor_str_t result = lexbor_str("abc=xyz");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, NULL, 0);
    test_ne(sp, NULL);

    entry = lxb_url_search_params_append(sp, name.data, name.length,
                                         value.data, value.length);
    test_ne(entry, NULL);

    status = lxb_url_search_params_serialize(sp, serialize_cb, &str);
    test_eq(status, LXB_STATUS_OK);

    test_ne(str.data, NULL);
    test_eq_str(str.data, result.data);

    str.data = lexbor_free(str.data);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_append)
{
    lxb_status_t status;
    lexbor_str_t str;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;
    lxb_url_search_entry_t *entry;

    static const lexbor_str_t init = lexbor_str("first=last&new=old");
    static const lexbor_str_t name = lexbor_str("abc");
    static const lexbor_str_t value = lexbor_str("xyz");
    static const lexbor_str_t result = lexbor_str("first=last&new=old&abc=xyz");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, init.data, init.length);
    test_ne(sp, NULL);

    entry = lxb_url_search_params_append(sp, name.data, name.length,
                                         value.data, value.length);
    test_ne(entry, NULL);

    status = lxb_url_search_params_serialize(sp, serialize_cb, &str);
    test_eq(status, LXB_STATUS_OK);

    test_ne(str.data, NULL);
    test_eq_str(str.data, result.data);

    str.data = lexbor_free(str.data);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_delete_name_value)
{
    lxb_status_t status;
    lexbor_str_t str;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;

    static const lexbor_str_t init = lexbor_str("first=last&abc=xyz&new=old");
    static const lexbor_str_t name = lexbor_str("abc");
    static const lexbor_str_t value = lexbor_str("xyz");
    static const lexbor_str_t result = lexbor_str("first=last&new=old");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, init.data, init.length);
    test_ne(sp, NULL);

    lxb_url_search_params_delete(sp, name.data, name.length,
                                 value.data, value.length);

    status = lxb_url_search_params_serialize(sp, serialize_cb, &str);
    test_eq(status, LXB_STATUS_OK);

    test_ne(str.data, NULL);
    test_eq_str(str.data, result.data);

    str.data = lexbor_free(str.data);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_delete_name)
{
    lxb_status_t status;
    lexbor_str_t str;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;

    static const lexbor_str_t init = lexbor_str("first=last&abc=xyz&new=old");
    static const lexbor_str_t name = lexbor_str("abc");
    static const lexbor_str_t result = lexbor_str("first=last&new=old");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, init.data, init.length);
    test_ne(sp, NULL);

    lxb_url_search_params_delete(sp, name.data, name.length, NULL, 0);

    status = lxb_url_search_params_serialize(sp, serialize_cb, &str);
    test_eq(status, LXB_STATUS_OK);

    test_ne(str.data, NULL);
    test_eq_str(str.data, result.data);

    str.data = lexbor_free(str.data);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_delete_name_duplicates)
{
    lxb_status_t status;
    lexbor_str_t str;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;

    static const lexbor_str_t init = lexbor_str("abc=123&first=last&abc=xyz&new=old");
    static const lexbor_str_t name = lexbor_str("abc");
    static const lexbor_str_t result = lexbor_str("first=last&new=old");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, init.data, init.length);
    test_ne(sp, NULL);

    lxb_url_search_params_delete(sp, name.data, name.length, NULL, 0);

    status = lxb_url_search_params_serialize(sp, serialize_cb, &str);
    test_eq(status, LXB_STATUS_OK);

    test_ne(str.data, NULL);
    test_eq_str(str.data, result.data);

    str.data = lexbor_free(str.data);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_delete_name_value_duplicates)
{
    lxb_status_t status;
    lexbor_str_t str;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;

    static const lexbor_str_t init = lexbor_str("abc=xyz&first=last&abc=xyz&new=old");
    static const lexbor_str_t name = lexbor_str("abc");
    static const lexbor_str_t value = lexbor_str("xyz");
    static const lexbor_str_t result = lexbor_str("first=last&new=old");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, init.data, init.length);
    test_ne(sp, NULL);

    lxb_url_search_params_delete(sp, name.data, name.length,
                                 value.data, value.length);

    status = lxb_url_search_params_serialize(sp, serialize_cb, &str);
    test_eq(status, LXB_STATUS_OK);

    test_ne(str.data, NULL);
    test_eq_str(str.data, result.data);

    str.data = lexbor_free(str.data);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_get)
{
    lxb_status_t status;
    lexbor_str_t *value;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;

    static const lexbor_str_t init = lexbor_str("first=last&abc=xyz&new=old&abc=123");
    static const lexbor_str_t name = lexbor_str("abc");
    static const lexbor_str_t result = lexbor_str("xyz");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, init.data, init.length);
    test_ne(sp, NULL);

    value = lxb_url_search_params_get(sp, name.data, name.length);
    test_ne(value, NULL);

    test_eq_str(value->data, result.data);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_init_null_get)
{
    lxb_status_t status;
    lexbor_str_t *value;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;

    static const lexbor_str_t name = lexbor_str("abc");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, NULL, 0);
    test_ne(sp, NULL);

    value = lxb_url_search_params_get(sp, name.data, name.length);
    test_eq(value, NULL);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_has_only_name)
{
    bool have;
    lxb_status_t status;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;

    static const lexbor_str_t init = lexbor_str("first=last&abc=xyz&new=old");
    static const lexbor_str_t name = lexbor_str("abc");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, init.data, init.length);
    test_ne(sp, NULL);

    have = lxb_url_search_params_has(sp, name.data, name.length, NULL, 0);
    test_eq(have, true);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_has_name_value)
{
    bool have;
    lxb_status_t status;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;

    static const lexbor_str_t init = lexbor_str("abc=123&first=last&abc=xyz&new=old");
    static const lexbor_str_t name = lexbor_str("abc");
    static const lexbor_str_t value = lexbor_str("xyz");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, init.data, init.length);
    test_ne(sp, NULL);

    have = lxb_url_search_params_has(sp, name.data, name.length,
                                     value.data, value.length);
    test_eq(have, true);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_has_name_value_ne)
{
    bool have;
    lxb_status_t status;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;

    static const lexbor_str_t init = lexbor_str("abc=123&first=last&abc=890&new=old");
    static const lexbor_str_t name = lexbor_str("abc");
    static const lexbor_str_t value = lexbor_str("xyz");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, init.data, init.length);
    test_ne(sp, NULL);

    have = lxb_url_search_params_has(sp, name.data, name.length,
                                     value.data, value.length);
    test_eq(have, false);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_null_init_has_only_name)
{
    bool have;
    lxb_status_t status;
    lexbor_mraw_t *mraw;
    lxb_url_search_params_t *sp;

    static const lexbor_str_t name = lexbor_str("abc");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, NULL, 0);
    test_ne(sp, NULL);

    have = lxb_url_search_params_has(sp, name.data, name.length, NULL, 0);
    test_eq(have, false);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_set)
{
    lxb_status_t status;
    lexbor_mraw_t *mraw;
    lexbor_str_t str;
    lxb_url_search_params_t *sp;
    lxb_url_search_entry_t *entry;

    static const lexbor_str_t init = lexbor_str("abc=123&first=last&abc=890&new=old&abc=all");
    static const lexbor_str_t name = lexbor_str("abc");
    static const lexbor_str_t value = lexbor_str("xyz");
    static const lexbor_str_t result = lexbor_str("abc=xyz&first=last&new=old");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, init.data, init.length);
    test_ne(sp, NULL);

    test_eq(sp->length, 5);

    entry = lxb_url_search_params_set(sp, name.data, name.length,
                                      value.data, value.length);
    test_ne(entry, NULL);

    test_eq(sp->length, 3);

    status = lxb_url_search_params_serialize(sp, serialize_cb, &str);
    test_eq(status, LXB_STATUS_OK);

    test_ne(str.data, NULL);
    test_eq_str(str.data, result.data);

    str.data = lexbor_free(str.data);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

TEST_BEGIN(params_set_without)
{
    lxb_status_t status;
    lexbor_mraw_t *mraw;
    lexbor_str_t str;
    lxb_url_search_params_t *sp;
    lxb_url_search_entry_t *entry;

    static const lexbor_str_t init = lexbor_str("first=last&new=old");
    static const lexbor_str_t name = lexbor_str("abc");
    static const lexbor_str_t value = lexbor_str("xyz");
    static const lexbor_str_t result = lexbor_str("first=last&new=old&abc=xyz");

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096);
    test_eq(status, LXB_STATUS_OK);

    sp = lxb_url_search_params_init(mraw, init.data, init.length);
    test_ne(sp, NULL);

    test_eq(sp->length, 2);

    entry = lxb_url_search_params_set(sp, name.data, name.length,
                                      value.data, value.length);
    test_ne(entry, NULL);

    test_eq(sp->length, 3);

    status = lxb_url_search_params_serialize(sp, serialize_cb, &str);
    test_eq(status, LXB_STATUS_OK);

    test_ne(str.data, NULL);
    test_eq_str(str.data, result.data);

    str.data = lexbor_free(str.data);

    lexbor_mraw_destroy(mraw, true);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(params_init);
    TEST_ADD(params_append);
    TEST_ADD(params_null_init_append);
    TEST_ADD(params_delete_name_value);
    TEST_ADD(params_delete_name);
    TEST_ADD(params_delete_name_duplicates);
    TEST_ADD(params_delete_name_value_duplicates);
    TEST_ADD(params_get);
    TEST_ADD(params_init_null_get);
    TEST_ADD(params_has_only_name);
    TEST_ADD(params_has_name_value);
    TEST_ADD(params_has_name_value_ne);
    TEST_ADD(params_null_init_has_only_name);
    TEST_ADD(params_set);
    TEST_ADD(params_set_without);

    TEST_RUN("lexbor/url/search_params");
    TEST_RELEASE();
}
