/*
 * Copyright (C) 2022-2023 Alexander Borisov
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


typedef struct utils_lxb_grammar_entry utils_lxb_grammar_entry_t;
typedef struct utils_lxb_grammar_node utils_lxb_grammar_node_t;

struct utils_lxb_grammar_node {
    utils_lxb_grammar_entry_t *entry;
    size_t                    idx;

    utils_lxb_grammar_node_t  *first_child;
    utils_lxb_grammar_node_t  *last_child;
    utils_lxb_grammar_node_t  *parent;

    utils_lxb_grammar_node_t  *next;
    utils_lxb_grammar_node_t  *prev;

    bool                      last;
};

typedef struct {
    lexbor_mraw_t            *mraw;
    size_t                   count;
    size_t                   limit;

    lexbor_array_t           result;
    lexbor_array_t           sorted;
    size_t                   length;

    lexbor_hash_t            dupls;

    utils_lxb_grammar_node_t root;
    utils_lxb_grammar_node_t calculated;

    bool                     without_sort;
}
utils_lxb_grammar_tree_t;

struct utils_lxb_grammar_entry {
    const lxb_grammar_node_t *node;

    size_t                   idx;
    bool                     used;

    union {
        utils_lxb_grammar_tree_t *tree;
        lexbor_str_t             str;
    } u;
};

typedef struct {
    uint64_t *id;
    size_t   len;
}
test_lexbor_grammar_ids_t;

static const lexbor_hash_search_t test_dupl_search = {
    .hash = lexbor_hash_make_id,
    .cmp = lexbor_str_data_ncmp
};

static const lexbor_hash_insert_t test_dupl_insert = {
    .hash = lexbor_hash_make_id,
    .cmp = lexbor_str_data_ncmp,
    .copy = lexbor_hash_copy
};

static const lexbor_str_t lxb_ws = lexbor_str(" ");
static const lexbor_str_t lxb_comma = lexbor_str(", ");
static const lexbor_str_t lxb_wo_ws = lexbor_str("");
static const lexbor_str_t lxb_wo_ws_comma = lexbor_str(",");

static const size_t test_max_repeat = 20;


static utils_lxb_grammar_tree_t *
utils_lxb_grammar_test_group(const lxb_grammar_node_t *root,
                             const lxb_grammar_node_t *declr,
                             lexbor_mraw_t *mraw, lexbor_array_t *stack,
                             size_t limit);

static lxb_status_t
utils_lxb_grammar_test_defined(const lxb_grammar_node_t *node,
                               lexbor_mraw_t *mraw, lexbor_array_t *stack,
                               utils_lxb_grammar_entry_t *test);

static lxb_status_t
utils_lxb_grammar_test_number(const lxb_grammar_node_t *node,
                              lexbor_array_t *stack,
                              utils_lxb_grammar_entry_t *test, bool percentage);

static lxb_status_t
utils_lxb_grammar_test_hex(const lxb_grammar_node_t *root,
                           lexbor_array_t *stack,
                           utils_lxb_grammar_entry_t *test);

static utils_lxb_grammar_tree_t *
utils_lxb_grammar_test_make(lexbor_array_obj_t *out,
                            const lxb_grammar_node_t *node, size_t limit,
                            bool without_sort);

static lxb_status_t
utils_lxb_grammar_test_and(lexbor_array_obj_t *out, size_t idx,
                           utils_lxb_grammar_tree_t *tree);

static lxb_status_t
utils_lxb_grammar_test_make_child(lexbor_array_obj_t *out,
                                  utils_lxb_grammar_tree_t *tree);

static lxb_status_t
utils_lxb_grammar_test_make_next(lexbor_array_obj_t *out,
                                 utils_lxb_grammar_tree_t *tree);

static lxb_status_t
utils_lxb_grammar_test_mix(utils_lxb_grammar_tree_t *tree,
                           utils_lxb_grammar_node_t *parent,
                           utils_lxb_grammar_node_t *node);

static lxb_status_t
utils_lxb_grammar_test_period(utils_lxb_grammar_tree_t *tree,
                              utils_lxb_grammar_node_t *parent,
                              utils_lxb_grammar_node_t *node);

static lxb_status_t
utils_lxb_grammar_test_compile(utils_lxb_grammar_tree_t *tree);

static lxb_status_t
utils_lxb_grammar_test_result(utils_lxb_grammar_tree_t *tree,
                              utils_lxb_grammar_node_t *node,
                              lexbor_array_t *indxs, size_t pos);

static lxb_status_t
utils_lxb_grammar_test_find(utils_lxb_grammar_tree_t *tree,
                            lexbor_array_t *indxs);

static lxb_status_t
utils_lxb_grammar_test_concat_ids(utils_lxb_grammar_tree_t *tree,
                                  utils_lxb_grammar_node_t **nodes,
                                  utils_lxb_grammar_node_t **sorted,
                                  size_t length, size_t *out_length);

static lxb_status_t
utils_lxb_grammar_test_wosort(utils_lxb_grammar_tree_t *tree,
                              utils_lxb_grammar_node_t **nodes,
                              test_lexbor_grammar_ids_t *ids,
                              size_t length, size_t *out_length);

static lxb_status_t
utils_lxb_grammar_test_sort(utils_lxb_grammar_tree_t *tree,
                            utils_lxb_grammar_node_t **nodes,
                            utils_lxb_grammar_node_t **sorted,
                            test_lexbor_grammar_ids_t *ids,
                            size_t length, size_t *out_length);

static lxb_status_t
utils_lxb_grammar_append_wodupl(utils_lxb_grammar_tree_t *tree,
                                lexbor_array_t *result, lexbor_array_t *sorted,
                                lexbor_str_t *str, lexbor_str_t *srd,
                                size_t length, size_t *out_length);

static lexbor_str_t *
utils_lxb_grammar_test_make_text(utils_lxb_grammar_tree_t *tree,
                                 utils_lxb_grammar_node_t **nodes,
                                 uint64_t *ids, size_t length,
                                 bool is_sorted, size_t *out_length);

static lxb_status_t
utils_lxb_grammar_test_concat_str(utils_lxb_grammar_tree_t *tree,
                                  utils_lxb_grammar_node_t *node,
                                  const lexbor_str_t *pstr,
                                  const lexbor_str_t *str, lexbor_str_t *result,
                                  const lexbor_str_t *sp, size_t count);

static lexbor_str_t *
utils_lxb_grammar_test_repeat(utils_lxb_grammar_tree_t *tree,
                              utils_lxb_grammar_node_t *node,
                              size_t *total, const lexbor_str_t *pstr,
                              const lexbor_str_t *str, lexbor_str_t *result,
                              const lexbor_str_t *sp, size_t min, size_t max);

static const lxb_grammar_node_t *
utils_lxb_grammar_test_find_declaration(const lxb_grammar_node_t *root,
                                        const lxb_grammar_node_t *el);


static utils_lxb_grammar_tree_t *
utils_lxb_grammar_tree_create(bool with_dupls)
{
    lxb_status_t status;
    lexbor_mraw_t *mraw;
    utils_lxb_grammar_tree_t *tree;

    mraw = lexbor_mraw_create();
    status = lexbor_mraw_init(mraw, 4096 * 16);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    tree = lexbor_mraw_calloc(mraw, sizeof(utils_lxb_grammar_tree_t));
    if (tree == NULL) {
        return NULL;
    }

    tree->mraw = mraw;

    status = lexbor_array_init(&tree->result, 4096 * 1000);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    status = lexbor_array_init(&tree->sorted, 4096 * 1000);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    if (!with_dupls) {
        return tree;
    }

    status = lexbor_hash_init(&tree->dupls, 4096 * 16,
                              sizeof(lexbor_hash_entry_t));
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    return tree;
}

lxb_inline utils_lxb_grammar_node_t *
utils_lxb_grammar_node_create(utils_lxb_grammar_tree_t *tree,
                              utils_lxb_grammar_entry_t *entry, size_t idx)
{
    utils_lxb_grammar_node_t *node;

    node = lexbor_mraw_calloc(tree->mraw, sizeof(utils_lxb_grammar_node_t));
    if (node == NULL) {
        return NULL;
    }

    node->entry = entry;
    node->idx = idx;

    return node;
}

lxb_inline void
utils_lxb_grammar_node_append_child(utils_lxb_grammar_node_t *root,
                                    utils_lxb_grammar_node_t *node)
{
    if (root->first_child != NULL) {
        root->last_child->next = node;
    }
    else {
        root->first_child = node;
    }

    node->parent = root;
    node->next = NULL;
    node->prev = root->last_child;

    root->last_child = node;
}

lxb_inline void
utils_lxb_grammar_node_insert_after(utils_lxb_grammar_node_t *root,
                                    utils_lxb_grammar_node_t *node)
{
    if (root->next != NULL) {
        root->next->prev = node;
    }

    node->next = root->next;
    node->prev = root;
    node->parent = root->parent;

    root->next = node;
}

lxb_inline lexbor_str_t *
utils_lxb_grammar_create_str(utils_lxb_grammar_tree_t *tree,
                             const lxb_char_t *data, size_t length)
{
    lexbor_str_t *str;

    str = lexbor_mraw_calloc(tree->mraw, sizeof(lexbor_str_t));
    if (str == NULL) {
        return NULL;
    }

    (void) lexbor_str_init(str, tree->mraw, length);
    if (str->data == NULL) {
        return NULL;
    }

    (void) lexbor_str_append(str, tree->mraw, data, length);

    return str;
}

static lxb_status_t
utils_lxb_grammar_result_append(utils_lxb_grammar_tree_t *tree,
                                const lxb_char_t *data, size_t length)
{
    lexbor_str_t *str;

    str = utils_lxb_grammar_create_str(tree, data, length);
    if (str == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return lexbor_array_push(&tree->result, str);
}

static lxb_status_t
utils_lxb_grammar_sorted_append(utils_lxb_grammar_tree_t *tree,
                                const lxb_char_t *data, size_t length)
{
    lexbor_str_t *str;

    str = utils_lxb_grammar_create_str(tree, data, length);
    if (str == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    return lexbor_array_push(&tree->sorted, str);
}

static lxb_status_t
utils_lxb_grammar_tree_append(utils_lxb_grammar_tree_t *tree,
                              const lxb_char_t *data, size_t length)
{
    lxb_status_t status;

    status = utils_lxb_grammar_result_append(tree, data, length);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = utils_lxb_grammar_sorted_append(tree, data, length);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    tree->length += 1;

    return LXB_STATUS_OK;
}


static lxb_status_t
serializer_callback(const lxb_char_t *data, size_t length, void *ctx)
{
    printf("%.*s", (int) length, data);

    return LXB_STATUS_OK;
}

lxb_status_t
utils_lxb_grammar_append_global(lxb_grammar_parser_t *parser,
                                lxb_grammar_node_t *to,
                                lxb_grammar_node_t *global)
{
    lxb_grammar_node_t *grp, *node;

    if (global == NULL) {
        return LXB_STATUS_OK;
    }

    if (to->combinator == LXB_GRAMMAR_COMBINATOR_ONE_OF) {
        lxb_grammar_node_insert_before(to->first_child, global);
        return LXB_STATUS_OK;
    }

    grp = lxb_grammar_node_create(parser, NULL, LXB_GRAMMAR_NODE_GROUP);
    if (grp == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    grp->combinator = to->combinator;
    grp->first_child = to->first_child;
    grp->last_child = to->last_child;

    node = grp->first_child;

    while (node != NULL) {
        node->parent = grp;
        node = node->next;
    }

    to->combinator = LXB_GRAMMAR_COMBINATOR_ONE_OF;
    to->first_child = NULL;
    to->last_child = NULL;

    lxb_grammar_node_insert_child(to, global);
    lxb_grammar_node_insert_child(to, grp);

    return LXB_STATUS_OK;
}

lxb_status_t
utils_lxb_grammar_test(const lxb_char_t *grammar, const size_t length,
                       lexbor_serialize_cb_f begin,
                       utils_lxb_grammar_test_cb_f cb,
                       lexbor_serialize_cb_f end, void *ctx)
{
    lxb_status_t status;
    lxb_grammar_parser_t *parser;
    lxb_grammar_document_t *doc;
    lxb_grammar_tokenizer_t *tkz;
    lxb_grammar_node_t *root;
    lexbor_mraw_t *mraw;
    lexbor_str_t **str, **str_order, name;
    lexbor_array_t *stack;
    lxb_grammar_node_t *node, *declr, *global, gelement;
    const lxb_tag_data_t *tag_data;
    const lxb_tag_data_t *tag_decl, *tag_global;
    utils_lxb_grammar_tree_t *tree;

    static const lexbor_str_t lxb_declr = lexbor_str("declarations");
    static const lexbor_str_t lxb_global = lexbor_str("global");

    stack = lexbor_array_create();
    mraw = lexbor_mraw_create();
    tkz = lxb_grammar_tokenizer_create();
    parser = lxb_grammar_parser_create();
    doc = NULL;

    /* Memrory init. */

    status = lexbor_array_init(stack, 128);
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

    tag_global = lxb_tag_data_by_name(parser->document->dom_document.tags,
                                      lxb_global.data, lxb_global.length);

    node = root->first_child;
    declr = NULL;
    global = NULL;

    while (node != NULL) {
        if (tag_global != NULL
            && tag_global->tag_id == node->u.node->local_name)
        {
            global = node;
        }

        if (tag_decl != NULL
            && tag_decl->tag_id == node->u.node->local_name)
        {
            declr = node;
        }

        node = node->next;
    }

    declr = (declr == NULL) ? root : declr->next;

    if (global != NULL) {
        memset(&gelement, 0x0, sizeof(lxb_grammar_node_t));

        gelement.type = LXB_GRAMMAR_NODE_ELEMENT;
        gelement.document = parser->document;
        gelement.multiplier.start = 1;
        gelement.multiplier.stop = 1;
        gelement.u.node = global->u.node;

        global = &gelement;
    }

    while (declr != NULL) {
        tag_data = lxb_tag_data_by_id(declr->u.node->local_name);
        if (tag_data == NULL) {
            status = LXB_STATUS_ERROR;
            goto done;
        }

        status = utils_lxb_grammar_append_global(parser, declr, global);
        if (status != LXB_STATUS_OK) {
            goto done;
        }

        tree = utils_lxb_grammar_test_group(root, declr, mraw, stack, 0);
        if (tree == NULL) {
            goto done;
        }

        if (global != NULL) {
            lxb_grammar_node_remove(declr->first_child);
        }

        status = begin(lexbor_hash_entry_str(&tag_data->entry),
                       tag_data->entry.length, ctx);
        if (status != LXB_STATUS_OK) {
            goto done;
        }

        str = (lexbor_str_t **) tree->result.list;
        str_order = (lexbor_str_t **) tree->sorted.list;

        for (size_t i = 0; i < tree->length; i++) {
            status = cb(lexbor_hash_entry_str(&tag_data->entry),
                        tag_data->entry.length,
                        str[i]->data, str[i]->length,
                        str_order[i]->data, str_order[i]->length,
                        (i + 1) == tree->length, false, ctx);

            if (status != LXB_STATUS_OK) {
                lexbor_mraw_destroy(tree->mraw, true);
                goto done;
            }
        }

        lexbor_mraw_destroy(tree->mraw, true);

        status = end(lexbor_hash_entry_str(&tag_data->entry),
                     tag_data->entry.length, ctx);
        if (status != LXB_STATUS_OK) {
            goto done;
        }

        lexbor_mraw_clean(mraw);

        declr = declr->next;
    }

done:

    lxb_grammar_tokenizer_destroy(tkz, true);
    lxb_grammar_parser_destroy(parser, true);
    lxb_grammar_document_destroy(doc);
    lexbor_mraw_destroy(mraw, true);
    lexbor_array_destroy(stack, true);

    return status;
}

static utils_lxb_grammar_tree_t *
utils_lxb_grammar_test_group(const lxb_grammar_node_t *root,
                             const lxb_grammar_node_t *declr,
                             lexbor_mraw_t *mraw, lexbor_array_t *stack,
                             size_t limit)
{
    size_t idx, length;
    lxb_char_t *p;
    lxb_status_t status;
    lexbor_array_obj_t out;
    utils_lxb_grammar_tree_t *tree;
    utils_lxb_grammar_entry_t *test;
    const lxb_grammar_node_t *node, *found;

    lxb_char_t buf[4600];
    const size_t buf_len = sizeof(buf) - 1;

    node = declr->first_child;

    status = lexbor_array_obj_init(&out, 16, sizeof(utils_lxb_grammar_entry_t));
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    idx = 0;

    while (node != NULL) {
        test = lexbor_array_obj_push(&out);
        if (test == NULL) {
            return NULL;
        }

        test->node = node;
        test->idx = idx++;

        switch (node->type) {
            case LXB_GRAMMAR_NODE_GROUP:
                tree = utils_lxb_grammar_test_group(root, node, mraw, stack,
                                                    node->limit);
                if (tree == NULL) {
                    return NULL;
                }

                if (node->limit != 0 && node->limit > tree->result.length) {
                    tree->result.length = node->limit;
                    tree->sorted.length = node->limit;
                    tree->length = node->limit;
                }

                test->u.tree = tree;

                break;

            case LXB_GRAMMAR_NODE_ELEMENT:
                status = lexbor_array_push(stack, (void *) node);
                if (status != LXB_STATUS_OK) {
                    return NULL;
                }

                status = utils_lxb_grammar_test_defined(node, mraw, stack, test);
                if (status == LXB_STATUS_OK) {
                    (void) lexbor_array_pop(stack);
                    break;
                }
                else if (status != LXB_STATUS_NEXT) {
                    return NULL;
                }

                found = utils_lxb_grammar_test_find_declaration(root, node);
                if (found == NULL) {
                    return NULL;
                }

                tree = utils_lxb_grammar_test_group(root, found, mraw, stack,
                                                    node->limit);
                if (tree == NULL) {
                    return NULL;
                }

                if (node->limit != 0 && node->limit > tree->result.length) {
                    tree->result.length = node->limit;
                    tree->sorted.length = node->limit;
                    tree->length = node->limit;
                }

                test->u.tree = tree;

                (void) lexbor_array_pop(stack);

                break;

            case LXB_GRAMMAR_NODE_WHITESPACE:
                break;

            case LXB_GRAMMAR_NODE_DELIM:
            case LXB_GRAMMAR_NODE_UNQUOTED:
            case LXB_GRAMMAR_NODE_STRING:
                (void) lexbor_str_init(&test->u.str, mraw, node->u.str.length);
                if (test->u.str.data == NULL) {
                    return NULL;
                }

                p = lexbor_str_append(&test->u.str, mraw, node->u.str.data,
                                      node->u.str.length);
                if (p == NULL) {
                    return NULL;
                }

                break;

            case LXB_GRAMMAR_NODE_NUMBER:
                length = lexbor_conv_float_to_data(node->u.num, buf, buf_len);

                (void) lexbor_str_init(&test->u.str, mraw, length);
                if (test->u.str.data == NULL) {
                    return NULL;
                }

                p = lexbor_str_append(&test->u.str, mraw, buf, length);
                if (p == NULL) {
                    return NULL;
                }

                break;

            default:
                break;
        }

        node = node->next;
    }

    tree = utils_lxb_grammar_test_make(&out, declr, limit, declr->skip_sort);

    (void) lexbor_array_obj_destroy(&out, false);

    return tree;
}

static utils_lxb_grammar_tree_t *
utils_lxb_grammar_test_make(lexbor_array_obj_t *out,
                            const lxb_grammar_node_t *declr, size_t limit,
                            bool without_sort)
{
    lxb_status_t status;
    utils_lxb_grammar_tree_t *tree;

    tree = utils_lxb_grammar_tree_create(true);
    if (tree == NULL) {
        return NULL;
    }

    tree->limit = limit;
    tree->without_sort = without_sort;

    switch (declr->combinator) {
        case LXB_GRAMMAR_COMBINATOR_NORMAL:
            status = utils_lxb_grammar_test_make_child(out, tree);
            break;

        case LXB_GRAMMAR_COMBINATOR_AND:
            status = utils_lxb_grammar_test_and(out, 0, tree);
            break;

        case LXB_GRAMMAR_COMBINATOR_OR:
            status = utils_lxb_grammar_test_and(out, 0, tree);
            break;

        case LXB_GRAMMAR_COMBINATOR_ONE_OF:
            status = utils_lxb_grammar_test_make_next(out, tree);
            break;

        default:
            return NULL;
    }

    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    status = utils_lxb_grammar_test_mix(tree, &tree->calculated,
                                        tree->root.first_child);
    if (status != LXB_STATUS_OK) {
        return NULL;
    }

    status = utils_lxb_grammar_test_compile(tree);
    if (status != LXB_STATUS_OK && status != LXB_STATUS_STOP) {
        return NULL;
    }

    (void) lexbor_hash_destroy(&tree->dupls, false);

    return tree;
}

static lxb_status_t
utils_lxb_grammar_test_and(lexbor_array_obj_t *out, size_t idx,
                           utils_lxb_grammar_tree_t *tree)
{
    lxb_status_t status;
    utils_lxb_grammar_entry_t *entry, *cur;

    if (idx == lexbor_array_obj_length(out)) {
        return utils_lxb_grammar_test_make_child(out, tree);
    }

    cur = lexbor_array_obj_get(out, idx);

    for (size_t i = 0; i < lexbor_array_obj_length(out); i++) {
        entry = lexbor_array_obj_get(out, i);

        if (!entry->used) {
            cur->idx = i;
            entry->used = true;

            status = utils_lxb_grammar_test_and(out, idx + 1, tree);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            entry->used = false;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_make_child(lexbor_array_obj_t *out,
                                  utils_lxb_grammar_tree_t *tree)
{
    utils_lxb_grammar_node_t *node, *root;
    utils_lxb_grammar_entry_t *entry;

    root = &tree->root;
    node = root;

    for (size_t i = 0, x; i < lexbor_array_obj_length(out); i++) {
        entry = lexbor_array_obj_get(out, i);

        x = entry->idx;
        entry = lexbor_array_obj_get(out, x);

        node = utils_lxb_grammar_node_create(tree, entry, x);
        if (node == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        utils_lxb_grammar_node_append_child(root, node);

        root = node;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_make_next(lexbor_array_obj_t *out,
                                 utils_lxb_grammar_tree_t *tree)
{
    utils_lxb_grammar_node_t *node, *root, *first;
    utils_lxb_grammar_entry_t *entry;

    root = NULL;
    node = NULL;
    first = NULL;

    for (size_t i = 0; i < lexbor_array_obj_length(out); i++) {
        entry = lexbor_array_obj_get(out, i);

        node = utils_lxb_grammar_node_create(tree, entry, i);
        if (node == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        if (root != NULL) {
            utils_lxb_grammar_node_insert_after(root, node);
        }
        else {
            first = node;
        }

        root = node;
    }

    tree->root.first_child = first;
    tree->root.last_child = node;

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_mix(utils_lxb_grammar_tree_t *tree,
                           utils_lxb_grammar_node_t *parent,
                           utils_lxb_grammar_node_t *node)
{
    lxb_status_t status;

    while (node != NULL) {
        status = utils_lxb_grammar_test_period(tree, parent, node);
        if (status != LXB_STATUS_OK) {
            if (status == LXB_STATUS_STOP) {
                return LXB_STATUS_OK;
            }

            return status;
        }

        if (tree->limit > 0 && tree->limit <= tree->count) {
            return LXB_STATUS_OK;
        }

        node = node->next;
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_period(utils_lxb_grammar_tree_t *tree,
                              utils_lxb_grammar_node_t *parent,
                              utils_lxb_grammar_node_t *node)
{
    lxb_status_t status;
    utils_lxb_grammar_node_t *new_node;
    utils_lxb_grammar_entry_t *entry;
    const lxb_grammar_period_t *multi;

    if (node == NULL) {
        parent->last = true;

        tree->count++;

        if (tree->limit > 0 && tree->limit <= tree->count) {
            return LXB_STATUS_STOP;
        }

        return LXB_STATUS_OK;
    }

    entry = node->entry;
    multi = &entry->node->multiplier;

    new_node = utils_lxb_grammar_node_create(tree, entry, node->idx);
    if (new_node == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    utils_lxb_grammar_node_append_child(parent, new_node);

    status = utils_lxb_grammar_test_period(tree, new_node,
                                           node->first_child);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (entry->node->parent->combinator == LXB_GRAMMAR_COMBINATOR_OR
        || multi->start == 0)
    {
        status = utils_lxb_grammar_test_period(tree, parent, node->first_child);
        if (status != LXB_STATUS_OK) {
            return status;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_compile(utils_lxb_grammar_tree_t *tree)
{
    lxb_status_t status;
    lexbor_array_t arr;

    status = lexbor_array_init(&arr, 128);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = utils_lxb_grammar_test_result(tree, tree->calculated.first_child,
                                           &arr, 0);

    (void) lexbor_array_destroy(&arr, false);

    return status;
}

static lxb_status_t
utils_lxb_grammar_test_result(utils_lxb_grammar_tree_t *tree,
                              utils_lxb_grammar_node_t *node,
                              lexbor_array_t *indxs, size_t pos)
{
    lxb_status_t status;

    if (node == NULL) {
        return LXB_STATUS_OK;
    }

    status = lexbor_array_push(indxs, node);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    if (node->last) {
        status = utils_lxb_grammar_test_find(tree, indxs);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        if (tree->limit != 0 && tree->result.length >= tree->limit) {
            tree->result.length = tree->limit;
            tree->sorted.length = tree->limit;
            tree->length = tree->limit;

            return LXB_STATUS_STOP;
        }
    }

    status = utils_lxb_grammar_test_result(tree, node->first_child, indxs,
                                           pos + 1);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    indxs->length = pos;

    return utils_lxb_grammar_test_result(tree, node->next, indxs, pos);
}

static lxb_status_t
utils_lxb_grammar_test_find(utils_lxb_grammar_tree_t *tree,
                            lexbor_array_t *indxs)
{
    size_t res_len;
    lxb_status_t status;
    utils_lxb_grammar_node_t **nodes, **sorted, *swap;

    sorted = lexbor_malloc(indxs->length * sizeof(utils_lxb_grammar_node_t *));
    if (sorted == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    nodes = (utils_lxb_grammar_node_t **) indxs->list;

    for (size_t i = 0; i < indxs->length; i++) {
        sorted[i] = nodes[i];

        for (long u = i, t = i - 1; t > -1; t--) {
            if (sorted[t]->idx > sorted[u]->idx) {
                swap = sorted[t];

                sorted[t] = sorted[u];
                sorted[u] = swap;

                u = t;
            }
            else {
                break;
            }
        }
    }

    status = utils_lxb_grammar_test_concat_ids(tree, nodes, sorted,
                                               indxs->length, &res_len);
    tree->length += res_len;

    lexbor_free(sorted);

    return status;
}

static lxb_status_t
utils_lxb_grammar_test_concat_ids(utils_lxb_grammar_tree_t *tree,
                                  utils_lxb_grammar_node_t **nodes,
                                  utils_lxb_grammar_node_t **sorted,
                                  size_t length, size_t *out_length)
{
    size_t prev_len, len, ids_len;
    lxb_status_t status;
    utils_lxb_grammar_node_t *node;
    utils_lxb_grammar_entry_t *entry;
    test_lexbor_grammar_ids_t *ids, *prev, *p;

    prev = NULL;
    prev_len = 1;
    *out_length = 0;

    ids = NULL;
    ids_len = 0;

    for (size_t i = 0; i < length; i++) {
        node = nodes[i];
        entry = node->entry;

        if (entry->node->type == LXB_GRAMMAR_NODE_GROUP
            || entry->node->type == LXB_GRAMMAR_NODE_ELEMENT)
        {
            len = entry->u.tree->length;
        }
        else {
            len = 1;
        }

        ids_len = len * prev_len;

        ids = lexbor_malloc(sizeof(test_lexbor_grammar_ids_t) * ids_len);
        if (ids == NULL) {
            if (prev != NULL) {
                lexbor_free(prev);
            }

            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        p = ids;

        if (prev == NULL) {
            for (size_t t = 0; t < len; t++) {
                p->len = 1;

                p->id = lexbor_mraw_alloc(tree->mraw, sizeof(uint64_t));
                if (p->id == NULL) {
                    goto failed_ids;
                }

                p->id[0] = (uint64_t) node->idx << 32 | (uint64_t) t;

                p++;
            }
        }
        else {
            for (size_t u = 0; u < prev_len; u++) {
                for (size_t t = 0; t < len; t++) {
                    p->len = prev[u].len + 1;

                    p->id = lexbor_mraw_alloc(tree->mraw,
                                              sizeof(uint64_t) * p->len);
                    if (p->id == NULL) {
                        goto failed_ids;
                    }

                    memcpy(p->id, prev[u].id, sizeof(uint64_t) * prev[u].len);

                    p->id[prev[u].len] = (uint64_t) node->idx << 32
                                         | (uint64_t) t;
                    p++;
                }
            }
        }

        if (prev != NULL) {
            lexbor_free(prev);
        }

        prev = ids;
        prev_len = ids_len;
    }

    if (tree->without_sort) {
        status = utils_lxb_grammar_test_wosort(tree, nodes, ids, ids_len,
                                               out_length);
    }
    else {
        status = utils_lxb_grammar_test_sort(tree, nodes, sorted, ids,
                                             ids_len, out_length);
    }

    lexbor_free(ids);

    return status;

failed_ids:

    if (prev != NULL) {
        lexbor_free(prev);
    }

    lexbor_free(ids);

    return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
}

static lxb_status_t
utils_lxb_grammar_test_wosort(utils_lxb_grammar_tree_t *tree,
                              utils_lxb_grammar_node_t **nodes,
                              test_lexbor_grammar_ids_t *ids,
                              size_t length, size_t *out_length)
{
    size_t len;
    lxb_status_t status;
    lexbor_str_t *str;
    test_lexbor_grammar_ids_t *p;

    for (size_t u = 0; u < length; u++) {
        p = &ids[u];

        str = utils_lxb_grammar_test_make_text(tree, nodes, p->id,
                                               p->len, false, &len);
        if (str == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        status = utils_lxb_grammar_append_wodupl(tree, &tree->result,
                                                 &tree->sorted, str, str, len,
                                                 out_length);
        if (status != LXB_STATUS_OK) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_sort(utils_lxb_grammar_tree_t *tree,
                            utils_lxb_grammar_node_t **nodes,
                            utils_lxb_grammar_node_t **sorted,
                            test_lexbor_grammar_ids_t *ids,
                            size_t length, size_t *out_length)
{
    size_t len, rlen;
    uint64_t swap, *id, *ids_sorted, *nsort;
    lxb_status_t status;
    lexbor_str_t *str, *srd;
    test_lexbor_grammar_ids_t *p;

    len = 128;
    ids_sorted = lexbor_malloc(sizeof(uint64_t) * len);
    if (ids_sorted == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    for (size_t u = 0; u < length; u++) {
        p = &ids[u];
        id = p->id;

        if (p->len > len) {
            len += 128;

            nsort = lexbor_realloc(ids_sorted, sizeof(uint64_t) * len);
            if (nsort == NULL) {
                goto failed;
            }

            ids_sorted = nsort;
        }

        for (size_t i = 0; i < p->len; i++) {
            ids_sorted[i] = id[i];

            for (long u = i, t = i - 1; t > -1; t--) {
                if (ids_sorted[t] > ids_sorted[u]) {
                    swap = ids_sorted[t];

                    ids_sorted[t] = ids_sorted[u];
                    ids_sorted[u] = swap;

                    u = t;
                }
                else {
                    break;
                }
            }
        }

        str = utils_lxb_grammar_test_make_text(tree, nodes, ids[u].id, p->len,
                                               false, &rlen);
        if (str == NULL) {
            goto failed;
        }

        srd = utils_lxb_grammar_test_make_text(tree, sorted, ids_sorted, p->len,
                                               true, &rlen);
        if (srd == NULL) {
            goto failed;
        }

        status = utils_lxb_grammar_append_wodupl(tree, &tree->result,
                                                 &tree->sorted, str, srd, rlen,
                                                 out_length);
        if (status != LXB_STATUS_OK) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    lexbor_free(ids_sorted);

    return LXB_STATUS_OK;

failed:

    lexbor_free(ids_sorted);

    return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
}

static lxb_status_t
utils_lxb_grammar_append_wodupl(utils_lxb_grammar_tree_t *tree,
                                lexbor_array_t *result, lexbor_array_t *sorted,
                                lexbor_str_t *str, lexbor_str_t *srd,
                                size_t length, size_t *out_length)
{
    void *data;
    size_t done;
    lxb_status_t status;
    lexbor_str_t *org;

    done = 0;

    for (size_t i = 0; i < length; i++) {
        org = &str[i];

        data = lexbor_hash_search(&tree->dupls, &test_dupl_search,
                                  org->data, org->length);
        if (data != NULL) {
            continue;
        }

        data = lexbor_hash_insert(&tree->dupls, &test_dupl_insert,
                                  org->data, org->length);
        if (data == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        status = lexbor_array_push(result, org);
        if (status != LXB_STATUS_OK) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        status = lexbor_array_push(sorted, &srd[i]);
        if (status != LXB_STATUS_OK) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        done += 1;
    }

    *out_length += done;

    return LXB_STATUS_OK;
}

static lexbor_str_t *
utils_lxb_grammar_test_make_text(utils_lxb_grammar_tree_t *tree,
                                 utils_lxb_grammar_node_t **nodes,
                                 uint64_t *ids, size_t length,
                                 bool is_sorted, size_t *out_length)
{
    lxb_status_t status;
    uint64_t id;
    size_t prev_len, str_len, total, min, max, cidx;
    lexbor_str_t *str, *ptr, *p;
    const lexbor_str_t *prev;
    const lxb_grammar_period_t *multi;
    utils_lxb_grammar_node_t *node;
    utils_lxb_grammar_entry_t *entry;
    lxb_dom_attr_t *attr;
    const lexbor_str_t *sp;
    const lxb_grammar_node_t *onode;

    static const lexbor_str_t attr_nows = lexbor_str("nows");

    prev = &lxb_wo_ws;
    prev_len = 1;

    str = NULL;
    str_len = 0;
    total = 0;

    for (size_t i = 0; i < length; i++) {
        node = nodes[i];
        entry = node->entry;
        onode = entry->node;
        multi = &onode->multiplier;

        id = ids[i];
        cidx = (id << 32) >> 32;

        if (onode->is_comma) {
            sp = &lxb_comma;
        }
        else {
            sp = &lxb_ws;
        }

        if (onode->type == LXB_GRAMMAR_NODE_ELEMENT) {
            attr = lxb_dom_element_attr_by_name(lxb_dom_interface_element(onode->u.node),
                                                attr_nows.data, attr_nows.length);
            if (attr != NULL) {
                if (sp == &lxb_comma) {
                    sp = &lxb_wo_ws_comma;
                }
                else {
                    sp = &lxb_wo_ws;
                }
            }
        }

        if (entry->node->type == LXB_GRAMMAR_NODE_GROUP
            || entry->node->type == LXB_GRAMMAR_NODE_ELEMENT)
        {
            if (is_sorted) {
                ptr = ((lexbor_str_t **) entry->u.tree->sorted.list)[cidx];
            }
            else {
                ptr = ((lexbor_str_t **) entry->u.tree->result.list)[cidx];
            }
        }
        else {
            ptr = &entry->u.str;
        }

        str_len = prev_len;

        if (multi->stop == -1 || multi->stop > test_max_repeat) {
            max = test_max_repeat;
        }
        else {
            max = multi->stop;
        }

        min = (multi->start > max) ? max : multi->start;
        min = (min == 0) ? 1 : min;
        max = (max < min) ? min : max;

        str_len += (max - min);

        str = lexbor_mraw_alloc(tree->mraw, sizeof(lexbor_str_t) * str_len);
        if (str == NULL) {
            return NULL;
        }

        p = &str[0];
        total = 0;

        for (size_t i = 0; i < prev_len; i++) {
            status = utils_lxb_grammar_test_concat_str(tree, node, &prev[i],
                                                       ptr, p, sp, min);
            if (status != LXB_STATUS_OK) {
                return NULL;
            }

            total++;

            p = utils_lxb_grammar_test_repeat(tree, node, &total, p,
                                              ptr, p + 1, sp, min, max);
            if (p == NULL) {
                return NULL;
            }
        }

        if (prev != &lxb_wo_ws) {
            lexbor_mraw_free(tree->mraw, (lexbor_str_t *) prev);
        }

        prev = str;
        prev_len = str_len;
    }

    *out_length = str_len;

    if (total != str_len) {
        printf("Cycles and Total are not equal.");
        return NULL;
    }

    return str;
}

static lxb_status_t
utils_lxb_grammar_test_concat_str(utils_lxb_grammar_tree_t *tree,
                                  utils_lxb_grammar_node_t *node,
                                  const lexbor_str_t *pstr,
                                  const lexbor_str_t *str, lexbor_str_t *result,
                                  const lexbor_str_t *sp, size_t count)
{
    size_t length;

    length = ((pstr->length + str->length) * count) + 2;

    (void) lexbor_str_init(result, tree->mraw, length);
    if (str->data == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    if (pstr->length != 0) {
        (void) lexbor_str_append(result, tree->mraw,
                                 pstr->data, pstr->length);

        if (!node->entry->node->skip_ws) {
            (void) lexbor_str_append(result, tree->mraw,
                                     lxb_ws.data, lxb_ws.length);
        }
    }

    (void) lexbor_str_append(result, tree->mraw, str->data, str->length);

    for (size_t i = 1; i < count; i++) {
        (void) lexbor_str_append(result, tree->mraw, sp->data, sp->length);
        (void) lexbor_str_append(result, tree->mraw, str->data, str->length);
    }

    return LXB_STATUS_OK;
}

static lexbor_str_t *
utils_lxb_grammar_test_repeat(utils_lxb_grammar_tree_t *tree,
                              utils_lxb_grammar_node_t *node,
                              size_t *total, const lexbor_str_t *pstr,
                              const lexbor_str_t *str, lexbor_str_t *result,
                              const lexbor_str_t *sp, size_t min, size_t max)
{
    size_t length, count;

    count = max - min;

    for (size_t i = 0; i < count; i++) {
        length = pstr->length + str->length + sp->length + 1;

        (void) lexbor_str_init(result, tree->mraw, length);
        if (str->data == NULL) {
            return NULL;
        }

        (void) lexbor_str_append(result, tree->mraw, pstr->data, pstr->length);

        (void) lexbor_str_append(result, tree->mraw,
                                 sp->data, sp->length);

        (void) lexbor_str_append(result, tree->mraw, str->data, str->length);

        pstr = result;
        result++;
    }

    *total += count;

    return result;
}

//static lxb_status_t
//utils_lxb_grammar_test_serialize(utils_lxb_grammar_tree_t *tree)
//{
//    size_t length;
//    lexbor_str_t **result, **sorted, *org, *srd;
//
//    length = 0;
//    result = (lexbor_str_t **) tree->result.list;
//    sorted = (lexbor_str_t **)tree->sorted.list;
//
//    while (length < tree->length) {
//        org = result[length];
//        srd = sorted[length];
//
//        length += 1;
//
//        printf("%s -- %s\n", (const char *) org->data,
//               (const char *) srd->data);
//    }
//
//    return LXB_STATUS_OK;
//}

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

static lxb_status_t
utils_lxb_grammar_test_defined(const lxb_grammar_node_t *node,
                               lexbor_mraw_t *mraw, lexbor_array_t *stack,
                               utils_lxb_grammar_entry_t *test)
{
    size_t length;
    const lxb_char_t *name;

    static const lexbor_str_t number = lexbor_str("number");
    static const lexbor_str_t integer = lexbor_str("integer");
    static const lexbor_str_t percentage = lexbor_str("percentage");
    static const lexbor_str_t hex = lexbor_str("hex");

    name = lxb_dom_element_qualified_name(lxb_dom_interface_element(node->u.node),
                                          &length);
    if (name == NULL) {
        return LXB_STATUS_NEXT;
    }

    if (number.length == length
        && lexbor_str_data_ncasecmp(number.data, name, length))
    {
        return utils_lxb_grammar_test_number(node, stack, test, false);
    }
    else if (integer.length == length
             && lexbor_str_data_ncasecmp(integer.data, name, length))
    {
        return utils_lxb_grammar_test_number(node, stack, test, false);
    }
    else if (percentage.length == length
             && lexbor_str_data_ncasecmp(percentage.data, name, length))
    {
        return utils_lxb_grammar_test_number(node, stack, test, true);
    }
    else if (hex.length == length
             && lexbor_str_data_ncasecmp(hex.data, name, length))
    {
        return utils_lxb_grammar_test_hex(node, stack, test);
    }

    return LXB_STATUS_NEXT;
}

static lxb_status_t
utils_lxb_grammar_test_number(const lxb_grammar_node_t *root,
                              lexbor_array_t *stack,
                              utils_lxb_grammar_entry_t *test, bool percentage)
{
    size_t length, idx;
    bool dmin, dmax;
    double min, max;
    lxb_status_t status;
    lxb_dom_attr_t *attr;
    const lxb_char_t *p;
    const lxb_grammar_node_t *node;
    utils_lxb_grammar_tree_t *tree;

    lxb_char_t buf[4600];
    const size_t buf_len = sizeof(buf) - 1;

    static const lexbor_str_t str_min = lexbor_str("min");
    static const lexbor_str_t str_max = lexbor_str("max");

    tree = utils_lxb_grammar_tree_create(false);
    if (tree == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
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

    status = utils_lxb_grammar_tree_append(tree, buf, length);
    if (status != LXB_STATUS_OK) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    length = lexbor_conv_float_to_data(max, buf, buf_len);

    if (percentage) {
        buf[length++] = '%';
    }

    status = utils_lxb_grammar_tree_append(tree, buf, length);
    if (status != LXB_STATUS_OK) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    test->u.tree = tree;
    tree->length = 2;

    return LXB_STATUS_OK;
}

static lxb_status_t
utils_lxb_grammar_test_hex(const lxb_grammar_node_t *root,
                           lexbor_array_t *stack,
                           utils_lxb_grammar_entry_t *test)
{
    lxb_char_t buf;
    lxb_status_t status;
    utils_lxb_grammar_tree_t *tree;

    tree = utils_lxb_grammar_tree_create(false);
    if (tree == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    buf = '0';

    status = utils_lxb_grammar_tree_append(tree, &buf, 1);
    if (status != LXB_STATUS_OK) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    buf = 'f';

    status = utils_lxb_grammar_tree_append(tree, &buf, 1);
    if (status != LXB_STATUS_OK) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    test->u.tree = tree;
    tree->length = 2;

    return LXB_STATUS_OK;
}
