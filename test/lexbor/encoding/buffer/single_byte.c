#include <unit/test.h>

#include <lexbor/encoding/encoding.h>


TEST_BEGIN_ARGS(decode_one_codepoint_at_a_time, lxb_encoding_t encoding,
                const lxb_char_t *data, size_t length,
                const lxb_codepoint_t *expected)
{
    lxb_status_t status;
    lxb_codepoint_t cp;
    const lxb_char_t *ref;
    lxb_encoding_decode_t ctx;
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data(encoding);
    test_ne(enc_data, NULL);

    status = lxb_encoding_decode_init(&ctx, enc_data, &cp, 1);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_encoding_decode_replace_set(&ctx,
          LXB_ENCODING_REPLACEMENT_BUFFER, LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    test_eq(status, LXB_STATUS_OK);

    ref = data;

    for (size_t i = 0; i < length; i++) {
        lxb_encoding_decode_buf_set(&ctx, &cp, 1);

        status = enc_data->decode(&ctx, &ref, data + length);
        test_eq(status, i + 1 == length ? LXB_STATUS_OK
                                       : LXB_STATUS_SMALL_BUFFER);
        test_eq(ref, data + i + 1);
        test_eq(lxb_encoding_decode_buf_used(&ctx), 1);
        test_eq(cp, expected[i]);
    }
}
TEST_END

TEST_BEGIN(decode_valid_text_at_buffer_boundary)
{
    /* Windows-1252 "café": 0xE9 is é. A full output buffer must not
     * consume and discard that final character, leaving "caf". */
    const lxb_char_t data[] = {'c', 'a', 'f', 0xE9};
    const lxb_codepoint_t expected[] = {'c', 'a', 'f', 0x00E9};

    TEST_CALL_ARGS(decode_one_codepoint_at_a_time, LXB_ENCODING_WINDOWS_1252,
                   data, sizeof(data), expected);
}
TEST_END

TEST_BEGIN(decode_undefined_bytes_at_buffer_boundary)
{
    /* Both 0xAA bytes are undefined in Windows-1253. Each replacement
     * must advance input exactly once, including the final byte. */
    const lxb_char_t data[] = {0xAA, 0xAA};
    const lxb_codepoint_t expected[] = {
        LXB_ENCODING_REPLACEMENT_CODEPOINT, LXB_ENCODING_REPLACEMENT_CODEPOINT
    };

    TEST_CALL_ARGS(decode_one_codepoint_at_a_time, LXB_ENCODING_WINDOWS_1253,
                   data, sizeof(data), expected);
}
TEST_END

int
main(int argc, const char *argv[])
{
    TEST_INIT();

    TEST_ADD(decode_valid_text_at_buffer_boundary);
    TEST_ADD(decode_undefined_bytes_at_buffer_boundary);

    TEST_RUN("lexbor/encoding/single_byte");
    TEST_RELEASE();
}
