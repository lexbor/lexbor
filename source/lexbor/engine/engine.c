/*
 * Copyright (C) 2024-2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/engine/engine.h"


static lxb_status_t
lxb_engine_html_parse_cb(const lxb_char_t *data, size_t len, void *ctx);


typedef struct {
    lxb_char_t *data;
    size_t     length;
    size_t     size;
}
lxb_engine_str_context_t;


lxb_engine_t *
lxb_engine_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_engine_t));
}

lxb_status_t
lxb_engine_init(lxb_engine_t *engine)
{
    lxb_status_t status;

    if (engine == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    engine->document = lxb_html_document_create();
    if (engine->document == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    status = lxb_html_document_css_init(engine->document, true);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    engine->html_encoding = NULL;

    return LXB_STATUS_OK;
}

lxb_engine_t *
lxb_engine_destroy(lxb_engine_t *engine)
{
    if (engine == NULL) {
        return NULL;
    }

    if (engine->document != NULL) {
        lxb_html_document_css_destroy(engine->document);
        engine->document = lxb_html_document_destroy(engine->document);
    }

    if (engine->html_encoding != NULL) {
        engine->html_encoding = lxb_html_encoding_destroy(engine->html_encoding,
                                                          true);
    }

    return lexbor_free(engine);
}

lxb_status_t
lxb_engine_parse(lxb_engine_t *engine, const lxb_char_t *html, size_t length,
                 lxb_encoding_t encoding)
{
    lxb_status_t status;
    lxb_html_document_t *doc;

    doc = engine->document;

    if (encoding == LXB_ENCODING_AUTO) {
        encoding = lxb_engine_encoding_from_meta(engine, html, length);
    }

    if (encoding != LXB_ENCODING_UTF_8 && encoding > LXB_ENCODING_UNDEFINED
        && encoding < LXB_ENCODING_LAST_ENTRY)
    {
        status = lxb_html_document_parse_chunk_begin(doc);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        status = lxb_engine_encoding_from_to(html, length, encoding,
                                             LXB_ENCODING_UTF_8,
                                             lxb_engine_html_parse_cb, doc);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        status = lxb_html_document_parse_chunk_end(doc);
    }
    else {
        if (encoding == LXB_ENCODING_UTF_8) {
            lxb_encoding_utf_8_skip_bom(&html, &length);
        }

        status = lxb_html_document_parse(doc, html, length);
    }

    return status;
}

static lxb_status_t
lxb_engine_html_parse_cb(const lxb_char_t *data, size_t len, void *ctx)
{
    return lxb_html_document_parse_chunk(ctx, data, len);
}

lxb_status_t
lxb_engine_encoding_from_to(const lxb_char_t *data, size_t length,
                            lxb_encoding_t from, lxb_encoding_t to,
                            lexbor_serialize_cb_f cb, void *ctx)
{
    size_t buf_length;
    lxb_status_t status, en_status, de_status;
    const lxb_char_t *end;
    const lxb_codepoint_t *cp_begin, *cp_end;
    lxb_encoding_decode_t decode;
    const lxb_encoding_data_t *decoder;
    lxb_encoding_encode_t encode;
    const lxb_encoding_data_t *encoder;
    lxb_codepoint_t cp[4096];
    lxb_char_t outbuf[4096];

    /* Skip BOM. */

    switch (from) {
        case LXB_ENCODING_UTF_8:
            lxb_encoding_utf_8_skip_bom(&data, &length);
            break;

        case LXB_ENCODING_UTF_16BE:
            lxb_encoding_utf_16be_skip_bom(&data, &length);
            break;

        case LXB_ENCODING_UTF_16LE:
            lxb_encoding_utf_16le_skip_bom(&data, &length);
            break;

        default:
            break;
    }

    /* Decode. */

    decoder = lxb_encoding_data(from);
    if (decoder == NULL) {
        return LXB_STATUS_ERROR_NOT_EXISTS;
    }

    status = lxb_encoding_decode_init(&decode, decoder, cp,
                                      sizeof(cp) / sizeof(lxb_codepoint_t));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = lxb_encoding_decode_replace_set(&decode,
                                             LXB_ENCODING_REPLACEMENT_BUFFER,
                                             LXB_ENCODING_REPLACEMENT_BUFFER_LEN);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Encode. */

    encoder = lxb_encoding_data(to);
    if (encoder == NULL) {
        return LXB_STATUS_ERROR_NOT_EXISTS;
    }

    status = lxb_encoding_encode_init(&encode, encoder, outbuf, sizeof(outbuf));
    if (status != LXB_STATUS_OK) {
        return status;
    }

    status = lxb_encoding_encode_replace_set(&encode,
                                             LXB_ENCODING_REPLACEMENT_BYTES,
                                             LXB_ENCODING_REPLACEMENT_SIZE);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* Process. */

    end = data + length;

    do {
        de_status = decoder->decode(&decode, &data, end);

        cp_begin = cp;
        cp_end = cp_begin + lxb_encoding_decode_buf_used(&decode);

        do {
            en_status = encoder->encode(&encode, &cp_begin, cp_end);

            buf_length = lxb_encoding_encode_buf_used(&encode);

            status = cb(outbuf, buf_length, ctx);
            if (status != LXB_STATUS_OK) {
                return status;
            }

            lxb_encoding_encode_buf_used_set(&encode, 0);
        }
        while (en_status == LXB_STATUS_SMALL_BUFFER);

        lxb_encoding_decode_buf_used_set(&decode, 0);
    }
    while (de_status == LXB_STATUS_SMALL_BUFFER);

    /* Finish. */

    (void) lxb_encoding_decode_finish(&decode);

    if (lxb_encoding_decode_buf_used(&decode)) {
        cp_begin = cp;
        cp_end = cp_begin + lxb_encoding_decode_buf_used(&decode);

        (void) encoder->encode(&encode, &cp_begin, cp_end);

        buf_length = lxb_encoding_encode_buf_used(&encode);

        status = cb(outbuf, buf_length, ctx);
        if (status != LXB_STATUS_OK) {
            return status;
        }

        lxb_encoding_encode_buf_used_set(&encode, 0);
    }

    (void) lxb_encoding_encode_finish(&encode);

    buf_length = lxb_encoding_encode_buf_used(&encode);

    if (buf_length != 0) {
        return cb(outbuf, buf_length, ctx);
    }

    return LXB_STATUS_OK;
}

lxb_encoding_t
lxb_engine_encoding_from_meta(lxb_engine_t *engine, const lxb_char_t *html,
                              size_t length)
{
    size_t i;
    lxb_status_t status;
    const lxb_char_t *end;
    lexbor_array_obj_t *enc_html;
    const lxb_encoding_data_t *data;
    lxb_html_encoding_entry_t *entry;

    if (engine->html_encoding == NULL) {
        engine->html_encoding = lxb_html_encoding_create();
        status = lxb_html_encoding_init(engine->html_encoding);
        if (status != LXB_STATUS_OK) {
            return LXB_ENCODING_UNDEFINED;
        }
    }
    else {
        lxb_html_encoding_clean(engine->html_encoding);
    }

    end = (length >= 2048) ? html + 2048 : html + length;

    status = lxb_html_encoding_determine(engine->html_encoding, html, end);
    if (status != LXB_STATUS_OK) {
        return LXB_ENCODING_UNDEFINED;
    }

    enc_html = lxb_html_encoding_meta_result(engine->html_encoding);

    for (i = 0; i < enc_html->length; i++) {
        entry = lexbor_array_obj_get(enc_html, i);

        data = lxb_encoding_data_by_pre_name(entry->name,
                                             entry->end - entry->name);
        if (data != NULL) {
            return data->encoding;
        }
    }

    return LXB_ENCODING_UNDEFINED;
}
