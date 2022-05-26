
//#include <lexbor/html/html.h>
//#include <lexbor/core/fs.h>
//#include <lexbor/core/perf.h>
//
//
//#define FAILED(...)                                                            \
//    do {                                                                       \
//        fprintf(stderr, __VA_ARGS__);                                          \
//        fprintf(stderr, "\n");                                                 \
//        exit(EXIT_FAILURE);                                                    \
//    }                                                                          \
//    while (0)
//
//#define PRINT(...)                                                             \
//    do {                                                                       \
//        fprintf(stdout, __VA_ARGS__);                                          \
//        fprintf(stdout, "\n");                                                 \
//    }                                                                          \
//    while (0)
//
//
//lxb_inline lxb_status_t
//serializer_callback(const lxb_char_t *data, size_t len, void *ctx)
//{
//    printf("%.*s", (int) len, (const char *) data);
//
//    return LXB_STATUS_OK;
//}
//
//lxb_inline void
//serialize(lxb_dom_node_t *node)
//{
//    lxb_status_t status;
//
//    status = lxb_html_serialize_pretty_tree_cb(node,
//                                               0,
//                                               0, serializer_callback, NULL);
//    if (status != LXB_STATUS_OK) {
//        FAILED("Failed to serialization HTML tree");
//    }
//}
//
//lexbor_action_t
//simple_walker(lxb_dom_node_t *node, void *ctx)
//{
//    lxb_dom_element_t *element;
//
//    if (node->local_name == LXB_TAG__TEXT) {
//        element = (lxb_dom_element_t *) node;
//
//        if (lxb_dom_element_first_attribute_noi(node) != NULL) {
//            printf(" ");
//        }
//    }
//
//    return LEXBOR_ACTION_OK;
//}
//
//
//int
//main(int argc, const char *argv[])
//{
//    lxb_status_t status;
//
//
//
//    static const lxb_char_t html[] = "<template><h1>Hello</h1></template>";
//    size_t html_len = sizeof(html) - 1;
//
//    void *perf = lexbor_perf_create();
//
////    size_t html_len;
////    const lxb_char_t *html = lexbor_fs_file_easy_read((lxb_char_t *) "/Users/alexanderborisov/new/input.html", &html_len);
////    if (html == NULL) {
////        return EXIT_FAILURE;
////    }
//
//    lexbor_perf_begin(perf);
//
//    lxb_html_element_t *root;
//    lxb_dom_node_t *html_node;
//    lxb_html_document_t *document;
//    const lxb_char_t name[] = "div";
//
//    /* Initialization */
//    document = lxb_html_document_create();
//    if (document == NULL) {
//        FAILED("Failed to create HTML Document");
//    }
//
//    root = lxb_html_document_create_element(document, name, sizeof(name) - 1, NULL);
//    if (root == NULL) {
//        return EXIT_FAILURE;
//    }
//
//
//    html_node = lxb_html_document_parse_fragment(document, &root->element, html, html_len);
//    if (html_node == NULL) {
//        return EXIT_FAILURE;
//    }
//
//    /* Parse HTML */
////    status = lxb_html_document_parse(document, html, html_len);
////    if (status != LXB_STATUS_OK) {
////        FAILED("Failed to parse HTML");
//
////    }
//
////    lxb_dom_collection_t *collection;
////    collection = lxb_dom_collection_make(&document->dom_document, 128);
//
////    lxb_dom_node_simple_walk(&document->dom_document.node, simple_walker, NULL);
//
//    serialize(html_node);
//
////    lxb_dom_element_t *body = lxb_dom_interface_element(document->body);
////
////    lxb_dom_elements_by_attr(body, collection, "id", 2, "content", 7, true);
////
////    lxb_dom_element_t *content_root = lxb_dom_collection_element(collection, 0);
////
////    size_t len;
////    lxb_char_t *text;
////    text = lxb_dom_node_text_content(&content_root->node, &len);
//
//
//    /* Print Incoming Data */
////    PRINT("HTML:");
////    PRINT("%.*s", (int) len, (const char *) text);
//
//    /* Print Result */
////    PRINT("\nHTML Tree:");
////    serialize(lxb_dom_interface_node(document));
//
//    /* Destroy document */
//    lxb_html_document_destroy(document);
//
//    lexbor_perf_end(perf);
//
//    printf("Sec: %f\n", lexbor_perf_in_sec(perf));
//
//    lexbor_perf_destroy(perf);
//
//    return 0;
//}
































//
//
//
//
//
//#include <lexbor/core/fs.h>
//#include <lexbor/css/css.h>
//#include <lexbor/core/perf.h>
//
//
//
//lxb_status_t
//callback(const lxb_char_t *data, size_t len, void *ctx)
//{
//    printf("%.*s", (int) len, (char *) data);
//    return LXB_STATUS_OK;
//}
//
//int
//main(int argc, const char *argv[])
//{
//    lxb_status_t status;
//    lxb_css_syntax_anb_t *anb;
//    lxb_css_selector_t *selector;
//    lxb_css_selector_list_t *list;
//    lxb_css_parser_t *parser;
//
////    void *perf = lexbor_perf_create();
//
////    lxb_char_t data[] = "-100.12E+10^ \r\n\r\f /* asas */\"abc\"'xyz'#hashsshhh@alala --lala url('http://lalala.ru') url(http://lalala.ru)";
////    lxb_char_t data[] = "\xff$";
//
//    /*
//      an+b
//
//      n-1
//      n- 1
//      n - 1
//
//      -n-1
//      -n- 1
//      -n - 1
//
//      n+1
//      n+ 1
//      n + 1
//     */
//
//
////    lxb_char_t data[] = ":not(";
////    lxb_char_t data[] = ":not()";
//    lxb_char_t data[] = ":has(:not(div, {(}, .class), #hash)";
//
//
////    lxb_char_t data[] = ":not(:nth-child(2n+1 of div, :has(, span, :current(sdsd), [id='123' s],)))";
////    lxb_char_t data[] = ":not(:nth-child(2n+1 of div, :has(, span, :current(sdsd), [id='123' s],)))";
////    lxb_char_t data[] = ":has(div, :not(:has(1%, div), sd";
//
////    lxb_char_t data[] = ":not(1%, sd";
//
////    lxb_char_t data[] = ":has(div, :current(span), .class)";
////    lxb_char_t data[] = ":has(div, [a=1%], hashs)";
////    lxb_char_t data[] = ":not(:not())";
////    lxb_char_t data[] = ":has(, span, :current(sdsd), [id='123' s],)";
////    lxb_char_t data[] = ":has(, ,)";
////    lxb_char_t data[] = ":not(, span, [id='123' s],)";
////    lxb_char_t data[] = ":has(.super, #hash, + [a=b s], div) + :where(:not(a, span), lala, #iiff)";
//    size_t length = sizeof(data) - 1;
//
//
////    size_t length;
////    lxb_char_t *data = lexbor_fs_file_easy_read("/Users/alexanderborisov/new/lexbor/css/bootstrap-large.css", &length);
//
//    parser = lxb_css_parser_create();
//    status = lxb_css_parser_init(parser, NULL, NULL);
//    if (status != LXB_STATUS_OK) {
//        return EXIT_FAILURE;
//    }
//
////    lexbor_perf_begin(perf);
//
////    anb = lxb_css_syntax_anb_parse(parser, data, length);
////
////    const lexbor_str_t *err = lxb_css_parser_last_error(parser);
////    if (err->data != NULL) {
////        printf("%s", (const char *) err->data);
////    }
////    else {
////        lxb_css_syntax_anb_serialize_char(anb, NULL);
////        lxb_css_syntax_anb_serialize(anb, callback, NULL);
////    }
//
//    list = lxb_css_selectors_parse(parser, data, length);
//
//
//    lxb_css_selector_serialize_list_chain(list, callback, NULL);
//    printf("\n\n");
//    printf("Log:\n");
//    lxb_css_log_serialize(parser->log, callback, NULL, "    ", 4);
//    printf("\n\n");
//
////    lexbor_perf_end(perf);
//
//    lxb_css_parser_clean(parser);
//
//    lxb_css_parser_destroy(parser, true);
//
//
////    lxb_css_syntax_tokenizer_chunk_cb_set(tkz, chunk_cb, inbuf);
//
//
////
////    do {
////        token = lxb_css_syntax_tokenizer_token(tkz);
////        if (token == NULL) {
////            return EXIT_FAILURE;
////        }
////
//////        check_raw(token);
////
////        name = lxb_css_syntax_token_type_name_by_id(token->type);
////        printf("%s: ", (const char *) name);
////
////        lxb_css_syntax_token_serialize_cb(token, callback, NULL);
////        printf("\n");
////    }
////    while (lxb_css_syntax_token_type(token) != LXB_CSS_SYNTAX_TOKEN__EOF);
//
////    printf("Sec: %f\n", lexbor_perf_in_sec(perf));
//
////    do {
////        token = lxb_css_syntax_token(tkz);
////        if (token == NULL) {
////            return EXIT_FAILURE;
////        }
////
////        lxb_css_syntax_token_serialize_cb(token, callback, NULL);
//////        lxb_css_syntax_token_consume(tkz);
////
////        printf("\n");
////    }
////    while (lxb_css_syntax_token_type(token) != LXB_CSS_SYNTAX_TOKEN__EOF);
//
//    return EXIT_SUCCESS;
//}
//
//
//
//
//




















///*
// * Copyright (C) 2021 Alexander Borisov
// *
// * Author: Alexander Borisov <borisov@lexbor.com>
// */
//
//#include <lexbor/core/core.h>
//#include <lexbor/html/html.h>
//#include <lexbor/css/css.h>
//#include <lexbor/selectors/selectors.h>
//
//
//
//lxb_status_t
//callback(const lxb_char_t *data, size_t len, void *ctx)
//{
//    printf("%.*s", (int) len, (const char *) data);
//
//    return LXB_STATUS_OK;
//}
//
//int
//main(int argc, const char *argv[])
//{
//    lxb_status_t status;
//    lxb_css_parser_t *parser;
//    lxb_css_stylesheet_t *stylesheet;
//    lxb_css_stylesheet_parser_t *ssp;
//
//    /* CSS Data. */
//
//    static const lxb_char_t css[] = //"@namespace 'https://html.org'\n"
//    "div p > span {width: 100%}\n"
//    ":not(:has(p)) {height: 30px}";
//
//    /* Create CSS parser. */
//
//    parser = lxb_css_parser_create();
//    status = lxb_css_parser_init(parser, NULL, NULL);
//    if (status != LXB_STATUS_OK) {
//        return EXIT_FAILURE;
//    }
//
//    /* Create CSS StyleSheet parser. */
//
//    ssp = lxb_css_stylesheet_parser_create();
//    status = lxb_css_stylesheet_parser_init(ssp);
//    if (status != LXB_STATUS_OK) {
//        return EXIT_FAILURE;
//    }
//
//    /*
//     * It is important that a new StyleSheet object is not created internally
//     * for each call to the parser.
//     */
//    lxb_css_parser_stylesheet_set(parser, ssp);
//
//    /* Parse and get the log. */
//
//    stylesheet = lxb_css_stylesheet_parse(parser, css,
//                                          (sizeof(css) / sizeof(lxb_char_t)) - 1);
//    if (stylesheet == NULL) {
//        return EXIT_FAILURE;
//    }
//
//    /* StyleSheet Serialization. */
//
//    printf("StyleSheet: ");
////    (void) lxb_css_selector_serialize_list_chain(list, callback, NULL);
//    printf("\n");
//
//    /* Destroy resources for CSS Parser. */
//    (void) lxb_css_parser_destroy(parser, true);
//
//    /* Destroy CSS StyleSheet memory. */
//    (void) lxb_css_stylesheet_parser_destroy(ssp, true, true);
//
//    return EXIT_SUCCESS;
//}












/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "/Users/alexanderborisov/new/lexbor/lexbor/examples/lexbor/html/base.h"
#include "lexbor/core/fs.h"

int
main(int argc, const char *argv[])
{
    lxb_html_element_t *element;
    lxb_html_body_element_t *body;
    lxb_html_document_t *document;

    static const lxb_char_t html[] = "<svg><a xlink:title>";
    size_t html_len = sizeof(html) - 1;

    /* Parse */
    document = parse(html, html_len);

    /* Print Incoming Data */
    PRINT("HTML:");
    PRINT("%s", (const char *) html);
    PRINT("\nTree after parse:");
    serialize(lxb_dom_interface_node(document));

    /* Get BODY element */
    body = lxb_html_document_body_element(document);

    /* Print Result */
    PRINT("\nTree after innerHTML set:");
    serialize(lxb_dom_interface_node(document));

    /* Destroy all */
    lxb_html_document_destroy(document);

    return 0;
}




















///*
// * Copyright (C) 2021 Alexander Borisov
// *
// * Author: Alexander Borisov <borisov@lexbor.com>
// */
//
//#include <lexbor/core/core.h>
//#include <lexbor/html/html.h>
//#include <lexbor/css/css.h>
//#include <lexbor/selectors/selectors.h>
//
//
//
//lxb_status_t
//callback(const lxb_char_t *data, size_t len, void *ctx)
//{
//    printf("%.*s", (int) len, (const char *) data);
//
//    return LXB_STATUS_OK;
//}
//
//int
//main(int argc, const char *argv[])
//{
//    lxb_status_t status;
//    lxb_css_parser_t *parser;
//    lxb_css_stylesheet_t *stylesheet;
//
//    /* CSS Data. */
//
////    static const lxb_char_t css[] = "@namespace 'https://html.org'\n"
////    "div p > span {width: 100%}\n"
////    ":not(:has(p)) {height: 30px}";
//
////    static const lxb_char_t css[] = "@media 'https://html.org';\n"
////    "#id .class {width: 10%; height: 200%}\n"
////    "@media 'https://html.org';\n";
//
////    static const lxb_char_t css[] = ":nth-child(2n+2 of :has(, :nth-child(1%)))";
//
////    static const lxb_char_t css[] = "#id .class {width: 123px; broken declaration; @font 'http://x.x/' {size: 10px} height: 45pt} "
////    ":has(:not([href='https://'])) {width: 321px; broxcxcken declarasdsdsdstion; @fxzcczxczcont 'http://x.zcxxzcx/' {size: 10444px} height: 45222pt}";
//
//    static const lxb_char_t css[] = "#id {width: !important}";
//
////    static const lxb_char_t css[] = "#a {@xxxx}";
////    "#id .class {aaaewe: 23424; width: 10%; @fdsfd  'https://html.org'; height: 200%}\n"
////    "1% .asdad #aasas {lala: 1; h: 'asass'; color: blue;}";
//
////    static const lxb_char_t css[] = "#id .class {width: 100px}";
//
//     size_t length = (sizeof(css) / sizeof(lxb_char_t)) - 1;
//
////    size_t length;
////    lxb_char_t *css = lexbor_fs_file_easy_read("/Users/alexanderborisov/new/lexbor/lexbor/test/files/lexbor/css/lexbor.css", &length);
//
//    /* Create CSS parser. */
//
//    parser = lxb_css_parser_create();
//    status = lxb_css_parser_init(parser, NULL, NULL, 128);
//    if (status != LXB_STATUS_OK) {
//        return EXIT_FAILURE;
//    }
//
//
////    lxb_css_parser_buffer_set(parser, css, length);
////
////    lxb_css_syntax_token_t *token;
////    bool run = true;
////
////    do {
////        token = lxb_css_syntax_token(parser->tkz);
////        if (token == NULL) {
////            parser->status = parser->status;
////            return parser->status;
////        }
////
////        if (token->type == LXB_CSS_SYNTAX_TOKEN__EOF) {
////            run = false;
////        }
////
////        lxb_css_syntax_token_consume(parser->tkz);
////    }
////    while (run);
//
//
//
//
//
////    lxb_css_syntax_anb_t anb = lxb_css_syntax_anb_parse(parser, css, length);
////    lxb_css_syntax_anb_serialize(&anb, callback, NULL);
//
//
//
//
////    lxb_css_selector_list_t *list;
////
////    list = lxb_css_selectors_parse(parser, css, length);
////
////    lxb_css_selector_serialize_list_chain(list, callback, NULL);
////    printf("\n");
////    lxb_css_log_serialize(parser->log, callback, NULL, " ", 1);
//
//
//
//
//
//
//
//
//
////
////    /* Create CSS StyleSheet parser. */
////
////    ssp = lxb_css_stylesheet_parser_create();
////    status = lxb_css_stylesheet_parser_init(ssp);
////    if (status != LXB_STATUS_OK) {
////        return EXIT_FAILURE;
////    }
////
////    /*
////     * It is important that a new StyleSheet object is not created internally
////     * for each call to the parser.
////     */
////    lxb_css_parser_stylesheet_set(parser, ssp);
////
////    /* Parse and get the log. */
////
//////    parser->receive_endings = true;
////
//
//
//
//
////    lxb_css_rule_declaration_list_t *dlist;
////    dlist = lxb_css_declaration_list_parse(parser, css, length);
////
////    printf("Declaration List: \n");
////    (void) lxb_css_rule_serialize_chain(&dlist->rule, callback, NULL);
////    printf("\n");
//
//
//
//
//    stylesheet = lxb_css_stylesheet_parse(parser, css, length);
//    if (stylesheet == NULL) {
//        return EXIT_FAILURE;
//    }
//
//
//    /* StyleSheet Serialization. */
//
//    printf("StyleSheet: \n");
//    (void) lxb_css_rule_serialize_chain(stylesheet->root, callback, NULL);
//    printf("\n");
//
//
//
//
//
//    /* Destroy CSS StyleSheet memory. */
////    (void) lxb_css_stylesheet_parser_destroy(ssp, true);
//
//    /* Destroy resources for CSS Parser. */
//    (void) lxb_css_parser_destroy(parser, true, true);
//
//    return EXIT_SUCCESS;
//}






































//
///*
// * Copyright (C) 2022 Alexander Borisov
// *
// * Author: Alexander Borisov <borisov@lexbor.com>
// */
//
//#include <lexbor/core/fs.h>
//#include <lexbor/core/array.h>
//
//#include <unit/test.h>
//#include <unit/kv.h>
//
//#include <lexbor/css/css.h>
//
//
//#define validate_pointer(ptr, name)                                           \
//if (ptr == NULL) {                                                        \
//TEST_PRINTLN("Required parameter missing: %s", (const char *) name);  \
//return LXB_STATUS_ERROR_UNEXPECTED_DATA; \
//}
//
//
//typedef struct {
//    unit_kv_t        *kv;
//    lxb_css_parser_t *parser;
//    lexbor_str_t     str;
//    lexbor_mraw_t    *mraw;
//}
//helper_t;
//
//typedef struct {
//    const lxb_char_t *begin;
//    const lxb_char_t *end;
//    lxb_char_t       ch;
//}
//chunk_ctx_t;
//
//typedef lxb_status_t
//(*parse_data_cb_f)(helper_t *helper, lexbor_str_t *str,
//                   unit_kv_array_t *entries);
//
//
//static lxb_status_t
//parse(helper_t *helper, const char *dir_path);
//
//static lexbor_action_t
//file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
//              const lxb_char_t *filename, size_t filename_len, void *ctx);
//
//static lxb_status_t
//check(helper_t *helper, unit_kv_value_t *value);
//
//static lxb_status_t
//check_entry(helper_t *helper, unit_kv_value_t *entry, parse_data_cb_f cb);
//
//static lxb_status_t
//parse_cb(helper_t *helper, lexbor_str_t *str, unit_kv_array_t *entries);
//
//static lxb_status_t
//parse_chunk_cb(helper_t *helper, lexbor_str_t *str, unit_kv_array_t *entries);
//
//static lxb_status_t
//chunk_cb(lxb_css_syntax_tokenizer_t *tkz, const lxb_char_t **data,
//         const lxb_char_t **end, void *ctx);
//
//static lxb_status_t
//print_error(helper_t *helper, unit_kv_value_t *value);
//
//static bool
//validate_type(unit_kv_value_t *value, const lexbor_str_t *need);
//
//static lxb_status_t
//compare(lxb_css_parser_t *parser, helper_t *helper, unit_kv_array_t *arr,
//        lxb_css_rule_declaration_list_t *list);
//
//
//int
//main(int argc, const char * argv[])
//{
//    lxb_status_t status;
//    helper_t helper = {0};
//    const char *dir_path;
//
//    if (argc != 2) {
//        printf("Usage:\n\tcss_parser <directory path>\n");
//        return EXIT_FAILURE;
//    }
//
//    dir_path = argv[1];
//
//    TEST_INIT();
//
//    helper.kv = unit_kv_create();
//    status = unit_kv_init(helper.kv, 256);
//
//    if (status != LXB_STATUS_OK) {
//        goto failed;
//    }
//
//    helper.mraw = lexbor_mraw_create();
//    status = lexbor_mraw_init(helper.mraw, 256);
//    if (status != LXB_STATUS_OK) {
//        goto failed;
//    }
//
//    status = parse(&helper, dir_path);
//    if (status != LXB_STATUS_OK) {
//        goto failed;
//    }
//
//    TEST_RUN("lexbor/css/declarations");
//
//    unit_kv_destroy(helper.kv, true);
//    lexbor_mraw_destroy(helper.mraw, true);
//
//    TEST_RELEASE();
//
//failed:
//
//    unit_kv_destroy(helper.kv, true);
//    lexbor_mraw_destroy(helper.mraw, true);
//
//    return EXIT_FAILURE;
//}
//
//static lxb_status_t
//parse(helper_t *helper, const char *dir_path)
//{
//    lxb_status_t status;
//
//    status = lexbor_fs_dir_read((const lxb_char_t *) dir_path,
//                                LEXBOR_FS_DIR_OPT_WITHOUT_DIR
//                                |LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN,
//                                file_callback, helper);
//
//    if (status != LXB_STATUS_OK) {
//        TEST_PRINTLN("Failed to read directory: %s", dir_path);
//    }
//
//    return status;
//}
//
//static lexbor_action_t
//file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
//              const lxb_char_t *filename, size_t filename_len, void *ctx)
//{
//    lxb_status_t status;
//    unit_kv_value_t *value;
//    helper_t *helper;
//
//    if (filename_len < 5 ||
//        strncmp((const char *) &filename[ (filename_len - 4) ], ".ton", 4) != 0)
//    {
//        return LEXBOR_ACTION_OK;
//    }
//
//    helper = ctx;
//
//    TEST_PRINTLN("Parse file: %s", fullpath);
//
//    unit_kv_clean(helper->kv);
//
//    status = unit_kv_parse_file(helper->kv, (const lxb_char_t *) fullpath);
//    if (status != LXB_STATUS_OK) {
//        lexbor_str_t str = unit_kv_parse_error_as_string(helper->kv);
//
//        TEST_PRINTLN("%s", str.data);
//
//        unit_kv_string_destroy(helper->kv, &str, false);
//
//        return EXIT_FAILURE;
//    }
//
//    value = unit_kv_value(helper->kv);
//    if (value == NULL) {
//        TEST_PRINTLN("Failed to get root value");
//        return EXIT_FAILURE;
//    }
//
//    TEST_PRINTLN("Check file: %s", fullpath);
//
//    status = check(helper, value);
//    if (status != LXB_STATUS_OK) {
//        exit(EXIT_FAILURE);
//    }
//
//    if (helper->parser != NULL) {
//        helper->parser = lxb_css_parser_destroy(helper->parser, true, true);
//    }
//
//    return LEXBOR_ACTION_OK;
//}
//
//static lxb_status_t
//check(helper_t *helper, unit_kv_value_t *value)
//{
//    lxb_status_t status;
//    unit_kv_array_t *entries;
//
//    if (unit_kv_is_array(value) == false) {
//        print_error(helper, value);
//
//        return LXB_STATUS_ERROR;
//    }
//
//    entries = unit_kv_array(value);
//
//    for (size_t i = 0; i < entries->length; i++) {
//        if (unit_kv_is_hash(entries->list[i]) == false) {
//            return print_error(helper, entries->list[i]);
//        }
//
//        TEST_PRINTLN("Test #"LEXBOR_FORMAT_Z, (i + 1));
//
//        status = check_entry(helper, entries->list[i], parse_cb);
//        if (status != LXB_STATUS_OK) {
//            return status;
//        }
//
//        lxb_css_parser_clean(helper->parser);
//
//        TEST_PRINTLN("Test #"LEXBOR_FORMAT_Z" chunks", (i + 1));
//
//        status = check_entry(helper, entries->list[i], parse_chunk_cb);
//        if (status != LXB_STATUS_OK) {
//            return status;
//        }
//
//        lxb_css_parser_clean(helper->parser);
//    }
//
//    return LXB_STATUS_OK;
//}
//
//static lxb_status_t
//check_entry(helper_t *helper, unit_kv_value_t *entry, parse_data_cb_f cb)
//{
//    lxb_status_t status;
//    lexbor_str_t *str;
//    unit_kv_array_t *entries;
//    unit_kv_value_t *data, *results;
//
//    /* Validate */
//    data = unit_kv_hash_value_nolen_c(entry, "data");
//    if (data == NULL) {
//        TEST_PRINTLN("Required parameter missing: data");
//        return print_error(helper, entry);
//    }
//
//    if (unit_kv_is_string(data) == false) {
//        TEST_PRINTLN("Parameter 'data' must be a STRING");
//        return print_error(helper, data);
//    }
//
//    results = unit_kv_hash_value_nolen_c(entry, "results");
//    if (results == NULL) {
//        TEST_PRINTLN("Required parameter missing: results");
//        return print_error(helper, entry);
//    }
//
//    if (unit_kv_is_array(results) == false) {
//        TEST_PRINTLN("Parameter 'results' must be an ARRAY");
//        return print_error(helper, results);
//    }
//
//    /* Parse */
//
//    str = unit_kv_string(data);
//    entries = unit_kv_array(results);
//
//    status = cb(helper, str, entries);
//
//    if (status != LXB_STATUS_OK) {
//        TEST_PRINTLN("Failed to CSS");
//
//        return print_error(helper, data);
//    }
//
//    return LXB_STATUS_OK;
//}
//
//static lxb_status_t
//parse_cb(helper_t *helper, lexbor_str_t *str, unit_kv_array_t *entries)
//{
//    lxb_status_t status;
//    lxb_css_rule_declaration_list_t *list;
//
//    if (helper->parser != NULL) {
//        helper->parser = lxb_css_parser_destroy(helper->parser, true, true);
//    }
//
//    helper->parser = lxb_css_parser_create();
//    status = lxb_css_parser_init(helper->parser, NULL, NULL, 64);
//    if (status != LXB_STATUS_OK) {
//        goto failed;
//    }
//
//    list = lxb_css_declaration_list_parse(helper->parser,
//                                          str->data, str->length);
//    if (list == NULL) {
//        status = helper->parser->status;
//        goto failed;
//    }
//
//    return LXB_STATUS_OK;
//
//failed:
//
//    helper->parser = lxb_css_parser_destroy(helper->parser, true, true);
//
//    return status;
//}
//
//static lxb_status_t
//parse_chunk_cb(helper_t *helper, lexbor_str_t *str, unit_kv_array_t *entries)
//{
//    chunk_ctx_t ctx;
//    lxb_status_t status;
//    lxb_css_rule_declaration_list_t *list;
//
//    if (helper->parser != NULL) {
//        helper->parser = lxb_css_parser_destroy(helper->parser, true, true);
//    }
//
//    helper->parser = lxb_css_parser_create();
//    status = lxb_css_parser_init(helper->parser, NULL, NULL, 64);
//    if (status != LXB_STATUS_OK) {
//        goto failed;
//    }
//
//    ctx.begin = str->data;
//    ctx.end = str->data + str->length;
//    ctx.ch = *str->data;
//
//    lxb_css_syntax_tokenizer_chunk_cb_set(helper->parser->tkz, chunk_cb, &ctx);
//
//    list = lxb_css_declaration_list_parse(helper->parser, &ctx.ch, 1);
//    if (list == NULL) {
//        status = helper->parser->status;
//        goto failed;
//    }
//
//    return LXB_STATUS_OK;
//
//failed:
//
//    helper->parser = lxb_css_parser_destroy(helper->parser, true, true);
//
//    return status;
//}
//
//static lxb_status_t
//chunk_cb(lxb_css_syntax_tokenizer_t *tkz, const lxb_char_t **data,
//         const lxb_char_t **end, void *ctx)
//{
//    chunk_ctx_t *chunk = ctx;
//
//    chunk->begin++;
//    chunk->ch = *chunk->begin;
//
//    if (chunk->begin < chunk->end) {
//        *data = &chunk->ch;
//        *end = *data + 1;
//    }
//
//    return LXB_STATUS_OK;
//}
//
//static lxb_status_t
//print_error(helper_t *helper, unit_kv_value_t *value)
//{
//    lexbor_str_t str;
//
//    str = unit_kv_value_position_as_string(helper->kv, value);
//    TEST_PRINTLN("%s", str.data);
//    unit_kv_string_destroy(helper->kv, &str, false);
//
//    str = unit_kv_value_fragment_as_string(helper->kv, value);
//    TEST_PRINTLN("%s", str.data);
//    unit_kv_string_destroy(helper->kv, &str, false);
//
//    return LXB_STATUS_ERROR;
//}
//
//static bool
//validate_type(unit_kv_value_t *value, const lexbor_str_t *need)
//{
//    lexbor_str_t *type;
//
//    if (!unit_kv_is_string(value)) {
//        return false;
//    }
//
//    type = unit_kv_string(value);
//
//    if (need->length != type->length) {
//        return false;
//    }
//
//    return lexbor_str_data_ncmp(need->data, type->data, type->length);
//}
//
//static lxb_status_t
//compare(lxb_css_parser_t *parser, helper_t *helper, unit_kv_array_t *arr,
//        lxb_css_rule_declaration_list_t *list)
//{
//    bool imp;
//    size_t i = 0;
//    lexbor_str_t *str, *str_name, *str_value;
//    lxb_status_t status;
//    unit_kv_value_t *entry, *type, *name, *value, *important;
//    lxb_css_property_type_t ntype;
//    lxb_css_rule_declaration_t *declr;
//    lxb_css_syntax_token_type_t token_type;
//
//    static const lexbor_str_t str_und = lexbor_str("undef");
//    static const lexbor_str_t str_cst = lexbor_str("custom");
//    static const lexbor_str_t str_pro = lexbor_str("property");
//
//    if (arr->length != list->count) {
//        TEST_PRINTLN("Result expected "LEXBOR_FORMAT_Z" received "LEXBOR_FORMAT_Z,
//                     arr->length, list->count);
//        return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
//    }
//
//    declr = lxb_css_rule_declaration(list->first);
//
//    while (i < arr->length) {
//        entry = arr->list[i++];
//
//        if (!unit_kv_is_hash(entry)) {
//            return false;
//        }
//
//        type = unit_kv_hash_value_nolen_c(entry, "type");
//        validate_pointer(type, "type");
//        if (!unit_kv_is_string(type)) {
//            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
//        }
//
//        name = unit_kv_hash_value_nolen_c(entry, "name");
//        validate_pointer(name, "name");
//        if (!unit_kv_is_string(name)) {
//            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
//        }
//
//        value = unit_kv_hash_value_nolen_c(entry, "value");
//        validate_pointer(value, "value");
//        if (!unit_kv_is_string(value)) {
//            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
//        }
//
//        important = unit_kv_hash_value_nolen_c(entry, "important");
//        validate_pointer(important, "important");
//        if (!unit_kv_is_bool(important)) {
//            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
//        }
//
//        /* Type. */
//
//        str = unit_kv_string(type);
//
//        if (str->length == str_und.length
//            && lexbor_str_data_ncmp(str->data, str_und.data, str->length))
//        {
//            ntype = LXB_CSS_PROPERTY__UNDEF;
//        }
//        else if (str->length == str_cst.length
//                 && lexbor_str_data_ncmp(str->data, str_cst.data, str->length))
//        {
//            ntype = LXB_CSS_PROPERTY__CUSTOM;
//        }
//        else if (str->length == str_pro.length
//                 && lexbor_str_data_ncmp(str->data, str_pro.data, str->length))
//        {
//            ntype = LXB_CSS_PROPERTY__LAST_ENTRY;
//        }
//        else {
//            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
//        }
//
//        /* Data. */
//
//        str_name = unit_kv_string(name);
//        str_value = unit_kv_string(value);
//        imp = unit_kv_bool(important);
//
//        /* Check. */
//
//        if (declr->type != ntype && declr->type < LXB_CSS_PROPERTY__CUSTOM) {
//            TEST_PRINTLN("Type does not match.");
//            return LXB_STATUS_ERROR_UNEXPECTED_RESULT;
//        }
//
//
//
//        lxb_css_rule_
//
//
//
//
//
//        status = lxb_css_syntax_token_serialize_str(token, &helper->str,
//                                                    helper->mraw);
//        if (status != LXB_STATUS_OK) {
//            return false;
//        }
//
//        str = unit_kv_string(value);
//
//        if (str->length != helper->str.length
//            || lexbor_str_data_ncmp(str->data,
//                                    helper->str.data, str->length) == false)
//        {
//            lexbor_str_clean(&helper->str);
//            return false;
//        }
//
//        lexbor_str_clean(&helper->str);
//
//        lxb_css_syntax_parser_consume(parser);
//        token = lxb_css_syntax_parser_token(parser);
//
//        declr = lxb_css_rule_declaration(declr->rule.next);
//    }
//
//    *out_length = i;
//
//    return i == arr->length && token->type == LXB_CSS_SYNTAX_TOKEN__TERMINATED;
//}










































///*
// * Copyright (C) 2019-2021 Alexander Borisov
// *
// * Author: Alexander Borisov <borisov@lexbor.com>
// */
//
//#include <lexbor/core/fs.h>
//#include <lexbor/core/array.h>
//
//#include <unit/test.h>
//#include <unit/kv.h>
//
//#include <lexbor/css/syntax/tokenizer.h>
//
//
//typedef struct {
//    unit_kv_t                  *kv;
//    lxb_css_syntax_tokenizer_t *tkz;
//    lexbor_str_t               str;
//    lexbor_mraw_t              *mraw;
//    lexbor_array_t             tokens;
//}
//helper_t;
//
//typedef struct {
//    const lxb_char_t *begin;
//    const lxb_char_t *end;
//    lxb_char_t       ch;
//}
//chunk_ctx_t;
//
//typedef lxb_status_t
//(*parse_data_cb_f)(helper_t *helper, lexbor_str_t *str);
//
//
//static lxb_status_t
//parse(helper_t *helper, const char *dir_path);
//
//static lexbor_action_t
//file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
//              const lxb_char_t *filename, size_t filename_len, void *ctx);
//
//static lxb_status_t
//check(helper_t *helper, unit_kv_value_t *value);
//
//static lxb_status_t
//check_entry(helper_t *helper, unit_kv_value_t *entry, parse_data_cb_f cb);
//
//static lxb_status_t
//parse_data_new_tkz_cb(helper_t *helper, lexbor_str_t *str);
//
//static lxb_status_t
//parse_data_new_tkz_chunk_cb(helper_t *helper, lexbor_str_t *str);
//
//static lxb_status_t
//chunk_cb(lxb_css_syntax_tokenizer_t *tkz, const lxb_char_t **data,
//         const lxb_char_t **end, void *ctx);
//
//static lxb_status_t
//check_token(helper_t *helper, unit_kv_value_t *entry,
//            lxb_css_syntax_token_t *token);
//
//static lxb_status_t
//print_error(helper_t *helper, unit_kv_value_t *value);
//
//
//int
//main(int argc, const char * argv[])
//{
//    lxb_status_t status;
//    helper_t helper = {0};
//    const char *dir_path;
//
////    if (argc != 2) {
////        printf("Usage:\n\tcss_tokenizer <directory path>\n");
////        return EXIT_FAILURE;
////    }
//
////    dir_path = argv[1];
//
//    dir_path = "/Users/alexanderborisov/new/lexbor/lexbor/test/files/lexbor/css/syntax/tokenizer";
//
//    TEST_INIT();
//
//    helper.kv = unit_kv_create();
//    status = unit_kv_init(helper.kv, 256);
//
//    if (status != LXB_STATUS_OK) {
//        goto failed;
//    }
//
//    status = lexbor_array_init(&helper.tokens, 4096);
//    if (status != LXB_STATUS_OK) {
//        goto failed;
//    }
//
//    helper.mraw = lexbor_mraw_create();
//    status = lexbor_mraw_init(helper.mraw, 256);
//    if (status != LXB_STATUS_OK) {
//        goto failed;
//    }
//
//    status = parse(&helper, dir_path);
//    if (status != LXB_STATUS_OK) {
//        goto failed;
//    }
//
//    TEST_RUN("lexbor/css/tokenizer");
//
//    unit_kv_destroy(helper.kv, true);
//    lexbor_array_destroy(&helper.tokens, false);
//    lexbor_mraw_destroy(helper.mraw, true);
//
//    TEST_RELEASE();
//
//failed:
//
//    unit_kv_destroy(helper.kv, true);
//    lexbor_array_destroy(&helper.tokens, false);
//    lexbor_mraw_destroy(helper.mraw, true);
//
//    return EXIT_FAILURE;
//}
//
//static lxb_status_t
//parse(helper_t *helper, const char *dir_path)
//{
//    lxb_status_t status;
//
//    status = lexbor_fs_dir_read((const lxb_char_t *) dir_path,
//                                LEXBOR_FS_DIR_OPT_WITHOUT_DIR
//                                |LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN,
//                                file_callback, helper);
//
//    if (status != LXB_STATUS_OK) {
//        TEST_PRINTLN("Failed to read directory: %s", dir_path);
//    }
//
//    return status;
//}
//
//
//static lexbor_action_t
//file_callback(const lxb_char_t *fullpath, size_t fullpath_len,
//              const lxb_char_t *filename, size_t filename_len, void *ctx)
//{
//    lxb_status_t status;
//    unit_kv_value_t *value;
//    helper_t *helper;
//
//    if (filename_len < 5 ||
//        strncmp((const char *) &filename[ (filename_len - 4) ], ".ton", 4) != 0)
//    {
//        return LEXBOR_ACTION_OK;
//    }
//
//    helper = ctx;
//
//    TEST_PRINTLN("Parse file: %s", fullpath);
//
//    unit_kv_clean(helper->kv);
//
//    status = unit_kv_parse_file(helper->kv, (const lxb_char_t *) fullpath);
//    if (status != LXB_STATUS_OK) {
//        lexbor_str_t str = unit_kv_parse_error_as_string(helper->kv);
//
//        TEST_PRINTLN("%s", str.data);
//
//        unit_kv_string_destroy(helper->kv, &str, false);
//
//        return EXIT_FAILURE;
//    }
//
//    value = unit_kv_value(helper->kv);
//    if (value == NULL) {
//        TEST_PRINTLN("Failed to get root value");
//        return EXIT_FAILURE;
//    }
//
//    TEST_PRINTLN("Check file: %s", fullpath);
//
//    status = check(helper, value);
//    if (status != LXB_STATUS_OK) {
//        exit(EXIT_FAILURE);
//    }
//
//    if (helper->tkz != NULL) {
//        helper->tkz = lxb_css_syntax_tokenizer_destroy(helper->tkz);
//    }
//
//    return LEXBOR_ACTION_OK;
//}
//
//static lxb_status_t
//check(helper_t *helper, unit_kv_value_t *value)
//{
//    lxb_status_t status;
//    unit_kv_array_t *entries;
//
//    if (unit_kv_is_array(value) == false) {
//        print_error(helper, value);
//
//        return LXB_STATUS_ERROR;
//    }
//
//    entries = unit_kv_array(value);
//
//    for (size_t i = 0; i < entries->length; i++) {
//        if (unit_kv_is_hash(entries->list[i]) == false) {
//            return print_error(helper, entries->list[i]);
//        }
//
//        TEST_PRINTLN("Test #"LEXBOR_FORMAT_Z, (i + 1));
//
//        status = check_entry(helper, entries->list[i], parse_data_new_tkz_cb);
//        if (status != LXB_STATUS_OK) {
//            return status;
//        }
//
//        lexbor_array_clean(&helper->tokens);
//        lxb_css_syntax_tokenizer_clean(helper->tkz);
//
//        TEST_PRINTLN("Test #"LEXBOR_FORMAT_Z" chunks", (i + 1));
//
//        status = check_entry(helper, entries->list[i],
//                             parse_data_new_tkz_chunk_cb);
//        if (status != LXB_STATUS_OK) {
//            return status;
//        }
//
//        lexbor_array_clean(&helper->tokens);
//        lxb_css_syntax_tokenizer_clean(helper->tkz);
//    }
//
//    return LXB_STATUS_OK;
//}
//
//static lxb_status_t
//check_entry(helper_t *helper, unit_kv_value_t *entry, parse_data_cb_f cb)
//{
//    lxb_status_t status;
//    lexbor_str_t *str;
//    unit_kv_array_t *token_entries;
//    unit_kv_value_t *data, *tokens;
//
//    /* Validate */
//    data = unit_kv_hash_value_nolen_c(entry, "data");
//    if (data == NULL) {
//        TEST_PRINTLN("Required parameter missing: data");
//
//        return print_error(helper, entry);
//    }
//
//    if (unit_kv_is_string(data) == false) {
//        TEST_PRINTLN("Parameter 'data' must be a STRING");
//
//        return print_error(helper, data);
//    }
//
//    tokens = unit_kv_hash_value_nolen_c(entry, "tokens");
//    if (tokens == NULL) {
//        TEST_PRINTLN("Required parameter missing: tokens");
//
//        return print_error(helper, entry);
//    }
//
//    if (unit_kv_is_array(tokens) == false) {
//        TEST_PRINTLN("Parameter 'tokens' must be an ARRAY");
//
//        return print_error(helper, tokens);
//    }
//
//    /* Parse */
//    str = unit_kv_string(data);
//
//    status = cb(helper, str);
//
//    if (status != LXB_STATUS_OK) {
//        TEST_PRINTLN("Failed to CSS");
//
//        return print_error(helper, data);
//    }
//
//    token_entries = unit_kv_array(tokens);
//
//    if (token_entries->length != helper->tokens.length) {
//        TEST_PRINTLN("Expected number of tokens does "
//                     "not converge with the received. "
//                     "Have: "LEXBOR_FORMAT_Z"; Need: "LEXBOR_FORMAT_Z,
//                     helper->tokens.length, token_entries->length);
//
//        return print_error(helper, data);
//    }
//
//    for (size_t i = 0; i < token_entries->length; i++) {
//        if (unit_kv_is_hash(token_entries->list[i]) == false) {
//            print_error(helper, token_entries->list[i]);
//            return LXB_STATUS_ERROR;
//        }
//
//        status = check_token(helper, token_entries->list[i],
//                             helper->tokens.list[i]);
//        if (status != LXB_STATUS_OK) {
//            return status;
//        }
//    }
//
//    return LXB_STATUS_OK;
//}
//
//static void
//check_raw(lxb_css_syntax_token_t *token)
//{
//    volatile lxb_char_t ch;
//    const lxb_char_t *p, *end;
//
//    p = lxb_css_syntax_token_base(token)->begin;
//    end = p + lxb_css_syntax_token_base(token)->length;
//
//    while (p < end) {
//        ch = *p++;
//        ch++;
//    }
//
//    if (token->type == LXB_CSS_SYNTAX_TOKEN_DIMENSION) {
//        p = lxb_css_syntax_token_dimension_string(token)->base.begin;
//        end = p + lxb_css_syntax_token_dimension_string(token)->base.length;
//
//        while (p < end) {
//            ch = *p++;
//            ch++;
//        }
//    }
//}
//
//static lxb_status_t
//parse_data_new_tkz_cb(helper_t *helper, lexbor_str_t *str)
//{
//    lxb_status_t status;
//    lxb_css_syntax_token_t *token;
//
//    if (helper->tkz != NULL) {
//        helper->tkz = lxb_css_syntax_tokenizer_destroy(helper->tkz);
//    }
//
//    helper->tkz = lxb_css_syntax_tokenizer_create();
//    status = lxb_css_syntax_tokenizer_init(helper->tkz);
//    if (status != LXB_STATUS_OK) {
//        helper->tkz = lxb_css_syntax_tokenizer_destroy(helper->tkz);
//
//        return status;
//    }
//
//    helper->tkz->with_comment = true;
//
//    lxb_css_syntax_tokenizer_buffer_set(helper->tkz, str->data, str->length);
//
//    do {
//        token = lxb_css_syntax_token_next(helper->tkz);
//        if (token == NULL) {
//            return helper->tkz->status;
//        }
//
//        check_raw(token);
//
//        if (lxb_css_syntax_token_type(token) != LXB_CSS_SYNTAX_TOKEN__EOF) {
//            status = lexbor_array_push(&helper->tokens, token);
//            if (status != LXB_STATUS_OK) {
//                return status;
//            }
//        }
//    }
//    while (lxb_css_syntax_token_type(token) != LXB_CSS_SYNTAX_TOKEN__EOF);
//
//    return LXB_STATUS_OK;
//}
//
//static lxb_status_t
//parse_data_new_tkz_chunk_cb(helper_t *helper, lexbor_str_t *str)
//{
//    chunk_ctx_t ctx;
//    lxb_status_t status;
//    lxb_css_syntax_token_t *token;
//
//    if (helper->tkz != NULL) {
//        helper->tkz = lxb_css_syntax_tokenizer_destroy(helper->tkz);
//    }
//
//    helper->tkz = lxb_css_syntax_tokenizer_create();
//    status = lxb_css_syntax_tokenizer_init(helper->tkz);
//    if (status != LXB_STATUS_OK) {
//        helper->tkz = lxb_css_syntax_tokenizer_destroy(helper->tkz);
//
//        return status;
//    }
//
//    helper->tkz->with_comment = true;
//
//    ctx.begin = str->data;
//    ctx.end = str->data + str->length;
//    ctx.ch = *str->data;
//
//    lxb_css_syntax_tokenizer_buffer_set(helper->tkz, &ctx.ch, 1);
//
//    lxb_css_syntax_tokenizer_chunk_cb_set(helper->tkz, chunk_cb, &ctx);
//
//    do {
//        token = lxb_css_syntax_token_next(helper->tkz);
//        if (token == NULL) {
//            return helper->tkz->status;
//        }
//
//        if (lxb_css_syntax_token_type(token) != LXB_CSS_SYNTAX_TOKEN__EOF) {
//            status = lexbor_array_push(&helper->tokens, token);
//            if (status != LXB_STATUS_OK) {
//                return status;
//            }
//        }
//    }
//    while (lxb_css_syntax_token_type(token) != LXB_CSS_SYNTAX_TOKEN__EOF);
//
//    return LXB_STATUS_OK;
//}
//
//static lxb_status_t
//chunk_cb(lxb_css_syntax_tokenizer_t *tkz, const lxb_char_t **data,
//         const lxb_char_t **end, void *ctx)
//{
//    chunk_ctx_t *chunk = ctx;
//
//    chunk->begin++;
//    chunk->ch = *chunk->begin;
//
//    if (chunk->begin < chunk->end) {
//        *data = &chunk->ch;
//        *end = *data + 1;
//    }
//
//    return LXB_STATUS_OK;
//}
//
//static lxb_status_t
//check_token(helper_t *helper, unit_kv_value_t *entry,
//            lxb_css_syntax_token_t *token)
//{
//    size_t length;
//    lexbor_str_t *str;
//    lxb_status_t status;
//    unit_kv_value_t *value;
//    lxb_css_syntax_token_type_t type;
//
//    /* Check type */
//
//    value = unit_kv_hash_value_nolen_c(entry, "type");
//    if (value == NULL) {
//        TEST_PRINTLN("Required parameter missing: type");
//
//        return print_error(helper, entry);
//    }
//
//    if (unit_kv_is_string(value) == false) {
//        TEST_PRINTLN("Parameter 'type' of token must be a STRING");
//
//        return print_error(helper, value);
//    }
//
//    str = unit_kv_string(value);
//
//    type = lxb_css_syntax_token_type_id_by_name(str->data, str->length);
//
//    if (type != token->type) {
//        const lxb_char_t *type_name;
//
//        type_name = lxb_css_syntax_token_type_name_by_id(token->type);
//
//        TEST_PRINTLN("Parameter 'type' not match; Have: %s; Need: %s",
//                     (char *) type_name, (char *) str->data);
//
//        return print_error(helper, value);
//    }
//
//    /* Check value */
//
//    value = unit_kv_hash_value_nolen_c(entry, "value");
//    if (value == NULL) {
//        return LXB_STATUS_OK;
//    }
//
//    if (unit_kv_is_string(value) == false) {
//        TEST_PRINTLN("Parameter 'value' of token must be a STRING");
//
//        return print_error(helper, value);
//    }
//
//    status = lxb_css_syntax_token_serialize_str(token, &helper->str,
//                                                helper->mraw);
//    if (status != LXB_STATUS_OK) {
//        TEST_PRINTLN("Failed to serialization token");
//
//        return print_error(helper, value);
//    }
//
//    str = unit_kv_string(value);
//
//    if (str->length != helper->str.length
//        || lexbor_str_data_ncmp(str->data, helper->str.data, str->length) == false)
//    {
//        lexbor_str_clean(&helper->str);
//
//        TEST_PRINTLN("Token not match. \nHave:\n%s\nNeed:\n%s",
//                     (const char *) helper->str.data, (const char *) str->data);
//
//        return print_error(helper, value);
//    }
//
//    lexbor_str_clean(&helper->str);
//
//    /* Check length */
//
//    value = unit_kv_hash_value_nolen_c(entry, "length");
//    if (value == NULL) {
//        return LXB_STATUS_OK;
//    }
//
//    length = (token->type == LXB_CSS_SYNTAX_TOKEN_DIMENSION)
//              ? token->types.dimension.num.base.length
//                + token->types.dimension.str.base.length
//              : lxb_css_syntax_token_base(token)->length;
//
//    if (unit_kv_number(value)->value.l != length) {
//        TEST_PRINTLN("Token length not match. \nHave:\n"
//                     LEXBOR_FORMAT_Z"\nNeed:\n%ld",
//                     length, unit_kv_number(value)->value.l);
//
//        return print_error(helper, value);
//    }
//
//    return LXB_STATUS_OK;
//}
//
//static lxb_status_t
//print_error(helper_t *helper, unit_kv_value_t *value)
//{
//    lexbor_str_t str;
//
//    str = unit_kv_value_position_as_string(helper->kv, value);
//    TEST_PRINTLN("%s", str.data);
//    unit_kv_string_destroy(helper->kv, &str, false);
//
//    str = unit_kv_value_fragment_as_string(helper->kv, value);
//    TEST_PRINTLN("%s", str.data);
//    unit_kv_string_destroy(helper->kv, &str, false);
//
//    return LXB_STATUS_ERROR;
//}












































//
//#include <lexbor/html/html.h>
//#include <lexbor/css/css.h>
//#include <lexbor/selectors/selectors.h>
//
//
//lxb_status_t
//callback(const lxb_char_t *data, size_t len, void *ctx)
//{
//    printf("%.*s", (int) len, (const char *) data);
//
//    return LXB_STATUS_OK;
//}
//
//lxb_status_t
//find_callback(lxb_dom_node_t *node, lxb_css_selector_specificity_t *spec,
//              void *ctx)
//{
//    return lxb_html_serialize_deep_cb(node, callback, NULL);
//}
//
//int main(void) {
//    lxb_status_t status;
//    lxb_dom_node_t *body;
//    lxb_html_document_t *document;
//    lxb_css_parser_t *parser;
//    lxb_selectors_t *selectors;
//    lxb_css_selector_list_t *list;
//
//    static const lxb_char_t html[] = "<table><tr>\n"
//    "<th headers=\"1.h.1.1\" id=\"press1.r.1.1.1\"><p class=\"sub2\">Food at home</p></th>\n"
//    "<td headers=\"1.h.1.2\"><span class=\"datavalue\">7.652</span></td>\n"
//    "<td headers=\"1.h.1.3 press1.h.2.3\"><span class=\"datavalue\">251.369</span></td>\n"
//    "<td headers=\"1.h.1.3 press1.h.2.4\"><span class=\"datavalue\">259.825</span></td>\n"
//    "<td headers=\"1.h.1.3 press1.h.2.5\"><span class=\"datavalue\">262.695</span></td>\n"
//    "<td headers=\"1.h.1.6 press1.h.2.6\"><span class=\"datavalue\">4.5</span></td>\n"
//    "<td headers=\"1.h.1.6 press1.h.2.7\"><span class=\"datavalue\">1.1</span></td>\n"
//    "<td headers=\"1.h.1.8 press1.h.2.8\"><span class=\"datavalue\">0.7</span></td>\n"
//    "<td headers=\"1.h.1.8 press1.h.2.9\"><span class=\"datavalue\">0.4</span></td>\n"
//    "<td headers=\"1.h.1.8 press1.h.2.10\"><span class=\"datavalue\">1.2</span></td>\n"
//    "</tr></table>";
//
//    static const lxb_char_t slctrs[] = "tr > td[headers = '1.h.1.8 press1.h.2.10'] > .datavalue";
//    /*
//     * OR
//     * static const lxb_char_t slctrs[] = "tr > td:last-child .datavalue";
//     * OR
//     * static const lxb_char_t slctrs[] = "tr td[headers *= 'press1.h.2.10'] span.datavalue";
//     * OR
//     * ...
//     */
//
//    document = lxb_html_document_create();
//    if (document == NULL) {
//        return EXIT_FAILURE;
//    }
//
//    status = lxb_html_document_parse(document, html, sizeof(html) - 1);
//    if (status != LXB_STATUS_OK) {
//        return EXIT_FAILURE;
//    }
//
//    /* Create CSS parser. */
//
//    parser = lxb_css_parser_create();
//    status = lxb_css_parser_init(parser, NULL, NULL, 128);
//    if (status != LXB_STATUS_OK) {
//        return EXIT_FAILURE;
//    }
//
//    /* Selectors. */
//
//    selectors = lxb_selectors_create();
//    status = lxb_selectors_init(selectors);
//    if (status != LXB_STATUS_OK) {
//        return EXIT_FAILURE;
//    }
//
//    list = lxb_css_selectors_parse(parser, slctrs, sizeof(slctrs) - 1);
//    if (parser->status != LXB_STATUS_OK) {
//        return EXIT_FAILURE;
//    }
//
//    /* Find DOM/HTML nodes by selectors. */
//
//    body = lxb_dom_interface_node(lxb_html_document_body_element(document));
//    if (body == NULL) {
//        return EXIT_FAILURE;
//    }
//
////    lxb_html_serialize_deep_cb(body, callback, NULL);
//
//    status = lxb_selectors_find(selectors, body, list, find_callback, NULL);
//    if (status != LXB_STATUS_OK) {
//        return EXIT_FAILURE;
//    }
//
//    printf("\n");
//
//    /* Destroy Selectors object. */
//    (void) lxb_selectors_destroy(selectors, true);
//
//    /* Destroy resources for CSS Parser. */
//    (void) lxb_css_parser_destroy(parser, true, true);
//
//    /* Destroy HTML Document. */
//    lxb_html_document_destroy(document);
//
//    return 0;
//}
