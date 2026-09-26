/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>

#include <unit/test.h>


const lxb_char_t html[] = "<div x=abc><span>darkness</span><xx>xXx</xx></div>";

const size_t html_length = sizeof(html) - 1;


TEST_BEGIN(single_clone)
{
    lxb_status_t status;
    lxb_dom_attr_t *attr_cloned, *attr_orig;
    lxb_dom_node_t *node, *clone;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* Parse. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Get <div x="abc">. */

    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document),
                                          collection,
                                          (lxb_char_t *) "div", 3);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 1);

    /* Clone <div x="abc">. */

    node = lxb_dom_collection_node(collection, 0);

    clone = lxb_dom_node_clone(node, 0);
    test_ne(clone, NULL);

    /* Insert cloned <div x="abc">. */

    lxb_dom_node_insert_after(node, clone);

    test_eq(node->next, clone);
    test_ne(node->next, node);

    /* Find all div tag. */

    collection->array.length = 0;

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document),
                                          collection,
                                          (lxb_char_t *) "div", 3);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 2);

    /* Get cloned attribute. */

    attr_cloned = lxb_dom_element_attr_by_name(lxb_dom_interface_element(clone),
                                               (lxb_char_t *) "x", 1);
    test_ne(attr_cloned, NULL);
    test_ne(attr_cloned->value, NULL);
    test_eq_str(attr_cloned->value->data, "abc");

    attr_orig = lxb_dom_element_attr_by_name(lxb_dom_interface_element(node),
                                             (lxb_char_t *) "x", 1);
    test_ne(attr_orig, NULL);
    test_ne(attr_orig->value, NULL);
    test_eq_str(attr_orig->value->data, "abc");

    test_ne(attr_cloned, attr_orig);
    test_ne(attr_cloned->value, attr_orig->value);
    test_ne(attr_cloned->value->data, attr_orig->value->data);

    test_eq_str(attr_cloned->value->data, attr_orig->value->data);
    test_eq(attr_cloned->value->length, attr_orig->value->length);

    /* Destroy. */

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(deep_clone)
{
    lxb_status_t status;
    lxb_dom_node_t *node, *clone, *span, *cloned_cpan;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* Parse. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Get <div x="abc">. */

    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document),
                                          collection,
                                          (lxb_char_t *) "div", 3);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 1);

    /* Clone <div x="abc">. */

    node = lxb_dom_collection_node(collection, 0);

    clone = lxb_dom_node_clone(node, 1);
    test_ne(clone, NULL);

    /* Insert cloned <div x="abc">. */

    lxb_dom_node_insert_after(node, clone);

    test_eq(node->next, clone);
    test_ne(node->next, node);

    /* Find all span tag. */

    collection->array.length = 0;

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document),
                                          collection,
                                          (lxb_char_t *) "span", 4);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 2);

    /* Check parents. */

    span = lxb_dom_collection_node(collection, 0);
    cloned_cpan = lxb_dom_collection_node(collection, 1);

    test_ne(span, NULL);
    test_ne(cloned_cpan, NULL);
    test_ne(span, cloned_cpan);

    test_eq(span->parent, node);
    test_eq(cloned_cpan->parent, clone);

    /* Destroy. */

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(text_clone)
{
    lxb_status_t status;
    lxb_dom_node_t *node, *clone;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    /* Parse. */

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Get <span>. */

    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document),
                                          collection,
                                          (lxb_char_t *) "span", 4);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 1);

    /* Clone #text. */

    node = lxb_dom_collection_node(collection, 0);

    clone = lxb_dom_node_clone(node->first_child, 0);
    test_ne(clone, NULL);

    /* Insert cloned #text. */

    lxb_dom_node_insert_after(node, clone);

    test_eq(node->next, clone);
    test_ne(node->next, node->first_child);

    /* Destroy. */

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(import_from)
{
    lxb_status_t status;
    lxb_dom_node_t *node_one, *node_two, *clone;
    lxb_html_document_t *document_one, *document_two;
    lxb_dom_collection_t *collection;

    /* Parse. */

    document_one = lxb_html_document_create();
    test_ne(document_one, NULL);

    status = lxb_html_document_parse(document_one, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    document_two = lxb_html_document_create();
    test_ne(document_two, NULL);

    status = lxb_html_document_parse(document_two, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Collection one. */

    collection = lxb_dom_collection_make(&document_one->dom_document, 16);
    test_ne(collection, NULL);

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document_one),
                                          collection,
                                          (lxb_char_t *) "div", 3);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 1);

    node_one = lxb_dom_collection_node(collection, 0);

    /* Collection two. */

    collection->array.length = 0;

    status = lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document_two),
                                          collection,
                                          (lxb_char_t *) "div", 3);
    test_eq(status, LXB_STATUS_OK);

    test_eq(lxb_dom_collection_length(collection), 1);

    node_two = lxb_dom_collection_node(collection, 0);

    /* Check. */

    test_ne(node_one, node_two);

    /* Clone. */

    clone = lxb_dom_document_import_node(&document_one->dom_document,
                                         node_two, 1);
    test_ne(clone, NULL);

    test_eq(clone->owner_document, &document_one->dom_document);

    /* Destroy. */

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document_one);
    lxb_html_document_destroy(document_two);
}
TEST_END


TEST_BEGIN(full_clone)
{
    lxb_status_t status;
    lexbor_str_t str_one, str_two;
    lxb_dom_node_t *node_one, *node_two, *clone;
    lxb_html_document_t *document_one, *document_two;

    /* Parse. */

    document_one = lxb_html_document_create();
    test_ne(document_one, NULL);

    status = lxb_html_document_parse(document_one, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    document_two = lxb_html_document_create();
    test_ne(document_two, NULL);

    status = lxb_html_document_parse(document_two, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Root. */

    node_one = lxb_dom_document_root(lxb_dom_interface_document(document_one));
    node_two = lxb_dom_document_root(lxb_dom_interface_document(document_two));

    /* Check. */

    test_ne(node_one, node_two);

    /* Clone. */

    clone = lxb_dom_document_import_node(lxb_dom_interface_document(document_one),
                                         node_two, 1);
    test_ne(clone, NULL);

    test_eq(clone->owner_document, &document_one->dom_document);

    lexbor_str_clean_all(&str_one);
    lexbor_str_clean_all(&str_two);

    status = lxb_html_serialize_pretty_tree_str(node_one, 0, 0, &str_one);
    test_eq(status, LXB_STATUS_OK);

    status = lxb_html_serialize_pretty_tree_str(clone, 0, 0, &str_two);
    test_eq(status, LXB_STATUS_OK);

    test_eq_str_n(str_one.data, str_one.length, str_two.data, str_two.length);

    /* Destroy. */

    lxb_html_document_destroy(document_one);
    lxb_html_document_destroy(document_two);
}
TEST_END

TEST_BEGIN(specialized_interface_clone)
{
    size_t length, object_size;
    const lxb_char_t *name;
    lexbor_str_t *value;
    lxb_dom_attr_t *attr;
    lxb_dom_node_t *node, *copy;
    lxb_dom_document_t *src, *dst;
    lxb_html_document_t *source, *target;
    const lxb_char_t text[] = "copied value";
    const lxb_char_t attr_name[] = "AuditName";
    const lxb_dom_node_type_t types[] = {
        LXB_DOM_NODE_TYPE_ATTRIBUTE,
        LXB_DOM_NODE_TYPE_CDATA_SECTION,
        LXB_DOM_NODE_TYPE_CHARACTER_DATA
    };
    const lxb_dom_interface_clone_f clone_funcs[] = {
        lxb_dom_interface_clone, lxb_html_interface_clone
    };
    const lxb_dom_interface_destroy_f destroy_funcs[] = {
        lxb_dom_interface_destroy, lxb_html_interface_destroy
    };

    for (size_t i = 0; i < sizeof(clone_funcs) / sizeof(clone_funcs[0]); i++) {
        for (size_t j = 0; j < sizeof(types) / sizeof(types[0]); j++) {
            source = lxb_html_document_create();
            target = lxb_html_document_create();
            test_ne(source, NULL);
            test_ne(target, NULL);
            src = lxb_dom_interface_document(source);
            dst = lxb_dom_interface_document(target);

            if (types[j] == LXB_DOM_NODE_TYPE_ATTRIBUTE) {
                attr = lxb_dom_attr_interface_create(src);
                test_ne(attr, NULL);
                test_eq(lxb_dom_attr_set_name(attr, attr_name,
                                              sizeof(attr_name) - 1, false),
                        LXB_STATUS_OK);
                test_eq(lxb_dom_attr_set_value(attr, text, sizeof(text) - 1),
                        LXB_STATUS_OK);
                node = lxb_dom_interface_node(attr);
                object_size = sizeof(lxb_dom_attr_t);
            }
            else {
                if (types[j] == LXB_DOM_NODE_TYPE_CDATA_SECTION) {
                    node = lxb_dom_interface_node(
                                  lxb_dom_cdata_section_interface_create(src));
                    object_size = sizeof(lxb_dom_cdata_section_t);
                }
                else {
                    node = lxb_dom_interface_node(
                                  lxb_dom_character_data_interface_create(src));
                    object_size = sizeof(lxb_dom_character_data_t);
                }
                test_ne(node, NULL);
                value = &lxb_dom_interface_character_data(node)->data;
                test_ne(lexbor_str_init_append(value, src->text, text,
                                               sizeof(text) - 1), NULL);
            }

            copy = clone_funcs[i](dst, node);
            test_ne(copy, NULL);
            test_eq(copy->type, types[j]);
            test_eq(copy->owner_document, dst);
            test_eq(copy->parent, NULL);
            test_eq(lexbor_mraw_data_size(copy) >= object_size, true);
            lxb_html_document_destroy(source);

            if (types[j] == LXB_DOM_NODE_TYPE_ATTRIBUTE) {
                attr = lxb_dom_interface_attr(copy);
                test_eq(attr->owner, NULL);
                name = lxb_dom_attr_qualified_name(attr, &length);
                test_eq_str_n(name, length, attr_name, sizeof(attr_name) - 1);

                /* local_name is remapped by lxb_dom_node_interface_copy() */
                name = lxb_dom_attr_local_name(attr, &length);
                test_eq_str_n(name, length, (const lxb_char_t *) "auditname",
                              sizeof("auditname") - 1);

                value = attr->value;
            }
            else {
                value = &lxb_dom_interface_character_data(copy)->data;
            }
            test_ne(value, NULL);
            test_eq_str_n(value->data, value->length, text, sizeof(text) - 1);
            test_eq(value->data[value->length], 0);

            test_eq(destroy_funcs[i](copy), NULL);
            test_eq_size(lexbor_mraw_reference_count(dst->mraw), 0UL);
            test_eq_size(lexbor_mraw_reference_count(dst->text), 0UL);
            lxb_html_document_destroy(target);
        }
    }
}
TEST_END

TEST_BEGIN(clone_null_destination)
{
    size_t object_size, nodes, texts;
    lxb_char_t *value;
    lxb_dom_node_t *node, *copy;
    lxb_dom_attr_t *attr;
    lxb_dom_document_t *doc;
    lxb_html_document_t *document;
    const lxb_char_t text[] = "copied value";
    const lxb_dom_node_type_t types[] = {
        LXB_DOM_NODE_TYPE_ATTRIBUTE,
        LXB_DOM_NODE_TYPE_CDATA_SECTION,
        LXB_DOM_NODE_TYPE_CHARACTER_DATA
    };
    const lxb_dom_interface_clone_f clone_funcs[] = {
        lxb_dom_interface_clone, lxb_html_interface_clone
    };
    const lxb_dom_interface_destroy_f destroy_funcs[] = {
        lxb_dom_interface_destroy, lxb_html_interface_destroy
    };

    /* A NULL destination means the node's own document. */
    for (size_t i = 0; i < sizeof(clone_funcs) / sizeof(clone_funcs[0]); i++) {
        for (size_t j = 0; j < sizeof(types) / sizeof(types[0]); j++) {
            document = lxb_html_document_create();
            test_ne(document, NULL);
            doc = lxb_dom_interface_document(document);

            if (types[j] == LXB_DOM_NODE_TYPE_ATTRIBUTE) {
                attr = lxb_dom_attr_interface_create(doc);
                test_ne(attr, NULL);
                test_eq(lxb_dom_attr_set_name(attr,
                                              (const lxb_char_t *) "AuditName",
                                              sizeof("AuditName") - 1, false),
                        LXB_STATUS_OK);
                test_eq(lxb_dom_attr_set_value(attr, text, sizeof(text) - 1),
                        LXB_STATUS_OK);
                node = lxb_dom_interface_node(attr);
                object_size = sizeof(lxb_dom_attr_t);
            }
            else if (types[j] == LXB_DOM_NODE_TYPE_CDATA_SECTION) {
                node = lxb_dom_interface_node(
                              lxb_dom_cdata_section_interface_create(doc));
                test_ne(node, NULL);
                object_size = sizeof(lxb_dom_cdata_section_t);
            }
            else {
                node = lxb_dom_interface_node(
                              lxb_dom_character_data_interface_create(doc));
                test_ne(node, NULL);
                object_size = sizeof(lxb_dom_character_data_t);
            }

            if (types[j] != LXB_DOM_NODE_TYPE_ATTRIBUTE) {
                test_ne(lexbor_str_init_append(
                            &lxb_dom_interface_character_data(node)->data,
                            doc->text, text, sizeof(text) - 1), NULL);
            }

            nodes = lexbor_mraw_reference_count(doc->mraw);
            texts = lexbor_mraw_reference_count(doc->text);

            copy = clone_funcs[i](NULL, node);
            test_ne(copy, NULL);
            test_eq(copy->type, types[j]);
            test_eq(copy->owner_document, doc);
            test_eq(copy->parent, NULL);
            test_eq(lexbor_mraw_data_size(copy) >= object_size, true);

            if (types[j] == LXB_DOM_NODE_TYPE_ATTRIBUTE) {
                value = lxb_dom_interface_attr(copy)->value->data;
            }
            else {
                value = lxb_dom_interface_character_data(copy)->data.data;
            }
            test_eq_str_n(value, sizeof(text) - 1, text, sizeof(text) - 1);

            test_eq(destroy_funcs[i](copy), NULL);
            test_eq_size(lexbor_mraw_reference_count(doc->mraw), nodes);
            test_eq_size(lexbor_mraw_reference_count(doc->text), texts);

            lxb_html_document_destroy(document);
        }
    }
}
TEST_END

TEST_BEGIN(shadow_root_dispatch)
{
    size_t nodes;
    lxb_dom_node_t *copy, *host;
    lxb_dom_document_t *doc;
    lxb_dom_shadow_root_t *shadow, *copy_root;
    lxb_html_document_t *document;
    const lxb_dom_interface_clone_f clone_funcs[] = {
        lxb_dom_interface_clone, lxb_html_interface_clone
    };
    const lxb_dom_interface_destroy_f destroy_funcs[] = {
        lxb_dom_interface_destroy, lxb_html_interface_destroy
    };

    for (size_t i = 0; i < sizeof(clone_funcs) / sizeof(clone_funcs[0]); i++) {
        for (size_t j = 0; j < sizeof(destroy_funcs) / sizeof(destroy_funcs[0]); j++) {
            document = lxb_html_document_create();
            test_ne(document, NULL);
            doc = lxb_dom_interface_document(document);

            host = lxb_dom_interface_node(
                       lxb_dom_document_create_element(doc,
                                       (const lxb_char_t *) "div", 3, NULL));
            test_ne(host, NULL);

            shadow = lxb_dom_shadow_root_interface_create(doc);
            test_ne(shadow, NULL);

            shadow->mode = LXB_DOM_SHADOW_ROOT_MODE_CLOSED;
            shadow->host = lxb_dom_interface_element(host);
            shadow->document_fragment.host = lxb_dom_interface_element(host);

            nodes = lexbor_mraw_reference_count(doc->mraw);

            copy = clone_funcs[i](doc, lxb_dom_interface_node(shadow));
            test_ne(copy, NULL);
            test_eq(copy->type, LXB_DOM_NODE_TYPE_SHADOW_ROOT);
            test_eq(copy->owner_document, doc);
            test_eq(copy->parent, NULL);
            test_eq(lexbor_mraw_data_size(copy) >= sizeof(lxb_dom_shadow_root_t),
                    true);

            copy_root = lxb_dom_interface_shadow_root(copy);
            test_eq(copy_root->mode, LXB_DOM_SHADOW_ROOT_MODE_CLOSED);
            test_eq(copy_root->host, NULL);
            test_eq(copy_root->document_fragment.host, NULL);

            test_eq(destroy_funcs[i](copy), NULL);
            test_eq_size(lexbor_mraw_reference_count(doc->mraw), nodes);

            test_eq(destroy_funcs[j](lxb_dom_interface_node(shadow)), NULL);
            test_eq_size(lexbor_mraw_reference_count(doc->mraw), nodes - 1);

            lxb_html_document_destroy(document);
        }
    }
}
TEST_END

TEST_BEGIN(fragment_clone)
{
    lxb_dom_node_t *copy;
    lxb_dom_text_t *text;
    lxb_dom_document_t *src, *dst;
    lxb_html_document_t *source, *target;
    lxb_html_template_element_t *template;
    const lxb_char_t contents[] = "fragment";
    const lxb_dom_interface_clone_f clone_funcs[] = {
        lxb_dom_interface_clone, lxb_html_interface_clone
    };

    for (size_t i = 0; i < sizeof(clone_funcs) / sizeof(clone_funcs[0]); i++) {
        for (size_t deep = 0; deep < 2; deep++) {
            source = lxb_html_document_create();
            target = lxb_html_document_create();
            test_ne(source, NULL);
            test_ne(target, NULL);
            src = lxb_dom_interface_document(source);
            dst = lxb_dom_interface_document(target);
            dst->clone_interface = clone_funcs[i];

            template = lxb_html_interface_template(
                lxb_html_document_create_element(source,
                                         (const lxb_char_t *) "template", 8,
                                         NULL));
            test_ne(template, NULL);
            test_ne(template->content, NULL);
            test_ne(template->content->host, NULL);
            text = lxb_dom_document_create_text_node(src, contents,
                                                      sizeof(contents) - 1);
            test_ne(text, NULL);
            lxb_dom_node_insert_child_wo_events(&template->content->node,
                                                 &text->char_data.node);

            copy = lxb_dom_document_import_node(dst, &template->content->node,
                                                 deep != 0);
            test_ne(copy, NULL);
            test_eq(copy->type, LXB_DOM_NODE_TYPE_DOCUMENT_FRAGMENT);
            test_eq(copy->owner_document, dst);
            test_eq(lexbor_mraw_data_size(copy)
                    >= sizeof(lxb_dom_document_fragment_t), true);
            test_eq(lxb_dom_interface_document_fragment(copy)->host, NULL);
            lxb_html_document_destroy(source);

            if (deep) {
                test_ne(copy->first_child, NULL);
                test_eq(copy->first_child, copy->last_child);
                test_eq(copy->first_child->owner_document, dst);
                text = lxb_dom_interface_text(copy->first_child);
                test_eq_str_n(text->char_data.data.data,
                              text->char_data.data.length,
                              contents, sizeof(contents) - 1);
            }
            else {
                test_eq(copy->first_child, NULL);
                test_eq(copy->last_child, NULL);
            }
            lxb_dom_node_destroy_deep(copy);
            test_eq_size(lexbor_mraw_reference_count(dst->mraw), 0UL);
            test_eq_size(lexbor_mraw_reference_count(dst->text), 0UL);
            lxb_html_document_destroy(target);
        }
    }
}
TEST_END

TEST_BEGIN(clone_empty_character_data)
{
    size_t object_size, nodes, texts;
    lxb_dom_node_t *node, *copy;
    lxb_dom_document_t *doc;
    lxb_html_document_t *document;
    const lxb_dom_node_type_t types[] = {
        LXB_DOM_NODE_TYPE_TEXT,
        LXB_DOM_NODE_TYPE_CDATA_SECTION,
        LXB_DOM_NODE_TYPE_COMMENT,
        LXB_DOM_NODE_TYPE_CHARACTER_DATA
    };
    const lxb_dom_interface_clone_f clone_funcs[] = {
        lxb_dom_interface_clone, lxb_html_interface_clone
    };
    const lxb_dom_interface_destroy_f destroy_funcs[] = {
        lxb_dom_interface_destroy, lxb_html_interface_destroy
    };

    /* Factory-created empty nodes have no character data allocation. */
    for (size_t i = 0; i < sizeof(clone_funcs) / sizeof(clone_funcs[0]); i++) {
        for (size_t j = 0; j < sizeof(types) / sizeof(types[0]); j++) {
            document = lxb_html_document_create();
            test_ne(document, NULL);
            doc = lxb_dom_interface_document(document);

            switch (types[j]) {
                case LXB_DOM_NODE_TYPE_TEXT:
                    node = lxb_dom_interface_node(
                                  lxb_dom_text_interface_create(doc));
                    object_size = sizeof(lxb_dom_text_t);
                    break;

                case LXB_DOM_NODE_TYPE_CDATA_SECTION:
                    node = lxb_dom_interface_node(
                                  lxb_dom_cdata_section_interface_create(doc));
                    object_size = sizeof(lxb_dom_cdata_section_t);
                    break;

                case LXB_DOM_NODE_TYPE_COMMENT:
                    node = lxb_dom_interface_node(
                                  lxb_dom_comment_interface_create(doc));
                    object_size = sizeof(lxb_dom_comment_t);
                    break;

                default:
                    node = lxb_dom_interface_node(
                                  lxb_dom_character_data_interface_create(doc));
                    object_size = sizeof(lxb_dom_character_data_t);
                    break;
            }

            test_ne(node, NULL);
            test_eq(lxb_dom_interface_character_data(node)->data.data, NULL);

            nodes = lexbor_mraw_reference_count(doc->mraw);
            texts = lexbor_mraw_reference_count(doc->text);

            copy = clone_funcs[i](doc, node);
            test_ne(copy, NULL);
            test_eq(copy->type, types[j]);
            test_eq(copy->owner_document, doc);
            test_eq(copy->parent, NULL);
            test_eq(lexbor_mraw_data_size(copy) >= object_size, true);
            test_eq(lxb_dom_interface_character_data(copy)->data.data, NULL);
            test_eq_size(lxb_dom_interface_character_data(copy)->data.length, 0UL);

            test_eq(destroy_funcs[i](copy), NULL);
            test_eq_size(lexbor_mraw_reference_count(doc->mraw), nodes);
            test_eq_size(lexbor_mraw_reference_count(doc->text), texts);

            lxb_html_document_destroy(document);
        }
    }
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(single_clone);
    TEST_ADD(deep_clone);
    TEST_ADD(text_clone);
    TEST_ADD(import_from);
    TEST_ADD(full_clone);
    TEST_ADD(specialized_interface_clone);
    TEST_ADD(fragment_clone);
    TEST_ADD(clone_null_destination);
    TEST_ADD(clone_empty_character_data);
    TEST_ADD(shadow_root_dispatch);

    TEST_RUN("lexbor/html/clone");
    TEST_RELEASE();
}
