/*
 * Copyright (C) 2021-2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/css/property.h"
#include "lexbor/css/parser.h"
#include "lexbor/css/rule.h"
#include "lexbor/css/value.h"
#include "lexbor/css/unit.h"
#include "lexbor/css/property/state.h"
#include "lexbor/css/property/res.h"


bool
lxb_css_property_state__undef(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_parser_failed(parser);
}

bool
lxb_css_property_state__custom(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_status_t status;
    lxb_css_rule_declaration_t *declar = ctx;
    lxb_css_property__custom_t *custom = declar->u.custom;

    (void) lexbor_str_init(&custom->value, parser->memory->mraw, 0);
    if (custom->value.data == NULL) {
        return lxb_css_parser_memory_fail(parser);
    }

    while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__TERMINATED) {
        status = lxb_css_syntax_token_serialize_str(token, &custom->value,
                                                    parser->memory->mraw);
        if (status != LXB_STATUS_OK) {
            return lxb_css_parser_memory_fail(parser);
        }

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token(parser);
    }

    return lxb_css_parser_success(parser);
}

bool
lxb_css_property_state_width(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_value_type_t type;
    const lxb_css_data_t *unit;
    lxb_css_property_width_t *width;
    lxb_css_rule_declaration_t *declar = ctx;

    width = declar->u.width;

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_DIMENSION:
            unit = lxb_css_unit_absolute_relative_by_name(lxb_css_syntax_token_dimension(token)->str.data,
                                                          lxb_css_syntax_token_dimension(token)->str.length);
            if (unit == NULL) {
                return lxb_css_parser_failed(parser);
            }

            width->type = LXB_CSS_VALUE__LENGTH;
            width->u.number.num = lxb_css_syntax_token_dimension(token)->num.num;
            width->u.number.is_float = lxb_css_syntax_token_dimension(token)->num.is_float;
            width->u.number.unit = (lxb_css_unit_t) unit->unique;
            break;

        case LXB_CSS_SYNTAX_TOKEN_NUMBER:
            if (lxb_css_syntax_token_number(token)->num != 0) {
                return lxb_css_parser_failed(parser);
            }

            width->type = LXB_CSS_VALUE__LENGTH;
            width->u.number.num = lxb_css_syntax_token_number(token)->num;
            width->u.number.is_float = lxb_css_syntax_token_number(token)->is_float;
            break;

        case LXB_CSS_SYNTAX_TOKEN_PERCENTAGE:
            width->type = LXB_CSS_VALUE__PERCENTAGE;
            width->u.percentage.num = lxb_css_syntax_token_percentage(token)->num;
            width->u.percentage.is_float = lxb_css_syntax_token_percentage(token)->is_float;
            break;

        case LXB_CSS_SYNTAX_TOKEN_IDENT:
            type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data,
                                         lxb_css_syntax_token_ident(token)->length);
            switch (type) {
                case LXB_CSS_VALUE_INITIAL:
                case LXB_CSS_VALUE_INHERIT:
                case LXB_CSS_VALUE_UNSET:
                case LXB_CSS_VALUE_REVERT:
                case LXB_CSS_VALUE_AUTO:
                case LXB_CSS_VALUE_MIN_CONTENT:
                case LXB_CSS_VALUE_MAX_CONTENT:
                    width->type = type;
                    break;

                default:
                    return lxb_css_parser_failed(parser);
            }

            break;

        default:
            return lxb_css_parser_failed(parser);
    }

    lxb_css_syntax_parser_consume(parser);

    return lxb_css_parser_success(parser);
}

bool
lxb_css_property_state_height(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_width(parser, token, ctx);
}
