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

#define LEXBOR_STR_RES_MAP_HEX
#define LEXBOR_STR_RES_MAP_LOWERCASE
#include "lexbor/core/str_res.h"


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

#define LXB_CSS_PROPERTY_STATE_HEX_MASK(n)  ((((uint32_t) 1 << 31) - 1) << (n))


static bool
lxb_css_property_state_color_rgba_old(lxb_css_parser_t *parser,
                                      const lxb_css_syntax_token_t *token,
                                      lxb_css_value_color_t *color);
static bool
lxb_css_property_state_color_hsla_old(lxb_css_parser_t *parser,
                                      const lxb_css_syntax_token_t *token,
                                      lxb_css_value_color_hsla_t *hsl);


static bool
lxb_css_property_state_length_percentage(lxb_css_parser_t *parser,
                                         const lxb_css_syntax_token_t *token,
                                         lxb_css_value_length_percentage_t *lp)
{
    const lxb_css_data_t *unit;

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_DIMENSION:
            unit = lxb_css_unit_absolute_relative_by_name(lxb_css_syntax_token_dimension(token)->str.data,
                                                          lxb_css_syntax_token_dimension(token)->str.length);
            if (unit == NULL) {
                return false;
            }

            lp->type = LXB_CSS_VALUE__LENGTH;
            lp->u.length.num = lxb_css_syntax_token_dimension(token)->num.num;
            lp->u.length.is_float = lxb_css_syntax_token_dimension(token)->num.is_float;
            lp->u.length.unit = (lxb_css_unit_t) unit->unique;
            break;

        case LXB_CSS_SYNTAX_TOKEN_NUMBER:
            if (lxb_css_syntax_token_number(token)->num != 0) {
                return false;
            }

            lp->type = LXB_CSS_VALUE__NUMBER;
            lp->u.length.num = lxb_css_syntax_token_number(token)->num;
            lp->u.length.is_float = lxb_css_syntax_token_number(token)->is_float;
            lp->u.length.unit = LXB_CSS_UNIT__UNDEF;
            break;

        case LXB_CSS_SYNTAX_TOKEN_PERCENTAGE:
            lp->type = LXB_CSS_VALUE__PERCENTAGE;
            lp->u.percentage.num = lxb_css_syntax_token_percentage(token)->num;
            lp->u.percentage.is_float = lxb_css_syntax_token_percentage(token)->is_float;
            break;

        default:
            return false;
    }

    lxb_css_syntax_parser_consume(parser);

    return true;
}

static bool
lxb_css_property_state_hue(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token,
                           lxb_css_value_hue_t *hue)
{
    const lxb_css_data_t *unit;

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_DIMENSION:
            unit = lxb_css_unit_angel_by_name(lxb_css_syntax_token_dimension(token)->str.data,
                                              lxb_css_syntax_token_dimension(token)->str.length);
            if (unit == NULL) {
                return false;
            }

            hue->type = LXB_CSS_VALUE__ANGLE;
            hue->u.number.num = lxb_css_syntax_token_dimension(token)->num.num;
            hue->u.number.is_float = lxb_css_syntax_token_dimension(token)->num.is_float;
            break;

        case LXB_CSS_SYNTAX_TOKEN_NUMBER:
            hue->type = LXB_CSS_VALUE__NUMBER;
            hue->u.number.num = lxb_css_syntax_token_number(token)->num;
            hue->u.number.is_float = lxb_css_syntax_token_number(token)->is_float;
            break;

        default:
            return false;
    }

    lxb_css_syntax_parser_consume(parser);

    return true;
}

lxb_inline bool
lxb_css_property_state_hue_none(lxb_css_parser_t *parser,
                                const lxb_css_syntax_token_t *token,
                                lxb_css_value_hue_t *hue)
{
    lxb_css_value_type_t type;

    if (token->type != LXB_CSS_SYNTAX_TOKEN_IDENT) {
        return lxb_css_property_state_hue(parser, token, hue);
    }

    type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data,
                                 lxb_css_syntax_token_ident(token)->length);
    if (type != LXB_CSS_VALUE_NONE) {
        return false;
    }

    hue->type = LXB_CSS_VALUE_NONE;

    lxb_css_syntax_parser_consume(parser);

    return true;
}

static bool
lxb_css_property_state_color_hex(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token,
                                 lxb_css_value_color_t *color)
{
    size_t length;
    uint32_t chex;
    lxb_char_t ch;
    const lxb_char_t *end, *p;
    lxb_css_value_color_hex_rgba_t *rgba;

    length = token->types.hash.length;

    if (length > 8) {
        return false;
    }

    p = token->types.hash.data;
    end = p + length;

    chex = 0;

    while (p < end) {
        ch = lexbor_str_res_map_lowercase[lexbor_str_res_map_hex[*p]];

        if (ch == 0xff) {
            return false;
        }

        chex = chex << 4 | ch;

        p++;
    }

    rgba = &color->u.hex.rgba;

    switch (length) {
        case 3:
            rgba->r = chex >> 8;
            rgba->g = chex >> 4 & ~LXB_CSS_PROPERTY_STATE_HEX_MASK(4);
            rgba->b = chex & ~LXB_CSS_PROPERTY_STATE_HEX_MASK(4);
            rgba->a = 0xff;

            color->u.hex.type = LXB_CSS_PROPERTY_COLOR_HEX_TYPE_3;
            break;

        case 4:
            rgba->r = chex >> 12;
            rgba->g = chex >> 8 & ~LXB_CSS_PROPERTY_STATE_HEX_MASK(4);
            rgba->b = chex >> 4 & ~LXB_CSS_PROPERTY_STATE_HEX_MASK(4);
            rgba->a = chex & ~LXB_CSS_PROPERTY_STATE_HEX_MASK(4);

            color->u.hex.type = LXB_CSS_PROPERTY_COLOR_HEX_TYPE_4;
            break;

        case 6:
            rgba->r = chex >> 16;
            rgba->g = chex >> 8 & ~LXB_CSS_PROPERTY_STATE_HEX_MASK(8);
            rgba->b = chex & ~LXB_CSS_PROPERTY_STATE_HEX_MASK(8);
            rgba->a = 0xff;

            color->u.hex.type = LXB_CSS_PROPERTY_COLOR_HEX_TYPE_6;
            break;

        case 8:
            rgba->r = chex >> 24;
            rgba->g = chex >> 16 & ~LXB_CSS_PROPERTY_STATE_HEX_MASK(8);
            rgba->b = chex >> 8 & ~LXB_CSS_PROPERTY_STATE_HEX_MASK(8);
            rgba->a = chex & ~LXB_CSS_PROPERTY_STATE_HEX_MASK(8);

            color->u.hex.type = LXB_CSS_PROPERTY_COLOR_HEX_TYPE_8;
            break;

        default:
            return false;
    }

    color->type = LXB_CSS_COLOR_HEX;

    lxb_css_syntax_parser_consume(parser);

    return true;
}

lxb_inline bool
lxb_css_property_state_number_percentage_none(lxb_css_parser_t *parser,
                                              const lxb_css_syntax_token_t *token,
                                              lxb_css_value_number_percentage_t *np)
{
    double num;
    lxb_css_value_type_t type;

    if (token->type == LXB_CSS_SYNTAX_TOKEN_NUMBER) {
        np->type = LXB_CSS_VALUE__NUMBER;
    }
    else if (token->type == LXB_CSS_SYNTAX_TOKEN_PERCENTAGE) {
        np->type = LXB_CSS_VALUE__PERCENTAGE;
    }
    else if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
        type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data,
                                     lxb_css_syntax_token_ident(token)->length);
        if (type != LXB_CSS_VALUE_NONE) {
            return false;
        }

        np->type = LXB_CSS_VALUE_NONE;

        return true;
    }
    else {
        return false;
    }

    num = lxb_css_syntax_token_number(token)->num;

    np->u.number.num = num;
    np->u.number.is_float = lxb_css_syntax_token_number(token)->is_float;

    lxb_css_syntax_parser_consume(parser);

    return true;
}

lxb_inline bool
lxb_css_property_state_percentage_none(lxb_css_parser_t *parser,
                                       const lxb_css_syntax_token_t *token,
                                       lxb_css_value_percentage_type_t *np)
{
    double num;
    lxb_css_value_type_t type;

    if (token->type == LXB_CSS_SYNTAX_TOKEN_PERCENTAGE) {
        np->type = LXB_CSS_VALUE__PERCENTAGE;
    }
    else if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
        type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data,
                                     lxb_css_syntax_token_ident(token)->length);
        if (type != LXB_CSS_VALUE_NONE) {
            return false;
        }

        np->type = LXB_CSS_VALUE_NONE;

        return true;
    }
    else {
        return false;
    }

    num = lxb_css_syntax_token_number(token)->num;

    np->percentage.num = num;
    np->percentage.is_float = lxb_css_syntax_token_number(token)->is_float;

    lxb_css_syntax_parser_consume(parser);

    return true;
}

lxb_inline bool
lxb_css_property_state_number_percentage(lxb_css_parser_t *parser,
                                         const lxb_css_syntax_token_t *token,
                                         lxb_css_value_number_percentage_t *np)
{
    double num;

    if (token->type == LXB_CSS_SYNTAX_TOKEN_NUMBER) {
        np->type = LXB_CSS_VALUE__NUMBER;
    }
    else if (token->type == LXB_CSS_SYNTAX_TOKEN_PERCENTAGE) {
        np->type = LXB_CSS_VALUE__PERCENTAGE;
    }
    else {
        return false;
    }

    num = lxb_css_syntax_token_number(token)->num;

    np->u.number.num = num;
    np->u.number.is_float = lxb_css_syntax_token_number(token)->is_float;

    lxb_css_syntax_parser_consume(parser);

    return true;
}

lxb_inline bool
lxb_css_property_state_percentage(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token,
                                  lxb_css_value_percentage_t *np)
{
    double num;

    if (token->type != LXB_CSS_SYNTAX_TOKEN_PERCENTAGE) {
        return false;
    }

    num = lxb_css_syntax_token_number(token)->num;

    np->num = num;
    np->is_float = lxb_css_syntax_token_number(token)->is_float;

    lxb_css_syntax_parser_consume(parser);

    return true;
}

static bool
lxb_css_property_state_color_rgba(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token,
                                  lxb_css_value_color_t *color)
{
    bool res;
    lxb_css_color_type_t type;
    lxb_css_value_color_rgba_t *rgb;

    rgb = &color->u.rgb;

    lxb_css_syntax_parser_consume(parser);
    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_number_percentage_none(parser, token, &rgb->r);
    if (res == false) {
        return false;
    }

    type = rgb->r.type;

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_COMMA) {
        /* Deprecated format. */

        if (type == LXB_CSS_VALUE_NONE) {
            return false;
        }

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token_wo_ws(parser);
        lxb_css_property_state_check_token(parser, token);

        return lxb_css_property_state_color_rgba_old(parser, token, color);
    }

    res = lxb_css_property_state_number_percentage_none(parser, token, &rgb->g);
    if (res == false) {
        return false;
    }

    if (type != rgb->g.type) {
        if (type == LXB_CSS_VALUE_NONE) {
            type = rgb->g.type;
        }
        else if (rgb->g.type != LXB_CSS_VALUE_NONE) {
            return false;
        }
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_number_percentage_none(parser, token, &rgb->b);
    if (res == false) {
        return false;
    }

    if (type != rgb->b.type && type != LXB_CSS_VALUE_NONE
        && rgb->b.type != LXB_CSS_VALUE_NONE)
    {
            return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_DELIM) {
        if (lxb_css_syntax_token_delim(token)->character != '/') {
            return false;
        }

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token_wo_ws(parser);
        lxb_css_property_state_check_token(parser, token);
    }
    else if (token->type == LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        lxb_css_syntax_parser_consume(parser);
        return true;
    }
    else {
        return false;
    }

    res = lxb_css_property_state_number_percentage_none(parser, token, &rgb->a);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type != LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        return false;
    }

    lxb_css_syntax_parser_consume(parser);

    return true;
}

static bool
lxb_css_property_state_color_rgba_old(lxb_css_parser_t *parser,
                                      const lxb_css_syntax_token_t *token,
                                      lxb_css_value_color_t *color)
{
    bool res;
    lxb_css_value_color_rgba_t *rgb;

    rgb = &color->u.rgb;

    res = lxb_css_property_state_number_percentage(parser, token, &rgb->g);
    if (res == false) {
        return false;
    }

    if (rgb->r.type != rgb->g.type) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type != LXB_CSS_SYNTAX_TOKEN_COMMA) {
        return false;
    }

    lxb_css_syntax_parser_consume(parser);
    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_number_percentage(parser, token, &rgb->b);
    if (res == false) {
        return false;
    }

    if (rgb->r.type != rgb->b.type) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        lxb_css_syntax_parser_consume(parser);
        return true;
    }
    else if (token->type != LXB_CSS_SYNTAX_TOKEN_COMMA) {
        return false;
    }

    lxb_css_syntax_parser_consume(parser);
    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_number_percentage(parser, token, &rgb->a);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type != LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        return false;
    }

    lxb_css_syntax_parser_consume(parser);

    return true;
}

static bool
lxb_css_property_state_color_hsla(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token,
                                  lxb_css_value_color_t *color)
{
    bool res;
    lxb_css_value_color_hsla_t *hsl;

    lxb_css_syntax_parser_consume(parser);
    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    hsl = &color->u.hsl;

    res = lxb_css_property_state_hue_none(parser, token, &hsl->h);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_COMMA) {
        /* Deprecated format. */

        if (hsl->h.type == LXB_CSS_VALUE_NONE) {
            return false;
        }

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token_wo_ws(parser);
        lxb_css_property_state_check_token(parser, token);

        return lxb_css_property_state_color_hsla_old(parser, token, hsl);
    }

    res = lxb_css_property_state_percentage_none(parser, token, &hsl->s);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_percentage_none(parser, token, &hsl->l);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_DELIM) {
        if (lxb_css_syntax_token_delim(token)->character != '/') {
            return false;
        }

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token_wo_ws(parser);
        lxb_css_property_state_check_token(parser, token);
    }
    else if (token->type == LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        lxb_css_syntax_parser_consume(parser);
        return true;
    }
    else {
        return false;
    }

    res = lxb_css_property_state_number_percentage_none(parser, token, &hsl->a);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type != LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        return false;
    }

    lxb_css_syntax_parser_consume(parser);

    return true;
}

static bool
lxb_css_property_state_color_hsla_old(lxb_css_parser_t *parser,
                                      const lxb_css_syntax_token_t *token,
                                      lxb_css_value_color_hsla_t *hsl)
{
    bool res;

    res = lxb_css_property_state_percentage(parser, token, &hsl->s.percentage);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type != LXB_CSS_SYNTAX_TOKEN_COMMA) {
        return false;
    }

    lxb_css_syntax_parser_consume(parser);
    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_percentage(parser, token, &hsl->l.percentage);
    if (res == false) {
        return false;
    }

    lxb_css_syntax_parser_consume(parser);
    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        goto done;
    }
    else if (token->type != LXB_CSS_SYNTAX_TOKEN_COMMA) {
        return false;
    }

    lxb_css_syntax_parser_consume(parser);
    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_number_percentage(parser, token, &hsl->a);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type != LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        return false;
    }

done:

    lxb_css_syntax_parser_consume(parser);

    hsl->s.type = LXB_CSS_VALUE__PERCENTAGE;
    hsl->l.type = LXB_CSS_VALUE__PERCENTAGE;

    return true;
}

static bool
lxb_css_property_state_color_lab(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token,
                                 lxb_css_value_color_t *color)
{
    bool res;
    lxb_css_value_color_lab_t *lab;

    lab = &color->u.lab;

    lxb_css_syntax_parser_consume(parser);
    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_number_percentage_none(parser, token, &lab->l);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_number_percentage_none(parser, token, &lab->a);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_number_percentage_none(parser, token, &lab->b);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_DELIM) {
        if (lxb_css_syntax_token_delim(token)->character != '/') {
            return false;
        }

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token_wo_ws(parser);
        lxb_css_property_state_check_token(parser, token);
    }
    else if (token->type == LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        lxb_css_syntax_parser_consume(parser);
        return true;
    }
    else {
        return false;
    }

    res = lxb_css_property_state_number_percentage_none(parser, token,
                                                        &lab->alpha);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type != LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        return false;
    }

    lxb_css_syntax_parser_consume(parser);

    return true;
}

static bool
lxb_css_property_state_color_lch(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token,
                                 lxb_css_value_color_t *color)
{
    bool res;
    lxb_css_value_color_lch_t *lch;

    lch = &color->u.lch;

    lxb_css_syntax_parser_consume(parser);
    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_number_percentage_none(parser, token, &lch->l);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_number_percentage_none(parser, token, &lch->c);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    res = lxb_css_property_state_hue_none(parser, token, &lch->h);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN_DELIM) {
        if (lxb_css_syntax_token_delim(token)->character != '/') {
            return false;
        }

        lxb_css_syntax_parser_consume(parser);
        token = lxb_css_syntax_parser_token_wo_ws(parser);
        lxb_css_property_state_check_token(parser, token);
    }
    else if (token->type == LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        lxb_css_syntax_parser_consume(parser);
        return true;
    }
    else {
        return false;
    }

    res = lxb_css_property_state_number_percentage_none(parser, token, &lch->a);
    if (res == false) {
        return false;
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type != LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS) {
        return false;
    }

    lxb_css_syntax_parser_consume(parser);

    return true;
}

static bool
lxb_css_property_state_color(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             lxb_css_value_color_t *color)
{
    lxb_css_value_type_t type;

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_HASH:
            color->type = LXB_CSS_VALUE_HEX;

            return lxb_css_property_state_color_hex(parser, token, color);

        case LXB_CSS_SYNTAX_TOKEN_FUNCTION:
            type = lxb_css_value_by_name(lxb_css_syntax_token_function(token)->data,
                                         lxb_css_syntax_token_function(token)->length);
            color->type = type;

            switch (type) {
                /* <color> */
                case LXB_CSS_VALUE_RGB:
                case LXB_CSS_VALUE_RGBA:
                    return lxb_css_property_state_color_rgba(parser, token,
                                                             color);

                case LXB_CSS_VALUE_HSL:
                case LXB_CSS_VALUE_HSLA:
                case LXB_CSS_VALUE_HWB:
                    return lxb_css_property_state_color_hsla(parser, token,
                                                             color);

                case LXB_CSS_VALUE_LAB:
                case LXB_CSS_VALUE_OKLAB:
                    return lxb_css_property_state_color_lab(parser, token,
                                                            color);

                case LXB_CSS_VALUE_LCH:
                case LXB_CSS_VALUE_OKLCH:
                    return lxb_css_property_state_color_lch(parser, token,
                                                            color);

                case LXB_CSS_VALUE_COLOR:
                    return false;

                default:
                    return false;
            }

            break;

        case LXB_CSS_SYNTAX_TOKEN_IDENT:
            type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data,
                                         lxb_css_syntax_token_ident(token)->length);
            switch (type) {
                /* <color> */
                case LXB_CSS_VALUE_CURRENTCOLOR:
                /* <system-color> */
                case LXB_CSS_VALUE_CANVAS:
                case LXB_CSS_VALUE_CANVASTEXT:
                case LXB_CSS_VALUE_LINKTEXT:
                case LXB_CSS_VALUE_VISITEDTEXT:
                case LXB_CSS_VALUE_ACTIVETEXT:
                case LXB_CSS_VALUE_BUTTONFACE:
                case LXB_CSS_VALUE_BUTTONTEXT:
                case LXB_CSS_VALUE_BUTTONBORDER:
                case LXB_CSS_VALUE_FIELD:
                case LXB_CSS_VALUE_FIELDTEXT:
                case LXB_CSS_VALUE_HIGHLIGHT:
                case LXB_CSS_VALUE_HIGHLIGHTTEXT:
                case LXB_CSS_VALUE_SELECTEDITEM:
                case LXB_CSS_VALUE_SELECTEDITEMTEXT:
                case LXB_CSS_VALUE_MARK:
                case LXB_CSS_VALUE_MARKTEXT:
                case LXB_CSS_VALUE_GRAYTEXT:
                case LXB_CSS_VALUE_ACCENTCOLOR:
                case LXB_CSS_VALUE_ACCENTCOLORTEXT:
                /* <absolute-color-base> */
                case LXB_CSS_VALUE_TRANSPARENT:
                /* <named-color> */
                case LXB_CSS_VALUE_ALICEBLUE:
                case LXB_CSS_VALUE_ANTIQUEWHITE:
                case LXB_CSS_VALUE_AQUA:
                case LXB_CSS_VALUE_AQUAMARINE:
                case LXB_CSS_VALUE_AZURE:
                case LXB_CSS_VALUE_BEIGE:
                case LXB_CSS_VALUE_BISQUE:
                case LXB_CSS_VALUE_BLACK:
                case LXB_CSS_VALUE_BLANCHEDALMOND:
                case LXB_CSS_VALUE_BLUE:
                case LXB_CSS_VALUE_BLUEVIOLET:
                case LXB_CSS_VALUE_BROWN:
                case LXB_CSS_VALUE_BURLYWOOD:
                case LXB_CSS_VALUE_CADETBLUE:
                case LXB_CSS_VALUE_CHARTREUSE:
                case LXB_CSS_VALUE_CHOCOLATE:
                case LXB_CSS_VALUE_CORAL:
                case LXB_CSS_VALUE_CORNFLOWERBLUE:
                case LXB_CSS_VALUE_CORNSILK:
                case LXB_CSS_VALUE_CRIMSON:
                case LXB_CSS_VALUE_CYAN:
                case LXB_CSS_VALUE_DARKBLUE:
                case LXB_CSS_VALUE_DARKCYAN:
                case LXB_CSS_VALUE_DARKGOLDENROD:
                case LXB_CSS_VALUE_DARKGRAY:
                case LXB_CSS_VALUE_DARKGREEN:
                case LXB_CSS_VALUE_DARKGREY:
                case LXB_CSS_VALUE_DARKKHAKI:
                case LXB_CSS_VALUE_DARKMAGENTA:
                case LXB_CSS_VALUE_DARKOLIVEGREEN:
                case LXB_CSS_VALUE_DARKORANGE:
                case LXB_CSS_VALUE_DARKORCHID:
                case LXB_CSS_VALUE_DARKRED:
                case LXB_CSS_VALUE_DARKSALMON:
                case LXB_CSS_VALUE_DARKSEAGREEN:
                case LXB_CSS_VALUE_DARKSLATEBLUE:
                case LXB_CSS_VALUE_DARKSLATEGRAY:
                case LXB_CSS_VALUE_DARKSLATEGREY:
                case LXB_CSS_VALUE_DARKTURQUOISE:
                case LXB_CSS_VALUE_DARKVIOLET:
                case LXB_CSS_VALUE_DEEPPINK:
                case LXB_CSS_VALUE_DEEPSKYBLUE:
                case LXB_CSS_VALUE_DIMGRAY:
                case LXB_CSS_VALUE_DIMGREY:
                case LXB_CSS_VALUE_DODGERBLUE:
                case LXB_CSS_VALUE_FIREBRICK:
                case LXB_CSS_VALUE_FLORALWHITE:
                case LXB_CSS_VALUE_FORESTGREEN:
                case LXB_CSS_VALUE_FUCHSIA:
                case LXB_CSS_VALUE_GAINSBORO:
                case LXB_CSS_VALUE_GHOSTWHITE:
                case LXB_CSS_VALUE_GOLD:
                case LXB_CSS_VALUE_GOLDENROD:
                case LXB_CSS_VALUE_GRAY:
                case LXB_CSS_VALUE_GREEN:
                case LXB_CSS_VALUE_GREENYELLOW:
                case LXB_CSS_VALUE_GREY:
                case LXB_CSS_VALUE_HONEYDEW:
                case LXB_CSS_VALUE_HOTPINK:
                case LXB_CSS_VALUE_INDIANRED:
                case LXB_CSS_VALUE_INDIGO:
                case LXB_CSS_VALUE_IVORY:
                case LXB_CSS_VALUE_KHAKI:
                case LXB_CSS_VALUE_LAVENDER:
                case LXB_CSS_VALUE_LAVENDERBLUSH:
                case LXB_CSS_VALUE_LAWNGREEN:
                case LXB_CSS_VALUE_LEMONCHIFFON:
                case LXB_CSS_VALUE_LIGHTBLUE:
                case LXB_CSS_VALUE_LIGHTCORAL:
                case LXB_CSS_VALUE_LIGHTCYAN:
                case LXB_CSS_VALUE_LIGHTGOLDENRODYELLOW:
                case LXB_CSS_VALUE_LIGHTGRAY:
                case LXB_CSS_VALUE_LIGHTGREEN:
                case LXB_CSS_VALUE_LIGHTGREY:
                case LXB_CSS_VALUE_LIGHTPINK:
                case LXB_CSS_VALUE_LIGHTSALMON:
                case LXB_CSS_VALUE_LIGHTSEAGREEN:
                case LXB_CSS_VALUE_LIGHTSKYBLUE:
                case LXB_CSS_VALUE_LIGHTSLATEGRAY:
                case LXB_CSS_VALUE_LIGHTSLATEGREY:
                case LXB_CSS_VALUE_LIGHTSTEELBLUE:
                case LXB_CSS_VALUE_LIGHTYELLOW:
                case LXB_CSS_VALUE_LIME:
                case LXB_CSS_VALUE_LIMEGREEN:
                case LXB_CSS_VALUE_LINEN:
                case LXB_CSS_VALUE_MAGENTA:
                case LXB_CSS_VALUE_MAROON:
                case LXB_CSS_VALUE_MEDIUMAQUAMARINE:
                case LXB_CSS_VALUE_MEDIUMBLUE:
                case LXB_CSS_VALUE_MEDIUMORCHID:
                case LXB_CSS_VALUE_MEDIUMPURPLE:
                case LXB_CSS_VALUE_MEDIUMSEAGREEN:
                case LXB_CSS_VALUE_MEDIUMSLATEBLUE:
                case LXB_CSS_VALUE_MEDIUMSPRINGGREEN:
                case LXB_CSS_VALUE_MEDIUMTURQUOISE:
                case LXB_CSS_VALUE_MEDIUMVIOLETRED:
                case LXB_CSS_VALUE_MIDNIGHTBLUE:
                case LXB_CSS_VALUE_MINTCREAM:
                case LXB_CSS_VALUE_MISTYROSE:
                case LXB_CSS_VALUE_MOCCASIN:
                case LXB_CSS_VALUE_NAVAJOWHITE:
                case LXB_CSS_VALUE_NAVY:
                case LXB_CSS_VALUE_OLDLACE:
                case LXB_CSS_VALUE_OLIVE:
                case LXB_CSS_VALUE_OLIVEDRAB:
                case LXB_CSS_VALUE_ORANGE:
                case LXB_CSS_VALUE_ORANGERED:
                case LXB_CSS_VALUE_ORCHID:
                case LXB_CSS_VALUE_PALEGOLDENROD:
                case LXB_CSS_VALUE_PALEGREEN:
                case LXB_CSS_VALUE_PALETURQUOISE:
                case LXB_CSS_VALUE_PALEVIOLETRED:
                case LXB_CSS_VALUE_PAPAYAWHIP:
                case LXB_CSS_VALUE_PEACHPUFF:
                case LXB_CSS_VALUE_PERU:
                case LXB_CSS_VALUE_PINK:
                case LXB_CSS_VALUE_PLUM:
                case LXB_CSS_VALUE_POWDERBLUE:
                case LXB_CSS_VALUE_PURPLE:
                case LXB_CSS_VALUE_REBECCAPURPLE:
                case LXB_CSS_VALUE_RED:
                case LXB_CSS_VALUE_ROSYBROWN:
                case LXB_CSS_VALUE_ROYALBLUE:
                case LXB_CSS_VALUE_SADDLEBROWN:
                case LXB_CSS_VALUE_SALMON:
                case LXB_CSS_VALUE_SANDYBROWN:
                case LXB_CSS_VALUE_SEAGREEN:
                case LXB_CSS_VALUE_SEASHELL:
                case LXB_CSS_VALUE_SIENNA:
                case LXB_CSS_VALUE_SILVER:
                case LXB_CSS_VALUE_SKYBLUE:
                case LXB_CSS_VALUE_SLATEBLUE:
                case LXB_CSS_VALUE_SLATEGRAY:
                case LXB_CSS_VALUE_SLATEGREY:
                case LXB_CSS_VALUE_SNOW:
                case LXB_CSS_VALUE_SPRINGGREEN:
                case LXB_CSS_VALUE_STEELBLUE:
                case LXB_CSS_VALUE_TAN:
                case LXB_CSS_VALUE_TEAL:
                case LXB_CSS_VALUE_THISTLE:
                case LXB_CSS_VALUE_TOMATO:
                case LXB_CSS_VALUE_TURQUOISE:
                case LXB_CSS_VALUE_VIOLET:
                case LXB_CSS_VALUE_WHEAT:
                case LXB_CSS_VALUE_WHITE:
                case LXB_CSS_VALUE_WHITESMOKE:
                case LXB_CSS_VALUE_YELLOW:
                case LXB_CSS_VALUE_YELLOWGREEN:
                    color->type = type;
                    break;

                default:
                    return false;
            }

            break;

        default:
            return false;
    }

    lxb_css_syntax_parser_consume(parser);

    return true;
}

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

        /* Global. */
        case LXB_CSS_VALUE_INITIAL:
        case LXB_CSS_VALUE_INHERIT:
        case LXB_CSS_VALUE_UNSET:
        case LXB_CSS_VALUE_REVERT:
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
    lxb_css_rule_declaration_t *declar = ctx;

    if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
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
                declar->u.width->type = type;
                break;

            default:
                return lxb_css_parser_failed(parser);
        }

        lxb_css_syntax_parser_consume(parser);

        return lxb_css_parser_success(parser);
    }

    if (!lxb_css_property_state_length_percentage(parser, token,
                                                  declar->u.user))
    {
        return lxb_css_parser_failed(parser);
    }

    return lxb_css_parser_success(parser);
}

bool
lxb_css_property_state_height(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_width(parser, token, ctx);
}

bool
lxb_css_property_state_box_sizing(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token, void *ctx)
{
    lxb_css_value_type_t type;
    lxb_css_rule_declaration_t *declar = ctx;

    if (token->type != LXB_CSS_SYNTAX_TOKEN_IDENT) {
        return lxb_css_parser_failed(parser);
    }

    type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data,
                                 lxb_css_syntax_token_ident(token)->length);
    switch (type) {
        case LXB_CSS_VALUE_INITIAL:
        case LXB_CSS_VALUE_INHERIT:
        case LXB_CSS_VALUE_UNSET:
        case LXB_CSS_VALUE_REVERT:
        case LXB_CSS_VALUE_CONTENT_BOX:
        case LXB_CSS_VALUE_BORDER_BOX:
            declar->u.box_sizing->type = type;
            break;

        default:
            return lxb_css_parser_failed(parser);
    }

    lxb_css_syntax_parser_consume(parser);

    return lxb_css_parser_success(parser);
}

bool
lxb_css_property_state_min_width(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_width(parser, token, ctx);
}

bool
lxb_css_property_state_min_height(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_width(parser, token, ctx);
}

bool
lxb_css_property_state_max_width(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_width(parser, token, ctx);
}

bool
lxb_css_property_state_max_height(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_width(parser, token, ctx);
}

static bool
lxb_css_property_state_mp(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token,
                          lxb_css_rule_declaration_t *declar, bool with_auto)
{
    unsigned int state;
    lxb_css_value_type_t type;
    lxb_css_property_margin_top_t *top;

    state = 1;

next:

    switch (state) {
        case 1:
            top = &declar->u.margin->top;
            break;

        case 2:
            top = &declar->u.margin->right;
            break;

        case 3:
            top = &declar->u.margin->bottom;
            break;

        case 4:
            top = &declar->u.margin->left;
            break;

        default:
            return lxb_css_parser_failed(parser);
    }

    if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
        type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data,
                                     lxb_css_syntax_token_ident(token)->length);
        switch (type) {
            case LXB_CSS_VALUE_INITIAL:
            case LXB_CSS_VALUE_INHERIT:
            case LXB_CSS_VALUE_UNSET:
            case LXB_CSS_VALUE_REVERT:
                top->type = type;
                break;

            case LXB_CSS_VALUE_AUTO:
                if (with_auto) {
                    top->type = type;
                    break;
                }

                /* Fall through. */

            default:
                return lxb_css_parser_failed(parser);
        }

        lxb_css_syntax_parser_consume(parser);
    }
    else if (!lxb_css_property_state_length_percentage(parser, token,
                                    (lxb_css_value_length_percentage_t *) top))
    {
        return lxb_css_parser_failed(parser);
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN__TERMINATED) {
        return lxb_css_parser_success(parser);
    }

    state++;

    goto next;
}

static bool
lxb_css_property_state_mp_top(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token,
                              lxb_css_rule_declaration_t *declar, bool with_auto)
{
    lxb_css_value_type_t type;

    if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
        type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data,
                                     lxb_css_syntax_token_ident(token)->length);
        switch (type) {
            case LXB_CSS_VALUE_INITIAL:
            case LXB_CSS_VALUE_INHERIT:
            case LXB_CSS_VALUE_UNSET:
            case LXB_CSS_VALUE_REVERT:
                declar->u.margin_top->type = type;
                break;

            case LXB_CSS_VALUE_AUTO:
                if (with_auto) {
                    declar->u.margin_top->type = type;
                    break;
                }

                /* Fall through. */

            default:
                return lxb_css_parser_failed(parser);
        }

        lxb_css_syntax_parser_consume(parser);

        return lxb_css_parser_success(parser);
    }

    if (!lxb_css_property_state_length_percentage(parser, token,
                                                  declar->u.user))
    {
        return lxb_css_parser_failed(parser);
    }

    return lxb_css_parser_success(parser);
}

bool
lxb_css_property_state_margin(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_mp(parser, token, ctx, true);
}

bool
lxb_css_property_state_margin_top(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_mp_top(parser, token, ctx, true);
}

bool
lxb_css_property_state_margin_right(lxb_css_parser_t *parser,
                                    const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_mp_top(parser, token, ctx, true);
}

bool
lxb_css_property_state_margin_bottom(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_mp_top(parser, token, ctx, true);
}

bool
lxb_css_property_state_margin_left(lxb_css_parser_t *parser,
                                   const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_mp_top(parser, token, ctx, true);
}

bool
lxb_css_property_state_padding(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_mp(parser, token, ctx, false);
}

bool
lxb_css_property_state_padding_top(lxb_css_parser_t *parser,
                                   const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_mp_top(parser, token, ctx, false);
}

bool
lxb_css_property_state_padding_right(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_mp_top(parser, token, ctx, false);
}

bool
lxb_css_property_state_padding_bottom(lxb_css_parser_t *parser,
                                      const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_mp_top(parser, token, ctx, false);
}

bool
lxb_css_property_state_padding_left(lxb_css_parser_t *parser,
                                    const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_mp_top(parser, token, ctx, false);
}

static bool
lxb_css_property_state_line_width_style_color(lxb_css_parser_t *parser,
                                              const lxb_css_syntax_token_t *token,
                                              lxb_css_property_border_t *border)
{
    lxb_css_value_type_t type;
    const lxb_css_data_t *unit;
    lxb_css_value_length_t *length;
    lxb_css_syntax_token_string_t *str;

    switch (token->type) {
        case LXB_CSS_SYNTAX_TOKEN_DIMENSION:
            if (border->width.type != LXB_CSS_VALUE__UNDEF) {
                return false;
            }

            str = &lxb_css_syntax_token_dimension(token)->str;

            unit = lxb_css_unit_absolute_relative_by_name(str->data,
                                                          str->length);
            if (unit == NULL) {
                return false;
            }

            length = &border->width.length;

            border->width.type = LXB_CSS_VALUE__LENGTH;
            length->num = lxb_css_syntax_token_dimension(token)->num.num;
            length->is_float = lxb_css_syntax_token_dimension(token)->num.is_float;
            length->unit = (lxb_css_unit_t) unit->unique;
            break;

        case LXB_CSS_SYNTAX_TOKEN_NUMBER:
            if (border->width.type != LXB_CSS_VALUE__UNDEF) {
                return false;
            }

            length = &border->width.length;

            border->width.type = LXB_CSS_VALUE__NUMBER;
            length->num = lxb_css_syntax_token_number(token)->num;
            length->is_float = lxb_css_syntax_token_number(token)->is_float;
            break;

        case LXB_CSS_SYNTAX_TOKEN_IDENT:
            type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data,
                                         lxb_css_syntax_token_ident(token)->length);
            switch (type) {
                case LXB_CSS_VALUE_THIN:
                case LXB_CSS_VALUE_MEDIUM:
                case LXB_CSS_VALUE_THICK:
                    if (border->width.type != LXB_CSS_VALUE__UNDEF) {
                        return false;
                    }

                    border->width.type = type;
                    break;

                case LXB_CSS_VALUE_NONE:
                case LXB_CSS_VALUE_HIDDEN:
                case LXB_CSS_VALUE_DOTTED:
                case LXB_CSS_VALUE_DASHED:
                case LXB_CSS_VALUE_SOLID:
                case LXB_CSS_VALUE_DOUBLE:
                case LXB_CSS_VALUE_GROOVE:
                case LXB_CSS_VALUE_RIDGE:
                case LXB_CSS_VALUE_INSET:
                case LXB_CSS_VALUE_OUTSET:
                    if (border->style != LXB_CSS_VALUE__UNDEF) {
                        return false;
                    }

                    border->style = type;
                    break;

                default:
                    goto color;
            }

            break;

        default:
            goto color;
    }

    lxb_css_syntax_parser_consume(parser);

    return true;

color:

    if (border->color.type != LXB_CSS_VALUE__UNDEF) {
        return false;
    }

    return lxb_css_property_state_color(parser, token, &border->color);
}

bool
lxb_css_property_state_border(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token, void *ctx)
{
    bool res;
    lxb_css_value_type_t type;
    lxb_css_rule_declaration_t *declar = ctx;

    if (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) {
        type = lxb_css_value_by_name(lxb_css_syntax_token_ident(token)->data,
                                     lxb_css_syntax_token_ident(token)->length);
        switch (type) {
            case LXB_CSS_VALUE_INITIAL:
            case LXB_CSS_VALUE_INHERIT:
            case LXB_CSS_VALUE_UNSET:
            case LXB_CSS_VALUE_REVERT:
                declar->u.border->style = type;

                lxb_css_syntax_parser_consume(parser);
                return lxb_css_parser_success(parser);

            default:
                break;
        }
    }

    res = lxb_css_property_state_line_width_style_color(parser, token,
                                                        declar->u.border);
    if (!res) {
        return lxb_css_parser_failed(parser);
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN__TERMINATED) {
        return lxb_css_parser_success(parser);
    }

    res = lxb_css_property_state_line_width_style_color(parser, token,
                                                        declar->u.border);
    if (!res) {
        return lxb_css_parser_failed(parser);
    }

    token = lxb_css_syntax_parser_token_wo_ws(parser);
    lxb_css_property_state_check_token(parser, token);

    if (token->type == LXB_CSS_SYNTAX_TOKEN__TERMINATED) {
        return lxb_css_parser_success(parser);
    }

    res = lxb_css_property_state_line_width_style_color(parser, token,
                                                        declar->u.border);
    if (!res) {
        return lxb_css_parser_failed(parser);
    }

    return lxb_css_parser_success(parser);
}

bool
lxb_css_property_state_border_top(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_border(parser, token, ctx);
}

bool
lxb_css_property_state_border_right(lxb_css_parser_t *parser,
                                    const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_border(parser, token, ctx);
}

bool
lxb_css_property_state_border_bottom(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_border(parser, token, ctx);
}

bool
lxb_css_property_state_border_left(lxb_css_parser_t *parser,
                                   const lxb_css_syntax_token_t *token, void *ctx)
{
    return lxb_css_property_state_border(parser, token, ctx);
}
