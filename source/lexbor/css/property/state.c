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


#define lxb_css_property_state_check_token(parser, token)                     \
    if ((token) == NULL) {                                                    \
        return lxb_css_parser_memory_fail(parser);                            \
    }

#define lxb_css_property_state_get_type(parser, token, type)                  \
    do {                                                                      \
        lxb_css_syntax_parser_consume(parser);                                \
                                                                              \
        token = lxb_css_syntax_parser_token_wo_ws(parser);                    \
        lxb_css_property_state_check_token(parser, token);                    \
                                                                              \
        if (token->type != LXB_CSS_SYNTAX_TOKEN_IDENT) {                      \
            return lxb_css_parser_success(parser);                            \
        }                                                                     \
                                                                              \
        type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data, \
                                  lxb_css_syntax_token_ident(token)->length); \
    }                                                                         \
    while (false)


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
lxb_css_property_state_display(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_value_type_t type;
    lxb_css_property_display_t *display;
    lxb_css_rule_declaration_t *declar = ctx;

    if (token->type != LXB_CSS_SYNTAX_TOKEN_IDENT) {
        return lxb_css_parser_failed(parser);
    }

    display = declar->u.display;

    type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data,
                                 lxb_css_syntax_token_ident(token)->length);

    switch (type) {
        /* <display-outside> */
        case LXB_CSS_DISPLAY_BLOCK:
        case LXB_CSS_DISPLAY_INLINE:
        case LXB_CSS_DISPLAY_RUN_IN:
            display->a = type;
            goto inside_listitem;

        /* <display-inside> */
        case LXB_CSS_DISPLAY_FLOW:
        case LXB_CSS_DISPLAY_FLOW_ROOT:
            display->a = type;
            goto outside_listitem;

        case LXB_CSS_DISPLAY_TABLE:
        case LXB_CSS_DISPLAY_FLEX:
        case LXB_CSS_DISPLAY_GRID:
        case LXB_CSS_DISPLAY_RUBY:
            display->a = type;
            goto outside;

        /* <display-internal> */
        case LXB_CSS_DISPLAY_LIST_ITEM:
            display->a = type;
            goto listitem_only;

        /* <display-internal> */
        case LXB_CSS_DISPLAY_TABLE_ROW_GROUP:
        case LXB_CSS_DISPLAY_TABLE_HEADER_GROUP:
        case LXB_CSS_DISPLAY_TABLE_FOOTER_GROUP:
        case LXB_CSS_DISPLAY_TABLE_ROW:
        case LXB_CSS_DISPLAY_TABLE_CELL:
        case LXB_CSS_DISPLAY_TABLE_COLUMN_GROUP:
        case LXB_CSS_DISPLAY_TABLE_COLUMN:
        case LXB_CSS_DISPLAY_TABLE_CAPTION:
        case LXB_CSS_DISPLAY_RUBY_BASE:
        case LXB_CSS_DISPLAY_RUBY_TEXT:
        case LXB_CSS_DISPLAY_RUBY_BASE_CONTAINER:
        case LXB_CSS_DISPLAY_RUBY_TEXT_CONTAINER:
        /* <display-box> */
        case LXB_CSS_DISPLAY_CONTENTS:
        case LXB_CSS_DISPLAY_NONE:
        /* <display-legacy> */
        case LXB_CSS_DISPLAY_INLINE_BLOCK:
        case LXB_CSS_DISPLAY_INLINE_TABLE:
        case LXB_CSS_DISPLAY_INLINE_FLEX:
        case LXB_CSS_DISPLAY_INLINE_GRID:
            display->a = type;
            goto done;

        default:
            return lxb_css_parser_failed(parser);
    }

inside_listitem:

    lxb_css_property_state_get_type(parser, token, type);

    switch (type) {
        /* <display-inside> */
        case LXB_CSS_DISPLAY_FLOW:
        case LXB_CSS_DISPLAY_FLOW_ROOT:
            display->b = type;
            break;

        case LXB_CSS_DISPLAY_TABLE:
        case LXB_CSS_DISPLAY_FLEX:
        case LXB_CSS_DISPLAY_GRID:
        case LXB_CSS_DISPLAY_RUBY:
            display->b = type;
            goto done;

        case LXB_CSS_DISPLAY_LIST_ITEM:
            display->b = type;
            goto flow_only;

        default:
            return lxb_css_parser_failed(parser);
    }

listitem:

    lxb_css_property_state_get_type(parser, token, type);

    if (type == LXB_CSS_DISPLAY_LIST_ITEM) {
        display->c = type;
        goto done;
    }

    return lxb_css_parser_failed(parser);

outside:

    lxb_css_property_state_get_type(parser, token, type);

    switch (type) {
        /* <display-outside> */
        case LXB_CSS_DISPLAY_BLOCK:
        case LXB_CSS_DISPLAY_INLINE:
        case LXB_CSS_DISPLAY_RUN_IN:
            if (display->b == LXB_CSS_PROPERTY__UNDEF) {
                display->b = type;
            }
            else {
                display->c = type;
            }

            goto done;

        default:
            return lxb_css_parser_failed(parser);
    }

outside_listitem:

    lxb_css_property_state_get_type(parser, token, type);

    switch (type) {
        /* <display-outside> */
        case LXB_CSS_DISPLAY_BLOCK:
        case LXB_CSS_DISPLAY_INLINE:
        case LXB_CSS_DISPLAY_RUN_IN:
            display->b = type;
            goto listitem;

        case LXB_CSS_DISPLAY_LIST_ITEM:
            display->b = type;
            goto outside;

        default:
            return lxb_css_parser_failed(parser);
    }

listitem_only:

    lxb_css_property_state_get_type(parser, token, type);

    switch (type) {
        /* <display-outside> */
        case LXB_CSS_DISPLAY_BLOCK:
        case LXB_CSS_DISPLAY_INLINE:
        case LXB_CSS_DISPLAY_RUN_IN:
            display->b = type;
            break;

        /* <display-listitem> */
        case LXB_CSS_DISPLAY_FLOW:
        case LXB_CSS_DISPLAY_FLOW_ROOT:
            display->b = type;
            goto outside;

        default:
            return lxb_css_parser_failed(parser);
    }

flow_only:

    lxb_css_property_state_get_type(parser, token, type);

    switch (type) {
        /* <display-listitem> */
        case LXB_CSS_DISPLAY_FLOW:
        case LXB_CSS_DISPLAY_FLOW_ROOT:
            display->c = type;
            break;

        default:
            return lxb_css_parser_failed(parser);
    }

done:

    lxb_css_syntax_parser_consume(parser);

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
