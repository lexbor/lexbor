/*
 * Copyright (C) 2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <unit/test.h>
#include <unit/kv.h>

#include <lexbor/css/css.h>


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


static const lxb_char_t css[] = "#id .class {width: 123px; broken "
                                "declaration; @font 'http://x.x/' "
                                "{size: 10px} height: 45pt}";
static const size_t length = sizeof(css) - 1;

static const size_t qualified_len = sizeof("#id .class ") - 1;
static const size_t qualified_begin = 0;

static const size_t declaration_len[6] = {
    sizeof("width") - 1, sizeof("123px") - 1,
    sizeof("broken declaration") - 1, 0,
    sizeof("height") - 1, sizeof("45pt") - 1
};
static const size_t declaration_begin[6] = {
    12, 19,
    26, 0,
    79, 87
};

static const size_t declarations_len = sizeof("width: 123px; broken "
                                              "declaration; @font 'http://x.x/' "
                                              "{size: 10px} height: 45pt") - 1;
static const size_t declarations_begin = 12;

static const size_t at_rule_name_len = sizeof("@font") - 1;
static const size_t at_rule_name_begin = 46;

static const size_t at_rule_prelude_len = sizeof(" 'http://x.x/' ") - 1;
static const size_t at_rule_prelude_begin = 51;

static const size_t at_rule_block_len = sizeof("size: 10px") - 1;
static const size_t at_rule_block_begin = 67;

static const size_t qualified_block_len = sizeof("width: 123px; broken "
                                                 "declaration; @font 'http://x.x/' "
                                                 "{size: 10px} height: 45pt") - 1;
static const size_t qualified_block_begin = 12;


TEST_BEGIN(rules_offsets)
{
    lxb_status_t status;
    lxb_css_parser_t *parser;
    size_t i = 0;

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to create CSS Parser");
    }

    status = lxb_css_syntax_parse_list_rules(parser, &css_list_rules,
                                             css, length, &i, true);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to parse CSS");
    }



    (void) lxb_css_parser_destroy(parser, true);
}
TEST_END


int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(rules_offsets);

    TEST_RUN("lexbor/css/syntax/rules_offsets");

    TEST_RELEASE();
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
    return lxb_css_parser_success(parser);
}

static bool
css_at_rule_block(lxb_css_parser_t *parser,
                  const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_at_rule_end(lxb_css_parser_t *parser,
                const lxb_css_syntax_token_t *token,
                void *ctx, bool failed)
{
    return LXB_STATUS_OK;
}

static bool
css_qualified_rule_state(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_parser_success(parser);
}

static bool
css_qualified_rule_block(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_syntax_rule_t *rule;
    const lxb_css_syntax_qualified_offset_t *qua_off;

    qua_off = lxb_css_parser_qualified_rule_offset(parser);

    if (qualified_len != qua_off->prelude_end - qua_off->prelude) {
        TEST_FAILURE("Qualified prelude failed");
    }

    if (!lexbor_str_data_ncmp(&css[qualified_begin], &css[qua_off->prelude],
                              qualified_len))
    {
        TEST_FAILURE("Qualified prelude compare failed");
    }

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
    const lxb_css_syntax_qualified_offset_t *qua_off;

    qua_off = lxb_css_parser_qualified_rule_offset(parser);

    if (qualified_block_len != qua_off->block_end - qua_off->block) {
        TEST_FAILURE("Qualified block failed");
    }

    if (!lexbor_str_data_ncmp(&css[qualified_block_begin], &css[qua_off->block],
                              qualified_block_len))
    {
        TEST_FAILURE("Qualified block compare failed");
    }

    return LXB_STATUS_OK;
}

static bool
css_declarations_name(lxb_css_parser_t *parser,
                      const lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_syntax_parser_consume(parser);
    return lxb_css_parser_success(parser);
}

static bool
css_declarations_value(lxb_css_parser_t *parser,
                       const lxb_css_syntax_token_t *token, void *ctx)
{
    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_declaration_end(lxb_css_parser_t *parser, void *ctx,
                    bool important, bool failed)
{
    size_t i = *((size_t *) ctx);

    const lxb_css_syntax_declarations_offset_t *decs_off;

    decs_off = lxb_css_parser_declarations_offset(parser);

    if (declaration_len[i] != decs_off->name_end - decs_off->name_begin) {
        TEST_FAILURE("Declaration name failed");
    }

    if (!lexbor_str_data_ncmp(&css[declaration_begin[i]],
                              &css[decs_off->name_begin], declaration_len[i]))
    {
        TEST_FAILURE("Declaration name compare failed");
    }

    i++;

    if (declaration_len[i] != decs_off->value_end - decs_off->value_begin) {
        TEST_FAILURE("Declaration value failed");
    }

    if (!lexbor_str_data_ncmp(&css[declaration_begin[i]],
                              &css[decs_off->value_begin], declaration_len[i]))
    {
        TEST_FAILURE("Declaration value compare failed");
    }

    *((size_t *) ctx) = ++i;

    return LXB_STATUS_OK;
}

static lxb_status_t
css_declarations_end(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token,
                     void *ctx, bool failed)
{
    const lxb_css_syntax_declarations_offset_t *decs_off;

    decs_off = lxb_css_parser_declarations_offset(parser);

    if (declarations_len != decs_off->end - decs_off->begin) {
        TEST_FAILURE("Declarations failed");
    }

    if (!lexbor_str_data_ncmp(&css[declarations_begin], &css[decs_off->begin],
                              declarations_len))
    {
        TEST_FAILURE("Declarations compare failed");
    }

    return LXB_STATUS_OK;
}

static bool
css_declarations_at_rule_state(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx)
{
    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    return lxb_css_parser_success(parser);
}

static bool
css_declarations_at_rule_block(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx)
{
    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_declarations_at_rule_end(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, bool failed)
{
    const lxb_css_syntax_at_rule_offset_t *at_rule;

    at_rule = lxb_css_parser_at_rule_offset(parser);

    if (at_rule_name_len != at_rule->prelude - at_rule->name) {
        TEST_FAILURE("Declaration AT-Rule name failed");
    }

    if (!lexbor_str_data_ncmp(&css[at_rule_name_begin], &css[at_rule->name],
                              at_rule_name_len))
    {
        TEST_FAILURE("Declaration AT-Rule name compare failed");
    }

    if (at_rule_prelude_len != at_rule->prelude_end - at_rule->prelude) {
        TEST_FAILURE("Declaration AT-Rule prelude failed");
    }

    if (!lexbor_str_data_ncmp(&css[at_rule_prelude_begin], &css[at_rule->prelude],
                              at_rule_prelude_len))
    {
        TEST_FAILURE("Declaration AT-Rule prelude compare failed");
    }

    if (at_rule_block_len != at_rule->block_end - at_rule->block) {
        TEST_FAILURE("Declaration AT-Rule block failed");
    }

    if (!lexbor_str_data_ncmp(&css[at_rule_block_begin], &css[at_rule->block],
                              at_rule_block_len))
    {
        TEST_FAILURE("Declaration AT-Rule block compare failed");
    }

    return LXB_STATUS_OK;
}

static bool
css_declarations_bad(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx)
{
    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    return lxb_css_parser_success(parser);
}
