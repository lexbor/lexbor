/*
 * Copyright (C) 2023-2024 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/punycode/punycode.h"
#include "lexbor/encoding/encoding.h"


enum {
    LXB_PUNYCODE_BASE = 36,
    LXB_PUNYCODE_TMIN = 1,
    LXB_PUNYCODE_TMAX = 26,
    LXB_PUNYCODE_SKEW = 38,
    LXB_PUNYCODE_DAMP = 700,
    LXB_PUNYCODE_INITIAL_BIAS = 72,
    LXB_PUNYCODE_INITIAL_N = 0x80,
    LXB_PUNYCODE_DELIMITER = 0x2D
};


static lxb_status_t
lxb_punycode_callback_cp(const lxb_codepoint_t *cps, size_t len, void *ctx);


lxb_inline lxb_char_t *
lxb_punycode_encode_realloc(lxb_char_t *p, lxb_char_t **buf,
                            const lxb_char_t **end, const lxb_char_t *buffer)
{
    size_t cur_size = *end - *buf;
    size_t nsize = cur_size * 2;
    lxb_char_t *tmp;

    if (*buf == buffer) {
        tmp = lexbor_malloc(nsize);
        if (tmp == NULL) {
            return NULL;
        }

        memcpy(tmp, *buf, cur_size);
    }
    else {
        tmp = lexbor_realloc(*buf, nsize);
        if (tmp == NULL) {
            return lexbor_free(*buf);
        }
    }

    *buf = tmp;
    *end = tmp + nsize;

    return tmp + cur_size;
}

lxb_inline lxb_codepoint_t *
lxb_punycode_decode_realloc(lxb_codepoint_t *p, lxb_codepoint_t **buf,
                            const lxb_codepoint_t **end,
                            const lxb_codepoint_t *buffer)
{
    size_t cur_size = *end - *buf;
    size_t nsize = cur_size * 2;
    lxb_codepoint_t *tmp;

    if (*buf == buffer) {
        tmp = lexbor_malloc(nsize * sizeof(lxb_codepoint_t));
        if (tmp == NULL) {
            return NULL;
        }

        memcpy(tmp, *buf, cur_size * sizeof(lxb_codepoint_t));
    }
    else {
        tmp = lexbor_realloc(*buf, nsize * sizeof(lxb_codepoint_t));
        if (tmp == NULL) {
            return lexbor_free(*buf);
        }
    }

    *buf = tmp;
    *end = tmp + nsize;

    return tmp + cur_size;
}

static char
lxb_punycode_encode_digit(size_t d) {
    return d + 22 + 75 * (d < 26);
}

static size_t
lxb_punycode_decode_digit(lxb_codepoint_t cp)
{
    return  cp - 48 < 10 ? cp - 22 : cp - 65 < 26 ? cp - 65
          : cp - 97 < 26 ? cp - 97 : LXB_PUNYCODE_BASE;
}

static size_t
lxb_punycode_adapt(size_t delta, size_t numpoints, bool firsttime)
{
    size_t k;

    delta = firsttime ? delta / LXB_PUNYCODE_DAMP : delta >> 1;
    delta += delta / numpoints;

    for (k = 0;
         delta > ((LXB_PUNYCODE_BASE - LXB_PUNYCODE_TMIN) * LXB_PUNYCODE_TMAX) / 2;
         k += LXB_PUNYCODE_BASE)
    {
        delta /= LXB_PUNYCODE_BASE - LXB_PUNYCODE_TMIN;
    }

    return k + (LXB_PUNYCODE_BASE - LXB_PUNYCODE_TMIN + 1)
           * delta / (delta + LXB_PUNYCODE_SKEW);
}

lxb_inline size_t
lxb_punycode_lowbit(size_t i)
{
    return i & (~i + 1);
}

lxb_inline size_t
lxb_punycode_tree_sum(const size_t *tree, size_t pos)
{
    size_t sum;

    for (sum = 0; pos > 0; pos -= lxb_punycode_lowbit(pos)) {
        sum += tree[pos];
    }

    return sum;
}

lxb_inline void
lxb_punycode_tree_add(size_t *tree, size_t length, size_t pos)
{
    for (pos += 1; pos <= length; pos += lxb_punycode_lowbit(pos)) {
        tree[pos] += 1;
    }
}

lxb_inline bool
lxb_punycode_before(const lxb_codepoint_t *cps, size_t a, size_t b)
{
    return cps[a] < cps[b] || (cps[a] == cps[b] && a < b);
}

static void
lxb_punycode_sift_down(size_t *order, size_t root, size_t length,
                       const lxb_codepoint_t *cps)
{
    size_t child, tmp;

    for (child = root * 2 + 1; child < length; child = root * 2 + 1) {
        if (child + 1 < length
            && lxb_punycode_before(cps, order[child], order[child + 1]))
        {
            child += 1;
        }

        if (lxb_punycode_before(cps, order[root], order[child]) == false) {
            break;
        }

        tmp = order[root];
        order[root] = order[child];
        order[child] = tmp;

        root = child;
    }
}

static void
lxb_punycode_sort(size_t *order, size_t length, const lxb_codepoint_t *cps)
{
    size_t i, tmp;

    for (i = length / 2; i > 0; i--) {
        lxb_punycode_sift_down(order, i - 1, length, cps);
    }

    for (i = length; i > 1; i--) {
        tmp = order[0];
        order[0] = order[i - 1];
        order[i - 1] = tmp;

        lxb_punycode_sift_down(order, 0, i - 1, cps);
    }
}

lxb_inline size_t *
lxb_punycode_encode_scratch(size_t length, size_t *scratch, size_t size)
{
    if (length > (SIZE_MAX / sizeof(size_t) - 1) / 2) {
        return NULL;
    }

    if (length * 2 + 1 <= size) {
        return scratch;
    }

    return lexbor_malloc((length * 2 + 1) * sizeof(size_t));
}

static lxb_status_t
lxb_punycode_encode_body(const lxb_codepoint_t *cps, size_t length,
                         size_t *order, size_t *tree, lxb_char_t *p,
                         lxb_char_t *buf, const lxb_char_t *end,
                         const lxb_char_t *buffer, lxb_punycode_encode_cb_f cb,
                         void *ctx)
{
    bool unchanged;
    size_t h, b, n, q, k, t, i, j, count, first, less, rank, prev,
           delta, bias;
    lxb_status_t status;
    lxb_codepoint_t m;

    b = p - buf;
    count = length - b;

    if (count == 0) {
        unchanged = true;
        goto done;
    }

    if (p > buf) {
        if (p >= end) {
            p = lxb_punycode_encode_realloc(p, &buf, &end, buffer);
            if (p == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }

        *p++ = LXB_PUNYCODE_DELIMITER;
    }

    unchanged = false;

    for (i = 1; i <= length; i++) {
        j = i + lxb_punycode_lowbit(i);

        if (j <= length) {
            tree[j] += tree[i];
        }
    }

    lxb_punycode_sort(order, count, cps);

    n = LXB_PUNYCODE_INITIAL_N;
    bias = LXB_PUNYCODE_INITIAL_BIAS;
    delta = 0;
    h = b;
    first = 0;
    i = 0;

    while (i < count) {
        while (first < i) {
            lxb_punycode_tree_add(tree, length, order[first++]);
        }

        m = cps[order[i]];

        if (m - n > (UINT32_MAX - delta) / (h + 1)) {
            status = LXB_STATUS_ERROR_OVERFLOW;
            goto failed;
        }

        delta += (m - n) * (h + 1);
        n = m;

        less = h;
        prev = 0;

        do {
            rank = lxb_punycode_tree_sum(tree, order[i]);

            if (rank - prev > UINT32_MAX - delta) {
                status = LXB_STATUS_ERROR_OVERFLOW;
                goto failed;
            }

            delta += rank - prev;
            prev = rank;

            q = delta;
            k = LXB_PUNYCODE_BASE;

            for (;; k += LXB_PUNYCODE_BASE) {
                t = k <= bias ? LXB_PUNYCODE_TMIN :
                k >= bias + LXB_PUNYCODE_TMAX
                ? LXB_PUNYCODE_TMAX : k - bias;

                if (q < t) {
                    break;
                }

                if (p >= end) {
                    p = lxb_punycode_encode_realloc(p, &buf, &end, buffer);
                    if (p == NULL) {
                        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    }
                }

                *p++ = lxb_punycode_encode_digit(t + (q - t)
                                                 % (LXB_PUNYCODE_BASE - t));
                q = (q - t) / (LXB_PUNYCODE_BASE - t);
            }

            if (p >= end) {
                p = lxb_punycode_encode_realloc(p, &buf, &end, buffer);
                if (p == NULL) {
                    return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                }
            }

            *p++ = lxb_punycode_encode_digit(q);
            bias = lxb_punycode_adapt(delta, h + 1, h == b);
            delta = 0;
            h += 1;
            i += 1;
        }
        while (i < count && cps[order[i]] == m);

        if (less - prev >= UINT32_MAX) {
            status = LXB_STATUS_ERROR_OVERFLOW;
            goto failed;
        }

        delta = less - prev + 1;
        n += 1;
    }

done:

    status = cb(buf, p - buf, ctx, unchanged);

failed:

    if (buf != buffer) {
        (void) lexbor_free(buf);
    }

    return status;
}

lxb_status_t
lxb_punycode_encode(const lxb_char_t *data, size_t length,
                    lxb_punycode_encode_cb_f cb, void *ctx)
{
    size_t i, cp_length, count;
    size_t *order, *tree, *mem;
    uint8_t len;
    lxb_char_t *p, *buf;
    lxb_status_t status;
    lxb_codepoint_t cp, *cps;
    const lxb_char_t *data_p, *data_end, *end;
    lxb_char_t buffer[4096];
    lxb_codepoint_t input[4096];
    size_t scratch[512];

    /*
     * Make GCC happy.
     * length variable can be 0.
     */
    input[0] = 0x00;

    p = buffer;
    buf = buffer;
    end = buffer + sizeof(buffer);

    data_p = data;
    data_end = data + length;
    cp_length = 0;

    while (data_p < data_end) {
        len = lxb_encoding_decode_utf_8_length(*data_p);
        if (len == 0) {
            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
        }

        data_p += len;
        cp_length += 1;
    }

    if (cp_length <= sizeof(input) / sizeof(lxb_codepoint_t)) {
        cps = input;
    }
    else {
        cps = lexbor_malloc(cp_length * sizeof(lxb_codepoint_t));
        if (cps == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }
    }

    mem = lxb_punycode_encode_scratch(cp_length, scratch,
                                      sizeof(scratch) / sizeof(size_t));
    if (mem == NULL) {
        status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        goto done;
    }

    tree = mem;
    order = mem + cp_length + 1;

    data_p = data;
    count = 0;

    for (i = 0; data_p < data_end; i++) {
        cp = lxb_encoding_decode_valid_utf_8_single(&data_p, data_end);
        if (cp == LXB_ENCODING_DECODE_ERROR) {
            if (buf != buffer) {
                (void) lexbor_free(buf);
            }

            status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
            goto done;
        }

        cps[i] = cp;

        if (cp < 0x80) {
            if (p >= end) {
                p = lxb_punycode_encode_realloc(p, &buf, &end, buffer);
                if (p == NULL) {
                    status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    goto done;
                }
            }

            *p++ = cp;
            tree[i + 1] = 1;
        }
        else {
            tree[i + 1] = 0;
            order[count++] = i;
        }
    }

    status = lxb_punycode_encode_body(cps, cp_length, order, tree, p, buf,
                                      end, buffer, cb, ctx);
done:

    if (mem != scratch && mem != NULL) {
        (void) lexbor_free(mem);
    }

    if (cps != input) {
        (void) lexbor_free(cps);
    }

    return status;
}

lxb_status_t
lxb_punycode_encode_cp(const lxb_codepoint_t *cps, size_t length,
                       lxb_punycode_encode_cb_f cb, void *ctx)
{
    size_t i, count;
    size_t *order, *tree, *mem;
    lxb_char_t *p, *buf;
    lxb_status_t status;
    lxb_codepoint_t cp;
    const lxb_char_t *end;
    lxb_char_t buffer[4096];
    size_t scratch[512];

    mem = lxb_punycode_encode_scratch(length, scratch,
                                      sizeof(scratch) / sizeof(size_t));
    if (mem == NULL) {
        return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
    }

    tree = mem;
    order = mem + length + 1;

    p = buffer;
    buf = buffer;
    end = buffer + sizeof(buffer);

    count = 0;

    for (i = 0; i < length; i++) {
        cp = cps[i];

        if (cp < 0x80) {
            if (p >= end) {
                p = lxb_punycode_encode_realloc(p, &buf, &end, buffer);
                if (p == NULL) {
                    status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
                    goto done;
                }
            }

            *p++ = cp;
            tree[i + 1] = 1;
        }
        else {
            tree[i + 1] = 0;
            order[count++] = i;
        }
    }

    status = lxb_punycode_encode_body(cps, length, order, tree, p, buf, end,
                                      buffer, cb, ctx);
done:

    if (mem != scratch) {
        (void) lexbor_free(mem);
    }

    return status;
}

lxb_status_t
lxb_punycode_decode(const lxb_char_t *data, size_t length,
                    lexbor_serialize_cb_f cb, void *ctx)
{
    lexbor_serialize_ctx_t nctx = {.cb = cb, .ctx = ctx};

    return lxb_punycode_decode_cb_cp(data, length, lxb_punycode_callback_cp,
                                     &nctx);
}

static lxb_status_t
lxb_punycode_callback_cp(const lxb_codepoint_t *cps, size_t len, void *ctx)
{
    uint8_t i;
    size_t length;
    lxb_status_t status;
    const lxb_codepoint_t *cps_p, *cps_end;
    lexbor_serialize_ctx_t *nctx = ctx;
    lxb_char_t *p, *buf, *end;
    lxb_char_t buffer[4096];

    /*
     * Make GCC happy.
     * len variable can be 0.
     */
    buffer[0] = 0x00;

    cps_p = cps;
    cps_end = cps_p + len;
    length = 0;

    while (cps_p < cps_end) {
        i = lxb_encoding_encode_utf_8_length(*cps_p++);
        if (i == 0) {
            return LXB_STATUS_ERROR_UNEXPECTED_DATA;
        }

        length += i;
    }

    buf = buffer;
    end = buffer + sizeof(buffer);

    if (buf + length > end) {
        buf = lexbor_malloc(length);
        if (buf == NULL) {
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        }

        end = buf + length;
    }

    p = buf;
    cps_p = cps;

    while (cps_p < cps_end) {
        (void) lxb_encoding_encode_utf_8_single(NULL, &p, end, *cps_p++);
    }

    status = nctx->cb(buf, p - buf, nctx->ctx);

    if (buf != buffer) {
        (void) lexbor_free(buf);
    }

    return status;
}

lxb_status_t
lxb_punycode_decode_cp(const lxb_codepoint_t *data, size_t length,
                       lexbor_serialize_cb_cp_f cb, void *ctx)
{
    size_t buf_len, digit, oldi, bias, w, k, t, i, h, in;
    const lxb_codepoint_t *delimiter, *data_p, *data_end;
    lxb_status_t status;
    lxb_codepoint_t cp, n;
    lxb_codepoint_t *p, *buf;
    const lxb_codepoint_t *end;
    lxb_codepoint_t buffer[4096];

    p = buffer;
    buf = buffer;
    buf_len = sizeof(buffer) / sizeof(lxb_codepoint_t);
    end = buffer + buf_len;

    data_p = data;
    data_end = data + length;
    delimiter = data_end;

    while (delimiter != data) {
        delimiter -= 1;

        if (*delimiter == LXB_PUNYCODE_DELIMITER) {
            break;
        }
    }

    while (data_p < delimiter) {
        cp = *data_p++;

        if (cp >= 0x80) {
            status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
            goto done;
        }

        if (p >= end) {
            p = lxb_punycode_decode_realloc(p, &buf, &end, buffer);
            if (p == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }

        *p++ = cp;
    }

    i = 0;
    n = LXB_PUNYCODE_INITIAL_N;
    bias = LXB_PUNYCODE_INITIAL_BIAS;
    data_p = (delimiter != data) ? delimiter + 1: data;
    in = data_p - data;

    for (; in < length; p++) {
        for (oldi = i, w = 1, k = LXB_PUNYCODE_BASE; ; k += LXB_PUNYCODE_BASE) {
            if (in >= length) {
                status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
                goto done;
            }

            cp = data[in++];
            digit = lxb_punycode_decode_digit(cp);

            if (digit >= LXB_PUNYCODE_BASE) {
                status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
                goto done;
            }

            if (digit > (UINT32_MAX - i) / w) {
                status = LXB_STATUS_ERROR_OVERFLOW;
                goto done;
            }

            i += digit * w;
            t = k <= bias ? LXB_PUNYCODE_TMIN
            : k >= bias + LXB_PUNYCODE_TMAX ? LXB_PUNYCODE_TMAX : k - bias;

            if (digit < t) {
                break;
            }

            if (w > UINT32_MAX / (LXB_PUNYCODE_BASE - t)) {
                status = LXB_STATUS_ERROR_OVERFLOW;
                goto done;
            }

            w *= (LXB_PUNYCODE_BASE - t);
        }

        h = (p - buf) + 1;

        bias = lxb_punycode_adapt(i - oldi, h, oldi == 0);

        if (i / h > UINT32_MAX - n) {
            status = LXB_STATUS_ERROR_OVERFLOW;
            goto done;
        }

        n += i / h;
        i %= h;

        if (p >= end) {
            p = lxb_punycode_decode_realloc(p, &buf, &end, buffer);
            if (p == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }

        memmove(buf + i + 1, buf + i, ((h - 1) - i) * sizeof(lxb_codepoint_t));
        buf[i++] = n;
    }

    status = cb(buf, p - buf, ctx);

done:

    if (buffer != buf) {
        (void) lexbor_free(buf);
    }

    return status;
}

lxb_status_t
lxb_punycode_decode_cb_cp(const lxb_char_t *data, size_t length,
                          lexbor_serialize_cb_cp_f cb, void *ctx)
{
    size_t buf_len, digit, oldi, bias, w, k, t, i, h, in;
    const lxb_char_t *delimiter, *data_p, *data_end;
    lxb_status_t status;
    lxb_codepoint_t cp, n;
    lxb_codepoint_t *p, *buf;
    const lxb_codepoint_t *end;
    lxb_codepoint_t buffer[4096];

    p = buffer;
    buf = buffer;
    buf_len = sizeof(buffer) / sizeof(lxb_codepoint_t);
    end = buffer + buf_len;

    data_p = data;
    data_end = data + length;
    delimiter = data_end;

    while (delimiter != data) {
        delimiter -= 1;

        if (*delimiter == LXB_PUNYCODE_DELIMITER) {
            break;
        }
    }

    while (data_p < delimiter) {
        cp = *data_p++;

        if (cp >= 0x80) {
            status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
            goto done;
        }

        if (p >= end) {
            p = lxb_punycode_decode_realloc(p, &buf, &end, buffer);
            if (p == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }

        *p++ = cp;
    }

    i = 0;
    n = LXB_PUNYCODE_INITIAL_N;
    bias = LXB_PUNYCODE_INITIAL_BIAS;
    data_p = (delimiter != data) ? delimiter + 1: data;
    in = data_p - data;

    for (; in < length; p++) {
        for (oldi = i, w = 1, k = LXB_PUNYCODE_BASE; ; k += LXB_PUNYCODE_BASE) {
            if (in >= length) {
                status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
                goto done;
            }

            cp = data[in++];
            digit = lxb_punycode_decode_digit(cp);

            if (digit >= LXB_PUNYCODE_BASE) {
                status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
                goto done;
            }

            if (digit > (UINT32_MAX - i) / w) {
                status = LXB_STATUS_ERROR_OVERFLOW;
                goto done;
            }

            i += digit * w;
            t = k <= bias ? LXB_PUNYCODE_TMIN
              : k >= bias + LXB_PUNYCODE_TMAX ? LXB_PUNYCODE_TMAX : k - bias;

            if (digit < t) {
                break;
            }

            if (w > UINT32_MAX / (LXB_PUNYCODE_BASE - t)) {
                status = LXB_STATUS_ERROR_OVERFLOW;
                goto done;
            }

            w *= (LXB_PUNYCODE_BASE - t);
        }

        h = (p - buf) + 1;

        bias = lxb_punycode_adapt(i - oldi, h, oldi == 0);

        if (i / h > UINT32_MAX - n) {
            status = LXB_STATUS_ERROR_OVERFLOW;
            goto done;
        }

        n += i / h;
        i %= h;

        if (p >= end) {
            p = lxb_punycode_decode_realloc(p, &buf, &end, buffer);
            if (p == NULL) {
                return LXB_STATUS_ERROR_MEMORY_ALLOCATION;
            }
        }

        memmove(buf + i + 1, buf + i, ((h - 1) - i) * sizeof(lxb_codepoint_t));
        buf[i++] = n;
    }

    status = cb(buf, p - buf, ctx);

done:

    if (buffer != buf) {
        (void) lexbor_free(buf);
    }

    return status;
}
