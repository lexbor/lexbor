/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_CSS_SELECTORS_SELECTOR_H
#define LEXBOR_CSS_SELECTORS_SELECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/str.h"
#include "lexbor/css/selectors/base.h"
#include "lexbor/css/syntax/anb.h"


typedef enum {
    LXB_CSS_SELECTOR_TYPE__UNDEF = 0x00,
    LXB_CSS_SELECTOR_TYPE_ANY,
    LXB_CSS_SELECTOR_TYPE_ELEMENT,                 /* div, tag name <div> */
    LXB_CSS_SELECTOR_TYPE_ID,                      /* #hash */
    LXB_CSS_SELECTOR_TYPE_CLASS,                   /* .class */
    LXB_CSS_SELECTOR_TYPE_ATTRIBUTE,               /* [key=val], <... key="val"> */
    LXB_CSS_SELECTOR_TYPE_PSEUDO_CLASS,            /* :pseudo */
    LXB_CSS_SELECTOR_TYPE_PSEUDO_CLASS_FUNCTION,   /* :function(...) */
    LXB_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT,          /* ::pseudo */
    LXB_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT_FUNCTION, /* ::function(...) */
    LXB_CSS_SELECTOR_TYPE__LAST_ENTRY
}
lxb_css_selector_type_t;

typedef enum {
    LXB_CSS_SELECTOR_COMBINATOR_DESCENDANT = 0x00, /* WHITESPACE */
    LXB_CSS_SELECTOR_COMBINATOR_CLOSE,             /* two compound selectors [key=val].foo */
    LXB_CSS_SELECTOR_COMBINATOR_CHILD,             /* '>' */
    LXB_CSS_SELECTOR_COMBINATOR_SIBLING,           /* '+' */
    LXB_CSS_SELECTOR_COMBINATOR_FOLLOWING,         /* '~' */
    LXB_CSS_SELECTOR_COMBINATOR_CELL,              /* '||' */
    LXB_CSS_SELECTOR_COMBINATOR__LAST_ENTRY
}
lxb_css_selector_combinator_t;

typedef enum {
    LXB_CSS_SELECTOR_MATCH_EQUAL = 0x00,  /*  = */
    LXB_CSS_SELECTOR_MATCH_INCLUDE,       /* ~= */
    LXB_CSS_SELECTOR_MATCH_DASH,          /* |= */
    LXB_CSS_SELECTOR_MATCH_PREFIX,        /* ^= */
    LXB_CSS_SELECTOR_MATCH_SUFFIX,        /* $= */
    LXB_CSS_SELECTOR_MATCH_SUBSTRING,     /* *= */
    LXB_CSS_SELECTOR_MATCH__LAST_ENTRY
}
lxb_css_selector_match_t;

typedef enum {
    LXB_CSS_SELECTOR_MODIFIER_UNSET = 0x00,
    LXB_CSS_SELECTOR_MODIFIER_I,
    LXB_CSS_SELECTOR_MODIFIER_S,
    LXB_CSS_SELECTOR_MODIFIER__LAST_ENTRY
}
lxb_css_selector_modifier_t;

typedef struct {
    lxb_css_selector_match_t    match;
    lxb_css_selector_modifier_t modifier;
    lexbor_str_t                value;
}
lxb_css_selector_attribute_t;

typedef struct {
    unsigned type;
    void     *data;
}
lxb_css_selector_pseudo_t;

typedef struct {
    lxb_css_syntax_anb_t          anb;
    lxb_css_selector_list_t       *of;
}
lxb_css_selector_anb_of_t;

struct lxb_css_selector {
    lxb_css_selector_type_t       type;
    lxb_css_selector_combinator_t combinator;

    lexbor_str_t                  name;
    lexbor_str_t                  ns;

    union lxb_css_selector_u {
        lxb_css_selector_attribute_t attribute;
        lxb_css_selector_pseudo_t    pseudo;
    }
    u;

    lxb_css_selector_t            *next;
    lxb_css_selector_t            *prev;

    lxb_css_selector_list_t       *list;
};

typedef struct {
    unsigned int a;
    unsigned int b;
    unsigned int c;
}
lxb_css_selector_specificity_t;

struct lxb_css_selector_list {
    lxb_css_selector_t             *first;
    lxb_css_selector_t             *last;

    lxb_css_selector_t             *parent;

    lxb_css_selector_list_t        *next;
    lxb_css_selector_list_t        *prev;

    lxb_css_selectors_memory_t     *memory;

    lxb_css_selector_specificity_t specificity;
    bool                           invalid;
};


LXB_API lxb_css_selector_t *
lxb_css_selector_create(lxb_css_selector_list_t *list);

LXB_API void
lxb_css_selector_destroy(lxb_css_selector_t *selector);

LXB_API void
lxb_css_selector_destroy_chain(lxb_css_selector_t *selector);

LXB_API lxb_css_selector_list_t *
lxb_css_selector_list_create(lxb_css_selectors_memory_t *mem);

LXB_API void
lxb_css_selector_list_remove(lxb_css_selector_list_t *list);

LXB_API void
lxb_css_selector_list_selectors_remove(lxb_css_selectors_t *selectors,
                                       lxb_css_selector_list_t *list);

LXB_API void
lxb_css_selector_list_destroy(lxb_css_selector_list_t *list);

LXB_API void
lxb_css_selector_list_destroy_chain(lxb_css_selector_list_t *list);

LXB_API void
lxb_css_selector_list_destroy_memory(lxb_css_selector_list_t *list);

LXB_API lxb_status_t
lxb_css_selector_serialize(lxb_css_selector_t *selector,
                           lexbor_serialize_cb_f cb, void *ctx);

LXB_API lxb_status_t
lxb_css_selector_serialize_chain(lxb_css_selector_t *selector,
                                 lexbor_serialize_cb_f cb, void *ctx);

LXB_API lxb_char_t *
lxb_css_selector_serialize_chain_char(lxb_css_selector_t *selector,
                                      size_t *out_length);

LXB_API lxb_status_t
lxb_css_selector_serialize_list(lxb_css_selector_list_t *list,
                                    lexbor_serialize_cb_f cb, void *ctx);

LXB_API lxb_status_t
lxb_css_selector_serialize_list_chain(lxb_css_selector_list_t *list,
                                      lexbor_serialize_cb_f cb, void *ctx);

LXB_API lxb_char_t *
lxb_css_selector_serialize_list_chain_char(lxb_css_selector_list_t *list,
                                           size_t *out_length);

LXB_API lxb_status_t
lxb_css_selector_serialize_anb_of(lxb_css_selector_anb_of_t *anbof,
                                  lexbor_serialize_cb_f cb, void *ctx);

LXB_API lxb_char_t *
lxb_css_selector_combinator(lxb_css_selector_t *selector, size_t *out_length);

LXB_API void
lxb_css_selector_list_append(lxb_css_selector_list_t *list,
                             lxb_css_selector_t *selector);

LXB_API void
lxb_css_selector_list_append_next(lxb_css_selector_list_t *dist,
                                  lxb_css_selector_list_t *src);

LXB_API void
lxb_css_selector_append_next(lxb_css_selector_t *dist, lxb_css_selector_t *src);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_CSS_SELECTORS_SELECTOR_H */
