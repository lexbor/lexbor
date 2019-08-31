/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/ns/ns.h>


TEST_BEGIN(data_by_id)
{
    const lxb_ns_data_t *ns_data;
    lxb_ns_heap_t ns_heap = {0};

    test_eq(lxb_ns_heap_init(&ns_heap, 32), LXB_STATUS_OK);

    ns_data = lxb_ns_data_by_id(&ns_heap, LXB_NS_SVG);
    test_ne(ns_data, NULL);
}
TEST_END

TEST_BEGIN(data_by_link)
{
    const lxb_ns_data_t *ns_data;
    lxb_ns_heap_t ns_heap = {0};

    test_eq(lxb_ns_heap_init(&ns_heap, 32), LXB_STATUS_OK);

    ns_data = lxb_ns_data_by_link(&ns_heap,
                                  (const lxb_char_t *) "http://www.w3.org/1998/Math/MathML",
                                  sizeof("http://www.w3.org/1998/Math/MathML") - 1);

    test_ne(ns_data, NULL);
    test_eq_u_str((const lxb_char_t *) "http://www.w3.org/1998/Math/MathML",
                  ns_data->link);
}
TEST_END

TEST_BEGIN(name_by_id)
{
    size_t len;
    const lxb_char_t *name;
    lxb_ns_heap_t ns_heap = {0};

    test_eq(lxb_ns_heap_init(&ns_heap, 32), LXB_STATUS_OK);

    name = lxb_ns_name_by_id(&ns_heap, LXB_NS_SVG, &len);
    test_eq_u_str(name, (const lxb_char_t *) "SVG");
    test_eq_size(len, 3UL);

    name = lxb_ns_name_by_id(&ns_heap, LXB_NS__LAST_ENTRY + 10, &len);
    test_eq(name, NULL);
    test_eq_size(len, 0UL);

    name = lxb_ns_name_by_id(&ns_heap, LXB_NS__LAST_ENTRY, &len);
    test_eq(name, NULL);
    test_eq_size(len, 0UL);
}
TEST_END

TEST_BEGIN(name_by_id_without_len)
{
    const lxb_char_t *name;
    lxb_ns_heap_t ns_heap = {0};

    test_eq(lxb_ns_heap_init(&ns_heap, 32), LXB_STATUS_OK);

    name = lxb_ns_name_by_id(&ns_heap, LXB_NS_SVG, NULL);
    test_eq_u_str(name, (const lxb_char_t *) "SVG");

    name = lxb_ns_name_by_id(&ns_heap, LXB_NS__LAST_ENTRY + 10, NULL);
    test_eq(name, NULL);

    name = lxb_ns_name_by_id(&ns_heap, LXB_NS__LAST_ENTRY, NULL);
    test_eq(name, NULL);
}
TEST_END

TEST_BEGIN(link_by_id)
{
    size_t len;
    const lxb_char_t *link;
    lxb_ns_heap_t ns_heap = {0};

    test_eq(lxb_ns_heap_init(&ns_heap, 32), LXB_STATUS_OK);

    link = lxb_ns_link_by_id(&ns_heap, LXB_NS_SVG, &len);
    test_ne(link, NULL);
    test_ne(len, 0UL);

    link = lxb_ns_link_by_id(&ns_heap, LXB_NS__LAST_ENTRY + 10, &len);
    test_eq(link, NULL);
    test_eq_size(len, 0UL);

    link = lxb_ns_link_by_id(&ns_heap, LXB_NS__LAST_ENTRY, &len);
    test_eq(link, NULL);
    test_eq_size(len, 0UL);
}
TEST_END

TEST_BEGIN(link_by_id_without_len)
{
    const lxb_char_t *link;
    lxb_ns_heap_t ns_heap = {0};

    test_eq(lxb_ns_heap_init(&ns_heap, 32), LXB_STATUS_OK);

    link = lxb_ns_link_by_id(&ns_heap, LXB_NS_SVG, NULL);
    test_ne(link, NULL);

    link = lxb_ns_link_by_id(&ns_heap, LXB_NS__LAST_ENTRY + 10, NULL);
    test_eq(link, NULL);

    link = lxb_ns_link_by_id(&ns_heap, LXB_NS__LAST_ENTRY, NULL);
    test_eq(link, NULL);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(data_by_id);
    TEST_ADD(data_by_link);
    TEST_ADD(name_by_id);
    TEST_ADD(name_by_id_without_len);
    TEST_ADD(link_by_id);
    TEST_ADD(link_by_id_without_len);

    TEST_RUN("lexbor/html/ns");
    TEST_RELEASE();
}
