/*
 * Copyright (C) 2026 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/html/common.h"


lxb_status_t
lxb_html_common_parsing_integer(const lxb_char_t *data, size_t length,
                                int64_t *result)
{
    bool sign;
    uint64_t value;
    const lxb_char_t *end;

    if (data == NULL || length == 0) {
        goto error;
    }

    end = data + length;

    /* 4. Skip ASCII whitespace. */

    while (data < end && lexbor_utils_whitespace(*data, ==, ||)) {
        data++;
    }

    /* 5. If position is past the end of input, return an error. */

    if (data >= end) {
        goto error;
    }

    /* 6. */

    sign = true;

    if (*data == '-') {
        sign = false;
        data++;

        if (data >= end) {
            goto error;
        }
    }
    else if (*data == '+') {
        data++;

        if (data >= end) {
            goto error;
        }
    }

    /* 7. If the character indicated by position is not an ASCII digit,
     * then return an error. */

    if (*data < '0' || *data > '9') {
        goto error;
    }

    /* 8. Collect a sequence of code points that are ASCII digits from input
     * given position, and interpret the resulting sequence as a base-ten
     * integer. Let value be that integer. */

    value = 0;

    while (data < end && *data >= '0' && *data <= '9') {
        value = value * 10 + (*data - '0');
        data++;
    }

    /* 9. If sign is "positive", return value, otherwise return the result
     * of subtracting value from zero. */

    *result = (sign) ? (int64_t) value : -((int64_t) value);

    return LXB_STATUS_OK;

error:

    *result = 0;

    return LXB_STATUS_ERROR;
}

lxb_status_t
lxb_html_common_parsing_nonneg_integer(const lxb_char_t *data, size_t length,
                                       int64_t *result)
{
    lxb_status_t status;

    /* 2. Let value be the result of parsing input using the rules
     * for parsing integers. */

    status = lxb_html_common_parsing_integer(data, length, result);

    /* 3. If value is an error, return an error. */

    if (status != LXB_STATUS_OK) {
        return status;
    }

    /* 4. If value is less than zero, return an error. */

    if (*result < 0) {
        return LXB_STATUS_ERROR;
    }

    /* 5. Return value. */

    return LXB_STATUS_OK;
}

