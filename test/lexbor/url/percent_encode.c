/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>
#include <lexbor/url/url.h>


typedef struct {
    const lexbor_str_t    input;
    const lexbor_str_t    output;
    lxb_encoding_t        encoding;
    lxb_url_map_type_t    enmap;
    bool                  space_as_plus;
    const uint8_t         *url_map;
    const lexbor_str_t    initial;
}
percent_encode_entry_t;


static const uint8_t custom_url_map[256] = {
    ['a'] = LXB_URL_MAP_QUERY,
    ['b'] = LXB_URL_MAP_PATH
};

static const percent_encode_entry_t entries[] = {
    {
        lexbor_str(""),
        lexbor_str(""),
        LXB_ENCODING_UTF_8,
        LXB_URL_MAP_COMPONENT,
        false,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("AZaz09-._~!*'()"),
        lexbor_str("AZaz09-._~!*'()"),
        LXB_ENCODING_UTF_8,
        LXB_URL_MAP_COMPONENT,
        false,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("\x00" "A /?\xC3\xA9"),
        lexbor_str("%00A%20%2F%3F%C3%A9"),
        LXB_ENCODING_UTF_8,
        LXB_URL_MAP_COMPONENT,
        false,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("A b"),
        lexbor_str("prefix:A+b"),
        LXB_ENCODING_UTF_8,
        LXB_URL_MAP_COMPONENT,
        true,
        NULL,
        lexbor_str("prefix:")
    },
    {
        lexbor_str("\xE2\x89\xA1\xE2\x80\xBD"),
        lexbor_str("%E2%89%A1%E2%80%BD"),
        LXB_ENCODING_UTF_8,
        LXB_URL_MAP_SPECIAL_QUERY,
        false,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("\xE2\x89\xA1"),
        lexbor_str("%81%DF"),
        LXB_ENCODING_SHIFT_JIS,
        LXB_URL_MAP_SPECIAL_QUERY,
        false,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("\xE2\x80\xBD"),
        lexbor_str("%26%238253%3B"),
        LXB_ENCODING_SHIFT_JIS,
        LXB_URL_MAP_SPECIAL_QUERY,
        false,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("1+1 \xE2\x89\xA1 2%20\xE2\x80\xBD"),
        lexbor_str("1+1%20%81%DF%202%20%26%238253%3B"),
        LXB_ENCODING_SHIFT_JIS,
        LXB_URL_MAP_SPECIAL_QUERY,
        false,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("\xC2\xA5"),
        lexbor_str("%1B(J\\%1B(B"),
        LXB_ENCODING_ISO_2022_JP,
        LXB_URL_MAP_SPECIAL_QUERY,
        false,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("caf\xC3\xA9"),
        lexbor_str("caf%E9"),
        LXB_ENCODING_WINDOWS_1252,
        LXB_URL_MAP_COMPONENT,
        false,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("\xD0\xAF"),
        lexbor_str("%DF"),
        LXB_ENCODING_WINDOWS_1251,
        LXB_URL_MAP_COMPONENT,
        false,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("\xE4\xB8\xAD\xE6\x96\x87"),
        lexbor_str("%A4%A4%A4%E5"),
        LXB_ENCODING_BIG5,
        LXB_URL_MAP_COMPONENT,
        false,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("a b+c~"),
        lexbor_str("a+b%2Bc%7E"),
        LXB_ENCODING_UTF_8,
        LXB_URL_MAP_X_WWW_FORM,
        true,
        NULL,
        lexbor_str("")
    },
    {
        lexbor_str("abc"),
        lexbor_str("%61bc"),
        LXB_ENCODING_UTF_8,
        LXB_URL_MAP_QUERY,
        false,
        custom_url_map,
        lexbor_str("")
    }
};


TEST_BEGIN(percent_encode)
{
    size_t length;
    lxb_char_t *data;
    lxb_status_t status;
    lexbor_mraw_t mraw;
    lexbor_str_t str;
    const uint8_t *url_map, *default_url_map;
    const lxb_encoding_data_t *encoding;
    const percent_encode_entry_t *entry;

    status = lexbor_mraw_init(&mraw, 1024);
    test_eq(status, LXB_STATUS_OK);

    default_url_map = lxb_url_get_percent_encoding_map();
    test_ne(default_url_map, NULL);

    length = sizeof(entries) / sizeof(percent_encode_entry_t);

    for (size_t i = 0; i < length; i++) {
        entry = &entries[i];
        encoding = lxb_encoding_data(entry->encoding);
        test_ne(encoding, NULL);

        str = (lexbor_str_t) {0};

        if (entry->initial.length != 0) {
            data = lexbor_str_init_append(&str, &mraw, entry->initial.data,
                                          entry->initial.length);
            test_ne(data, NULL);
        }

        url_map = entry->url_map;
        if (url_map == NULL) {
            url_map = default_url_map;
        }

        status = lxb_url_percent_encode_encoding(entry->input.data,
                                                 entry->input.length,
                                                 &str, &mraw, url_map, encoding,
                                                 entry->enmap,
                                                 entry->space_as_plus);
        test_eq(status, LXB_STATUS_OK);

        if (str.length != entry->output.length
            || memcmp(str.data, entry->output.data, str.length) != 0)
        {
            TEST_PRINTLN("Percent-encode entry %zu (%s)", i + 1,
                         encoding->name);
        }

        test_eq_str_n(str.data, str.length, entry->output.data,
                      entry->output.length);

        lexbor_str_destroy(&str, &mraw, false);
    }

    lexbor_mraw_destroy(&mraw, false);
}
TEST_END

int
main(int argc, const char *argv[])
{
    TEST_INIT();

    TEST_ADD(percent_encode);

    TEST_RUN("lexbor/url/percent_encode");
    TEST_RELEASE();
}
