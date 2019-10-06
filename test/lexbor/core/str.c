/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/core/str.h>


TEST_BEGIN_ARGS(test_make_mraw, lexbor_mraw_t **mraw)
{
    *mraw = lexbor_mraw_create();
    test_eq(lexbor_mraw_init(*mraw, 1024), LXB_STATUS_OK);
}
TEST_END

TEST_BEGIN_ARGS(test_destroy_mraw, lexbor_mraw_t **mraw)
{
    *mraw = lexbor_mraw_destroy(*mraw, true);
}
TEST_END

TEST_BEGIN(init)
{
    lexbor_mraw_t *mraw;
    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lexbor_str_t *str = lexbor_str_create();
    lxb_char_t *value = lexbor_str_init(str, mraw, 128);

    test_ne(value, NULL);

    lexbor_str_destroy(str, mraw, true);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(init_null)
{
    lexbor_mraw_t *mraw;
    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(NULL, mraw, 128);
    test_eq(value, NULL);

    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(init_stack)
{
    lexbor_str_t str;
    lexbor_mraw_t *mraw;

    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(&str, mraw, 128);
    test_ne(value, NULL);

    lexbor_str_destroy(&str, mraw, false);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(init_args)
{
    lexbor_str_t str;
    lexbor_mraw_t *mraw;

    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(&str, mraw, 0);
    test_ne(value, NULL);

    lexbor_str_destroy(&str, mraw, false);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(append)
{
    lexbor_str_t str;
    lexbor_mraw_t *mraw;

    const lxb_char_t *cat = (lxb_char_t *) "Cat";
    size_t cat_size = strlen((const char *) cat);

    const lxb_char_t *dog = (lxb_char_t *) "Dog";
    size_t dog_size = strlen((const char *) dog);

    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(&str, mraw, 0);
    test_ne(value, NULL);

    /* Cat */
    lexbor_str_append(&str, mraw, cat, cat_size);
    test_eq_size(str.length, cat_size);
    test_eq_size(lexbor_str_size(&str), lexbor_mem_align(cat_size + 1));

    test_eq_u_str(str.data, (lxb_char_t *) "Cat");
    test_eq(str.data[cat_size], (lxb_char_t) '\0');

    /* Dog */
    size_t total = cat_size + dog_size;
    lexbor_str_append(&str, mraw, dog, dog_size);
    test_eq_size(str.length, total);
    test_eq_size(lexbor_str_size(&str), lexbor_mem_align(total + 1));

    test_eq_u_str(str.data, (lxb_char_t *) "CatDog");
    test_eq(str.data[total], (lxb_char_t) '\0');

    test_eq(value, str.data);

    lexbor_str_destroy(&str, mraw, false);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(append_edge)
{
    lexbor_str_t str, str_fake;
    lexbor_mraw_t *mraw;

    const lxb_char_t *cat = (lxb_char_t *) "Cat";
    size_t cat_size = strlen((const char *) cat);

    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(&str, mraw, 3);
    test_ne(value, NULL);

    test_ne(lexbor_str_init(&str_fake, mraw, 3), NULL);

    /* Cat */
    lexbor_str_append(&str, mraw, cat, cat_size);
    test_eq_size(str.length, cat_size);
    test_eq_size(lexbor_str_size(&str), lexbor_mem_align(cat_size + 1));

    test_eq(value, str.data);

    lexbor_str_destroy(&str, mraw, false);
    lexbor_str_destroy(&str_fake, mraw, false);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(append_reuse)
{
    lexbor_str_t str, str_fake, str_reuse;
    lexbor_mraw_t *mraw;

    const lxb_char_t *cat = (lxb_char_t *) "Cat and Dog and Some Animals";
    size_t cat_size = strlen((const char *) cat);

    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(&str, mraw, 2);
    test_ne(value, NULL);

    test_ne(lexbor_str_init(&str_fake, mraw, 3), NULL);

    /* Cat */
    lexbor_str_append(&str, mraw, cat, cat_size);
    test_eq_size(str.length, cat_size);
    test_eq_size(lexbor_str_size(&str), lexbor_mem_align(cat_size + 1));

    test_ne(value, str.data);

    /* Reuse */
    lxb_char_t *reuse = lexbor_str_init(&str_reuse, mraw, 2);
    test_ne(reuse, NULL);

    test_eq(reuse, value);

    /* Destroy */
    lexbor_str_destroy(&str, mraw, false);
    lexbor_str_destroy(&str_fake, mraw, false);
    lexbor_str_destroy(&str_reuse, mraw, false);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(append_before)
{
    lexbor_str_t str;
    lexbor_mraw_t *mraw;

    const lxb_char_t *cat = (lxb_char_t *) "Cat";
    size_t cat_size = strlen((const char *) cat);

    const lxb_char_t *dog = (lxb_char_t *) "Dog";
    size_t dog_size = strlen((const char *) dog);

    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(&str, mraw, 0);
    test_ne(value, NULL);

    /* Dog */
    lexbor_str_append_before(&str, mraw, dog, dog_size);
    test_eq_size(str.length, dog_size);
    test_eq_size(lexbor_str_size(&str), lexbor_mem_align(dog_size + 1));

    test_eq_u_str(str.data, (lxb_char_t *) "Dog");
    test_eq(str.data[dog_size], (lxb_char_t) '\0');

    /* Cat */
    size_t total = cat_size + dog_size;
    lexbor_str_append_before(&str, mraw, cat, cat_size);
    test_eq_size(str.length, total);
    test_eq_size(lexbor_str_size(&str), lexbor_mem_align(total + 1));

    test_eq_u_str(str.data, (lxb_char_t *) "CatDog");
    test_eq(str.data[total], (lxb_char_t) '\0');

    test_eq(value, str.data);

    lexbor_str_destroy(&str, mraw, false);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(append_with_rep_null_chars)
{
    lexbor_str_t str;
    lexbor_mraw_t *mraw;

    const lxb_char_t *data = (lxb_char_t *) "C\0at";
    size_t data_size = 4;

    const char *exp = "C""\xEF\xBF\xBD""at";
    size_t exp_size = strlen(exp);

    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(&str, mraw, 0);
    test_ne(value, NULL);

    lexbor_str_append_with_rep_null_chars(&str, mraw, data, data_size);
    test_eq_size(str.length, exp_size);
    test_eq_size(lexbor_str_size(&str), lexbor_mem_align(exp_size + 1));

    test_eq_u_str(str.data, (lxb_char_t *) exp);
    test_eq(str.data[exp_size], (lxb_char_t) '\0');

    test_eq(value, str.data);

    lexbor_str_destroy(&str, mraw, false);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(append_with_rep_null_chars_in_begin)
{
    lexbor_str_t str;
    lexbor_mraw_t *mraw;

    const lxb_char_t *data = (lxb_char_t *) "\0Cat";
    size_t data_size = 4;

    const char *exp = "\xEF\xBF\xBD""Cat";
    size_t exp_size = strlen(exp);

    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(&str, mraw, 0);
    test_ne(value, NULL);

    lexbor_str_append_with_rep_null_chars(&str, mraw, data, data_size);
    test_eq_size(str.length, exp_size);
    test_eq_size(lexbor_str_size(&str), lexbor_mem_align(exp_size + 1));

    test_eq_u_str(str.data, (lxb_char_t *) exp);
    test_eq(str.data[exp_size], (lxb_char_t) '\0');

    test_eq(value, str.data);

    lexbor_str_destroy(&str, mraw, false);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(append_with_rep_null_chars_in_end)
{
    lexbor_str_t str;
    lexbor_mraw_t *mraw;

    const lxb_char_t *data = (lxb_char_t *) "Cat\0";
    size_t data_size = 4;

    const char *exp = "Cat\xEF\xBF\xBD";
    size_t exp_size = strlen(exp);

    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(&str, mraw, 0);
    test_ne(value, NULL);

    lexbor_str_append_with_rep_null_chars(&str, mraw, data, data_size);
    test_eq_size(str.length, exp_size);
    test_eq_size(lexbor_str_size(&str), lexbor_mem_align(exp_size + 1));

    test_eq_u_str(str.data, (lxb_char_t *) exp);
    test_eq(str.data[exp_size], (lxb_char_t) '\0');

    test_eq(value, str.data);

    lexbor_str_destroy(&str, mraw, false);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(append_with_rep_null_chars_loneliness)
{
    lexbor_str_t str;
    lexbor_mraw_t *mraw;

    const lxb_char_t *data = (lxb_char_t *) "\0";
    size_t data_size = 1;

    const char *exp = "\xEF\xBF\xBD";
    size_t exp_size = strlen(exp);

    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(&str, mraw, 0);
    test_ne(value, NULL);

    lexbor_str_append_with_rep_null_chars(&str, mraw, data, data_size);
    test_eq_size(str.length, exp_size);
    test_eq_size(lexbor_str_size(&str), lexbor_mem_align(exp_size + 1));

    test_eq_u_str(str.data, (lxb_char_t *) exp);
    test_eq(str.data[exp_size], (lxb_char_t) '\0');

    test_eq(value, str.data);

    lexbor_str_destroy(&str, mraw, false);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(clean)
{
    lexbor_str_t str;
    lexbor_mraw_t *mraw;

    const lxb_char_t *data = (lxb_char_t *) "Novichok";
    size_t data_size = strlen((const char *) data);

    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lxb_char_t *value = lexbor_str_init(&str, mraw, 128);
    test_ne(value, NULL);

    lexbor_str_append(&str, mraw, data, data_size);

    test_eq_size(str.length, data_size);
    lexbor_str_clean(&str);
    test_eq_size(str.length, 0UL);

    lexbor_str_destroy(&str, mraw, false);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(destroy)
{
    lexbor_mraw_t *mraw;
    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lexbor_str_t *str = lexbor_str_create();
    test_ne(lexbor_str_init(str, mraw, 128), NULL);

    test_eq(lexbor_str_destroy(str, mraw, true), NULL);

    str = lexbor_str_create();
    test_ne(lexbor_str_init(str, mraw, 128), NULL);

    test_eq(lexbor_str_destroy(str, mraw, false), str);
    test_eq(lexbor_str_destroy(str, mraw, true), NULL);
    test_eq(lexbor_str_destroy(NULL, mraw, false), NULL);

    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
}
TEST_END

TEST_BEGIN(destroy_stack)
{
    lexbor_mraw_t *mraw;
    TEST_CALL_ARGS(test_make_mraw, &mraw);

    lexbor_str_t str;
    test_ne(lexbor_str_init(&str, mraw, 128), NULL);

    test_eq(lexbor_str_destroy(&str, mraw, false), &str);
    TEST_CALL_ARGS(test_destroy_mraw, &mraw);
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
    TEST_ADD(append);
    TEST_ADD(append_edge);
    TEST_ADD(append_reuse);
    TEST_ADD(append_before);
    TEST_ADD(append_with_rep_null_chars);
    TEST_ADD(append_with_rep_null_chars_in_begin);
    TEST_ADD(append_with_rep_null_chars_in_end);
    TEST_ADD(append_with_rep_null_chars_loneliness);
    TEST_ADD(clean);
    TEST_ADD(destroy);
    TEST_ADD(destroy_stack);

    TEST_RUN("lexbor/core/str");
    TEST_RELEASE();
}
