/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/encoding.h"
#include "lexbor/encoding/encoding.h"
#include "lexbor/core/fs.h"
#include "lexbor/core/str.h"
#include "unit/test.h"


typedef struct {
    unsigned test_total;
    unsigned test_fail;
} test_ctx_t;


static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx_ptr);

static bool
process_test_file(test_ctx_t *ctx, const char *filepath);

static bool
run_test(test_ctx_t *ctx, const char *filepath,
         const lxb_char_t *data, size_t data_len,
         const lxb_char_t *expected, size_t expected_len, unsigned test_num);


int
main(int argc, const char *argv[])
{
    test_ctx_t ctx;
    lxb_status_t status;
    const lxb_char_t *full_path;

    if (argc != 2) {
        printf("Usage:\n\thtml5lib_tests <directory path>\n");
        return EXIT_FAILURE;
    }

    full_path = (const lxb_char_t *) argv[1];

    memset(&ctx, 0, sizeof(test_ctx_t));

    status = lexbor_fs_dir_read(full_path, LEXBOR_FS_DIR_OPT_WITHOUT_DIR
                                |LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN,
                                file_callback, &ctx);

    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to read directory: %s", argv[1]);
        return EXIT_FAILURE;
    }

    TEST_PRINTLN("\nResults: %u total, %u failed, %u passed",
                 ctx.test_total, ctx.test_fail,
                 ctx.test_total - ctx.test_fail);

    if (ctx.test_fail > 0) {
        TEST_PRINTLN("FAILED");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*
 * Directory callback: process only .dat files.
 */
static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx_ptr)
{
    test_ctx_t *ctx = ctx_ptr;

    if (filename_len < 5
        || strncmp((const char *) &filename[filename_len - 4], ".dat", 4) != 0)
    {
        return LEXBOR_ACTION_OK;
    }

    if (!process_test_file(ctx, (const char *) fullpath)) {
        return LEXBOR_ACTION_STOP;
    }

    return LEXBOR_ACTION_OK;
}

/*
 * Parse a .dat file and run all tests within it.
 *
 * Format:
 *   #data
 *   <html content, possibly multi-line>
 *   #encoding
 *   <expected encoding name>
 *   <blank line or next #data>
 */
static bool
process_test_file(test_ctx_t *ctx, const char *filepath)
{
    size_t file_len;
    lxb_char_t *file_data;
    bool in_data, in_encoding;
    const char *p, *end, *line_start;
    const char *data_start;
    size_t data_len;
    size_t expected_len;
    size_t test_num;
    lxb_char_t expected[128];

    file_data = lexbor_fs_file_easy_read((const lxb_char_t *) filepath,
                                         &file_len);
    if (file_data == NULL) {
        TEST_PRINTLN("Failed to read file: %s", filepath);
        return false;
    }

    TEST_PRINTLN("Test file: %s", filepath);

    p = (const char *) file_data;
    end = p + file_len;
    test_num = 0;

    in_data = false;
    in_encoding = false;
    data_start = NULL;
    data_len = 0;
    expected_len = 0;

    while (p < end) {
        /* Find end of current line. */
        line_start = p;

        while (p < end && *p != '\n') {
            p++;
        }

        /* line_start..p is the current line (without \n). */

        if ((size_t) (p - line_start) == 5
            && memcmp(line_start, "#data", 5) == 0)
        {
            in_data = true;
            in_encoding = false;
            data_start = (p < end) ? p + 1 : p;
            data_len = 0;

            /* Skip \n */
            if (p < end) {
                p++;
            }

            continue;
        }

        if ((size_t) (p - line_start) == 9
            && memcmp(line_start, "#encoding", 9) == 0)
        {
            if (in_data && data_start != NULL) {
                /* data_len: from data_start to line_start - 1 (skip trailing \n). */
                if (line_start > data_start && line_start[-1] == '\n') {
                    data_len = (size_t) (line_start - data_start - 1);
                }
                else {
                    data_len = (size_t) (line_start - data_start);
                }
            }

            in_data = false;
            in_encoding = true;

            /* Skip \n */
            if (p < end) {
                p++;
            }

            continue;
        }

        if (in_encoding) {
            /* This line is the expected encoding name. */
            expected_len = (size_t) (p - line_start);

            if (expected_len >= sizeof(expected)) {
                expected_len = sizeof(expected) - 1;
            }

            memcpy(expected, line_start, expected_len);
            expected[expected_len] = '\0';

            in_encoding = false;

            /* Run the test. */
            test_num++;
            ctx->test_total++;

            if (!run_test(ctx, filepath,
                          (const lxb_char_t *) data_start, data_len,
                          expected, expected_len, test_num))
            {
                ctx->test_fail++;
            }

            data_start = NULL;
            data_len = 0;
        }

        /* Skip \n */
        if (p < end) {
            p++;
        }
    }

    lexbor_free(file_data);

    return true;
}

/*
 * Run a single encoding test.
 *
 * 1. Check for UTF-8 BOM (EF BB BF).
 * 2. Run lxb_html_encoding_determine() (prescan for meta charset).
 * 3. Resolve the found encoding name through lxb_encoding_data_prescan_validate().
 * 4. If nothing found, the default is "windows-1252".
 * 5. Compare (case-insensitive) with the expected encoding.
 */
static bool
run_test(test_ctx_t *ctx, const char *filepath,
         const lxb_char_t *data, size_t data_len,
         const lxb_char_t *expected, size_t expected_len, unsigned test_num)
{
    size_t length;
    lxb_encoding_t enc;
    lxb_status_t status;
    lxb_html_encoding_t em;
    lxb_html_encoding_entry_t *entry;
    const lxb_encoding_data_t *enc_data;
    const lxb_char_t *result_name;

    /* 1. Check for BOM. */
    enc = lxb_encoding_bom_sniff(data, data_len);
    if (enc != LXB_ENCODING_DEFAULT) {
        enc_data = lxb_encoding_data(enc);
        result_name = enc_data->name;
        goto compare;
    }

    /* 2. Prescan. */
    status = lxb_html_encoding_init(&em);
    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("  #%u: FAIL (init error)", test_num);
        return false;
    }

    status = lxb_html_encoding_determine(&em, data, data + data_len);
    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("  #%u: FAIL (determine error)", test_num);
        lxb_html_encoding_destroy(&em, false);
        return false;
    }

    length = lxb_html_encoding_meta_length(&em);

    /*
     * 3. Iterate over all found entries and take the first one that resolves
     *    to a valid encoding. Per spec, unknown labels are skipped and the
     *    prescan continues.
     */
    enc_data = NULL;

    for (size_t i = 0; i < length; i++) {
        entry = lxb_html_encoding_meta_entry(&em, i);

        enc_data = lxb_encoding_data_prescan_validate(entry->name,
                                                      (size_t) (entry->end - entry->name));
        if (enc_data != NULL) {
            break;
        }
    }

    if (enc_data == NULL) {
        /* No valid encoding found — default. */
        lxb_html_encoding_destroy(&em, false);
        result_name = (const lxb_char_t *) "windows-1252";
        goto compare;
    }

    result_name = enc_data->name;

    lxb_html_encoding_destroy(&em, false);

compare:

    if (lexbor_str_data_casecmp(result_name, expected)) {
        return true;
    }

    TEST_PRINTLN("  #%u: FAIL in %s", test_num, filepath);
    TEST_PRINTLN("    Expected: %s", expected);
    TEST_PRINTLN("    Got:      %s", result_name);

    return false;
}
