/*
 * Copyright (C) 2022-2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "../base.h"

#include <lexbor/core/fs.h>
#include <lexbor/css/css.h>


static lxb_status_t
css_parse(lxb_css_parser_t *parser, const lxb_char_t *data, size_t length);

static bool
css_blank_list_rules_next(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_blank_list_rules_end(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token,
                         void *ctx, bool failed);

static const lxb_css_syntax_cb_at_rule_t *
css_blank_at_rule_begin(lxb_css_parser_t *parser,
                        const lxb_css_syntax_token_t *token, void *ctx,
                        void **out_rule);

static bool
css_blank_at_rule_prelude(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token,
                          void *ctx);

static lxb_status_t
css_blank_at_rule_prelude_end(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token,
                              void *ctx, bool failed);

static const lxb_css_syntax_cb_block_t *
css_blank_at_rule_block_begin(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token,
                              void *ctx, void **out_rule);
static bool
css_blank_at_rule_prelude_failed(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token,
                                 void *ctx);
static lxb_status_t
css_blank_at_rule_end(lxb_css_parser_t *parser,
                      const lxb_css_syntax_token_t *token,
                      void *ctx, bool failed);

static const lxb_css_syntax_cb_qualified_rule_t *
css_blank_qualified_rule_begin(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token,
                               void *ctx, void **out_rule);

static bool
css_blank_qualified_rule_prelude(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token,
                                 void *ctx);

static lxb_status_t
css_blank_qualified_rule_prelude_end(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token,
                                     void *ctx, bool failed);

static const lxb_css_syntax_cb_block_t *
css_blank_qualified_rule_block_begin(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token,
                                     void *ctx, void **out_rule);

static bool
css_blank_qualified_rule_prelude_failed(lxb_css_parser_t *parser,
                                        const lxb_css_syntax_token_t *token,
                                        void *ctx);
static lxb_status_t
css_blank_qualified_rule_end(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, bool failed);
static bool
css_blank_block_next(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_blank_block_end(lxb_css_parser_t *parser,
                    const lxb_css_syntax_token_t *token,
                    void *ctx, bool failed);

static const lxb_css_syntax_cb_declarations_t *
css_blank_declarations_begin(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, void **out_rule);

static lxb_css_parser_state_f
css_blank_declaration_name(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token,
                           void *ctx, void **out_rule);
static bool
css_blank_declaration_value(lxb_css_parser_t *parser,
                            const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_blank_declaration_end(lxb_css_parser_t *parser,
                          void *declarations, void *ctx,
                          const lxb_css_syntax_token_t *token,
                          lxb_css_syntax_declaration_offset_t *offset,
                          bool important, bool failed);

static lxb_status_t
css_blank_declarations_end(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token,
                           void *ctx, bool failed);

static bool
css_blank_declarations_bad(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token, void *ctx);


static const lxb_css_syntax_cb_list_rules_t lxb_css_blank_list_rules = {
    .at_rule = css_blank_at_rule_begin,
    .qualified_rule = css_blank_qualified_rule_begin,
    .next = css_blank_list_rules_next,
    .cb.failed = lxb_css_state_failed,
    .cb.end = css_blank_list_rules_end
};

static const lxb_css_syntax_cb_at_rule_t lxb_css_blank_at_rule = {
    .prelude = css_blank_at_rule_prelude,
    .prelude_end = css_blank_at_rule_prelude_end,
    .block = css_blank_at_rule_block_begin,
    .cb.failed = css_blank_at_rule_prelude_failed,
    .cb.end = css_blank_at_rule_end
};

static const lxb_css_syntax_cb_qualified_rule_t lxb_css_blank_qualified_rule = {
    .prelude = css_blank_qualified_rule_prelude,
    .prelude_end = css_blank_qualified_rule_prelude_end,
    .block = css_blank_qualified_rule_block_begin,
    .cb.failed = css_blank_qualified_rule_prelude_failed,
    .cb.end = css_blank_qualified_rule_end
};

static const lxb_css_syntax_cb_block_t lxb_css_blank_block = {
    .at_rule = css_blank_at_rule_begin,
    .declarations = css_blank_declarations_begin,
    .qualified_rule = css_blank_qualified_rule_begin,
    .next = css_blank_block_next,
    .cb.failed = lxb_css_state_failed,
    .cb.end = css_blank_block_end,
};

static const lxb_css_syntax_cb_declarations_t lxb_css_blank_declaration = {
    .name = css_blank_declaration_name,
    .end = css_blank_declaration_end,
    .cb.failed = css_blank_declarations_bad,
    .cb.end = css_blank_declarations_end
};


int
main(int argc, const char *argv[])
{
    size_t css_len;
    lxb_char_t *css;
    lxb_status_t status;
    lxb_css_parser_t *parser;
    const lxb_char_t *fl;

    if (argc != 2) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "\tcolorize <file>\n");
        FAILED("Invalid number of arguments");
    }

    fl = (const lxb_char_t *) argv[1];

    css = lexbor_fs_file_easy_read(fl, &css_len);
    if (css == NULL) {
        FAILED("Failed to read CSS file");
    }

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    if (status != LXB_STATUS_OK) {
        FAILED("Failed to create CSS Parser");
    }

    status = css_parse(parser, css, css_len);

    printf("\n");

    (void) lexbor_free(css);
    (void) lxb_css_parser_destroy(parser, true);

    if (status != LXB_STATUS_OK) {
        FAILED("Failed to parse CSS");
    }

    return EXIT_SUCCESS;
}

static lxb_status_t
css_parse(lxb_css_parser_t *parser, const lxb_char_t *data, size_t length)
{
    lxb_css_syntax_rule_t *rule;

    lxb_css_parser_buffer_set(parser, data, length);

    rule = lxb_css_syntax_parser_list_rules_push(parser,
                                                 &lxb_css_blank_list_rules,
                                                 NULL, NULL,
                                                 LXB_CSS_SYNTAX_TOKEN_UNDEF);
    if (rule == NULL) {
        return LXB_STATUS_ERROR;
    }

    return lxb_css_syntax_parser_run(parser);
}

lxb_status_t
token_cb_f(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, data);

    return LXB_STATUS_OK;
}

static void
css_consule_tokens(lxb_css_parser_t *parser,
                   const lxb_css_syntax_token_t *token)
{
    printf("\t");

    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        (void) lxb_css_syntax_token_serialize(token, token_cb_f, NULL);

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    printf("\n");
}

static bool
css_blank_list_rules_next(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_list_rules_end(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token,
                         void *ctx, bool failed)
{
    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_at_rule_t *
css_blank_at_rule_begin(lxb_css_parser_t *parser,
                        const lxb_css_syntax_token_t *token, void *ctx,
                        void **out_rule)
{
    PRINT("At-Rule Begin");

    printf("\t");
    (void) lxb_css_syntax_token_serialize(token, token_cb_f, NULL);
    printf("\n");

    return &lxb_css_blank_at_rule;
}

static bool
css_blank_at_rule_prelude(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token, void *ctx)
{
    PRINT("At-Rule Prelude Begin");

    css_consule_tokens(parser, token);

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_at_rule_prelude_end(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token,
                              void *ctx, bool failed)
{
    PRINT("At-Rule Prelude End");

    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_block_t *
css_blank_at_rule_block_begin(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token,
                              void *ctx, void **out_rule)
{
    PRINT("At-Rule Block Begin");

    return &lxb_css_blank_block;
}

static bool
css_blank_at_rule_prelude_failed(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token,
                                 void *ctx)
{
    /* We won't be able to get in here, it's just a formality. */
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_at_rule_end(lxb_css_parser_t *parser,
                      const lxb_css_syntax_token_t *token,
                      void *ctx, bool failed)
{
    PRINT("At-Rule End");

    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_qualified_rule_t *
css_blank_qualified_rule_begin(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token,
                               void *ctx, void **out_rule)
{
    PRINT("Qualified Rule Begin");

    return &lxb_css_blank_qualified_rule;
}

static bool
css_blank_qualified_rule_prelude(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token,
                                 void *ctx)
{
    PRINT("Qualified Rule Prelude Begin");

    css_consule_tokens(parser, token);

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_qualified_rule_prelude_end(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token,
                                     void *ctx, bool failed)
{
    PRINT("Qualified Rule Prelude End");

    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_block_t *
css_blank_qualified_rule_block_begin(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token,
                                     void *ctx, void **out_rule)
{
    PRINT("Qualified Rule Block Begin");

    return &lxb_css_blank_block;
}

static bool
css_blank_qualified_rule_prelude_failed(lxb_css_parser_t *parser,
                                        const lxb_css_syntax_token_t *token,
                                        void *ctx)
{
    /* We won't be able to get in here, it's just a formality. */
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_qualified_rule_end(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, bool failed)
{
    PRINT("Qualified Rule End");

    return LXB_STATUS_OK;
}

static bool
css_blank_block_next(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx)
{
    PRINT("Block Next");

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_block_end(lxb_css_parser_t *parser,
                    const lxb_css_syntax_token_t *token,
                    void *ctx, bool failed)
{
    PRINT("Block End");

    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_declarations_t *
css_blank_declarations_begin(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, void **out_rule)
{
    PRINT("Declarations Begin");

    return &lxb_css_blank_declaration;
}

static lxb_css_parser_state_f
css_blank_declaration_name(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token,
                           void *ctx, void **out_rule)
{
    PRINT("Declaration Name");

    printf("\t");
    (void) lxb_css_syntax_token_serialize(token, token_cb_f, NULL);
    printf("\n");

    return css_blank_declaration_value;
}

static bool
css_blank_declaration_value(lxb_css_parser_t *parser,
                            const lxb_css_syntax_token_t *token, void *ctx)
{
    PRINT("Declaration Value");

    css_consule_tokens(parser, token);

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_declaration_end(lxb_css_parser_t *parser,
                          void *declarations, void *ctx,
                          const lxb_css_syntax_token_t *token,
                          lxb_css_syntax_declaration_offset_t *offset,
                          bool important, bool failed)
{
    PRINT("Declaration End");

    return LXB_STATUS_OK;
}

static lxb_status_t
css_blank_declarations_end(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token,
                           void *ctx, bool failed)
{
    PRINT("Declarations End");

    return LXB_STATUS_OK;
}

static bool
css_blank_declarations_bad(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token, void *ctx)
{
    /* We won't be able to get in here, it's just a formality. */
    return lxb_css_parser_success(parser);
}
