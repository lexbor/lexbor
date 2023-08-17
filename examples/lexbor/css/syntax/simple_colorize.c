/*
 * Copyright (C) 2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "../base.h"

#include <lexbor/core/fs.h>
#include <lexbor/css/css.h>

static lxb_status_t
css_parse(lxb_css_parser_t *parser, const lxb_char_t *data, size_t length);

static bool
css_list_rules_state(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx);

static bool
css_list_rules_next(lxb_css_parser_t *parser,
                    const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_list_rules_end(lxb_css_parser_t *parser,
                   const lxb_css_syntax_token_t *token, void *ctx, bool failed);

static bool
css_at_rule_state(lxb_css_parser_t *parser,
                  const lxb_css_syntax_token_t *token, void *ctx);

static bool
css_at_rule_block(lxb_css_parser_t *parser,
                  const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_at_rule_end(lxb_css_parser_t *parser, const lxb_css_syntax_token_t *token,
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

typedef struct {
    const lxb_char_t *data;
    uintptr_t        offset;
}
css_ctx_t;


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
    css_ctx_t ctx;
    lxb_css_syntax_rule_t *stack;

    ctx.data = data;
    ctx.offset = 0;

    lxb_css_parser_buffer_set(parser, data, length);

    stack = lxb_css_syntax_parser_list_rules_push(parser, NULL, NULL,
                                                  &css_list_rules,
                                                  &ctx, true,
                                                  LXB_CSS_SYNTAX_TOKEN_UNDEF);
    if (stack == NULL) {
        return LXB_STATUS_ERROR;
    }

    printf("\n");

    return lxb_css_syntax_parser_run(parser);
}

lxb_status_t
token_cb_f(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, data);

    return LXB_STATUS_OK;
}

lxb_inline void
css_consule_tokens(lxb_css_parser_t *parser,
                   const lxb_css_syntax_token_t *token, void *ctx)
{
    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        (void) lxb_css_syntax_token_serialize(token, token_cb_f, ctx);

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }
}

lxb_inline void
css_print_token(const lxb_css_syntax_token_t *token, void *ctx)
{
    css_ctx_t *context = ctx;

    printf("%.*s", (int) token->types.base.length,
           &context->data[token->offset]);
}

lxb_inline void
css_print_token_offset(const lxb_css_syntax_token_t *token, void *ctx)
{
    css_ctx_t *context = ctx;

    if (token->offset <= context->offset) {
        return;
    }

    printf("%.*s", (int) (token->offset - context->offset),
           &context->data[context->offset]);
}

static bool
css_list_rules_state(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx)
{
    printf("\033[32m");

    return lxb_css_parser_success(parser);
}

static bool
css_list_rules_next(lxb_css_parser_t *parser,
                    const lxb_css_syntax_token_t *token, void *ctx)
{
    printf("\033[32m");

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_list_rules_end(lxb_css_parser_t *parser,
                   const lxb_css_syntax_token_t *token, void *ctx, bool failed)
{
    return LXB_STATUS_OK;
}

static bool
css_at_rule_state(lxb_css_parser_t *parser,
                  const lxb_css_syntax_token_t *token, void *ctx)
{
    css_print_token_offset(token, ctx);

    printf("\033[35m");
    css_print_token(token, ctx);

    /* Skip At-Rule name. */

    lxb_css_syntax_parser_consume(parser);
    token = lxb_css_syntax_parser_token(parser);

    printf("\033[33m");

    css_consule_tokens(parser, token, ctx);

    printf("\033[39m");

    return lxb_css_parser_success(parser);
}

static bool
css_at_rule_block(lxb_css_parser_t *parser,
                  const lxb_css_syntax_token_t *token, void *ctx)
{
    printf("\033[33m{\033[39m");

    css_consule_tokens(parser, token, ctx);

    printf("\033[33m}\033[39m");

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_at_rule_end(lxb_css_parser_t *parser, const lxb_css_syntax_token_t *token,
                void *ctx, bool failed)
{
    css_ctx_t *context = ctx;

    context->offset = token->offset + lxb_css_syntax_token_base(token)->length;

    return LXB_STATUS_OK;
}

static bool
css_qualified_rule_state(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token, void *ctx)
{
    css_print_token_offset(token, ctx);

    printf("\033[33m");

    css_consule_tokens(parser, token, ctx);

    printf("\033[39m");

    return lxb_css_parser_success(parser);
}

static bool
css_qualified_rule_block(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_syntax_rule_t *stack;
    css_ctx_t *context = ctx;

    context->offset = token->offset + lxb_css_syntax_token_base(token)->length;

    printf("\033[33m{\033[39m");

    if (token->type == LXB_CSS_SYNTAX_TOKEN__END) {
        return lxb_css_parser_success(parser);
    }

    stack = lxb_css_syntax_parser_declarations_push(parser, token,
                                                    css_qualified_rule_back,
                                                    &css_declarations, ctx,
                                                    LXB_CSS_SYNTAX_TOKEN_RC_BRACKET);
    if (stack == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    return true;
}

static bool
css_qualified_rule_back(lxb_css_parser_t *parser,
                        const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_qualified_rule_end(lxb_css_parser_t *parser,
                       const lxb_css_syntax_token_t *token,
                       void *ctx, bool failed)
{
    css_ctx_t *context = ctx;

    context->offset = token->offset + lxb_css_syntax_token_base(token)->length;

    printf("\033[33m}\033[39m");

    return LXB_STATUS_OK;
}

static bool
css_declarations_name(lxb_css_parser_t *parser,
                      const lxb_css_syntax_token_t *token, void *ctx)
{
    css_ctx_t *context = ctx;

    css_print_token_offset(token, ctx);

    context->offset = token->offset + lxb_css_syntax_token_base(token)->length;

    printf("\033[31m");

    css_consule_tokens(parser, token, ctx);

    printf("\033[39m");

    return lxb_css_parser_success(parser);
}

static bool
css_declarations_value(lxb_css_parser_t *parser,
                       const lxb_css_syntax_token_t *token, void *ctx)
{
    css_ctx_t *context = ctx;

    css_print_token_offset(token, ctx);

    printf("\033[36m");

    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        context->offset = token->offset + lxb_css_syntax_token_base(token)->length;

        (void) lxb_css_syntax_token_serialize(token, token_cb_f, ctx);

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    printf("\033[39m");

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_declaration_end(lxb_css_parser_t *parser, void *ctx,
                    bool important, bool failed)
{
    return LXB_STATUS_OK;
}

static lxb_status_t
css_declarations_end(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token,
                     void *ctx, bool failed)
{
    css_ctx_t *context = ctx;

    context->offset = token->offset + lxb_css_syntax_token_base(token)->length;

    return LXB_STATUS_OK;
}

static bool
css_declarations_at_rule_state(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx)
{
    css_print_token_offset(token, ctx);

    printf("\033[35m");
    css_print_token(token, ctx);

    /* Skip At-Rule name. */

    lxb_css_syntax_parser_consume(parser);
    token = lxb_css_syntax_parser_token(parser);

    printf("\033[33m");

    css_consule_tokens(parser, token, ctx);

    printf("\033[39m");

    return lxb_css_parser_success(parser);
}

static bool
css_declarations_at_rule_block(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx)
{
    printf("\033[33m{\033[39m");

    css_consule_tokens(parser, token, ctx);

    printf("\033[33m}\033[39m");

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_declarations_at_rule_end(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, bool failed)
{
    css_ctx_t *context = ctx;

    context->offset = token->offset + lxb_css_syntax_token_base(token)->length;

    return LXB_STATUS_OK;
}

static bool
css_declarations_bad(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx)
{
    css_consule_tokens(parser, token, ctx);

    return lxb_css_parser_success(parser);
}
