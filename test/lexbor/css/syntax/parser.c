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
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA); \
    }


typedef struct {
    unit_kv_t        *kv;
    lxb_css_parser_t *parser;
    lexbor_str_t     str;
    lexbor_mraw_t    *mraw;

    unit_kv_array_t  *entries;
    size_t           idx;

    size_t           i;
    size_t           prelude;
    size_t           block;
    bool             name;
    bool             value;
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

static bool
css_list_rules_state(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx);

static bool
css_list_rules_next(lxb_css_parser_t *parser,
                    const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_list_rules_end(lxb_css_parser_t *parser,
                   const lxb_css_syntax_token_t *token,
                   void *ctx, bool failed);

static bool
css_at_rule_state(lxb_css_parser_t *parser,
                  const lxb_css_syntax_token_t *token, void *ctx);

static bool
css_at_rule_block(lxb_css_parser_t *parser,
                  const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_at_rule_end(lxb_css_parser_t *parser,
                const lxb_css_syntax_token_t *token,
                void *ctx, bool failed);

static bool
css_qualified_rule_state(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token, void *ctx);

static bool
css_qualified_rule_block(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token, void *ctx);

static bool
css_qualified_rule_back(lxb_css_parser_t *parser,
                        const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_qualified_rule_end(lxb_css_parser_t *parser,
                       const lxb_css_syntax_token_t *token,
                       void *ctx, bool failed);

static bool
css_declarations_name(lxb_css_parser_t *parser,
                      const lxb_css_syntax_token_t *token, void *ctx);

static bool
css_declarations_value(lxb_css_parser_t *parser,
                       const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_declaration_end(lxb_css_parser_t *parser, void *ctx,
                    bool important, bool failed);

static lxb_status_t
css_declarations_end(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token,
                     void *ctx, bool failed);

static bool
css_declarations_at_rule_state(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx);

static bool
css_declarations_at_rule_block(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_declarations_at_rule_end(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, bool failed);

static bool
css_declarations_bad(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
print_error(helper_t *helper, unit_kv_value_t *value);

static bool
validate_type(unit_kv_value_t *value, const lexbor_str_t *need);

static bool
compare(lxb_css_parser_t *parser, helper_t *helper, unit_kv_array_t *arr,
        const lxb_css_syntax_token_t *token, size_t *out_length);


static const lxb_css_syntax_cb_at_rule_t css_at_rule = {
    .state = css_at_rule_state,
    .block = css_at_rule_block,
    .failed = lxb_css_state_failed,
    .end = css_at_rule_end
};

static const lxb_css_syntax_cb_qualified_rule_t css_qualified_rule = {
    .state = css_qualified_rule_state,
    .block = css_qualified_rule_block,
    .failed = lxb_css_state_failed,
    .end = css_qualified_rule_end
};

static const lxb_css_syntax_cb_list_rules_t css_list_rules = {
    .cb.state = css_list_rules_state,
    .cb.failed = lxb_css_state_failed,
    .cb.end = css_list_rules_end,
    .next = css_list_rules_next,
    .at_rule = &css_at_rule,
    .qualified_rule = &css_qualified_rule
};

static const lxb_css_syntax_cb_at_rule_t css_declarations_at_rule = {
    .state = css_declarations_at_rule_state,
    .block = css_declarations_at_rule_block,
    .failed = lxb_css_state_failed,
    .end = css_declarations_at_rule_end
};

static const lxb_css_syntax_cb_declarations_t css_declarations = {
    .cb.state = css_declarations_name,
    .cb.block = css_declarations_value,
    .cb.failed = css_declarations_bad,
    .cb.end = css_declarations_end,
    .declaration_end = css_declaration_end,
    .at_rule = &css_declarations_at_rule
};

static const lexbor_str_t at_rule = lexbor_str("at-rule");
static const lexbor_str_t qualified_rule = lexbor_str("qualified-rule");


int
main(int argc, const char * argv[])
{
    lxb_status_t status;
    helper_t helper = {0};
    const char *dir_path;

    if (argc != 2) {
        printf("Usage:\n\tcss_parser <directory path>\n");
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

    status = parse(&helper, dir_path);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    TEST_RUN("lexbor/css/syntax/parser");

    unit_kv_destroy(helper.kv, true);
    lexbor_mraw_destroy(helper.mraw, true);

    TEST_RELEASE();

failed:

    unit_kv_destroy(helper.kv, true);
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

    if (helper->parser != NULL) {
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
    token_entries = unit_kv_array(tokens);

    status = cb(helper, str, token_entries);

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

    if (helper->parser != NULL) {
        helper->parser = lxb_css_parser_destroy(helper->parser, true);
    }

    helper->parser = lxb_css_parser_create();
    status = lxb_css_parser_init(helper->parser, NULL);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    helper->entries = entries;
    helper->idx = 0;

    status = lxb_css_syntax_parse_list_rules(helper->parser, &css_list_rules,
                                             str->data, str->length,
                                             helper, true);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    return LXB_STATUS_OK;

failed:

    helper->parser = lxb_css_parser_destroy(helper->parser, true);

    return status;
}

static bool
css_list_rules_state(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_parser_success(parser);
}

static bool
css_list_rules_next(lxb_css_parser_t *parser,
                    const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_list_rules_end(lxb_css_parser_t *parser,
                   const lxb_css_syntax_token_t *token,
                   void *ctx, bool failed)
{
    return LXB_STATUS_OK;
}

static bool
css_at_rule_state(lxb_css_parser_t *parser,
                  const lxb_css_syntax_token_t *token, void *ctx)
{
    unit_kv_array_t *arr;

    helper_t *helper = ctx;
    unit_kv_value_t *entry = helper->entries->list[helper->idx];

    unit_kv_value_t *type = unit_kv_hash_value_nolen_c(entry, "type");
    unit_kv_value_t *prelude = unit_kv_hash_value_nolen_c(entry, "prelude");

    validate_pointer(type, "type");
    validate_pointer(prelude, "prelude");

    if (!validate_type(type, &at_rule)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    if (!unit_kv_is_array(prelude)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    arr = unit_kv_array(prelude);

    if (!compare(parser, helper, arr, token, &helper->prelude)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    return lxb_css_parser_success(parser);
}

static bool
css_at_rule_block(lxb_css_parser_t *parser,
                  const lxb_css_syntax_token_t *token, void *ctx)
{
    unit_kv_array_t *arr;

    helper_t *helper = ctx;
    unit_kv_value_t *entry = helper->entries->list[helper->idx];

    unit_kv_value_t *type = unit_kv_hash_value_nolen_c(entry, "type");
    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");

    validate_pointer(type, "type");
    validate_pointer(block, "block");

    /* Check for empty block. */

    if (token->type == LXB_CSS_SYNTAX_TOKEN__END) {
        return lxb_css_parser_success(parser);
    }

    if (!validate_type(type, &at_rule)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    if (!unit_kv_is_array(block)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    arr = unit_kv_array(block);

    if (!compare(parser, helper, arr, token, &helper->block)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_at_rule_end(lxb_css_parser_t *parser,
                const lxb_css_syntax_token_t *token,
                void *ctx, bool failed)
{
    helper_t *helper = ctx;
    unit_kv_value_t *entry = helper->entries->list[helper->idx];

    unit_kv_value_t *prelude = unit_kv_hash_value_nolen_c(entry, "prelude");
    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");

    if ((prelude == NULL && helper->prelude != 0)
        || (block == NULL && helper->block != 0))
    {
        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    if (unit_kv_array(prelude)->length != helper->prelude
        || unit_kv_array(block)->length != helper->block)
    {
        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    helper->idx++;
    helper->prelude = 0;
    helper->block = 0;

    return LXB_STATUS_OK;
}

static bool
css_qualified_rule_state(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token, void *ctx)
{
    unit_kv_array_t *arr;

    helper_t *helper = ctx;
    unit_kv_value_t *entry = helper->entries->list[helper->idx];

    unit_kv_value_t *type = unit_kv_hash_value_nolen_c(entry, "type");
    unit_kv_value_t *prelude = unit_kv_hash_value_nolen_c(entry, "prelude");

    validate_pointer(type, "type");
    validate_pointer(prelude, "prelude");

    if (!validate_type(type, &qualified_rule)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    if (!unit_kv_is_array(prelude)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    arr = unit_kv_array(prelude);

    if (!compare(parser, helper, arr, token, &helper->prelude)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    return lxb_css_parser_success(parser);
}

static bool
css_qualified_rule_block(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token, void *ctx)
{
    helper_t *helper = ctx;
    lxb_css_syntax_rule_t *rule;

    /* Check for empty block. */

    if (token->type == LXB_CSS_SYNTAX_TOKEN__END) {
        return lxb_css_parser_success(parser);
    }

    helper->i = 0;
    helper->name = false;
    helper->value = false;

    rule = lxb_css_syntax_parser_declarations_push(parser, token,
                                                   css_qualified_rule_back,
                                                   &css_declarations, ctx,
                                                   LXB_CSS_SYNTAX_TOKEN_RC_BRACKET);
    if (rule == NULL) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    return true;
}

static bool
css_qualified_rule_back(lxb_css_parser_t *parser,
                        const lxb_css_syntax_token_t *token, void *ctx)
{
    if (token->type == LXB_CSS_SYNTAX_TOKEN__END) {
        return lxb_css_parser_success(parser);
    }

    /* Error */

    lxb_css_syntax_parser_consume(parser);

    return true;
}

static lxb_status_t
css_qualified_rule_end(lxb_css_parser_t *parser,
                       const lxb_css_syntax_token_t *token,
                       void *ctx, bool failed)
{
    helper_t *helper = ctx;

    helper->idx++;
    helper->prelude = 0;
    helper->block = 0;

    return LXB_STATUS_OK;
}

static bool
css_declarations_name(lxb_css_parser_t *parser,
                      const lxb_css_syntax_token_t *token, void *ctx)
{
    lexbor_str_t *str;
    lxb_status_t status;
    unit_kv_array_t *arr;
    unit_kv_value_t *name;

    helper_t *helper = ctx;
    unit_kv_value_t *entry = helper->entries->list[helper->idx];

    unit_kv_value_t *type = unit_kv_hash_value_nolen_c(entry, "type");
    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");

    validate_pointer(type, "type");
    validate_pointer(block, "block");

    if (!validate_type(type, &qualified_rule)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    if (!unit_kv_is_array(block)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    arr = unit_kv_array(block);

    if (!unit_kv_is_hash(arr->list[helper->i])) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    name = unit_kv_hash_value_nolen_c(arr->list[helper->i], "name");

    validate_pointer(name, "name");

    if (!unit_kv_is_string(name)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    str = unit_kv_string(name);

    status = lxb_css_syntax_token_serialize_str(token, &helper->str,
                                                helper->mraw);
    if (status != LXB_STATUS_OK) {
        return false;
    }

    if (str->length != helper->str.length
        || lexbor_str_data_ncmp(str->data,
                                helper->str.data, str->length) == false)
    {
        lexbor_str_clean(&helper->str);
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    lexbor_str_clean(&helper->str);

    lxb_css_syntax_parser_consume(parser);

    helper->name = true;

    return lxb_css_parser_success(parser);
}

static bool
css_declarations_value(lxb_css_parser_t *parser,
                       const lxb_css_syntax_token_t *token, void *ctx)
{
    lexbor_str_t *str;
    lxb_status_t status;
    unit_kv_array_t *arr;
    unit_kv_value_t *value;

    helper_t *helper = ctx;
    unit_kv_value_t *entry = helper->entries->list[helper->idx];

    unit_kv_value_t *type = unit_kv_hash_value_nolen_c(entry, "type");
    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");

    validate_pointer(type, "type");
    validate_pointer(block, "block");

    if (!validate_type(type, &qualified_rule)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    if (!unit_kv_is_array(block)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    arr = unit_kv_array(block);

    if (!unit_kv_is_hash(arr->list[helper->i])) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    value = unit_kv_hash_value_nolen_c(arr->list[helper->i], "value");

    validate_pointer(value, "value");

    if (!unit_kv_is_string(value)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    str = unit_kv_string(value);

    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        status = lxb_css_syntax_token_serialize_str(token, &helper->str,
                                                    helper->mraw);
        if (status != LXB_STATUS_OK) {
            return lxb_css_parser_fail(parser,
                                       LXB_STATUS_ERROR_UNEXPECTED_DATA);
        }

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    if (str->length != helper->str.length
        || lexbor_str_data_ncmp(str->data,
                                helper->str.data, str->length) == false)
    {
        lexbor_str_clean(&helper->str);
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    lexbor_str_clean(&helper->str);

    helper->value = true;

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_declaration_end(lxb_css_parser_t *parser, void *ctx,
                    bool important, bool failed)
{
    unit_kv_array_t *arr;
    unit_kv_value_t *imp, *name, *value;

    helper_t *helper = ctx;
    unit_kv_value_t *entry = helper->entries->list[helper->idx];
    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");

    if (!unit_kv_is_array(block)) {
        return LXB_STATUS_ERROR_UNEXPECTED_DATA;
    }

    arr = unit_kv_array(block);

    if (!unit_kv_is_hash(arr->list[helper->i])) {
        return LXB_STATUS_ERROR_UNEXPECTED_DATA;
    }

    imp = unit_kv_hash_value_nolen_c(arr->list[helper->i], "important");

    if (imp != NULL) {
        if (!unit_kv_is_bool(imp)) {
            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
        }

        if (unit_kv_bool(imp) != important) {
            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
        }
    }

    name = unit_kv_hash_value_nolen_c(arr->list[helper->i], "name");
    value = unit_kv_hash_value_nolen_c(arr->list[helper->i], "value");

    if (name != NULL && !helper->name) {
        TEST_FAILURE("Falied name");
    }

    if (name == NULL && helper->name) {
        TEST_FAILURE("Falied name");
    }

    if (value != NULL && !helper->value) {
        TEST_FAILURE("Falied value");
    }

    if (value == NULL && helper->value) {
        TEST_FAILURE("Falied value");
    }

    helper->i++;
    helper->name = false;
    helper->value = false;

    return LXB_STATUS_OK;
}

static lxb_status_t
css_declarations_end(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token,
                     void *ctx, bool failed)
{
    unit_kv_array_t *arr;

    helper_t *helper = ctx;
    unit_kv_value_t *entry = helper->entries->list[helper->idx];

    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");

    if (!unit_kv_is_array(block)) {
        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    arr = unit_kv_array(block);

    if (arr->length != helper->i) {
        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    return LXB_STATUS_OK;
}

static bool
css_declarations_at_rule_state(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx)
{
    lexbor_str_t *str;
    lxb_status_t status;
    unit_kv_array_t *arr;
    unit_kv_value_t *prelude;

    helper_t *helper = ctx;
    unit_kv_value_t *entry = helper->entries->list[helper->idx];

    unit_kv_value_t *type = unit_kv_hash_value_nolen_c(entry, "type");
    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");

    validate_pointer(type, "type");
    validate_pointer(block, "block");

    if (!validate_type(type, &qualified_rule)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    if (!unit_kv_is_array(block)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    arr = unit_kv_array(block);

    if (!unit_kv_is_hash(arr->list[helper->i])) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    prelude = unit_kv_hash_value_nolen_c(arr->list[helper->i], "prelude");

    validate_pointer(prelude, "prelude");

    if (!unit_kv_is_string(prelude)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    str = unit_kv_string(prelude);

    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        status = lxb_css_syntax_token_serialize_str(token, &helper->str,
                                                    helper->mraw);
        if (status != LXB_STATUS_OK) {
            return lxb_css_parser_fail(parser,
                                       LXB_STATUS_ERROR_UNEXPECTED_DATA);
        }

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    if (str->length != helper->str.length
        || lexbor_str_data_ncmp(str->data,
                                helper->str.data, str->length) == false)
    {
        lexbor_str_clean(&helper->str);
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    lexbor_str_clean(&helper->str);

    return lxb_css_parser_success(parser);
}

static bool
css_declarations_at_rule_block(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx)
{
    lexbor_str_t *str;
    lxb_status_t status;
    unit_kv_array_t *arr;

    helper_t *helper = ctx;
    unit_kv_value_t *entry = helper->entries->list[helper->idx];

    unit_kv_value_t *type = unit_kv_hash_value_nolen_c(entry, "type");
    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");

    /* Check for empty block. */

    if (token->type == LXB_CSS_SYNTAX_TOKEN__END) {
        return lxb_css_parser_success(parser);
    }

    validate_pointer(type, "type");
    validate_pointer(block, "block");

    if (!validate_type(type, &qualified_rule)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    if (!unit_kv_is_array(block)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    arr = unit_kv_array(block);

    if (!unit_kv_is_hash(arr->list[helper->i])) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    block = unit_kv_hash_value_nolen_c(arr->list[helper->i], "block");

    validate_pointer(block, "block");

    if (!unit_kv_is_string(block)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    str = unit_kv_string(block);

    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        status = lxb_css_syntax_token_serialize_str(token, &helper->str,
                                                    helper->mraw);
        if (status != LXB_STATUS_OK) {
            return lxb_css_parser_fail(parser,
                                       LXB_STATUS_ERROR_UNEXPECTED_DATA);
        }

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    if (str->length != helper->str.length
        || lexbor_str_data_ncmp(str->data,
                                helper->str.data, str->length) == false)
    {
        lexbor_str_clean(&helper->str);
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    lexbor_str_clean(&helper->str);

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_declarations_at_rule_end(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, bool failed)
{
    helper_t *helper = ctx;

    helper->i++;

    return LXB_STATUS_OK;
}

static bool
css_declarations_bad(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx)
{
    lexbor_str_t *str;
    lxb_status_t status;
    unit_kv_array_t *arr;
    unit_kv_value_t *bad;

    helper_t *helper = ctx;
    unit_kv_value_t *entry = helper->entries->list[helper->idx];

    unit_kv_value_t *type = unit_kv_hash_value_nolen_c(entry, "type");
    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");

    validate_pointer(type, "type");
    validate_pointer(block, "block");

    if (!validate_type(type, &qualified_rule)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    if (!unit_kv_is_array(block)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    arr = unit_kv_array(block);

    if (!unit_kv_is_hash(arr->list[helper->i])) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    bad = unit_kv_hash_value_nolen_c(arr->list[helper->i], "bad");

    validate_pointer(bad, "bad");

    if (!unit_kv_is_string(bad)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    str = unit_kv_string(bad);

    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        status = lxb_css_syntax_token_serialize_str(token, &helper->str,
                                                    helper->mraw);
        if (status != LXB_STATUS_OK) {
            return lxb_css_parser_fail(parser,
                                       LXB_STATUS_ERROR_UNEXPECTED_DATA);
        }

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    if (str->length != helper->str.length
        || lexbor_str_data_ncmp(str->data,
                                helper->str.data, str->length) == false)
    {
        lexbor_str_clean(&helper->str);
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    lexbor_str_clean(&helper->str);

    return lxb_css_parser_success(parser);
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

static bool
validate_type(unit_kv_value_t *value, const lexbor_str_t *need)
{
    lexbor_str_t *type;

    if (!unit_kv_is_string(value)) {
        return false;
    }

    type = unit_kv_string(value);

    if (need->length != type->length) {
        return false;
    }

    return lexbor_str_data_ncmp(need->data, type->data, type->length);
}

static bool
compare(lxb_css_parser_t *parser, helper_t *helper, unit_kv_array_t *arr,
        const lxb_css_syntax_token_t *token, size_t *out_length)
{
    size_t i = 0;
    lexbor_str_t *str;
    lxb_status_t status;
    unit_kv_value_t *entry, *type, *value;
    lxb_css_syntax_token_type_t token_type;

    while (i < arr->length) {
        entry = arr->list[i++];

        if (!unit_kv_is_hash(entry)) {
            return false;
        }

        type = unit_kv_hash_value_nolen_c(entry, "type");

        validate_pointer(type, "type");

        if (!unit_kv_is_string(type)) {
            return false;
        }

        str = unit_kv_string(type);

        token_type = lxb_css_syntax_token_type_id_by_name(str->data,
                                                          str->length);

        if (token_type != token->type) {
            return false;
        }

        value = unit_kv_hash_value_nolen_c(entry, "value");

        validate_pointer(value, "value");

        status = lxb_css_syntax_token_serialize_str(token, &helper->str,
                                                    helper->mraw);
        if (status != LXB_STATUS_OK) {
            return false;
        }

        str = unit_kv_string(value);

        if (str->length != helper->str.length
            || lexbor_str_data_ncmp(str->data,
                                    helper->str.data, str->length) == false)
        {
            lexbor_str_clean(&helper->str);
            return false;
        }

        lexbor_str_clean(&helper->str);

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    *out_length = i;

    return i == arr->length && token->type == LXB_CSS_SYNTAX_TOKEN__END;
}
