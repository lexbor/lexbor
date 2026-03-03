/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/encoding.h>
#include <unit/test.h>


static lxb_status_t
prescan_check(const lxb_char_t *data, size_t data_len,
              const char *expect_name, size_t expect_len)
{
    size_t out_length;
    const lxb_char_t *name;
    lxb_html_encoding_t em;

    if (lxb_html_encoding_init(&em) != LXB_STATUS_OK) {
        return LXB_STATUS_ERROR;
    }

    name = lxb_html_encoding_prescan(&em, data, data + data_len, &out_length);

    if (expect_name == NULL) {
        if (name != NULL) {
            lxb_html_encoding_destroy(&em, false);
            return LXB_STATUS_ERROR;
        }
    }
    else {
        if (name == NULL) {
            lxb_html_encoding_destroy(&em, false);
            return LXB_STATUS_ERROR;
        }

        if (out_length != expect_len
            || memcmp(name, expect_name, expect_len) != 0)
        {
            lxb_html_encoding_destroy(&em, false);
            return LXB_STATUS_ERROR;
        }
    }

    lxb_html_encoding_destroy(&em, false);

    return LXB_STATUS_OK;
}

static lxb_status_t
prescan_check_str(const char *data, const char *expect_name)
{
    size_t expect_len = (expect_name != NULL) ? strlen(expect_name) : 0;
    return prescan_check((const lxb_char_t *) data, strlen(data),
                         expect_name, expect_len);
}

/* UTF-16LE XML declaration: <\0?\0x\0 */
TEST_BEGIN(utf16le_xml_declaration)
{
    lxb_status_t status;
    static const lxb_char_t data[] = {0x3C, 0x00, 0x3F, 0x00, 0x78, 0x00};

    status = prescan_check(data, sizeof(data), "UTF-16LE", 8);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* UTF-16BE XML declaration: \0<\0?\0x */
TEST_BEGIN(utf16be_xml_declaration)
{
    lxb_status_t status;
    static const lxb_char_t data[] = {0x00, 0x3C, 0x00, 0x3F, 0x00, 0x78};

    status = prescan_check(data, sizeof(data), "UTF-16BE", 8);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* UTF-16LE with extra data after the 6 bytes. */
TEST_BEGIN(utf16le_xml_declaration_extra)
{
    lxb_status_t status;
    static const lxb_char_t data[] = {0x3C, 0x00, 0x3F, 0x00, 0x78, 0x00,
                                      0x6D, 0x00, 0x6C, 0x00};

    status = prescan_check(data, sizeof(data), "UTF-16LE", 8);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* UTF-16BE with extra data after the 6 bytes. */
TEST_BEGIN(utf16be_xml_declaration_extra)
{
    lxb_status_t status;
    static const lxb_char_t data[] = {0x00, 0x3C, 0x00, 0x3F, 0x00, 0x78,
                                      0x00, 0x6D, 0x00, 0x6C};

    status = prescan_check(data, sizeof(data), "UTF-16BE", 8);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Data shorter than 6 bytes: UTF-16 check is skipped, no meta => NULL. */
TEST_BEGIN(short_data_no_encoding)
{
    lxb_status_t status;

    status = prescan_check((const lxb_char_t *) "abc", 3, NULL, 0);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check((const lxb_char_t *) "<", 1, NULL, 0);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check((const lxb_char_t *) "", 0, NULL, 0);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* 5-byte data starting like UTF-16LE but too short. */
TEST_BEGIN(short_data_almost_utf16le)
{
    lxb_status_t status;
    static const lxb_char_t data[] = {0x3C, 0x00, 0x3F, 0x00, 0x78};

    status = prescan_check(data, sizeof(data), NULL, 0);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* 5-byte data starting like UTF-16BE but too short. */
TEST_BEGIN(short_data_almost_utf16be)
{
    lxb_status_t status;
    static const lxb_char_t data[] = {0x00, 0x3C, 0x00, 0x3F, 0x00};

    status = prescan_check(data, sizeof(data), NULL, 0);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* 6 bytes that look like neither UTF-16LE nor UTF-16BE. */
TEST_BEGIN(six_bytes_no_utf16)
{
    lxb_status_t status;
    static const lxb_char_t data[] = {0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x20};

    status = prescan_check(data, sizeof(data), NULL, 0);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* No meta tag found -> NULL. */
TEST_BEGIN(no_meta_tag)
{
    lxb_status_t status;

    status = prescan_check_str("<html><head><title>Test</title></head></html>",
                               NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<div>content</div>", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Meta without charset or content -> no encoding found. */
TEST_BEGIN(meta_no_charset)
{
    lxb_status_t status;

    status = prescan_check_str("<meta name=\"viewport\" content=\"width\">",
                               NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta http-equiv=\"content-type\">", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Meta with content but no http-equiv="content-type" -> need_pragma fails. */
TEST_BEGIN(meta_content_no_http_equiv)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta content=\"text/html; charset=utf-8\">", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Meta http-equiv with content but no charset in content -> no encoding. */
TEST_BEGIN(meta_http_equiv_no_charset_in_content)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"text/html\">", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Simple meta charset -> returns as-is (no alias match). */
TEST_BEGIN(meta_charset_simple)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset='utf-8'>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset=\"utf-8\">", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Meta charset with spaces around '='. */
TEST_BEGIN(meta_charset_spaces)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset =utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset = utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset   =   utf-8   >", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset = 'utf-8'>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Meta charset=windows-1251 (no alias match, returned as-is). */
TEST_BEGIN(meta_charset_windows_1251)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=windows-1251>", "windows-1251");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* http-equiv="content-type" + content="...;charset=..." */
TEST_BEGIN(meta_http_equiv_content_type)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">",
        "utf-8");
    test_eq(status, LXB_STATUS_OK);

    /* Reversed attribute order. */
    status = prescan_check_str(
        "<meta content=\"text/html; charset=utf-8\" http-equiv=\"content-type\">",
        "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* charset in content with quotes. */
TEST_BEGIN(meta_content_charset_quoted)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"text/html; charset='utf-8'\">",
        "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alias: utf-16le -> UTF-8. */
TEST_BEGIN(alias_utf_16le)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=utf-16le>", "UTF-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset=UTF-16LE>", "UTF-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset=\"utf-16le\">", "UTF-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alias: utf-16be -> UTF-8. */
TEST_BEGIN(alias_utf_16be)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=utf-16be>", "UTF-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset=UTF-16BE>", "UTF-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alias: utf-16 -> UTF-8. */
TEST_BEGIN(alias_utf_16)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=utf-16>", "UTF-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alias: unicode -> UTF-8. */
TEST_BEGIN(alias_unicode)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=unicode>", "UTF-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alias: unicodefeff -> UTF-8. */
TEST_BEGIN(alias_unicodefeff)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=unicodefeff>", "UTF-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alias: unicodefffe -> UTF-8. */
TEST_BEGIN(alias_unicodefffe)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=unicodefffe>", "UTF-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alias: csunicode -> UTF-8. */
TEST_BEGIN(alias_csunicode)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=csunicode>", "UTF-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alias: iso-10646-ucs-2 -> UTF-8. */
TEST_BEGIN(alias_iso_10646_ucs_2)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=iso-10646-ucs-2>", "UTF-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alias: ucs-2 -> UTF-8. */
TEST_BEGIN(alias_ucs_2)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=ucs-2>", "UTF-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alias: x-user-defined -> windows-1252. */
TEST_BEGIN(alias_x_user_defined)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=x-user-defined>",
                               "windows-1252");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset=X-USER-DEFINED>",
                               "windows-1252");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Encoding not in alias table -> returned as-is from the input data. */
TEST_BEGIN(no_alias_match)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=iso-8859-1>", "iso-8859-1");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset=koi8-r>", "koi8-r");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset=shift_jis>", "shift_jis");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset=euc-jp>", "euc-jp");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Both charset and http-equiv: charset takes precedence (need_pragma=0x01). */
TEST_BEGIN(charset_and_http_equiv)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str(
        "<meta charset=utf-8 http-equiv=\"content-type\">", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Unclosed quoted value in charset -> content attribute has no value. */
TEST_BEGIN(meta_content_unclosed_quote)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"text/html; charset='utf-8\">",
        NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"text/html; charset='utf-8   \">",
        NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Wrong http-equiv value -> no encoding (need_pragma check fails). */
TEST_BEGIN(meta_http_equiv_wrong_value)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-typ\" content=\"text/html; charset=utf-8\">",
        NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str(
        "<meta http-equiv=\"refresh\" content=\"text/html; charset=utf-8\">",
        NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* HTML before meta (comment, other tags). */
TEST_BEGIN(html_before_meta)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<html>\n <meta http-equiv=\"content-type\" charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str(
        "<!-- comment --><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str(
        "<?xml version=\"1.0\"?><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* End tag with attributes skipped. */
TEST_BEGIN(end_tag_skipped)
{
    lxb_status_t status;

    status = prescan_check_str(
        "</html lala='><meta charset=cp1251>'>\n"
        " <meta http-equiv=\"content-type\" charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("</div><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Non-meta tags with attributes are skipped. */
TEST_BEGIN(non_meta_tags_skipped)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<div id=\"test\" class=\"foo\"><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str(
        "<link rel=\"stylesheet\" href=\"style.css\"><meta charset=utf-8>",
        "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Meta with extra attributes (non-charset, non-content, non-http-equiv). */
TEST_BEGIN(meta_extra_attributes)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta charset=\"windows-1251\" name=\"viewport\" content=\"width\">",
        "windows-1251");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str(
        "<meta bu charset=\"windows-1251\" be name=\"viewport\" bu content=\"width\" be>",
        "windows-1251");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Comment before meta. */
TEST_BEGIN(comment_before_meta)
{
    lxb_status_t status;

    status = prescan_check_str("<!-- --><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    /* Multi-line comment. */
    status = prescan_check_str(
        "<!-- multi\nline\ncomment --><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    /* Comment with > inside (not ending with --). */
    status = prescan_check_str(
        "<!-- foo > bar --><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Bogus comment <!...> (not starting with <!--). */
TEST_BEGIN(bogus_comment_before_meta)
{
    lxb_status_t status;

    status = prescan_check_str("<!DOCTYPE html><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<!foo bar><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Processing instruction <?...> before meta. */
TEST_BEGIN(processing_instruction_before_meta)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<?xml version=\"1.0\"?><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Non-alpha character after '<' -> ignored. */
TEST_BEGIN(non_alpha_after_lt)
{
    lxb_status_t status;

    status = prescan_check_str("<1><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("< ><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* End tag with non-alpha after </ -> skip to >. */
TEST_BEGIN(end_tag_non_alpha)
{
    lxb_status_t status;

    status = prescan_check_str("</1><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* End tag too short (less than 3 bytes after </X). */
TEST_BEGIN(end_tag_short)
{
    lxb_status_t status;

    status = prescan_check_str("</a", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Data truncated before meta can be found. */
TEST_BEGIN(truncated_data)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=utf-", NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta char", NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta", NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Only '<' at end of data. */
TEST_BEGIN(lone_lt_at_end)
{
    lxb_status_t status;

    status = prescan_check_str("hello<", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* <!-X (not a comment) -> treated as bogus, skip to >. */
TEST_BEGIN(not_a_comment_excl)
{
    lxb_status_t status;

    status = prescan_check_str("<!-x><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* <! with less than 5 bytes remaining -> return (end of data). */
TEST_BEGIN(excl_short_data)
{
    lxb_status_t status;

    status = prescan_check_str("<!abc", NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<!ab", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Tag starting with alpha but not "meta" -> skip attributes. */
TEST_BEGIN(non_meta_alpha_tag)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<div class=\"x\"><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str(
        "<span><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* "meta" followed by a non-space/non-slash -> treated as different tag name. */
TEST_BEGIN(meta_like_tag_name)
{
    lxb_status_t status;

    /* "metadata" != "meta", skip_attributes path. */
    status = prescan_check_str(
        "<metadata charset=utf-8><meta charset=koi8-r>", "koi8-r");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* "meta" followed by separator characters: tab, LF, FF, CR, space, '/'. */
TEST_BEGIN(meta_separators)
{
    lxb_status_t status;

    /* Tab */
    status = prescan_check_str("<meta\tcharset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    /* LF */
    status = prescan_check_str("<meta\ncharset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    /* FF */
    status = prescan_check_str("<meta\x0C" "charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    /* CR */
    status = prescan_check_str("<meta\rcharset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    /* Slash */
    status = prescan_check_str("<meta/charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alpha tag too short to be "meta" (< 6 bytes after data ptr). */
TEST_BEGIN(alpha_tag_short_data)
{
    lxb_status_t status;

    status = prescan_check_str("<abcde", NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<abcd", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Tag name ending with '>': skip attributes immediately. */
TEST_BEGIN(tag_ends_at_gt)
{
    lxb_status_t status;

    status = prescan_check_str("<div><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Empty data. */
TEST_BEGIN(empty_data)
{
    lxb_status_t status;

    status = prescan_check((const lxb_char_t *) "", 0, NULL, 0);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Multiple meta tags: prescan returns the first one found. */
TEST_BEGIN(multiple_meta_first)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta charset=windows-1251><meta charset=utf-8>", "windows-1251");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Alias via http-equiv content: "charset=utf-16le" in content -> UTF-8. */
TEST_BEGIN(alias_via_content)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-16le\">",
        "UTF-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"text/html; charset=x-user-defined\">",
        "windows-1252");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Duplicate attribute: second charset is ignored. */
TEST_BEGIN(duplicate_attribute_ignored)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta charset=windows-1251 charset=utf-8>", "windows-1251");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Attribute name shorter than 7 chars -> skipped (not charset/content/http-equiv). */
TEST_BEGIN(short_attribute_name_skipped)
{
    lxb_status_t status;

    status = prescan_check_str("<meta foo=bar charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta id=x charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Attribute without value -> skipped. */
TEST_BEGIN(attribute_no_value)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Case insensitivity of meta tag name. */
TEST_BEGIN(meta_case_insensitive)
{
    lxb_status_t status;

    status = prescan_check_str("<META charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<Meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<mEtA charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Case insensitivity of attribute names. */
TEST_BEGIN(attribute_case_insensitive)
{
    lxb_status_t status;

    status = prescan_check_str("<meta CHARSET=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str(
        "<meta HTTP-EQUIV=\"content-type\" CONTENT=\"text/html; charset=utf-8\">",
        "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* No closing '>' on meta tag: attribute value not terminated. */
TEST_BEGIN(meta_no_closing_gt)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=utf-8", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Content attribute with charset keyword but no '=' after it -> loop again. */
TEST_BEGIN(content_charset_no_equals)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"charset utf-8\">",
        NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Content attribute: charset= at end (no value after =). */
TEST_BEGIN(content_charset_eq_at_end)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"charset=\">", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Content attribute: charset=value terminated by ';'. */
TEST_BEGIN(content_charset_semicolon)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" "
        "content=\"text/html; charset=utf-8; boundary=something\">",
        "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Content attribute: charset=value terminated by space. */
TEST_BEGIN(content_charset_space_terminated)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" "
        "content=\"text/html; charset=utf-8 extra\">",
        "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/*
 * Stress / adversarial tests below.
 */

/* NUL bytes inside data -- should not crash, NUL is just a regular byte. */
TEST_BEGIN(nul_bytes_in_data)
{
    lxb_status_t status;

    /* NUL between < and meta -- '<' found, next byte is 0x00 (non-alpha),
       break in default. Then no more '<' found by memchr. No encoding. */
    static const lxb_char_t d1[] = "<\x00meta charset=utf-8>";
    status = prescan_check(d1, sizeof(d1) - 1, NULL, 0);
    test_eq(status, LXB_STATUS_OK);

    /* NUL inside charset value: "utf" + NUL + "8" is a 5-byte value. */
    static const lxb_char_t d2[] = "<meta charset=\"utf\x00" "8\">";
    status = prescan_check(d2, sizeof(d2) - 1, "utf\x00" "8", 5);
    test_eq(status, LXB_STATUS_OK);

    /* NUL inside attribute name -- no match for "charset". */
    static const lxb_char_t d3[] = "<meta char\x00set=utf-8>";
    status = prescan_check(d3, sizeof(d3) - 1, NULL, 0);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* UTF-16LE pattern followed by a valid meta -- UTF-16LE wins (early return). */
TEST_BEGIN(utf16_wins_over_meta)
{
    lxb_status_t status;
    static const lxb_char_t data[] = {
        0x3C, 0x00, 0x3F, 0x00, 0x78, 0x00,  /* UTF-16LE signature */
        '<', 'm', 'e', 't', 'a', ' ',
        'c', 'h', 'a', 'r', 's', 'e', 't', '=',
        'u', 't', 'f', '-', '8', '>'
    };

    status = prescan_check(data, sizeof(data), "UTF-16LE", 8);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* UTF-16 partial match: first 5 bytes match LE, 6th doesn't. */
TEST_BEGIN(utf16le_partial_mismatch)
{
    lxb_status_t status;
    /* 0x3C 0x00 0x3F 0x00 0x78 0x01 -- last byte wrong. */
    static const lxb_char_t data[] = {0x3C, 0x00, 0x3F, 0x00, 0x78, 0x01};

    status = prescan_check(data, sizeof(data), NULL, 0);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* UTF-16BE partial match: first 5 bytes match, 6th doesn't. */
TEST_BEGIN(utf16be_partial_mismatch)
{
    lxb_status_t status;
    static const lxb_char_t data[] = {0x00, 0x3C, 0x00, 0x3F, 0x00, 0x79};

    status = prescan_check(data, sizeof(data), NULL, 0);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Many nested comments before meta. */
TEST_BEGIN(many_comments)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<!-- a --><!-- b --><!-- c --><!-- d --><!-- e -->"
        "<meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Comment that never closes -- data ends inside comment. */
TEST_BEGIN(comment_never_closes)
{
    lxb_status_t status;

    status = prescan_check_str("<!-- this comment never ends", NULL);
    test_eq(status, LXB_STATUS_OK);

    /* Comment with > but no -- before it, then data ends. */
    status = prescan_check_str("<!-- has > but no dash dash end", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Comment ending: --> vs -> vs --->. Boundary for data[-3]=='-' && data[-2]=='-'. */
TEST_BEGIN(comment_end_boundary)
{
    lxb_status_t status;

    /* Minimal comment: <!---->  (data[-3]='-', data[-2]='-'  after '>'). */
    status = prescan_check_str("<!----><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    /* <!----> is closed, but what about <!--->? Accesses data[-3].
       After <!---> : the '>' at position, data[-2]='-', data[-3]='-'?
       Let's see: "<!--->": !,  then -, then -, then -, then >.
       tag_end finds '>' -> data points after '>'.
       data[-3]='-', data[-2]='-' -> yes, breaks. */
    status = prescan_check_str("<!---><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    /* Extra dashes: <!-----> */
    status = prescan_check_str("<!-----><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Processing instruction without closing '>' -- data ends. */
TEST_BEGIN(pi_never_closes)
{
    lxb_status_t status;

    status = prescan_check_str("<?xml version", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Multiple '<' characters without valid tags. */
TEST_BEGIN(multiple_bare_lt)
{
    lxb_status_t status;

    status = prescan_check_str("<<<<<<<<<meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);

    /* '<' at the very end after other '<' chars. */
    status = prescan_check_str("<<<<<", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Tag that is exactly "meta" but data ends right after separator. */
TEST_BEGIN(meta_ends_after_separator)
{
    lxb_status_t status;

    /* "<meta " -- 6 bytes, space is separator, but no attributes follow. */
    status = prescan_check_str("<meta ", NULL);
    test_eq(status, LXB_STATUS_OK);

    /* "<meta\t" */
    status = prescan_check_str("<meta\t", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Attribute value = '>' -- stops attribute parsing. */
TEST_BEGIN(attr_value_is_gt)
{
    lxb_status_t status;

    /* charset=  then > as value start -- get_attribute returns at '>'. */
    status = prescan_check_str("<meta charset=>", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Attribute with empty quoted value -> zero-length encoding name returned. */
TEST_BEGIN(attr_empty_quoted_value)
{
    lxb_status_t status;

    /* Empty value "" -> charset found but with length 0.
       prescan returns non-NULL with out_length=0. */
    status = prescan_check_str("<meta charset=\"\">", "");
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset=''>", "");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Attribute quoted value starts at end of data -- open quote, then EOF. */
TEST_BEGIN(attr_quote_then_eof)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=\"", NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset='", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Spaces-only attribute value area, then EOF. */
TEST_BEGIN(attr_eq_spaces_eof)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=   ", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Attribute name ends with '/' (treated like '>'). */
TEST_BEGIN(attr_name_slash)
{
    lxb_status_t status;

    /* <meta charset/> -- name is "charset", hits '/' -> return without value.
       Then '>' is next -> end. No value -> no encoding. */
    status = prescan_check_str("<meta charset/>", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Attribute name runs to end of data (no '=', no space, no '>'). */
TEST_BEGIN(attr_name_to_eof)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charsetxxxxxxxxx", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* content attr: "charset" appears but not enough bytes for full keyword. */
TEST_BEGIN(content_charset_too_short)
{
    lxb_status_t status;

    /* content value is only 6 chars, "charse" -- loop can't find "charset". */
    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"charse\">", NULL);
    test_eq(status, LXB_STATUS_OK);

    /* Exactly 7 chars "charset" but value_end == data+7, so (data+7)<end fails. */
    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"charset\">", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* content attr: "charset" keyword repeats -- first without '=', second with. */
TEST_BEGIN(content_charset_repeat)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" "
        "content=\"charset nope; charset=utf-8\">", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* content attr: charset= then quote then quote mismatch (value has ' but opened with "). */
TEST_BEGIN(content_charset_quote_in_value)
{
    lxb_status_t status;

    /* charset=utf-8' -- unquoted, but encounters ' -> returns NULL. */
    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" "
        "content=\"text/html; charset=utf-8'\">", NULL);
    test_eq(status, LXB_STATUS_OK);

    /* charset=utf-8" -- unquoted, but encounters " -> returns NULL. */
    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" "
        "content='text/html; charset=utf-8\"'>", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* content attr: charset= then only spaces till end. */
TEST_BEGIN(content_charset_eq_spaces_end)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"charset=   \">", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* content attr: charset=<value> runs to the end of content without delimiter. */
TEST_BEGIN(content_charset_value_to_end)
{
    lxb_status_t status;

    /* Unquoted value "utf-8" runs to closing quote of content attr. The
       lxb_html_encoding_content sees (data+7)<end is false eventually,
       value is never terminated by ';' or space. data reaches end, then
       data != name check passes -> returns value. */
    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"charset=utf-8\">",
        "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* content attr: charset='value' (quoted inside content string). */
TEST_BEGIN(content_charset_double_quoted)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" "
        "content=\"text/html; charset='koi8-r'\">",
        "koi8-r");
    test_eq(status, LXB_STATUS_OK);

    /* Double-quote inside single-quoted outer. */
    status = prescan_check_str(
        "<meta http-equiv='content-type' "
        "content='text/html; charset=\"koi8-r\"'>",
        "koi8-r");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Very long tag name that starts with "meta" but has many chars after. */
TEST_BEGIN(long_non_meta_tag)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<metaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA>"
        "<meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* End tag </meta ...> -- goes through '/' path, then skip_attributes. */
TEST_BEGIN(end_tag_meta)
{
    lxb_status_t status;

    /* </meta> should NOT be treated as a meta tag. It goes to skip_attributes. */
    status = prescan_check_str("</meta charset=utf-8><meta charset=koi8-r>",
                               "koi8-r");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* End tag: </UPPERCASE> */
TEST_BEGIN(end_tag_uppercase)
{
    lxb_status_t status;

    status = prescan_check_str("</DIV><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Attribute with only '=' and nothing else (edge in get_attribute). */
TEST_BEGIN(bare_equals_attr)
{
    lxb_status_t status;

    /* <meta = charset=utf-8>  -- '=' starts attr name. In name_state, '=' is
       found immediately -> name_end = data, goto value_state. name is "=" (len 0).
       value_state gets " charset=utf-8>" -> value is "charset=utf-8".
       The meta sees name len=0 < 7 -> skip. Next get_attribute hits '>' -> done.
       No encoding found. */
    status = prescan_check_str("<meta = charset=utf-8>", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Lots of whitespace variations in attribute parsing. */
TEST_BEGIN(attr_whitespace_chaos)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta \t\n\r\x0C charset \t\n\r\x0C = \t\n\r\x0C utf-8 \t\n\r\x0C >",
        "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* <! followed by exactly -- but no > at all -- comment loop runs to end. */
TEST_BEGIN(comment_dashes_no_gt)
{
    lxb_status_t status;

    status = prescan_check_str("<!--", NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<!---", NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<!-- no close", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* End tag </ then EOF (less than 3 bytes remain). */
TEST_BEGIN(end_tag_slash_eof)
{
    lxb_status_t status;

    status = prescan_check_str("</", NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("</x", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* End tag </  then non-alpha, no '>' -> skip to '>'. But no '>' -> end. */
TEST_BEGIN(end_tag_nonalpha_no_gt)
{
    lxb_status_t status;

    status = prescan_check_str("</123", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Attribute: name space = space value, but next attr is name=value with no space. */
TEST_BEGIN(attr_parse_tight)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=content-type content=charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* content with multiple "charset" substrings before the real one. */
TEST_BEGIN(content_many_charset_substrings)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" "
        "content=\"charsetX charsetY charset=utf-8\">",
        "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Attribute value: unquoted, ends at '>' after some chars. */
TEST_BEGIN(attr_unquoted_value_gt)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=utf-8>rest", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* High bytes (0x80+) in tag names and values -- not alpha, not special. */
TEST_BEGIN(high_bytes)
{
    lxb_status_t status;

    /* <\xFF> -- non-alpha, non-special after '<' -> break, loop continues. */
    static const lxb_char_t d1[] = "<\xFF><meta charset=utf-8>";
    status = prescan_check(d1, sizeof(d1) - 1, "utf-8", 5);
    test_eq(status, LXB_STATUS_OK);

    /* charset value with high bytes. */
    static const lxb_char_t d2[] = "<meta charset=\xE4\xB8\xAD\xE6\x96\x87>";
    status = prescan_check(d2, sizeof(d2) - 1,
                           (const char *) "\xE4\xB8\xAD\xE6\x96\x87", 6);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Only spaces between tags. */
TEST_BEGIN(only_spaces_between)
{
    lxb_status_t status;

    status = prescan_check_str(
        "   \t\n\r   <meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Tag that looks like meta but 5th char exactly on boundary ('>' vs alpha). */
TEST_BEGIN(meta_fifth_char_boundary)
{
    lxb_status_t status;

    /* "<meta>" -- data+=4 puts at '>', switch(*data++) reads '>' (0x3E) which
       is not a separator -> default -> skip_attributes. data is now past '>',
       pointing at '<' of second meta. skip_name reads through "<meta " (doesn't
       stop at '<' 0x3C), stops at space before "charset". Then get_attribute
       parses "charset=utf-8>" but this is the skip_attributes path for a
       non-meta tag, so the result is discarded. No encoding found. */
    status = prescan_check_str("<meta><meta charset=utf-8>", NULL);
    test_eq(status, LXB_STATUS_OK);

    /* <meta/> -- slash is separator -> enters meta attribute parsing. */
    status = prescan_check_str("<meta/><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* content: have_content is already true -> second content attr ignored. */
TEST_BEGIN(duplicate_content_ignored)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" "
        "content=\"text/html; charset=utf-8\" "
        "content=\"text/html; charset=koi8-r\">",
        "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* content + charset: both pushed to result. prescan returns entry[0] (content's). */
TEST_BEGIN(content_then_charset)
{
    lxb_status_t status;

    /* http-equiv -> got_pragma. content pushes "koi8-r" (need_pragma=0x02).
       charset pushes "utf-8" (need_pragma=0x01). After loop, need_pragma=0x01,
       nothing popped. Result has [0]="koi8-r", [1]="utf-8".
       prescan takes entry[0]. */
    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" "
        "content=\"text/html; charset=koi8-r\" charset=utf-8>",
        "koi8-r");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* All data is just whitespace -- no tags. */
TEST_BEGIN(all_whitespace)
{
    lxb_status_t status;

    status = prescan_check_str("     \t\n\r\x0C     ", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Just ">" with no preceding "<". */
TEST_BEGIN(gt_without_lt)
{
    lxb_status_t status;

    status = prescan_check_str(">>>><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Tag name reaches end without space or '>'. */
TEST_BEGIN(skip_name_reaches_end)
{
    lxb_status_t status;

    /* <div... -- skip_name runs to end, data >= end -> return OK. */
    status = prescan_check_str("<divclassname", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* content attr: charset= with empty string after '=' (immediate ';'). */
TEST_BEGIN(content_charset_eq_immediate_semicolon)
{
    lxb_status_t status;

    /* charset=; -- after '=', skip_spaces -> ';'. Not quote. name = data (';').
       for loop: *data is ';' -> goto done, *name_end = data. name==name_end,
       zero-length entry. need_pragma=0x02, got_pragma=true -> entry stays.
       prescan returns non-NULL with out_length=0. */
    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"charset=;\">", "");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* content attr: charset= then immediate space. */
TEST_BEGIN(content_charset_eq_immediate_space)
{
    lxb_status_t status;

    /* After charset=, skip_spaces skips the space. Then what's next?
       Actually skip_spaces would skip ' ' and land on next non-space.
       "charset= " -- after '=', skip_spaces on ' ': skips it, reaches '"' (outer quote end).
       But wait, the content value is between outer quotes: content="charset= "
       So value is "charset= ", value_end points after the space, before closing ".
       Actually the content value is: charset= <space>
       After charset=, skip_spaces: data+1 = space, skip it, now at end of value -> return NULL. */
    status = prescan_check_str(
        "<meta http-equiv=\"content-type\" content=\"charset= \">", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Bogus: <!> -- data[1] is '>', handled as bogus comment, tag_end finds '>'. */
TEST_BEGIN(bogus_excl_immediate_gt)
{
    lxb_status_t status;

    /* <!> -- data = '!', (data+5) vs end: full string is 22 chars, not short.
       data[1]='>' != '-' -> bogus path, tag_end finds '>' immediately.
       Then continues to <meta charset=utf-8>. */
    status = prescan_check_str("<!><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* charset= where '=' comes right at the end of data stream. */
TEST_BEGIN(attr_eq_at_eof)
{
    lxb_status_t status;

    status = prescan_check_str("<meta charset=", NULL);
    test_eq(status, LXB_STATUS_OK);

    status = prescan_check_str("<meta charset =", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Non-meta tag skip_attributes: attribute with unclosed quote -> value=NULL,
   runs to end, then loop reads next attribute etc. */
TEST_BEGIN(non_meta_unclosed_attr_quote)
{
    lxb_status_t status;

    status = prescan_check_str(
        "<div class=\"unclosed><meta charset=utf-8>", NULL);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* '<' immediately followed by '!' then '-' then '-' then '>' (5 bytes: <!-->) */
TEST_BEGIN(minimal_comment_5bytes)
{
    lxb_status_t status;

    /* "<!-->rest<meta charset=utf-8>" -- full string is 28 chars, not short.
       data[1]='-', data[2]='-' -> comment path. tag_end finds '>' at "<!-->"
       position. data[-3]='-', data[-2]='-' -> comment closed. Then continues
       and finds <meta charset=utf-8>. */
    status = prescan_check_str("<!-->rest<meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

/* Exactly "<!-->" with 6+ bytes. */
TEST_BEGIN(comment_immediate_gt_6plus)
{
    lxb_status_t status;

    /* "<!-->" + enough chars: data = '!', (data+5) > end? '!' is at idx 1,
       data+5 = 6. Total string "<!--><meta charset=utf-8>" = 25.
       So (data+5) > end is false. data[1]='-', data[2]='-' -> it's a comment.
       Enter while loop: tag_end finds first '>' at index 4.
       data after tag_end = index 5. Check data[-3]='!' data[-2]='-' -> '-' yes,
       data[-3]='!' not '-' -> no break.  Hmm wait: data points to char after '>'.
       data[-1]='>', data[-2]='-', data[-3]='-'. Yes! Both '-'. -> break.
       Wait: "<!-->" indices: 0='<', 1='!', 2='-', 3='-', 4='>'.
       tag_end returns 5 (data+1 after '>'). data[-3] = data[5-3] = data[2] = '-'.
       data[-2] = data[3] = '-'. Both '-' -> break. OK so comment closes. */
    status = prescan_check_str("<!--><meta charset=utf-8>", "utf-8");
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(utf16le_xml_declaration);
    TEST_ADD(utf16be_xml_declaration);
    TEST_ADD(utf16le_xml_declaration_extra);
    TEST_ADD(utf16be_xml_declaration_extra);
    TEST_ADD(short_data_no_encoding);
    TEST_ADD(short_data_almost_utf16le);
    TEST_ADD(short_data_almost_utf16be);
    TEST_ADD(six_bytes_no_utf16);
    TEST_ADD(no_meta_tag);
    TEST_ADD(meta_no_charset);
    TEST_ADD(meta_content_no_http_equiv);
    TEST_ADD(meta_http_equiv_no_charset_in_content);
    TEST_ADD(meta_charset_simple);
    TEST_ADD(meta_charset_spaces);
    TEST_ADD(meta_charset_windows_1251);
    TEST_ADD(meta_http_equiv_content_type);
    TEST_ADD(meta_content_charset_quoted);
    TEST_ADD(alias_utf_16le);
    TEST_ADD(alias_utf_16be);
    TEST_ADD(alias_utf_16);
    TEST_ADD(alias_unicode);
    TEST_ADD(alias_unicodefeff);
    TEST_ADD(alias_unicodefffe);
    TEST_ADD(alias_csunicode);
    TEST_ADD(alias_iso_10646_ucs_2);
    TEST_ADD(alias_ucs_2);
    TEST_ADD(alias_x_user_defined);
    TEST_ADD(no_alias_match);
    TEST_ADD(charset_and_http_equiv);
    TEST_ADD(meta_content_unclosed_quote);
    TEST_ADD(meta_http_equiv_wrong_value);
    TEST_ADD(html_before_meta);
    TEST_ADD(end_tag_skipped);
    TEST_ADD(non_meta_tags_skipped);
    TEST_ADD(meta_extra_attributes);
    TEST_ADD(comment_before_meta);
    TEST_ADD(bogus_comment_before_meta);
    TEST_ADD(processing_instruction_before_meta);
    TEST_ADD(non_alpha_after_lt);
    TEST_ADD(end_tag_non_alpha);
    TEST_ADD(end_tag_short);
    TEST_ADD(truncated_data);
    TEST_ADD(lone_lt_at_end);
    TEST_ADD(not_a_comment_excl);
    TEST_ADD(excl_short_data);
    TEST_ADD(non_meta_alpha_tag);
    TEST_ADD(meta_like_tag_name);
    TEST_ADD(meta_separators);
    TEST_ADD(alpha_tag_short_data);
    TEST_ADD(tag_ends_at_gt);
    TEST_ADD(empty_data);
    TEST_ADD(multiple_meta_first);
    TEST_ADD(alias_via_content);
    TEST_ADD(duplicate_attribute_ignored);
    TEST_ADD(short_attribute_name_skipped);
    TEST_ADD(attribute_no_value);
    TEST_ADD(meta_case_insensitive);
    TEST_ADD(attribute_case_insensitive);
    TEST_ADD(meta_no_closing_gt);
    TEST_ADD(content_charset_no_equals);
    TEST_ADD(content_charset_eq_at_end);
    TEST_ADD(content_charset_semicolon);
    TEST_ADD(content_charset_space_terminated);

    TEST_ADD(nul_bytes_in_data);
    TEST_ADD(utf16_wins_over_meta);
    TEST_ADD(utf16le_partial_mismatch);
    TEST_ADD(utf16be_partial_mismatch);
    TEST_ADD(many_comments);
    TEST_ADD(comment_never_closes);
    TEST_ADD(comment_end_boundary);
    TEST_ADD(pi_never_closes);
    TEST_ADD(multiple_bare_lt);
    TEST_ADD(meta_ends_after_separator);
    TEST_ADD(attr_value_is_gt);
    TEST_ADD(attr_empty_quoted_value);
    TEST_ADD(attr_quote_then_eof);
    TEST_ADD(attr_eq_spaces_eof);
    TEST_ADD(attr_name_slash);
    TEST_ADD(attr_name_to_eof);
    TEST_ADD(content_charset_too_short);
    TEST_ADD(content_charset_repeat);
    TEST_ADD(content_charset_quote_in_value);
    TEST_ADD(content_charset_eq_spaces_end);
    TEST_ADD(content_charset_value_to_end);
    TEST_ADD(content_charset_double_quoted);
    TEST_ADD(long_non_meta_tag);
    TEST_ADD(end_tag_meta);
    TEST_ADD(end_tag_uppercase);
    TEST_ADD(bare_equals_attr);
    TEST_ADD(attr_whitespace_chaos);
    TEST_ADD(comment_dashes_no_gt);
    TEST_ADD(end_tag_slash_eof);
    TEST_ADD(end_tag_nonalpha_no_gt);
    TEST_ADD(attr_parse_tight);
    TEST_ADD(content_many_charset_substrings);
    TEST_ADD(attr_unquoted_value_gt);
    TEST_ADD(high_bytes);
    TEST_ADD(only_spaces_between);
    TEST_ADD(meta_fifth_char_boundary);
    TEST_ADD(duplicate_content_ignored);
    TEST_ADD(content_then_charset);
    TEST_ADD(all_whitespace);
    TEST_ADD(gt_without_lt);
    TEST_ADD(skip_name_reaches_end);
    TEST_ADD(content_charset_eq_immediate_semicolon);
    TEST_ADD(content_charset_eq_immediate_space);
    TEST_ADD(bogus_excl_immediate_gt);
    TEST_ADD(attr_eq_at_eof);
    TEST_ADD(non_meta_unclosed_attr_quote);
    TEST_ADD(minimal_comment_5bytes);
    TEST_ADD(comment_immediate_gt_6plus);

    TEST_RUN("lexbor/html/encoding_prescan");
    TEST_RELEASE();
}
