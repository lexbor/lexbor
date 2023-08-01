/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/unicode/unicode.h>
#include <lexbor/encoding/encoding.h>


TEST_BEGIN(all)
{
    size_t count = 0;
    lxb_codepoint_t cp;

    for (cp = 0x0000; cp <= LXB_ENCODING_MAX_CODEPOINT; cp++) {
        count += (size_t) lxb_unicode_idna_type(cp);
    }

    test_eq(count, 4165205);
}
TEST_END

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

    TEST_ADD(all);
    TEST_ADD(edge);

    TEST_RUN("lexbor/unicode/idna");
    TEST_RELEASE();
}
