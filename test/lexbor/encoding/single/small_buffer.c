/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/encoding/encoding.h>


#define GUARD_BYTE  0xCC
#define BUFFER_SIZE 16


/*
 * Encoded before the tested code point to move stateful encoders
 * (ISO-2022-JP) out of the initial state.
 */
static const lxb_codepoint_t prefixes[] = {
    0x00A5, /* ISO-2022-JP: Roman. */
    0x3042  /* ISO-2022-JP: JIS X 0208. */
};

static const lxb_codepoint_t supplementary[] = {
    0x10000, 0x1F600, 0x20000, 0x2A6D6, 0x2F8A7, 0x10FFFF, 0x110000
};


static bool
guard_intact(const lxb_char_t *begin, const lxb_char_t *end)
{
    while (begin < end) {
        if (*begin++ != GUARD_BYTE) {
            return false;
        }
    }

    return true;
}

/*
 * The output must never go past end: every capacity smaller than
 * the encoded length gives SMALL_BUFFER without moving the pointer or changing
 * the state. Bytes before end may be overwritten; the exact capacity gives
 * the full result.
 */
static bool
check_cp(const lxb_encoding_data_t *enc_data, const lxb_encoding_encode_t *base,
         lxb_codepoint_t cp)
{
    int8_t ref_len, len;
    size_t cap;
    lxb_char_t *p;
    lxb_char_t ref[BUFFER_SIZE], buf[BUFFER_SIZE];
    lxb_encoding_encode_t ctx;

    ctx = *base;
    p = ref;
    ref_len = enc_data->encode_single(&ctx, &p, ref + sizeof(ref), cp);

    if (ref_len < 0) {
        if (p != ref || ctx.state != base->state) {
            return false;
        }

        ctx = *base;
        memset(buf, GUARD_BYTE, sizeof(buf));
        p = buf;

        len = enc_data->encode_single(&ctx, &p, buf, cp);

        return (len == LXB_ENCODING_ENCODE_ERROR
                || len == LXB_ENCODING_ENCODE_SMALL_BUFFER)
               && p == buf && ctx.state == base->state
               && guard_intact(buf, buf + sizeof(buf));
    }

    if (ref_len == 0 || p - ref != ref_len) {
        return false;
    }

    for (cap = 0; cap < (size_t) ref_len; cap++) {
        ctx = *base;
        memset(buf, GUARD_BYTE, sizeof(buf));
        p = buf;

        len = enc_data->encode_single(&ctx, &p, buf + cap, cp);

        if (len != LXB_ENCODING_ENCODE_SMALL_BUFFER || p != buf
            || ctx.state != base->state
            || !guard_intact(buf + cap, buf + sizeof(buf)))
        {
            return false;
        }
    }

    ctx = *base;
    memset(buf, GUARD_BYTE, sizeof(buf));
    p = buf;

    len = enc_data->encode_single(&ctx, &p, buf + ref_len, cp);

    return len == ref_len && p == buf + ref_len
           && memcmp(buf, ref, ref_len) == 0
           && guard_intact(buf + ref_len, buf + sizeof(buf));
}

static bool
check_encoding(const lxb_encoding_data_t *enc_data,
               const lxb_encoding_encode_t *base)
{
    size_t i;
    lxb_codepoint_t cp;

    for (cp = 0; cp <= 0xFFFF; cp++) {
        if (!check_cp(enc_data, base, cp)) {
            TEST_PRINTLN("%s: U+%04X, state %u",
                         (const char *) enc_data->name, (unsigned) cp,
                         base->state);
            return false;
        }
    }

    for (i = 0; i < sizeof(supplementary) / sizeof(supplementary[0]); i++) {
        if (!check_cp(enc_data, base, supplementary[i])) {
            TEST_PRINTLN("%s: U+%04X, state %u",
                         (const char *) enc_data->name,
                         (unsigned) supplementary[i], base->state);
            return false;
        }
    }

    return true;
}

TEST_BEGIN(encode_single_small_buffer)
{
    size_t i;
    int8_t len;
    unsigned init_state;
    lxb_encoding_t id;
    lxb_char_t buf[BUFFER_SIZE], *p;
    lxb_encoding_encode_t base;
    const lxb_encoding_data_t *enc_data;

    for (id = 0; id < LXB_ENCODING_LAST_ENTRY; id++) {
        enc_data = lxb_encoding_data(id);
        test_ne(enc_data, NULL);
        test_ne(enc_data->encode_single, NULL);

        test_eq(lxb_encoding_encode_init_single(&base, enc_data),
                LXB_STATUS_OK);
        test_eq(check_encoding(enc_data, &base), true);

        init_state = base.state;

        for (i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); i++) {
            test_eq(lxb_encoding_encode_init_single(&base, enc_data),
                    LXB_STATUS_OK);

            p = buf;
            len = enc_data->encode_single(&base, &p, buf + sizeof(buf),
                                          prefixes[i]);
            if (len < 0 || base.state == init_state) {
                continue;
            }

            test_eq(check_encoding(enc_data, &base), true);
        }
    }
}
TEST_END

TEST_BEGIN(encode_single_ascii_fill)
{
    size_t i;
    int8_t len;
    lxb_encoding_t id;
    lxb_char_t *buf, *p;
    lxb_encoding_encode_t ctx;
    const lxb_encoding_data_t *enc_data;

    /* Exactly sized heap buffer: ASan catches a write past end. */
    buf = lexbor_malloc(4);
    test_ne(buf, NULL);

    for (id = 0; id < LXB_ENCODING_LAST_ENTRY; id++) {
        if (id == LXB_ENCODING_AUTO || id == LXB_ENCODING_UNDEFINED
            || id == LXB_ENCODING_REPLACEMENT
            || id == LXB_ENCODING_UTF_16BE || id == LXB_ENCODING_UTF_16LE)
        {
            continue;
        }

        enc_data = lxb_encoding_data(id);
        test_ne(enc_data, NULL);

        test_eq(lxb_encoding_encode_init_single(&ctx, enc_data),
                LXB_STATUS_OK);

        p = buf;

        for (i = 0; i < 4; i++) {
            len = enc_data->encode_single(&ctx, &p, buf + 4, 'a' + i);
            test_eq(len, 1);
        }

        len = enc_data->encode_single(&ctx, &p, buf + 4, 'e');
        test_eq(len, LXB_ENCODING_ENCODE_SMALL_BUFFER);
        test_eq(p, buf + 4);
        test_eq(memcmp(buf, "abcd", 4), 0);
    }

    lexbor_free(buf);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(encode_single_small_buffer);
    TEST_ADD(encode_single_ascii_fill);

    TEST_RUN("lexbor/encoding/small_buffer");
    TEST_RELEASE();
}
