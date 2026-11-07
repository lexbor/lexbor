/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Stephane Janel <https://github.com/sjanel>
 */

#include <lexbor/html/interfaces/document.h>

#include <unit/test.h>

/* Index of the allocation to fail, and number of allocations made so far. */
static size_t test_fail_at;
static size_t test_count;

static const lxb_char_t test_html[] =
    "<!doctype html><html><head><title>t</title></head><body>"
    "<p class=a>x<b>y</b></p><div id=d><span>z</span>"
    "<table><tr><td>1</td></tr></table></div></body></html>";

static bool test_fail(void) { return test_count++ == test_fail_at; }

static void *test_malloc(size_t size) {
  return test_fail() ? NULL : malloc(size);
}

static void *test_realloc(void *dst, size_t size) {
  return test_fail() ? NULL : realloc(dst, size);
}

static void *test_calloc(size_t num, size_t size) {
  return test_fail() ? NULL : calloc(num, size);
}

static void test_fail_allocation(size_t index) {
  test_count = 0;
  test_fail_at = index;

  (void)lexbor_memory_setup(test_malloc, test_realloc, test_calloc, free);
}

/* Restores the allocator, and tells if the allocation to fail was reached. */
static bool test_restore_allocator(void) {
  (void)lexbor_memory_setup(malloc, realloc, calloc, free);

  return test_count > test_fail_at;
}

TEST_BEGIN(document_create) {
  size_t index;
  bool failed;
  lxb_html_document_t *document;

  /* Fails each allocation of the document creation in turn. */
  for (index = 0;; index++) {
    test_fail_allocation(index);
    document = lxb_html_document_create();
    failed = test_restore_allocator();

    if (!failed) {
      break;
    }

    test_eq(document, NULL);
  }

  test_ne(document, NULL);
  test_eq(index > 1, true);

  lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(document_parse) {
  size_t index;
  bool failed;
  lxb_status_t status;
  lxb_html_document_t *document;

  /* Fails each allocation of the parsing in turn. */
  for (index = 0;; index++) {
    document = lxb_html_document_create();
    test_ne(document, NULL);

    test_fail_allocation(index);
    status =
        lxb_html_document_parse(document, test_html, sizeof(test_html) - 1);
    failed = test_restore_allocator();

    if (!failed) {
      break;
    }

    /* Some allocations are optional (caches with a fallback). */
    if (status == LXB_STATUS_OK) {
      test_ne(lxb_html_document_body_element(document), NULL);
    }

    lxb_html_document_destroy(document);
  }

  test_eq(status, LXB_STATUS_OK);
  test_ne(lxb_html_document_body_element(document), NULL);
  test_eq(index > 1, true);

  lxb_html_document_destroy(document);
}
TEST_END

int main(int argc, const char *argv[]) {
  TEST_INIT();

  TEST_ADD(document_create);
  TEST_ADD(document_parse);

  TEST_RUN("lexbor/html/allocation_failure");
  TEST_RELEASE();
}
