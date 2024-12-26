/*
 * Copyright (C) 2023-2024 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/unicode/unicode.h>
#include <lexbor/encoding/encoding.h>


TEST_BEGIN(edge)
{
    lxb_unicode_idna_type_t type;

    type = lxb_unicode_idna_type(0xE00FF);
    test_eq(type, LXB_UNICODE_IDNA_DISALLOWED);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(edge);

    TEST_RUN("lexbor/unicode/idna");
    TEST_RELEASE();
}
