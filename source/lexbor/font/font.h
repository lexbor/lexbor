/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_FONT_H
#define LEXBOR_FONT_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/core/mraw.h"
/* Required */
#include "lexbor/font/cmap.h"
#include "lexbor/font/head.h"
#include "lexbor/font/hhea.h"
#include "lexbor/font/maxp.h"
#include "lexbor/font/hmtx.h"
#include "lexbor/font/name.h"
#include "lexbor/font/os_2.h"
#include "lexbor/font/post.h"
/* Truetype */
#include "lexbor/font/loca.h"
#include "lexbor/font/glyf.h"


/*
 * TODO
 * X: done
 * n: not entirely done
 *
 * Required:
 * [n] cmap FIXME: format 2 not done
 * [X] head
 * [X] hhea
 * [X] hmtx
 * [X] maxp
 * [n] name FIXME: last array not done
 * [X] os_2
 * [n] post FIXME: str_data not done
 *
 * Truetype:
 * [ ] cvt
 * [ ] fpgm
 * [X] glyf
 * [X] loca
 * [ ] prep
 * [ ] gasp
 */

typedef enum {
    /* Required Tables. */
    LXB_FONT_TABLE_TYPE_CMAP = 1885433187,
    LXB_FONT_TABLE_TYPE_HEAD = 1684104552,
    LXB_FONT_TABLE_TYPE_HHEA = 1634035816,
    LXB_FONT_TABLE_TYPE_HMTX = 2020896104,
    LXB_FONT_TABLE_TYPE_MAXP = 1886937453,
    LXB_FONT_TABLE_TYPE_NAME = 1701667182,
    LXB_FONT_TABLE_TYPE_OS_2 = 841962319,   // OS/2
    LXB_FONT_TABLE_TYPE_POST = 1953722224,

    /* Tables Related to TrueType Outlines. */
    LXB_FONT_TABLE_TYPE_CVT  = 1953915648,
    LXB_FONT_TABLE_TYPE_FPGM = 1835495526,
    LXB_FONT_TABLE_TYPE_GLYF = 1719233639,
    LXB_FONT_TABLE_TYPE_LOCA = 1633906540,
    LXB_FONT_TABLE_TYPE_PREP = 1885696624,
    LXB_FONT_TABLE_TYPE_GASP = 1886609767,

    /* Tables Related to PostScript Outlines. */
    LXB_FONT_TABLE_TYPE_CFF  = 1179009792,
    LXB_FONT_TABLE_TYPE_VORG = 1196576598,

    /* Tables Related to SVG. */
    LXB_FONT_TABLE_TYPE_SVG  = 1196839680,

    /* Tables Related to Bitmap Glyphs. */
    LXB_FONT_TABLE_TYPE_EBDT = 1413759557,
    LXB_FONT_TABLE_TYPE_EBLC = 1129071173,
    LXB_FONT_TABLE_TYPE_EBSC = 1129529925,
    LXB_FONT_TABLE_TYPE_CBDT = 1413759555,
    LXB_FONT_TABLE_TYPE_CBLC = 1129071171,

    /* Advanced Typographic Tables. */
    LXB_FONT_TABLE_TYPE_BASE = 1163084098,
    LXB_FONT_TABLE_TYPE_GDEF = 1178944583,
    LXB_FONT_TABLE_TYPE_GPOS = 1397706823,
    LXB_FONT_TABLE_TYPE_GSUB = 1112888135,
    LXB_FONT_TABLE_TYPE_JSTF = 1179931466,
    LXB_FONT_TABLE_TYPE_MATH = 1213481293,

    /* Other OpenType Tables. */
    LXB_FONT_TABLE_TYPE_DSIG = 1195987780,
    LXB_FONT_TABLE_TYPE_HDMX = 2020435048,
    LXB_FONT_TABLE_TYPE_KERN = 1852990827,
    LXB_FONT_TABLE_TYPE_LTSH = 1213420620,
    LXB_FONT_TABLE_TYPE_PCLT = 1414284112,
    LXB_FONT_TABLE_TYPE_VDMX = 1481458774,
    LXB_FONT_TABLE_TYPE_VHEA = 1634035830,
    LXB_FONT_TABLE_TYPE_VMTX = 2020896118,
    LXB_FONT_TABLE_TYPE_COLR = 1380732739,
    LXB_FONT_TABLE_TYPE_CPAL = 1279348803
}
lxb_font_table_t;

typedef enum {
    LXB_FONT_TKEY_CMAP,
    LXB_FONT_TKEY_HEAD,
    LXB_FONT_TKEY_HHEA,
    LXB_FONT_TKEY_HMTX,
    LXB_FONT_TKEY_MAXP,
    LXB_FONT_TKEY_NAME,
    LXB_FONT_TKEY_OS_2,
    LXB_FONT_TKEY_POST,
    LXB_FONT_TKEY_CVT ,
    LXB_FONT_TKEY_FPGM,
    LXB_FONT_TKEY_GLYF,
    LXB_FONT_TKEY_LOCA,
    LXB_FONT_TKEY_PREP,
    LXB_FONT_TKEY_GASP,
    LXB_FONT_TKEY_CFF ,
    LXB_FONT_TKEY_VORG,
    LXB_FONT_TKEY_SVG ,
    LXB_FONT_TKEY_EBDT,
    LXB_FONT_TKEY_EBLC,
    LXB_FONT_TKEY_EBSC,
    LXB_FONT_TKEY_CBDT,
    LXB_FONT_TKEY_CBLC,
    LXB_FONT_TKEY_BASE,
    LXB_FONT_TKEY_GDEF,
    LXB_FONT_TKEY_GPOS,
    LXB_FONT_TKEY_GSUB,
    LXB_FONT_TKEY_JSTF,
    LXB_FONT_TKEY_MATH,
    LXB_FONT_TKEY_DSIG,
    LXB_FONT_TKEY_HDMX,
    LXB_FONT_TKEY_KERN,
    LXB_FONT_TKEY_LTSH,
    LXB_FONT_TKEY_PCLT,
    LXB_FONT_TKEY_VDMX,
    LXB_FONT_TKEY_VHEA,
    LXB_FONT_TKEY_VMTX,
    LXB_FONT_TKEY_COLR,
    LXB_FONT_TKEY_CPAL,
    LXB_FONT_TKEY_LAST__ENTRY
}
lxb_font_table_key_t;

typedef struct {
    uint32_t sfnt_version;
    uint16_t num_tables;
    uint16_t search_range;
    uint16_t entry_selector;
    uint16_t range_shift;
}
lxb_font_offset_table_t;

typedef struct {
    uint32_t table_tag;
    uint32_t checksum;
    uint32_t offset;
    uint32_t length;
}
lxb_font_table_record_t;

typedef struct lxb_font_s {
    /* TrueType or CFF OpenType. */
    uint8_t               is_truetype;
    /* Table record count. */
    uint16_t              num_tables;

    lxb_status_t          status;

    struct {
        uint32_t tables_offset[LXB_FONT_TKEY_LAST__ENTRY];
    } cache;

    lexbor_mraw_t         *mraw;

    /* Required tables. */
    lxb_font_table_cmap_t *table_cmap;
    lxb_font_table_head_t *table_head;
    lxb_font_table_hhea_t *table_hhea;
    lxb_font_table_hmtx_t *table_hmtx;
    lxb_font_table_maxp_t *table_maxp;
    lxb_font_table_name_t *table_name;
    lxb_font_table_os_2_t *table_os_2;
    lxb_font_table_post_t *table_post;

    /* TrueType tables. */
    lxb_font_table_loca_t *table_glyf;
    lxb_font_table_loca_t *table_loca;
}
lxb_font_t;


LXB_API lxb_font_t *
lxb_font_create(void);

LXB_API lxb_status_t
lxb_font_init(lxb_font_t *mf);

LXB_API void
lxb_font_clean(lxb_font_t *mf);

LXB_API lxb_font_t *
lxb_font_destroy(lxb_font_t *mf, bool self_destroy);


LXB_API lxb_status_t
lxb_font_load_from_file(lxb_font_t *mf, const lxb_char_t *filepath,
                        uint8_t **out_data, size_t *out_size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_FONT_H */
