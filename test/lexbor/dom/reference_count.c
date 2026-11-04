/*
 * Copyright (C) 2026 Lexbor authors
 */

#include <unit/test.h>

#include <lexbor/dom/interfaces/attr.h>
#include <lexbor/dom/interfaces/cdata_section.h>
#include <lexbor/dom/interfaces/comment.h>
#include <lexbor/dom/interfaces/document_fragment.h>
#include <lexbor/dom/interfaces/document_type.h>
#include <lexbor/dom/interfaces/processing_instruction.h>
#include <lexbor/dom/interfaces/shadow_root.h>
#include <lexbor/html/html.h>
#include <lexbor/html/interfaces/template_element.h>


TEST_BEGIN(interface_lifetime)
{
    size_t i, remaining, allocations;
    lxb_dom_node_t *node;
    lxb_dom_document_t *doc;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);
    doc = lxb_dom_interface_document(document);
    allocations = doc->mraw->ref_count;

    lxb_dom_interface_t *interfaces[] = {
        lxb_dom_node_interface_create(doc),
        lxb_dom_attr_interface_create(doc),
        lxb_dom_character_data_interface_create(doc),
        lxb_dom_shadow_root_interface_create(doc),
        lxb_dom_element_interface_create(doc),
        lxb_dom_text_interface_create(doc),
        lxb_dom_cdata_section_interface_create(doc),
        lxb_dom_processing_instruction_interface_create(doc),
        lxb_dom_comment_interface_create(doc),
        lxb_dom_document_interface_create(doc),
        lxb_dom_document_type_interface_create(doc),
        lxb_dom_document_fragment_interface_create(doc)
    };

    for (i = 0; i < sizeof(interfaces) / sizeof(interfaces[0]); i++) {
        node = interfaces[i];
        test_ne(node, NULL);
        test_eq(node->ref_count, 1);
        test_eq(node->owner_document, doc);

        /* There is no node retain API; callers increment the public field. */
        node->ref_count += 2;
        for (remaining = 2; remaining > 0; remaining--) {
            test_eq(lxb_dom_interface_destroy(node), node);
            test_eq(node->ref_count, remaining);
        }

        test_eq(lxb_dom_interface_destroy(node), NULL);
    }

    test_eq(doc->mraw->ref_count, allocations);
    test_eq(doc->node.ref_count, 1);
    test_eq(lxb_dom_interface_destroy(NULL), NULL);
    test_eq(lxb_html_document_destroy(document), NULL);
}
TEST_END

TEST_BEGIN(character_data_lifetime)
{
    size_t i, allocations, text_allocations;
    lxb_dom_node_t *node;
    lxb_dom_character_data_t *data;
    lxb_dom_document_t *doc;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);
    doc = lxb_dom_interface_document(document);
    allocations = doc->mraw->ref_count;
    text_allocations = doc->text->ref_count;

    lxb_dom_interface_t *interfaces[] = {
        lxb_dom_character_data_interface_create(doc),
        lxb_dom_text_interface_create(doc),
        lxb_dom_cdata_section_interface_create(doc),
        lxb_dom_comment_interface_create(doc),
        lxb_dom_processing_instruction_interface_create(doc)
    };

    for (i = 0; i < sizeof(interfaces) / sizeof(interfaces[0]); i++) {
        data = interfaces[i];
        test_ne(data, NULL);
        node = lxb_dom_interface_node(data);
        test_ne(lexbor_str_init(&data->data, doc->text, 7), NULL);
        test_ne(lexbor_str_append(&data->data, doc->text,
                                  (const lxb_char_t *) "payload", 7), NULL);

        node->ref_count++;
        test_eq(lxb_dom_interface_destroy(node), node);
        test_eq(node->ref_count, 1);
        test_eq_str_n(data->data.data, data->data.length, "payload", 7);
        test_eq(doc->text->ref_count, text_allocations + 1);

        test_eq(lxb_dom_interface_destroy(node), NULL);
        test_eq(doc->text->ref_count, text_allocations);
    }

    test_eq(doc->mraw->ref_count, allocations);
    test_eq(lxb_html_document_destroy(document), NULL);
}
TEST_END

TEST_BEGIN(auxiliary_strings_lifetime)
{
    size_t allocations, text_allocations;
    lxb_dom_document_t *doc;
    lxb_dom_processing_instruction_t *pi;
    lxb_dom_document_type_t *doctype;
    lxb_dom_exception_code_t code;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);
    doc = lxb_dom_interface_document(document);
    allocations = doc->mraw->ref_count;
    text_allocations = doc->text->ref_count;

    pi = lxb_dom_document_create_processing_instruction(doc,
            (const lxb_char_t *) "target", 6,
            (const lxb_char_t *) "data", 4);
    test_ne(pi, NULL);
    pi->char_data.node.ref_count++;
    test_eq(lxb_dom_interface_destroy(pi), pi);
    test_eq_str_n(pi->target.data, pi->target.length, "target", 6);
    test_eq_str_n(pi->char_data.data.data, pi->char_data.data.length, "data", 4);
    test_eq(lxb_dom_interface_destroy(pi), NULL);
    test_eq(doc->text->ref_count, text_allocations);

    doctype = lxb_dom_document_type_create(doc,
                (const lxb_char_t *) "html", 4,
                (const lxb_char_t *) "public", 6,
                (const lxb_char_t *) "system", 6, &code);
    test_ne(doctype, NULL);
    test_eq(code, LXB_DOM_EXCEPTION_OK);
    doctype->node.ref_count++;
    test_eq(lxb_dom_interface_destroy(doctype), doctype);
    test_eq_str_n(doctype->public_id.data, doctype->public_id.length, "public", 6);
    test_eq_str_n(doctype->system_id.data, doctype->system_id.length, "system", 6);
    test_eq(lxb_dom_interface_destroy(doctype), NULL);

    test_eq(doc->mraw->ref_count, allocations);
    test_eq(doc->text->ref_count, text_allocations);
    test_eq(lxb_html_document_destroy(document), NULL);
}
TEST_END

TEST_BEGIN(element_attributes_lifetime)
{
    size_t allocations, text_allocations;
    lxb_dom_document_t *doc;
    lxb_dom_element_t *element;
    lxb_dom_attr_t *attr;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);
    doc = lxb_dom_interface_document(document);
    allocations = doc->mraw->ref_count;
    text_allocations = doc->text->ref_count;

    element = lxb_dom_element_interface_create(doc);
    test_ne(element, NULL);
    attr = lxb_dom_element_set_attribute(element,
                (const lxb_char_t *) "id", 2,
                (const lxb_char_t *) "value", 5);
    test_ne(attr, NULL);
    test_eq(attr->node.ref_count, 1);

    element->node.ref_count++;
    test_eq(lxb_dom_interface_destroy(element), element);
    test_eq(element->node.ref_count, 1);
    test_eq(element->first_attr, attr);
    test_eq(attr->owner, element);
    test_eq(attr->node.ref_count, 1);
    test_eq_str_n(attr->value->data, attr->value->length, "value", 5);

    /* An independently retained attribute outlives the element's release. */
    attr->node.ref_count++;
    test_eq(lxb_dom_interface_destroy(element), NULL);
    test_eq(attr->node.ref_count, 1);
    test_eq_str_n(attr->value->data, attr->value->length, "value", 5);
    test_eq(lxb_dom_interface_destroy(attr), NULL);

    test_eq(doc->mraw->ref_count, allocations);
    test_eq(doc->text->ref_count, text_allocations);
    test_eq(lxb_html_document_destroy(document), NULL);
}
TEST_END

TEST_BEGIN(clone_has_independent_count)
{
    lxb_dom_document_t *doc;
    lxb_dom_text_t *text, *clone;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    test_ne(document, NULL);
    doc = lxb_dom_interface_document(document);
    text = lxb_dom_document_create_text_node(doc,
                                            (const lxb_char_t *) "data", 4);
    test_ne(text, NULL);
    lxb_dom_interface_node(text)->ref_count += 2;

    clone = lxb_dom_interface_clone(NULL, text);
    test_ne(clone, NULL);
    test_ne(clone, text);
    test_eq(lxb_dom_interface_node(clone)->ref_count, 1);
    test_eq(lxb_dom_interface_node(text)->ref_count, 3);
    test_eq(lxb_dom_interface_destroy(clone), NULL);
    test_eq_str_n(text->char_data.data.data, text->char_data.data.length,
                  "data", 4);
    test_eq(lxb_dom_interface_destroy(text), text);
    test_eq(lxb_dom_interface_destroy(text), text);
    test_eq(lxb_dom_interface_destroy(text), NULL);
    test_eq(lxb_html_document_destroy(document), NULL);
}
TEST_END

TEST_BEGIN(document_lifetime)
{
    lxb_status_t status;
    lxb_dom_document_t *doc, *derived;

    doc = lxb_dom_document_create(NULL);
    test_ne(doc, NULL);
    status = lxb_dom_document_init(doc, NULL, lxb_dom_interface_create,
                lxb_dom_interface_clone, lxb_dom_interface_destroy,
                LXB_DOM_DOCUMENT_DTYPE_UNDEF, LXB_NS__UNDEF);
    test_eq(status, LXB_STATUS_OK);
    test_eq(doc->node.ref_count, 1);
    doc->node.ref_count++;
    test_eq(lxb_dom_document_destroy(doc), doc);
    test_eq(doc->node.ref_count, 1);

    derived = lxb_dom_document_interface_create(doc);
    test_ne(derived, NULL);
    test_eq(derived->node.ref_count, 1);
    derived->node.ref_count++;
    test_eq(lxb_dom_document_destroy(derived), derived);
    test_eq(derived->node.ref_count, 1);
    test_eq(lxb_dom_document_destroy(derived), NULL);
    test_eq(doc->node.ref_count, 1);
    test_eq(doc->mraw->ref_count, 0);
    test_eq(lxb_dom_document_destroy(doc), NULL);
    test_eq(lxb_dom_document_destroy(NULL), NULL);
}
TEST_END

TEST_BEGIN(html_document_parser_lifetime)
{
    lxb_status_t status;
    lxb_html_document_t *document;
    lxb_dom_document_t *doc;
    lxb_html_parser_t *parser;
    static const lxb_char_t html[] = "<p>data</p>";

    document = lxb_html_document_create();
    test_ne(document, NULL);
    status = lxb_html_document_parse(document, html, sizeof(html) - 1);
    test_eq(status, LXB_STATUS_OK);
    doc = lxb_dom_interface_document(document);
    parser = doc->parser;
    test_ne(parser, NULL);
    test_eq(parser->ref_count, 1);
    test_eq(lxb_html_parser_ref(parser), parser);

    doc->node.ref_count++;
    test_eq(lxb_html_document_destroy(document), document);
    test_eq(doc->node.ref_count, 1);
    test_eq(parser->ref_count, 2);
    test_eq(doc->parser, parser);
    test_ne(document->body, NULL);

    test_eq(lxb_html_document_destroy(document), NULL);
    test_eq(parser->ref_count, 1);
    test_eq(lxb_html_parser_unref(parser), NULL);
    test_eq(lxb_html_document_destroy(NULL), NULL);
}
TEST_END

TEST_BEGIN(template_content_lifetime)
{
    size_t allocations;
    lxb_html_document_t *document;
    lxb_dom_document_t *doc;
    lxb_html_template_element_t *element;
    lxb_dom_document_fragment_t *content;

    document = lxb_html_document_create();
    test_ne(document, NULL);
    doc = lxb_dom_interface_document(document);
    allocations = doc->mraw->ref_count;
    element = lxb_html_interface_create(document, LXB_TAG_TEMPLATE, LXB_NS_HTML);
    test_ne(element, NULL);
    test_eq(lxb_dom_interface_node(element)->ref_count, 1);
    content = element->content;
    test_ne(content, NULL);
    test_eq(content->node.ref_count, 1);

    lxb_dom_interface_node(element)->ref_count++;
    test_eq(lxb_html_interface_destroy(element), element);
    test_eq(element->content, content);
    test_eq(content->node.ref_count, 1);
    test_eq(doc->mraw->ref_count, allocations + 2);
    test_eq(lxb_html_interface_destroy(element), NULL);
    test_eq(doc->mraw->ref_count, allocations);
    test_eq(lxb_html_document_destroy(document), NULL);
}
TEST_END

int
main(int argc, const char *argv[])
{
    TEST_INIT();

    TEST_ADD(interface_lifetime);
    TEST_ADD(character_data_lifetime);
    TEST_ADD(auxiliary_strings_lifetime);
    TEST_ADD(element_attributes_lifetime);
    TEST_ADD(clone_has_independent_count);
    TEST_ADD(document_lifetime);
    TEST_ADD(html_document_parser_lifetime);
    TEST_ADD(template_content_lifetime);

    TEST_RUN("lexbor/dom/reference_count");
    TEST_RELEASE();
}
