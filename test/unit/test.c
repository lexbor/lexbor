/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "unit/test.h"

test_t *
test_create(void)
{
    return lexbor_calloc(1, sizeof(test_t));
}

lxb_status_t
test_init(test_t *test)
{
    lxb_status_t status;

    if (test == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    /* List */
    test->list = lexbor_array_create();
    status = lexbor_array_init(test->list, 1024);

    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Entries */
    test->entries = lexbor_dobject_create();
    status = lexbor_dobject_init(test->entries, 1024, sizeof(test_entry_t));

    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Stack */
    test->ao_stack = lexbor_array_obj_create();
    status = lexbor_array_obj_init(test->ao_stack, 256, sizeof(test_stack_t));

    if (status != LXB_STATUS_OK) {
        return status;
    }

    test->bad = 0;
    test->error = false;

    return LXB_STATUS_OK;
}

void
test_clean(test_t *test)
{
    test->bad = 0;
    test->error = false;

    lexbor_array_clean(test->list);
    lexbor_dobject_clean(test->entries);
    lexbor_array_obj_clean(test->ao_stack);
}

test_t *
test_destroy(test_t *test, bool self_destroy)
{
    if (test == NULL) {
        return NULL;
    }

    test->list = lexbor_array_destroy(test->list, true);
    test->entries = lexbor_dobject_destroy(test->entries, true);
    test->ao_stack = lexbor_array_obj_destroy(test->ao_stack, true);

    if (self_destroy) {
        return lexbor_free(test);
    }

    return test;
}

lxb_status_t
test_add(test_t *test, test_func_t test_func, char *test_name)
{
    test_entry_t *entry = lexbor_dobject_alloc(test->entries);

    if (entry == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    entry->func = test_func;
    entry->name = test_name;

    return lexbor_array_push(test->list, entry);
}

void
test_run(test_t *test, char *name)
{
    size_t bad_tmp;
    test_entry_t *entry;
    lexbor_array_t *list = test->list;

    printf("Run '%s' tests:\n", name);

    for (size_t i = 0; i < lexbor_array_length(list); i++) {
        lexbor_array_obj_clean(test->ao_stack);
        entry = lexbor_array_get(list, i);

        fprintf(stdout, LEXBOR_FORMAT_Z ") %s: ", (i + 1), entry->name);

        bad_tmp = test->bad;
        entry->func(test);

        if (bad_tmp == test->bad) {
            fprintf(stdout, "ok\n");
        }
    }

    fprintf(stdout,
            "Failed: " LEXBOR_FORMAT_Z "\n"
            "Total: " LEXBOR_FORMAT_Z "\n",
            test->bad, lexbor_array_length(list));
}

bool
test_is_success(test_t *test)
{
    return test->bad == 0;
}
