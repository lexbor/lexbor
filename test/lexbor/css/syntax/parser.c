/*
 * Copyright (C) 2022-2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/core/fs.h>
#include <lexbor/core/array.h>
#include <lexbor/core/print.h>

#include <unit/test.h>
#include <unit/kv.h>

#include <lexbor/css/css.h>


#define validate_pointer(helper, entry, ptr, name)                            \
    if (ptr == NULL) {                                                        \
        const lexbor_str_t str = {.data = (lxb_char_t *) name,                \
                                  .length = strlen(name)};                    \
        TEST_PRINTLN("Required parameter missing");                           \
        error_expect_type(helper, entry, &str);                               \
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA); \
    }

#define validate_pointer_p(helper, entry, ptr, name)                          \
    if (ptr == NULL) {                                                        \
        const lexbor_str_t str = {.data = (lxb_char_t *) name,                \
                                  .length = strlen(name)};                    \
        TEST_PRINTLN("Required parameter missing");                           \
        error_expect_type(helper, entry, &str);                               \
        return NULL;                                                          \
    }

#define FAILED(...)                                                           \
    do {                                                                      \
        fprintf(stderr, __VA_ARGS__);                                         \
        fprintf(stderr, "\n");                                                \
        exit(EXIT_FAILURE);                                                   \
    }                                                                         \
    while (0)


typedef struct {
    unit_kv_t        *kv;
    lxb_css_parser_t *parser;
    lexbor_str_t     str;
    lexbor_mraw_t    *mraw;
}
helper_t;

typedef struct {
    helper_t         *helper;
    unit_kv_array_t  *entries;
    size_t           idx;
    bool             have_block;
}
context_t;

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
test_css_list_rules_next(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
test_css_list_rules_end(lxb_css_parser_t *parser,
                        const lxb_css_syntax_token_t *token,
                        void *ctx, bool failed);

static const lxb_css_syntax_cb_at_rule_t *
test_css_at_rule_begin(lxb_css_parser_t *parser,
                       const lxb_css_syntax_token_t *token, void *ctx,
                       void **out_rule);

static bool
test_css_at_rule_prelude(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token,
                         void *ctx);

static lxb_status_t
test_css_at_rule_prelude_end(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, bool failed);

static const lxb_css_syntax_cb_block_t *
test_css_at_rule_block_begin(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, void **out_rule);
static bool
test_css_at_rule_prelude_failed(lxb_css_parser_t *parser,
                                const lxb_css_syntax_token_t *token,
                                void *ctx);
static lxb_status_t
test_css_at_rule_end(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token,
                     void *ctx, bool failed);

static const lxb_css_syntax_cb_qualified_rule_t *
test_css_qualified_rule_begin(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token,
                              void *ctx, void **out_rule);

static bool
test_css_qualified_rule_prelude(lxb_css_parser_t *parser,
                                const lxb_css_syntax_token_t *token,
                                void *ctx);

static lxb_status_t
test_css_qualified_rule_prelude_end(lxb_css_parser_t *parser,
                                    const lxb_css_syntax_token_t *token,
                                    void *ctx, bool failed);

static const lxb_css_syntax_cb_block_t *
test_css_qualified_rule_block_begin(lxb_css_parser_t *parser,
                                    const lxb_css_syntax_token_t *token,
                                    void *ctx, void **out_rule);

static bool
test_css_qualified_rule_prelude_failed(lxb_css_parser_t *parser,
                                       const lxb_css_syntax_token_t *token,
                                       void *ctx);
static lxb_status_t
test_css_qualified_rule_end(lxb_css_parser_t *parser,
                            const lxb_css_syntax_token_t *token,
                            void *ctx, bool failed);
static bool
test_css_block_next(lxb_css_parser_t *parser,
                    const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
test_css_block_end(lxb_css_parser_t *parser,
                   const lxb_css_syntax_token_t *token,
                   void *ctx, bool failed);

static const lxb_css_syntax_cb_declarations_t *
test_css_declarations_begin(lxb_css_parser_t *parser,
                            const lxb_css_syntax_token_t *token,
                            void *ctx, void **out_rule);

static lxb_css_parser_state_f
test_css_declaration_name(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token,
                          void *ctx, void **out_rule);
static bool
test_css_declaration_value(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
test_css_declaration_end(lxb_css_parser_t *parser,
                         void *declarations, void *ctx,
                         const lxb_css_syntax_token_t *token,
                         lxb_css_syntax_declaration_offset_t *offset,
                         bool important, bool failed);

static lxb_status_t
test_css_declarations_end(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token,
                          void *ctx, bool failed);

static bool
test_css_declarations_bad(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token, void *ctx);

static void
error_expect(helper_t *helper, unit_kv_value_t *value,
             const lxb_css_syntax_token_t *token);

static void
error_expect_str(helper_t *helper, unit_kv_value_t *value,
                 const lexbor_str_t *str);

static void
error_expect_type(helper_t *helper, unit_kv_value_t *value,
                  const lexbor_str_t *str);

static lxb_status_t
print_error(helper_t *helper, unit_kv_value_t *value);

static bool
validate_type(helper_t *helper, unit_kv_value_t *value,
              const lexbor_str_t *need);

static bool
validate_string(helper_t *helper, unit_kv_value_t *value,
                const lxb_css_syntax_token_t *token);

static bool
compare(helper_t *helper, unit_kv_value_t *prelude,
        const lxb_css_syntax_token_t *token);


static const lxb_css_syntax_cb_list_rules_t test_css_list_rules = {
    .at_rule = test_css_at_rule_begin,
    .qualified_rule = test_css_qualified_rule_begin,
    .next = test_css_list_rules_next,
    .cb.failed = lxb_css_state_failed,
    .cb.end = test_css_list_rules_end
};

static const lxb_css_syntax_cb_at_rule_t test_css_at_rule = {
    .prelude = test_css_at_rule_prelude,
    .prelude_end = test_css_at_rule_prelude_end,
    .block = test_css_at_rule_block_begin,
    .cb.failed = test_css_at_rule_prelude_failed,
    .cb.end = test_css_at_rule_end
};

static const lxb_css_syntax_cb_qualified_rule_t test_css_qualified_rule = {
    .prelude = test_css_qualified_rule_prelude,
    .prelude_end = test_css_qualified_rule_prelude_end,
    .block = test_css_qualified_rule_block_begin,
    .cb.failed = test_css_qualified_rule_prelude_failed,
    .cb.end = test_css_qualified_rule_end
};

static const lxb_css_syntax_cb_block_t test_css_block = {
    .at_rule = test_css_at_rule_begin,
    .declarations = test_css_declarations_begin,
    .qualified_rule = test_css_qualified_rule_begin,
    .next = test_css_block_next,
    .cb.failed = lxb_css_state_failed,
    .cb.end = test_css_block_end,
};

static const lxb_css_syntax_cb_declarations_t test_css_declaration = {
    .name = test_css_declaration_name,
    .end = test_css_declaration_end,
    .cb.failed = test_css_declarations_bad,
    .cb.end = test_css_declarations_end
};

static const lexbor_str_t at_rule = lexbor_str("at-rule");
static const lexbor_str_t qualified_rule = lexbor_str("qualified-rule");
static const lexbor_str_t declarations_rule = lexbor_str("declarations");


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
    context_t context;
    lxb_status_t status;
    lxb_css_syntax_rule_t *rule;

    helper->parser = lxb_css_parser_create();
    status = lxb_css_parser_init(helper->parser, NULL);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    context.helper = helper;
    context.entries = entries;
    context.idx = 0;

    helper->parser->memory = lxb_css_memory_create();
    status = lxb_css_memory_init(helper->parser->memory, 1024);
    if (status != LXB_STATUS_OK) {
        goto failed;
    }

    lxb_css_parser_buffer_set(helper->parser, str->data, str->length);

    rule = lxb_css_syntax_parser_list_rules_push(helper->parser,
                                                 &test_css_list_rules,
                                                 NULL, &context,
                                                 LXB_CSS_SYNTAX_TOKEN_UNDEF);
    if (rule == NULL) {
        goto failed;
    }

    status = lxb_css_syntax_parser_run(helper->parser);

failed:

    helper->parser->memory = lxb_css_memory_destroy(helper->parser->memory,
                                                    true);
    helper->parser = lxb_css_parser_destroy(helper->parser, true);

    return status;
}

static bool
test_css_list_rules_next(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token, void *ctx)
{
    context_t *context = ctx;

    context->idx += 1;

    return lxb_css_parser_success(parser);
}

static lxb_status_t
test_css_list_rules_end(lxb_css_parser_t *parser,
                        const lxb_css_syntax_token_t *token,
                        void *ctx, bool failed)
{
    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_at_rule_t *
test_css_at_rule_begin(lxb_css_parser_t *parser,
                       const lxb_css_syntax_token_t *token, void *ctx,
                       void **out_rule)
{
    context_t *context = ctx;
    unit_kv_value_t *entry = context->entries->list[context->idx];

    unit_kv_value_t *type = unit_kv_hash_value_nolen_c(entry, "type");
    unit_kv_value_t *name = unit_kv_hash_value_nolen_c(entry, "name");

    validate_pointer_p(context->helper, entry, type, "type");
    validate_pointer_p(context->helper, entry, name, "name");

    if (!validate_type(context->helper, type, &at_rule)) {
        return NULL;
    }

    if (!validate_string(context->helper, name, token)) {
        return NULL;
    }

    context->have_block = false;

    *out_rule = ctx;

    return &test_css_at_rule;
}

static bool
test_css_at_rule_prelude(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token, void *ctx)
{
    context_t *context = ctx;
    unit_kv_value_t *entry = context->entries->list[context->idx];

    unit_kv_value_t *prelude = unit_kv_hash_value_nolen_c(entry, "prelude");
    validate_pointer(context->helper, entry, prelude, "prelude");

    if (!compare(context->helper, prelude, token)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    return lxb_css_parser_success(parser);
}

static lxb_status_t
test_css_at_rule_prelude_end(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, bool failed)
{
    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_block_t *
test_css_at_rule_block_begin(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, void **out_rule)
{
    context_t *block_ctx;
    context_t *context = ctx;
    unit_kv_value_t *entry = context->entries->list[context->idx];

    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");
    validate_pointer_p(context->helper, entry, block, "block");

    const lexbor_str_t err_block = lexbor_str("Block is not array");

    if (!unit_kv_is_array(block)) {
        error_expect_str(context->helper, block, &err_block);
        return NULL;
    }

    block_ctx = lexbor_calloc(1, sizeof(context_t));
    if (block_ctx == NULL) {
        return NULL;
    }

    context->have_block = true;

    block_ctx->helper = context->helper;
    block_ctx->entries = unit_kv_array(block);
    block_ctx->idx = 0;

    *out_rule = block_ctx;

    return &test_css_block;
}

static bool
test_css_at_rule_prelude_failed(lxb_css_parser_t *parser,
                                const lxb_css_syntax_token_t *token,
                                void *ctx)
{
    /* We won't be able to get in here, it's just a formality. */
    return lxb_css_parser_success(parser);
}

static lxb_status_t
test_css_at_rule_end(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token,
                     void *ctx, bool failed)
{
    context_t *context = ctx;
    unit_kv_value_t *entry = context->entries->list[context->idx];
    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");

    const lexbor_str_t err_block = lexbor_str("We have a block, but we "
                                              "didn't expect it");

    if (block == NULL && context->have_block) {
        error_expect_str(context->helper, entry, &err_block);
        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_qualified_rule_t *
test_css_qualified_rule_begin(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token,
                              void *ctx, void **out_rule)
{
    context_t *context = ctx;
    unit_kv_value_t *entry = context->entries->list[context->idx];

    unit_kv_value_t *type = unit_kv_hash_value_nolen_c(entry, "type");
    validate_pointer_p(context->helper, entry, type, "type");

    if (!validate_type(context->helper, type, &qualified_rule)) {
        return NULL;
    }

    *out_rule = ctx;

    return &test_css_qualified_rule;
}

static bool
test_css_qualified_rule_prelude(lxb_css_parser_t *parser,
                                const lxb_css_syntax_token_t *token,
                                void *ctx)
{
    context_t *context = ctx;
    unit_kv_value_t *entry = context->entries->list[context->idx];

    unit_kv_value_t *prelude = unit_kv_hash_value_nolen_c(entry, "prelude");
    validate_pointer(context->helper, entry, prelude, "prelude");

    if (!compare(context->helper, prelude, token)) {
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    return lxb_css_parser_success(parser);
}

static lxb_status_t
test_css_qualified_rule_prelude_end(lxb_css_parser_t *parser,
                                    const lxb_css_syntax_token_t *token,
                                    void *ctx, bool failed)
{
    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_block_t *
test_css_qualified_rule_block_begin(lxb_css_parser_t *parser,
                                    const lxb_css_syntax_token_t *token,
                                    void *ctx, void **out_rule)
{
    context_t *block_ctx;
    context_t *context = ctx;
    unit_kv_value_t *entry = context->entries->list[context->idx];

    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");
    validate_pointer_p(context->helper, entry, block, "block");

    const lexbor_str_t err_block = lexbor_str("Block is not array");

    if (!unit_kv_is_array(block)) {
        error_expect_str(context->helper, block, &err_block);
        return NULL;
    }

    block_ctx = lexbor_calloc(1, sizeof(context_t));
    if (block_ctx == NULL) {
        return NULL;
    }

    context->have_block = true;

    block_ctx->helper = context->helper;
    block_ctx->entries = unit_kv_array(block);
    block_ctx->idx = 0;

    *out_rule = block_ctx;

    return &test_css_block;
}

static bool
test_css_qualified_rule_prelude_failed(lxb_css_parser_t *parser,
                                       const lxb_css_syntax_token_t *token,
                                       void *ctx)
{
    /* We won't be able to get in here, it's just a formality. */
    return lxb_css_parser_success(parser);
}

static lxb_status_t
test_css_qualified_rule_end(lxb_css_parser_t *parser,
                            const lxb_css_syntax_token_t *token,
                            void *ctx, bool failed)
{
    context_t *context = ctx;
    unit_kv_value_t *entry = context->entries->list[context->idx];
    unit_kv_value_t *block = unit_kv_hash_value_nolen_c(entry, "block");

    const lexbor_str_t err_block = lexbor_str("We have a block, but we "
                                              "didn't expect it");

    if (block == NULL && context->have_block) {
        error_expect_str(context->helper, entry, &err_block);
        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    return LXB_STATUS_OK;
}

static bool
test_css_block_next(lxb_css_parser_t *parser,
                    const lxb_css_syntax_token_t *token, void *ctx)
{
    context_t *context = ctx;

    context->idx += 1;

    return lxb_css_parser_success(parser);
}

static lxb_status_t
test_css_block_end(lxb_css_parser_t *parser,
                   const lxb_css_syntax_token_t *token,
                   void *ctx, bool failed)
{
    context_t *context = ctx;

    if (context->entries->length != context->idx) {
        TEST_PRINTLN("The number of elements in the block does not "
                     "match the declared number.");
        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    lexbor_free(context);

    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_declarations_t *
test_css_declarations_begin(lxb_css_parser_t *parser,
                            const lxb_css_syntax_token_t *token,
                            void *ctx, void **out_rule)
{
    context_t *declrs_ctx;
    context_t *context = ctx;
    unit_kv_value_t *entry = context->entries->list[context->idx];

    unit_kv_value_t *type = unit_kv_hash_value_nolen_c(entry, "type");
    unit_kv_value_t *declarations = unit_kv_hash_value_nolen_c(entry,
                                                               "declarations");
    validate_pointer_p(context->helper, entry, type, "type");
    validate_pointer_p(context->helper, entry, declarations, "declarations");

    const lexbor_str_t err_declarations = lexbor_str("Declarations is not array");

    if (!validate_type(context->helper, type, &declarations_rule)) {
        return NULL;
    }

    if (!unit_kv_is_array(declarations)) {
        error_expect_str(context->helper, declarations, &err_declarations);
        return NULL;
    }

    declrs_ctx = lexbor_malloc(sizeof(context_t));
    if (declrs_ctx == NULL) {
        return NULL;
    }

    context->have_block = true;

    declrs_ctx->helper = context->helper;
    declrs_ctx->entries = unit_kv_array(declarations);
    declrs_ctx->idx = 0;

    *out_rule = declrs_ctx;

    return &test_css_declaration;
}

static lxb_css_parser_state_f
test_css_declaration_name(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token,
                          void *ctx, void **out_rule)
{
    context_t *context = ctx;
    unit_kv_value_t *entry = context->entries->list[context->idx];

    unit_kv_value_t *name = unit_kv_hash_value_nolen_c(entry, "name");
    validate_pointer_p(context->helper, entry, name, "name");

    if (!validate_string(context->helper, name, token)) {
        return NULL;
    }

    *out_rule = ctx;

    return test_css_declaration_value;
}

static bool
test_css_declaration_value(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_status_t status;
    lexbor_str_t *str;
    context_t *context = ctx;
    helper_t *helper = context->helper;
    unit_kv_value_t *entry = context->entries->list[context->idx];

    unit_kv_value_t *value = unit_kv_hash_value_nolen_c(entry, "value");
    validate_pointer_p(context->helper, entry, value, "value");

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
        TEST_PRINTLN("Declaration value need: %s\nDeclaration value have: %s",
                     (const char *) str->data, (const char *) str->data);

        lexbor_str_clean(&helper->str);
        return lxb_css_parser_fail(parser, LXB_STATUS_ERROR_UNEXPECTED_DATA);
    }

    lexbor_str_clean(&helper->str);

    return lxb_css_parser_success(parser);
}

static lxb_status_t
test_css_declaration_end(lxb_css_parser_t *parser,
                         void *declarations, void *ctx,
                         const lxb_css_syntax_token_t *token,
                         lxb_css_syntax_declaration_offset_t *offset,
                         bool important, bool failed)
{
    context_t *context = ctx;

    context->idx += 1;

    return LXB_STATUS_OK;
}

static lxb_status_t
test_css_declarations_end(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token,
                          void *ctx, bool failed)
{
    context_t *context = ctx;

    if (context->entries->length != context->idx) {
        TEST_PRINTLN("The number of elements in the declarations does not "
                     "match the declared number.");

        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
    }

    lexbor_free(context);

    return LXB_STATUS_OK;
}

static bool
test_css_declarations_bad(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token, void *ctx)
{
    /* We won't be able to get in here, it's just a formality. */
    return lxb_css_parser_success(parser);
}

static void
error_expect(helper_t *helper, unit_kv_value_t *value,
             const lxb_css_syntax_token_t *token)
{
    lxb_status_t status;
    lexbor_str_t expect;

    const char format[] = "Expect: %s\nHave: %s\n";

    status = lxb_css_syntax_token_serialize_str(token, &helper->str,
                                                helper->mraw);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to serialize data.");
    }

    expect = unit_kv_value_position_as_string(helper->kv, value);

    printf(format, (const char *) expect.data,
           (const char *) helper->str.data);

    lexbor_str_clean(&helper->str);
    unit_kv_string_destroy(helper->kv, &expect, false);
}

static void
error_expect_str(helper_t *helper, unit_kv_value_t *value,
                 const lexbor_str_t *str)
{
    lexbor_str_t expect;

    const char format[] = "Expect: %s\nHave: %s\n";

    expect = unit_kv_value_position_as_string(helper->kv, value);

    printf(format, (const char *) expect.data, (const char *) str->data);

    lexbor_str_clean(&helper->str);
    unit_kv_string_destroy(helper->kv, &expect, false);
}

static void
error_expect_type(helper_t *helper, unit_kv_value_t *value,
                  const lexbor_str_t *str)
{
    lexbor_str_t expect;

    const char format[] = "In: %s\nNeed entry: %s\n";

    expect = unit_kv_value_position_as_string(helper->kv, value);

    printf(format, (const char *) expect.data, (const char *) str->data);

    lexbor_str_clean(&helper->str);
    unit_kv_string_destroy(helper->kv, &expect, false);
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
validate_type(helper_t *helper, unit_kv_value_t *value,
              const lexbor_str_t *need)
{
    bool res;
    lexbor_str_t *type;

    if (!unit_kv_is_string(value)) {
        return false;
    }

    type = unit_kv_string(value);

    if (need->length != type->length) {
        return false;
    }

    res = lexbor_str_data_ncmp(need->data, type->data, type->length);

    if (!res) {
        error_expect_str(helper, value, need);
    }

    return res;
}

static bool
validate_string(helper_t *helper, unit_kv_value_t *value,
                const lxb_css_syntax_token_t *token)
{
    bool res;
    lxb_status_t status;
    lexbor_str_t *need;

    if (!unit_kv_is_string(value)) {
        return false;
    }

    status = lxb_css_syntax_token_serialize_str(token, &helper->str,
                                                helper->mraw);
    if (status != LXB_STATUS_OK) {
        return false;
    }

    res = false;
    need = unit_kv_string(value);

    if (helper->str.length == need->length) {
        res = lexbor_str_data_ncmp(helper->str.data, need->data, need->length);
    }

    lexbor_str_clean(&helper->str);

    if (!res) {
        (void) error_expect(helper, value, token);
    }

    return res;
}

static bool
compare(helper_t *helper, unit_kv_value_t *prelude,
        const lxb_css_syntax_token_t *token)
{
    size_t i;
    lexbor_str_t *str;
    lxb_status_t status;
    unit_kv_array_t *arr;
    lxb_css_parser_t *parser;
    unit_kv_value_t *entry, *type, *value;
    lxb_css_syntax_token_type_t token_type;

    const lexbor_str_t err_str = lexbor_str("string");
    const lexbor_str_t err_prelude = lexbor_str("Prelude is not array");
    const lexbor_str_t err_count = lexbor_str("The number of tokens does "
                                              "not match");

    if (!unit_kv_is_array(prelude)) {
        error_expect_str(helper, prelude, &err_prelude);
        return false;
    }

    i = 0;
    arr = unit_kv_array(prelude);
    parser = helper->parser;

    while (i < arr->length) {
        entry = arr->list[i++];

        if (!unit_kv_is_hash(entry)) {
            return false;
        }

        type = unit_kv_hash_value_nolen_c(entry, "type");

        validate_pointer(helper, entry, type, "type");

        if (!unit_kv_is_string(type)) {
            error_expect_type(helper, type, &err_str);
            return false;
        }

        str = unit_kv_string(type);

        token_type = lxb_css_syntax_token_type_id_by_name(str->data,
                                                          str->length);

        if (token_type != token->type) {
            error_expect(helper, type, token);
            return false;
        }

        value = unit_kv_hash_value_nolen_c(entry, "value");

        validate_pointer(helper, entry, value, "value");

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
            error_expect(helper, value, token);
            return false;
        }

        lexbor_str_clean(&helper->str);

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    if (i != arr->length || token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        error_expect_str(helper, prelude, &err_count);
        return false;
    }

    return true;
}
