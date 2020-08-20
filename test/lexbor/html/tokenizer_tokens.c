/*
 * Copyright (C) 2018-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/core/fs.h>

#include "tokenizer_helper.h"


static void
print_error(tokenizer_helper_t *helper, unit_kv_value_t *value);

static lxb_status_t
check(tokenizer_helper_t *helper, unit_kv_value_t *value);

static lxb_status_t
check_entry(tokenizer_helper_t *helper, unit_kv_value_t *entry, bool is_stream);

static lxb_status_t
check_token(tokenizer_helper_t *helper, unit_kv_value_t *entry,
            lxb_html_token_t *token);

static lxb_status_t
check_token_tag(tokenizer_helper_t *helper, unit_kv_value_t *entry,
                lxb_html_token_t *token);

static lxb_status_t
check_token_text(tokenizer_helper_t *helper, unit_kv_value_t *entry,
                 lxb_html_token_t *token);

static lxb_status_t
check_token_type(tokenizer_helper_t *helper, unit_kv_value_t *entry,
                 lxb_html_token_t *token);

static lxb_status_t
check_token_attr(tokenizer_helper_t *helper, unit_kv_value_t *entry,
                 lxb_html_token_t *token);

static lxb_status_t
check_token_attr_param(tokenizer_helper_t *helper, unit_kv_value_t *entry,
                       lexbor_str_t *str, lxb_html_token_attr_t *attr,
                       const char *param, size_t i);

static lxb_status_t
check_errors(tokenizer_helper_t *helper, unit_kv_value_t *value);

static lxb_status_t
check_quirks(tokenizer_helper_t *helper, unit_kv_value_t *value);

tokenizer_helper_t *
init(void);

lxb_status_t
parse(tokenizer_helper_t *helper, const char *dir_path);

void
destroy(tokenizer_helper_t *helper);


static lxb_status_t
check(tokenizer_helper_t *helper, unit_kv_value_t *value)
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
            print_error(helper, entries->list[i]);
            return LXB_STATUS_ERROR;
        }

        status = check_entry(helper, entries->list[i], false);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        status = check_entry(helper, entries->list[i], true);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
check_entry(tokenizer_helper_t *helper, unit_kv_value_t *entry, bool is_stream)
{
    lxb_status_t status;
    lexbor_str_t *str;
    unit_kv_array_t *token_entries;
    unit_kv_value_t *data, *tokens, *errors, *quirks;
    lxb_html_token_t *token;

    /* Validate */
    data = unit_kv_hash_value_nolen_c(entry, "data");
    if (data == NULL) {
        TEST_PRINTLN("Required parameter missing: data");
        print_error(helper, entry);

        return LXB_STATUS_ERROR;
    }

    if (unit_kv_is_string(data) == false) {
        TEST_PRINTLN("Parameter 'data' must be STRING");
        print_error(helper, data);

        return LXB_STATUS_ERROR;
    }

    tokens = unit_kv_hash_value_nolen_c(entry, "tokens");
    if (tokens == NULL) {
        TEST_PRINTLN("Required parameter missing: tokens");
        print_error(helper, entry);

        return LXB_STATUS_ERROR;
    }

    if (unit_kv_is_array(tokens) == false) {
        TEST_PRINTLN("Parameter 'tokens' must be ARRAY");
        print_error(helper, tokens);

        return LXB_STATUS_ERROR;
    }

    /* Parse */
    str = unit_kv_string(data);

    if (is_stream) {
        status = tokenizer_helper_tkz_parse_stream(helper->tkz,
                                                   str->data, str->length);
    }
    else {
        status = tokenizer_helper_tkz_parse(helper->tkz,
                                            str->data, str->length);
    }

    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to parse HTML");
        print_error(helper, data);

        return status;
    }

    token_entries = unit_kv_array(tokens);

    if (token_entries->length != helper->tokens.length) {
        TEST_PRINTLN("Expected number of tokens does "
                     "not converge with the received. "
                     "Have: "LEXBOR_FORMAT_Z"; Need: "LEXBOR_FORMAT_Z,
                     helper->tokens.length, token_entries->length);

        print_error(helper, data);

        return LXB_STATUS_ERROR;
    }

    for (size_t i = 0; i < token_entries->length; i++) {
        token = helper->tokens.list[i];

        if (unit_kv_is_hash(token_entries->list[i]) == false) {
            print_error(helper, token_entries->list[i]);
            return LXB_STATUS_ERROR;
        }

        status = check_token(helper, token_entries->list[i], token);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    quirks = unit_kv_hash_value_nolen_c(entry, "quirks");

    status = check_quirks(helper, quirks);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    errors = unit_kv_hash_value_nolen_c(entry, "errors");
    return check_errors(helper, errors);
}

static lxb_status_t
check_token(tokenizer_helper_t *helper, unit_kv_value_t *entry,
            lxb_html_token_t *token)
{
    lxb_status_t status;

    status = check_token_tag(helper, entry, token);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = check_token_text(helper, entry, token);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = check_token_type(helper, entry, token);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = check_token_attr(helper, entry, token);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
check_token_tag(tokenizer_helper_t *helper, unit_kv_value_t *entry,
                lxb_html_token_t *token)
{
    lexbor_str_t *str;
    unit_kv_value_t *value;
    lxb_tag_id_t tag_id;

    value = unit_kv_hash_value_nolen_c(entry, "tag");
    if (value == NULL) {
        return LXB_STATUS_OK;
    }

    if (unit_kv_is_string(value) == false) {
        TEST_PRINTLN("Parameter 'tag' must be STRING");
        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    str = unit_kv_string(value);

    tag_id = lxb_tag_id_by_name(helper->tkz->tags, str->data, str->length);
    if (token->tag_id != tag_id) {
        TEST_PRINTLN("Parameter 'tag' not match");
        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
check_token_text(tokenizer_helper_t *helper, unit_kv_value_t *entry,
                 lxb_html_token_t *token)
{
    size_t length;
    lexbor_str_t *str;
    unit_kv_value_t *value;

    value = unit_kv_hash_value_nolen_c(entry, "text");
    if (value == NULL) {
        return LXB_STATUS_OK;
    }

    if (unit_kv_is_string(value) == false) {
        TEST_PRINTLN("Parameter 'text' must be STRING");
        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    str = unit_kv_string(value);

    length = token->text_end - token->text_start;

    if (length != str->length ||
        lexbor_str_data_ncasecmp(token->text_start, str->data, length) == false)
    {
        TEST_PRINTLN("Token text not match. \n"
                     "Have:\n%.*s\nNeed:\n%s",
                     (int) length, (const char *) token->text_start,
                     (const char *) str->data);

        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
check_token_type(tokenizer_helper_t *helper, unit_kv_value_t *entry,
                 lxb_html_token_t *token)
{
    lexbor_str_t *str;
    unit_kv_value_t *value;
    unit_kv_array_t *type_entries;
    lexbor_bst_map_entry_t *bm_entry;

    lxb_html_token_type_t need_type = 0;

    value = unit_kv_hash_value_nolen_c(entry, "type");
    if (value == NULL) {
        return LXB_STATUS_OK;
    }

    if (unit_kv_is_array(value) == false) {
        TEST_PRINTLN("Parameter 'type' must be ARRAY");
        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    type_entries = unit_kv_array(value);

    for (size_t i = 0; i < type_entries->length; i++) {
        if (unit_kv_is_chars(type_entries->list[i]) == false) {
            TEST_PRINTLN("Entries in 'type' parameter must be CHARS");
            print_error(helper, type_entries->list[i]);

            return LXB_STATUS_ERROR;
        }

        str = unit_kv_string(type_entries->list[i]);

        bm_entry = lexbor_bst_map_search(helper->map, helper->types_root,
                                         str->data, str->length);
        if (bm_entry == NULL) {
            TEST_PRINTLN("Entry '%s' from 'type' parameter not found",
                         (const char *) str->data);

            print_error(helper, type_entries->list[i]);

            return LXB_STATUS_ERROR;
        }

        need_type |= (lxb_html_token_type_t) (uintptr_t) bm_entry->value;
    }

    if (need_type != token->type) {
        TEST_PRINTLN("The 'type' parameter did not match. Have: %d; Need: %d",
                     token->type, need_type);

        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
check_token_attr(tokenizer_helper_t *helper, unit_kv_value_t *entry,
                 lxb_html_token_t *token)
{
    lexbor_str_t attr_name, attr_value;
    unit_kv_value_t *value;
    unit_kv_array_t *attr_entries;
    lxb_html_token_attr_t *attr;

    lxb_status_t status = LXB_STATUS_OK;

    value = unit_kv_hash_value_nolen_c(entry, "attr");
    if (value == NULL) {
        return LXB_STATUS_OK;
    }

    if (unit_kv_is_array(value) == false) {
        TEST_PRINTLN("Parameter 'attr' must be ARRAY");
        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    attr_entries = unit_kv_array(value);
    attr = token->attr_first;

    if (attr_entries->length == 0 && attr == NULL) {
        return LXB_STATUS_OK;
    }

    for (size_t i = 0; i < attr_entries->length; i++) {
        if (attr == NULL) {
            TEST_PRINTLN("Attributes count not match. "
                         "Have: "LEXBOR_FORMAT_Z"; Need: "LEXBOR_FORMAT_Z,
                         i, attr_entries->length);

            print_error(helper, value);

            return LXB_STATUS_ERROR;
        }

        if (unit_kv_is_hash(attr_entries->list[i]) == false) {
            TEST_PRINTLN("Entries in 'attr' parameter must be HASH");
            print_error(helper, attr_entries->list[i]);

            return LXB_STATUS_ERROR;
        }

        attr_name.data = (lxb_char_t *) lxb_html_token_attr_name(attr, &attr_name.length);

        attr_value.data = attr->value;
        attr_value.length = attr->value_size;

        status = check_token_attr_param(helper, attr_entries->list[i],
                                        &attr_name, attr, "name", i);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        status = check_token_attr_param(helper, attr_entries->list[i],
                                        &attr_value, attr, "value", i);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        attr = attr->next;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
check_token_attr_param(tokenizer_helper_t *helper, unit_kv_value_t *entry,
                       lexbor_str_t *str, lxb_html_token_attr_t *attr,
                       const char *param, size_t i)
{
    unit_kv_value_t *name;
    lexbor_str_t *val_str;

    name = unit_kv_hash_value_nolen_c(entry, param);
    if (name == NULL) {
        if (str->length != 0) {
            TEST_PRINTLN("%s in "LEXBOR_FORMAT_Z" attribute not match",
                         param, i);

            print_error(helper, entry);

            return LXB_STATUS_ERROR;
        }
    }
    else {
        if (unit_kv_is_null(name) == true) {
            if (strcmp(param, "name") == 0) {
                if (attr->name_begin == NULL && attr->name_end == NULL) {
                    return LXB_STATUS_OK;
                }
            }
            else {
                if (attr->value_begin == NULL && attr->value_end == NULL) {
                    return LXB_STATUS_OK;
                }
            }

            TEST_PRINTLN("Parameter '%s' in attribute not match. \n"
                         "Have: not NULL; Need: NULL", param);

            print_error(helper, name);

            return LXB_STATUS_ERROR;
        }

        if (unit_kv_is_string(name) == false) {
            TEST_PRINTLN("Parameter '%s' in attribute must be STRING", param);
            print_error(helper, name);

            return LXB_STATUS_ERROR;
        }

        val_str = unit_kv_string(name);

        if (strcmp(param, "name") == 0) {
            if (attr->name_begin == NULL && attr->name_end == NULL) {
                goto error;
            }
        }
        else {
            if (attr->value_begin == NULL && attr->value_end == NULL) {
                goto error;
            }
        }

        if (val_str->length != str->length ||
            lexbor_str_data_ncasecmp(val_str->data, str->data, str->length) == false)
        {
            TEST_PRINTLN("Parameter '%s' in attribute not match. \n"
                         "Have: %.*s; Need: %s", param,
                         (int) str->length, (const char *) str->data,
                         (const char *) val_str->data);

            print_error(helper, name);

            return LXB_STATUS_ERROR;
        }
    }

    return LXB_STATUS_OK;

error:

    TEST_PRINTLN("Parameter '%s' in attribute not match. \n"
                 "Have: NULL; Need: %s", param, (const char *) val_str->data);

    print_error(helper, name);

    return LXB_STATUS_ERROR;
}

static lxb_status_t
check_errors(tokenizer_helper_t *helper, unit_kv_value_t *value)
{
    lexbor_str_t *str;
    unit_kv_array_t *entries;
    lexbor_bst_map_entry_t *bm_entry;
    lxb_html_tokenizer_error_t *error;

    if (value == NULL) {
        return LXB_STATUS_OK;
    }

    if (unit_kv_is_array(value) == false) {
        TEST_PRINTLN("Parameter 'errors' must be ARRAY");
        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    size_t tkz_err_len = lexbor_array_obj_length(helper->tkz->parse_errors);

    entries = unit_kv_array(value);

    if (entries->length != 0 && (entries->length != tkz_err_len)) {
        TEST_PRINTLN("Entries count in 'errors' parameter not match with "
                     "tokenizer 'parse_errors' count");

        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    for (size_t i = 0; i < entries->length; i++) {
        if (unit_kv_is_chars(entries->list[i]) == false) {
            TEST_PRINTLN("Entries in 'errors' parameter must be CHARS");
            print_error(helper, entries->list[i]);

            return LXB_STATUS_ERROR;
        }

        str = unit_kv_string(entries->list[i]);

        bm_entry = lexbor_bst_map_search(helper->map, helper->errors_root,
                                         str->data, str->length);
        if (bm_entry == NULL) {
            TEST_PRINTLN("Entry '%s' from 'errors' parameter not found",
                         (const char *) str->data);

            print_error(helper, entries->list[i]);

            return LXB_STATUS_ERROR;
        }

        error = lexbor_array_obj_get(helper->tkz->parse_errors, i);
        if (error == NULL) {
            TEST_PRINTLN("NULL value in tokenizator 'parse_errors' list: "
                         "in position "LEXBOR_FORMAT_Z, i);

            print_error(helper, entries->list[i]);

            return LXB_STATUS_ERROR;
        }

        if (error->id != (lxb_html_tokenizer_error_id_t) bm_entry->value) {
            TEST_PRINTLN("The 'errors' parameter did not match. "
                         "Have: %d; Need: %d", error->id,
                         (lxb_html_tokenizer_error_id_t) bm_entry->value);

            print_error(helper, value);

            return LXB_STATUS_ERROR;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
check_quirks(tokenizer_helper_t *helper, unit_kv_value_t *value)
{
    lexbor_str_t *str;
    char *val;

    if (value == NULL) {
        return LXB_STATUS_OK;
    }

    if (unit_kv_is_chars(value) == false) {
        TEST_PRINTLN("Parameter 'quirks' must be CHARS");
        print_error(helper, value);

        return LXB_STATUS_ERROR;
    }

    str = unit_kv_string(value);

    val = "LXB_HTML_COMPAT_MODE_NO_QUIRKS";
    if (lexbor_str_data_cmp((const lxb_char_t *) val, str->data)) {
        return LXB_STATUS_OK;
    }

    val = "LXB_HTML_COMPAT_MODE_QUIRKS";
    if (lexbor_str_data_cmp((const lxb_char_t *) val, str->data)) {
        return LXB_STATUS_OK;
    }

    val = "LXB_HTML_COMPAT_MODE_LIMITED_QUIRKS";
    if (lexbor_str_data_cmp((const lxb_char_t *) val, str->data)) {
        return LXB_STATUS_OK;
    }

    TEST_PRINTLN("The 'quirks' parameter did not match.");

    print_error(helper, value);

    return LXB_STATUS_ERROR;
}

static void
print_error(tokenizer_helper_t *helper, unit_kv_value_t *value)
{
    lexbor_str_t str;

    str = unit_kv_value_position_as_string(helper->kv, value);
    TEST_PRINTLN("%s", str.data);
    unit_kv_string_destroy(helper->kv, &str, false);

    str = unit_kv_value_fragment_as_string(helper->kv, value);
    TEST_PRINTLN("%s", str.data);
    unit_kv_string_destroy(helper->kv, &str, false);
}

static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx)
{
    unit_kv_value_t *value;
    tokenizer_helper_t *helper;

    if (filename_len < 5 ||
        strncmp((const char *) &filename[ (filename_len - 4) ], ".ton", 4) != 0)
    {
        return LEXBOR_ACTION_OK;
    }

    helper = ctx;

    TEST_PRINTLN("Parse file: %s", fullpath);

    unit_kv_clean(helper->kv);

    helper->status = unit_kv_parse_file(helper->kv,
                                        (const lxb_char_t *) fullpath);
    if (helper->status != LXB_STATUS_OK) {
        lexbor_str_t str = unit_kv_parse_error_as_string(helper->kv);

        TEST_PRINTLN("%s", str.data);

        unit_kv_string_destroy(helper->kv, &str, false);

        return LEXBOR_ACTION_STOP;
    }

    value = unit_kv_value(helper->kv);
    if (value == NULL) {
        helper->status = LXB_STATUS_ERROR;

        return LEXBOR_ACTION_STOP;
    }

    TEST_PRINTLN("Check file: %s", fullpath);

    helper->status = check(helper, value);
    if (helper->status != LXB_STATUS_OK) {
        return LEXBOR_ACTION_STOP;
    }

    return LEXBOR_ACTION_OK;
}

tokenizer_helper_t *
init(void)
{
    return tokenizer_helper_make();
}

lxb_status_t
parse(tokenizer_helper_t *helper, const char *dir_path)
{
    lxb_status_t status;

    status = lexbor_fs_dir_read((const lxb_char_t *) dir_path,
                                LEXBOR_FS_DIR_OPT_WITHOUT_DIR
                                |LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN,
                                file_callback, helper);

    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to read directory: %s", dir_path);

        return status;
    }

    return helper->status;
}

void
destroy(tokenizer_helper_t *helper)
{
    tokenizer_helper_destroy(helper);
}

int
main(int argc, const char * argv[])
{
    if (argc != 2) {
        printf("Usage:\n\ttokenizer_tokens <directory path>\n");
        return EXIT_FAILURE;
    }

    tokenizer_helper_t *helper = NULL;

    TEST_INIT();

    helper = init();
    if (helper == NULL) {
        TEST_PRINTLN("Failed to allocate memory for helper");
        goto failed;
    }

    if (parse(helper, argv[1]) != LXB_STATUS_OK) {
        goto failed;
    }

    destroy(helper);

    TEST_RUN("lexbor/html/tokenizer_tokens");
    TEST_RELEASE();

failed:

    destroy(helper);

    return EXIT_FAILURE;
}
