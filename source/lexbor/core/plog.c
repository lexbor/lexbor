/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "lexbor/core/plog.h"


lxb_status_t
lexbor_plog_init(lexbor_plog_t *plog, size_t init_size, size_t struct_size)
{
    lxb_status_t status;

    if (plog == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    if (struct_size < sizeof(lexbor_plog_entry_t)) {
        struct_size = sizeof(lexbor_plog_entry_t);
    }

    status = lexbor_array_obj_init(&plog->list, init_size, struct_size);
    if (status != LXB_STATUS_OK) {
        return status;
    }

    return LXB_STATUS_OK;
}

lexbor_plog_t *
lexbor_plog_destroy(lexbor_plog_t *plog, bool self_destroy)
{
    if (plog == NULL) {
        return NULL;
    }

    lexbor_array_obj_destroy(&plog->list, false);

    if (self_destroy) {
        return lexbor_free(plog);
    }

    return plog;
}
