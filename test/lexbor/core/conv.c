/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <string.h>

#include <unit/test.h>

#include <lexbor/core/conv.h>


TEST_BEGIN(int64_fits)
{
    size_t n;
    lxb_char_t buf[8];

    memset(buf, 0xAA, sizeof(buf));
    n = lexbor_conv_int64_to_data(5, buf, 1);
    test_eq(n, 1);
    test_eq(buf[0], '5');
    test_eq(buf[1], 0xAA);

    memset(buf, 0xAA, sizeof(buf));
    n = lexbor_conv_int64_to_data(5, buf, 2);
    test_eq(n, 1);
    test_eq(buf[0], '5');
    test_eq(buf[1], 0x00);
    test_eq(buf[2], 0xAA);

    memset(buf, 0xAA, sizeof(buf));
    n = lexbor_conv_int64_to_data(123, buf, sizeof(buf));
    test_eq(n, 3);
    test_eq(buf[0], '1');
    test_eq(buf[1], '2');
    test_eq(buf[2], '3');
    test_eq(buf[3], 0x00);
}
TEST_END

TEST_BEGIN(int64_truncates)
{
    size_t n;
    lxb_char_t buf[8];

    memset(buf, 0xAA, sizeof(buf));
    n = lexbor_conv_int64_to_data(123, buf, 2);
    test_eq(n, 2);
    test_eq(buf[0], '1');
    test_eq(buf[1], '2');
    test_eq(buf[2], 0xAA);
}
TEST_END

TEST_BEGIN(hex_fits)
{
    size_t n;
    lxb_char_t buf[8];

    memset(buf, 0xAA, sizeof(buf));
    n = lexbor_conv_dec_to_hex(0xABC, buf, 3, false);
    test_eq(n, 3);
    test_eq(buf[0], 'a');
    test_eq(buf[1], 'b');
    test_eq(buf[2], 'c');
    test_eq(buf[3], 0xAA);

    memset(buf, 0xAA, sizeof(buf));
    n = lexbor_conv_dec_to_hex(0xABC, buf, 3, true);
    test_eq(n, 3);
    test_eq(buf[0], 'A');
    test_eq(buf[1], 'B');
    test_eq(buf[2], 'C');
}
TEST_END

TEST_BEGIN(hex_capacity)
{
    size_t n;
    lxb_char_t buf[8];

    memset(buf, 0xAA, sizeof(buf));
    n = lexbor_conv_dec_to_hex(0xABC, buf, 1, false);
    test_eq(n, 1);
    test_eq(buf[0], 'a');
    test_eq(buf[1], 0xAA);

    memset(buf, 0xAA, sizeof(buf));
    n = lexbor_conv_dec_to_hex(0xABC, buf, 0, false);
    test_eq(n, 0);
    test_eq(buf[0], 0xAA);

    memset(buf, 0xAA, sizeof(buf));
    n = lexbor_conv_dec_to_hex(0, buf, 1, false);
    test_eq(n, 1);
    test_eq(buf[0], '0');
    test_eq(buf[1], 0xAA);
}
TEST_END

int
main(int argc, const char * argv[])
{
    (void) argc;
    (void) argv;

    TEST_INIT();

    TEST_ADD(int64_fits);
    TEST_ADD(int64_truncates);
    TEST_ADD(hex_fits);
    TEST_ADD(hex_capacity);

    TEST_RUN("lexbor/core/conv");
    TEST_RELEASE();
}
