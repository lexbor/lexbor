/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <math.h>
#include <inttypes.h>

#include "lexbor/unicode/unicode.h"
#include "lexbor/unicode/tables.h"
#include "lexbor/encoding/encoding.h"


/* Hangul syllables for modern Korean. */
static const lxb_codepoint_t lxb_unicode_sb = 0xAC00;
static const lxb_codepoint_t lxb_unicode_sl = 0xD7A3;
/* Hangul vowels (syllable nucleuses). */
static const lxb_codepoint_t lxb_unicode_lb = 0x1100;
static const lxb_codepoint_t lxb_unicode_ll = 0x1112;
/* Hangul vowels (syllable nucleuses). */
static const lxb_codepoint_t lxb_unicode_vb = 0x1161;
static const lxb_codepoint_t lxb_unicode_vl = 0x1175;
/* Hangul trailing consonants (syllable codas). */
static const lxb_codepoint_t lxb_unicode_tb = 0x11A8;
static const lxb_codepoint_t lxb_unicode_tl = 0x11C2;
static const lxb_codepoint_t lxb_unicode_ts = 0x11A7;
static const lxb_codepoint_t lxb_unicode_vc = lxb_unicode_vl - lxb_unicode_vb + 1;
static const lxb_codepoint_t lxb_unicode_tc = lxb_unicode_tl - lxb_unicode_ts + 1;


static void
lxb_unicode_canonical(lxb_unicode_buffer_t *starter, lxb_unicode_buffer_t *op,
                      lxb_unicode_buffer_t *p);

static void
lxb_unicode_compatibility(lxb_unicode_buffer_t *starter,
                          lxb_unicode_buffer_t *op, lxb_unicode_buffer_t *p);

static void
lxb_unicode_canonical_composition(lxb_unicode_buffer_t *p,
                                  const lxb_unicode_buffer_t *end);

static lxb_unicode_buffer_t *
lxb_unicode_canonical_decomposition(lxb_unicode_normalizer_t *uc,
                                    lxb_codepoint_t cp,
                                    lxb_unicode_buffer_t **buf,
                                    const lxb_unicode_buffer_t **end);

static lxb_unicode_buffer_t *
lxb_unicode_compatibility_decomposition(lxb_unicode_normalizer_t *uc,
                                        lxb_codepoint_t cp,
                                        lxb_unicode_buffer_t **buf,
                                        const lxb_unicode_buffer_t **end);

static lxb_unicode_buffer_t *
lxb_unicode_entry_decomposition_hangul(lxb_unicode_normalizer_t *uc,
                                       lxb_unicode_buffer_t **buf,
                                       const lxb_unicode_buffer_t **end,
                                       lxb_codepoint_t cp);

static lxb_codepoint_t
lxb_unicode_entry_compose_hangul(lxb_codepoint_t first, lxb_codepoint_t second);


lxb_unicode_normalizer_t *
lxb_unicode_normalizer_create(void)
{
    return lexbor_malloc(sizeof(lxb_unicode_normalizer_t));
}

lxb_status_t
lxb_unicode_normalizer_init(lxb_unicode_normalizer_t *uc,
                            lxb_unicode_form_t form)
{
    lxb_status_t status;
    static const size_t buf_length = 4096;

    if (uc == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    status = lxb_unicode_normalization_form_set(uc, form);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    uc->tmp_lenght = 0;
    uc->starter = NULL;

    uc->buf = lexbor_malloc(buf_length * sizeof(lxb_unicode_buffer_t));
    if (uc->buf == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    uc->end = uc->buf + buf_length;
    uc->p = uc->buf;
    uc->ican = uc->buf;
    uc->quick_ccc = 0;
    uc->flush_cp = 1024;

    return LXB_STATUS_OK;
}

void
lxb_unicode_normalizer_clean(lxb_unicode_normalizer_t *uc)
{
    uc->tmp_lenght = 0;
    uc->starter = NULL;
    uc->p = uc->buf;
    uc->ican = uc->buf;
    uc->quick_ccc = 0;
}

lxb_unicode_normalizer_t *
lxb_unicode_normalizer_destroy(lxb_unicode_normalizer_t *uc, bool self_destroy)
{
    if (uc == NULL) {
        return NULL;
    }

    if (uc->buf != NULL) {
        uc->buf = lexbor_free(uc->buf);
    }

    if (self_destroy) {
        return lexbor_free(uc);
    }

    return uc;
}

lxb_status_t
lxb_unicode_normalization_form_set(lxb_unicode_normalizer_t *uc,
                                   lxb_unicode_form_t form)
{
    switch (form) {
        case LXB_UNICODE_NFC:
            uc->decomposition = lxb_unicode_canonical_decomposition;
            uc->composition = lxb_unicode_canonical;
            uc->quick_type = LXB_UNICODE_NFC_QUICK_NO|LXB_UNICODE_NFC_QUICK_MAYBE;
            break;

        case LXB_UNICODE_NFD:
            uc->decomposition = lxb_unicode_canonical_decomposition;
            uc->composition = lxb_unicode_compatibility;
            uc->quick_type = LXB_UNICODE_NFD_QUICK_NO;
            break;

        case LXB_UNICODE_NFKC:
            uc->decomposition = lxb_unicode_compatibility_decomposition;
            uc->composition = lxb_unicode_canonical;
            uc->quick_type = LXB_UNICODE_NFKC_QUICK_NO|LXB_UNICODE_NFKC_QUICK_MAYBE;
            break;

        case LXB_UNICODE_NFKD:
            uc->decomposition = lxb_unicode_compatibility_decomposition;
            uc->composition = lxb_unicode_compatibility;
            uc->quick_type = LXB_UNICODE_NFKD_QUICK_NO;
            break;

        default:
            return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_unicode_flush(lxb_unicode_normalizer_t *uc, lexbor_serialize_cb_f cb,
                  void *ctx)
{
    int8_t res;
    lxb_char_t *tmp;
    lxb_status_t status;
    lxb_unicode_buffer_t *p, *end;
    lxb_char_t buffer[4096];
    const lxb_char_t *buffer_end = buffer + sizeof(buffer);

    p = uc->buf;
    end = uc->ican;
    tmp = buffer;

    while (p < end) {
        if (p->cp != LXB_ENCODING_ERROR_CODEPOINT) {
            res = lxb_encoding_encode_utf_8_single(NULL, &tmp, buffer_end,
                                                   p->cp);
            if (res == LXB_ENCODING_ENCODE_SMALL_BUFFER) {
                status = cb(buffer, tmp - buffer, ctx);
                if (status != LXB_STATUS_OK) {
                    return status;
                }

                tmp = buffer;

                continue;
            }
        }

        p += 1;
    }

    if (tmp != buffer) {
        return cb(buffer, tmp - buffer, ctx);
    }

    return LXB_STATUS_OK;
}

lxb_inline void
lxb_unicode_check_buf(lxb_unicode_normalizer_t *uc, lxb_unicode_buffer_t **p,
                      const lxb_unicode_buffer_t **end, size_t length)
{
    size_t len, new_len, starter_len;
    lxb_unicode_buffer_t *buf;

    static const size_t buf_length = 1024;

    if (*p + length >= *end) {
        len = *p - uc->buf;
        new_len = (uc->end - uc->buf) + buf_length + length;
        starter_len = (uc->starter != NULL) ? uc->starter - uc->buf : 0;

        buf = lexbor_realloc(uc->buf, new_len * sizeof(lxb_unicode_buffer_t));
        if (buf == NULL) {
            *p = NULL;
        }

        if (uc->starter != NULL) {
            uc->starter = buf + starter_len;
        }

        uc->buf = buf;
        uc->end = buf + new_len;

        *p = buf + len;
        *end = uc->end;
    }
}

lxb_inline void
lxb_unicode_reorder(lxb_unicode_buffer_t *p, lxb_unicode_buffer_t *starter)
{
    lxb_unicode_buffer_t swap;
    lxb_unicode_buffer_t *end = p;

    while (p > starter) {
        if (p[-1].ccc > p->ccc) {
            swap = *p;

            *p = p[-1];
            p[-1] = swap;

            if (p < end) {
                p += 1;
                continue;
            }
        }

        p -= 1;
    }
}

lxb_inline const lxb_char_t *
lxb_unicode_restore(lxb_unicode_normalizer_t *uc, const lxb_char_t *data,
                    const lxb_char_t *end, lxb_codepoint_t *cp, bool is_last)
{
    size_t i, len;
    lxb_char_t *tmp;

    tmp = uc->tmp;
    len = uc->tmp_lenght;

    i = lxb_encoding_decode_utf_8_length(tmp[0]);

    while (len < i && data < end) {
        tmp[ len++ ] = *data;
        data += 1;
    }

    *cp = lxb_encoding_decode_valid_utf_8_single((const lxb_char_t **) &tmp,
                                                 tmp + i);
    if (*cp == LXB_ENCODING_DECODE_ERROR) {
        if (!is_last) {
            uc->tmp_lenght = len;
            return NULL;
        }

        *cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
    }

    uc->tmp_lenght = 0;

    return data;
}

lxb_status_t
lxb_unicode_normalize(lxb_unicode_normalizer_t *uc, const lxb_char_t *data,
                      size_t length, lexbor_serialize_cb_f cb, void *ctx,
                      bool is_last)
{
    lxb_status_t status;
    lxb_codepoint_t cp;
    const lxb_char_t *end, *tp;
    lxb_unicode_buffer_t *p, *dp, *op, *buf;
    const lxb_unicode_buffer_t *buf_end;

    end = data + length;

    buf_end = uc->end;
    p = uc->p;

    if (uc->tmp_lenght != 0) {
        data = lxb_unicode_restore(uc, data, end, &cp, is_last);
        if (data == NULL) {
            return LXB_STATUS_OK;
        }

        goto restore;
    }

    while (data < end) {
        tp = data;

        cp = lxb_encoding_decode_valid_utf_8_single(&data, end);
        if (cp == LXB_ENCODING_DECODE_ERROR) {
            if (data >= end && !is_last) {
                uc->p = p;
                uc->tmp_lenght = end - tp;

                memcpy(uc->tmp, tp, uc->tmp_lenght);

                return LXB_STATUS_OK;
            }

            cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
        }

    restore:

        dp = uc->decomposition(uc, cp, &p, &buf_end);

        while (p < dp) {
            if (p->ccc == 0) {
                op = p - 1;
                buf = uc->buf;

                if (uc->starter == NULL) {
                    lxb_unicode_reorder(op, buf);

                    uc->starter = p++;
                    continue;
                }

                uc->composition(uc->starter, op, p + 1);

                if (p->cp != LXB_ENCODING_ERROR_CODEPOINT) {
                    uc->starter = p;
                    uc->ican = p;

                    if (p - buf >= uc->flush_cp) {
                        status = lxb_unicode_flush(uc, cb, ctx);
                        if (status != LXB_STATUS_OK) {
                            return status;
                        }

                        buf->cp = p->cp;
                        buf->ccc = p->ccc;

                        dp = buf + (dp - p);
                        p = buf;

                        uc->starter = p;
                        uc->ican = p;
                    }
                }
            }

            p += 1;
        }
    }

    status = LXB_STATUS_OK;

    if (is_last) {
        if (uc->starter != NULL && uc->starter != p - 1) {
            uc->composition(uc->starter, p - 1, p);
        }

        uc->ican = p;

        status = lxb_unicode_flush(uc, cb, ctx);

        uc->p = uc->buf;
        uc->ican = uc->buf;
        uc->starter = NULL;
    }
    else {
        uc->p = p;
    }

    return status;
}

lxb_status_t
lxb_unicode_normalize_end(lxb_unicode_normalizer_t *uc,
                          lexbor_serialize_cb_f cb, void *ctx)
{
    return lxb_unicode_normalize(uc, NULL, 0, cb, ctx, true);
}

bool
lxb_unicode_quick_check(lxb_unicode_normalizer_t *uc, const lxb_char_t *data,
                        size_t length, bool is_last)
{
    lxb_codepoint_t cp;
    const lxb_char_t *end, *tp;
    const lxb_unicode_entry_t *entry;

    end = data + length;

    if (uc->tmp_lenght != 0) {
        data = lxb_unicode_restore(uc, data, end, &cp, is_last);
        if (data == NULL) {
            return LXB_STATUS_OK;
        }

        goto restore;
    }

    while (data < end) {
        tp = data;

        cp = lxb_encoding_decode_valid_utf_8_single(&data, end);
        if (cp == LXB_ENCODING_DECODE_ERROR) {
            if (data >= end && !is_last) {
                uc->tmp_lenght = end - tp;

                memcpy(uc->tmp, tp, uc->tmp_lenght);

                return LXB_STATUS_OK;
            }

            cp = LXB_ENCODING_REPLACEMENT_CODEPOINT;
        }

    restore:

        entry = lxb_unicode_entry(cp);

        if (entry != NULL) {
            if (entry->quick & uc->quick_type) {
                goto ok_true;
            }

            if (entry->ccc < uc->quick_ccc) {
                goto ok_true;
            }

            uc->quick_ccc = entry->ccc;
        }
        else if (uc->quick_type & (LXB_UNICODE_NFD_QUICK_NO|LXB_UNICODE_NFKD_QUICK_NO)
                 && cp >= lxb_unicode_sb && cp <= lxb_unicode_sl)
        {
            goto ok_true;
        }
    }

    if (is_last) {
        uc->quick_ccc = 0;
    }

    return false;

ok_true:

    uc->quick_ccc = 0;

    return true;
}

bool
lxb_unicode_quick_check_end(lxb_unicode_normalizer_t *uc)
{
    return lxb_unicode_quick_check(uc, NULL, 0, true);
}

static void
lxb_unicode_canonical(lxb_unicode_buffer_t *starter, lxb_unicode_buffer_t *op,
                      lxb_unicode_buffer_t *p)
{
    lxb_unicode_reorder(op, starter);
    lxb_unicode_canonical_composition(starter, p);
}

static void
lxb_unicode_compatibility(lxb_unicode_buffer_t *starter,
                          lxb_unicode_buffer_t *op, lxb_unicode_buffer_t *p)
{
    (void) p;
    lxb_unicode_reorder(op, starter);
}

static void
lxb_unicode_canonical_composition(lxb_unicode_buffer_t *p,
                                  const lxb_unicode_buffer_t *end)
{
    lxb_codepoint_t cp;
    lxb_unicode_buffer_t *starter;
    const lxb_unicode_entry_t *entry;
    const lxb_unicode_compose_entry_t *centry;

    /* p is a starter. */

    starter = p++;

    while (p < end) {
        if (p->cp == LXB_ENCODING_ERROR_CODEPOINT) {
            p += 1;
            continue;
        }

        if (p[-1].ccc != 0 && p[-1].ccc >= p->ccc) {
            p += 1;
            continue;
        }

        centry = lxb_unicode_compose_entry(starter->cp, p->cp);

        if (centry != NULL) {
            if (!centry->exclusion) {
                entry = lxb_unicode_entry(centry->cp);

                starter->cp = entry->cp;
                starter->ccc = entry->ccc;

                p->cp = LXB_ENCODING_ERROR_CODEPOINT;
                p->ccc = 0;
            }
        }
        else {
            cp = lxb_unicode_entry_compose_hangul(starter->cp, p->cp);

            if (cp != LXB_ENCODING_ERROR_CODEPOINT) {
                starter->cp = cp;
                starter->ccc = 0;

                p->cp = LXB_ENCODING_ERROR_CODEPOINT;
                p->ccc = 0;
            }
        }

        p += 1;
    }
}

static lxb_unicode_buffer_t *
lxb_unicode_canonical_decomposition(lxb_unicode_normalizer_t *uc,
                                    lxb_codepoint_t cp,
                                    lxb_unicode_buffer_t **buf,
                                    const lxb_unicode_buffer_t **end)
{
    size_t i, length;
    lxb_unicode_buffer_t *p;
    const lxb_codepoint_t *mapping;
    const lxb_unicode_entry_t *entry;
    const lxb_unicode_decomposition_t *cde;

    entry = lxb_unicode_entry(cp);

    if (entry != NULL && entry->cde != NULL
        && entry->cde->type == LXB_UNICODE_DECOMPOSITION_TYPE__UNDEF)
    {
        cde = entry->cde;

        length = cde->length;
        mapping = cde->mapping;

        lxb_unicode_check_buf(uc, buf, end, length);
        if (*buf == NULL) {
            return NULL;
        }

        p = *buf;

        for (i = 0; i < length; i++) {
            entry = lxb_unicode_entry(mapping[i]);

            p->cp = mapping[i];
            p->ccc = (entry != NULL) ? entry->ccc : 0;

            p += 1;
        }
    }
    else if (cp >= lxb_unicode_sb && cp <= lxb_unicode_sl) {
        return lxb_unicode_entry_decomposition_hangul(uc, buf, end, cp);
    }
    else {
        lxb_unicode_check_buf(uc, buf, end, 1);
        if (*buf == NULL) {
            return NULL;
        }

        p = *buf;

        if (entry != NULL) {
            p->cp = entry->cp;
            p->ccc = entry->ccc;
        }
        else {
            p->cp = cp;
            p->ccc = 0;
        }

        p += 1;
    }

    return p;
}

static lxb_unicode_buffer_t *
lxb_unicode_compatibility_decomposition(lxb_unicode_normalizer_t *uc,
                                        lxb_codepoint_t cp,
                                        lxb_unicode_buffer_t **buf,
                                        const lxb_unicode_buffer_t **end)
{
    size_t i, length;
    lxb_unicode_buffer_t *p;
    const lxb_codepoint_t *mapping;
    const lxb_unicode_entry_t *entry;
    const lxb_unicode_decomposition_t *kde;

    entry = lxb_unicode_entry(cp);

    if (entry != NULL && entry->kde != NULL) {
        kde = entry->kde;

        length = kde->length;
        mapping = kde->mapping;

        lxb_unicode_check_buf(uc, buf, end, length);
        if (*buf == NULL) {
            return NULL;
        }

        p = *buf;

        for (i = 0; i < length; i++) {
            entry = lxb_unicode_entry(mapping[i]);

            p->cp = mapping[i];
            p->ccc = (entry != NULL) ? entry->ccc : 0;

            p += 1;
        }
    }
    else if (cp >= lxb_unicode_sb && cp <= lxb_unicode_sl) {
        return lxb_unicode_entry_decomposition_hangul(uc, buf, end, cp);
    }
    else {
        lxb_unicode_check_buf(uc, buf, end, 1);
        if (*buf == NULL) {
            return NULL;
        }

        p = *buf;

        p->cp = entry->cp;
        p->ccc = entry->ccc;

        p += 1;
    }

    return p;
}

static lxb_unicode_buffer_t *
lxb_unicode_entry_decomposition_hangul(lxb_unicode_normalizer_t *uc,
                                       lxb_unicode_buffer_t **buf,
                                       const lxb_unicode_buffer_t **end,
                                       lxb_codepoint_t cp)
{
    lxb_unicode_buffer_t *p;
    lxb_codepoint_t sid = cp - lxb_unicode_sb;
    lxb_codepoint_t tid = sid % lxb_unicode_tc;
    lxb_codepoint_t x = floorf((sid - tid) / lxb_unicode_tc);

    lxb_unicode_check_buf(uc, buf, end, 2 + (tid != 0));
    if (*buf == NULL) {
        return NULL;
    }

    p = *buf;

    p->cp = lxb_unicode_lb + floorf(x / lxb_unicode_vc);
    p->ccc = 0;
    p += 1;

    p->cp = lxb_unicode_vb + (x  % lxb_unicode_vc);
    p->ccc = 0;
    p += 1;

    if (tid != 0) {
        p->cp = lxb_unicode_ts + tid;
        p->ccc = 0;
        p += 1;
    }

    return p;
}

static lxb_codepoint_t
lxb_unicode_entry_compose_hangul(lxb_codepoint_t first, lxb_codepoint_t second)
{

    if (first >= lxb_unicode_lb && first <= lxb_unicode_ll
        && second >= lxb_unicode_vb && second <= lxb_unicode_vl)
    {
        return lxb_unicode_sb
            + (((first - lxb_unicode_lb)
                * lxb_unicode_vc) + second - lxb_unicode_vb)
            * lxb_unicode_tc;
    }

    if (first >= lxb_unicode_sb && first <= lxb_unicode_sl
        && (first - lxb_unicode_sb) % lxb_unicode_tc == 0
        && second >= lxb_unicode_tb && second <= lxb_unicode_tl)
    {
        return first + second - lxb_unicode_ts;
    }

    return LXB_ENCODING_ERROR_CODEPOINT;
}

const lxb_unicode_entry_t *
lxb_unicode_entry(lxb_codepoint_t cp)
{
    size_t idx;
    const lxb_unicode_node_t *node;
    const lxb_unicode_table_t *table = &lxb_unicode_map;

    do {
        idx = cp % table->length;
        node = &table->nodes[idx];

        if (node->entry != NULL && node->entry->cp == cp) {
            return node->entry;
        }

        table = node->table;
    }
    while (table != NULL);

    return NULL;
}

const lxb_unicode_compose_entry_t *
lxb_unicode_compose_entry(lxb_codepoint_t first, lxb_codepoint_t second)
{
    size_t idx;
    uint32_t id = (first % 65535) << 16 | (second % 65535);
    const lxb_unicode_compose_node_t *node;
    const lxb_unicode_compose_table_t *table = &lxb_unicode_composition;

    do {
        idx = id % table->length;
        node = &table->nodes[idx];

        if (node->entry != NULL && node->entry->idx == id) {
            return node->entry;
        }

        table = node->table;
    }
    while (table != NULL);

    return NULL;
}
