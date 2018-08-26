/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include <unit/test.h>

#include <lexbor/tag/tag.h>

#define LXB_HTML_TAG_RES_DATA
#define LXB_HTML_TAG_RES_SHS_DATA
#include <lexbor/html/tag_res.h>


TEST_BEGIN(init)
{
    lxb_tag_heap_t *tag_heap = lxb_tag_heap_create();
    lxb_status_t status = lxb_tag_heap_init(tag_heap, 32, lxb_html_tag_res_data,
                                            lxb_html_tag_res_shs_data, 0);

    test_eq(status, LXB_STATUS_OK);

    lxb_tag_heap_destroy(tag_heap, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lxb_tag_heap_init(NULL, 32, lxb_html_tag_res_data,
                                            lxb_html_tag_res_shs_data, 0);
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_args)
{
    lxb_tag_heap_t tag_heap;
    lxb_status_t status = lxb_tag_heap_init(&tag_heap, 0, lxb_html_tag_res_data,
                                            lxb_html_tag_res_shs_data, 0);
    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lxb_status_t status;
    lxb_tag_heap_t tag_heap;

    status = lxb_tag_heap_init(&tag_heap, 32, lxb_html_tag_res_data,
                               lxb_html_tag_res_shs_data, 0);
    test_eq(status, LXB_STATUS_OK);

    lxb_tag_heap_destroy(&tag_heap, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lxb_tag_data_t *tag_data;
    lxb_tag_heap_t tag_heap;

    test_eq(lxb_tag_heap_init(&tag_heap, 32, lxb_html_tag_res_data,
                              lxb_html_tag_res_shs_data, 0), LXB_STATUS_OK);

    tag_data = lxb_tag_append(&tag_heap, (const lxb_char_t *) "best_tag", 8);
    test_ne(tag_data, NULL);

    lxb_tag_heap_destroy(&tag_heap, false);
}
TEST_END

TEST_BEGIN(data_by_id)
{
    const lxb_tag_data_t *tag_data;
    lxb_tag_heap_t tag_heap;

    test_eq(lxb_tag_heap_init(&tag_heap, 32, lxb_html_tag_res_data,
                              lxb_html_tag_res_shs_data, 0), LXB_STATUS_OK);

    tag_data = lxb_tag_data_by_id(&tag_heap, LXB_TAG_DIV);
    test_ne(tag_data, NULL);

    tag_data = lxb_tag_data_by_id(&tag_heap, LXB_TAG__LAST_ENTRY + 100);
    test_eq(tag_data, NULL);

    lxb_tag_heap_destroy(&tag_heap, false);
}
TEST_END

TEST_BEGIN(data_by_name)
{
    const lxb_tag_data_t *tag_data;
    lxb_tag_heap_t tag_heap;

    test_eq(lxb_tag_heap_init(&tag_heap, 32, lxb_html_tag_res_data,
                              lxb_html_tag_res_shs_data, 0), LXB_STATUS_OK);

    tag_data = lxb_tag_data_by_name(&tag_heap,
                                         (const lxb_char_t *) "div", 3);
    test_ne(tag_data, NULL);

    tag_data = lxb_tag_data_by_name(&tag_heap,
                                         (const lxb_char_t *) "zzz", 3);
    test_eq(tag_data, NULL);

    lxb_tag_heap_destroy(&tag_heap, false);
}
TEST_END

TEST_BEGIN(name_by_id)
{
    size_t len;
    const lxb_char_t *name;
    lxb_tag_heap_t tag_heap;

    test_eq(lxb_tag_heap_init(&tag_heap, 32, lxb_html_tag_res_data,
                              lxb_html_tag_res_shs_data, 0), LXB_STATUS_OK);

    name = lxb_tag_name_by_id(&tag_heap, LXB_TAG_DIV, LXB_NS_HTML, &len);
    test_eq_u_str(name, (const lxb_char_t *) "div");
    test_eq_size(len, 3UL);

    name = lxb_tag_name_by_id(&tag_heap, LXB_TAG__LAST_ENTRY + 10,
                                   LXB_NS_HTML, &len);
    test_eq(name, NULL);
    test_eq_size(len, 0UL);

    name = lxb_tag_name_by_id(&tag_heap, LXB_TAG__LAST_ENTRY,
                                   LXB_NS_HTML, &len);
    test_eq(name, NULL);
    test_eq_size(len, 0UL);

    lxb_tag_heap_destroy(&tag_heap, false);
}
TEST_END

TEST_BEGIN(name_by_id_without_len)
{
    const lxb_char_t *name;
    lxb_tag_heap_t tag_heap;

    test_eq(lxb_tag_heap_init(&tag_heap, 32, lxb_html_tag_res_data,
                              lxb_html_tag_res_shs_data, 0), LXB_STATUS_OK);

    name = lxb_tag_name_by_id(&tag_heap, LXB_TAG_DIV,  LXB_NS_HTML, NULL);
    test_eq_u_str(name, (const lxb_char_t *) "div");

    name = lxb_tag_name_by_id(&tag_heap, LXB_TAG__LAST_ENTRY + 10,
                              LXB_NS_HTML, NULL);
    test_eq(name, NULL);

    name = lxb_tag_name_by_id(&tag_heap, LXB_TAG__LAST_ENTRY,
                                   LXB_NS_HTML, NULL);
    test_eq(name, NULL);

    lxb_tag_heap_destroy(&tag_heap, false);
}
TEST_END

TEST_BEGIN(id_by_name)
{
    lxb_tag_id_t tag_id;
    lxb_tag_heap_t tag_heap;

    test_eq(lxb_tag_heap_init(&tag_heap, 32, lxb_html_tag_res_data,
                              lxb_html_tag_res_shs_data, 0), LXB_STATUS_OK);

    tag_id = lxb_tag_id_by_name(&tag_heap, (const lxb_char_t *) "div", 3);
    test_eq(tag_id, LXB_TAG_DIV);

    tag_id = lxb_tag_id_by_name(&tag_heap, (const lxb_char_t *) "zzz", 3);
    test_eq(tag_id, LXB_TAG__UNDEF);

    lxb_tag_heap_destroy(&tag_heap, false);
}
TEST_END

TEST_BEGIN(append)
{
    int n;
    char tmp[32];
    lxb_tag_id_t tag_id;
    lxb_tag_heap_t tag_heap;
    const lxb_tag_data_t *tag_data;

    test_eq(lxb_tag_heap_init(&tag_heap, 32, lxb_html_tag_res_data,
                              lxb_html_tag_res_shs_data, 0), LXB_STATUS_OK);

    for (size_t i = 0; i < 10000; i++) {
        n = sprintf(tmp, "test-"LEXBOR_FORMAT_Z, i);

        tag_data = lxb_tag_append(&tag_heap,
                                       (const lxb_char_t *) tmp, (size_t) n);

        test_ne(tag_data, NULL);
        test_eq_u_str(tag_data->name, (const lxb_char_t *) tmp);
        test_eq_size(tag_data->name_len, (size_t) n);
    }

    for (size_t i = 0; i < 10000; i++) {
        n = sprintf(tmp, "test-"LEXBOR_FORMAT_Z, i);

        tag_data = lxb_tag_data_by_name(&tag_heap,
                                             (const lxb_char_t *) tmp,
                                             (size_t) n);

        test_ne(tag_data, NULL);
        test_eq_u_str(tag_data->name, (const lxb_char_t *) tmp);
        test_eq_size(tag_data->name_len, (size_t) n);

        tag_id = lxb_tag_id_by_name(&tag_heap,
                                         (const lxb_char_t *) tmp, (size_t) n);
        test_eq_u_int(tag_id, tag_data->tag_id);
    }

    lxb_tag_heap_destroy(&tag_heap, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lxb_tag_heap_t *tag_heap = lxb_tag_heap_create();
    test_eq(lxb_tag_heap_init(tag_heap, 32, lxb_html_tag_res_data,
                              lxb_html_tag_res_shs_data, 0), LXB_STATUS_OK);

    test_eq(lxb_tag_heap_destroy(tag_heap, true), NULL);

    tag_heap = lxb_tag_heap_create();
    test_eq(lxb_tag_heap_init(tag_heap, 32, lxb_html_tag_res_data,
                              lxb_html_tag_res_shs_data, 0), LXB_STATUS_OK);

    test_eq(lxb_tag_heap_destroy(tag_heap, false), tag_heap);
    test_eq(lxb_tag_heap_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lxb_tag_heap_t tag_heap;
    lxb_tag_heap_init(&tag_heap, 32, lxb_html_tag_res_data,
                      lxb_html_tag_res_shs_data, 0);

    test_eq(lxb_tag_heap_destroy(&tag_heap, false), &tag_heap);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(init);
    TEST_ADD(init_null);
    TEST_ADD(init_stack);
    TEST_ADD(clean);
    TEST_ADD(data_by_id);
    TEST_ADD(data_by_name);
    TEST_ADD(name_by_id);
    TEST_ADD(name_by_id_without_len);
    TEST_ADD(id_by_name);
    TEST_ADD(append);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_RUN("lexbor/html/tag");
    TEST_RELEASE();
}
