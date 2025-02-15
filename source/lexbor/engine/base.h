/*
 * Copyright (C) 2024-2025 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_ENGINE_BASE_H
#define LEXBOR_ENGINE_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/base.h"


#define LXB_ENGINE_VERSION_MAJOR 0
#define LXB_ENGINE_VERSION_MINOR 1
#define LXB_ENGINE_VERSION_PATCH 0

#define LXB_ENGINE_VERSION_STRING LEXBOR_STRINGIZE(LXB_ENGINE_VERSION_MAJOR) "."  \
        LEXBOR_STRINGIZE(LXB_ENGINE_VERSION_MINOR) "."  \
        LEXBOR_STRINGIZE(LXB_ENGINE_VERSION_PATCH)


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_ENGINE_BASE_H */
