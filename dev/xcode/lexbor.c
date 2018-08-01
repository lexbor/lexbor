/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <lex.borisov@gmail.com>
 */

#define FAIL_AND_EXIT(...) fprintf(stderr, __VA_ARGS__"\n"); exit(1)

#include "lexbor/core/fs.h"
#include "lexbor/html/parser.h"
#include "lexbor/html/serialize.h"
#include "lexbor/dom/interfaces/text.h"
#include "lexbor/html/interfaces/head_element.h"
#include "lexbor/html/interfaces/body_element.h"


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

    lxb_char_t html[] = "textarea content with <em>pseudo</em> <foo>markup";
    size_t size = sizeof(html) - 1;

//    size_t size;
//    lxb_char_t full_path[] = "/Users/alexanderborisov/Library/Developer/Xcode/DerivedData/lexbor-gzcibxrzmtowvbdhqpflilnxstbc/Build/Products/Debug/lexbor";
//    lxb_char_t *html = lexbor_fs_file_easy_read(full_path, &size);
//    if (html == NULL) {
//        FAIL_AND_EXIT("Failed to read file");
//    }

    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);

    if (status != LXB_STATUS_OK) {
        FAIL_AND_EXIT("Failed to create parser");
    }

    parser->tree->scripting = true;

    if (0) {
        document = lxb_html_parse(parser, html, size);
        if (document == NULL) {
            FAIL_AND_EXIT("Failed to parse");
        }

        ret = lxb_dom_interface_node(document);
    }
    else {
        ret = lxb_html_parse_fragment_by_tag_id(parser, NULL,
                                                LXB_HTML_TAG_TEXTAREA, LXB_HTML_NS_HTML,
                                                html, size);
    }

    status = lxb_html_serialize_pretty_tree_cb(ret, LXB_HTML_SERIALIZE_OPT_WITHOUT_CLOSING,
                                               0, serializer_callback, NULL);
    if (status != LXB_STATUS_OK) {
        FAIL_AND_EXIT("Failed to serialization tree");
    }

//    lxb_dom_text_t *text = document->body->element.element.node.first_child->first_child;

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
//#include "lexbor/html/tag.h"
//#include "lexbor/html/tag_res.h"
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
//    if (token->tag_id == LXB_HTML_TAG__TEXT
//        || token->tag_id == LXB_HTML_TAG__EM_COMMENT)
//    {
//        status = lxb_html_token_parse_data(token, &pc, &str, mraw);
//        if (status != LXB_STATUS_OK) {
//            return NULL;
//        }
//
//        printf("%s", (char *) str.data);
//    }
//    else {
//        const lxb_html_tag_data_t *tag_data;
//        tag_data = lxb_html_tag_data_by_id(tkz->tag_heap, token->tag_id);
//
//        tag_data = lxb_html_tag_data_by_name(tkz->tag_heap, "sef�stf", strlen("sef�stf"));
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
