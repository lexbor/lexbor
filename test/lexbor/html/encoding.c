/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <string.h>

#include <unit/test.h>

#include <lexbor/html/encoding.h>


static const lxb_char_t *
full_check(const char *data, const lxb_char_t **out_end, size_t idx)
{
    lxb_status_t status;
    const lxb_char_t *name;
    lxb_html_encoding_t em;
    lxb_html_encoding_entry_t *entry;

    *out_end = NULL;

    status = lxb_html_encoding_init(&em);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    status = lxb_html_encoding_determine(&em, (const lxb_char_t *) data,
                                         (const lxb_char_t *) (data + strlen(data)));
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    entry = lxb_html_encoding_meta_entry(&em, idx);
    if (entry == NULL) {
        goto failed;
    }

    name = entry->name;
    *out_end = entry->end;

    lxb_html_encoding_destroy(&em, false);

    return name;

failed:

    lxb_html_encoding_destroy(&em, false);

    return NULL;
}

static const lxb_char_t *
bad_check(const char *data, const lxb_char_t **out_end, size_t idx)
{
    size_t len;
    lxb_status_t status;
    const lxb_char_t *name;
    lxb_html_encoding_t em;
    lxb_html_encoding_entry_t *entry;

    *out_end = NULL;

    status = lxb_html_encoding_init(&em);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    len = strlen(data);

    for (size_t i = 0; i <= len; i++) {
        status = lxb_html_encoding_determine(&em, (const lxb_char_t *) data, 
                                             (const lxb_char_t *) (data + i));
        if (status != LXB_STATUS_OK) {
            goto failed;
        }

        entry = lxb_html_encoding_meta_entry(&em, idx);
        if (entry != NULL) {
            goto done;
        }

        lxb_html_encoding_clean(&em);
    }

failed:

    lxb_html_encoding_destroy(&em, false);

    return NULL;

done:

    name = entry->name;
    *out_end = entry->end;

    lxb_html_encoding_destroy(&em, false);

    return name;
}

static lxb_status_t
check(const char *data, const char *need, size_t idx)
{
    const lxb_char_t *name, *end;

    name = full_check(data, &end, idx);
    if (need != NULL) {
        if (name == NULL) {
            return LXB_STATUS_ERROR;
        }

        if (strlen(need) != (end - name) || memcmp(name, need, (end - name))) {
            return LXB_STATUS_ERROR;
        }
    }
    else if (name != NULL) {
        return LXB_STATUS_ERROR;
    }

    name = bad_check(data, &end, idx);
    if (need != NULL) {
        if (name == NULL) {
            return LXB_STATUS_ERROR;
        }

        if (strlen(need) != (end - name) || memcmp(name, need, (end - name))) {
            return LXB_STATUS_ERROR;
        }
    }
    else if (name != NULL) {
        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}

TEST_BEGIN(by_meta)
{
    lxb_status_t status;

    status = check("<meta charset=utf-8>", "utf-8", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta charset='utf-8'>", "utf-8", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta charset=' utf-8 '>", " utf-8 ", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta http-equiv=\"content-type\" charset=utf-8>", "utf-8", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta charset=utf-8 http-equiv=\"content-type\">", "utf-8", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">", "utf-8", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta content=\"text/html; charset=utf-8\" http-equiv=\"content-type\">", "utf-8", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta content=\"text/html; charset=utf-8\" http-equiv=\"content-typ\">", NULL, 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta http-equiv=\"content-type\" content=\"text/html; charset='utf-8'\">", "utf-8", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta http-equiv=\"content-type\" content=\"text/html; charset=' utf-8 '\">", " utf-8 ", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta http-equiv=\"content-type\" content=\"text/html; charset='utf-8\">", "utf-8", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta http-equiv=\"content-type\" content=\"text/html; charset='utf-8   \">", "utf-8   ", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta http-equiv=\"content-type\" content=\"text/html; charset=windows-1251\">"
                   "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">", "utf-8", 1);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta content=\"text/html; charset=utf-8\">", NULL, 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta http-equiv=\"content-type\" content=\"text/html\">", NULL, 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<html>\n <meta http-equiv=\"content-type\" charset=utf-8>", "utf-8", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("</html lala='><meta charset=cp1251>'>\n <meta http-equiv=\"content-type\" charset=utf-8>", "utf-8", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta charset=\"windows-1251\" name=\"viewport\" content=\"width\">", "windows-1251", 0);
    test_eq(status, LXB_STATUS_OK);

    status = check("<meta bu charset=\"windows-1251\" be name=\"viewport\" bu content=\"width\" be>", "windows-1251", 0);
    test_eq(status, LXB_STATUS_OK);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(by_meta);

    TEST_RUN("lexbor/html/encoding");
    TEST_RELEASE();
}
