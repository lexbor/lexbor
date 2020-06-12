#include <lexbor/html/html.h>


#ifndef LEXBOR_HAVE_FUZZER
int
main(int argc, const char *argv[])
{
    bool loop;
    size_t size;
    lxb_status_t status;
    lxb_html_document_t *document;
    char inbuf[4096];

    document = lxb_html_document_create();
    if (document == NULL) {
        return EXIT_FAILURE;
    }

    status = lxb_html_document_parse_chunk_begin(document);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    loop = true;

    do {
        size = fread(inbuf, 1, sizeof(inbuf), stdin);
        if (size != sizeof(inbuf)) {
            if (feof(stdin)) {
                loop = false;
            }
            else {
                return EXIT_FAILURE;
            }
        }

        status = lxb_html_document_parse_chunk(document,
                                              (const lxb_char_t *) inbuf, size);
        if (status != LXB_STATUS_OK) {
            return EXIT_FAILURE;
        }
    }
    while (loop);

    status = lxb_html_document_parse_chunk_end(document);
    if (status != LXB_STATUS_OK) {
        return EXIT_FAILURE;
    }

    lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
#endif

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
