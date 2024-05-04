/*
 * Copyright (C) 2024 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 *
 * For correct work of this program it is necessary to build the project
 * by define LEXBOR_WITH_PERF.
 *
 * cmake . -DLEXBOR_WITH_PERF=ON
 */

#include <lexbor/html/html.h>
#include <lexbor/core/perf.h>
#include <lexbor/core/fs.h>


static lexbor_action_t
test_dir(const lxb_char_t *fullpath, size_t fullpath_len,
         const lxb_char_t *filename, size_t filename_len, void *ctx);

static int
test_comp(const void * elem1, const void * elem2);

static void
test_usage(void);


int
main(int argc, const char *argv[])
{
    char *end;
    long num;
    size_t repeats;
    lxb_status_t status;
    const lxb_char_t *dirpath;

    if (argc < 2) {
        test_usage();
        return EXIT_SUCCESS;
    }

    dirpath = (const lxb_char_t *) argv[1];
    repeats = 1000;

    if (argc > 2) {
        num = (size_t) strtol(argv[2], &end, 10);

        if (argv[2] == end) {
            printf("Failed to convert string to number\n");
            return LEXBOR_ACTION_STOP;
        }

        if (num < 0) {
            printf("Repeats must be > 0\n");
            return LEXBOR_ACTION_STOP;
        }

        repeats = (size_t) num;
    }

    status = lexbor_fs_dir_read(dirpath, LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN
                                | LEXBOR_FS_DIR_OPT_WITHOUT_DIR, test_dir,
                                &repeats);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static lexbor_action_t
test_dir(const lxb_char_t *fullpath, size_t fullpath_len,
         const lxb_char_t *filename, size_t filename_len, void *ctx)
{
    void *perf;
    size_t length, repeats;
    double cur, total, *median;
    lxb_char_t *html;
    lxb_status_t status;
    lxb_html_document_t *document;

    repeats = *((size_t *) ctx);

    median = lexbor_malloc(sizeof(double) * repeats);
    if (median == NULL) {
        printf("Failed memory allocate for median\n");
        return LEXBOR_ACTION_STOP;
    }

    html = lexbor_fs_file_easy_read(fullpath, &length);
    if (html == NULL) {
        printf("Failed to read file %s\n", fullpath);
        lexbor_free(median);
        return LEXBOR_ACTION_STOP;
    }

    total = 0.0f;

    perf = lexbor_perf_create();

    for (size_t i = 0; i < repeats; i++) {
        document = lxb_html_document_create();

        lexbor_perf_begin(perf);

        status = lxb_html_document_parse(document, html, length);

        lexbor_perf_end(perf);

        if (status != LXB_STATUS_OK) {
            printf("Failed to parse HTML file %s\n", fullpath);
            goto failed;
        }

        cur = lexbor_perf_in_sec(perf);

        total += cur;
        median[i] = cur;

        lxb_html_document_destroy(document);
    }

    qsort(median, repeats, sizeof(double *), test_comp);

    printf("Average %s: %f\n", filename, total / repeats);
    printf("Median %s: %f\n", filename, median[ (size_t) (repeats / 2) ]);

    lexbor_perf_destroy(perf);
    lexbor_free(median);
    lexbor_free(html);

    return LEXBOR_ACTION_OK;

failed:

    lxb_html_document_destroy(document);
    lexbor_perf_destroy(perf);
    lexbor_free(median);
    lexbor_free(html);

    return LEXBOR_ACTION_STOP;

}

static int
test_comp(const void * elem1, const void * elem2)
{
    int f = *((double*) elem1);
    int s = *((double*) elem2);

    if (f > s) return  1;
    if (f < s) return -1;

    return 0;
}

static void
test_usage(void)
{
    printf("Usage: html_perf <dir_path_to_html_files> <repeat_count>\n");
}
