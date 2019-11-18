/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/encoding/encoding.h"


int
main(int argc, const char *argv[])
{
    const lxb_encoding_data_t *enc_data;

    enc_data = lxb_encoding_data_by_name((lxb_char_t *) "uTf-8", 5);
    if (enc_data == NULL) {
        return EXIT_FAILURE;
    }

    printf("%s\n", enc_data->name);

    return EXIT_SUCCESS;
}
