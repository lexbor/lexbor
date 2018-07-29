/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#include <unit/test.h>

#include <lexbor/html/ns.h>


TEST_BEGIN(data_by_id)
{
    const lxb_html_ns_data_t *ns_data;

    ns_data = lxb_html_ns_data_by_id(LXB_HTML_NS_SVG);
    test_ne(ns_data, NULL);
}
TEST_END

TEST_BEGIN(data_by_name)
{
    const lxb_html_ns_data_t *ns_data;

    ns_data = lxb_html_ns_data_by_name((const lxb_char_t *) "svg", 3);
    test_ne(ns_data, NULL);
}
TEST_END

TEST_BEGIN(name_by_id)
{
    size_t len;
    const lxb_char_t *name;

    name = lxb_html_ns_name_by_id(LXB_HTML_NS_SVG, &len);
    test_eq_u_str(name, (const lxb_char_t *) "SVG");
    test_eq_size(len, 3UL);

    name = lxb_html_ns_name_by_id(LXB_HTML_NS__LAST_ENTRY + 10, &len);
    test_eq(name, NULL);
    test_eq_size(len, 0UL);

    name = lxb_html_ns_name_by_id(LXB_HTML_NS__LAST_ENTRY, &len);
    test_eq(name, NULL);
    test_eq_size(len, 0UL);
}
TEST_END

TEST_BEGIN(name_by_id_without_len)
{
    const lxb_char_t *name;

    name = lxb_html_ns_name_by_id(LXB_HTML_NS_SVG, NULL);
    test_eq_u_str(name, (const lxb_char_t *) "SVG");

    name = lxb_html_ns_name_by_id(LXB_HTML_NS__LAST_ENTRY + 10, NULL);
    test_eq(name, NULL);

    name = lxb_html_ns_name_by_id(LXB_HTML_NS__LAST_ENTRY, NULL);
    test_eq(name, NULL);
}
TEST_END

TEST_BEGIN(link_by_id)
{
    size_t len;
    const lxb_char_t *link;

    link = lxb_html_ns_link_by_id(LXB_HTML_NS_SVG, &len);
    test_ne(link, NULL);
    test_ne(len, 0UL);

    link = lxb_html_ns_link_by_id(LXB_HTML_NS__LAST_ENTRY + 10, &len);
    test_eq(link, NULL);
    test_eq_size(len, 0UL);

    link = lxb_html_ns_link_by_id(LXB_HTML_NS__LAST_ENTRY, &len);
    test_eq(link, NULL);
    test_eq_size(len, 0UL);
}
TEST_END

TEST_BEGIN(link_by_id_without_len)
{
    const lxb_char_t *link;

    link = lxb_html_ns_link_by_id(LXB_HTML_NS_SVG, NULL);
    test_ne(link, NULL);

    link = lxb_html_ns_link_by_id(LXB_HTML_NS__LAST_ENTRY + 10, NULL);
    test_eq(link, NULL);

    link = lxb_html_ns_link_by_id(LXB_HTML_NS__LAST_ENTRY, NULL);
    test_eq(link, NULL);
}
TEST_END

TEST_BEGIN(id_by_name)
{
    lxb_html_ns_id_t ns_id;

    ns_id = lxb_html_ns_id_by_name((const lxb_char_t *) "svg", 3);
    test_eq(ns_id, LXB_HTML_NS_SVG);

    ns_id = lxb_html_ns_id_by_name((const lxb_char_t *) "zzz", 3);
    test_eq(ns_id, LXB_HTML_NS__UNDEF);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(data_by_id);
    TEST_ADD(data_by_name);
    TEST_ADD(name_by_id);
    TEST_ADD(name_by_id_without_len);
    TEST_ADD(link_by_id);
    TEST_ADD(link_by_id_without_len);
    TEST_ADD(id_by_name);

    TEST_RUN("lexbor/core/ns");
    TEST_RELEASE();
}
