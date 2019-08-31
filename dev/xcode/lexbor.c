/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#define FAIL_AND_EXIT(...) fprintf(stderr, __VA_ARGS__"\n"); exit(1)

#include "lexbor/core/fs.h"
#include "lexbor/html/parser.h"
#include "lexbor/html/serialize.h"
#include "lexbor/dom/interfaces/text.h"
#include "lexbor/dom/interfaces/element.h"
#include "lexbor/html/interfaces/head_element.h"
#include "lexbor/html/interfaces/body_element.h"


#include "lexbor/core/perf.h"

lxb_status_t
serializer_callback(const lxb_char_t *data, size_t len, void *ctx)
{
    printf("%.*s", (int) len, (const char *) data);

    return LXB_STATUS_OK;
}

int main(int argc, const char * argv[])
{
    lxb_status_t status;
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;
    lxb_dom_node_t *ret;

    void *perf = lexbor_perf_create();

    const lxb_ns_data_t *entry;
    lxb_ns_heap_t *ns_heap;

    ns_heap = lxb_ns_heap_create();
    lxb_ns_heap_init(ns_heap, 32);

    entry = lxb_ns_data_by_link(ns_heap, (const lxb_char_t *) "#undef", 6);

//    lxb_char_t html[] = "<div id=\"TMpanel\">";
//    size_t size = sizeof(html) - 1;

    size_t size;
    lxb_char_t full_path[] = "/new/test/habr.html";
    lxb_char_t *html = lexbor_fs_file_easy_read(full_path, &size);
    if (html == NULL) {
        FAIL_AND_EXIT("Failed to read file");
    }

    lexbor_perf_begin(perf);

    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);

    if (status != LXB_STATUS_OK) {
        FAIL_AND_EXIT("Failed to create parser");
    }

    parser->tree->scripting = true;

//    if (1) {
//        document = lxb_html_parse(parser, html, size);
//        if (document == NULL) {
//            FAIL_AND_EXIT("Failed to parse");
//        }
//
//        ret = lxb_dom_interface_node(document);
//    }
//    else {
//        ret = lxb_html_parse_fragment_by_tag_id(parser, NULL,
//                                                147, 4,
//                                                html, size);
//    }

    document = lxb_html_parse(parser, html, size);
//    document = lxb_html_parse_chunk_begin(parser);
//
//    for (size_t i = 0; i < size; i++) {
//        status = lxb_html_parse_chunk_process(parser, &html[i], 1);
//    }
//
//    status = lxb_html_parse_chunk_end(parser);

    lexbor_perf_end(perf);
    printf("Time: %0.5f\n", lexbor_perf_in_sec(perf));

    lxb_dom_collection_t *collection = lxb_dom_collection_make(&document->dom_document, 128);
    if (collection == NULL) {
        FAIL_AND_EXIT("Failed to create collection");
    }


    lexbor_perf_begin(perf);

    lxb_dom_element_t *el_root = &lxb_html_document_body_element(document)->element.element;
    status = lxb_dom_elements_by_tag_name(el_root, collection, "div", strlen("div"));
    if (status != LXB_STATUS_OK) {
        FAIL_AND_EXIT("Failed to create get elements");
    }

    lexbor_perf_end(perf);
    printf("Find: %0.5f\n", lexbor_perf_in_sec(perf));


//    for (size_t i = 0; i < collection->array.length; i++) {
//        lxb_dom_element_t *element = collection->array.list[i];
//
//        status = lxb_html_serialize_pretty_cb(&element->node, LXB_HTML_SERIALIZE_OPT_WITHOUT_CLOSING,
//                                              0, serializer_callback, NULL);
//        if (status != LXB_STATUS_OK) {
//            FAIL_AND_EXIT("Failed to serialization tree");
//        }
//    }

//    status = lxb_html_serialize_pretty_tree_cb(&document->dom_document.node, LXB_HTML_SERIALIZE_OPT_WITHOUT_CLOSING,
//                                               0, serializer_callback, NULL);
//    if (status != LXB_STATUS_OK) {
//        FAIL_AND_EXIT("Failed to serialization tree");
//    }

    lxb_html_parser_destroy(parser, true);

    return 0;
}


//#include "lexbor/html/tokenizer.h"
//
//#include "lexbor/core/mraw.h"
//#include "lexbor/core/str.h"
//
//#include "lexbor/core/shs.h"
//
//#define FAIL_AND_EXIT(...) fprintf(stderr, __VA_ARGS__"\n"); exit(1)
//
//#include "lexbor/tag/tag.h"
//#include "lexbor/tag/res.h"
//
//#include "lexbor/html/parser_char.h"

//lxb_html_token_t *
//test_callback_token_done(lxb_html_tokenizer_t *tkz,
//                         lxb_html_token_t *token, void *ctx)
//{
//    lexbor_str_t str = {0}, name = {0}, value = {0};
//    lxb_status_t status;
//    lxb_html_token_attr_t *attr;
//    lexbor_mraw_t *mraw = ctx;
//
//    lexbor_str_clean(&str);
//
//    /*
//    status = lxb_html_token_make_data(token, &str, mraw);
//    */
//
//    lxb_html_parser_char_t pc = {0};
//
//    if (token->tag_id == LXB_TAG__TEXT
//        || token->tag_id == LXB_TAG__EM_COMMENT)
//    {
//        status = lxb_html_token_parse_data(token, &pc, &str, mraw);
//        if (status != LXB_STATUS_OK) {
//            return NULL;
//        }
//
//        printf("%s", (char *) str.data);
//    }
//    else {
//        const lxb_tag_data_t *tag_data;
//        tag_data = lxb_tag_data_by_id(tkz->tag_heap, token->tag_id);
//
//        tag_data = lxb_tag_data_by_name(tkz->tag_heap, "sef�stf", strlen("sef�stf"));
//
//        if (tag_data != NULL) {
//            printf("%s", (char *) tag_data->name);
//        }
//    }
//
//    attr = token->attr_first;
//    while (attr != NULL) {
//        status = lxb_html_token_attr_parse(attr, &pc, &name, &value, mraw);
//        if (status != LXB_STATUS_OK) {
//            return NULL;
//        }
//
//        if (name.data) {
//            printf(" %s", (char *) name.data);
//            lexbor_str_clean(&name);
//        }
//
//        if (value.data) {
//            printf("=%s", (char *) value.data);
//            lexbor_str_clean(&value);
//        }
//
//        attr = attr->next;
//    }
//
//    printf("\n");
//
//    lexbor_str_destroy(&str, mraw, false);
//
//    return token;
//}


//int main(int argc, const char * argv[])
//{
//    lxb_char_t html[] = "<div \0SuperNaME>";
////    lxb_char_t html[] = "&acirc=";
//    size_t html_size = sizeof(html) - 1;
//
//    lxb_char_t *ch;
//
//    lexbor_mraw_t mraw;
//    lexbor_mraw_init(&mraw, 1024);
//
//    lxb_html_tokenizer_t *tkz = lxb_html_tokenizer_create();
//    lxb_status_t status = lxb_html_tokenizer_init(tkz);
//    if (status != LXB_STATUS_OK) {
//        FAIL_AND_EXIT("Could not init tokenizer");
//    }
//
//    lxb_html_tokenizer_callback_token_done_set(tkz, test_callback_token_done,
//                                               &mraw);
//
//    status = lxb_html_tokenizer_begin(tkz);
//    if (status != LXB_STATUS_OK) {
//        FAIL_AND_EXIT("Failed to 'begin' HTML");
//    }
//
//    //tkz->state = lxb_html_tokenizer_state_script_data_before;
///*
//    status = lxb_html_tokenizer_chunk(tkz, html, html_size);
//*/
//
//    for (size_t i = 0; i < html_size; i++) {
//        ch = lexbor_malloc(1);
//        *ch = html[i];
//
//        status = lxb_html_tokenizer_chunk(tkz, ch, 1);
//        if (status != LXB_STATUS_OK) {
//            FAIL_AND_EXIT("Failed to 'chunk' HTML");
//        }
//    }
//
//    status = lxb_html_tokenizer_end(tkz);
//    if (status != LXB_STATUS_OK) {
//        FAIL_AND_EXIT("Failed to parse ending of HTML");
//    }
//
//    lxb_html_tokenizer_destroy(tkz, true);
//
//    return 0;
//}
