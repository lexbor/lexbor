/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/unicode/unicode.h>


static const lxb_unicode_idna_flag_t hyphens = LXB_UNICODE_IDNA_FLAG_CHECK_HYPHENS;


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

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(check_hyphens);

    TEST_RUN("lexbor/unicode/idna_check_hyphens");
    TEST_RELEASE();
}
