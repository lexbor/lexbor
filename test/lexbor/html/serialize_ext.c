/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/core/fs.h>
#include <lexbor/html/html.h>

#include <unit/test.h>
#include <unit/kv.h>


typedef struct {
    lxb_tag_id_t tag_id;
    lxb_ns_id_t  ns;
}
fragment_entry_t;


static size_t test_count = 0;
static lexbor_hash_t *test_tags;


static lxb_status_t
parse(const lxb_char_t *dir_path);

static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx);

static void
check_entries(unit_kv_t *kv, unit_kv_value_t *value);

static void
check_entry(unit_kv_t *kv, unit_kv_value_t *hash);

static lxb_html_serialize_ext_opt_t
get_options(unit_kv_t *kv, unit_kv_value_t *hash);

static lexbor_str_t *
get_indent(unit_kv_t *kv, unit_kv_value_t *hash);

static bool
get_bool(unit_kv_t *kv, unit_kv_value_t *hash, const char *name);

static fragment_entry_t
get_fragment(unit_kv_t *kv, unit_kv_value_t *hash);

static lxb_ns_id_t
ns_id_by_name(const lxb_char_t *name, size_t len);

static lexbor_str_t *
hash_get_str(unit_kv_t *kv, unit_kv_value_t *hash, const char *name);

static void
print_error(unit_kv_t *kv, unit_kv_value_t *value);


static lxb_status_t
parse(const lxb_char_t *dir_path)
{
    return lexbor_fs_dir_read(dir_path,
                              LEXBOR_FS_DIR_OPT_WITHOUT_DIR
                              |LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN,
                              file_callback, NULL);
}

static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx)
{
    unit_kv_t *kv;
    lxb_status_t status;
    unit_kv_value_t *arr_value;

    if (filename_len < 5
        || strncmp((const char *) &filename[filename_len - 4], ".ton", 4) != 0)
    {
        return LEXBOR_ACTION_OK;
    }

    TEST_PRINTLN("Test file: %s", fullpath);

    kv = unit_kv_create();
    status = unit_kv_init(kv, 256);

    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to create KV parser");
    }

    status = unit_kv_parse_file(kv, fullpath);
    if (status != LXB_STATUS_OK) {
        lexbor_str_t str = unit_kv_parse_error_as_string(kv);

        TEST_PRINTLN("Failed to parse test file:");
        TEST_PRINTLN("%s", str.data);

        unit_kv_string_destroy(kv, &str, false);

        exit(EXIT_FAILURE);
    }

    arr_value = unit_kv_value(kv);
    if (arr_value == NULL) {
        TEST_PRINTLN("File is empty");
    }

    if (unit_kv_is_array(arr_value) == false) {
        TEST_PRINTLN("Error: Need array, but we have:");
        print_error(kv, arr_value);
        exit(EXIT_FAILURE);
    }

    check_entries(kv, arr_value);

    unit_kv_destroy(kv, true);

    return LEXBOR_ACTION_OK;
}

static void
check_entries(unit_kv_t *kv, unit_kv_value_t *value)
{
    unit_kv_array_t *entries = unit_kv_array(value);

    for (size_t i = 0; i < entries->length; i++) {
        TEST_PRINTLN("\tTest number: "LEXBOR_FORMAT_Z, (i + 1));

        if (unit_kv_is_hash(entries->list[i]) == false) {
            TEST_PRINTLN("Error: Need hash (object), but we have:");
            print_error(kv, entries->list[i]);
            exit(EXIT_FAILURE);
        }

        check_entry(kv, entries->list[i]);
        test_count += 1;
    }
}

static void
check_entry(unit_kv_t *kv, unit_kv_value_t *hash)
{
    bool from_document;
    lxb_status_t status;
    lxb_dom_node_t *root;
    lxb_dom_node_t *target;
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;
    lexbor_str_t *data, *expected, *indent_str;
    lxb_html_serialize_ext_opt_t opt;
    fragment_entry_t fragment;
    lexbor_str_t res = {0};

    data = hash_get_str(kv, hash, "data");
    expected = hash_get_str(kv, hash, "result");
    opt = get_options(kv, hash);
    indent_str = get_indent(kv, hash);
    from_document = get_bool(kv, hash, "from_document");
    fragment = get_fragment(kv, hash);

    /* Parse. */

    if (fragment.tag_id != LXB_TAG__UNDEF) {
        parser = lxb_html_parser_create();
        status = lxb_html_parser_init(parser);
        if (status != LXB_STATUS_OK) {
            TEST_FAILURE("Failed to create parser");
        }

        root = lxb_html_parse_fragment_by_tag_id(parser, NULL,
                                                 fragment.tag_id,
                                                 fragment.ns,
                                                 data->data, data->length);
        if (root == NULL) {
            TEST_PRINTLN("Failed to parse fragment data: %s", data->data);
        }

        document = lxb_html_interface_document(root->owner_document);
        target = root;

        lxb_html_parser_destroy(parser);
    }
    else {
        document = lxb_html_document_create();
        if (document == NULL) {
            TEST_FAILURE("Failed to create document");
        }

        status = lxb_html_document_parse(document, data->data, data->length);
        if (status != LXB_STATUS_OK) {
            TEST_FAILURE("Failed to parse data: %s", data->data);
        }

        if (from_document) {
            target = lxb_dom_interface_node(document);
        }
        else {
            /* Serialize from body element. */
            lxb_html_body_element_t *body;

            body = lxb_html_document_body_element(document);
            if (body == NULL) {
                TEST_FAILURE("Failed to find body element");
            }

            target = lxb_dom_interface_node(body);
        }
    }

    /* Serialize. */

    status = lxb_html_serialize_ext_tree_str(target, &res, opt,
                                             indent_str, false);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to serialize");
    }

    /* For PRETTY output, strip trailing newline for comparison. */

    if ((opt & LXB_HTML_SERIALIZE_EXT_OPT_PRETTY) && res.length != 0) {
        if (res.data[res.length - 1] == '\n') {
            res.length--;
            res.data[res.length] = 0x00;
        }
    }

    /* Compare. */

    if (res.length == expected->length
        && (res.length == 0
            || memcmp(res.data, expected->data, res.length) == 0))
    {
        lxb_html_document_destroy(document);
        return;
    }

    TEST_PRINTLN("Error: The result does not match.");
    TEST_PRINTLN("Data:");
    TEST_PRINTLN("%.*s", (int) data->length, (const char *) data->data);
    TEST_PRINTLN("Need (%lu):", (unsigned long) expected->length);
    TEST_PRINTLN("%.*s", (int) expected->length,
                 (const char *) expected->data);
    TEST_PRINTLN("Have (%lu):", (unsigned long) res.length);
    TEST_PRINTLN("%.*s", (int) res.length, (const char *) res.data);

    lxb_html_document_destroy(document);

    TEST_FAILURE(" ");
}

static lxb_html_serialize_ext_opt_t
get_options(unit_kv_t *kv, unit_kv_value_t *hash)
{
    unit_kv_value_t *data;
    unit_kv_array_t *arr;
    lexbor_str_t *str;
    lxb_html_serialize_ext_opt_t opt, tmp_opt;

    opt = LXB_HTML_SERIALIZE_EXT_OPT_UNDEF;

    data = unit_kv_hash_value_nolen_c(hash, "options");
    if (data == NULL) {
        return opt;
    }

    if (unit_kv_is_array(data) == false) {
        TEST_PRINTLN("Parameter 'options' must be ARRAY");
        print_error(kv, data);
        exit(EXIT_FAILURE);
    }

    arr = unit_kv_array(data);

    for (size_t i = 0; i < arr->length; i++) {
        if (unit_kv_is_string(arr->list[i]) == false) {
            TEST_PRINTLN("Option must be STRING");
            print_error(kv, arr->list[i]);
            exit(EXIT_FAILURE);
        }

        str = unit_kv_string(arr->list[i]);

        tmp_opt = lxb_html_serialize_ext_str_to_opt(str->data, str->length);
        if (tmp_opt == LXB_HTML_SERIALIZE_EXT_OPT_UNDEF) {
            TEST_PRINTLN("Unknown option: %.*s",
                         (int) str->length, (const char *) str->data);
            exit(EXIT_FAILURE);
        }

        opt |= tmp_opt;
    }

    return opt;
}

static lexbor_str_t *
get_indent(unit_kv_t *kv, unit_kv_value_t *hash)
{
    unit_kv_value_t *data;

    data = unit_kv_hash_value_nolen_c(hash, "indent");
    if (data == NULL) {
        return NULL;
    }

    if (unit_kv_is_string(data) == false) {
        TEST_PRINTLN("Parameter 'indent' must be STRING");
        print_error(kv, data);
        exit(EXIT_FAILURE);
    }

    return unit_kv_string(data);
}

static bool
get_bool(unit_kv_t *kv, unit_kv_value_t *hash, const char *name)
{
    unit_kv_value_t *data;

    data = unit_kv_hash_value_nolen_c(hash, name);
    if (data == NULL) {
        return false;
    }

    if (unit_kv_is_bool(data) == false) {
        TEST_PRINTLN("Parameter '%s' must be BOOL", name);
        print_error(kv, data);
        exit(EXIT_FAILURE);
    }

    return unit_kv_bool(data);
}

static fragment_entry_t
get_fragment(unit_kv_t *kv, unit_kv_value_t *hash)
{
    unit_kv_value_t *data;
    lexbor_str_t *tag_name, *ns_name;
    fragment_entry_t entry = {0};

    data = unit_kv_hash_value_nolen_c(hash, "fragment");
    if (data == NULL) {
        return entry;
    }

    if (unit_kv_is_hash(data) == false) {
        TEST_PRINTLN("Parameter 'fragment' must be HASH");
        print_error(kv, data);
        exit(EXIT_FAILURE);
    }

    tag_name = hash_get_str(kv, data, "tag");
    ns_name = hash_get_str(kv, data, "ns");

    entry.tag_id = lxb_tag_id_by_name(test_tags,
                                      tag_name->data, tag_name->length);
    if (entry.tag_id == LXB_TAG__UNDEF) {
        TEST_PRINTLN("Unknown tag: %.*s",
                     (int) tag_name->length, tag_name->data);
        print_error(kv, data);
        exit(EXIT_FAILURE);
    }

    entry.ns = ns_id_by_name(ns_name->data, ns_name->length);

    return entry;
}

static lxb_ns_id_t
ns_id_by_name(const lxb_char_t *name, size_t len)
{
    if (len == 4 && memcmp(name, "html", 4) == 0) {
        return LXB_NS_HTML;
    }

    if (len == 4 && memcmp(name, "math", 4) == 0) {
        return LXB_NS_MATH;
    }

    if (len == 3 && memcmp(name, "svg", 3) == 0) {
        return LXB_NS_SVG;
    }

    if (len == 3 && memcmp(name, "xml", 3) == 0) {
        return LXB_NS_XML;
    }

    TEST_FAILURE("Unknown namespace: %.*s", (int) len, (const char *) name);

    return LXB_NS__UNDEF;
}

static lexbor_str_t *
hash_get_str(unit_kv_t *kv, unit_kv_value_t *hash, const char *name)
{
    unit_kv_value_t *data;

    data = unit_kv_hash_value_nolen_c(hash, name);
    if (data == NULL) {
        TEST_PRINTLN("Required parameter missing: %s", name);
        print_error(kv, hash);
        exit(EXIT_FAILURE);
    }

    if (unit_kv_is_string(data) == false) {
        TEST_PRINTLN("Parameter '%s' must be STRING", name);
        print_error(kv, data);
        exit(EXIT_FAILURE);
    }

    return unit_kv_string(data);
}

static void
print_error(unit_kv_t *kv, unit_kv_value_t *value)
{
    lexbor_str_t str;

    str = unit_kv_value_position_as_string(kv, value);
    TEST_PRINTLN("%s", str.data);
    unit_kv_string_destroy(kv, &str, false);

    str = unit_kv_value_fragment_as_string(kv, value);
    TEST_PRINTLN("%s", str.data);
    unit_kv_string_destroy(kv, &str, false);
}

int
main(int argc, const char *argv[])
{
    lxb_status_t status;

    if (argc != 2) {
        printf("Usage:\n\tserialize_ext <directory path>\n");
        return EXIT_FAILURE;
    }

    test_tags = lexbor_hash_create();
    status = lexbor_hash_init(test_tags, 128, sizeof(lxb_tag_data_t));
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to create tag hash");
    }

    TEST_INIT();
    TEST_RUN("lexbor/html/serialize_ext");

    status = parse((const lxb_char_t *) argv[1]);
    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to read directory: %s", argv[1]);
        return EXIT_FAILURE;
    }

    lexbor_hash_destroy(test_tags, true);

    TEST_PRINTLN("\nResults: "LEXBOR_FORMAT_Z" total, "LEXBOR_FORMAT_Z" failed,"
                 " "LEXBOR_FORMAT_Z" passed", test_count, (size_t) 0,
                 test_count);

    TEST_RELEASE();
}
