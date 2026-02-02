/*
 * Copyright (C) 2024 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "base.h"


/* Helper function to calculate line and column number from a position in the HTML */
static void
calculate_line_column(const lxb_char_t *html, const lxb_char_t *pos,
                     size_t *line, size_t *column)
{
    const lxb_char_t *p = html;
    *line = 1;
    *column = 1;

    while (p < pos) {
        if (*p == '\n') {
            (*line)++;
            *column = 1;
        }
        else {
            (*column)++;
        }
        p++;
    }
}

/* Error name strings for tokenizer errors */
static const char *
tokenizer_error_name(lxb_html_tokenizer_error_id_t id)
{
    switch (id) {
        case LXB_HTML_TOKENIZER_ERROR_ABCLOFEMCO:
            return "abrupt-closing-of-empty-comment";
        case LXB_HTML_TOKENIZER_ERROR_ABDOPUID:
            return "abrupt-doctype-public-identifier";
        case LXB_HTML_TOKENIZER_ERROR_ABDOSYID:
            return "abrupt-doctype-system-identifier";
        case LXB_HTML_TOKENIZER_ERROR_ABOFDIINNUCHRE:
            return "absence-of-digits-in-numeric-character-reference";
        case LXB_HTML_TOKENIZER_ERROR_CDINHTCO:
            return "cdata-in-html-content";
        case LXB_HTML_TOKENIZER_ERROR_CHREOUUNRA:
            return "character-reference-outside-unicode-range";
        case LXB_HTML_TOKENIZER_ERROR_COCHININST:
            return "control-character-in-input-stream";
        case LXB_HTML_TOKENIZER_ERROR_COCHRE:
            return "control-character-reference";
        case LXB_HTML_TOKENIZER_ERROR_ENTAWIAT:
            return "end-tag-with-attributes";
        case LXB_HTML_TOKENIZER_ERROR_DUAT:
            return "duplicate-attribute";
        case LXB_HTML_TOKENIZER_ERROR_ENTAWITRSO:
            return "end-tag-with-trailing-solidus";
        case LXB_HTML_TOKENIZER_ERROR_EOBETANA:
            return "eof-before-tag-name";
        case LXB_HTML_TOKENIZER_ERROR_EOINCD:
            return "eof-in-cdata";
        case LXB_HTML_TOKENIZER_ERROR_EOINCO:
            return "eof-in-comment";
        case LXB_HTML_TOKENIZER_ERROR_EOINDO:
            return "eof-in-doctype";
        case LXB_HTML_TOKENIZER_ERROR_EOINSCHTCOLITE:
            return "eof-in-script-html-comment-like-text";
        case LXB_HTML_TOKENIZER_ERROR_EOINTA:
            return "eof-in-tag";
        case LXB_HTML_TOKENIZER_ERROR_INCLCO:
            return "incorrectly-closed-comment";
        case LXB_HTML_TOKENIZER_ERROR_INOPCO:
            return "incorrectly-opened-comment";
        case LXB_HTML_TOKENIZER_ERROR_INCHSEAFDONA:
            return "invalid-character-sequence-after-doctype-name";
        case LXB_HTML_TOKENIZER_ERROR_INFICHOFTANA:
            return "invalid-first-character-of-tag-name";
        case LXB_HTML_TOKENIZER_ERROR_MIATVA:
            return "missing-attribute-value";
        case LXB_HTML_TOKENIZER_ERROR_MIDONA:
            return "missing-doctype-name";
        case LXB_HTML_TOKENIZER_ERROR_MIDOPUID:
            return "missing-doctype-public-identifier";
        case LXB_HTML_TOKENIZER_ERROR_MIDOSYID:
            return "missing-doctype-system-identifier";
        case LXB_HTML_TOKENIZER_ERROR_MIENTANA:
            return "missing-end-tag-name";
        case LXB_HTML_TOKENIZER_ERROR_MIQUBEDOPUID:
            return "missing-quote-before-doctype-public-identifier";
        case LXB_HTML_TOKENIZER_ERROR_MIQUBEDOSYID:
            return "missing-quote-before-doctype-system-identifier";
        case LXB_HTML_TOKENIZER_ERROR_MISEAFCHRE:
            return "missing-semicolon-after-character-reference";
        case LXB_HTML_TOKENIZER_ERROR_MIWHAFDOPUKE:
            return "missing-whitespace-after-doctype-public-keyword";
        case LXB_HTML_TOKENIZER_ERROR_MIWHAFDOSYKE:
            return "missing-whitespace-after-doctype-system-keyword";
        case LXB_HTML_TOKENIZER_ERROR_MIWHBEDONA:
            return "missing-whitespace-before-doctype-name";
        case LXB_HTML_TOKENIZER_ERROR_MIWHBEAT:
            return "missing-whitespace-between-attributes";
        case LXB_HTML_TOKENIZER_ERROR_MIWHBEDOPUANSYID:
            return "missing-whitespace-between-doctype-public-and-system-identifiers";
        case LXB_HTML_TOKENIZER_ERROR_NECO:
            return "nested-comment";
        case LXB_HTML_TOKENIZER_ERROR_NOCHRE:
            return "noncharacter-character-reference";
        case LXB_HTML_TOKENIZER_ERROR_NOININST:
            return "noncharacter-in-input-stream";
        case LXB_HTML_TOKENIZER_ERROR_NOVOHTELSTTAWITRSO:
            return "non-void-html-element-start-tag-with-trailing-solidus";
        case LXB_HTML_TOKENIZER_ERROR_NUCHRE:
            return "null-character-reference";
        case LXB_HTML_TOKENIZER_ERROR_SUCHRE:
            return "surrogate-character-reference";
        case LXB_HTML_TOKENIZER_ERROR_SUININST:
            return "surrogate-in-input-stream";
        case LXB_HTML_TOKENIZER_ERROR_UNCHAFDOSYID:
            return "unexpected-character-after-doctype-system-identifier";
        case LXB_HTML_TOKENIZER_ERROR_UNCHINATNA:
            return "unexpected-character-in-attribute-name";
        case LXB_HTML_TOKENIZER_ERROR_UNCHINUNATVA:
            return "unexpected-character-in-unquoted-attribute-value";
        case LXB_HTML_TOKENIZER_ERROR_UNEQSIBEATNA:
            return "unexpected-equals-sign-before-attribute-name";
        case LXB_HTML_TOKENIZER_ERROR_UNNUCH:
            return "unexpected-null-character";
        case LXB_HTML_TOKENIZER_ERROR_UNQUMAINOFTANA:
            return "unexpected-question-mark-instead-of-tag-name";
        case LXB_HTML_TOKENIZER_ERROR_UNSOINTA:
            return "unexpected-solidus-in-tag";
        case LXB_HTML_TOKENIZER_ERROR_UNNACHRE:
            return "unknown-named-character-reference";
        default:
            return "unknown-error";
    }
}

/* Error name strings for tree builder errors */
static const char *
tree_error_name(lxb_html_tree_error_id_t id)
{
    switch (id) {
        case LXB_HTML_RULES_ERROR_UNTO:
            return "unexpected-token";
        case LXB_HTML_RULES_ERROR_UNCLTO:
            return "unexpected-closed-token";
        case LXB_HTML_RULES_ERROR_NUCH:
            return "null-character";
        case LXB_HTML_RULES_ERROR_UNCHTO:
            return "unexpected-character-token";
        case LXB_HTML_RULES_ERROR_UNTOININMO:
            return "unexpected-token-in-initial-mode";
        case LXB_HTML_RULES_ERROR_BADOTOININMO:
            return "bad-doctype-token-in-initial-mode";
        case LXB_HTML_RULES_ERROR_DOTOINBEHTMO:
            return "doctype-token-in-before-html-mode";
        case LXB_HTML_RULES_ERROR_UNCLTOINBEHTMO:
            return "unexpected-closed-token-in-before-html-mode";
        case LXB_HTML_RULES_ERROR_DOTOINBEHEMO:
            return "doctype-token-in-before-head-mode";
        case LXB_HTML_RULES_ERROR_UNCLTOINBEHEMO:
            return "unexpected-closed-token-in-before-head-mode";
        case LXB_HTML_RULES_ERROR_DOTOINHEMO:
            return "doctype-token-in-head-mode";
        case LXB_HTML_RULES_ERROR_NOVOHTELSTTAWITRSO:
            return "non-void-html-element-start-tag-with-trailing-solidus";
        case LXB_HTML_RULES_ERROR_HETOINHEMO:
            return "head-token-in-head-mode";
        case LXB_HTML_RULES_ERROR_UNCLTOINHEMO:
            return "unexpected-closed-token-in-head-mode";
        case LXB_HTML_RULES_ERROR_TECLTOWIOPINHEMO:
            return "template-closed-token-without-opening-in-head-mode";
        case LXB_HTML_RULES_ERROR_TEELISNOCUINHEMO:
            return "template-element-is-not-current-in-head-mode";
        case LXB_HTML_RULES_ERROR_DOTOINHENOMO:
            return "doctype-token-in-head-noscript-mode";
        case LXB_HTML_RULES_ERROR_DOTOAFHEMO:
            return "doctype-token-after-head-mode";
        case LXB_HTML_RULES_ERROR_HETOAFHEMO:
            return "head-token-after-head-mode";
        case LXB_HTML_RULES_ERROR_DOTOINBOMO:
            return "doctype-token-in-body-mode";
        case LXB_HTML_RULES_ERROR_BAENOPELISWR:
            return "bad-ending-open-elements-is-wrong";
        case LXB_HTML_RULES_ERROR_OPELISWR:
            return "open-elements-is-wrong";
        case LXB_HTML_RULES_ERROR_UNELINOPELST:
            return "unexpected-element-in-open-elements-stack";
        case LXB_HTML_RULES_ERROR_MIELINOPELST:
            return "missing-element-in-open-elements-stack";
        case LXB_HTML_RULES_ERROR_NOBOELINSC:
            return "no-body-element-in-scope";
        case LXB_HTML_RULES_ERROR_MIELINSC:
            return "missing-element-in-scope";
        case LXB_HTML_RULES_ERROR_UNELINSC:
            return "unexpected-element-in-scope";
        case LXB_HTML_RULES_ERROR_UNELINACFOST:
            return "unexpected-element-in-active-formatting-stack";
        case LXB_HTML_RULES_ERROR_UNENOFFI:
            return "unexpected-end-of-file";
        case LXB_HTML_RULES_ERROR_CHINTATE:
            return "characters-in-table-text";
        case LXB_HTML_RULES_ERROR_DOTOINTAMO:
            return "doctype-token-in-table-mode";
        case LXB_HTML_RULES_ERROR_DOTOINSEMO:
            return "doctype-token-in-select-mode";
        case LXB_HTML_RULES_ERROR_DOTOAFBOMO:
            return "doctype-token-after-body-mode";
        case LXB_HTML_RULES_ERROR_DOTOINFRMO:
            return "doctype-token-in-frameset-mode";
        case LXB_HTML_RULES_ERROR_DOTOAFFRMO:
            return "doctype-token-after-frameset-mode";
        case LXB_HTML_RULES_ERROR_DOTOFOCOMO:
            return "doctype-token-foreign-content-mode";
        case LXB_HTML_RULES_ERROR_SEINSC:
            return "select-in-scope";
        case LXB_HTML_RULES_ERROR_FRPASEINCOPAIN:
            return "fragment-parsing-select-in-context-parse-input";
        case LXB_HTML_RULES_ERROR_FRPASEINCOPASE:
            return "fragment-parsing-select-in-context-parse-select";
        case LXB_HTML_RULES_ERROR_HRPASEOPOPINSC:
            return "hr-parsing-select-option-optgroup-in-scope";
        case LXB_HTML_RULES_ERROR_OPPAOPINSC:
            return "option-parsing-option-in-scope";
        case LXB_HTML_RULES_ERROR_OPPAOPOPINSC:
            return "optgroup-parsing-option-optgroup-in-scope";
        default:
            return "unknown-error";
    }
}


int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;

    /* HTML with intentional errors for demonstration */
    static const lxb_char_t html[] = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>Parse Errors Example</title>\n"
        "</head>\n"
        "<body>\n"
        "    <div><p>Unclosed paragraph\n"
        "    <div id=>Empty attribute value</div>\n"
        "    <img src='test.png'/>\n"
        "    <!--Unclosed comment\n"
        "    <div attr1=val1 attr2>Multiple attribute issues</div>\n"
        "</body>\n"
        "</html>";
    size_t html_len = sizeof(html) - 1;

    /* Print the HTML being parsed */
    printf("Parsing HTML:\n");
    printf("=============\n");
    printf("%s\n\n", (const char *) html);

    /* Create parser */
    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);

    if (status != LXB_STATUS_OK) {
        FAILED("Failed to create HTML parser");
    }

    /* Parse HTML */
    document = lxb_html_parse(parser, html, html_len);
    if (document == NULL) {
        FAILED("Failed to parse HTML");
    }

    /* Access tokenizer errors */
    printf("Tokenizer Errors:\n");
    printf("=================\n");
    
    lexbor_array_obj_t *tkz_errors = parser->tkz->parse_errors;
    if (tkz_errors != NULL && lexbor_array_obj_length(tkz_errors) > 0) {
        for (size_t i = 0; i < lexbor_array_obj_length(tkz_errors); i++) {
            lxb_html_tokenizer_error_t *error;
            error = (lxb_html_tokenizer_error_t *) lexbor_array_obj_get(tkz_errors, i);
            
            if (error != NULL) {
                size_t line, column;
                calculate_line_column(html, error->pos, &line, &column);
                
                printf("Error %zu: %s\n", i + 1, tokenizer_error_name(error->id));
                printf("  Location: Line %zu, Column %zu\n", line, column);
            }
        }
    }
    else {
        printf("No tokenizer errors found.\n");
    }

    /* Access tree builder errors */
    printf("\nTree Builder Errors:\n");
    printf("====================\n");
    
    lexbor_array_obj_t *tree_errors = parser->tree->parse_errors;
    if (tree_errors != NULL && lexbor_array_obj_length(tree_errors) > 0) {
        for (size_t i = 0; i < lexbor_array_obj_length(tree_errors); i++) {
            lxb_html_tree_error_t *error;
            error = (lxb_html_tree_error_t *) lexbor_array_obj_get(tree_errors, i);
            
            if (error != NULL) {
                size_t line, column;
                /* Use the begin position for the error location */
                calculate_line_column(html, error->begin, &line, &column);
                
                printf("Error %zu: %s\n", i + 1, tree_error_name(error->id));
                printf("  Location: Line %zu, Column %zu\n", line, column);
            }
        }
    }
    else {
        printf("No tree builder errors found.\n");
    }

    /* Cleanup */
    lxb_html_document_destroy(document);
    lxb_html_parser_destroy(parser);

    return 0;
}
