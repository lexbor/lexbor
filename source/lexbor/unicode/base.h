/*
 * Copyright (C) 2023 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_UNICODE_BASE_H
#define LEXBOR_UNICODE_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/base.h"
#include "lexbor/core/str.h"


#define LXB_UNICODE_VERSION_MAJOR 0
#define LXB_UNICODE_VERSION_MINOR 1
#define LXB_UNICODE_VERSION_PATCH 0

#define LXB_UNICODE_VERSION_STRING LEXBOR_STRINGIZE(LXB_UNICODE_VERSION_MAJOR) "." \
                                   LEXBOR_STRINGIZE(LXB_UNICODE_VERSION_MINOR) "." \
                                   LEXBOR_STRINGIZE(LXB_UNICODE_VERSION_PATCH)


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_UNICODE_BASE_H */
