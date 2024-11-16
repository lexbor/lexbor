/*
 * Copyright (C) 2024 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */


#include <unit/test.h>
#include <lexbor/core/str.h>
#include <lexbor/encoding/encoding.h>


typedef struct {
    lexbor_str_t utf8;
    lexbor_str_t utf8_result;

    lexbor_str_t utf16be;
    lexbor_str_t utf16be_result;

    lexbor_str_t utf16le;
    lexbor_str_t utf16le_result;
}
bom_entry_t;


TEST_BEGIN(skip_bom)
{
    bom_entry_t *entry;
    bom_entry_t boms[] =
    {
        {
            .utf8           = lexbor_str("\xEF\xBB\xBFЯΩ"),
            .utf8_result    = lexbor_str("ЯΩ"),
            .utf16be        = lexbor_str("\xFE\xFFЯΩ"),
            .utf16be_result = lexbor_str("ЯΩ"),
            .utf16le        = lexbor_str("\xFF\xFEЯΩ"),
            .utf16le_result = lexbor_str("ЯΩ"),
        },
        {
            .utf8           = lexbor_str("\xEF\xBFЯΩ"),
            .utf8_result    = lexbor_str("\xEF\xBFЯΩ"),
            .utf16be        = lexbor_str("\xFEЯΩ"),
            .utf16be_result = lexbor_str("\xFEЯΩ"),
            .utf16le        = lexbor_str("\xFFЯΩ"),
            .utf16le_result = lexbor_str("\xFFЯΩ"),
        },
        {
            .utf8           = lexbor_str("\xEFЯΩ"),
            .utf8_result    = lexbor_str("\xEFЯΩ"),
            .utf16be        = lexbor_str("ЯΩ"),
            .utf16be_result = lexbor_str("ЯΩ"),
            .utf16le        = lexbor_str("ЯΩ"),
            .utf16le_result = lexbor_str("ЯΩ"),
        },
        {
            .utf8           = lexbor_str("ЯΩ"),
            .utf8_result    = lexbor_str("ЯΩ"),
            .utf16be        = lexbor_str(""),
            .utf16be_result = lexbor_str(""),
            .utf16le        = lexbor_str(""),
            .utf16le_result = lexbor_str(""),
        }
    };
    size_t length = sizeof(boms) / sizeof(bom_entry_t);

    for (size_t i = 0; i < length; i++) {
        entry = &boms[i];

        lxb_encoding_utf_8_skip_bom((const lxb_char_t **) &entry->utf8.data,
                                    &entry->utf8.length);
        test_eq_str_n(entry->utf8.data, entry->utf8.length,
                      entry->utf8_result.data, entry->utf8_result.length);

        lxb_encoding_utf_16be_skip_bom((const lxb_char_t **) &entry->utf16be.data,
                                       &entry->utf16be.length);
        test_eq_str_n(entry->utf16be.data, entry->utf16be.length,
                      entry->utf16be_result.data, entry->utf16be_result.length);

        lxb_encoding_utf_16le_skip_bom((const lxb_char_t **) &entry->utf16le.data,
                                       &entry->utf16le.length);
        test_eq_str_n(entry->utf16le.data, entry->utf16le.length,
                      entry->utf16le_result.data, entry->utf16le_result.length);
    }
}
TEST_END


int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(skip_bom);

    TEST_RUN("lexbor/encoding/encoding");
    TEST_RELEASE();
}
