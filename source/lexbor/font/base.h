/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_BASE_H
#define LEXBOR_FONT_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/base.h"
#include "lexbor/font/font.h"

#define LXB_FONT_VERSION_MAJOR 0
#define LXB_FONT_VERSION_MINOR 1
#define LXB_FONT_VERSION_PATCH 0

#define LXB_FONT_VERSION_STRING                                                \
        LEXBOR_STRINGIZE(LXB_FONT_VERSION_MAJOR) "."                           \
        LEXBOR_STRINGIZE(LXB_FONT_VERSION_MINOR) "."                           \
        LEXBOR_STRINGIZE(LXB_FONT_VERSION_PATCH)

#define LXB_FONT_ALLOC(count_, type_)                                          \
    lexbor_mraw_calloc(mf->mraw, (count_) * sizeof(type_))


lxb_inline void *
lxb_font_failed(lxb_font_t *mf, lxb_status_t status)
{
    mf->status = status;
    return NULL;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_BASE_H */
