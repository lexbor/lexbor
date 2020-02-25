/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include <unit/test.h>

#include <lexbor/font/font.h>


TEST_BEGIN(init)
{
    lxb_font_t *mf;
    lxb_status_t status;
    uint8_t *data;
    size_t size;

    mf = lxb_font_create();
    printf(" * 0 : 0x%p\n", mf);
    test_ne(mf, NULL);

    status = lxb_font_init(mf);
    printf(" * 1 : %u\n", status);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_font_load_from_file(mf, "../../files/lexbor/font/arial.ttf",
                                     &data, &size);
    printf(" * 2 : %u\n", status);

    test_eq(status, LXB_STATUS_OK);

    lxb_font_destroy(mf, true);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(init);

    TEST_RUN("lexbor/font/offset_table");
    TEST_RELEASE();
}
