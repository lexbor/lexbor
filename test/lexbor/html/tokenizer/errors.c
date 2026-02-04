/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>

#include <lexbor/html/html.h>


TEST_BEGIN(all)
{
    size_t len;
    const lxb_char_t *desc;

    for (lxb_html_tokenizer_error_id_t id = 0; id < LXB_HTML_TOKENIZER_ERROR_LAST_ENTRY + 2; id++) {
        desc = lxb_html_tokenizer_error_to_string(id, &len);
        test_ne(desc, NULL);
        test_ne(len, 0);
    }
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(all);

    TEST_RUN("lexbor/html/tokenizer/errors");
    TEST_RELEASE();
}
