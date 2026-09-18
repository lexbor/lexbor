/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/encoding/encoding.h>


TEST_BEGIN(decode_single_empty)
{
    lxb_encoding_t id;
    lxb_codepoint_t cp;
    lxb_encoding_decode_t ctx;
    const lxb_char_t dummy = 0x41;
    const lxb_char_t *p;
    const lxb_encoding_data_t *enc_data;

    for (id = 0; id < LXB_ENCODING_LAST_ENTRY; id++) {
        enc_data = lxb_encoding_data(id);
        test_ne(enc_data, NULL);
        test_ne(enc_data->decode_single, NULL);

        test_eq(lxb_encoding_decode_init_single(&ctx, enc_data),
                LXB_STATUS_OK);

        p = &dummy;
        cp = enc_data->decode_single(&ctx, &p, p);

        if (id == LXB_ENCODING_AUTO || id == LXB_ENCODING_UNDEFINED
            || id == LXB_ENCODING_REPLACEMENT)
        {
            test_eq(cp, LXB_ENCODING_DECODE_ERROR);
        }
        else {
            test_eq(cp, LXB_ENCODING_DECODE_CONTINUE);
        }

        test_eq(p, &dummy);
    }
}
TEST_END

TEST_BEGIN(valid_utf_8_empty)
{
    lxb_codepoint_t cp;
    const lxb_char_t dummy = 0x41;
    const lxb_char_t *p = &dummy;

    cp = lxb_encoding_decode_valid_utf_8_single(&p, p);
    test_eq(cp, LXB_ENCODING_DECODE_ERROR);
    test_eq(p, &dummy);
}
TEST_END

int
main(int argc, const char * argv[])
{
    (void) argc;
    (void) argv;

    TEST_INIT();

    TEST_ADD(decode_single_empty);
    TEST_ADD(valid_utf_8_empty);

    TEST_RUN("lexbor/encoding/empty_slice");
    TEST_RELEASE();
}
