/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/encoding.h>

#include <lexbor/core/fs.h>


#define FAILED(with_usage, ...)                                                \
    do {                                                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
                                                                               \
        if (with_usage) {                                                      \
            usage();                                                           \
        }                                                                      \
                                                                               \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
    while (0)

static void
usage(void)
{
    printf("Usage:\n");
    printf("    encoding <file-path-to-html>\n");
}

int
main(int argc, const char *argv[])
{
    size_t len;
    lxb_char_t *html;
    lxb_status_t status;
    lxb_html_encoding_t em;
    lxb_html_encoding_entry_t *entry;

    if (argc != 2) {
        usage();
        exit(EXIT_SUCCESS);
    }

    html = lexbor_fs_file_easy_read((lxb_char_t *) argv[1], &len);
    if (html == NULL) {
        FAILED(true, "Failed to read file: %s", argv[1]);
    }

    status = lxb_html_encoding_init(&em);
    if (status != LXB_STATUS_OK) {
        FAILED(false, "Failed to init html encoding");
    }

    /*
     * By specification:
     * "For instance, a user agent might wait 500ms or 1024 bytes,
     *  whichever came first."
     * 
     * This is not a strict rule, but it saves time. Most often,
     * a meta encoding tag is present in the first 1024 bytes of HTML.
     */

     /*
        if (len > 1024) {
            len = 1024;
        }
    */

    status = lxb_html_encoding_determine(&em, html, (html + len));
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    entry = lxb_html_encoding_meta_entry(&em, 0);
    if (entry != NULL) {
        printf("%.*s\n", (int) (entry->end - entry->name), entry->name);
    }
    else {
        printf("Encoding not found\n");
    }

    lexbor_free(html);
    lxb_html_encoding_destroy(&em, false);

    return 0;

failed:

    lexbor_free(html);
    lxb_html_encoding_destroy(&em, false);

    FAILED(false, "Failed to determine encoding");
}
