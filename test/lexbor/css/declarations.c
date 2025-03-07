/*
 * Copyright (C) 2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/core/fs.h>
#include <lexbor/core/array.h>

#include <unit/test.h>
#include <unit/kv.h>

#include <lexbor/css/css.h>


#define validate_pointer(ptr, name)                                           \
    if (ptr == NULL) {                                                        \
        TEST_PRINTLN("Required parameter missing: %s", (const char *) name);  \
        return LXB_STATUS_ERROR_UNEXPECTED_DATA; \
    }


typedef struct {
    unit_kv_t        *kv;
    lxb_css_parser_t *parser;
    lexbor_str_t     str;
    lexbor_mraw_t    *mraw;
    lxb_css_memory_t *mem;
}
helper_t;

typedef struct {
    const lxb_char_t *begin;
    const lxb_char_t *end;
    lxb_char_t       ch;
}
chunk_ctx_t;

typedef lxb_status_t
(*parse_data_cb_f)(helper_t *helper, lexbor_str_t *str,
                   unit_kv_array_t *entries);


static lxb_status_t
parse(helper_t *helper, const char *dir_path);

static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx);

static lxb_status_t
check(helper_t *helper, unit_kv_value_t *value);

static lxb_status_t
check_entry(helper_t *helper, unit_kv_value_t *entry, parse_data_cb_f cb);

static lxb_status_t
parse_cb(helper_t *helper, lexbor_str_t *str, unit_kv_array_t *entries);

static lxb_status_t
print_error(helper_t *helper, unit_kv_value_t *value);

static lxb_status_t
compare(helper_t *helper, unit_kv_array_t *arr,
        lxb_css_rule_declaration_list_t *list);

static lxb_status_t
compare_declr(helper_t *helper, lxb_css_rule_t *rule,
              unit_kv_value_t *entry, lxb_css_property_type_t type,
              lexbor_str_t *name, lexbor_str_t *value, bool important);

static lxb_status_t
compare_at(helper_t *helper, lxb_css_rule_t *rule,
           unit_kv_value_t *entry, lxb_css_at_rule_type_t type,
           lexbor_str_t *name, lexbor_str_t *value);

static lxb_status_t
print_compare_error(const char *name, lexbor_str_t *need, lexbor_str_t *have);


int
main(int argc, const char * argv[])
{
    lxb_status_t status;
    helper_t helper = {0};
    const char *dir_path;

    if (argc != 2) {
        printf("Usage:\n\tdeclarations <directory path>\n");
        return EXIT_FAILURE;
    }

    dir_path = argv[1];

    TEST_INIT();

    helper.kv = unit_kv_create();
    status = unit_kv_init(helper.kv, 256);

    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    helper.mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(helper.mraw, 256);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    helper.mem = lxb_css_memory_create();
    status = lxb_css_memory_init(helper.mem, 128);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    status = parse(&helper, dir_path);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    TEST_RUN("lexbor/css/declarations");

    unit_kv_destroy(helper.kv, true);
    lexbor_mraw_destroy(helper.mraw, true);
    lxb_css_memory_destroy(helper.mem, true);

    TEST_RELEASE();

failed:

    unit_kv_destroy(helper.kv, true);
    lexbor_mraw_destroy(helper.mraw, true);
    lxb_css_memory_destroy(helper.mem, true);

    return EXIT_FAILURE;
}

static lxb_status_t
parse(helper_t *helper, const char *dir_path)
{
    lxb_status_t status;

    status = lexbor_fs_dir_read((const lxb_char_t *) dir_path,
                                LEXBOR_FS_DIR_OPT_WITHOUT_DIR
                                |LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN,
                                file_callback, helper);

    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to read directory: %s", dir_path);
    }

    return status;
}

static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx)
{
    lxb_status_t status;
    unit_kv_value_t *value;
    helper_t *helper;

    if (filename_len < 5 ||
        strncmp((const char *) &filename[ (filename_len - 4) ], ".ton", 4) != 0)
    {
        return LEXBOR_ACTION_OK;
    }

    helper = ctx;

    TEST_PRINTLN("Parse file: %s", fullpath);

    unit_kv_clean(helper->kv);

    status = unit_kv_parse_file(helper->kv, (const lxb_char_t *) fullpath);
    if (status != LXB_STATUS_OK) {
        lexbor_str_t str = unit_kv_parse_error_as_string(helper->kv);

        TEST_PRINTLN("%s", str.data);

        unit_kv_string_destroy(helper->kv, &str, false);

        return EXIT_FAILURE;
    }

    value = unit_kv_value(helper->kv);
    if (value == NULL) {
        TEST_PRINTLN("Failed to get root value");
        return EXIT_FAILURE;
    }

    TEST_PRINTLN("Check file: %s", fullpath);

    status = check(helper, value);
    if (status != LXB_STATUS_OK) {
        exit(EXIT_FAILURE);
    }

    if (helper->parser != NULL) {
        lxb_css_memory_clean(helper->mem);
        helper->parser = lxb_css_parser_destroy(helper->parser, true);
    }

    return LEXBOR_ACTION_OK;
}

static lxb_status_t
check(helper_t *helper, unit_kv_value_t *value)
{
    lxb_status_t status;
    unit_kv_array_t *entries;

    if (unit_kv_is_array(value) == false) {
        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    entries = unit_kv_array(value);

    for (size_t i = 0; i < entries->length; i++) {
        if (unit_kv_is_hash(entries->list[i]) == false) {
            return print_error(helper, entries->list[i]);
        }

        TEST_PRINTLN("Test #"LEXBOR_FORMAT_Z, (i + 1));

        status = check_entry(helper, entries->list[i], parse_cb);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        lxb_css_parser_clean(helper->parser);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
check_entry(helper_t *helper, unit_kv_value_t *entry, parse_data_cb_f cb)
{
    lxb_status_t status;
    lexbor_str_t *str;
    unit_kv_array_t *entries;
    unit_kv_value_t *data, *results;

    /* Validate */
    data = unit_kv_hash_value_nolen_c(entry, "data");
    if (data == NULL) {
        TEST_PRINTLN("Required parameter missing: data");
        return print_error(helper, entry);
    }

    if (unit_kv_is_string(data) == false) {
        TEST_PRINTLN("Parameter 'data' must be a STRING");
        return print_error(helper, data);
    }

    results = unit_kv_hash_value_nolen_c(entry, "results");
    if (results == NULL) {
        TEST_PRINTLN("Required parameter missing: results");
        return print_error(helper, entry);
    }

    if (unit_kv_is_array(results) == false) {
        TEST_PRINTLN("Parameter 'results' must be an ARRAY");
        return print_error(helper, results);
    }

    /* Parse */

    str = unit_kv_string(data);
    entries = unit_kv_array(results);

    status = cb(helper, str, entries);

    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to CSS");

        return print_error(helper, data);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
parse_cb(helper_t *helper, lexbor_str_t *str, unit_kv_array_t *entries)
{
    lxb_status_t status;
    lxb_css_rule_declaration_list_t *list;

    if (helper->parser != NULL) {
        lxb_css_memory_clean(helper->mem);
        helper->parser = lxb_css_parser_destroy(helper->parser, true);
    }

    helper->parser = lxb_css_parser_create();
    status = lxb_css_parser_init(helper->parser, NULL);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    lxb_css_memory_clean(helper->mem);

    list = lxb_css_declaration_list_parse(helper->parser, helper->mem,
                                          str->data, str->length);
    if (list == NULL) {
        status = helper->parser->status;
        goto failed;
    }

    return compare(helper, entries, list);

failed:

    helper->parser = lxb_css_parser_destroy(helper->parser, true);

    return status;
}

static lxb_status_t
print_error(helper_t *helper, unit_kv_value_t *value)
{
    lexbor_str_t str;

    str = unit_kv_value_position_as_string(helper->kv, value);
    TEST_PRINTLN("%s", str.data);
    unit_kv_string_destroy(helper->kv, &str, false);

    str = unit_kv_value_fragment_as_string(helper->kv, value);
    TEST_PRINTLN("%s", str.data);
    unit_kv_string_destroy(helper->kv, &str, false);

    return LXB_STATUS_ERROR;
}

static lxb_status_t
compare(helper_t *helper, unit_kv_array_t *arr,
        lxb_css_rule_declaration_list_t *list)
{
    bool imp, is_at;
    size_t i = 0;
    uintptr_t ntype;
    lexbor_str_t *str, *str_name, *str_value;
    lxb_status_t status;
    unit_kv_value_t *entry, *type, *name, *value, *important, *at;
    lxb_css_rule_t *rule;

    static const lexbor_str_t str_und = lexbor_str("undef");
    static const lexbor_str_t str_cst = lexbor_str("custom");
    static const lexbor_str_t str_pro = lexbor_str("property");

    if (arr->length != list->count) {
        TEST_PRINTLN("Result expected "LEXBOR_FORMAT_Z" received "LEXBOR_FORMAT_Z,
                     arr->length, list->count);
        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    rule = list->first;

    while (i < arr->length) {
        entry = arr->list[i++];

        if (!unit_kv_is_hash(entry)) {
            return false;
        }

        is_at = false;

        at = unit_kv_hash_value_nolen_c(entry, "at");
        if (at != NULL) {
            if (!unit_kv_is_bool(at)) {
                return LXB_STATUS_ERROR_UNEXPECTED_DATA;
            }

            is_at = unit_kv_bool(at);
        }

        type = unit_kv_hash_value_nolen_c(entry, "type");
        validate_pointer(type, "type");
        if (!unit_kv_is_string(type)) {
            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
        }

        name = unit_kv_hash_value_nolen_c(entry, "name");
        validate_pointer(name, "name");
        if (!unit_kv_is_string(name)) {
            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
        }

        value = unit_kv_hash_value_nolen_c(entry, "value");
        validate_pointer(value, "value");
        if (!unit_kv_is_string(value)) {
            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
        }


        /* Type. */

        str = unit_kv_string(type);

        if (str->length == str_und.length
            && lexbor_str_data_ncmp(str->data, str_und.data, str->length))
        {
            ntype = LXB_CSS_PROPERTY__UNDEF;
        }
        else if (str->length == str_cst.length
                 && lexbor_str_data_ncmp(str->data, str_cst.data, str->length))
        {
            ntype = LXB_CSS_PROPERTY__CUSTOM;
        }
        else if (str->length == str_pro.length
                 && lexbor_str_data_ncmp(str->data, str_pro.data, str->length))
        {
            ntype = LXB_CSS_PROPERTY__LAST_ENTRY;
        }
        else {
            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
        }

        /* Data. */

        str_name = unit_kv_string(name);
        str_value = unit_kv_string(value);

        if (is_at) {
            status = compare_at(helper, rule, entry,
                                (lxb_css_at_rule_type_t) ntype,
                                str_name, str_value);
        }
        else {
            important = unit_kv_hash_value_nolen_c(entry, "important");
            validate_pointer(important, "important");
            if (!unit_kv_is_bool(important)) {
                return LXB_STATUS_ERROR_UNEXPECTED_DATA;
            }

            imp = unit_kv_bool(important);

            status = compare_declr(helper, rule, entry,
                                   (lxb_css_property_type_t) ntype,
                                   str_name, str_value, imp);
        }

        if (status != LXB_STATUS_OK) {
            return status;
        }

        rule = rule->next;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
compare_declr(helper_t *helper, lxb_css_rule_t *rule,
              unit_kv_value_t *entry, lxb_css_property_type_t type,
              lexbor_str_t *name, lexbor_str_t *value, bool important)
{
    lxb_status_t status;
    lexbor_str_t *str = &helper->str;
    lxb_css_rule_declaration_t *declr = lxb_css_rule_declaration(rule);

    if (declr->type != type && declr->type < LXB_CSS_PROPERTY__CUSTOM) {
        TEST_PRINTLN("Type does not match.");
        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    /* Name. */

    lexbor_str_clean(str);

    status = lxb_css_property_serialize_name_str(declr->u.user, declr->type,
                                                 helper->mraw, str);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (str->length != name->length
        || lexbor_str_data_ncmp(str->data, name->data, name->length) == false)
    {
        return print_compare_error("name", name, str);
    }

    lexbor_str_clean(str);

    /* Value. */

    status = lxb_css_property_serialize_str(declr->u.user, declr->type,
                                            helper->mraw, str);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (str->length != value->length
        || lexbor_str_data_ncmp(str->data, value->data, value->length) == false)
    {
        return print_compare_error("value", value, str);
    }

    if (declr->important != important) {
        TEST_PRINTLN("Failed: important");
        TEST_PRINTLN("Need: %s", (important) ? "true" : "false");
        TEST_PRINTLN("Have: %s", (declr->important) ? "true" : "false");

        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
compare_at(helper_t *helper, lxb_css_rule_t *rule,
           unit_kv_value_t *entry, lxb_css_at_rule_type_t type,
           lexbor_str_t *name, lexbor_str_t *value)
{
    lxb_status_t status;
    lexbor_str_t *str = &helper->str;
    lxb_css_rule_at_t *at = lxb_css_rule_at(rule);

    if (at->type != type && at->type < LXB_CSS_AT_RULE__CUSTOM) {
        TEST_PRINTLN("Type does not match.");
        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    /* Name. */

    lexbor_str_clean(str);

    status = lxb_css_at_rule_serialize_name_str(at->u.user, at->type,
                                                helper->mraw, str);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (str->length != name->length
        || lexbor_str_data_ncmp(str->data, name->data, name->length) == false)
    {
        return print_compare_error("name", name, str);
    }

    lexbor_str_clean(str);

    /* Value. */

    status = lxb_css_at_rule_serialize_str(at->u.user, at->type,
                                           helper->mraw, str);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (str->length != value->length
        || lexbor_str_data_ncmp(str->data, value->data, value->length) == false)
    {
        return print_compare_error("value", value, str);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
print_compare_error(const char *name, lexbor_str_t *need, lexbor_str_t *have)
{
    TEST_PRINTLN("Failed: %s", name);
    TEST_PRINTLN("Need: %.*s", (int) need->length, (const char *) need->data);
    TEST_PRINTLN("Have: %.*s", (int) have->length, (const char *) have->data);

    return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
}
