/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

/*
 * Test runner for WPT HTML tree construction tests.
 *
 * The test files are consumed directly in the WPT .dat format from:
 * html/syntax/parsing/resources
 *
 * Parse errors are read and skipped for now.  The runner compares only the
 * serialized tree dump.  Each test is parsed both as one full input buffer and
 * byte-by-byte through the chunk parser.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lexbor/core/fs.h>
#include <lexbor/core/str.h>
#include <lexbor/tag/tag.h>
#include <lexbor/ns/ns.h>
#include <lexbor/html/parser.h>
#include <lexbor/html/serialize.h>
#include <lexbor/html/interfaces/document.h>

#include <unit/test.h>


typedef enum {
    WPT_SECTION_NONE,
    WPT_SECTION_DATA,
    WPT_SECTION_ERRORS,
    WPT_SECTION_NEW_ERRORS,
    WPT_SECTION_FRAGMENT,
    WPT_SECTION_DOCUMENT
}
wpt_section_t;

typedef enum {
    WPT_MARKER_NONE,
    WPT_MARKER_DATA,
    WPT_MARKER_ERRORS,
    WPT_MARKER_NEW_ERRORS,
    WPT_MARKER_FRAGMENT,
    WPT_MARKER_SCRIPT_OFF,
    WPT_MARKER_SCRIPT_ON,
    WPT_MARKER_DOCUMENT
}
wpt_marker_t;

typedef enum {
    WPT_PARSE_FULL,
    WPT_PARSE_CHUNK_BY_BYTE,
    WPT_PARSE__LAST
}
wpt_parse_mode_t;

typedef struct {
    const char *content;
    const char *reason;
}
wpt_skip_content_t;

typedef struct {
    lxb_tag_id_t tag_id;
    lxb_ns_id_t  ns;
}
fragment_entry_t;

typedef struct {
    unsigned        number;

    const lxb_char_t *data;
    size_t            data_len;

    const lxb_char_t *fragment;
    size_t            fragment_len;

    lxb_char_t      *document;
    size_t           document_len;

    int              scripting;
    bool             have_data;
    bool             have_fragment;
    bool             have_document;
}
wpt_test_t;

typedef struct {
    lexbor_hash_t *tags;
    bool           fatal_error;
    unsigned      files;
    unsigned      test_total[WPT_PARSE__LAST];
    unsigned      test_fail[WPT_PARSE__LAST];
    unsigned      test_skip[WPT_PARSE__LAST];
}
wpt_ctx_t;


static const wpt_skip_content_t wpt_js_required_content[] = {
    {"document.write(", "document.write requires JS execution"},
    {"document.writeln(", "document.writeln requires JS execution"},
    {"document.getElementById(", "DOM mutation requires JS execution"},
    {"document.getElementsByTagName(", "DOM mutation requires JS execution"},
    {NULL, NULL}
};


static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx);

static bool
process_test_file(wpt_ctx_t *ctx, const char *filepath);

static bool
run_test(wpt_ctx_t *ctx, const char *filepath, const wpt_test_t *test,
         bool scripting);

static bool
run_test_with_mode(wpt_ctx_t *ctx, const char *filepath,
                   const wpt_test_t *test, bool scripting,
                   wpt_parse_mode_t mode);

static lxb_html_document_t *
parse_document_chunk_by_byte(lxb_html_parser_t *parser,
                             const lxb_char_t *data, size_t len);

static lxb_dom_node_t *
parse_fragment_chunk_by_byte(lxb_html_parser_t *parser,
                             const fragment_entry_t *fragment,
                             const lxb_char_t *data, size_t len);

static bool
serialize_and_compare(const char *filepath, const wpt_test_t *test,
                      lxb_dom_node_t *root, bool scripting,
                      wpt_parse_mode_t mode);

static const wpt_skip_content_t *
wpt_test_requires_js(const wpt_test_t *test, bool scripting);

static bool
data_contains(const lxb_char_t *data, size_t len, const char *content);

static const char *
parse_mode_name(wpt_parse_mode_t mode);

static void
wpt_test_init(wpt_test_t *test);

static void
wpt_test_clean(wpt_test_t *test);

static bool
test_finalize_document(wpt_test_t *test, const lxb_char_t *start,
                       const lxb_char_t *end);

static void
test_finalize_data(wpt_test_t *test, const lxb_char_t *end);

static bool
test_run_and_clean(wpt_ctx_t *ctx, const char *filepath, wpt_test_t *test);

static lxb_char_t *
document_normalize(const lxb_char_t *data, size_t len, size_t *out_len);

static wpt_marker_t
marker_by_line(const lxb_char_t *data, size_t len);

static bool
line_eq(const lxb_char_t *data, size_t len, const char *str);

static const lxb_char_t *
line_next(const lxb_char_t *line, const lxb_char_t *end);

static fragment_entry_t
fragment_entry(wpt_ctx_t *ctx, const lxb_char_t *data, size_t len);

static void
print_data(const char *name, const lxb_char_t *data, size_t len);


int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    wpt_ctx_t ctx = {0};

    if (argc != 2) {
        printf("Usage:\n\twpt_tree_construction <directory path>\n");
        return EXIT_FAILURE;
    }

    ctx.tags = lexbor_hash_create();
    if (ctx.tags == NULL) {
        TEST_PRINTLN("Failed to create tag hash");
        return EXIT_FAILURE;
    }

    status = lexbor_hash_init(ctx.tags, 128, sizeof(lxb_tag_data_t));
    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to create tag hash");
        lexbor_hash_destroy(ctx.tags, true);
        return EXIT_FAILURE;
    }

    status = lexbor_fs_dir_read((const lxb_char_t *) argv[1],
                                LEXBOR_FS_DIR_OPT_WITHOUT_DIR
                                | LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN,
                                file_callback, &ctx);

    lexbor_hash_destroy(ctx.tags, true);

    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to read directory: %s", argv[1]);
        return EXIT_FAILURE;
    }

    if (ctx.fatal_error) {
        TEST_PRINTLN("Failed to process WPT files in: %s", argv[1]);
        return EXIT_FAILURE;
    }

    if (ctx.files == 0) {
        TEST_PRINTLN("No WPT .dat files found in: %s", argv[1]);
        return EXIT_FAILURE;
    }

    TEST_PRINTLN("\nResults:");
    TEST_PRINTLN("  full: %u total, %u failed, %u passed, %u skipped",
                 ctx.test_total[WPT_PARSE_FULL],
                 ctx.test_fail[WPT_PARSE_FULL],
                 ctx.test_total[WPT_PARSE_FULL]
                 - ctx.test_fail[WPT_PARSE_FULL]
                 - ctx.test_skip[WPT_PARSE_FULL],
                 ctx.test_skip[WPT_PARSE_FULL]);
    TEST_PRINTLN("  chunk-by-byte: %u total, %u failed, %u passed, %u skipped",
                 ctx.test_total[WPT_PARSE_CHUNK_BY_BYTE],
                 ctx.test_fail[WPT_PARSE_CHUNK_BY_BYTE],
                 ctx.test_total[WPT_PARSE_CHUNK_BY_BYTE]
                 - ctx.test_fail[WPT_PARSE_CHUNK_BY_BYTE]
                 - ctx.test_skip[WPT_PARSE_CHUNK_BY_BYTE],
                 ctx.test_skip[WPT_PARSE_CHUNK_BY_BYTE]);

    if (ctx.test_fail[WPT_PARSE_FULL] != 0
        || ctx.test_fail[WPT_PARSE_CHUNK_BY_BYTE] != 0)
    {
        TEST_PRINTLN("FAILED");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx)
{
    wpt_ctx_t *wpt = ctx;

    if (filename_len < 5
        || strncmp((const char *) &filename[filename_len - 4], ".dat", 4) != 0)
    {
        return LEXBOR_ACTION_OK;
    }

    wpt->files += 1;

    if (process_test_file(wpt, (const char *) fullpath) == false) {
        wpt->fatal_error = true;
        return LEXBOR_ACTION_STOP;
    }

    return LEXBOR_ACTION_OK;
}

static bool
process_test_file(wpt_ctx_t *ctx, const char *filepath)
{
    size_t file_len;
    lxb_char_t *file_data;
    const lxb_char_t *p, *end, *line, *next, *section_start;
    size_t line_len;
    unsigned test_number;
    wpt_marker_t marker;
    wpt_section_t section;
    wpt_test_t test;

    file_data = lexbor_fs_file_easy_read((const lxb_char_t *) filepath,
                                         &file_len);
    if (file_data == NULL) {
        TEST_PRINTLN("Failed to read file: %s", filepath);
        return false;
    }

    TEST_PRINTLN("Test file: %s", filepath);

    wpt_test_init(&test);

    p = file_data;
    end = file_data + file_len;
    section_start = NULL;
    section = WPT_SECTION_NONE;
    test_number = 0;

    while (p < end) {
        line = p;

        while (p < end && *p != '\n') {
            p++;
        }

        line_len = (size_t) (p - line);
        if (line_len != 0 && line[line_len - 1] == '\r') {
            line_len -= 1;
        }

        next = line_next(p, end);
        marker = marker_by_line(line, line_len);

        if (marker == WPT_MARKER_DATA) {
            if (test.have_data || test.have_document) {
                if (test.have_document) {
                    if (test_finalize_document(&test, section_start, line)
                        == false)
                    {
                        lexbor_free(file_data);
                        wpt_test_clean(&test);
                        return false;
                    }
                }

                if (test_run_and_clean(ctx, filepath, &test) == false) {
                    lexbor_free(file_data);
                    return false;
                }

                wpt_test_init(&test);
            }

            test_number++;
            test.number = test_number;
            test.data = next;
            test.have_data = true;
            section = WPT_SECTION_DATA;
            p = next;

            continue;
        }

        if (marker != WPT_MARKER_NONE) {
            if (section == WPT_SECTION_DATA) {
                test_finalize_data(&test, line);
            }

            switch (marker) {
                case WPT_MARKER_ERRORS:
                    section = WPT_SECTION_ERRORS;
                    break;

                case WPT_MARKER_NEW_ERRORS:
                    section = WPT_SECTION_NEW_ERRORS;
                    break;

                case WPT_MARKER_FRAGMENT:
                    section = WPT_SECTION_FRAGMENT;
                    break;

                case WPT_MARKER_SCRIPT_OFF:
                    test.scripting = 0;
                    section = WPT_SECTION_NONE;
                    break;

                case WPT_MARKER_SCRIPT_ON:
                    test.scripting = 1;
                    section = WPT_SECTION_NONE;
                    break;

                case WPT_MARKER_DOCUMENT:
                    test.have_document = true;
                    section_start = next;
                    section = WPT_SECTION_DOCUMENT;
                    break;

                default:
                    break;
            }

            p = next;
            continue;
        }

        if (section == WPT_SECTION_FRAGMENT && test.have_fragment == false) {
            test.fragment = line;
            test.fragment_len = line_len;
            test.have_fragment = true;
            section = WPT_SECTION_NONE;
        }

        p = next;
    }

    if (test.have_data || test.have_document) {
        if (test.have_document) {
            if (test_finalize_document(&test, section_start, end) == false) {
                lexbor_free(file_data);
                wpt_test_clean(&test);
                return false;
            }
        }

        if (test_run_and_clean(ctx, filepath, &test) == false) {
            lexbor_free(file_data);
            return false;
        }
    }

    lexbor_free(file_data);

    return true;
}

static bool
test_run_and_clean(wpt_ctx_t *ctx, const char *filepath, wpt_test_t *test)
{
    bool ok;

    if (test->have_data == false || test->have_document == false) {
        TEST_PRINTLN("Malformed test in %s near #%u", filepath, test->number);
        wpt_test_clean(test);
        return false;
    }

    if (test->scripting == -1) {
        ok = run_test(ctx, filepath, test, false);
        ok = run_test(ctx, filepath, test, true) && ok;
    }
    else {
        ok = run_test(ctx, filepath, test, test->scripting != 0);
    }

    wpt_test_clean(test);

    return ok;
}

static bool
run_test(wpt_ctx_t *ctx, const char *filepath, const wpt_test_t *test,
         bool scripting)
{
    bool ok;

    ok = run_test_with_mode(ctx, filepath, test, scripting, WPT_PARSE_FULL);
    ok = run_test_with_mode(ctx, filepath, test, scripting,
                            WPT_PARSE_CHUNK_BY_BYTE) && ok;

    return ok;
}

static bool
run_test_with_mode(wpt_ctx_t *ctx, const char *filepath,
                   const wpt_test_t *test, bool scripting,
                   wpt_parse_mode_t mode)
{
    lxb_status_t status;
    lxb_dom_node_t *root;
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;
    fragment_entry_t fragment;

    ctx->test_total[mode]++;

    if (wpt_test_requires_js(test, scripting) != NULL) {
        ctx->test_skip[mode]++;
        return true;
    }

    parser = lxb_html_parser_create();
    if (parser == NULL) {
        TEST_PRINTLN("Failed to create parser");
        return false;
    }

    status = lxb_html_parser_init(parser);
    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to init parser");
        lxb_html_parser_destroy(parser);
        return false;
    }

    lxb_html_tree_scripting_set(parser->tree, scripting);

    if (test->have_fragment) {
        fragment = fragment_entry(ctx, test->fragment, test->fragment_len);
        if (fragment.tag_id == LXB_TAG__UNDEF) {
            ctx->test_fail[mode]++;
            lxb_html_parser_destroy(parser);
            return true;
        }

        if (mode == WPT_PARSE_CHUNK_BY_BYTE) {
            root = parse_fragment_chunk_by_byte(parser, &fragment,
                                                test->data, test->data_len);
        }
        else {
            root = lxb_html_parse_fragment_by_tag_id(parser, NULL,
                                                     fragment.tag_id, fragment.ns,
                                                     test->data, test->data_len);
        }

        if (root == NULL) {
            TEST_PRINTLN("Failed to parse fragment: %s #%u, parse: %s",
                         filepath, test->number, parse_mode_name(mode));
            ctx->test_fail[mode]++;
            lxb_html_parser_destroy(parser);
            return true;
        }

        if (serialize_and_compare(filepath, test, root, scripting, mode)
            == false)
        {
            ctx->test_fail[mode]++;
        }

        document = lxb_html_interface_document(root->owner_document);
        lxb_html_document_destroy(document);
        lxb_html_parser_destroy(parser);

        return true;
    }

    if (mode == WPT_PARSE_CHUNK_BY_BYTE) {
        document = parse_document_chunk_by_byte(parser, test->data,
                                                test->data_len);
    }
    else {
        document = lxb_html_parse(parser, test->data, test->data_len);
    }

    if (document == NULL) {
        TEST_PRINTLN("Failed to parse document: %s #%u, parse: %s",
                     filepath, test->number, parse_mode_name(mode));
        ctx->test_fail[mode]++;
        lxb_html_parser_destroy(parser);
        return true;
    }

    root = lxb_dom_interface_node(document);
    if (serialize_and_compare(filepath, test, root, scripting, mode) == false) {
        ctx->test_fail[mode]++;
    }

    lxb_html_document_destroy(document);
    lxb_html_parser_destroy(parser);

    return true;
}

static lxb_html_document_t *
parse_document_chunk_by_byte(lxb_html_parser_t *parser,
                             const lxb_char_t *data, size_t len)
{
    size_t i;
    lxb_status_t status;
    lxb_html_document_t *document;

    document = lxb_html_parse_chunk_begin(parser);
    if (document == NULL) {
        return NULL;
    }

    for (i = 0; i < len; i++) {
        lxb_char_t chunk = data[i];

        status = lxb_html_parse_chunk_process(parser, &chunk, 1);
        if (status != LXB_STATUS_OK) {
            lxb_html_document_destroy(document);
            return NULL;
        }
    }

    status = lxb_html_parse_chunk_end(parser);
    if (status != LXB_STATUS_OK) {
        lxb_html_document_destroy(document);
        return NULL;
    }

    return document;
}

static lxb_dom_node_t *
parse_fragment_chunk_by_byte(lxb_html_parser_t *parser,
                             const fragment_entry_t *fragment,
                             const lxb_char_t *data, size_t len)
{
    size_t i;
    lxb_status_t status;

    status = lxb_html_parse_fragment_chunk_begin(parser, NULL,
                                                 fragment->tag_id,
                                                 fragment->ns);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    for (i = 0; i < len; i++) {
        lxb_char_t chunk = data[i];

        status = lxb_html_parse_fragment_chunk_process(parser, &chunk, 1);
        if (status != LXB_STATUS_OK) {
            return NULL;
        }
    }

    return lxb_html_parse_fragment_chunk_end(parser);
}

static bool
serialize_and_compare(const char *filepath, const wpt_test_t *test,
                      lxb_dom_node_t *root, bool scripting,
                      wpt_parse_mode_t mode)
{
    lxb_status_t status;
    lexbor_str_t res = {0};

    status = lxb_html_serialize_pretty_deep_str(root,
                                                LXB_HTML_SERIALIZE_OPT_HTML5TEST,
                                                0, &res);
    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to serialize: %s #%u", filepath, test->number);
        return false;
    }

    if (res.length != 0 && res.data[res.length - 1] == '\n') {
        res.length -= 1;
        res.data[res.length] = 0x00;
    }

    if (res.length == test->document_len
        && (res.length == 0
            || memcmp(res.data, test->document, res.length) == 0))
    {
        return true;
    }

    TEST_PRINTLN("Error: The result does not match.");
    TEST_PRINTLN("File: %s", filepath);
    TEST_PRINTLN("Test: #%u, scripting: %s, parse: %s", test->number,
                 scripting ? "on" : "off", parse_mode_name(mode));
    print_data("Data", test->data, test->data_len);
    print_data("Need", test->document, test->document_len);
    print_data("Have", res.data, res.length);

    return false;
}

static const wpt_skip_content_t *
wpt_test_requires_js(const wpt_test_t *test, bool scripting)
{
    const wpt_skip_content_t *skip;

    if (scripting == false) {
        return NULL;
    }

    for (skip = wpt_js_required_content; skip->content != NULL; skip++) {
        if (data_contains(test->data, test->data_len, skip->content)) {
            return skip;
        }
    }

    return NULL;
}

static bool
data_contains(const lxb_char_t *data, size_t len, const char *content)
{
    size_t i, content_len;

    if (data == NULL || content == NULL) {
        return false;
    }

    content_len = strlen(content);
    if (content_len == 0) {
        return true;
    }

    if (len < content_len) {
        return false;
    }

    for (i = 0; i <= len - content_len; i++) {
        if (memcmp(&data[i], content, content_len) == 0) {
            return true;
        }
    }

    return false;
}

static const char *
parse_mode_name(wpt_parse_mode_t mode)
{
    switch (mode) {
        case WPT_PARSE_FULL:
            return "full";

        case WPT_PARSE_CHUNK_BY_BYTE:
            return "chunk-by-byte";

        default:
            return "unknown";
    }
}

static void
wpt_test_init(wpt_test_t *test)
{
    memset(test, 0, sizeof(wpt_test_t));
    test->scripting = -1;
}

static void
wpt_test_clean(wpt_test_t *test)
{
    if (test->document != NULL) {
        lexbor_free(test->document);
    }

    wpt_test_init(test);
}

static bool
test_finalize_document(wpt_test_t *test, const lxb_char_t *start,
                       const lxb_char_t *end)
{
    if (start == NULL || end < start) {
        return false;
    }

    test->document = document_normalize(start, (size_t) (end - start),
                                        &test->document_len);
    if (test->document == NULL) {
        return false;
    }

    return true;
}

static void
test_finalize_data(wpt_test_t *test, const lxb_char_t *end)
{
    size_t len;

    if (test->data == NULL || end < test->data) {
        test->data_len = 0;
        return;
    }

    len = (size_t) (end - test->data);

    if (len != 0 && test->data[len - 1] == '\n') {
        len -= 1;
    }

    test->data_len = len;
}

static lxb_char_t *
document_normalize(const lxb_char_t *data, size_t len, size_t *out_len)
{
    lxb_char_t *result, *out;
    const lxb_char_t *p, *end, *line;
    size_t line_len;

    while (len >= 2 && data[len - 1] == '\n' && data[len - 2] == '\n') {
        len -= 1;
    }

    if (len != 0 && data[len - 1] == '\n') {
        len -= 1;
    }

    result = lexbor_malloc(len + 1);
    if (result == NULL) {
        return NULL;
    }

    out = result;
    p = data;
    end = data + len;

    while (p < end) {
        line = p;

        while (p < end && *p != '\n') {
            p++;
        }

        line_len = (size_t) (p - line);
        if (line_len >= 2 && line[0] == '|' && line[1] == ' ') {
            line += 2;
            line_len -= 2;
        }

        if (line_len != 0) {
            memcpy(out, line, line_len);
            out += line_len;
        }

        if (p < end) {
            *out++ = '\n';
            p++;
        }
    }

    *out = 0x00;
    *out_len = (size_t) (out - result);

    return result;
}

static wpt_marker_t
marker_by_line(const lxb_char_t *data, size_t len)
{
    if (line_eq(data, len, "#data")) {
        return WPT_MARKER_DATA;
    }

    if (line_eq(data, len, "#errors")) {
        return WPT_MARKER_ERRORS;
    }

    if (line_eq(data, len, "#new-errors")) {
        return WPT_MARKER_NEW_ERRORS;
    }

    if (line_eq(data, len, "#document-fragment")) {
        return WPT_MARKER_FRAGMENT;
    }

    if (line_eq(data, len, "#script-off")) {
        return WPT_MARKER_SCRIPT_OFF;
    }

    if (line_eq(data, len, "#script-on")) {
        return WPT_MARKER_SCRIPT_ON;
    }

    if (line_eq(data, len, "#document")) {
        return WPT_MARKER_DOCUMENT;
    }

    return WPT_MARKER_NONE;
}

static bool
line_eq(const lxb_char_t *data, size_t len, const char *str)
{
    size_t str_len = strlen(str);

    return len == str_len && memcmp(data, str, str_len) == 0;
}

static const lxb_char_t *
line_next(const lxb_char_t *line, const lxb_char_t *end)
{
    if (line < end && *line == '\n') {
        return line + 1;
    }

    return line;
}

static fragment_entry_t
fragment_entry(wpt_ctx_t *ctx, const lxb_char_t *data, size_t len)
{
    lxb_ns_id_t ns;
    lxb_tag_id_t tag_id;
    fragment_entry_t entry = {0};

    entry.tag_id = LXB_TAG__UNDEF;
    entry.ns = LXB_NS_HTML;

    if (len > 4 && memcmp(data, "svg ", 4) == 0) {
        ns = LXB_NS_SVG;
        data += 4;
        len -= 4;
    }
    else if (len > 5 && memcmp(data, "math ", 5) == 0) {
        ns = LXB_NS_MATH;
        data += 5;
        len -= 5;
    }
    else {
        ns = LXB_NS_HTML;
    }

    tag_id = lxb_tag_id_by_name(ctx->tags, data, len);
    if (tag_id == LXB_TAG__UNDEF) {
        TEST_PRINTLN("Unknown fragment context: %.*s", (int) len, data);
        return entry;
    }

    entry.tag_id = tag_id;
    entry.ns = ns;

    return entry;
}

static void
print_data(const char *name, const lxb_char_t *data, size_t len)
{
    TEST_PRINTLN("%s (%lu):", name, (unsigned long) len);

    if (len != 0) {
        fwrite(data, 1, len, stdout);
    }

    printf("\n");
}
