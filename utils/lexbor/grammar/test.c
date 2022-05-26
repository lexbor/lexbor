/*
 * Copyright (C) 2022 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <math.h>

#include "lexbor/grammar/test.h"
#include "lexbor/grammar/tokenizer.h"
#include "lexbor/grammar/parser.h"
#include "lexbor/grammar/json.h"
#include "lexbor/dom/interfaces/element.h"
#include "lexbor/core/conv.h"


typedef struct {
    const lxb_grammar_node_t *node;

    size_t                   idx;
    bool                     used;

    union {
        lexbor_array_obj_t result;
        lexbor_str_t       str;
    } u;
}
utils_lxb_grammar_test_t;


static const lexbor_hash_search_t test_dupl_search = {
    .hash = lexbor_hash_make_id,
    .cmp = lexbor_str_data_ncmp
};

static const lexbor_hash_insert_t test_dupl_insert = {
    .hash = lexbor_hash_make_id,
    .cmp = lexbor_str_data_ncmp,
    .copy = lexbor_hash_copy
};


static lxb_status_t
utils_lxb_grammar_test_group(const lxb_grammar_node_t *root,
                             const lxb_grammar_node_t *declr,
                             lexbor_mraw_t *mraw, lexbor_hash_t *dupl,
                             lexbor_array_t *stack,
                             lexbor_array_obj_t *result);

static lxb_status_t
utils_lxb_grammar_test_defined(const lxb_grammar_node_t *node,
                               lexbor_mraw_t *mraw, lexbor_array_t *stack,
                               utils_lxb_grammar_test_t *test);

static lxb_status_t
utils_lxb_grammar_test_number(const lxb_grammar_node_t *node,
                              lexbor_mraw_t *mraw, lexbor_array_t *stack,
                              utils_lxb_grammar_test_t *test, bool percentage);

static lxb_status_t
utils_lxb_grammar_test_result_append(lexbor_mraw_t *mraw, const lxb_char_t *data,
                                     size_t length, lexbor_array_obj_t *result);

static lxb_status_t
utils_lxb_grammar_test_make(lexbor_array_obj_t *out, size_t idx,
                            const lxb_grammar_node_t *node,
                            const lexbor_str_t *origin, lexbor_mraw_t *mraw,
                            lexbor_hash_t *dupl, lexbor_array_obj_t *result);

static lxb_status_t
utils_lxb_grammar_test_and(lexbor_array_obj_t *out, size_t idx,
                           const lxb_grammar_node_t *node,
                           lexbor_mraw_t *mraw, lexbor_hash_t *dupl, bool or,
                           lexbor_array_obj_t *result);

static lxb_status_t
utils_lxb_grammar_test_save(lexbor_array_obj_t *out, size_t idx,
                            const lexbor_str_t *origin, lexbor_mraw_t *mraw,
                            lexbor_hash_t *dupl, bool or,
                            lexbor_array_obj_t *result);

static lxb_status_t
utils_lxb_grammar_test_multi(lexbor_array_obj_t *out, size_t idx,
                             const lexbor_str_t *origin,
                             const lexbor_str_t *cur,
                             const lxb_grammar_period_t *multi,
                             lexbor_mraw_t *mraw, lexbor_hash_t *dupl,
                             bool or, bool comma,
                             lexbor_array_obj_t *result);

static lxb_status_t
utils_lxb_grammar_test_one(lexbor_array_obj_t *out,
                           const lexbor_str_t *origin, lexbor_mraw_t *mraw,
                           lexbor_hash_t *dupl, lexbor_array_obj_t *result);

static const lxb_grammar_node_t *
utils_lxb_grammar_test_find_declaration(const lxb_grammar_node_t *root,
                                        const lxb_grammar_node_t *el);


static lxb_status_t
serializer_callback(const lxb_char_t *data, size_t length, void *ctx)
{
    printf("%.*s", (int) length, data);

    return LXB_STATUS_OK;
}

lxb_status_t
utils_lxb_grammar_test(const lxb_char_t *grammar, const size_t length,
                       lexbor_serialize_cb_f begin,
                       utils_lxb_grammar_test_cb_f cb,
                       lexbor_serialize_cb_f end, void *ctx)
{
    bool print_declr;
    lxb_status_t status;
    lxb_grammar_parser_t *parser;
    lxb_grammar_document_t *doc;
    lxb_grammar_tokenizer_t *tkz;
    lxb_grammar_node_t *root;
    lexbor_mraw_t *mraw;
    lexbor_str_t *str, name;
    lexbor_array_t *stack;
    lexbor_array_obj_t result;
    const lxb_tag_data_t *tag_data;
    const lxb_grammar_node_t *declr;
    const lxb_tag_data_t *tag_decl;
    lexbor_hash_t *hash;

    static const lexbor_str_t lxb_declr = lexbor_str("declarations");

    stack = lexbor_array_create();
    hash = lexbor_hash_create();
    mraw = lexbor_mraw_create();
    tkz = lxb_grammar_tokenizer_create();
    parser = lxb_grammar_parser_create();
    doc = NULL;

    /* Memrory init. */

    status = lexbor_array_init(stack, 128);
    if (status != LXB_STATUS_OK) {
        goto done;
    }

    status = lexbor_hash_init(hash, 1024, sizeof(lexbor_hash_entry_t));
    if (status != LXB_STATUS_OK) {
        goto done;
    }

    status = lexbor_mraw_init(mraw, 4096 * 12);
    if (status != LXB_STATUS_OK) {
        goto done;
    }

    (void) lexbor_str_init(&name, mraw, 1024);
    if (name.data == NULL) {
        status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        goto done;
    }

    /* Create tokenizer and process data. */

    status = lxb_grammar_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        goto done;
    }

    doc = lxb_grammar_tokenizer_process(tkz, grammar, length);
    if (doc == NULL) {
        status = LXB_STATUS_ERROR;
        goto done;
    }

    /* Parse tokens. */

    status = lxb_grammar_parser_init(parser);
    if (status != LXB_STATUS_OK) {
        goto done;
    }

    root = lxb_grammar_parser_process(parser, doc);
    if (root == NULL) {
        lxb_grammar_parser_print_last_error(parser);
        lxb_grammar_document_destroy(doc);

        status = LXB_STATUS_ERROR;
        goto done;
    }

    printf("Out: \n");

    status = lxb_grammar_node_serialize_deep(root, serializer_callback, NULL);
    if (status != LXB_STATUS_OK) {
        goto done;
    }

    printf("\nJSON AST: \n");
    lxb_grammar_json_ast(root, 0, serializer_callback, NULL);

    tag_decl = lxb_tag_data_by_name(parser->document->dom_document.tags,
                                    lxb_declr.data, lxb_declr.length);
    if (tag_decl == NULL) {
        status = LXB_STATUS_ERROR;
        goto done;
    }

    print_declr = false;

    declr = root->first_child;

    while (declr != NULL) {
        tag_data = lxb_tag_data_by_id(declr->u.node->owner_document->tags,
                                      declr->u.node->local_name);
        if (tag_data == NULL) {
            status = LXB_STATUS_ERROR;
            goto done;
        }

        status = lexbor_array_obj_init(&result, 16, sizeof(lexbor_str_t));
        if (status != LXB_STATUS_OK) {
            goto done;
        }

        status = utils_lxb_grammar_test_group(root, declr, mraw,
                                              hash, stack, &result);
        if (status != LXB_STATUS_OK) {
            goto done;
        }

        if (print_declr) {
            status = begin(lexbor_hash_entry_str(&tag_data->entry),
                           tag_data->entry.length, ctx);
            if (status != LXB_STATUS_OK) {
                goto done;
            }

            for (size_t idx = 0; idx < lexbor_array_obj_length(&result); idx++) {
                str = lexbor_array_obj_get(&result, idx);

                status = cb(lexbor_hash_entry_str(&tag_data->entry),
                            tag_data->entry.length, str->data, str->length,
                            (idx + 1) == lexbor_array_obj_length(&result),
                            false, ctx);
                if (status != LXB_STATUS_OK) {
                    goto done;
                }
            }

            status = end(lexbor_hash_entry_str(&tag_data->entry),
                         tag_data->entry.length, ctx);
            if (status != LXB_STATUS_OK) {
                goto done;
            }
        }

        (void) lexbor_array_obj_destroy(&result, false);
        lexbor_mraw_clean(mraw);

        if (!print_declr && tag_decl->tag_id == declr->u.node->local_name) {
            print_declr = true;
        }

        declr = declr->next;
    }

done:

    lxb_grammar_tokenizer_destroy(tkz, true);
    lxb_grammar_parser_destroy(parser, true);
    lxb_grammar_document_destroy(doc);
    lexbor_mraw_destroy(mraw, true);
    lexbor_hash_destroy(hash, true);
    lexbor_array_destroy(stack, true);

    return status;
}

static lxb_status_t
utils_lxb_grammar_test_group(const lxb_grammar_node_t *root,
                             const lxb_grammar_node_t *declr,
                             lexbor_mraw_t *mraw, lexbor_hash_t *dupl,
                             lexbor_array_t *stack, lexbor_array_obj_t *result)
{
    size_t idx, length;
    lxb_char_t *p;
    lxb_status_t status;
    lexbor_array_obj_t out;
    utils_lxb_grammar_test_t *test;
    const lxb_grammar_node_t *node, *found;

    lxb_char_t buf[4600];
    const size_t buf_len = sizeof(buf) - 1;

    node = declr->first_child;

    lexbor_array_obj_clean(result);

    status = lexbor_array_obj_init(&out, 16, sizeof(utils_lxb_grammar_test_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    idx = 0;

    while (node != NULL) {
        test = lexbor_array_obj_push(&out);
        if (test == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        test->node = node;
        test->idx = idx++;

        switch (node->type) {
            case LXB_GRAMMAR_NODE_GROUP:
                status = lexbor_array_obj_init(&test->u.result, 16,
                                               sizeof(lexbor_str_t));
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                status = utils_lxb_grammar_test_group(root, node, mraw, dupl,
                                                      stack, &test->u.result);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                break;

            case LXB_GRAMMAR_NODE_ELEMENT:
                status = lexbor_array_push(stack, (void *) node);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                status = utils_lxb_grammar_test_defined(node, mraw, stack, test);
                if (status == LXB_STATUS_OK) {
                    (void) lexbor_array_pop(stack);
                    break;
                }
                else if (status != LXB_STATUS_NEXT) {
                    return status;
                }

                found = utils_lxb_grammar_test_find_declaration(root, node);
                if (found == NULL) {
                    return LXB_STATUS_ERROR_NOT_EXISTS;
                }

                status = lexbor_array_obj_init(&test->u.result, 16,
                                               sizeof(lexbor_str_t));
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                status = utils_lxb_grammar_test_group(root, found, mraw, dupl,
                                                      stack, &test->u.result);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                (void) lexbor_array_pop(stack);

                break;

            case LXB_GRAMMAR_NODE_WHITESPACE:
                break;

            case LXB_GRAMMAR_NODE_DELIM:
            case LXB_GRAMMAR_NODE_UNQUOTED:
                (void) lexbor_str_init(&test->u.str, mraw, node->u.str.length);
                if (test->u.str.data == NULL) {
                    return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                }

                p = lexbor_str_append(&test->u.str, mraw, node->u.str.data,
                                      node->u.str.length);
                if (p == NULL) {
                    return LXB_STATUS_ERROR;
                }

                break;

            case LXB_GRAMMAR_NODE_NUMBER:
                length = lexbor_conv_float_to_data(node->u.num, buf, buf_len);

                (void) lexbor_str_init(&test->u.str, mraw, length);
                if (test->u.str.data == NULL) {
                    return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                }

                p = lexbor_str_append(&test->u.str, mraw, buf, length);
                if (p == NULL) {
                    return LXB_STATUS_ERROR;
                }

                break;

            default:
                break;
        }

        node = node->next;
    }

    status = utils_lxb_grammar_test_make(&out, 0, declr, NULL,
                                         mraw, dupl, result);

    lexbor_hash_clean(dupl);

    (void) lexbor_array_obj_destroy(&out, false);

    return status;
}

static lxb_status_t
utils_lxb_grammar_test_defined(const lxb_grammar_node_t *node,
                               lexbor_mraw_t *mraw, lexbor_array_t *stack,
                               utils_lxb_grammar_test_t *test)
{
    size_t length;
    const lxb_char_t *name;

    static const lexbor_str_t number = lexbor_str("number");
    static const lexbor_str_t percentage = lexbor_str("percentage");

    name = lxb_dom_element_qualified_name(lxb_dom_interface_element(node->u.node),
                                          &length);
    if (name == NULL) {
        return LXB_STATUS_NEXT;
    }

    if (number.length == length
        && lexbor_str_data_ncasecmp(number.data, name, length))
    {
        return utils_lxb_grammar_test_number(node, mraw, stack, test, false);
    }
    else if (percentage.length == length
             && lexbor_str_data_ncasecmp(percentage.data, name, length))
    {
        return utils_lxb_grammar_test_number(node, mraw, stack, test, true);
    }

    return LXB_STATUS_NEXT;
}

static lxb_status_t
utils_lxb_grammar_test_number(const lxb_grammar_node_t *root,
                              lexbor_mraw_t *mraw, lexbor_array_t *stack,
                              utils_lxb_grammar_test_t *test, bool percentage)
{
    size_t length, idx;
    bool dmin, dmax;
    double min, max;
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    const lxb_char_t *p;
    const lxb_grammar_node_t *node;

    lxb_char_t buf[4600];
    const size_t buf_len = sizeof(buf) - 1;

    static const lexbor_str_t str_min = lexbor_str("min");
    static const lexbor_str_t str_max = lexbor_str("max");

    status = lexbor_array_obj_init(&test->u.result, 16, sizeof(lexbor_str_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    min = -INFINITY;
    max = INFINITY;

    dmin = false;
    dmax = false;

    idx = lexbor_array_length(stack);

    while (idx != 0) {
        node = lexbor_array_get(stack, --idx);

        if (!dmin) {
            attr = lxb_dom_element_attr_by_name(lxb_dom_interface_element(node->u.node),
                                                str_min.data, str_min.length);
            if (attr != NULL) {
                p = lxb_dom_attr_value(attr, &length);
                if (p != NULL) {
                    min = lexbor_conv_data_to_double(&p, length);
                    dmin = true;
                }
            }
        }

        if (!dmax) {
            attr = lxb_dom_element_attr_by_name(lxb_dom_interface_element(node->u.node),
                                                str_max.data, str_max.length);
            if (attr != NULL) {
                p = lxb_dom_attr_value(attr, &length);
                if (p != NULL) {
                    max = lexbor_conv_data_to_double(&p, length);
                    dmax = true;
                }
            }
        }
    }

    if (min < -128000) {
        min = -128000;
    }

    if (max > 128000) {
        max = 128000;
    }

    length = lexbor_conv_float_to_data(min, buf, buf_len);

    if (percentage) {
        buf[length++] = '%';
    }

    status = utils_lxb_grammar_test_result_append(mraw, buf, length,
                                                  &test->u.result);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    length = lexbor_conv_float_to_data(max, buf, buf_len);

    if (percentage) {
        buf[length++] = '%';
    }

    status = utils_lxb_grammar_test_result_append(mraw, buf, length,
                                                  &test->u.result);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_result_append(lexbor_mraw_t *mraw, const lxb_char_t *data,
                                     size_t length, lexbor_array_obj_t *result)
{
    lexbor_str_t *str;

    str = lexbor_array_obj_push(result);
    if (str == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    (void) lexbor_str_init(str, mraw, length);
    if (str->data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    (void) lexbor_str_append(str, mraw, data, length);

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_make(lexbor_array_obj_t *out, size_t idx,
                            const lxb_grammar_node_t *declr,
                            const lexbor_str_t *origin, lexbor_mraw_t *mraw,
                            lexbor_hash_t *dupl, lexbor_array_obj_t *result)
{
    switch (declr->combinator) {
        case LXB_GRAMMAR_COMBINATOR_NORMAL:
            return utils_lxb_grammar_test_save(out, idx, origin, mraw,
                                               NULL, false, result);

        case LXB_GRAMMAR_COMBINATOR_AND:
            return utils_lxb_grammar_test_and(out, idx, declr->first_child,
                                              mraw, dupl, false, result);

        case LXB_GRAMMAR_COMBINATOR_OR:
            return utils_lxb_grammar_test_and(out, idx, declr->first_child,
                                              mraw, dupl, true, result);

        case LXB_GRAMMAR_COMBINATOR_ONE_OF:
            return utils_lxb_grammar_test_one(out, origin, mraw, dupl, result);

        default:
            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
    }
}

static lxb_status_t
utils_lxb_grammar_test_and(lexbor_array_obj_t *out, size_t idx,
                           const lxb_grammar_node_t *node,
                           lexbor_mraw_t *mraw, lexbor_hash_t *dupl, bool or,
                           lexbor_array_obj_t *result)
{
    lxb_status_t status;
    utils_lxb_grammar_test_t *test, *cur;

    if (idx == lexbor_array_obj_length(out)) {
        return utils_lxb_grammar_test_save(out, 0, NULL, mraw, dupl, or,
                                           result);
    }

    cur = lexbor_array_obj_get(out, idx);

    for (size_t i = 0; i < lexbor_array_obj_length(out); i++) {
        test = lexbor_array_obj_get(out, i);

        if (!test->used) {
            cur->idx = i;
            test->used = true;

            status = utils_lxb_grammar_test_and(out, idx + 1, NULL,
                                                mraw, dupl, or, result);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            test->used = false;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_save(lexbor_array_obj_t *out, size_t idx,
                            const lexbor_str_t *origin, lexbor_mraw_t *mraw,
                            lexbor_hash_t *dupl, bool or,
                            lexbor_array_obj_t *result)
{
    void *data;
    lxb_status_t status;
    lexbor_str_t *str;
    lexbor_array_obj_t *res;
    const lxb_grammar_period_t *multi;
    utils_lxb_grammar_test_t *test, *cur;

    static const lexbor_str_t lxb_empty = lexbor_str("");

    if (idx == lexbor_array_obj_length(out)) {
        if (origin == NULL) {
            if (or) {
                return LXB_STATUS_OK;
            }

            origin = &lxb_empty;
        }

        if (dupl != NULL) {
            data = lexbor_hash_search(dupl, &test_dupl_search,
                                      origin->data, origin->length);
            if (data != NULL) {
                return LXB_STATUS_OK;
            }

            data = lexbor_hash_insert(dupl, &test_dupl_insert,
                                      origin->data, origin->length);
            if (data == NULL) {
                return LXB_STATUS_ERROR;
            }
        }

        str = lexbor_array_obj_push(result);
        if (str == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        (void) lexbor_str_init(str, mraw, origin->length);
        if (str->data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        (void) lexbor_str_append(str, mraw, origin->data, origin->length);

        return LXB_STATUS_OK;
    }

    test = lexbor_array_obj_get(out, idx);
    cur = lexbor_array_obj_get(out, test->idx);
    multi = &cur->node->multiplier;

    if (cur->node->type == LXB_GRAMMAR_NODE_GROUP
        || cur->node->type == LXB_GRAMMAR_NODE_ELEMENT)
    {
        res = &cur->u.result;

        for (size_t t = 0; t < lexbor_array_obj_length(res); t++) {
            str = lexbor_array_obj_get(res, t);

            if (str->length == 0 && multi->start == 1 && multi->stop == 0) {
                continue;
            }

            status = utils_lxb_grammar_test_multi(out, idx, origin, str,
                                                  multi, mraw, dupl, or,
                                                  cur->node->is_comma, result);
            if (status != LXB_STATUS_OK) {
                return status;
            }
        }
    }
    else {
        status = utils_lxb_grammar_test_multi(out, idx, origin, &cur->u.str,
                                              multi, mraw, dupl, or,
                                              cur->node->is_comma, result);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    if (or || cur->node->multiplier.start == 0) {
        return utils_lxb_grammar_test_save(out, idx + 1, origin,
                                           mraw, dupl, or, result);
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_multi(lexbor_array_obj_t *out, size_t idx,
                             const lexbor_str_t *origin,
                             const lexbor_str_t *cur,
                             const lxb_grammar_period_t *multi,
                             lexbor_mraw_t *mraw, lexbor_hash_t *dupl,
                             bool or, bool comma,
                             lexbor_array_obj_t *result)
{
    long stop;
    lxb_char_t *p;
    lxb_status_t status;
    lexbor_str_t str;
    const lexbor_str_t *sep;
    utils_lxb_grammar_test_t *test;

    static const lexbor_str_t lxb_ws = lexbor_str(" ");
    static const lexbor_str_t lxb_comma = lexbor_str(", ");

    test = lexbor_array_obj_get(out, idx);
    test = lexbor_array_obj_get(out, test->idx);

    sep = (comma) ? &lxb_comma : &lxb_ws;

    (void) lexbor_str_init(&str, mraw, 128);
    if (str.data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    if (origin != NULL) {
        p = lexbor_str_append(&str, mraw, origin->data, origin->length);
        if (p == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        if (multi->start > 0 && !test->node->skip_ws) {
            p = lexbor_str_append(&str, mraw, lxb_ws.data, lxb_ws.length);
            if (p == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }
    }

    if (cur->length == 0) {
        return utils_lxb_grammar_test_save(out, idx + 1, &str,
                                           mraw, dupl, or, result);
    }

    for (size_t i = 1; i <= multi->start; i++) {
        p = lexbor_str_append(&str, mraw, cur->data, cur->length);
        if (p == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        if (i < multi->start) {
            p = lexbor_str_append(&str, mraw, sep->data, sep->length);
            if (p == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }
    }

    if (multi->start != 0) {
        status = utils_lxb_grammar_test_save(out, idx + 1, &str,
                                             mraw, dupl, or, result);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    stop = multi->stop;

    if (stop == -1) {
        stop = 20;
    }
    else if (stop > 20) {
        stop = 20;
    }

    for (size_t i = multi->start; i < stop; i++) {
        if (str.length != 0) {
            p = lexbor_str_append(&str, mraw, sep->data, sep->length);
            if (p == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }

        p = lexbor_str_append(&str, mraw, cur->data, cur->length);
        if (p == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        status = utils_lxb_grammar_test_save(out, idx + 1, &str,
                                             mraw, dupl, or, result);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_one(lexbor_array_obj_t *out,
                           const lexbor_str_t *origin, lexbor_mraw_t *mraw,
                           lexbor_hash_t *dupl, lexbor_array_obj_t *result)
{
    size_t length;
    lxb_status_t status;
    lexbor_str_t *str;
    lexbor_array_obj_t *res;
    utils_lxb_grammar_test_t *cur;
    const lxb_grammar_period_t *multi;

    length = lexbor_array_obj_length(out);

    for (size_t i = 0; i < length; i++) {
        out->length = length;

        cur = lexbor_array_obj_get(out, i);
        multi = &cur->node->multiplier;

        if (cur->node->type == LXB_GRAMMAR_NODE_GROUP
            || cur->node->type == LXB_GRAMMAR_NODE_ELEMENT)
        {
            res = &cur->u.result;

            for (size_t t = 0; t < lexbor_array_obj_length(res); t++) {
                str = lexbor_array_obj_get(res, t);

                if (str->length == 0 && multi->start == 1 && multi->stop == 0) {
                    continue;
                }

                out->length = 1;

                status = utils_lxb_grammar_test_multi(out, 0, origin, str,
                                                      multi, mraw, dupl, false,
                                                      cur->node->is_comma,
                                                      result);
                if (status != LXB_STATUS_OK) {
                    goto failed;
                }
            }
        }
        else {
            out->length = 1;

            status = utils_lxb_grammar_test_multi(out, 0, origin, &cur->u.str,
                                                  multi, mraw, dupl, false,
                                                  cur->node->is_comma, result);
            if (status != LXB_STATUS_OK) {
                goto failed;
            }
        }

        if (cur->node->multiplier.start == 0) {
            status = utils_lxb_grammar_test_save(out, 0, origin, mraw,
                                                 dupl, false, result);
            if (status != LXB_STATUS_OK) {
                goto failed;
            }
        }
    }

    out->length = length;

    return LXB_STATUS_OK;

failed:

    out->length = length;

    return status;
}

static const lxb_grammar_node_t *
utils_lxb_grammar_test_find_declaration(const lxb_grammar_node_t *root,
                                        const lxb_grammar_node_t *el)
{
    const lxb_grammar_node_t *node = root->first_child;

    while (node) {
        if (node->u.node->local_name == el->u.node->local_name) {
            return node;
        }

        node = node->next;
    }

    return NULL;
}
