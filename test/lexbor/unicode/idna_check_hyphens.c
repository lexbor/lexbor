/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/unicode/unicode.h>
#include <lexbor/encoding/encoding.h>


static const lxb_unicode_idna_flag_t hyphens = LXB_UNICODE_IDNA_FLAG_CHECK_HYPHENS;


TEST_BEGIN_ARGS(test_label, const lexbor_str_t *label, bool need)
{
    size_t length;
    lxb_char_t *data;
    lxb_codepoint_t *cps;
    const lxb_char_t *p, *end;

    TEST_PRINTLN("Label: %.*s", (int) label->length,
                 (const char *) label->data);

    p = label->data;
    end = label->data + label->length;
    length = 0;

    while (p < end) {
        (void) lxb_encoding_decode_valid_utf_8_single(&p, end);
        length += 1;
    }

    data = lexbor_malloc(label->length);
    test_ne(data, NULL);

    cps = lexbor_malloc(length * sizeof(lxb_codepoint_t));
    test_ne(cps, NULL);

    memcpy(data, label->data, label->length);

    p = label->data;
    length = 0;

    while (p < end) {
        cps[length++] = lxb_encoding_decode_valid_utf_8_single(&p, end);
    }

    test_eq_bool(lxb_unicode_idna_validity_criteria(data, label->length,
                                                    hyphens), need);
    test_eq_bool(lxb_unicode_idna_validity_criteria_cp(cps, length,
                                                       hyphens), need);

    lexbor_free(cps);
    lexbor_free(data);
}
TEST_END

TEST_BEGIN(check_hyphens)
{
    /* Hyphen only in the fourth position (index 3) is allowed. */
    test_eq_bool(lxb_unicode_idna_validity_criteria(
                     (const lxb_char_t *) "abc-def", 7, hyphens), true);
    test_eq_bool(lxb_unicode_idna_validity_criteria(
                     (const lxb_char_t *) "a-b-c", 5, hyphens), true);
    test_eq_bool(lxb_unicode_idna_validity_criteria(
                     (const lxb_char_t *) "a--bcd", 6, hyphens), true);

    /* Hyphen in both the third and fourth positions (indices 2 and 3). */
    test_eq_bool(lxb_unicode_idna_validity_criteria(
                     (const lxb_char_t *) "ab--cd", 6, hyphens), false);
    test_eq_bool(lxb_unicode_idna_validity_criteria(
                     (const lxb_char_t *) "xn--", 4, hyphens), false);
    test_eq_bool(lxb_unicode_idna_validity_criteria(
                     (const lxb_char_t *) "ab--", 4, hyphens), false);

    /* Leading or trailing hyphen. The trailing check must use the last byte. */
    test_eq_bool(lxb_unicode_idna_validity_criteria(
                     (const lxb_char_t *) "-abc", 4, hyphens), false);
    test_eq_bool(lxb_unicode_idna_validity_criteria(
                     (const lxb_char_t *) "abc-", 4, hyphens), false);
    test_eq_bool(lxb_unicode_idna_validity_criteria(
                     (const lxb_char_t *) "-", 1, hyphens), false);

    test_eq_bool(lxb_unicode_idna_validity_criteria(
                     (const lxb_char_t *) "a-b", 3, hyphens), true);
    test_eq_bool(lxb_unicode_idna_validity_criteria(
                     (const lxb_char_t *) "ab-c", 4, hyphens), true);

    {
        static const lxb_codepoint_t ok[] = {
            0x0061, 0x0062, 0x0063, 0x002D, 0x0064, 0x0065, 0x0066
        };
        static const lxb_codepoint_t both[] = {
            0x0061, 0x0062, 0x002D, 0x002D, 0x0063, 0x0064
        };
        static const lxb_codepoint_t lead[] = {
            0x002D, 0x0061, 0x0062, 0x0063
        };
        static const lxb_codepoint_t tail[] = {
            0x0061, 0x0062, 0x0063, 0x002D
        };

        test_eq_bool(lxb_unicode_idna_validity_criteria_cp(
                         ok, sizeof(ok) / sizeof(ok[0]), hyphens), true);
        test_eq_bool(lxb_unicode_idna_validity_criteria_cp(
                         both, sizeof(both) / sizeof(both[0]), hyphens), false);
        test_eq_bool(lxb_unicode_idna_validity_criteria_cp(
                         lead, sizeof(lead) / sizeof(lead[0]), hyphens), false);
        test_eq_bool(lxb_unicode_idna_validity_criteria_cp(
                         tail, sizeof(tail) / sizeof(tail[0]), hyphens), false);
    }
}
TEST_END

TEST_BEGIN(check_hyphens_utf_8)
{
    size_t i;

    static const lexbor_str_t valid[] = {
        lexbor_str("\xC3\xA9--x"),
        lexbor_str("\xC3\xB1--ab"),
        lexbor_str("\xC3\xA9-\xC3\xB1-x"),
        lexbor_str("\xC3\xA9\xC3\xB1-x"),
        lexbor_str("\xC3\xA9\xC3\xB1x-y"),
        lexbor_str("\xC3\xA9-\xC3\xB1"),
        lexbor_str("\xE6\x97\xA5--x"),
        lexbor_str("\xE6\x97\xA5\xE6\x9C\xAC")
    };

    static const lexbor_str_t invalid[] = {
        lexbor_str("\xC3\xA9" "a--b"),
        lexbor_str("a\xC3\xA9--b"),
        lexbor_str("\xC3\xA9\xC3\xB1--x"),
        lexbor_str("\xE6\x97\xA5\xE6\x9C\xAC--x"),
        lexbor_str("\xC3\xB1-"),
        lexbor_str("-\xC3\xB1"),
        lexbor_str("\xE6\x97\xA5-")
    };

    for (i = 0; i < sizeof(valid) / sizeof(lexbor_str_t); i++) {
        TEST_CALL_ARGS(test_label, &valid[i], true);
    }

    for (i = 0; i < sizeof(invalid) / sizeof(lexbor_str_t); i++) {
        TEST_CALL_ARGS(test_label, &invalid[i], false);
    }
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(check_hyphens);
    TEST_ADD(check_hyphens_utf_8);

    TEST_RUN("lexbor/unicode/idna_check_hyphens");
    TEST_RELEASE();
}
