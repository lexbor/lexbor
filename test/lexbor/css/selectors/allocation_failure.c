/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/css/css.h>

#include <unit/test.h>


typedef struct {
    lxb_char_t *data;
    size_t     length;
    lxb_char_t *result;
}
test_input_t;

typedef struct {
    test_input_t *list;
    size_t       length;
    size_t       size;
}
test_inputs_t;


static size_t test_fail_at;
static size_t test_count;
static size_t test_live;
static bool   test_armed;
static bool   test_fail_all;
static bool   test_reached;

static test_inputs_t test_inputs;

static const char *test_selectors[] = {
    ":lexbor-contains(\"abc\" i)",
    ":lexbor-contains(abc)",
    ":lexbor-contains(1):is(a)",
    ":is(:lexbor-contains(1), a)",
    ":is(:lexbor-contains(abc i), :nth-child(2n+1 of p, .a))",
    ":where(a, :lexbor-contains(\"abc\" i x))",
    ":nth-child(2n+1 of p):nth-last-child(odd of .a > b)",
    ":nth-of-type(2n x), a",
    ":current(a, :not(.b)) :has(> c)"
};


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

static lxb_char_t *
test_result(const lxb_char_t *data, size_t length)
{
    lxb_char_t *result;
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_selector_list_t *list;

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, NULL);
    if (status != LXB_STATUS_OK) {
        TEST_FAILURE("Failed to init the CSS parser");
    }

    list = lxb_css_selectors_parse(parser, data, length);

    if (list != NULL) {
        result = lxb_css_selector_serialize_list_chain_char(list, NULL);
        if (result == NULL) {
            TEST_FAILURE("Failed to serialize a selector list");
        }

        lxb_css_selector_list_destroy_memory(list);
    }
    else {
        result = NULL;
    }

    (void) lxb_css_parser_destroy(parser, true);

    return result;
}

static void
test_input_add(lxb_char_t *data, size_t length)
{
    test_input_t *input;

    if (test_inputs.length == test_inputs.size) {
        test_inputs.size = (test_inputs.size + 32) * 2;

        input = lexbor_realloc(test_inputs.list,
                               test_inputs.size * sizeof(test_input_t));
        if (input == NULL) {
            TEST_FAILURE("Failed to allocate the inputs");
        }

        test_inputs.list = input;
    }

    input = &test_inputs.list[test_inputs.length++];

    input->data = data;
    input->length = length;
    input->result = test_result(data, length);
}

static lxb_char_t *
test_make(const char *begin, size_t count, char ch, const char *end,
          size_t *length)
{
    size_t blen, elen;
    lxb_char_t *data;

    blen = strlen(begin);
    elen = strlen(end);

    data = lexbor_malloc(blen + count + elen + 1);
    if (data == NULL) {
        TEST_FAILURE("Failed to allocate an input");
    }

    memcpy(data, begin, blen);
    memset(data + blen, ch, count);
    memcpy(data + blen + count, end, elen + 1);

    *length = blen + count + elen;

    return data;
}

static lxb_char_t *
test_nested_make(const char *begin, const char *open, size_t count,
                 const char *inner, const char *end, size_t *length)
{
    size_t i, blen, olen, ilen, elen;
    lxb_char_t *data, *p;

    blen = strlen(begin);
    olen = strlen(open);
    ilen = strlen(inner);
    elen = strlen(end);

    data = lexbor_malloc(blen + (olen + 1) * count + ilen + elen + 1);
    if (data == NULL) {
        TEST_FAILURE("Failed to allocate an input");
    }

    memcpy(data, begin, blen);
    p = data + blen;

    for (i = 0; i < count; i++) {
        memcpy(p, open, olen);
        p += olen;
    }

    memcpy(p, inner, ilen);
    p += ilen;

    memset(p, ')', count);
    p += count;

    memcpy(p, end, elen + 1);
    p += elen;

    *length = (size_t) (p - data);

    return data;
}

static void
test_inputs_init(void)
{
    size_t i, length;
    lxb_char_t *data;

    for (i = 0; i < sizeof(test_selectors) / sizeof(char *); i++) {
        data = test_make(test_selectors[i], 0, 0, "", &length);
        test_input_add(data, length);
    }

    for (i = 3968; i <= 4096; i++) {
        data = test_make(".", i, 'p', ":lexbor-contains(abc i)", &length);
        test_input_add(data, length);

        data = test_make(".", i, 'p', ":nth-child(2n+1 of q)", &length);
        test_input_add(data, length);
    }

    data = test_make(":lexbor-contains(\"", 5000, 'x', "\" i)", &length);
    test_input_add(data, length);

    data = test_nested_make("", ":nth-child(2n+1 of ", 80, "a", "", &length);
    test_input_add(data, length);

    data = test_nested_make(":is(", ":nth-child(2n+1 of ", 80, "a", ")",
                            &length);
    test_input_add(data, length);

    data = test_nested_make("", ":is(", 140, ":lexbor-contains(abc)", "",
                            &length);
    test_input_add(data, length);
}

static void
test_inputs_destroy(void)
{
    size_t i;

    for (i = 0; i < test_inputs.length; i++) {
        lexbor_free(test_inputs.list[i].data);
        lexbor_free(test_inputs.list[i].result);
    }

    lexbor_free(test_inputs.list);
}

static bool
test_list_is(lxb_css_selector_list_t *list, const lxb_char_t *result)
{
    bool is;
    lxb_char_t *have;

    have = lxb_css_selector_serialize_list_chain_char(list, NULL);
    if (have == NULL || result == NULL) {
        lexbor_free(have);
        return false;
    }

    is = strcmp((const char *) have, (const char *) result) == 0;

    lexbor_free(have);

    return is;
}


TEST_BEGIN_ARGS(parse_input, test_input_t *input, bool all)
{
    size_t index, live;
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_selector_list_t *list;

    for (index = 0;; index++) {
        live = test_live;

        parser = lxb_css_parser_create();
        status = lxb_css_parser_init(parser, NULL);
        test_eq(status, LXB_STATUS_OK);

        test_arm(index, all);
        list = lxb_css_selectors_parse(parser, input->data, input->length);
        test_disarm();

        if (list != NULL) {
            test_eq(test_list_is(list, input->result), true);

            lxb_css_selector_list_destroy_memory(list);
        }

        (void) lxb_css_parser_destroy(parser, true);

        test_eq_size(test_live, live);

        if (!test_reached) {
            break;
        }
    }

    test_eq(list == NULL, input->result == NULL);
    test_ne_size(index, 0);
}
TEST_END

TEST_BEGIN_ARGS(parse_memory_input, test_input_t *input, bool all)
{
    size_t index, live;
    lxb_status_t status;
    lxb_css_parser_t *parser;
    lxb_css_memory_t *memory;
    lxb_css_selector_list_t *list;

    for (index = 0;; index++) {
        live = test_live;

        memory = lxb_css_memory_create();
        status = lxb_css_memory_init(memory, 128);
        test_eq(status, LXB_STATUS_OK);

        parser = lxb_css_parser_create();
        status = lxb_css_parser_init(parser, NULL);
        test_eq(status, LXB_STATUS_OK);

        lxb_css_parser_memory_set(parser, memory);

        test_arm(index, all);
        list = lxb_css_selectors_parse(parser, input->data, input->length);
        test_disarm();

        if (list != NULL) {
            test_eq(test_list_is(list, input->result), true);

            lxb_css_selector_list_destroy_chain(list);
            test_eq_size(lexbor_mraw_reference_count(memory->mraw), 0);
        }

        (void) lxb_css_parser_destroy(parser, true);
        (void) lxb_css_memory_destroy(memory, true);

        test_eq_size(test_live, live);

        if (!test_reached) {
            break;
        }
    }

    test_eq(list == NULL, input->result == NULL);
    test_ne_size(index, 0);
}
TEST_END

TEST_BEGIN(parse)
{
    size_t i;

    for (i = 0; i < test_inputs.length; i++) {
        TEST_CALL_ARGS(parse_input, &test_inputs.list[i], false);
        TEST_CALL_ARGS(parse_input, &test_inputs.list[i], true);
    }
}
TEST_END

TEST_BEGIN(parse_memory)
{
    size_t i;

    for (i = 0; i < test_inputs.length; i++) {
        TEST_CALL_ARGS(parse_memory_input, &test_inputs.list[i], false);
        TEST_CALL_ARGS(parse_memory_input, &test_inputs.list[i], true);
    }
}
TEST_END


int
main(int argc, const char *argv[])
{
    TEST_INIT();

    (void) lexbor_memory_setup(test_malloc, test_realloc, test_calloc,
                               test_free);

    test_inputs_init();

    TEST_ADD(parse);
    TEST_ADD(parse_memory);

    TEST_RUN("lexbor/css/selectors/allocation_failure");

    test_inputs_destroy();

    (void) lexbor_memory_setup(malloc, realloc, calloc, free);

    TEST_RELEASE();
}
