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
/* TrueType */
#include "lexbor/font/cvt.h"
#include "lexbor/font/fpgm.h"
#include "lexbor/font/gasp.h"
#include "lexbor/font/loca.h"
#include "lexbor/font/glyf.h"
#include "lexbor/font/prep.h"
/* CFF outlines */
/* SVG */
#include "lexbor/font/svg.h"
/* Bitmap glyphs */
#include "lexbor/font/ebsc.h"
#include "lexbor/font/sbix.h"
/* Advanced typographic */
/* OpenType font variations */
#include "lexbor/font/fvar.h"
#include "lexbor/font/mvar.h"
/* Color fonts */
/* Other OpentType */

#define LXB_FONT_MAKE_TAG_ID(c1_, c2_, c3_, c4_)                               \
    ((((uint32_t) (c4_)) << 24) |                                              \
     (((uint32_t) (c3_)) << 16) |                                              \
     (((uint32_t) (c2_)) <<  8) |                                              \
     (((uint32_t) (c1_))      ))


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
 * [X] cvt
 * [X] fpgm
 * [X] glyf
 * [X] loca
 * [X] prep
 * [X] gasp
 *
 * CFF outlines
 * [ ] cff
 * [ ] cff2
 * [ ] vorg
 *
 * SVG
 * [X] svg
 *
 * Bitmap glyphs
 * [ ] ebdt
 * [ ] eblc
 * [X] ebsc
 * [ ] cbdt
 * [ ] cblc
 * [X] sbix
 *
 * Advanced typographic
 * [ ] base
 * [ ] gdef
 * [ ] gpos
 * [ ] gsub
 * [ ] jstf
 * [ ] math
 *
 * OpenType font variations
 * [ ] avar
 * [ ] cvar
 * [X] fvar
 * [ ] gvar
 * [ ] hvar
 * [X] mvar
 * [ ] stat
 * [ ] vvar
 *
 * Color fonts
 * [ ] colr
 * [ ] cpal
 * [ ] cbdt (see Bitmap glyphs)
 * [ ] cblc (see Bitmap glyphs)
 * [X] sbix (see Bitmap glyphs)
 * [X] svg  (see SVG)
 *
 * Other OpentType
 * [ ] dsig
 * [ ] hdmx
 * [ ] kern
 * [ ] ltsh
 * [ ] merg
 * [ ] meta
 * [ ] stat (see OpenType font variations)
 * [ ] pclt
 * [ ] vdmx
 * [ ] vhea
 * [ ] vmtx
 */

typedef enum {
    /* Required Tables. */
    LXB_FONT_TABLE_TAG_ID_CMAP = LXB_FONT_MAKE_TAG_ID('c', 'm', 'a', 'p'),
    LXB_FONT_TABLE_TAG_ID_HEAD = LXB_FONT_MAKE_TAG_ID('h', 'e', 'a', 'd'),
    LXB_FONT_TABLE_TAG_ID_HHEA = LXB_FONT_MAKE_TAG_ID('h', 'h', 'e', 'a'),
    LXB_FONT_TABLE_TAG_ID_HMTX = LXB_FONT_MAKE_TAG_ID('h', 'm', 't', 'x'),
    LXB_FONT_TABLE_TAG_ID_MAXP = LXB_FONT_MAKE_TAG_ID('m', 'a', 'x', 'p'),
    LXB_FONT_TABLE_TAG_ID_NAME = LXB_FONT_MAKE_TAG_ID('n', 'a', 'm', 'e'),
    LXB_FONT_TABLE_TAG_ID_OS_2 = LXB_FONT_MAKE_TAG_ID('O', 'S', '/', '2'),
    LXB_FONT_TABLE_TAG_ID_POST = LXB_FONT_MAKE_TAG_ID('p', 'o', 's', 't'),

    /* Tables Related to TrueType Outlines. */
    LXB_FONT_TABLE_TAG_ID_CVT  = LXB_FONT_MAKE_TAG_ID('c', 'v', 't', ' '),
    LXB_FONT_TABLE_TAG_ID_FPGM = LXB_FONT_MAKE_TAG_ID('f', 'p', 'g', 'm'),
    LXB_FONT_TABLE_TAG_ID_GLYF = LXB_FONT_MAKE_TAG_ID('g', 'l', 'y', 'f'),
    LXB_FONT_TABLE_TAG_ID_LOCA = LXB_FONT_MAKE_TAG_ID('l', 'o', 'c', 'a'),
    LXB_FONT_TABLE_TAG_ID_PREP = LXB_FONT_MAKE_TAG_ID('p', 'r', 'e', 'p'),
    LXB_FONT_TABLE_TAG_ID_GASP = LXB_FONT_MAKE_TAG_ID('g', 'a', 's', 'p'),

    /* Tables Related to CFF Outlines. */
    LXB_FONT_TABLE_TAG_ID_CFF  = LXB_FONT_MAKE_TAG_ID('C', 'F', 'F', ' '),
    LXB_FONT_TABLE_TAG_ID_CFF2 = LXB_FONT_MAKE_TAG_ID('C', 'F', 'F', '2'),
    LXB_FONT_TABLE_TAG_ID_VORG = LXB_FONT_MAKE_TAG_ID('V', 'O', 'R', 'G'),

    /* Tables Related to SVG. */
    LXB_FONT_TABLE_TAG_ID_SVG  = LXB_FONT_MAKE_TAG_ID('S', 'V', 'G', ' '),

    /* Tables Related to Bitmap Glyphs. */
    LXB_FONT_TABLE_TAG_ID_EBDT = LXB_FONT_MAKE_TAG_ID('E', 'B', 'D', 'T'),
    LXB_FONT_TABLE_TAG_ID_EBLC = LXB_FONT_MAKE_TAG_ID('E', 'B', 'L', 'C'),
    LXB_FONT_TABLE_TAG_ID_EBSC = LXB_FONT_MAKE_TAG_ID('E', 'B', 'S', 'C'),
    LXB_FONT_TABLE_TAG_ID_CBDT = LXB_FONT_MAKE_TAG_ID('C', 'B', 'D', 'T'),
    LXB_FONT_TABLE_TAG_ID_CBLC = LXB_FONT_MAKE_TAG_ID('C', 'B', 'L', 'C'),
    LXB_FONT_TABLE_TAG_ID_SBIX = LXB_FONT_MAKE_TAG_ID('s', 'b', 'i', 'x'),

    /* Advanced Typographic Tables. */
    LXB_FONT_TABLE_TAG_ID_BASE = LXB_FONT_MAKE_TAG_ID('B', 'A', 'S', 'E'),
    LXB_FONT_TABLE_TAG_ID_GDEF = LXB_FONT_MAKE_TAG_ID('G', 'D', 'E', 'F'),
    LXB_FONT_TABLE_TAG_ID_GPOS = LXB_FONT_MAKE_TAG_ID('G', 'P', 'O', 'S'),
    LXB_FONT_TABLE_TAG_ID_GSUB = LXB_FONT_MAKE_TAG_ID('G', 'S', 'U', 'B'),
    LXB_FONT_TABLE_TAG_ID_JSTF = LXB_FONT_MAKE_TAG_ID('J', 'S', 'T', 'F'),
    LXB_FONT_TABLE_TAG_ID_MATH = LXB_FONT_MAKE_TAG_ID('M', 'A', 'T', 'H'),

    /* OpenType Font Variation Tables. */
    LXB_FONT_TABLE_TAG_ID_AVAR = LXB_FONT_MAKE_TAG_ID('A', 'V', 'A', 'R'),
    LXB_FONT_TABLE_TAG_ID_CVAR = LXB_FONT_MAKE_TAG_ID('C', 'V', 'A', 'R'),
    LXB_FONT_TABLE_TAG_ID_FVAR = LXB_FONT_MAKE_TAG_ID('F', 'V', 'A', 'R'),
    LXB_FONT_TABLE_TAG_ID_GVAR = LXB_FONT_MAKE_TAG_ID('G', 'V', 'A', 'R'),
    LXB_FONT_TABLE_TAG_ID_HVAR = LXB_FONT_MAKE_TAG_ID('H', 'V', 'A', 'R'),
    LXB_FONT_TABLE_TAG_ID_MVAR = LXB_FONT_MAKE_TAG_ID('M', 'V', 'A', 'R'),
    LXB_FONT_TABLE_TAG_ID_STAT = LXB_FONT_MAKE_TAG_ID('S', 'T', 'A', 'T'),
    LXB_FONT_TABLE_TAG_ID_VVAR = LXB_FONT_MAKE_TAG_ID('V', 'V', 'A', 'R'),

    /* Color Fonts Related Tables. */
    LXB_FONT_TABLE_TAG_ID_COLR = LXB_FONT_MAKE_TAG_ID('C', 'O', 'L', 'R'),
    LXB_FONT_TABLE_TAG_ID_CPAL = LXB_FONT_MAKE_TAG_ID('C', 'P', 'A', 'L'),

    /* Other OpenType Tables */
    LXB_FONT_TABLE_TAG_ID_DSIG = LXB_FONT_MAKE_TAG_ID('D', 'S', 'I', 'G'),
    LXB_FONT_TABLE_TAG_ID_HDMX = LXB_FONT_MAKE_TAG_ID('h', 'd', 'm', 'x'),
    LXB_FONT_TABLE_TAG_ID_KERN = LXB_FONT_MAKE_TAG_ID('k', 'e', 'r', 'n'),
    LXB_FONT_TABLE_TAG_ID_LTSH = LXB_FONT_MAKE_TAG_ID('L', 'T', 'S', 'H'),
    LXB_FONT_TABLE_TAG_ID_MERG = LXB_FONT_MAKE_TAG_ID('M', 'E', 'R', 'G'),
    LXB_FONT_TABLE_TAG_ID_META = LXB_FONT_MAKE_TAG_ID('m', 'e', 't', 'a'),
    LXB_FONT_TABLE_TAG_ID_PCLT = LXB_FONT_MAKE_TAG_ID('P', 'C', 'L', 'T'),
    LXB_FONT_TABLE_TAG_ID_VDMX = LXB_FONT_MAKE_TAG_ID('V', 'E', 'M', 'X'),
    LXB_FONT_TABLE_TAG_ID_VHEA = LXB_FONT_MAKE_TAG_ID('v', 'h', 'e', 'a'),
    LXB_FONT_TABLE_TAG_ID_VMTX = LXB_FONT_MAKE_TAG_ID('v', 'm', 't', 'x')
}
lxb_font_table_tag_id_t;

typedef enum {
    /* Required Tables. */
    LXB_FONT_TKEY_CMAP,
    LXB_FONT_TKEY_HEAD,
    LXB_FONT_TKEY_HHEA,
    LXB_FONT_TKEY_HMTX,
    LXB_FONT_TKEY_MAXP,
    LXB_FONT_TKEY_NAME,
    LXB_FONT_TKEY_OS_2,
    LXB_FONT_TKEY_POST,
    /* Tables Related to TrueType Outlines. */
    LXB_FONT_TKEY_CVT ,
    LXB_FONT_TKEY_FPGM,
    LXB_FONT_TKEY_GLYF,
    LXB_FONT_TKEY_LOCA,
    LXB_FONT_TKEY_PREP,
    LXB_FONT_TKEY_GASP,
    /* Tables Related to CFF Outlines. */
    LXB_FONT_TKEY_CFF ,
    LXB_FONT_TKEY_CFF2,
    LXB_FONT_TKEY_VORG,
    /* Tables Related to SVG. */
    LXB_FONT_TKEY_SVG ,
    /* Tables Related to Bitmap Glyphs. */
    LXB_FONT_TKEY_EBDT,
    LXB_FONT_TKEY_EBLC,
    LXB_FONT_TKEY_EBSC,
    LXB_FONT_TKEY_CBDT,
    LXB_FONT_TKEY_CBLC,
    LXB_FONT_TKEY_SBIX,
    /* Advanced Typographic Tables. */
    LXB_FONT_TKEY_BASE,
    LXB_FONT_TKEY_GDEF,
    LXB_FONT_TKEY_GPOS,
    LXB_FONT_TKEY_GSUB,
    LXB_FONT_TKEY_JSTF,
    LXB_FONT_TKEY_MATH,
    /* OpenType Font Variation Tables. */
    LXB_FONT_TKEY_AVAR,
    LXB_FONT_TKEY_CVAR,
    LXB_FONT_TKEY_FVAR,
    LXB_FONT_TKEY_GVAR,
    LXB_FONT_TKEY_HVAR,
    LXB_FONT_TKEY_MVAR,
    LXB_FONT_TKEY_STAT,
    LXB_FONT_TKEY_VVAR,
    /* Color Fonts Related Tables. */
    LXB_FONT_TKEY_COLR,
    LXB_FONT_TKEY_CPAL,
    /* Other OpenType Tables */
    LXB_FONT_TKEY_DSIG,
    LXB_FONT_TKEY_HDMX,
    LXB_FONT_TKEY_KERN,
    LXB_FONT_TKEY_LTSH,
    LXB_FONT_TKEY_MERG,
    LXB_FONT_TKEY_META,
    LXB_FONT_TKEY_PCLT,
    LXB_FONT_TKEY_VDMX,
    LXB_FONT_TKEY_VHEA,
    LXB_FONT_TKEY_VMTX,

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
        uint32_t tables_length[LXB_FONT_TKEY_LAST__ENTRY];
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
    lxb_font_table_cvt_t  *table_cvt;
    lxb_font_table_fpgm_t *table_fpgm;
    lxb_font_table_gasp_t *table_gasp;
    lxb_font_table_glyf_t *table_glyf;
    lxb_font_table_loca_t *table_loca;
    lxb_font_table_prep_t *table_prep;

    /* CFF outlines tables. */

    /* SVG table. */
    lxb_font_table_svg_t  *table_svg;

    /* Bitmap glyphs tables. */
    lxb_font_table_ebsc_t *table_ebsc;
    lxb_font_table_sbix_t *table_sbix;

    /* Advanced typographic tables. */

    /* OpenType font variations. */
    lxb_font_table_fvar_t *table_fvar;
    lxb_font_table_mvar_t *table_mvar;

    /* Color fonts tables. */

    /* Other OpentType tables. */
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
