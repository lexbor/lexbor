/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

/*
 * Test runner for html5lib-tests tokenizer test suite.
 * The test files are in JSON format (parsed by unit_kv).
 *
 * Usage: html5lib_tests <directory path>
 */

#include "lexbor/html/html.h"
#include "lexbor/core/fs.h"
#include "unit/kv.h"
#include "unit/test.h"


/*
 * Test context.
 */
typedef struct {
    unit_kv_t *kv;
    unsigned  test_total;
    unsigned  test_fail;
} test_ctx_t;

typedef struct {
    const char                    *name;
    lxb_html_tokenizer_error_id_t id;
} error_map_entry_t;


static const error_map_entry_t error_map[] = {
    {"abrupt-closing-of-empty-comment",                        LXB_HTML_TOKENIZER_ERROR_ABCLOFEMCO},
    {"abrupt-doctype-public-identifier",                       LXB_HTML_TOKENIZER_ERROR_ABDOPUID},
    {"abrupt-doctype-system-identifier",                       LXB_HTML_TOKENIZER_ERROR_ABDOSYID},
    {"absence-of-digits-in-numeric-character-reference",       LXB_HTML_TOKENIZER_ERROR_ABOFDIINNUCHRE},
    {"cdata-in-html-content",                                  LXB_HTML_TOKENIZER_ERROR_CDINHTCO},
    {"character-reference-outside-unicode-range",              LXB_HTML_TOKENIZER_ERROR_CHREOUUNRA},
    {"control-character-in-input-stream",                      LXB_HTML_TOKENIZER_ERROR_COCHININST},
    {"control-character-reference",                            LXB_HTML_TOKENIZER_ERROR_COCHRE},
    {"end-tag-with-attributes",                                LXB_HTML_TOKENIZER_ERROR_ENTAWIAT},
    {"duplicate-attribute",                                    LXB_HTML_TOKENIZER_ERROR_DUAT},
    {"end-tag-with-trailing-solidus",                          LXB_HTML_TOKENIZER_ERROR_ENTAWITRSO},
    {"eof-before-tag-name",                                    LXB_HTML_TOKENIZER_ERROR_EOBETANA},
    {"eof-in-cdata",                                           LXB_HTML_TOKENIZER_ERROR_EOINCD},
    {"eof-in-comment",                                         LXB_HTML_TOKENIZER_ERROR_EOINCO},
    {"eof-in-doctype",                                         LXB_HTML_TOKENIZER_ERROR_EOINDO},
    {"eof-in-script-html-comment-like-text",                   LXB_HTML_TOKENIZER_ERROR_EOINSCHTCOLITE},
    {"eof-in-tag",                                             LXB_HTML_TOKENIZER_ERROR_EOINTA},
    {"incorrectly-closed-comment",                             LXB_HTML_TOKENIZER_ERROR_INCLCO},
    {"incorrectly-opened-comment",                             LXB_HTML_TOKENIZER_ERROR_INOPCO},
    {"invalid-character-sequence-after-doctype-name",          LXB_HTML_TOKENIZER_ERROR_INCHSEAFDONA},
    {"invalid-first-character-of-tag-name",                    LXB_HTML_TOKENIZER_ERROR_INFICHOFTANA},
    {"missing-attribute-value",                                LXB_HTML_TOKENIZER_ERROR_MIATVA},
    {"missing-doctype-name",                                   LXB_HTML_TOKENIZER_ERROR_MIDONA},
    {"missing-doctype-public-identifier",                      LXB_HTML_TOKENIZER_ERROR_MIDOPUID},
    {"missing-doctype-system-identifier",                      LXB_HTML_TOKENIZER_ERROR_MIDOSYID},
    {"missing-end-tag-name",                                   LXB_HTML_TOKENIZER_ERROR_MIENTANA},
    {"missing-quote-before-doctype-public-identifier",         LXB_HTML_TOKENIZER_ERROR_MIQUBEDOPUID},
    {"missing-quote-before-doctype-system-identifier",         LXB_HTML_TOKENIZER_ERROR_MIQUBEDOSYID},
    {"missing-semicolon-after-character-reference",            LXB_HTML_TOKENIZER_ERROR_MISEAFCHRE},
    {"missing-whitespace-after-doctype-public-keyword",        LXB_HTML_TOKENIZER_ERROR_MIWHAFDOPUKE},
    {"missing-whitespace-after-doctype-system-keyword",        LXB_HTML_TOKENIZER_ERROR_MIWHAFDOSYKE},
    {"missing-whitespace-before-doctype-name",                 LXB_HTML_TOKENIZER_ERROR_MIWHBEDONA},
    {"missing-whitespace-between-attributes",                  LXB_HTML_TOKENIZER_ERROR_MIWHBEAT},
    {"missing-whitespace-between-doctype-public-and-system-identifiers", LXB_HTML_TOKENIZER_ERROR_MIWHBEDOPUANSYID},
    {"nested-comment",                                         LXB_HTML_TOKENIZER_ERROR_NECO},
    {"noncharacter-character-reference",                       LXB_HTML_TOKENIZER_ERROR_NOCHRE},
    {"noncharacter-in-input-stream",                           LXB_HTML_TOKENIZER_ERROR_NOININST},
    {"non-void-html-element-start-tag-with-trailing-solidus",  LXB_HTML_TOKENIZER_ERROR_NOVOHTELSTTAWITRSO},
    {"null-character-reference",                               LXB_HTML_TOKENIZER_ERROR_NUCHRE},
    {"surrogate-character-reference",                          LXB_HTML_TOKENIZER_ERROR_SUCHRE},
    {"surrogate-in-input-stream",                              LXB_HTML_TOKENIZER_ERROR_SUININST},
    {"unexpected-character-after-doctype-system-identifier",   LXB_HTML_TOKENIZER_ERROR_UNCHAFDOSYID},
    {"unexpected-character-in-attribute-name",                 LXB_HTML_TOKENIZER_ERROR_UNCHINATNA},
    {"unexpected-character-in-unquoted-attribute-value",       LXB_HTML_TOKENIZER_ERROR_UNCHINUNATVA},
    {"unexpected-equals-sign-before-attribute-name",           LXB_HTML_TOKENIZER_ERROR_UNEQSIBEATNA},
    {"unexpected-null-character",                              LXB_HTML_TOKENIZER_ERROR_UNNUCH},
    {"unexpected-question-mark-instead-of-tag-name",           LXB_HTML_TOKENIZER_ERROR_UNQUMAINOFTANA},
    {"unexpected-solidus-in-tag",                              LXB_HTML_TOKENIZER_ERROR_UNSOINTA},
    {"unknown-named-character-reference",                      LXB_HTML_TOKENIZER_ERROR_UNNACHRE},
    {NULL, 0}
};


static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx_ptr);
static bool
process_test_file(test_ctx_t *ctx, const char *filepath);

static bool
run_test_entry(test_ctx_t *ctx, unit_kv_value_t *test_obj);

static lxb_html_tokenizer_t *
tokenizer_parse(unit_kv_t *kv, const lexbor_str_t *html,
                unit_kv_value_t *state_name, unit_kv_value_t *last_start_tag,
                unit_kv_value_t *double_escaped, bool chunked);

static lxb_html_token_t *
tokenizer_callback_token_done(lxb_html_tokenizer_t *tkz,
                              lxb_html_token_t *token, void *ctx);

static bool
compare_output(unit_kv_t *kv, lxb_html_tokenizer_t *tkz,
               const unit_kv_array_t *output, unit_kv_value_t *double_escaped);

static bool
compare_token(unit_kv_t *kv, lxb_html_tokenizer_t *tkz, lxb_html_token_t *token,
              unit_kv_value_t *expected, bool double_escaped);

static bool
compare_errors(unit_kv_t *kv, lxb_html_tokenizer_t *tkz,
               unit_kv_value_t *errors);

static void
double_unescape(lxb_html_tokenizer_t *tkz, const lxb_char_t *data, size_t length,
                lexbor_str_t *out);

static size_t
encode_utf8(uint32_t cp, lxb_char_t *buf);

static unsigned
hex_digit(char c);

static bool
set_initial_state(lxb_html_tokenizer_t *tkz, const char *state_name,
                  const char *last_start_tag);

static lxb_html_tokenizer_error_id_t
error_code_by_name(const char *name);

static lexbor_str_t *
kv_str(unit_kv_t *kv, unit_kv_value_t *val, const char *name);

static const unit_kv_array_t *
kv_array(unit_kv_t *kv, unit_kv_value_t *val, const char *name);

static void
print_error(unit_kv_t *kv, unit_kv_value_t *value);

static void
print_fail(unit_kv_t *kv, unit_kv_value_t *val, const char *fmt, ...);


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

    ctx.kv = unit_kv_create();
    status = unit_kv_init(ctx.kv, 256);
    if (status != LXB_STATUS_OK) {
        TEST_PRINTLN("Failed to init KV parser");
        return EXIT_FAILURE;
    }

    status = lexbor_fs_dir_read(full_path, LEXBOR_FS_DIR_OPT_WITHOUT_DIR
                                |LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN,
                                file_callback, &ctx);

    unit_kv_destroy(ctx.kv, true);

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
 * Directory callback.
 */
static lexbor_action_t
file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
              const lxb_char_t *filename, size_t filename_len, void *ctx_ptr)
{
    test_ctx_t *ctx = ctx_ptr;

    if (filename_len < 6
        || strncmp((const char *) &filename[filename_len - 5], ".test", 5) != 0)
    {
        return LEXBOR_ACTION_OK;
    }

    if (!process_test_file(ctx, (const char *) fullpath)) {
        return LEXBOR_ACTION_STOP;
    }

    return LEXBOR_ACTION_OK;
}

/*
 * Process a single .test file.
 */
static bool
process_test_file(test_ctx_t *ctx, const char *filepath)
{
    lxb_status_t status;
    unit_kv_value_t *tests, *root, *test_obj;
    unit_kv_array_t *tests_arr;

    unit_kv_clean(ctx->kv);

    status = unit_kv_parse_file(ctx->kv, (const lxb_char_t *) filepath);
    if (status != LXB_STATUS_OK) {
        lexbor_str_t str = unit_kv_parse_error_as_string(ctx->kv);

        TEST_PRINTLN("Failed to parse test file: %s", filepath);
        TEST_PRINTLN("%s", str.data);

        unit_kv_string_destroy(ctx->kv, &str, false);

        return false;
    }

    root = unit_kv_value(ctx->kv);

    if (root == NULL || !unit_kv_is_hash(root)) {
        TEST_PRINTLN("Root is not a hash in file: %s", filepath);
        return false;
    }

    /* Find the "tests" array. */
    tests = unit_kv_hash_value_nolen_c(root, "tests");

    if (tests == NULL) {
        TEST_PRINTLN("No 'tests' array in file: %s", filepath);
        return true;
    }

    tests_arr = unit_kv_array(tests);

    TEST_PRINTLN("Test file: %s ("LEXBOR_FORMAT_Z" tests)", filepath,
                 tests_arr->length);

    for (size_t i = 0; i < tests_arr->length; i++) {
        test_obj = tests_arr->list[i];

        if (!unit_kv_is_hash(test_obj)) {
            continue;
        }

        ctx->test_total++;

        if (!run_test_entry(ctx, test_obj)) {
            ctx->test_fail++;
        }
    }

    return true;
}

/*
 * Run a single test entry.
 * Run once for each initialState, both full and stream.
 */
static bool
run_test_entry(test_ctx_t *ctx, unit_kv_value_t *test_obj)
{
    bool ok, is;
    lexbor_str_t *input_str;
    const unit_kv_array_t *output_arr;
    lxb_html_tokenizer_t *tkz;
    unit_kv_value_t *input_val, *output_val, *errors_val, *states_val, *val;
    unit_kv_value_t *last_tag_val, *dbl_esc_val;

    input_val = unit_kv_hash_value_nolen_c(test_obj, "input");
    output_val = unit_kv_hash_value_nolen_c(test_obj, "output");
    errors_val = unit_kv_hash_value_nolen_c(test_obj, "errors");
    states_val = unit_kv_hash_value_nolen_c(test_obj, "initialStates");
    last_tag_val = unit_kv_hash_value_nolen_c(test_obj, "lastStartTag");
    dbl_esc_val = unit_kv_hash_value_nolen_c(test_obj, "doubleEscaped");

    /* `description`, `input` and `output` are always present. */

    input_str = kv_str(ctx->kv, input_val, "input");
    output_arr = kv_array(ctx->kv, output_val, "output");

    ok = true;

    if (states_val == NULL) {
        tkz = tokenizer_parse(ctx->kv, input_str, NULL, NULL,
                              dbl_esc_val, false);
        is = compare_output(ctx->kv, tkz, output_arr, dbl_esc_val);
        if (!is) {
            ok = false;
        }

        is = compare_errors(ctx->kv, tkz, errors_val);
        if (!is) {
            ok = false;
        }

        lxb_html_tokenizer_tags_destroy(tkz);
        lxb_html_tokenizer_attrs_destroy(tkz);
        lxb_html_tokenizer_destroy(tkz);
    }
    else {
        if (!unit_kv_is_array(states_val)) {
            TEST_PRINTLN("'initialStates' is not an array in test");
            print_error(ctx->kv, states_val);
            return false;
        }

        /* Run tests for each initial state */
        size_t state_count = unit_kv_array(states_val)->length;

        for (size_t i = 0; i < state_count; i++) {
            val = unit_kv_array(states_val)->list[i];

            tkz = tokenizer_parse(ctx->kv, input_str, val, last_tag_val,
                                  dbl_esc_val, false);
            is = compare_output(ctx->kv, tkz, output_arr, dbl_esc_val);
            if (!is) {
                ok = false;
            }

            is = compare_errors(ctx->kv, tkz, errors_val);
            if (!is) {
                ok = false;
            }

            lxb_html_tokenizer_tags_destroy(tkz);
            lxb_html_tokenizer_attrs_destroy(tkz);
            lxb_html_tokenizer_destroy(tkz);
        }
    }

    return ok;
}

static lxb_html_tokenizer_t *
tokenizer_parse(unit_kv_t *kv, const lexbor_str_t *html,
                unit_kv_value_t *state_name, unit_kv_value_t *last_start_tag,
                unit_kv_value_t *double_escaped, bool chunked)
{
    bool is;
    lxb_char_t *tag;
    lexbor_str_t out, *state, *last_tag;
    lxb_status_t status;
    lxb_html_tokenizer_t *tkz;
    lxb_char_t c;

    tkz = lxb_html_tokenizer_create();
    status = lxb_html_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to initialize tokenizer");
    }

    status = lxb_html_tokenizer_tags_make(tkz, 128);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to create tags array");
    }

    status = lxb_html_tokenizer_attrs_make(tkz, 128);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to create attributes array");
    }

    lxb_html_tokenizer_callback_token_done_set(tkz, tokenizer_callback_token_done,
                                               NULL);

    lxb_html_tokenizer_input_validation_set(tkz, true);

    if (state_name != NULL) {
        state = kv_str(kv, state_name, "initialStates");
        last_tag = (last_start_tag != NULL)
        ? kv_str(kv, last_start_tag, "lastStartTag")
        : NULL;

        tag = (last_tag != NULL) ? last_tag->data : NULL;

        is = set_initial_state(tkz, (const char *) state->data,
                               (const char *) tag);
        if (!is) {
            TEST_PRINTLN("Failed to set initial state: %s",
                         (const char *) state->data);
            print_error(kv, state_name);
            exit(EXIT_FAILURE);
        }
    }

    if (double_escaped != NULL && unit_kv_bool(double_escaped)) {
        double_unescape(tkz, html->data, html->length, &out);
    }
    else {
        out.data = html->data;
        out.length = html->length;
    }

    status = lxb_html_tokenizer_begin(tkz);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to begin tokenization");
    }

    if (!chunked) {
        status = lxb_html_tokenizer_chunk(tkz, out.data, out.length);
        if (status != LXB_STATUS_OK) {
            TEST_FAILURE("Failed to tokenize input");
        }
    }
    else {
        for (size_t i = 0; i < html->length; i++) {
            c = html->data[i];

            status = lxb_html_tokenizer_chunk(tkz, &c, 1);
            if (status != LXB_STATUS_OK) {
                TEST_FAILURE("Failed to tokenize input (chunked)");
            }
        }
    }

    lxb_html_tokenizer_end(tkz);

    return tkz;
}

static lxb_html_token_t *
tokenizer_callback_token_done(lxb_html_tokenizer_t *tkz,
                              lxb_html_token_t *token, void *ctx)
{
    lexbor_str_t str;

    if (token->text_start != NULL) {
        lexbor_str_init_append(&str, tkz->mraw, token->text_start,
                               token->text_end - token->text_start);
        if (str.data == NULL) {
            TEST_FAILURE("Failed to initialize token text string");
        }

        token->text_start = str.data;
        token->text_end = str.data + str.length;
    }

    return lxb_html_token_create(tkz->dobj_token);
}

static bool
compare_output(unit_kv_t *kv, lxb_html_tokenizer_t *tkz,
               const unit_kv_array_t *output, unit_kv_value_t *double_escaped)
{
    bool d_esc;
    unit_kv_value_t *expected;
    lxb_html_token_t *token;
    lexbor_str_t tmp;

    memset(&tmp, 0, sizeof(lexbor_str_t));

    lexbor_str_init(&tmp, tkz->mraw, 256);
    if (tmp.data == NULL) {
        TEST_FAILURE("Failed to initialize temporary string");
    }

    d_esc = double_escaped != NULL && unit_kv_bool(double_escaped);

    for (unsigned i = 0; i < output->length; i++) {
        expected = output->list[i];

        token = lexbor_dobject_by_absolute_position(tkz->dobj_token, i);
        if (token == NULL) {
            TEST_PRINTLN("Missing token at position %u", i);
            print_error(kv, expected);
            return false;
        }

        tmp.length = 0;

        while (token->tag_id == LXB_TAG__TEXT) {
            lexbor_str_append(&tmp, tkz->mraw, token->text_start,
                              token->text_end - token->text_start);

            token = lexbor_dobject_by_absolute_position(tkz->dobj_token, ++i);
            if (token == NULL) {
                TEST_FAILURE("Missing token at position %u", i);
            }

            if (token->tag_id != LXB_TAG__TEXT) {
                token = lexbor_dobject_by_absolute_position(tkz->dobj_token, --i);

                token->text_start = tmp.data;
                token->text_end = tmp.data + tmp.length;

                break;
            }
        }

        if (!compare_token(kv, tkz, token, expected, d_esc)) {
            return false;
        }
    }

    return true;
}

static bool
compare_token(unit_kv_t *kv, lxb_html_tokenizer_t *tkz, lxb_html_token_t *token,
              unit_kv_value_t *expected, bool double_escaped)
{
    bool match;
    unsigned have_len, want_len;
    lxb_tag_id_t want_tag;
    unit_kv_array_t *arr;
    lexbor_str_t *type_str, *data_str, tmp_str;
    lxb_html_token_attr_t *attr;
    const lxb_char_t *have_data, *want_data;

    if (!unit_kv_is_array(expected)) {
        print_fail(kv, expected, "Expected output token is not array");
        return false;
    }

    arr = unit_kv_array(expected);

    if (arr->length < 2) {
        print_fail(kv, expected, "Expected output token too short");
        return false;
    }

    type_str = kv_str(kv, arr->list[0], "Token Type");
    if (type_str == NULL) {
        print_fail(kv, expected, "Token type is not a string");
        return false;
    }

    if (unit_kv_is_null(arr->list[1])) {
        data_str = NULL;
    }
    else {
        data_str = kv_str(kv, arr->list[1], "Token Data");
    }

    want_data = NULL;
    want_len = 0;

    if (data_str != NULL) {
        if (double_escaped) {
            double_unescape(tkz, data_str->data, data_str->length, &tmp_str);

            want_data = tmp_str.data;
            want_len = (unsigned) tmp_str.length;
        }
        else {
            want_data = data_str->data;
            want_len = (unsigned) data_str->length;
        }
    }

    const char *type_name = (const char *) type_str->data;

    /* Character token */
    if (strcmp(type_name, "Character") == 0) {
        if (token->tag_id != LXB_TAG__TEXT) {
            print_fail(kv, expected, "Expected Character token, got tag_id=%d",
                       (int) token->tag_id);
            return false;
        }

        have_len = (unsigned) (token->text_end - token->text_start);
        have_data = token->text_start;

        match = (have_len == want_len
                 && memcmp(have_data, want_data, want_len) == 0);

        if (!match) {
            print_fail(kv, expected, "Character data mismatch.\n"
                       "    Have (%u bytes): %.*s\n"
                       "    Want (%u bytes): %.*s",
                       have_len, (int) have_len, have_data,
                       want_len, (int) want_len, want_data);
        }

        return match;
    }

    /* Comment token */
    if (strcmp(type_name, "Comment") == 0) {
        if (token->tag_id != LXB_TAG__EM_COMMENT) {
            print_fail(kv, expected, "Expected Comment token, got tag_id=%d",
                       (int) token->tag_id);
            return false;
        }

        have_len = (unsigned) (token->text_end - token->text_start);
        have_data = token->text_start;

        match = (have_len == want_len
                 && memcmp(have_data, want_data, want_len) == 0);

        if (!match) {
            print_fail(kv, expected, "Comment data mismatch.\n"
                       "    Have (%u bytes): %.*s\n"
                       "    Want (%u bytes): %.*s",
                       have_len, (int) have_len, have_data,
                       want_len, (int) want_len, want_data);
        }

        return match;
    }

    /* StartTag token */
    if (strcmp(type_name, "StartTag") == 0) {
        if (token->type & LXB_HTML_TOKEN_TYPE_CLOSE) {
            print_fail(kv, expected, "Expected StartTag but got close tag");
            return false;
        }

        if (data_str == NULL) {
            print_fail(kv, expected, "StartTag name is not a string");
            return false;
        }

        want_tag = lxb_tag_id_by_name(tkz->tags, data_str->data,
                                      data_str->length);
        if (token->tag_id != want_tag) {
            print_fail(kv, expected, "StartTag name mismatch. Want: %s",
                       data_str->data);
            return false;
        }

        /* Check attributes (arr->list[2] is hash/object if present) */
        if (arr->length >= 3 && unit_kv_is_hash(arr->list[2])) {
            unit_kv_value_t *attrs_hash;

            attrs_hash = arr->list[2];
            attr = token->attr_first;

            /*
             * The KV hash doesn't store items in order with count.
             * Instead, we iterate token attrs and check each against the hash.
             */
            attr = token->attr_first;

            for (size_t i = 0; attr != NULL; attr = attr->next, i++) {
                size_t name_len;
                const lxb_char_t *name_data;
                unit_kv_value_t *want_val;
                lexbor_str_t *want_str;

                name_data = lxb_html_token_attr_name(attr, &name_len);
                want_val = unit_kv_hash_value(attrs_hash, name_data, name_len);

                if (want_val == NULL) {
                    print_fail(kv, expected, "Unexpected attr '%.*s' not in expected hash",
                               (int) name_len, name_data);
                    return false;
                }

                want_str = kv_str(kv, want_val, "Value");

                if (attr->value_size != want_str->length
                    || (want_str->length > 0
                        && memcmp(attr->value, want_str->data,
                                  want_str->length) != 0))
                {
                    print_fail(kv, expected, "Attr value mismatch for '%.*s'. "
                               "Have: %.*s, Want: %.*s",
                               (int) name_len, name_data,
                               (int) attr->value_size, attr->value,
                               (int) want_str->length, want_str->data);
                    return false;
                }
            }
        }

        /* Check self-closing (arr->list[3] == true) */
        if (arr->length >= 4 && unit_kv_is_bool(arr->list[3])
            && unit_kv_bool(arr->list[3]))
        {
            if (!(token->type & LXB_HTML_TOKEN_TYPE_CLOSE_SELF)) {
                print_fail(kv, expected, "Expected self-closing flag on StartTag");
                return false;
            }
        }

        return true;
    }

    /* EndTag token */
    if (strcmp(type_name, "EndTag") == 0) {
        if (!(token->type & LXB_HTML_TOKEN_TYPE_CLOSE)) {
            print_fail(kv, expected, "Expected EndTag but got open tag");
            return false;
        }

        if (data_str == NULL) {
            print_fail(kv, expected, "EndTag name is not a string");
            return false;
        }

        want_tag = lxb_tag_id_by_name(tkz->tags, data_str->data,
                                      data_str->length);
        if (token->tag_id != want_tag) {
            print_fail(kv, expected, "EndTag name mismatch. Want: %s",
                       data_str->data);
            return false;
        }

        return true;
    }

    /* DOCTYPE token */
    if (strcmp(type_name, "DOCTYPE") == 0) {
        if (token->tag_id != LXB_TAG__EM_DOCTYPE) {
            print_fail(kv, expected, "Expected DOCTYPE token, got tag_id=%d",
                       (int) token->tag_id);
            return false;
        }

        /*
         * items[1] = name (string or null)
         * items[2] = public_id (string or null)
         * items[3] = system_id (string or null)
         * items[4] = correctness (bool): true = no quirks, false = quirks
         */

        attr = token->attr_first;

        /* Check name */
        if (unit_kv_is_null(arr->list[1])) {
            if (attr != NULL && attr->name_begin != NULL) {
                print_fail(kv, expected,
                           "DOCTYPE name should be null but is not");
                return false;
            }
        }
        else if (data_str != NULL) {
            if (attr == NULL) {
                print_fail(kv, expected, "DOCTYPE has no attributes (no name)");
                return false;
            }

            size_t name_len;
            const lxb_char_t *name_data =
            lxb_html_token_attr_name(attr, &name_len);

            if (name_len != data_str->length
                || lexbor_str_data_ncasecmp(name_data, data_str->data,
                                            data_str->length) == false)
            {
                print_fail(kv, expected, "DOCTYPE name mismatch. "
                           "Have: %.*s, Want: %.*s",
                           (int) name_len, name_data,
                           (int) data_str->length, data_str->data);
                return false;
            }
        }

        /* Check public_id (items[2]) */
        if (arr->length >= 3) {
            unit_kv_value_t *pub = arr->list[2];
            lxb_html_token_attr_t *pub_attr = (attr != NULL) ? attr->next
            : NULL;

            if (!unit_kv_is_null(pub)) {
                lexbor_str_t *pub_str = kv_str(kv, pub, "Public");

                if (pub_str != NULL) {
                    if (pub_attr == NULL || pub_attr->value == NULL) {
                        print_fail(kv, expected,
                                   "DOCTYPE public_id should be '%.*s' "
                                   "but is missing",
                                   (int) pub_str->length, pub_str->data);
                        return false;
                    }

                    if (pub_attr->value_size != pub_str->length
                        || memcmp(pub_attr->value, pub_str->data,
                                  pub_str->length) != 0)
                    {
                        print_fail(kv, expected, "DOCTYPE public_id mismatch. "
                                   "Have: %.*s, Want: %.*s",
                                   (int) pub_attr->value_size, pub_attr->value,
                                   (int) pub_str->length, pub_str->data);
                        return false;
                    }
                }
            }

            /* Check system_id (items[3]) */
            if (arr->length >= 4) {
                unit_kv_value_t *sys = arr->list[3];

                if (!unit_kv_is_null(sys)) {
                    lexbor_str_t *sys_str = kv_str(kv, sys, "System");
                    lxb_html_token_attr_t *sys_attr = NULL;

                    if (pub_attr != NULL) {
                        sys_attr = pub_attr->next;
                    }

                    if (unit_kv_is_null(pub) && attr != NULL) {
                        sys_attr = attr->next;
                    }

                    if (sys_str != NULL) {
                        if (sys_attr == NULL || sys_attr->value == NULL) {
                            print_fail(kv, expected, "DOCTYPE system_id should be '%.*s' "
                                       "but is missing",
                                       (int) sys_str->length, sys_str->data);
                            return false;
                        }

                        if (sys_attr->value_size != sys_str->length
                            || memcmp(sys_attr->value, sys_str->data,
                                      sys_str->length) != 0)
                        {
                            print_fail(kv, expected, "DOCTYPE system_id mismatch. "
                                       "Have: %.*s, Want: %.*s",
                                       (int) sys_attr->value_size,
                                       sys_attr->value,
                                       (int) sys_str->length, sys_str->data);
                            return false;
                        }
                    }
                }
            }

            /* Check correctness/force-quirks (items[4]) */
            if (arr->length >= 5 && unit_kv_is_bool(arr->list[4])) {
                bool has_quirks =
                (token->type & LXB_HTML_TOKEN_TYPE_FORCE_QUIRKS) != 0;

                if (unit_kv_bool(arr->list[4]) && has_quirks) {
                    print_fail(kv, expected, "DOCTYPE should not have force-quirks "
                               "but does");
                    return false;
                }

                if (!unit_kv_bool(arr->list[4]) && !has_quirks) {
                    print_fail(kv, expected, "DOCTYPE should have force-quirks "
                               "but doesn't");
                    return false;
                }
            }
        }

        return true;
    }

    print_fail(kv, expected, "Unknown token type: %s", type_name);

    return false;
}

static bool
compare_errors(unit_kv_t *kv, lxb_html_tokenizer_t *tkz,
               unit_kv_value_t *errors)
{
    unsigned tkz_err_len;
    lexbor_str_t *code_str;
    unit_kv_value_t *err, *code_val;
    unit_kv_array_t *err_arr;
    lxb_html_tokenizer_error_t *actual_err;
    lxb_html_tokenizer_error_id_t expected_id;

    tkz_err_len = (unsigned) lexbor_array_obj_length(tkz->parse_errors);

    if (errors == NULL || unit_kv_is_null(errors)) {
        if (tkz_err_len > 0) {
            if (errors != NULL) {
                print_fail(kv, errors, "Expected no errors, but tokenizer "
                           "produced %u errors", tkz_err_len);
            }
            else {
                TEST_PRINTLN("Expected no errors, but tokenizer produced %u "
                             "errors", tkz_err_len);
            }

            return false;
        }

        return true;
    }

    if (!unit_kv_is_array(errors)) {
        print_fail(kv, errors, "Errors field is not an array");
        exit(EXIT_FAILURE);
    }

    err_arr = unit_kv_array(errors);

    if (err_arr->length != tkz_err_len) {
        print_fail(kv, errors, "Error count mismatch. Have: %u, Want: %u",
                   tkz_err_len, err_arr->length);
        return false;
    }

    for (unsigned i = 0; i < err_arr->length; i++) {
        err = err_arr->list[i];

        if (!unit_kv_is_hash(err)) {
            print_fail(kv, err, "Error entry %u is not an object", i);
            return false;
        }

        code_val = unit_kv_hash_value_nolen_c(err, "code");
        code_str = kv_str(kv, code_val, "code");

        if (code_str == NULL) {
            print_fail(kv, err, "Error entry %u missing 'code'", i);
            return false;
        }

        expected_id = error_code_by_name((const char *) code_str->data);

        if (expected_id == LXB_HTML_TOKENIZER_ERROR_LAST_ENTRY) {
            print_fail(kv, code_val, "Unknown error code: %s", code_str->data);
            return false;
        }

        actual_err = lexbor_array_obj_get(tkz->parse_errors, i);

        if (actual_err == NULL) {
            print_fail(kv, code_val, "NULL parse error at position %u", i);
            return false;
        }

        if (actual_err->id != expected_id) {
            print_fail(kv, code_val,
                       "Error code mismatch at %u. Have: %d, Want: %d (%s)",
                       i, actual_err->id, expected_id,
                       (const char *) code_str->data);
            return false;
        }
    }

    return true;
}

static void
double_unescape(lxb_html_tokenizer_t *tkz, const lxb_char_t *data, size_t length,
                lexbor_str_t *out)
{
    size_t len;
    uint32_t cp;
    const lxb_char_t *end;
    lxb_char_t buf[4];

    lexbor_str_init(out, tkz->mraw, length);
    if (out->data == NULL) {
        TEST_FAILURE("Failed to initialize output string");
    }

    end = data + length;

    while (data < end) {
        if (*data == '\\' && data + 6 <= end && data[1] == 'u') {
            cp = 0;
            data += 2;

            for (int j = 0; j < 4; j++) {
                cp = (cp << 4) | hex_digit(*data++);
            }

            len = encode_utf8(cp, buf);
            if (len == 0) {
                TEST_FAILURE("Invalid Unicode code point: U+%X", cp);
            }

            lexbor_str_append(out, tkz->mraw, buf, len);
        }
        else {
            lexbor_str_append_one(out, tkz->mraw, *data++);
        }
    }
}

static unsigned
hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';

    TEST_FAILURE("Failed convert hex digit: invalid character '%c'", c);
}

static size_t
encode_utf8(uint32_t cp, lxb_char_t *buf)
{
    if (cp <= 0x7F) {
        buf[0] = (lxb_char_t) cp;
        return 1;
    }
    else if (cp <= 0x7FF) {
        buf[0] = (lxb_char_t) (0xC0 | (cp >> 6));
        buf[1] = (lxb_char_t) (0x80 | (cp & 0x3F));
        return 2;
    }
    else if (cp <= 0xFFFF) {
        buf[0] = (lxb_char_t) (0xE0 | (cp >> 12));
        buf[1] = (lxb_char_t) (0x80 | ((cp >> 6) & 0x3F));
        buf[2] = (lxb_char_t) (0x80 | (cp & 0x3F));
        return 3;
    }
    else if (cp <= 0x10FFFF) {
        buf[0] = (lxb_char_t) (0xF0 | (cp >> 18));
        buf[1] = (lxb_char_t) (0x80 | ((cp >> 12) & 0x3F));
        buf[2] = (lxb_char_t) (0x80 | ((cp >> 6) & 0x3F));
        buf[3] = (lxb_char_t) (0x80 | (cp & 0x3F));
        return 4;
    }

    return 0;
}

static bool
set_initial_state(lxb_html_tokenizer_t *tkz, const char *state_name,
                  const char *last_start_tag)
{
    lxb_tag_id_t tag_id = LXB_TAG__UNDEF;

    if (last_start_tag != NULL && last_start_tag[0] != '\0') {
        tag_id = lxb_tag_id_by_name(tkz->tags,
                                    (const lxb_char_t *) last_start_tag,
                                    strlen(last_start_tag));
    }

    if (strcmp(state_name, "Data state") == 0) {
        tkz->state = lxb_html_tokenizer_state_data_before;
        return true;
    }

    if (strcmp(state_name, "PLAINTEXT state") == 0) {
        tkz->state = lxb_html_tokenizer_state_plaintext_before;
        return true;
    }

    if (strcmp(state_name, "RCDATA state") == 0) {
        if (tag_id != LXB_TAG__UNDEF) {
            tkz->tmp_tag_id = tag_id;
        }

        tkz->state = lxb_html_tokenizer_state_rcdata_before;
        return true;
    }

    if (strcmp(state_name, "RAWTEXT state") == 0) {
        if (tag_id != LXB_TAG__UNDEF) {
            tkz->tmp_tag_id = tag_id;
        }

        tkz->state = lxb_html_tokenizer_state_rawtext_before;
        return true;
    }

    if (strcmp(state_name, "Script data state") == 0) {
        if (tag_id != LXB_TAG__UNDEF) {
            tkz->tmp_tag_id = tag_id;
        }

        tkz->state = lxb_html_tokenizer_state_script_data_before;
        return true;
    }

    if (strcmp(state_name, "CDATA section state") == 0) {
        tkz->state = lxb_html_tokenizer_state_cdata_section_before;
        return true;
    }

    return false;
}

static lxb_html_tokenizer_error_id_t
error_code_by_name(const char *name)
{
    for (size_t i = 0; error_map[i].name != NULL; i++) {
        if (strcmp(error_map[i].name, name) == 0) {
            return error_map[i].id;
        }
    }

    return LXB_HTML_TOKENIZER_ERROR_LAST_ENTRY;
}


static lexbor_str_t *
kv_str(unit_kv_t *kv, unit_kv_value_t *val, const char *name)
{
    if (!unit_kv_is_string(val)) {
        TEST_PRINTLN("Parameter '%s' must be STRING", name);
        print_error(kv, val);
        exit(EXIT_FAILURE);
    }

    return unit_kv_string(val);
}

static const unit_kv_array_t *
kv_array(unit_kv_t *kv, unit_kv_value_t *val, const char *name)
{
    if (!unit_kv_is_array(val)) {
        TEST_PRINTLN("Parameter '%s' must be ARRAY", name);
        print_error(kv, val);
        exit(EXIT_FAILURE);
    }

    return unit_kv_array(val);
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

static void
print_fail(unit_kv_t *kv, unit_kv_value_t *val, const char *fmt, ...)
{
    va_list args;

    printf("Failed:\n");

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    if (val != NULL) {
        print_error(kv, val);
    }
}
