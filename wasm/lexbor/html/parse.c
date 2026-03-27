/*
 * WASM API for HTML parsing with lexbor.
 *
 * Build: emcmake cmake .. -DLEXBOR_BUILD_WASM=ON -DLEXBOR_BUILD_STATIC=ON
 */

#include <string.h>

#include <lexbor/html/parser.h>
#include <lexbor/html/serialize.h>

#include <emscripten/emscripten.h>


static char   result_buf[1024 * 1024];
static size_t result_len;


static lxb_status_t
serializer_callback(const lxb_char_t *data, size_t len, void *ctx)
{
    if (result_len + len < sizeof(result_buf)) {
        memcpy(result_buf + result_len, data, len);
        result_len += len;
    }

    return LXB_STATUS_OK;
}

EMSCRIPTEN_KEEPALIVE
const char *
parse_html(const lxb_char_t *html, size_t len)
{
    lxb_status_t status;
    lxb_html_document_t *doc;

    doc = lxb_html_document_create();
    if (doc == NULL) {
        return "";
    }

    status = lxb_html_document_parse(doc, html, len);
    if (status != LXB_STATUS_OK) {
        lxb_html_document_destroy(doc);
        return "";
    }

    result_len = 0;

    lxb_html_serialize_tree_cb(lxb_dom_interface_node(doc),
                               serializer_callback, NULL);

    result_buf[result_len] = '\0';

    lxb_html_document_destroy(doc);

    return result_buf;
}
