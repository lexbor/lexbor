/*
 * Copyright (C) 2019-2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/core/fs.h>
#include <lexbor/core/array.h>

#include <unit/test.h>
#include <unit/kv.h>

#include <lexbor/css/syntax/tokenizer.h>


typedef struct {
    unit_kv_t                  *kv;
    lxb_css_syntax_tokenizer_t *tkz;
    lexbor_str_t               str;
    lexbor_mraw_t              *mraw;
    lexbor_array_t             tokens;
}
helper_t;

typedef struct {
    const lxb_char_t *begin;
    const lxb_char_t *end;
    lxb_char_t       ch;
}
chunk_ctx_t;

typedef lxb_status_t
(*parse_data_cb_f)(helper_t *helper, lexbor_str_t *str);


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
parse_data_new_tkz_cb(helper_t *helper, lexbor_str_t *str);

static lxb_status_t
parse_data_new_tkz_chunk_cb(helper_t *helper, lexbor_str_t *str);

static lxb_status_t
chunk_cb(lxb_css_syntax_tokenizer_t *tkz, const lxb_char_t **data,
         const lxb_char_t **end, void *ctx);

static lxb_status_t
check_token(helper_t *helper, unit_kv_value_t *entry,
            lxb_css_syntax_token_t *token);

static lxb_status_t
print_error(helper_t *helper, unit_kv_value_t *value);


int
main(int argc, const char * argv[])
{
    lxb_status_t status;
    helper_t helper = {0};
    const char *dir_path;

    if (argc != 2) {
        printf("Usage:\n\tcss_tokenizer <directory path>\n");
        return EXIT_FAILURE;
    }

    dir_path = argv[1];

    TEST_INIT();

    helper.kv = unit_kv_create();
    status = unit_kv_init(helper.kv, 256);

    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    status = lexbor_array_init(&helper.tokens, 4096);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    helper.mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(helper.mraw, 256);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    status = parse(&helper, dir_path);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    TEST_RUN("lexbor/css/tokenizer");

    unit_kv_destroy(helper.kv, true);
    lexbor_array_destroy(&helper.tokens, false);
    lexbor_mraw_destroy(helper.mraw, true);

    TEST_RELEASE();

failed:

    unit_kv_destroy(helper.kv, true);
    lexbor_array_destroy(&helper.tokens, false);
    lexbor_mraw_destroy(helper.mraw, true);

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

    if (helper->tkz != NULL) {
        helper->tkz = lxb_css_syntax_tokenizer_destroy(helper->tkz);
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

        status = check_entry(helper, entries->list[i], parse_data_new_tkz_cb);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        lexbor_array_clean(&helper->tokens);
        lxb_css_syntax_tokenizer_clean(helper->tkz);

        TEST_PRINTLN("Test #"LEXBOR_FORMAT_Z" chunks", (i + 1));

        status = check_entry(helper, entries->list[i],
                             parse_data_new_tkz_chunk_cb);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        lexbor_array_clean(&helper->tokens);
        lxb_css_syntax_tokenizer_clean(helper->tkz);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
check_entry(helper_t *helper, unit_kv_value_t *entry, parse_data_cb_f cb)
{
    lxb_status_t status;
    lexbor_str_t *str;
    unit_kv_array_t *token_entries;
    unit_kv_value_t *data, *tokens;

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

    tokens = unit_kv_hash_value_nolen_c(entry, "tokens");
    if (tokens == NULL) {
        TEST_PRINTLN("Required parameter missing: tokens");

        return print_error(helper, entry);
    }

    if (unit_kv_is_array(tokens) == false) {
        TEST_PRINTLN("Parameter 'tokens' must be an ARRAY");

        return print_error(helper, tokens);
    }

    /* Parse */
    str = unit_kv_string(data);

    status = cb(helper, str);

    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to CSS");

        return print_error(helper, data);
    }

    token_entries = unit_kv_array(tokens);

    if (token_entries->length != helper->tokens.length) {
        TEST_PRINTLN("Expected number of tokens does "
                     "not converge with the received. "
                     "Have: "LEXBOR_FORMAT_Z"; Need: "LEXBOR_FORMAT_Z,
                     helper->tokens.length, token_entries->length);

        return print_error(helper, data);
    }

    for (size_t i = 0; i < token_entries->length; i++) {
        if (unit_kv_is_hash(token_entries->list[i]) == false) {
            print_error(helper, token_entries->list[i]);
            return LXB_STATUS_ERROR;
        }

        status = check_token(helper, token_entries->list[i],
                             helper->tokens.list[i]);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

static void
check_raw(lxb_css_syntax_token_t *token)
{
    volatile lxb_char_t ch;
    const lxb_char_t *p, *end;

    p = lxb_css_syntax_token_base(token)->begin;
    end = lxb_css_syntax_token_base(token)->end;

    while (p < end) {
        ch = *p++;
        ch++;
    }

    if (token->type == LXB_CSS_SYNTAX_TOKEN_DIMENSION) {
        p = lxb_css_syntax_token_dimension_string(token)->base.begin;
        end = lxb_css_syntax_token_dimension_string(token)->base.end;

        while (p < end) {
            ch = *p++;
            ch++;
        }
    }
}

static lxb_status_t
parse_data_new_tkz_cb(helper_t *helper, lexbor_str_t *str)
{
    lxb_status_t status;
    lxb_css_syntax_token_t *token;

    if (helper->tkz != NULL) {
        helper->tkz = lxb_css_syntax_tokenizer_destroy(helper->tkz);
    }

    helper->tkz = lxb_css_syntax_tokenizer_create();
    status = lxb_css_syntax_tokenizer_init(helper->tkz);
    if (status != LXB_STATUS_OK) {
        helper->tkz = lxb_css_syntax_tokenizer_destroy(helper->tkz);

        return status;
    }

    lxb_css_syntax_tokenizer_buffer_set(helper->tkz, str->data, str->length);

    do {
        token = lxb_css_syntax_token_next(helper->tkz);
        if (token == NULL) {
            return helper->tkz->status;
        }

        check_raw(token);

        if (lxb_css_syntax_token_type(token) != LXB_CSS_SYNTAX_TOKEN__EOF) {
            status = lexbor_array_push(&helper->tokens, token);
            if (status != LXB_STATUS_OK) {
                return status;
            }
        }
    }
    while (lxb_css_syntax_token_type(token) != LXB_CSS_SYNTAX_TOKEN__EOF);

    return LXB_STATUS_OK;
}

static lxb_status_t
parse_data_new_tkz_chunk_cb(helper_t *helper, lexbor_str_t *str)
{
    chunk_ctx_t ctx;
    lxb_status_t status;
    lxb_css_syntax_token_t *token;

    if (helper->tkz != NULL) {
        helper->tkz = lxb_css_syntax_tokenizer_destroy(helper->tkz);
    }

    helper->tkz = lxb_css_syntax_tokenizer_create();
    status = lxb_css_syntax_tokenizer_init(helper->tkz);
    if (status != LXB_STATUS_OK) {
        helper->tkz = lxb_css_syntax_tokenizer_destroy(helper->tkz);

        return status;
    }

    ctx.begin = str->data;
    ctx.end = str->data + str->length;
    ctx.ch = *str->data;

    lxb_css_syntax_tokenizer_buffer_set(helper->tkz, &ctx.ch, 1);

    lxb_css_syntax_tokenizer_chunk_cb_set(helper->tkz, chunk_cb, &ctx);

    do {
        token = lxb_css_syntax_token_next(helper->tkz);
        if (token == NULL) {
            return helper->tkz->status;
        }

        if (lxb_css_syntax_token_type(token) != LXB_CSS_SYNTAX_TOKEN__EOF) {
            status = lexbor_array_push(&helper->tokens, token);
            if (status != LXB_STATUS_OK) {
                return status;
            }
        }
    }
    while (lxb_css_syntax_token_type(token) != LXB_CSS_SYNTAX_TOKEN__EOF);

    return LXB_STATUS_OK;
}

static lxb_status_t
chunk_cb(lxb_css_syntax_tokenizer_t *tkz, const lxb_char_t **data,
         const lxb_char_t **end, void *ctx)
{
    chunk_ctx_t *chunk = ctx;

    chunk->begin++;
    chunk->ch = *chunk->begin;

    if (chunk->begin < chunk->end) {
        *data = &chunk->ch;
        *end = *data + 1;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
check_token(helper_t *helper, unit_kv_value_t *entry,
            lxb_css_syntax_token_t *token)
{
    lexbor_str_t *str;
    lxb_status_t status;
    unit_kv_value_t *value;
    lxb_css_syntax_token_type_t type;

    /* Check type */

    value = unit_kv_hash_value_nolen_c(entry, "type");
    if (value == NULL) {
        TEST_PRINTLN("Required parameter missing: type");

        return print_error(helper, entry);
    }

    if (unit_kv_is_string(value) == false) {
        TEST_PRINTLN("Parameter 'type' of token must be a STRING");

        return print_error(helper, value);
    }

    str = unit_kv_string(value);

    type = lxb_css_syntax_token_type_id_by_name(str->data, str->length);

    if (type != token->type) {
        const lxb_char_t *type_name;

        type_name = lxb_css_syntax_token_type_name_by_id(token->type);

        TEST_PRINTLN("Parameter 'type' not match; Have: %s; Need: %s",
                     (char *) type_name, (char *) str->data);

        return print_error(helper, value);
    }

    /* Check value */

    value = unit_kv_hash_value_nolen_c(entry, "value");
    if (value == NULL) {
        return LXB_STATUS_OK;
    }

    if (unit_kv_is_string(value) == false) {
        TEST_PRINTLN("Parameter 'value' of token must be a STRING");

        return print_error(helper, value);
    }

    status = lxb_css_syntax_token_serialize_str(token, &helper->str,
                                                helper->mraw);
    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to serialization token");

        return print_error(helper, value);
    }

    str = unit_kv_string(value);

    if (str->length != helper->str.length
        || lexbor_str_data_ncmp(str->data, helper->str.data, str->length) == false)
    {
        lexbor_str_clean(&helper->str);

        TEST_PRINTLN("Token not match. \nHave:\n%s\nNeed:\n%s",
                     (const char *) helper->str.data, (const char *) str->data);

        return print_error(helper, value);
    }

    lexbor_str_clean(&helper->str);

    return LXB_STATUS_OK;
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
