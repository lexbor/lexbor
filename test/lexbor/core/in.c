/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/in.h>


TEST_BEGIN(init)
{
    lexbor_in_t *incoming = lexbor_in_create();
    lxb_status_t status = lexbor_in_init(incoming, 1024);

    test_eq(status, LXB_STATUS_OK);

    lexbor_in_destroy(incoming, true);
}
TEST_END

TEST_BEGIN(init_null)
{
    lxb_status_t status = lexbor_in_init(NULL, 1024);
    test_eq(status, LXB_STATUS_ERROR_OBJECT_IS_NULL);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_in_t incoming;
    lxb_status_t status = lexbor_in_init(&incoming, 1024);

    test_eq(status, LXB_STATUS_OK);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(init_args)
{
    lexbor_in_t incoming = {0};
    lxb_status_t status = lexbor_in_init(&incoming, 0);

    test_eq(status, LXB_STATUS_ERROR_WRONG_ARGS);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(node_make)
{
    lexbor_in_t incoming;
    lexbor_in_node_t *node;

    lxb_char_t *data = (lxb_char_t *) "some";
    size_t data_len = strlen((const char *) data);

    test_eq(lexbor_in_init(&incoming, 1024), LXB_STATUS_OK);

    node = lexbor_in_node_make(&incoming, NULL, data, data_len);
    test_ne(node, NULL);

    test_eq_size(node->offset, 0UL);

    test_eq(node->begin, data);
    test_eq(node->end, (data + data_len));

    test_eq(node->next, NULL);
    test_eq(node->prev, NULL);

    test_eq(node->incoming, &incoming);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(node_make_arg_null)
{
    lexbor_in_t incoming;
    lexbor_in_node_t *node;

    lxb_char_t *data = (lxb_char_t *) "some";
    size_t data_len = strlen((const char *) data);

    test_eq(lexbor_in_init(&incoming, 1024), LXB_STATUS_OK);

    node = lexbor_in_node_make(&incoming, NULL, NULL, data_len);
    test_ne(node, NULL);

    test_eq_size(node->offset, 0UL);

    test_eq(node->begin, NULL);
    test_ne(node->end, NULL);

    test_eq(node->next, NULL);
    test_eq(node->prev, NULL);

    test_eq(node->incoming, &incoming);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(node_make_arg_null_0)
{
    lexbor_in_t incoming;
    lexbor_in_node_t *node;

    test_eq(lexbor_in_init(&incoming, 1024), LXB_STATUS_OK);

    node = lexbor_in_node_make(&incoming, NULL, NULL, 0);
    test_ne(node, NULL);

    test_eq_size(node->offset, 0UL);

    test_eq(node->begin, NULL);
    test_eq(node->end, NULL);

    test_eq(node->next, NULL);
    test_eq(node->prev, NULL);

    test_eq(node->incoming, &incoming);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(node_make_arg_data_0)
{
    lexbor_in_t incoming;
    lexbor_in_node_t *node;

    lxb_char_t *data = (lxb_char_t *) "some";

    test_eq(lexbor_in_init(&incoming, 1024), LXB_STATUS_OK);

    node = lexbor_in_node_make(&incoming, NULL, data, 0);
    test_ne(node, NULL);

    test_eq_size(node->offset, 0UL);

    test_eq(node->begin, data);
    test_eq(node->end, data);

    test_eq(node->next, NULL);
    test_eq(node->prev, NULL);

    test_eq(node->incoming, &incoming);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(node_clean)
{
    lexbor_in_t incoming;
    lexbor_in_node_t *node;

    lxb_char_t *data = (lxb_char_t *) "some";
    size_t data_len = strlen((const char *) data);

    test_eq(lexbor_in_init(&incoming, 1024), LXB_STATUS_OK);

    node = lexbor_in_node_make(&incoming, NULL, data, data_len);
    test_ne(node, NULL);

    lexbor_in_node_clean(node);

    test_eq_size(node->offset, 0UL);
    test_eq(node->begin, NULL);
    test_eq(node->end, NULL);
    test_eq(node->next, NULL);
    test_eq(node->prev, NULL);

    test_eq(node->incoming, &incoming);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(node_destroy)
{
    lexbor_in_t incoming;
    lexbor_in_node_t *node;

    lxb_char_t *data = (lxb_char_t *) "some";
    size_t data_len = strlen((const char *) data);

    test_eq(lexbor_in_init(&incoming, 1024), LXB_STATUS_OK);

    node = lexbor_in_node_make(&incoming, NULL, data, data_len);
    test_ne(node, NULL);

    test_eq(lexbor_in_node_destroy(&incoming, node, true), NULL);

    node = lexbor_in_node_make(&incoming, NULL, data, data_len);
    test_ne(node, NULL);

    test_eq(lexbor_in_node_destroy(&incoming, node, false), node);
    test_eq(lexbor_in_node_destroy(&incoming, NULL, false), NULL);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(node_split)
{
    lexbor_in_t incoming;
    lexbor_in_node_t *node, *new_node;

    lxb_char_t *data = (lxb_char_t *) "some";
    size_t data_len = strlen((const char *) data);

    test_eq(lexbor_in_init(&incoming, 1024), LXB_STATUS_OK);

    node = lexbor_in_node_make(&incoming, NULL, data, data_len);
    test_ne(node, NULL);

    new_node = lexbor_in_node_split(node, (data + 2UL));

    /* node */
    test_eq_size(node->offset, 0UL);
    test_eq(node->begin, data);
    test_eq(node->end, (data + 2UL));
    test_eq(node->next, new_node);
    test_eq(node->prev, NULL);
    test_eq(node->incoming, &incoming);

    /* new_node */
    test_eq_size(new_node->offset, 2UL);
    test_eq(new_node->begin, (data + 2UL));
    test_eq(new_node->end, (data + data_len));
    test_eq(new_node->next, NULL);
    test_eq(new_node->prev, node);
    test_eq(new_node->incoming, &incoming);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(node_find)
{
    lexbor_in_t incoming;
    lexbor_in_node_t *node, *found_node;

    lxb_char_t *data = (lxb_char_t *) "some";
    size_t data_len = strlen((const char *) data);

    test_eq(lexbor_in_init(&incoming, 1024), LXB_STATUS_OK);

    node = lexbor_in_node_make(&incoming, NULL, data, data_len);
    test_ne(node, NULL);

    node = lexbor_in_node_make(&incoming, node, (const lxb_char_t *) "test", 4);
    test_ne(node, NULL);

    found_node = lexbor_in_node_find(node, (data + 2UL));

    test_eq_size(found_node->offset, 0UL);
    test_eq(found_node->begin, data);
    test_eq(found_node->end, (data + data_len));
    test_ne(found_node->next, NULL);
    test_eq(found_node->prev, NULL);
    test_eq(found_node->incoming, &incoming);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(node_param)
{
    lexbor_in_t incoming;
    lexbor_in_node_t *node;

    lxb_char_t *data = (lxb_char_t *) "some";
    size_t data_len = strlen((const char *) data);

    test_eq(lexbor_in_init(&incoming, 1024), LXB_STATUS_OK);

    node = lexbor_in_node_make(&incoming, NULL, data, data_len);
    test_ne(node, NULL);

    test_eq_size(lexbor_in_node_offset(node), 0UL);
    test_eq(lexbor_in_node_begin(node), data);
    test_eq(lexbor_in_node_end(node), (data + data_len));
    test_eq(lexbor_in_node_next(node), NULL);
    test_eq(lexbor_in_node_prev(node), NULL);
    test_eq(lexbor_in_node_in(node), &incoming);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_in_t incoming;
    lexbor_in_init(&incoming, 1024);

    lexbor_in_clean(&incoming);

    lexbor_in_destroy(&incoming, false);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_in_t *incoming = lexbor_in_create();
    lexbor_in_init(incoming, 1024);

    test_eq(lexbor_in_destroy(incoming, true), NULL);

    incoming = lexbor_in_create();
    lexbor_in_init(incoming, 1021);

    test_eq(lexbor_in_destroy(incoming, false), incoming);
    test_eq(lexbor_in_destroy(incoming, true), NULL);
    test_eq(lexbor_in_destroy(NULL, false), NULL);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_in_t incoming;
    lexbor_in_init(&incoming, 1023);

    test_eq(lexbor_in_destroy(&incoming, false), &incoming);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(init);
    TEST_ADD(init_null);
    TEST_ADD(init_stack);
    TEST_ADD(init_args);
    TEST_ADD(node_make);
    TEST_ADD(node_make_arg_null);
    TEST_ADD(node_make_arg_null_0);
    TEST_ADD(node_make_arg_data_0);
    TEST_ADD(node_split);
    TEST_ADD(node_find);
    TEST_ADD(node_param);
    TEST_ADD(node_clean);
    TEST_ADD(node_destroy);
    TEST_ADD(clean);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_RUN("lexbor/core/in");
    TEST_RELEASE();
}
