/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


lxb_font_table_cvt_t *
lxb_font_table_cvt(lxb_font_t *mf, uint8_t* font_data, size_t size)
{
    lxb_font_table_cvt_t *table;
    uint8_t *data;
    uint32_t offset;
    uint32_t length;
    uint32_t i;

    table = LXB_FONT_ALLOC(1, lxb_font_table_cvt_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    offset = mf->cache.tables_offset[LXB_FONT_TKEY_CVT];
    length = mf->cache.tables_length[LXB_FONT_TKEY_CVT];
    data = font_data + offset;

    if (size < (offset + length)) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }

    table->values = LXB_FONT_ALLOC(length / 2, int16_t);
    if (table->values == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    for (i = 0; i < (length / 2); i++) {
      table->values[i] = lxb_font_read_16(&data);
    }

    return table;
}
