/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#ifndef LEXBOR_HTML_PARSER_H
#define LEXBOR_HTML_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/html/base.h>
#include <lexbor/html/tree.h>
#include <lexbor/html/interfaces/document.h>


typedef enum {
    LXB_HTML_PARSER_STATE_BEGIN            = 0x00,
    LXB_HTML_PARSER_STATE_PROCESS          = 0x01,
    LXB_HTML_PARSER_STATE_END              = 0x02,
    LXB_HTML_PARSER_STATE_FRAGMENT_PROCESS = 0x03,
    LXB_HTML_PARSER_STATE_ERROR            = 0x04
}
lxb_html_parser_state_t;

typedef struct {
    lxb_html_tokenizer_t    *tkz;
    lxb_html_tree_t         *tree;
    lxb_html_tree_t         *original_tree;
    lxb_html_tag_heap_t     *tag_heap;

    lxb_dom_node_t          *root;
    lxb_dom_node_t          *form;

    lxb_html_parser_state_t state;
    lxb_status_t            status;
}
lxb_html_parser_t;


LXB_API lxb_html_parser_t *
lxb_html_parser_create(void);

LXB_API lxb_status_t
lxb_html_parser_init(lxb_html_parser_t *parser);

LXB_API void
lxb_html_parser_clean(lxb_html_parser_t *parser);

LXB_API lxb_html_parser_t *
lxb_html_parser_destroy(lxb_html_parser_t *parser, bool self_destroy);


LXB_API lxb_html_document_t *
lxb_html_parse(lxb_html_parser_t *parser, const lxb_char_t *html, size_t size);


LXB_API lxb_dom_node_t *
lxb_html_parse_fragment(lxb_html_parser_t *parser, lxb_html_element_t *element,
                        const lxb_char_t *html, size_t size);

LXB_API lxb_dom_node_t *
lxb_html_parse_fragment_by_tag_id(lxb_html_parser_t *parser,
                                  lxb_html_document_t *document,
                                  lxb_html_tag_id_t tag_id, lxb_html_ns_id_t ns,
                                  const lxb_char_t *html, size_t size);


LXB_API lxb_html_document_t *
lxb_html_parse_chunk_begin(lxb_html_parser_t *parser);

LXB_API lxb_status_t
lxb_html_parse_chunk_process(lxb_html_parser_t *parser,
                             const lxb_char_t *html, size_t size);

LXB_API lxb_status_t
lxb_html_parse_chunk_end(lxb_html_parser_t *parser);


LXB_API lxb_status_t
lxb_html_parse_fragment_chunk_begin(lxb_html_parser_t *parser,
                                    lxb_html_document_t *document,
                                    lxb_html_tag_id_t tag_id,
                                    lxb_html_ns_id_t ns);

LXB_API lxb_status_t
lxb_html_parse_fragment_chunk_process(lxb_html_parser_t *parser,
                                      const lxb_char_t *html, size_t size);

LXB_API lxb_dom_node_t *
lxb_html_parse_fragment_chunk_end(lxb_html_parser_t *parser);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_HTML_PARSER_H */