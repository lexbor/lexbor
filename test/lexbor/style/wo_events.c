/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/style/style.h>
#include <lexbor/html/parser.h>
#include <lexbor/html/serialize.h>
#include <lexbor/html/interfaces/selectedcontent_element.h>
#include <lexbor/html/interfaces/option_element.h>
#include <lexbor/dom/collection.h>
#include <unit/test.h>


/*
 * Test: Style mutations are not called when WO_EVENTS is set directly
 * on the document.  Inserting/removing elements must not trigger style
 * computation, so serialize_str should return an empty string.
 */
TEST_BEGIN(wo_events_document_direct)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_css_parser_t *parser;
    lxb_css_stylesheet_t *sst;
    lxb_html_element_t *div, *body;
    lxb_html_document_t *document;

    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t class_str = lexbor_str("class");
    static const lexbor_str_t target_str = lexbor_str("target");

    static const lexbor_str_t slctrs = lexbor_str("div.target {width: 100px}");

    /* Create document and init style system. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_parse(document,
                                     (lxb_char_t *) "<html><body></body></html>",
                                     25);
    test_eq(status, LXB_STATUS_OK);

    /* Attach stylesheet. */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    test_eq(status, LXB_STATUS_OK);

    sst = lxb_css_stylesheet_create(parser->memory);
    status = lxb_css_stylesheet_parse(sst, parser, slctrs.data, slctrs.length);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_document_stylesheet_attach(document, sst);
    test_eq(status, LXB_STATUS_OK);

    /* Set WO_EVENTS — from now on, no mutation callbacks should fire. */

    lxb_html_document_dom_opt_set(document, LXB_DOM_DOCUMENT_OPT_WO_EVENTS);

    /* Create element with matching class. */

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         class_str.data, class_str.length,
                                         target_str.data, target_str.length);
    test_ne(attr, NULL);

    /* Insert into body — inserted callback must NOT fire. */

    body = lxb_html_interface_element(document->body);
    test_ne(body, NULL);

    lxb_html_element_insert_child(body, div);

    /* Style must be empty — insertion step was skipped. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq(out.length, 0UL);

    /* Remove — removed callback must NOT fire, and must not crash. */

    lxb_dom_node_remove(lxb_dom_interface_node(div));

    /* Destroy resources. */

    lxb_dom_node_destroy(lxb_dom_interface_node(div));
    (void) lxb_css_stylesheet_destroy(sst, true);
    (void) lxb_css_parser_destroy(parser, true);
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

/*
 * Test: Parser propagates dom_opt to the created document.
 */
TEST_BEGIN(wo_events_parser)
{
    lxb_status_t status;
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;

    static const lxb_char_t html[] = "<html><head></head><body>"
                                     "<div>hello</div></body></html>";
    size_t html_len = sizeof(html) - 1;

    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);
    test_eq(status, LXB_STATUS_OK);

    lxb_html_parser_dom_opt_set(parser, LXB_DOM_DOCUMENT_OPT_WO_EVENTS);

    document = lxb_html_parse(parser, html, html_len);
    test_ne(document, NULL);

    /* Document must carry the option. */

    test_eq(lxb_html_document_dom_opt(document) & LXB_DOM_DOCUMENT_OPT_WO_EVENTS,
            LXB_DOM_DOCUMENT_OPT_WO_EVENTS);

    /* Tree must still be built correctly. */

    test_ne(document->body, NULL);

    lxb_html_document_destroy(document);
    lxb_html_parser_destroy(parser);
}
TEST_END

/*
 * Test: Attribute mutation callbacks are not fired with WO_EVENTS.
 * Setting a style attribute must not trigger inline style processing.
 */
TEST_BEGIN(wo_events_attr_no_callback)
{
    lxb_status_t status;
    lexbor_str_t out;
    lxb_dom_attr_t *attr;
    lxb_html_element_t *div;
    lxb_html_document_t *document;

    static const lexbor_str_t div_str = lexbor_str("div");
    static const lexbor_str_t style_name = lexbor_str("style");
    static const lexbor_str_t style_value = lexbor_str("color: red");

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_style_init(document);
    test_eq(status, LXB_STATUS_OK);

    /* Set WO_EVENTS before any DOM manipulation. */

    lxb_html_document_dom_opt_set(document, LXB_DOM_DOCUMENT_OPT_WO_EVENTS);

    div = lxb_html_document_create_element(document, div_str.data,
                                           div_str.length, NULL);
    test_ne(div, NULL);

    /* Set style attribute — attr_mutation->append must NOT fire. */

    attr = lxb_dom_element_set_attribute(lxb_dom_interface_element(div),
                                         style_name.data, style_name.length,
                                         style_value.data, style_value.length);
    test_ne(attr, NULL);

    /* The raw attribute value is stored, but inline style is not processed. */

    out.data = NULL;
    status = lxb_html_element_style_serialize_str(div, &out,
                                                  LXB_DOM_ELEMENT_STYLE_OPT_UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq(out.length, 0UL);

    /* Remove attribute — must not crash. */

    status = lxb_dom_element_attr_remove(lxb_dom_interface_element(div), attr);
    test_eq(status, LXB_STATUS_OK);

    lxb_dom_node_destroy(lxb_dom_interface_node(div));
    (void) lxb_style_destroy(document);
    (void) lxb_html_document_destroy(document);
}
TEST_END

/*
 * Test: Parser chunk API propagates dom_opt.
 */
TEST_BEGIN(wo_events_parser_chunk)
{
    lxb_status_t status;
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;

    static const lxb_char_t html[] = "<html><head></head><body>"
                                     "<p>chunk</p></body></html>";
    size_t html_len = sizeof(html) - 1;

    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);
    test_eq(status, LXB_STATUS_OK);

    lxb_html_parser_dom_opt_set(parser, LXB_DOM_DOCUMENT_OPT_WO_EVENTS);

    document = lxb_html_parse_chunk_begin(parser);
    test_ne(document, NULL);

    status = lxb_html_parse_chunk_process(parser, html, html_len);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_parse_chunk_end(parser);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_html_document_dom_opt(document) & LXB_DOM_DOCUMENT_OPT_WO_EVENTS,
            LXB_DOM_DOCUMENT_OPT_WO_EVENTS);

    test_ne(document->body, NULL);

    lxb_html_document_destroy(document);
    lxb_html_parser_destroy(parser);
}
TEST_END

/*
 * Helper: find first <selectedcontent> descendant of root.
 */
static lexbor_action_t
find_selectedcontent_cb(lxb_dom_node_t *node, void *ctx)
{
    if (node->local_name == LXB_TAG_SELECTEDCONTENT
        && node->ns == LXB_NS_HTML)
    {
        *(lxb_dom_node_t **) ctx = node;
        return LEXBOR_ACTION_STOP;
    }

    return LEXBOR_ACTION_OK;
}

/*
 * Test: <select>/<selectedcontent> with events enabled — selectedcontent
 * must be active (disabled == false) and have children cloned from the
 * selected option.
 *
 * Then the same HTML parsed with WO_EVENTS — selectedcontent must remain
 * disabled (default) with no children.
 */
TEST_BEGIN(wo_events_selectedcontent)
{
    lxb_status_t status;
    lxb_html_parser_t *parser;
    lxb_html_document_t *document;
    lxb_dom_node_t *sc_node;

    static const lxb_char_t html[] =
        "<html><head></head><body>"
        "<select>"
        "<button><selectedcontent></selectedcontent></button>"
        "<option>a</option>"
        "<option selected>b</option>"
        "</select>"
        "</body></html>";
    size_t html_len = sizeof(html) - 1;

    /* ---- Part A: events enabled (default) ---- */

    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);
    test_eq(status, LXB_STATUS_OK);

    document = lxb_html_parse(parser, html, html_len);
    test_ne(document, NULL);

    /* Find <selectedcontent>. */

    sc_node = NULL;
    lxb_dom_node_simple_walk(lxb_dom_interface_node(document),
                             find_selectedcontent_cb, &sc_node);
    test_ne(sc_node, NULL);

    /*
     * With events: attr_mutation->append fires for "selected" attribute,
     * setting option->selectedness = true.  When the option is popped from
     * the open elements stack, its content is cloned into <selectedcontent>.
     */

    test_ne(sc_node->first_child, NULL);

    lxb_html_document_destroy(document);
    lxb_html_parser_destroy(parser);

    /* ---- Part B: WO_EVENTS ---- */

    parser = lxb_html_parser_create();
    status = lxb_html_parser_init(parser);
    test_eq(status, LXB_STATUS_OK);

    lxb_html_parser_dom_opt_set(parser, LXB_DOM_DOCUMENT_OPT_WO_EVENTS);

    document = lxb_html_parse(parser, html, html_len);
    test_ne(document, NULL);

    /* Find <selectedcontent>. */

    sc_node = NULL;
    lxb_dom_node_simple_walk(lxb_dom_interface_node(document),
                             find_selectedcontent_cb, &sc_node);
    test_ne(sc_node, NULL);

    /*
     * With WO_EVENTS: attr_mutation->append is NOT called for "selected",
     * so option->selectedness stays false.  When popped from open elements,
     * lxb_html_option_maybe_clone_to_selectedcontent returns early because
     * selectedness == false.  Result: no children in <selectedcontent>.
     */

    test_eq(sc_node->first_child, NULL);

    lxb_html_document_destroy(document);
    lxb_html_parser_destroy(parser);
}
TEST_END


int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(wo_events_document_direct);
    TEST_ADD(wo_events_parser);
    TEST_ADD(wo_events_attr_no_callback);
    TEST_ADD(wo_events_parser_chunk);
    TEST_ADD(wo_events_selectedcontent);

    TEST_RUN("lexbor/style/wo_events");
    TEST_RELEASE();
}
