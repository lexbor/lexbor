/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Stephane Janel <https://github.com/sjanel>
 */

#include <lexbor/html/interfaces/document.h>
#include <lexbor/html/parser.h>
#include <lexbor/html/serialize.h>

#include <unit/test.h>


#define TEST_PUT(...)                                                          \
    (length += (size_t) snprintf((char *) data + length, size - length,        \
                                 __VA_ARGS__))


typedef struct {
    const lxb_char_t *data;
    size_t           length;
    lxb_char_t       *document;
    size_t           document_length;
    lxb_char_t       *fragment;
    size_t           fragment_length;
}
test_input_t;

typedef struct {
    lxb_char_t *data;
    size_t     length;
    size_t     size;
}
test_buffer_t;


static size_t test_fail_at;
static size_t test_count;
static size_t test_live;
static bool   test_armed;
static bool   test_fail_all;
static bool   test_reached;

static const char test_small[] =
    "<!doctype html><html><head><title>t</title></head><body>"
    "<p class=a>x<b>y</b></p><div id=d><span>z</span>"
    "<table><tr><td>1</td></tr></table></div></body></html>";

static test_input_t test_inputs[3];


static bool
test_fail(void)
{
    size_t index;

    if (!test_armed) {
        return false;
    }

    index = test_count++;

    if (index == test_fail_at) {
        test_reached = true;
        return true;
    }

    return test_fail_all && index > test_fail_at;
}

static void *
test_malloc(size_t size)
{
    void *ptr;

    if (test_fail()) {
        return NULL;
    }

    ptr = malloc(size);
    if (ptr != NULL) {
        test_live++;
    }

    return ptr;
}

static void *
test_realloc(void *dst, size_t size)
{
    void *ptr;

    if (test_fail()) {
        return NULL;
    }

    ptr = realloc(dst, size);

    if (dst == NULL) {
        if (ptr != NULL) {
            test_live++;
        }
    }
    else if (ptr == NULL && size == 0) {
        test_live--;
    }

    return ptr;
}

static void *
test_calloc(size_t num, size_t size)
{
    void *ptr;

    if (test_fail()) {
        return NULL;
    }

    ptr = calloc(num, size);
    if (ptr != NULL) {
        test_live++;
    }

    return ptr;
}

static void
test_free(void *dst)
{
    if (dst != NULL) {
        test_live--;
    }

    free(dst);
}

static void
test_arm(size_t index, bool all)
{
    test_count = 0;
    test_fail_at = index;
    test_fail_all = all;
    test_reached = false;
    test_armed = true;
}

static void
test_disarm(void)
{
    test_armed = false;
}

static lxb_status_t
test_serialize_cb(const lxb_char_t *data, size_t len, void *ctx)
{
    size_t size;
    lxb_char_t *tmp;
    test_buffer_t *buf = ctx;

    if (buf->size - buf->length < len) {
        size = (buf->length + len) * 2;

        tmp = lexbor_realloc(buf->data, size);
        if (tmp == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        buf->data = tmp;
        buf->size = size;
    }

    memcpy(buf->data + buf->length, data, len);
    buf->length += len;

    return LXB_STATUS_OK;
}

static lxb_char_t *
test_serialize(lxb_dom_node_t *node, size_t *length)
{
    lxb_status_t status;
    test_buffer_t buf = {0};

    status = lxb_html_serialize_deep_cb(node, test_serialize_cb, &buf);
    if (status != LXB_STATUS_OK) {
        return lexbor_free(buf.data);
    }

    *length = buf.length;

    return buf.data;
}

static bool
test_tree_is(lxb_dom_node_t *node, const lxb_char_t *expect, size_t length)
{
    bool is;
    size_t len;
    lxb_char_t *data;

    data = test_serialize(node, &len);
    if (data == NULL) {
        return false;
    }

    is = len == length && memcmp(data, expect, len) == 0;

    lexbor_free(data);

    return is;
}

static lxb_html_document_t *
test_document_with_body(void)
{
    lxb_status_t status;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    if (document == NULL) {
        return NULL;
    }

    status = lxb_html_document_parse(document,
                         (const lxb_char_t *) "<!DOCTYPE html><div>", 20);
    if (status != LXB_STATUS_OK) {
        return lxb_html_document_destroy(document);
    }

    return document;
}

static lxb_status_t
test_input_init(test_input_t *input, const lxb_char_t *data, size_t length)
{
    lxb_status_t status;
    lxb_dom_node_t *fragment;
    lxb_html_document_t *document;

    input->data = data;
    input->length = length;

    document = test_document_with_body();
    if (document == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    fragment = lxb_html_document_parse_fragment(document,
                     lxb_dom_interface_element(document->body), data, length);
    if (fragment == NULL) {
        lxb_html_document_destroy(document);
        return LXB_STATUS_ERROR;
    }

    input->fragment = test_serialize(fragment, &input->fragment_length);

    status = lxb_html_document_parse(document, data, length);
    if (status == LXB_STATUS_OK) {
        input->document = test_serialize(lxb_dom_interface_node(document),
                                         &input->document_length);
    }

    lxb_html_document_destroy(document);

    if (input->fragment == NULL || input->document == NULL) {
        return LXB_STATUS_ERROR;
    }

    return LXB_STATUS_OK;
}

static void
test_input_destroy(test_input_t *input)
{
    input->document = lexbor_free(input->document);
    input->fragment = lexbor_free(input->fragment);
}

/*
 * Large enough to grow the memory of nodes, text, names and stacks in the
 * middle of the tree construction, not only while the parser is created.
 */
static lxb_char_t *
test_large_make(size_t *out_length)
{
    size_t i, length = 0, size = 512 * 1024;
    lxb_char_t *data;

    data = lexbor_malloc(size);
    if (data == NULL) {
        return NULL;
    }

    TEST_PUT("<!DOCTYPE html><html><head><title>large</title></head><body>");

    for (i = 0; i < 100; i++) {
        TEST_PUT("<div class=c%zu id=i%zu data-u%zu=\"v&amp;%zu\">"
                 "<x-tag-%zu attr-%zu=%zu>text %zu &copy; &#x41;</x-tag-%zu>"
                 "<table>foster %zu<tr><td>%zu</table><b><i>%zu</b></i>"
                 "<svg viewBox='0 0 %zu 1'><path d=M0></path></svg></div>",
                 i, i, i, i, i, i, i, i, i, i, i, i, i);
    }

    for (i = 0; i < 300; i++) {
        TEST_PUT("<div>");
    }

    TEST_PUT("deep");

    for (i = 0; i < 300; i++) {
        TEST_PUT("</div>");
    }

    TEST_PUT("<p title=\"");

    for (i = 0; i < 20000; i++) {
        TEST_PUT("v");
    }

    TEST_PUT("\">");

    for (i = 0; i < 20000; i++) {
        TEST_PUT("t");
    }

    TEST_PUT("</p></body></html>");

    *out_length = length;

    return data;
}

static lxb_char_t *
test_templates_make(size_t *out_length)
{
    size_t i, length = 0, size = 128 * 1024;
    lxb_char_t *data;

    data = lexbor_malloc(size);
    if (data == NULL) {
        return NULL;
    }

    for (i = 0; i < 2000; i++) {
        TEST_PUT("<template><p>%zu</p></template>", i);
    }

    *out_length = length;

    return data;
}


TEST_BEGIN(document_create)
{
    size_t index, live;
    lxb_html_document_t *document;

    for (index = 0;; index++) {
        live = test_live;

        test_arm(index, false);
        document = lxb_html_document_create();
        test_disarm();

        lxb_html_document_destroy(document);

        test_eq_size(test_live, live);

        if (!test_reached) {
            break;
        }

        test_eq(document, NULL);
    }

    test_ne(document, NULL);
    test_ne_size(index, 0);
}
TEST_END

TEST_BEGIN_ARGS(document_parse_input, test_input_t *input, bool all)
{
    size_t index, live;
    lxb_status_t status;
    lxb_html_document_t *document;

    for (index = 0;; index++) {
        live = test_live;

        document = lxb_html_document_create();
        test_ne(document, NULL);

        test_arm(index, all);
        status = lxb_html_document_parse(document, input->data,
                                         input->length);
        test_disarm();

        if (status == LXB_STATUS_OK) {
            test_eq(test_tree_is(lxb_dom_interface_node(document),
                                 input->document, input->document_length),
                    true);
        }

        lxb_html_document_destroy(document);

        test_eq_size(test_live, live);

        if (!test_reached) {
            break;
        }
    }

    test_eq(status, LXB_STATUS_OK);
    test_ne_size(index, 0);
}
TEST_END

TEST_BEGIN_ARGS(document_reparse_input, test_input_t *input, bool all)
{
    size_t index, live;
    lxb_status_t status;
    lxb_html_document_t *document;

    for (index = 0;; index++) {
        live = test_live;

        document = test_document_with_body();
        test_ne(document, NULL);

        test_arm(index, all);
        status = lxb_html_document_parse(document, input->data,
                                         input->length);
        test_disarm();

        if (status == LXB_STATUS_OK) {
            test_eq(test_tree_is(lxb_dom_interface_node(document),
                                 input->document, input->document_length),
                    true);
        }

        lxb_html_document_destroy(document);

        test_eq_size(test_live, live);

        if (!test_reached) {
            break;
        }
    }

    test_eq(status, LXB_STATUS_OK);
}
TEST_END

static lxb_status_t
test_parse_chunked(lxb_html_document_t *document, const test_input_t *input)
{
    size_t i, len;
    lxb_status_t status;

    status = lxb_html_document_parse_chunk_begin(document);

    for (i = 0; status == LXB_STATUS_OK && i < input->length; i += len) {
        len = input->length - i < 7 ? input->length - i : 7;

        status = lxb_html_document_parse_chunk(document, input->data + i, len);
    }

    if (status != LXB_STATUS_OK) {
        return status;
    }

    return lxb_html_document_parse_chunk_end(document);
}

TEST_BEGIN_ARGS(document_parse_again_input, test_input_t *input, bool all,
                bool chunked)
{
    size_t index, live;
    lxb_status_t status;
    lxb_html_document_t *document;

    for (index = 0;; index++) {
        live = test_live;

        document = lxb_html_document_create();
        test_ne(document, NULL);

        test_arm(index, all);
        status = lxb_html_document_parse(document, input->data,
                                         input->length);
        test_disarm();

        if (status != LXB_STATUS_OK) {
            if (chunked) {
                status = test_parse_chunked(document, input);
            }
            else {
                status = lxb_html_document_parse(document, input->data,
                                                 input->length);
            }

            test_eq(status, LXB_STATUS_OK);
        }

        test_eq(test_tree_is(lxb_dom_interface_node(document),
                             input->document, input->document_length), true);

        lxb_html_document_destroy(document);

        test_eq_size(test_live, live);

        if (!test_reached) {
            break;
        }
    }
}
TEST_END

TEST_BEGIN_ARGS(document_parse_chunk_input, test_input_t *input, bool all)
{
    size_t index, live, i, len;
    lxb_status_t status;
    lxb_html_document_t *document;

    for (index = 0;; index++) {
        live = test_live;

        document = lxb_html_document_create();
        test_ne(document, NULL);

        test_arm(index, all);
        status = lxb_html_document_parse_chunk_begin(document);

        for (i = 0; status == LXB_STATUS_OK && i < input->length; i += len) {
            len = input->length - i < 7 ? input->length - i : 7;

            status = lxb_html_document_parse_chunk(document, input->data + i,
                                                   len);
        }

        if (status == LXB_STATUS_OK) {
            status = lxb_html_document_parse_chunk_end(document);
        }

        test_disarm();

        if (status == LXB_STATUS_OK) {
            test_eq(test_tree_is(lxb_dom_interface_node(document),
                                 input->document, input->document_length),
                    true);
        }

        lxb_html_document_destroy(document);

        test_eq_size(test_live, live);

        if (!test_reached) {
            break;
        }
    }

    test_eq(status, LXB_STATUS_OK);
}
TEST_END

TEST_BEGIN_ARGS(document_parse_fragment_input, test_input_t *input, bool all)
{
    size_t index, live;
    lxb_dom_node_t *fragment;
    lxb_html_document_t *document;

    for (index = 0;; index++) {
        live = test_live;

        document = test_document_with_body();
        test_ne(document, NULL);

        test_arm(index, all);
        fragment = lxb_html_document_parse_fragment(document,
                                   lxb_dom_interface_element(document->body),
                                   input->data, input->length);
        test_disarm();

        if (fragment != NULL) {
            test_eq(test_tree_is(fragment, input->fragment,
                                 input->fragment_length), true);
        }

        lxb_html_document_destroy(document);

        test_eq_size(test_live, live);

        if (!test_reached) {
            break;
        }
    }

    test_ne(fragment, NULL);
}
TEST_END

TEST_BEGIN_ARGS(parser_parse_fragment_input, test_input_t *input, bool all)
{
    size_t index, live;
    lxb_status_t status;
    lxb_dom_node_t *fragment;
    lxb_html_parser_t *parser;

    for (index = 0;; index++) {
        live = test_live;

        parser = lxb_html_parser_create();
        status = lxb_html_parser_init(parser);
        test_eq(status, LXB_STATUS_OK);

        test_arm(index, all);
        fragment = lxb_html_parse_fragment_by_tag_id(parser, NULL,
                                                     LXB_TAG_BODY, LXB_NS_HTML,
                                                     input->data,
                                                     input->length);
        test_disarm();

        if (fragment != NULL) {
            test_eq(test_tree_is(fragment, input->fragment,
                                 input->fragment_length), true);

            lxb_html_document_destroy(
                      lxb_html_interface_document(fragment->owner_document));
        }

        lxb_html_parser_destroy(parser);

        test_eq_size(test_live, live);

        if (!test_reached) {
            break;
        }
    }

    test_ne(fragment, NULL);
}
TEST_END

TEST_BEGIN_ARGS(parser_parse_fragment_chunk_input, test_input_t *input,
                bool all)
{
    size_t index, live, i, len;
    bool is_ready;
    lxb_status_t status, begin;
    lxb_dom_node_t *fragment;
    lxb_html_parser_t *parser;

    for (index = 0;; index++) {
        live = test_live;
        fragment = NULL;

        parser = lxb_html_parser_create();
        status = lxb_html_parser_init(parser);
        test_eq(status, LXB_STATUS_OK);

        test_arm(index, all);
        begin = lxb_html_parse_fragment_chunk_begin(parser, NULL, LXB_TAG_BODY,
                                                    LXB_NS_HTML);
        is_ready = lxb_html_parser_state(parser)
                   == LXB_HTML_PARSER_STATE_FRAGMENT_PROCESS;
        status = begin;

        for (i = 0; status == LXB_STATUS_OK && i < input->length; i += len) {
            len = input->length - i < 7 ? input->length - i : 7;

            status = lxb_html_parse_fragment_chunk_process(parser,
                                                           input->data + i,
                                                           len);
        }

        if (status == LXB_STATUS_OK) {
            fragment = lxb_html_parse_fragment_chunk_end(parser);
        }

        test_disarm();

        test_eq(begin == LXB_STATUS_OK, is_ready);

        if (fragment != NULL) {
            test_eq(test_tree_is(fragment, input->fragment,
                                 input->fragment_length), true);

            lxb_html_document_destroy(
                      lxb_html_interface_document(fragment->owner_document));
        }

        lxb_html_parser_destroy(parser);

        test_eq_size(test_live, live);

        if (!test_reached) {
            break;
        }
    }

    test_ne(fragment, NULL);
}
TEST_END

TEST_BEGIN(document_parse)
{
    size_t i;

    for (i = 0; i < sizeof(test_inputs) / sizeof(test_input_t); i++) {
        TEST_CALL_ARGS(document_parse_input, &test_inputs[i], false);
        TEST_CALL_ARGS(document_parse_input, &test_inputs[i], true);
    }
}
TEST_END

TEST_BEGIN(document_reparse)
{
    size_t i;

    for (i = 0; i < sizeof(test_inputs) / sizeof(test_input_t); i++) {
        TEST_CALL_ARGS(document_reparse_input, &test_inputs[i], false);
        TEST_CALL_ARGS(document_reparse_input, &test_inputs[i], true);
    }
}
TEST_END

TEST_BEGIN(document_parse_again)
{
    size_t i;

    for (i = 0; i < sizeof(test_inputs) / sizeof(test_input_t); i++) {
        TEST_CALL_ARGS(document_parse_again_input, &test_inputs[i],
                       false, false);
        TEST_CALL_ARGS(document_parse_again_input, &test_inputs[i],
                       true, false);
        TEST_CALL_ARGS(document_parse_again_input, &test_inputs[i],
                       false, true);
        TEST_CALL_ARGS(document_parse_again_input, &test_inputs[i],
                       true, true);
    }
}
TEST_END

TEST_BEGIN(document_parse_chunk)
{
    size_t i;

    for (i = 0; i < sizeof(test_inputs) / sizeof(test_input_t); i++) {
        TEST_CALL_ARGS(document_parse_chunk_input, &test_inputs[i], false);
        TEST_CALL_ARGS(document_parse_chunk_input, &test_inputs[i], true);
    }
}
TEST_END

TEST_BEGIN(document_parse_fragment)
{
    size_t i;

    for (i = 0; i < sizeof(test_inputs) / sizeof(test_input_t); i++) {
        TEST_CALL_ARGS(document_parse_fragment_input, &test_inputs[i], false);
        TEST_CALL_ARGS(document_parse_fragment_input, &test_inputs[i], true);
    }
}
TEST_END

TEST_BEGIN(parser_parse_fragment)
{
    size_t i;

    for (i = 0; i < sizeof(test_inputs) / sizeof(test_input_t); i++) {
        TEST_CALL_ARGS(parser_parse_fragment_input, &test_inputs[i], false);
        TEST_CALL_ARGS(parser_parse_fragment_input, &test_inputs[i], true);
    }
}
TEST_END

TEST_BEGIN(parser_parse_fragment_chunk)
{
    size_t i;

    for (i = 0; i < sizeof(test_inputs) / sizeof(test_input_t); i++) {
        TEST_CALL_ARGS(parser_parse_fragment_chunk_input, &test_inputs[i],
                       false);
        TEST_CALL_ARGS(parser_parse_fragment_chunk_input, &test_inputs[i],
                       true);
    }
}
TEST_END


int
main(int argc, const char *argv[])
{
    size_t i, length, templates_length;
    lxb_char_t *large, *templates;
    lxb_status_t status;

    TEST_INIT();

    (void) lexbor_memory_setup(test_malloc, test_realloc, test_calloc,
                               test_free);

    large = test_large_make(&length);
    templates = test_templates_make(&templates_length);

    if (large == NULL || templates == NULL) {
        TEST_FAILURE("Failed to make the inputs");
    }

    status = test_input_init(&test_inputs[0], (const lxb_char_t *) test_small,
                             sizeof(test_small) - 1);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to parse the small input");
    }

    status = test_input_init(&test_inputs[1], large, length);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to parse the large input");
    }

    status = test_input_init(&test_inputs[2], templates, templates_length);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to parse the templates input");
    }

    TEST_ADD(document_create);
    TEST_ADD(document_parse);
    TEST_ADD(document_reparse);
    TEST_ADD(document_parse_again);
    TEST_ADD(document_parse_chunk);
    TEST_ADD(document_parse_fragment);
    TEST_ADD(parser_parse_fragment);
    TEST_ADD(parser_parse_fragment_chunk);

    TEST_RUN("lexbor/html/allocation_failure");

    for (i = 0; i < sizeof(test_inputs) / sizeof(test_input_t); i++) {
        test_input_destroy(&test_inputs[i]);
    }

    lexbor_free(large);
    lexbor_free(templates);

    (void) lexbor_memory_setup(malloc, realloc, calloc, free);

    TEST_RELEASE();
}
