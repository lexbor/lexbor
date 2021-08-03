/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef TEST_H
#define TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include <lexbor/core/def.h>
#include <lexbor/core/array.h>
#include <lexbor/core/dobject.h>
#include <lexbor/core/array_obj.h>


#define TEST_PRINT(...)                                                        \
    do {                                                                       \
        printf(__VA_ARGS__);                                                   \
    }                                                                          \
    while (0)

#define TEST_PRINTLN(...)                                                      \
    do {                                                                       \
        printf(__VA_ARGS__);                                                   \
        printf("\n");                                                          \
    }                                                                          \
    while (0)


#define TEST_FAILURE(...)                                                      \
    do {                                                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
    while (0)

#define test_call_error()                                                      \
    do {                                                                       \
        TEST_OBJ_NAME->bad++;                                                  \
        TEST_OBJ_NAME->error = true;                                           \
                                                                               \
        fprintf(stdout, "Failure\n%s:%d:%s\n",                                 \
        __FILE__, __LINE__, __func__);                                         \
        TEST_STACK_PRINT();                                                    \
        return NULL;                                                           \
    }                                                                          \
    while (0)


#define TEST_RETVAL return_val
#define TEST_RETVAL_ARG void *return_val

#define TEST_OBJ_NAME my_test
#define TEST_OBJ_ARG test_t *TEST_OBJ_NAME


#define TEST_STACK_CLEAN() lexbor_array_obj_clean(TEST_OBJ_NAME->ao_stack)


#define TEST_STACK_PUSH()                                                      \
    do {                                                                       \
        test_stack_t *stack = lexbor_array_obj_push(TEST_OBJ_NAME->ao_stack);  \
        if (stack == NULL) {                                                   \
            TEST_FAILURE("Failed to allocate memory");                         \
        }                                                                      \
                                                                               \
        stack->file = __FILE__;                                                \
        stack->func = __func__;                                                \
        stack->line = __LINE__;                                                \
    }                                                                          \
    while (0)


#define TEST_STACK_PRINT()                                                     \
    do {                                                                       \
        test_stack_t *stack;                                                   \
        size_t i = lexbor_array_obj_length(TEST_OBJ_NAME->ao_stack);           \
                                                                               \
        while (i != 0) {                                                       \
            i--;                                                               \
            stack = (test_stack_t *) lexbor_array_obj_get(TEST_OBJ_NAME->ao_stack, i); \
            TEST_PRINTLN("%s:%d:%s",stack->file, stack->line, stack->func);    \
        }                                                                      \
    }                                                                          \
    while (0)


#define TEST_STACK_POP()                                                       \
    do {                                                                       \
        lexbor_array_obj_pop(TEST_OBJ_NAME->ao_stack);                         \
    }                                                                          \
    while (0)


#define TEST_CALL(name)                                                        \
    do {                                                                       \
        TEST_STACK_PUSH();                                                     \
        TEST_RETVAL = name(TEST_OBJ_ARG);                                      \
        TEST_STACK_POP();                                                      \
                                                                               \
        if (TEST_OBJ_NAME->error) {                                            \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    while (0)


#define TEST_CALL_ARGS(name, ...)                                              \
    do {                                                                       \
        TEST_STACK_PUSH();                                                     \
        TEST_RETVAL = name(TEST_OBJ_NAME, __VA_ARGS__);                        \
        TEST_STACK_POP();                                                      \
                                                                               \
        if (TEST_OBJ_NAME->error) {                                            \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    while (0)


#define TEST_BEGIN_ARGS(name, ...)                                             \
void *                                                                         \
name(TEST_OBJ_ARG, __VA_ARGS__);                                               \
void *                                                                         \
name(TEST_OBJ_ARG, __VA_ARGS__)                                                \
{                                                                              \
    TEST_RETVAL_ARG = NULL;                                                    \
    TEST_OBJ_NAME->error = false;                                              \
    (void) TEST_RETVAL;                                                        \


#define TEST_BEGIN(name)                                                       \
void *                                                                         \
name(TEST_OBJ_ARG);                                                            \
void *                                                                         \
name(TEST_OBJ_ARG)                                                             \
{                                                                              \
    TEST_RETVAL_ARG = NULL;                                                    \
    TEST_OBJ_NAME->error = false;                                              \
    (void) TEST_RETVAL;                                                        \


#define TEST_END                                                               \
    return NULL;                                                               \
}

#define TEST_END_RETVAL(return_val)                                            \
    return return_val;                                                         \
}


#define TEST_INIT()                                                            \
    TEST_OBJ_ARG;                                                              \
    do {                                                                       \
        TEST_OBJ_NAME = test_create();                                         \
        if (test_init(TEST_OBJ_NAME) != LXB_STATUS_OK) {                       \
            TEST_FAILURE("Failed to create Test object");                      \
        }                                                                      \
    }                                                                          \
    while (0)


#define TEST_ADD(test_name)                                                    \
    do {                                                                       \
        if (test_add(TEST_OBJ_NAME, test_name,                                 \
                     (char *) LEXBOR_STRINGIZE(test_name))                     \
            != LXB_STATUS_OK)                                                  \
        {                                                                      \
            TEST_FAILURE("Failed to add test to list");                        \
        }                                                                      \
    }                                                                          \
    while (0)


#define TEST_RUN(name)                                                         \
    test_run(TEST_OBJ_NAME, (char *) name)


#define TEST_RELEASE()                                                         \
    do {                                                                       \
        bool is_success = test_is_success(TEST_OBJ_NAME);                      \
        test_destroy(TEST_OBJ_NAME, true);                                     \
        return (is_success) ? EXIT_SUCCESS : EXIT_FAILURE;                     \
    }                                                                          \
    while (0)


#define test_eq(have, need)                                                    \
    do {                                                                       \
        if ((have) != (need)) {                                                \
            TEST_OBJ_NAME->bad++;                                              \
            TEST_OBJ_NAME->error = true;                                       \
                                                                               \
            fprintf(stdout, "Failure\n%s:%d:%s\n",                             \
                    __FILE__, __LINE__, __func__);                             \
            TEST_STACK_PRINT();                                                \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    while (0)


#define test_eq_type(have, need, type, act)                                    \
    do {                                                                       \
        if ((have) act (need)) {                                               \
            TEST_OBJ_NAME->bad++;                                              \
            TEST_OBJ_NAME->error = true;                                       \
                                                                               \
            fprintf(stdout, "Received '" type "', "                            \
                    "but expected '" type "'\n%s:%d:%s\n",                     \
                    (have), (need), __FILE__, __LINE__, __func__);             \
            TEST_STACK_PRINT();                                                \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    while (0)


#define test_eq_int(have, need) test_eq_type(have, need, "%d", !=)
#define test_eq_u_int(have, need) test_eq_type(have, need, "%u", !=)

#define test_eq_long(have, need) test_eq_type(have, need, "%l", !=)
#define test_eq_u_long(have, need) test_eq_type(have, need, "%lu", !=)

#define test_eq_size(have, need) test_eq_type(have, need, LEXBOR_FORMAT_Z, !=)
#define test_ne_size(have, need) test_eq_type(have, need, LEXBOR_FORMAT_Z, ==)

#define test_eq_double(have, need) test_eq_type(have, need, "%f", !=)
#define test_eq_float(have, need) test_eq_double((double) have, (double) need)

#define test_eq_short(have, need) test_eq_type((int) have, (int) need, "%d", !=)

#define test_eq_u_short(have, need)                                            \
    test_eq_type((unsigned int) have, (unsigned int) need, "%d", !=)

#define test_eq_char(have, need) test_eq_type(have, need, "%c", !=)
#define test_eq_u_char(have, need) test_eq_type(have, need, "%c", !=)

#define test_eq_bool(have, need)                                               \
    do {                                                                       \
        if ((have) != (need)) {                                                \
            TEST_OBJ_NAME->bad++;                                              \
            TEST_OBJ_NAME->error = true;                                       \
                                                                               \
            fprintf(stdout, "Received %s, "                                    \
                    "but expected %s\n%s:%d:%s\n",                             \
                    ((have) ? "True" : "False"), ((need) ? "True" : "False"),  \
                    __FILE__, __LINE__, __func__);                             \
            TEST_STACK_PRINT();                                                \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    while (0)

#define test_eq_str_n(have, hlen, need, nlen)                                  \
    do {                                                                       \
        if (hlen != nlen || memcmp((need), (have), (hlen)) != 0) {             \
            TEST_OBJ_NAME->bad++;                                              \
            TEST_OBJ_NAME->error = true;                                       \
                                                                               \
            fprintf(stdout, "Received:\n%.*s\n"                                \
                    "Expected:\n%.*s\n%s:%d:%s\n",                             \
                    (int) (hlen), (have), (int) (nlen), (need),                \
                    __FILE__, __LINE__, __func__);                             \
            TEST_STACK_PRINT();                                                \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    while (0)

#define test_ne_str_n(have, hlen, need, nlen)                                  \
    do {                                                                       \
        if (hlen == nlen && memcmp((need), (have), (hlen)) == 0) {             \
            TEST_OBJ_NAME->bad++;                                              \
            TEST_OBJ_NAME->error = true;                                       \
                                                                               \
            fprintf(stdout, "Received:\n%.*s\n"                                \
                    "Expected:\n%.*s\n%s:%d:%s\n",                             \
                    (int) (hlen), (char *) (have),                             \
                    (int) (nlen), (char *) (need),                             \
                    __FILE__, __LINE__, __func__);                             \
            TEST_STACK_PRINT();                                                \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    while (0)

#define test_eq_str(have, need)                                                \
    do {                                                                       \
        size_t hlen = strlen((const char *) (have));                           \
        size_t nlen = strlen((const char *) (need));                           \
        test_eq_str_n(have, hlen, need, nlen);                                 \
    }                                                                          \
    while (0)

#define test_eq_u_str_n(have, hlen, need, nlen)                                \
    test_eq_str_n(have, hlen, need, nlen)

#define test_eq_u_str(have, need)                                              \
    test_eq_str(have, need)

#define test_ne_str(have, need)                                                \
    do {                                                                       \
        size_t hlen = strlen((const char *) (have));                           \
        size_t nlen = strlen((const char *) (need));                           \
        test_ne_str_n(have, hlen, need, nlen);                                 \
    }                                                                          \
    while (0)

#define test_ne_u_str_n(have, hlen, need, nlen)                                \
    test_eq_str_n(have, hlen, need, nlen)

#define test_ne_u_str(have, need)                                              \
    test_ne_str(have, need)


#define test_ne(have, need)                                                    \
    do {                                                                       \
        if ((have) == (need)) {                                                \
            TEST_OBJ_NAME->bad++;                                              \
            TEST_OBJ_NAME->error = true;                                       \
                                                                               \
            fprintf(stdout, "Failure\n%s:%d:%s\n",                             \
                    __FILE__, __LINE__, __func__);                             \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    while (0)

/* > */
#define test_gt(have, need)                                                    \
    do {                                                                       \
        if ((have) < (need)) {                                                 \
            TEST_OBJ_NAME->bad++;                                              \
            TEST_OBJ_NAME->error = true;                                       \
                                                                               \
            fprintf(stdout, "Failure\n%s:%d:%s\n",                             \
                    __FILE__, __LINE__, __func__);                             \
            return NULL;                                                       \
        }                                                                      \
    }                                                                          \
    while (0)


typedef struct {
    lexbor_array_t     *list;
    lexbor_dobject_t   *entries;
    lexbor_array_obj_t *ao_stack;

    size_t             bad;
    bool               error;
}
test_t;

typedef void * (*test_func_t)(test_t *test);

typedef struct {
    test_func_t func;
    char        *name;
}
test_entry_t;

typedef struct {
    const char *file;
    const char *func;
    int        line;
}
test_stack_t;


LXB_API test_t *
test_create(void);

LXB_API lxb_status_t
test_init(test_t *test);

LXB_API void
test_clean(test_t *test);

LXB_API test_t *
test_destroy(test_t *test, bool self_destroy);


LXB_API lxb_status_t
test_add(test_t *test, test_func_t test_func, char *test_name);

LXB_API void
test_run(test_t *test, char *name);

LXB_API bool
test_is_success(test_t *test);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TEST_H */
