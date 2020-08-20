/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/core/array.h"
#include "lexbor/core/bst_map.h"

#include "lexbor/html/token.h"
#include "lexbor/html/tokenizer.h"

#include "unit/test.h"
#include "unit/kv.h"


#define THTE_ADD(name)                                                         \
    do {                                                                       \
        entry = lexbor_bst_map_insert(bst_map, root,                           \
                                  (const lxb_char_t *) LEXBOR_STRINGIZE(name), \
                                  strlen(LEXBOR_STRINGIZE(name)),              \
                                  (void *) name);                              \
        if (entry == NULL) {                                                   \
            goto error;                                                        \
        }                                                                      \
    }                                                                          \
    while (0)


typedef struct tokenizer_helper tokenizer_helper_t;


tokenizer_helper_t *
tokenizer_helper_make(void);

void
tokenizer_helper_destroy(tokenizer_helper_t *helper);

lxb_status_t
tokenizer_helper_errors_init(lexbor_bst_map_t *bst_map,
                             lexbor_bst_entry_t **root);

lxb_status_t
tokenizer_helper_types_init(lexbor_bst_map_t *bst_map,
                            lexbor_bst_entry_t **root);

lxb_html_token_t *
tokenizer_helper_tkz_callback_token_done(lxb_html_tokenizer_t *tkz,
                                         lxb_html_token_t *token, void *ctx);

lxb_html_tokenizer_t *
tokenizer_helper_tkz_init(tokenizer_helper_t *helper);

lxb_status_t
tokenizer_helper_tkz_parse(lxb_html_tokenizer_t *tkz,
                           const lxb_char_t *html, size_t len);

lxb_status_t
tokenizer_helper_tkz_parse_stream(lxb_html_tokenizer_t *tkz,
                                  const lxb_char_t *html, size_t len);

void
tokenizer_helper_tkz_destroy(lxb_html_tokenizer_t *tkz);


struct tokenizer_helper {
    unit_kv_t            *kv;
    lxb_html_tokenizer_t *tkz;
    lexbor_bst_map_t     *map;
    lexbor_bst_entry_t   *errors_root;
    lexbor_bst_entry_t   *types_root;
    lexbor_array_t       tokens;
    lexbor_array_t       ch_list;

    lxb_status_t         status;
};


tokenizer_helper_t *
tokenizer_helper_make(void)
{
    lxb_status_t status;
    tokenizer_helper_t *helper;

    helper = lexbor_calloc(1, sizeof(tokenizer_helper_t));
    if (helper == NULL) {
        return NULL;
    }

    helper->map = lexbor_bst_map_create();

    status = lexbor_bst_map_init(helper->map, 128);
    if (status != LXB_STATUS_OK) {
        goto error;
    }

    status = tokenizer_helper_errors_init(helper->map, &helper->errors_root);
    if (status != LXB_STATUS_OK) {
        goto error;
    }

    status = tokenizer_helper_types_init(helper->map, &helper->types_root);
    if (status != LXB_STATUS_OK) {
        goto error;
    }

    status = lexbor_array_init(&helper->ch_list, 4096);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    helper->kv = unit_kv_create();

    status = unit_kv_init(helper->kv, 256);
    if (status != LXB_STATUS_OK) {
        unit_kv_destroy(helper->kv, true);
        goto error;
    }

    helper->status = LXB_STATUS_OK;
    helper->tkz = tokenizer_helper_tkz_init(helper);

    return helper;

error:

    lexbor_bst_map_destroy(helper->map, true);

    return lexbor_free(helper);
}

void
tokenizer_helper_destroy(tokenizer_helper_t *helper)
{
    lxb_char_t *ch;
    
    unit_kv_destroy(helper->kv, true);
    lexbor_bst_map_destroy(helper->map, true);
    tokenizer_helper_tkz_destroy(helper->tkz);

    for (size_t i = 0; i < helper->ch_list.length; i++) {
        ch = lexbor_array_get(&helper->ch_list, i);

        if (ch != NULL) {
            lexbor_free(ch);
        }
    }

    lexbor_array_destroy(&helper->ch_list, false);
    lexbor_free(helper);
}

lxb_status_t
tokenizer_helper_errors_init(lexbor_bst_map_t *bst_map,
                             lexbor_bst_entry_t **root)
{
    lexbor_bst_map_entry_t *entry;

    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_ABCLOFEMCO);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_ABDOPUID);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_ABDOSYID);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_ABOFDIINNUCHRE);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_CDINHTCO);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_CHREOUUNRA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_COCHININST);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_COCHRE);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_ENTAWIAT);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_DUAT);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_ENTAWITRSO);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_EOBETANA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_EOINCD);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_EOINCO);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_EOINDO);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_EOINSCHTCOLITE);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_EOINTA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_INCLCO);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_INOPCO);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_INCHSEAFDONA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_INFICHOFTANA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIATVA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIDONA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIDOPUID);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIDOSYID);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIENTANA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIQUBEDOPUID);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIQUBEDOSYID);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MISEAFCHRE);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIWHAFDOPUKE);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIWHAFDOSYKE);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIWHBEDONA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIWHBEAT);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_MIWHBEDOPUANSYID);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_NECO);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_NOCHRE);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_NOININST);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_NOVOHTELSTTAWITRSO);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_NUCHRE);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_SUCHRE);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_SUININST);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_UNCHAFDOSYID);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_UNCHINATNA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_UNCHINUNATVA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_UNEQSIBEATNA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_UNNUCH);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_UNQUMAINOFTANA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_UNSOINTA);
    THTE_ADD(LXB_HTML_TOKENIZER_ERROR_UNNACHRE);

    return LXB_STATUS_OK;

error:

    return LXB_STATUS_ERROR;
}

lxb_status_t
tokenizer_helper_types_init(lexbor_bst_map_t *bst_map,
                            lexbor_bst_entry_t **root)
{
    lexbor_bst_map_entry_t *entry;

    THTE_ADD(LXB_HTML_TOKEN_TYPE_OPEN);
    THTE_ADD(LXB_HTML_TOKEN_TYPE_CLOSE);
    THTE_ADD(LXB_HTML_TOKEN_TYPE_CLOSE_SELF);
    THTE_ADD(LXB_HTML_TOKEN_TYPE_FORCE_QUIRKS);
    THTE_ADD(LXB_HTML_TOKEN_TYPE_DONE);

    return LXB_STATUS_OK;

error:

    return LXB_STATUS_ERROR;
}

lxb_html_token_t *
tokenizer_helper_tkz_callback_token_done(lxb_html_tokenizer_t *tkz,
                                         lxb_html_token_t *token, void *ctx)
{
    size_t length;
    lxb_char_t *data;
    lxb_status_t status;
    tokenizer_helper_t *helper = ctx;

    if (token->tag_id == LXB_TAG__END_OF_FILE) {
        return token;
    }

    status = lexbor_array_push(&helper->tokens, token);
    if (status != LXB_STATUS_OK) {
        tkz->status = status;
        return NULL;
    }

    if (token->text_start != NULL) {
        length = token->text_end - token->text_start;

        data = lexbor_mraw_alloc(tkz->mraw, length);
        if (data == NULL) {
            return NULL;
        }

        memcpy(data, token->text_start, length);

        token->text_start = data;
        token->text_end = data + length;
    }

    return lexbor_dobject_alloc(tkz->dobj_token);
}

lxb_html_tokenizer_t *
tokenizer_helper_tkz_init(tokenizer_helper_t *helper)
{
    lxb_status_t status;
    lxb_html_tokenizer_t *tkz;

    status = lexbor_array_init(&helper->tokens, 128);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    tkz = lxb_html_tokenizer_create();

    status = lxb_html_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        lexbor_array_destroy(&helper->tokens,false);
        return lxb_html_tokenizer_destroy(tkz);
    }

    status = lxb_html_tokenizer_tags_make(tkz, 128);
    if (status != LXB_STATUS_OK) {
        lexbor_array_destroy(&helper->tokens,false);
        return lxb_html_tokenizer_destroy(tkz);
    }

    status = lxb_html_tokenizer_attrs_make(tkz, 128);
    if (status != LXB_STATUS_OK) {
        lexbor_array_destroy(&helper->tokens,false);
        return lxb_html_tokenizer_destroy(tkz);
    }

    lxb_html_tokenizer_callback_token_done_set(tkz,
                                       tokenizer_helper_tkz_callback_token_done,
                                       helper);

    return tkz;
}

lxb_status_t
tokenizer_helper_tkz_parse(lxb_html_tokenizer_t *tkz,
                           const lxb_char_t *html, size_t len)
{
    lxb_status_t status;
    tokenizer_helper_t *helper;

    helper = lxb_html_tokenizer_callback_token_done_ctx(tkz);
    if (helper == NULL) {
        return LXB_STATUS_ERROR;
    }

    lexbor_array_clean(&helper->tokens);
    lxb_html_tokenizer_clean(tkz);

    status = lxb_html_tokenizer_begin(tkz);
    if (status != LXB_STATUS_OK) {
        return LXB_STATUS_ERROR;
    }

    status = lxb_html_tokenizer_chunk(tkz, html, len);
    if (status != LXB_STATUS_OK) {
        return LXB_STATUS_ERROR;
    }

    return lxb_html_tokenizer_end(tkz);
}

lxb_status_t
tokenizer_helper_tkz_parse_stream(lxb_html_tokenizer_t *tkz,
                                  const lxb_char_t *html, size_t len)
{
    lxb_char_t *ch;
    lxb_status_t status;
    tokenizer_helper_t *helper;

    helper = lxb_html_tokenizer_callback_token_done_ctx(tkz);
    if (helper == NULL) {
        return LXB_STATUS_ERROR;
    }

    lexbor_array_clean(&helper->tokens);
    lxb_html_tokenizer_clean(tkz);

    status = lxb_html_tokenizer_begin(tkz);
    if (status != LXB_STATUS_OK) {
        return LXB_STATUS_ERROR;
    }

    if (lexbor_array_length(&helper->ch_list) < len) {
        status = lexbor_array_set(&helper->ch_list, len, NULL);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    for (size_t i = 0; i < len; i++) {
        ch = lexbor_array_get(&helper->ch_list, i);
        
        if (ch == NULL) {
            ch = lexbor_malloc(sizeof(char));
            if (ch == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }

            lexbor_array_set(&helper->ch_list, i, ch);
        }

        *ch = html[i];

        status = lxb_html_tokenizer_chunk(tkz, ch, 1);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return lxb_html_tokenizer_end(tkz);
}

void
tokenizer_helper_tkz_destroy(lxb_html_tokenizer_t *tkz)
{
    tokenizer_helper_t *helper;

    helper = lxb_html_tokenizer_callback_token_done_ctx(tkz);
    if (helper != NULL) {
        lexbor_array_destroy(&helper->tokens, false);
    }

    lxb_html_tokenizer_tags_destroy(tkz);
    lxb_html_tokenizer_attrs_destroy(tkz);
    lxb_html_tokenizer_destroy(tkz);
}

#undef THTE_ADD
