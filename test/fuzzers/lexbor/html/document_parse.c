#include <lexbor/html/html.h>


int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t length)
{
    lxb_status_t status;
    lxb_html_document_t *document;

    document = lxb_html_document_create();
    if (document == NULL) {
        return EXIT_FAILURE;
    }

    status = lxb_html_document_parse(document, data, length);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
