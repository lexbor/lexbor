/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/encoding/decode.h"
#include "lexbor/encoding/single.h"
#include "lexbor/encoding/multi.h"
#include "lexbor/encoding/range.h"


#define LXB_ENCODING_DECODE_UTF_8_BOUNDARY(lower, upper)                       \
    do {                                                                       \
        ch = **data;                                                           \
                                                                               \
        if (ch < lower || ch > upper) {                                        \
            goto failed;                                                       \
        }                                                                      \
                                                                               \
        (*data)++;                                                             \
        needed--;                                                              \
        ctx->codepoint = (ctx->codepoint << 6) | (ch & 0x3F);                  \
    }                                                                          \
    while (0)

#define LXB_ENCODING_DECODE_UTF_8_BOUNDARY_SET(first, two, f_lower, s_upper)   \
    do {                                                                       \
        if (ch == first) {                                                     \
            ctx->u.utf_8.lower = f_lower;                                      \
            ctx->u.utf_8.upper = 0xBF;                                         \
        }                                                                      \
        else if (ch == two) {                                                  \
            ctx->u.utf_8.lower = 0x80;                                         \
            ctx->u.utf_8.upper = s_upper;                                      \
        }                                                                      \
    }                                                                          \
    while (0)


lxb_codepoint_t
lxb_encoding_decode_default(lxb_encoding_decode_t *ctx,
                            const lxb_char_t **data, const lxb_char_t *end)
{
    return lxb_encoding_decode_utf_8(ctx, data, end);
}

lxb_codepoint_t
lxb_encoding_decode_auto(lxb_encoding_decode_t *ctx,
                         const lxb_char_t **data, const lxb_char_t *end)
{
    return LXB_ENCODING_DECODE_ERROR;
}

lxb_codepoint_t
lxb_encoding_decode_undefined(lxb_encoding_decode_t *ctx,
                              const lxb_char_t **data, const lxb_char_t *end)
{
    return LXB_ENCODING_DECODE_ERROR;
}

lxb_codepoint_t
lxb_encoding_decode_big5(lxb_encoding_decode_t *ctx,
                         const lxb_char_t **data, const lxb_char_t *end)
{
    uint32_t index;
    lxb_char_t lead, byte;

    if (ctx->u.lead != 0x00) {
        if (ctx->second_codepoint != 0x00) {
            (*data)++;

            ctx->u.lead = 0x00;

            ctx->codepoint = ctx->second_codepoint;
            ctx->second_codepoint = 0x00;

            return ctx->codepoint;
        }

        lead = (lxb_char_t) ctx->u.lead;
        ctx->u.lead = 0x00;

        goto lead_state;
    }

    lead = **data;

    if (lead < 0x80) {
        return *(*data)++;
    }

    if ((unsigned) (lead - 0x81) > (0xFE - 0x81)) {
        return LXB_ENCODING_DECODE_ERROR;
    }

    if (++(*data) >= end) {
        ctx->u.lead = lead;

        return LXB_ENCODING_DECODE_CONTINUE;
    }

lead_state:

    index = 0;
    byte = **data;

    if ((unsigned) (byte - 0x40) <= (0x7E - 0x40)
        || (unsigned) (byte - 0xA1) <= (0xFE - 0xA1))
    {
        if (byte < 0x7F) {
            /* Max index == (0xFE - 0x81) * 157 + (0x7E - 0x62) == 19653 */
            index = (lead - 0x81) * 157 + (byte - 0x40);
        }
        else {
            /* Max index == (0xFE - 0x81) * 157 + (0xFE - 0x62) == 19781 */
            index = (lead - 0x81) * 157 + (byte - 0x62);
        }
    }

    /*
     * 1133 U+00CA U+0304  Ê̄ (LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND MACRON)
     * 1135 U+00CA U+030C  Ê̌ (LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND CARON)
     * 1164 U+00EA U+0304  ê̄ (LATIN SMALL LETTER E WITH CIRCUMFLEX AND MACRON)
     * 1166 U+00EA U+030C  ê̌ (LATIN SMALL LETTER E WITH CIRCUMFLEX AND CARON)
     */
    switch (index) {
        case 1133:
            ctx->u.lead = lead;
            ctx->second_codepoint = 0x0304;
            return 0x00CA;

        case 1135:
            ctx->u.lead = lead;
            ctx->second_codepoint = 0x030C;
            return 0x00CA;

        case 1164:
            ctx->u.lead = lead;
            ctx->second_codepoint = 0x0304;
            return 0x00EA;

        case 1166:
            ctx->u.lead = lead;
            ctx->second_codepoint = 0x030C;
            return 0x00EA;

        case 0:
            goto failed;
    }

    ctx->codepoint = lxb_encoding_multi_index_big5[index].codepoint;
    if (ctx->codepoint == LXB_ENCODING_DECODE_ERROR) {
        goto failed;
    }

    (*data)++;

    return ctx->codepoint;

failed:

    if (byte >= 0x80) {
        (*data)++;
    }

    return LXB_ENCODING_DECODE_ERROR;
}

lxb_codepoint_t
lxb_encoding_decode_euc_jp(lxb_encoding_decode_t *ctx,
                           const lxb_char_t **data, const lxb_char_t *end)
{
    bool is_jis0212;
    lxb_char_t byte, lead;

    if (ctx->u.euc_jp.lead != 0x00) {
        lead = ctx->u.euc_jp.lead;
        byte = *(*data)++;

        ctx->u.euc_jp.lead = 0x00;

        if (ctx->u.euc_jp.is_jis0212) {
            is_jis0212 = true;
            ctx->u.euc_jp.is_jis0212 = false;

            goto lead_jis_state;
        }

        goto lead_state;
    }

    lead = *(*data)++;

    if (lead < 0x80) {
        return lead;
    }

    if ((unsigned) (lead - 0xA1) > (0xFE - 0xA1)
        && (lead != 0x8E && lead != 0x8F))
    {
        return LXB_ENCODING_DECODE_ERROR;
    }

    if (*data >= end) {
        ctx->u.euc_jp.lead = lead;
        return LXB_ENCODING_DECODE_CONTINUE;
    }

    byte = *(*data)++;

lead_state:

    if (lead == 0x8E && (unsigned) (byte - 0xA1) <= (0xDF - 0xA1)) {
        return 0xFF61 - 0xA1 + byte;
    }

    is_jis0212 = false;

    if (lead == 0x8F && (unsigned) (byte - 0xA1) <= (0xFE - 0xA1)) {
        if (*data >= end) {
            ctx->u.euc_jp.lead = byte;
            ctx->u.euc_jp.is_jis0212 = true;

            return LXB_ENCODING_DECODE_CONTINUE;
        }

        lead = byte;
        byte = *(*data)++;
        is_jis0212 = true;
    }

lead_jis_state:

    if ((unsigned) (lead - 0xA1) > (0xFE - 0xA1)
        || (unsigned) (byte - 0xA1) > (0xFE - 0xA1))
    {
        goto failed;
    }

    /* Max index == (0xFE - 0xA1) * 94 + 0xFE - 0xA1 == 8835 */
    ctx->codepoint = (lead - 0xA1) * 94 + byte - 0xA1;

    if (is_jis0212) {
        if ((sizeof(lxb_encoding_multi_index_jis0212)
             / sizeof(lxb_encoding_multi_index_t)) <= ctx->codepoint)
        {
            goto failed;
        }

        ctx->codepoint = lxb_encoding_multi_index_jis0212[ctx->codepoint].codepoint;
    }
    else {
        if ((sizeof(lxb_encoding_multi_index_jis0208)
             / sizeof(lxb_encoding_multi_index_t)) <= ctx->codepoint)
        {
            goto failed;
        }

        ctx->codepoint = lxb_encoding_multi_index_jis0208[ctx->codepoint].codepoint;
    }

    if (ctx->codepoint == LXB_ENCODING_DECODE_ERROR) {
        goto failed;
    }

    return ctx->codepoint;

failed:

    if (byte < 0x80) {
        (*data)--;
    }

    return LXB_ENCODING_DECODE_ERROR;
}

lxb_codepoint_t
lxb_encoding_decode_euc_kr(lxb_encoding_decode_t *ctx,
                           const lxb_char_t **data, const lxb_char_t *end)
{
    lxb_char_t lead, byte;

    if (ctx->u.lead != 0x00) {
        lead = (lxb_char_t) ctx->u.lead;
        ctx->u.lead = 0x00;

        goto lead_state;
    }

    lead = *(*data)++;

    if (lead < 0x80) {
        return lead;
    }

    if ((unsigned) (lead - 0x81) > (0xFE - 0x81)) {
        return LXB_ENCODING_DECODE_ERROR;
    }

    if (*data == end) {
        ctx->u.lead = lead;
        return LXB_ENCODING_DECODE_CONTINUE;
    }

lead_state:

    byte = *(*data)++;

    if ((unsigned) (byte - 0x41) > (0xFE - 0x41)) {
        goto failed;
    }

    /* Max index == (0xFE - 0x81) * 190 + (0xFE - 0x41) == 23939 */
    ctx->codepoint = (lead - 0x81) * 190 + (byte - 0x41);

    if (ctx->codepoint >= sizeof(lxb_encoding_multi_index_euc_kr)
                          / sizeof(lxb_encoding_multi_index_t))
    {
        goto failed;
    }

    ctx->codepoint = lxb_encoding_multi_index_euc_kr[ctx->codepoint].codepoint;
    if (ctx->codepoint == LXB_ENCODING_DECODE_ERROR) {
        goto failed;
    }

    return ctx->codepoint;

failed:

    if (byte < 0x80) {
        (*data)--;
    }

    return LXB_ENCODING_DECODE_ERROR;
}

lxb_codepoint_t
lxb_encoding_decode_gbk(lxb_encoding_decode_t *ctx,
                        const lxb_char_t **data, const lxb_char_t *end)
{
    return lxb_encoding_decode_gb18030(ctx, data, end);
}

lxb_codepoint_t
lxb_encoding_decode_ibm866(lxb_encoding_decode_t *ctx,
                           const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_ibm866[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_2022_jp(lxb_encoding_decode_t *ctx,
                                const lxb_char_t **data, const lxb_char_t *end)
{
    lxb_char_t byte;
    lxb_encoding_ctx_2022_jp_t *iso = &ctx->u.iso_2022_jp;

    if (iso->prepand != 0x00) {
        byte = iso->prepand;
        iso->prepand = 0x00;

        goto prepand;
    }

    do {
        byte = *(*data)++;

    prepand:

        switch (iso->state) {
            case LXB_ENCODING_DECODE_2022_JP_ASCII:
                if (byte == 0x1B) {
                    iso->state = LXB_ENCODING_DECODE_2022_JP_ESCAPE_START;

                    break;
                }

                /* 0x00 to 0x7F, excluding 0x0E, 0x0F, and 0x1B */
                if ((unsigned) (byte - 0x00) <= (0x7F - 0x00)
                    && byte != 0x0E && byte != 0x0F)
                {
                    iso->out_flag = false;

                    return byte;
                }

                iso->out_flag = false;

                return LXB_ENCODING_DECODE_ERROR;

            case LXB_ENCODING_DECODE_2022_JP_ROMAN:
                switch (byte) {
                    case 0x1B:
                        iso->state = LXB_ENCODING_DECODE_2022_JP_ESCAPE_START;

                        continue;

                    case 0x5C:
                        iso->out_flag = false;

                        return 0x00A5;

                    case 0x7E:
                        iso->out_flag = false;

                        return 0x203E;

                    case 0x0E:
                    case 0x0F:
                        break;

                    default:
                        /* 0x00 to 0x7F */
                        if ((unsigned) (byte - 0x00) <= (0x7F - 0x00)) {
                            iso->out_flag = false;

                            return byte;
                        }

                        break;
                }

                iso->out_flag = false;

                return LXB_ENCODING_DECODE_ERROR;

            case LXB_ENCODING_DECODE_2022_JP_KATAKANA:
                if (byte == 0x1B) {
                    iso->state = LXB_ENCODING_DECODE_2022_JP_ESCAPE_START;

                    break;
                }

                /* 0x21 to 0x5F */
                if ((unsigned) (byte - 0x21) <= (0x5F - 0x21)) {
                    iso->out_flag = false;

                    return 0xFF61 - 0x21 + byte;
                }

                iso->out_flag = false;

                return LXB_ENCODING_DECODE_ERROR;

            case LXB_ENCODING_DECODE_2022_JP_LEAD:
                if (byte == 0x1B) {
                    iso->state = LXB_ENCODING_DECODE_2022_JP_ESCAPE_START;

                    break;
                }

                /* 0x21 to 0x7E */
                if ((unsigned) (byte - 0x21) <= (0x7E - 0x21)) {
                    iso->out_flag = false;
                    iso->lead = byte;
                    iso->state = LXB_ENCODING_DECODE_2022_JP_TRAIL;

                    break;
                }

                iso->out_flag = false;

                return LXB_ENCODING_DECODE_ERROR;

            case LXB_ENCODING_DECODE_2022_JP_TRAIL:
                if (byte == 0x1B) {
                    iso->state = LXB_ENCODING_DECODE_2022_JP_ESCAPE_START;

                    return LXB_ENCODING_DECODE_ERROR;
                }

                iso->state = LXB_ENCODING_DECODE_2022_JP_LEAD;

                /* 0x21 to 0x7E */
                if ((unsigned) (byte - 0x21) <= (0x7E - 0x21)) {
                    /* Max index == (0x7E - 0x21) * 94 + 0x7E - 0x21 == 8835 */
                    ctx->codepoint = (iso->lead - 0x21) * 94 + byte - 0x21;

                    return lxb_encoding_multi_index_jis0208[ctx->codepoint].codepoint;
                }

                return LXB_ENCODING_DECODE_ERROR;

            case LXB_ENCODING_DECODE_2022_JP_ESCAPE_START:
                if (byte == 0x24 || byte == 0x28) {
                    iso->state = LXB_ENCODING_DECODE_2022_JP_ESCAPE;
                    iso->lead = byte;

                    break;
                }

                (*data)--;

                iso->out_flag = false;
                iso->state = ctx->u.iso_2022_jp.out_state;

                return LXB_ENCODING_DECODE_ERROR;

            case LXB_ENCODING_DECODE_2022_JP_ESCAPE:
                iso->state = LXB_ENCODING_DECODE_2022_JP_UNSET;

                if (iso->lead == 0x28) {
                    if (byte == 0x42) {
                        iso->state = LXB_ENCODING_DECODE_2022_JP_ASCII;
                    }
                    else if (byte == 0x4A) {
                        iso->state = LXB_ENCODING_DECODE_2022_JP_ROMAN;
                    }
                    else if (byte == 0x49) {
                        iso->state = LXB_ENCODING_DECODE_2022_JP_KATAKANA;
                    }
                }
                else if (iso->lead == 0x24) {
                    if (byte == 0x40 || byte == 0x42) {
                        iso->state = LXB_ENCODING_DECODE_2022_JP_LEAD;
                    }
                }

                if (iso->state == LXB_ENCODING_DECODE_2022_JP_UNSET) {
                    iso->prepand = iso->lead;
                    iso->lead = 0x00;

                    (*data)--;

                    iso->out_flag = false;
                    iso->state = iso->out_state;

                    return LXB_ENCODING_DECODE_ERROR;
                }

                iso->lead = 0x00;
                iso->out_state = iso->state;

                if (iso->out_flag) {
                    return LXB_ENCODING_DECODE_ERROR;
                }

                iso->out_flag = true;

                break;
        }
    }
    while (*data < end);

    return LXB_ENCODING_DECODE_CONTINUE;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_10(lxb_encoding_decode_t *ctx,
                                const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_10[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_13(lxb_encoding_decode_t *ctx,
                                const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_13[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_14(lxb_encoding_decode_t *ctx,
                                const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_14[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_15(lxb_encoding_decode_t *ctx,
                                const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_15[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_16(lxb_encoding_decode_t *ctx,
                                const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_16[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_2(lxb_encoding_decode_t *ctx,
                               const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_2[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_3(lxb_encoding_decode_t *ctx,
                               const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_3[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_4(lxb_encoding_decode_t *ctx,
                               const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_4[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_5(lxb_encoding_decode_t *ctx,
                               const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_5[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_6(lxb_encoding_decode_t *ctx,
                               const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_6[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_7(lxb_encoding_decode_t *ctx,
                               const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_7[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_8(lxb_encoding_decode_t *ctx,
                               const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_8[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_iso_8859_8_i(lxb_encoding_decode_t *ctx,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_iso_8859_8[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_koi8_r(lxb_encoding_decode_t *ctx,
                           const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_koi8_r[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_koi8_u(lxb_encoding_decode_t *ctx,
                           const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_koi8_u[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_shift_jis(lxb_encoding_decode_t *ctx,
                              const lxb_char_t **data, const lxb_char_t *end)
{
    lxb_char_t byte, lead;

    if (ctx->u.lead != 0x00) {
        lead = (lxb_char_t) ctx->u.lead;
        ctx->u.lead = 0x00;

        goto lead_state;
    }

    lead = *(*data)++;

    if (lead <= 0x80) {
        return lead;
    }

    if ((unsigned) (lead - 0xA1) <= (0xDF - 0xA1)) {
        return 0xFF61 - 0xA1 + lead;
    }

    if ((unsigned) (lead - 0x81) > (0x9F - 0x81)
        && lead != 0xE0 && lead != 0xFC)
    {
        return LXB_ENCODING_DECODE_ERROR;
    }

    if (*data >= end) {
        ctx->u.lead = lead;

        return LXB_ENCODING_DECODE_CONTINUE;
    }

lead_state:

    byte = *(*data)++;

    if (byte < 0x7F) {
        ctx->codepoint = 0x40;
    }
    else {
        ctx->codepoint = 0x41;
    }

    if (lead < 0xA0) {
        ctx->second_codepoint = 0x81;
    }
    else {
        ctx->second_codepoint = 0xC1;
    }

    if ((unsigned) (byte - 0x40) <= (0x7E - 0x40)
        || (unsigned) (byte - 0x80) <= (0xFC - 0x80))
    {
        /* Max index == (0xFC - 0xC1) * 188 + 0xFC - 0x41 = 11279 */
        ctx->codepoint = (lead - ctx->second_codepoint) * 188
                          + byte - ctx->codepoint;

        if (ctx->codepoint >= (sizeof(lxb_encoding_multi_index_jis0208)
            / sizeof(lxb_encoding_multi_index_t)))
        {
            goto failed;
        }

        if ((unsigned) (ctx->codepoint - 8836) <= (10715 - 8836)) {
            return 0xE000 - 8836 + ctx->codepoint;
        }

        ctx->codepoint = lxb_encoding_multi_index_jis0208[ctx->codepoint].codepoint;
        if (ctx->codepoint == LXB_ENCODING_DECODE_ERROR) {
            if (byte < 0x80) {
                (*data)--;
            }
        }

        return ctx->codepoint;
    }

failed:

    if (byte < 0x80) {
        (*data)--;
    }

    return LXB_ENCODING_DECODE_ERROR;
}

lxb_inline lxb_codepoint_t
lxb_encoding_decode_utf_16(lxb_encoding_decode_t *ctx, bool is_be,
                           const lxb_char_t **data, const lxb_char_t *end)
{
    unsigned lead;
    lxb_codepoint_t unit;

    if (ctx->u.lead != 0x00) {
        lead = ctx->u.lead - 0x01;
        ctx->u.lead = 0x00;

        goto lead_state;
    }

pair_state:

    lead = *(*data)++;

    if (*data >= end) {
        ctx->u.lead = lead + 0x01;
        return LXB_ENCODING_DECODE_CONTINUE;
    }

lead_state:

    /* For UTF-16BE or UTF-16LE */
    if (is_be) {
        unit = (lead << 8) + *(*data)++;
    }
    else {
        unit = (*(*data)++ << 8) + lead;
    }

    if (ctx->second_codepoint != 0x00) {
        if ((unsigned) (unit - 0xDC00) <= (0xDFFF - 0xDC00)) {
            ctx->codepoint = 0x10000 + ((ctx->second_codepoint - 0xD800) << 10)
                             + (unit - 0xDC00);

            ctx->second_codepoint = 0x00;
            return ctx->codepoint;
        }

        (*data)--;

        ctx->u.lead = lead + 0x01;
        ctx->second_codepoint = 0x00;

        return LXB_ENCODING_DECODE_ERROR;
    }

    /* Surrogate pair */
    if ((unsigned) (unit - 0xD800) <= (0xDFFF - 0xD800)) {
        if ((unsigned) (unit - 0xDC00) <= (0xDFFF - 0xDC00)) {
            return LXB_ENCODING_DECODE_ERROR;
        }

        ctx->second_codepoint = unit;

        if (*data >= end) {
            return LXB_ENCODING_DECODE_CONTINUE;
        }

        goto pair_state;
    }

    return unit;
}

lxb_codepoint_t
lxb_encoding_decode_utf_16be(lxb_encoding_decode_t *ctx,
                             const lxb_char_t **data, const lxb_char_t *end)
{
    return lxb_encoding_decode_utf_16(ctx, true, data, end);
}

lxb_codepoint_t
lxb_encoding_decode_utf_16le(lxb_encoding_decode_t *ctx,
                             const lxb_char_t **data, const lxb_char_t *end)
{
    return lxb_encoding_decode_utf_16(ctx, false, data, end);
}

lxb_codepoint_t
lxb_encoding_decode_utf_8(lxb_encoding_decode_t *ctx,
                          const lxb_char_t **data, const lxb_char_t *end)
{
    unsigned needed;
    lxb_char_t ch;
    const lxb_char_t *p;

    if (ctx->u.utf_8.needed != 0) {
        needed = ctx->u.utf_8.needed;

        if (ctx->u.utf_8.lower != 0x00) {
            LXB_ENCODING_DECODE_UTF_8_BOUNDARY(ctx->u.utf_8.lower,
                                               ctx->u.utf_8.upper);
            ctx->u.utf_8.lower = 0x00;
        }

        goto decode;
    }

    ch = *(*data)++;

    if (ch < 0x80) {
        return ch;
    }
    else if (ch <= 0xDF) {
        if (ch < 0xC2) {
            return LXB_ENCODING_DECODE_ERROR;
        }

        needed = 1;
        ctx->codepoint = ch & 0x1F;
    }
    else if (ch < 0xF0) {
        needed = 2;
        ctx->codepoint = ch & 0x0F;

        if (*data == end) {
            LXB_ENCODING_DECODE_UTF_8_BOUNDARY_SET(0xE0, 0xED, 0xA0, 0x9F);

            goto next;
        }

        if (ch == 0xE0) {
            LXB_ENCODING_DECODE_UTF_8_BOUNDARY(0xA0, 0xBF);
        }
        else if (ch == 0xED) {
            LXB_ENCODING_DECODE_UTF_8_BOUNDARY(0x80, 0x9F);
        }
    }
    else if (ch < 0xF5) {
        needed = 3;
        ctx->codepoint = ch & 0x07;

        if (*data == end) {
            LXB_ENCODING_DECODE_UTF_8_BOUNDARY_SET(0xF0, 0xF4, 0x90, 0x8F);

            goto next;
        }

        if (ch == 0xF0) {
            LXB_ENCODING_DECODE_UTF_8_BOUNDARY(0x90, 0xBF);
        }
        else if (ch == 0xF4) {
            LXB_ENCODING_DECODE_UTF_8_BOUNDARY(0x80, 0x8F);
        }
    }
    else {
        return LXB_ENCODING_DECODE_ERROR;
    }

decode:

    for (p = *data; p < end; p++) {
        ch = *p;

        if (ch < 0x80 || ch > 0xBF) {
            *data = p;

            goto failed;
        }

        ctx->codepoint = (ctx->codepoint << 6) | (ch & 0x3F);

        if (--needed == 0) {
            *data = p + 1;

            return ctx->codepoint;
        }
    }

    *data = p;

next:

    ctx->u.utf_8.needed = needed;

    return LXB_ENCODING_DECODE_CONTINUE;

failed:

    ctx->u.utf_8.lower = 0x00;
    ctx->u.utf_8.needed = 0;

    return LXB_ENCODING_DECODE_ERROR;
}

lxb_inline lxb_codepoint_t
lxb_encoding_decode_gb18030_range(uint32_t index)
{
    size_t mid, left, right;
    const lxb_encoding_range_index_t *range;

    /*
     * Pointer greater than 39419 and less than 189000,
     * or pointer is greater than 1237575
     */
    if ((unsigned) (index - 39419) < (189000 - 39419)
        || index > 1237575)
    {
        return LXB_ENCODING_DECODE_ERROR;
    }

    if (index == 7457) {
        return 0xE7C7;
    }

    left = 0;
    right = LXB_ENCODING_RANGE_INDEX_GB18030_SIZE;
    range = lxb_encoding_range_index_gb18030;

    /* Some compilers say about uninitialized mid */
    mid = 0;

    while (left < right) {
        mid = left + (right - left) / 2;

        if (range[mid].index < index) {
            left = mid + 1;

            if (left < right && range[ left ].index > index) {
                break;
            }
        }
        else if (range[mid].index > index) {
            right = mid - 1;

            if (right > 0 && range[right].index <= index) {
                mid = right;
                break;
            }
        }
        else {
            break;
        }
    }

    return range[mid].codepoint + index - range[mid].index;
}

lxb_codepoint_t
lxb_encoding_decode_gb18030(lxb_encoding_decode_t *ctx,
                            const lxb_char_t **data, const lxb_char_t *end)
{
    uint32_t pointer;
    lxb_char_t first, second, third, offset;

    /* Make compiler happy */
    second = 0x00;

    if (ctx->u.gb18030.first != 0) {
        if (ctx->u.gb18030.third != 0x00) {
            first = ctx->u.gb18030.first;
            second = ctx->u.gb18030.second;
            third = ctx->u.gb18030.third;

            memset(&ctx->u.gb18030, 0, sizeof(lxb_encoding_ctx_gb18030_t));

            if (ctx->prepend) {
                /* The first is always < 0x80 */
                ctx->u.gb18030.first = third;

                return second;
            }

            goto third_state;
        }
        else if (ctx->u.gb18030.second != 0x00) {
            first = ctx->u.gb18030.first;
            second = ctx->u.gb18030.second;

            memset(&ctx->u.gb18030, 0, sizeof(lxb_encoding_ctx_gb18030_t));

            goto second_state;
        }

        first = ctx->u.gb18030.first;
        ctx->u.gb18030.first = 0x00;

        if (ctx->prepend) {
            ctx->prepend = false;
            goto prepend_first;
        }

        goto first_state;
    }

    first = *(*data)++;

prepend_first:

    if (first < 0x80) {
        return first;
    }

    if (first == 0x80) {
        return 0x20AC;
    }

    /* Range 0x81 to 0xFE, inclusive */
    if ((unsigned) (first - 0x81) > (0xFE - 0x81)) {
        return LXB_ENCODING_DECODE_ERROR;
    }

    if (*data == end) {
        ctx->u.gb18030.first = first;
        return LXB_ENCODING_DECODE_CONTINUE;
    }

    /* First */
first_state:

    second = *(*data)++;

    /* Range 0x30 to 0x39, inclusive */
    if ((unsigned) (second - 0x30) > (0x39 - 0x30)) {
        offset = (second < 0x7F) ? 0x40 : 0x41;

        /* Range 0x40 to 0x7E, inclusive, or 0x80 to 0xFE, inclusive */
        if ((unsigned) (second - 0x40) <= (0x7E - 0x40)
            || (unsigned) (second - 0x80) <= (0xFE - 0x80))
        {
            pointer = (first - 0x81) * 190 + (second - offset);
        }
        else {
            goto failed;
        }

        /* Max pointer value == (0xFE - 0x81) * 190 + (0xFE - 0x41) == 23939 */
        ctx->codepoint = lxb_encoding_multi_index_gb18030[pointer].codepoint;
        if (ctx->codepoint == LXB_ENCODING_DECODE_ERROR) {
            goto failed;
        }

        return ctx->codepoint;
    }

    if (*data == end) {
        ctx->u.gb18030.first = first;
        ctx->u.gb18030.second = second;

        return LXB_ENCODING_DECODE_CONTINUE;
    }

    /* Second */
second_state:

    third = *(*data)++;

    /* Range 0x81 to 0xFE, inclusive */
    if ((unsigned) (third - 0x81) > (0xFE - 0x81)) {
        (*data)--;

        ctx->prepend = true;
        ctx->u.gb18030.first = second;

        return LXB_ENCODING_DECODE_ERROR;
    }

    if (*data == end) {
        ctx->u.gb18030.first = first;
        ctx->u.gb18030.second = second;
        ctx->u.gb18030.third = third;

        return LXB_ENCODING_DECODE_CONTINUE;
    }

    /* Third */
third_state:

    /* Range 0x30 to 0x39, inclusive */
    if ((unsigned) (**data - 0x30) > (0x39 - 0x30)) {
        ctx->prepend = true;

        /* First is a fake for trigger */
        ctx->u.gb18030.first = 0x01;
        ctx->u.gb18030.second = second;
        ctx->u.gb18030.third = third;

        return LXB_ENCODING_DECODE_ERROR;
    }

    pointer = ((first  - 0x81) * (10 * 126 * 10))
            + ((second - 0x30) * (10 * 126))
            + ((third  - 0x81) * 10) + (*(*data)++) - 0x30;

    return lxb_encoding_decode_gb18030_range(pointer);

failed:

    if (second < 0x80) {
        (*data)--;
    }

    return LXB_ENCODING_DECODE_ERROR;
}

lxb_codepoint_t
lxb_encoding_decode_macintosh(lxb_encoding_decode_t *ctx,
                              const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_macintosh[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_replacement(lxb_encoding_decode_t *ctx,
                                const lxb_char_t **data, const lxb_char_t *end)
{
    return LXB_ENCODING_DECODE_ERROR;
}

lxb_codepoint_t
lxb_encoding_decode_windows_1250(lxb_encoding_decode_t *ctx,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_windows_1250[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_windows_1251(lxb_encoding_decode_t *ctx,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_windows_1251[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_windows_1252(lxb_encoding_decode_t *ctx,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_windows_1252[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_windows_1253(lxb_encoding_decode_t *ctx,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_windows_1253[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_windows_1254(lxb_encoding_decode_t *ctx,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_windows_1254[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_windows_1255(lxb_encoding_decode_t *ctx,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_windows_1255[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_windows_1256(lxb_encoding_decode_t *ctx,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_windows_1256[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_windows_1257(lxb_encoding_decode_t *ctx,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_windows_1257[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_windows_1258(lxb_encoding_decode_t *ctx,
                                 const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_windows_1258[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_windows_874(lxb_encoding_decode_t *ctx,
                                const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_windows_874[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_x_mac_cyrillic(lxb_encoding_decode_t *ctx,
                                   const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return lxb_encoding_single_index_x_mac_cyrillic[*(*data)++ - 0x80].codepoint;
}

lxb_codepoint_t
lxb_encoding_decode_x_user_defined(lxb_encoding_decode_t *ctx,
                                   const lxb_char_t **data, const lxb_char_t *end)
{
    if (**data < 0x80) {
        return *(*data)++;
    }

    return 0xF780 + (*(*data)++) - 0x80;
}
