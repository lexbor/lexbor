/*
 * Copyright (C) 2021-2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LXB_CSS_PROPERTY_STATE_H
#define LXB_CSS_PROPERTY_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/css/base.h"


LXB_API bool
lxb_css_property_state__undef(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state__custom(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_display(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_width(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_height(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_box_sizing(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_min_width(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_min_height(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_max_width(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_max_height(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_margin(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_margin_top(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_margin_right(lxb_css_parser_t *parser,
                                    const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_margin_bottom(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_margin_left(lxb_css_parser_t *parser,
                                   const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_padding(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_padding_top(lxb_css_parser_t *parser,
                                   const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_padding_right(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_padding_bottom(lxb_css_parser_t *parser,
                                      const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_padding_left(lxb_css_parser_t *parser,
                                    const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_border(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_border_top(lxb_css_parser_t *parser,
                                  const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_border_right(lxb_css_parser_t *parser,
                                    const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_border_bottom(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token, void *ctx);
LXB_API bool
lxb_css_property_state_border_left(lxb_css_parser_t *parser,
                                   const lxb_css_syntax_token_t *token, void *ctx);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LXB_CSS_PROPERTY_STATE_H */
