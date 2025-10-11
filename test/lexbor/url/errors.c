/*
 * Copyright (C) 2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>
#include <lexbor/url/url.h>


TEST_BEGIN(use_log_after_free)
{
    lxb_status_t status;
    lxb_url_parser_t parser;
    lexbor_plog_entry_t *error;
    static const lexbor_str_t input = lexbor_str("x://:\n/");

    status = lxb_url_parser_init(&parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    (void) lxb_url_parse(&parser, NULL, input.data, input.length);

    while ((error = lexbor_array_obj_pop(&parser.log->list)) != NULL) {
        printf("error: %d at %s\n", error->id, error->data);
    }

    lxb_url_parser_memory_destroy(&parser);
    lxb_url_parser_destroy(&parser, false);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(use_log_after_free);

    TEST_RUN("lexbor/url/errors");
    TEST_RELEASE();
}
