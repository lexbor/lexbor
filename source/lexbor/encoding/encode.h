/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_ENCODING_ENCODE_H
#define LEXBOR_ENCODING_ENCODE_H

#ifdef __cplusplus
extern "C" {
#endif


#include "lexbor/encoding/base.h"


LXB_API int8_t
lxb_encoding_encode_default(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                            const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_auto(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                         const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_undefined(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                              const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_big5(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                         const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_euc_jp(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                           const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_euc_kr(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                           const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_gbk(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                        const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_ibm866(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                           const lxb_char_t *end, lxb_codepoint_t cp);

/*
 * Important!
 *
 * To properly complete the encoding, you must call the function
 * lxb_encoding_encode_iso_2022_jp_eof.
 */
LXB_API int8_t
lxb_encoding_encode_iso_2022_jp(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_2022_jp_eof(lxb_encoding_encode_t *ctx,
                                    lxb_char_t **data, const lxb_char_t *end);

LXB_API int8_t
lxb_encoding_encode_iso_8859_10(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_13(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_14(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_15(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_16(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_2(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                               const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_3(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                               const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_4(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                               const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_5(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                               const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_6(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                               const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_7(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                               const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_8(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                               const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_iso_8859_8_i(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                 const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_koi8_r(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                           const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_koi8_u(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                           const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_shift_jis(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                              const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_utf_16be(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                             const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_utf_16le(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                             const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_utf_8(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                          const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_gb18030(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                            const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_macintosh(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                              const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_replacement(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_windows_1250(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                 const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_windows_1251(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                 const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_windows_1252(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                 const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_windows_1253(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                 const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_windows_1254(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                 const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_windows_1255(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                 const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_windows_1256(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                 const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_windows_1257(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                 const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_windows_1258(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                 const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_windows_874(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_x_mac_cyrillic(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                   const lxb_char_t *end, lxb_codepoint_t cp);

LXB_API int8_t
lxb_encoding_encode_x_user_defined(lxb_encoding_encode_t *ctx, lxb_char_t **data,
                                   const lxb_char_t *end, lxb_codepoint_t cp);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_ENCODING_ENCODE_H */
