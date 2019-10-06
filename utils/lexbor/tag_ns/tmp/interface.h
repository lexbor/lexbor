/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef LEXBOR_%%M_PREFIX%%_%%M_NAME%%_H
#define LEXBOR_%%M_PREFIX%%_%%M_NAME%%_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/%%prefix%%/interface.h"
%%includes%%

struct lxb_%%prefix%%_%%name%% {
    %%vars%%
};


%%type%% *
lxb_%%prefix%%_%%name%%_create(lxb_%%prefix%%_document_t *document);

%%type%% *
lxb_%%prefix%%_%%name%%_destroy(%%type%% *%%name%%);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_%%M_PREFIX%%_%%M_NAME%%_H */
