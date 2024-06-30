/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/font/base.h"
#include "lexbor/font/read.h"
#include "lexbor/font/font.h"


lxb_font_t *
lxb_font_create(void)
{
    return lexbor_calloc(1, sizeof(lxb_font_t));
}

lxb_status_t
lxb_font_init(lxb_font_t *mf)
{
    if (mf == NULL) {
        return LXB_STATUS_ERROR_OBJECT_IS_NULL;
    }

    mf->mraw = lexbor_mraw_create();

    return lexbor_mraw_init(mf->mraw, 4096 * 4);
}

void
lxb_font_clean(lxb_font_t *mf)
{
    lexbor_mraw_t *mraw = mf->mraw;

    memset(mf, 0, sizeof(lxb_font_t));
    lexbor_mraw_clean(mraw);

    mf->mraw = mraw;
}

lxb_font_t *
lxb_font_destroy(lxb_font_t *mf, bool self_destroy)
{
    if (mf == NULL) {
        return NULL;
    }

    mf->mraw = lexbor_mraw_destroy(mf->mraw, true);

    if (self_destroy) {
        return lexbor_free(mf);
    }

    return mf;
}

static lxb_status_t
lxb_font_load_table(lxb_font_t *mf, uint8_t *font_data, size_t size)
{
    uint8_t *data;
    uint32_t tag;
    uint32_t offset;
    uint32_t length;

    /* Move to the beginning of the table records array. */
    data = font_data + 12;

    for (uint16_t i = 0; i < mf->num_tables; i++) {
        tag = lxb_font_read_u32_as_net(&data);
        /* FIXME: specification mentions a checksum test. Should we do it ? */
        lxb_font_read_u32(&data); /* checkSum */
        offset = lxb_font_read_u32(&data);
        length = lxb_font_read_u32(&data);


#define LXB_FONT_TABLE_CACHE(type_)                                            \
    case LXB_FONT_TABLE_TAG_ID_ ## type_:                                      \
        mf->cache.tables_offset[ LXB_FONT_TKEY_ ## type_ ] = offset;           \
        mf->cache.tables_length[ LXB_FONT_TKEY_ ## type_ ] = length;           \
        break


        switch (tag) {
            /* Required Tables. */
            LXB_FONT_TABLE_CACHE(CMAP);
            LXB_FONT_TABLE_CACHE(HEAD);
            LXB_FONT_TABLE_CACHE(HHEA);
            LXB_FONT_TABLE_CACHE(HMTX);
            LXB_FONT_TABLE_CACHE(MAXP);
            LXB_FONT_TABLE_CACHE(NAME);
            LXB_FONT_TABLE_CACHE(OS_2);
            LXB_FONT_TABLE_CACHE(POST);

            /* Tables Related to TrueType Outlines. */
            LXB_FONT_TABLE_CACHE(CVT);
            LXB_FONT_TABLE_CACHE(FPGM);
            LXB_FONT_TABLE_CACHE(GLYF);
            LXB_FONT_TABLE_CACHE(LOCA);
            LXB_FONT_TABLE_CACHE(PREP);
            LXB_FONT_TABLE_CACHE(GASP);

            /* Tables Related to CFF Outlines. */
            LXB_FONT_TABLE_CACHE(CFF);
            LXB_FONT_TABLE_CACHE(CFF2);
            LXB_FONT_TABLE_CACHE(VORG);

            /* Tables Related to SVG. */
            LXB_FONT_TABLE_CACHE(SVG);

            /* Tables Related to Bitmap Glyphs. */
            LXB_FONT_TABLE_CACHE(EBDT);
            LXB_FONT_TABLE_CACHE(EBLC);
            LXB_FONT_TABLE_CACHE(EBSC);
            LXB_FONT_TABLE_CACHE(CBDT);
            LXB_FONT_TABLE_CACHE(CBLC);
            LXB_FONT_TABLE_CACHE(SBIX);

            /* Advanced Typographic Tables. */
            LXB_FONT_TABLE_CACHE(BASE);
            LXB_FONT_TABLE_CACHE(GDEF);
            LXB_FONT_TABLE_CACHE(GPOS);
            LXB_FONT_TABLE_CACHE(GSUB);
            LXB_FONT_TABLE_CACHE(JSTF);
            LXB_FONT_TABLE_CACHE(MATH);

            /* OpenType Font Variations Tables. */
            LXB_FONT_TABLE_CACHE(AVAR);
            LXB_FONT_TABLE_CACHE(CVAR);
            LXB_FONT_TABLE_CACHE(FVAR);
            LXB_FONT_TABLE_CACHE(GVAR);
            LXB_FONT_TABLE_CACHE(HVAR);
            LXB_FONT_TABLE_CACHE(MVAR);
            LXB_FONT_TABLE_CACHE(STAT);
            LXB_FONT_TABLE_CACHE(VVAR);

            /* Color Fonts Tables. */
            LXB_FONT_TABLE_CACHE(COLR);
            LXB_FONT_TABLE_CACHE(CPAL);

            /* Other OpenType Tables. */
            LXB_FONT_TABLE_CACHE(DSIG);
            LXB_FONT_TABLE_CACHE(HDMX);
            LXB_FONT_TABLE_CACHE(KERN);
            LXB_FONT_TABLE_CACHE(LTSH);
            LXB_FONT_TABLE_CACHE(MERG);
            LXB_FONT_TABLE_CACHE(META);
            LXB_FONT_TABLE_CACHE(PCLT);
            LXB_FONT_TABLE_CACHE(VDMX);
            LXB_FONT_TABLE_CACHE(VHEA);
            LXB_FONT_TABLE_CACHE(VMTX);
        }


#undef LXB_FONT_TABLE_CACHE


    }

    /* Check that the required font tables are loaded. */
    for (int32_t i = LXB_FONT_TKEY_CMAP; i <= LXB_FONT_TKEY_POST; i++) {
        if (mf->cache.tables_offset[i] == 0) {
            return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
        }
    }

    /* Check that the TrueType font tables are loaded. */
    for (int32_t i = LXB_FONT_TKEY_CVT; i <= LXB_FONT_TKEY_GASP; i++) {
        if (mf->cache.tables_offset[i] == 0) {
            return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
        }
    }

    /* Check that the Bitmap glyphs font tables are loaded. */
    /* for (int32_t i = LXB_FONT_TKEY_EBSC; i <= LXB_FONT_TKEY_EBSC; i++) { */
    /*     if (mf->cache.tables_offset[i] == 0) { */
    /*         return LXB_STATUS_ERROR_INCOMPLETE_OBJECT; */
    /*     } */
    /* } */


#define LXB_FONT_TABLE_LOAD(type_)                                             \
    mf->table_ ## type_ = lxb_font_table_ ## type_(mf, font_data, size);       \
    if (mf->table_ ## type_ == NULL) {                                         \
        return mf->status;                                                     \
    }


    /* Required Tables. */
    LXB_FONT_TABLE_LOAD(cmap);
    LXB_FONT_TABLE_LOAD(head);
    LXB_FONT_TABLE_LOAD(hhea);
    LXB_FONT_TABLE_LOAD(maxp);
    /* hmtx table must be parsed after hhea and maxp tables. */
    LXB_FONT_TABLE_LOAD(hmtx);
    LXB_FONT_TABLE_LOAD(name);
    LXB_FONT_TABLE_LOAD(os_2);
    /* post table must be parsed after maxp table. */
    LXB_FONT_TABLE_LOAD(post);

    /* TrueType Tables. */
    LXB_FONT_TABLE_LOAD(cvt);
    LXB_FONT_TABLE_LOAD(fpgm);
    /* loca table must be parsed after head and maxp tables. */
    LXB_FONT_TABLE_LOAD(loca);
    /* glyf table must be parsed after maxp and loca tables */
    LXB_FONT_TABLE_LOAD(glyf);
    LXB_FONT_TABLE_LOAD(prep);
    LXB_FONT_TABLE_LOAD(gasp);

    /* CFF Outlines Tables. */

    /* SVG Table. */
    if (mf->cache.tables_offset[LXB_FONT_TKEY_SVG] != 0) {
        LXB_FONT_TABLE_LOAD(svg);
    }

    /* Bitmap glyphs Tables. */
    if (mf->cache.tables_offset[LXB_FONT_TKEY_EBSC] != 0) {
        LXB_FONT_TABLE_LOAD(ebsc);
    }
    if (mf->cache.tables_offset[LXB_FONT_TKEY_SBIX] != 0) {
        LXB_FONT_TABLE_LOAD(sbix);
    }

    /* Advanced Typographic Tables. */

    /* OpenType Font Variation Tables. */
    if (mf->cache.tables_offset[LXB_FONT_TKEY_FVAR] != 0) {
        LXB_FONT_TABLE_LOAD(fvar);
    }
    /* mvar table must be parsed after fvar table. */
    if (mf->cache.tables_offset[LXB_FONT_TKEY_MVAR] != 0) {
        LXB_FONT_TABLE_LOAD(mvar);
    }

    /* Color Fonts Related Tables. */

    /* Other OpenType Tables */


#undef LXB_FONT_TABLE_LOAD

    return LXB_STATUS_OK;
}

lxb_status_t
lxb_font_load_from_file(lxb_font_t *mf, const lxb_char_t *filepath,
                        uint8_t **out_data, size_t *out_size)
{
    FILE *fh;
    uint8_t *data, *in_data;
    long size;
    size_t bytes_read;
    uint32_t version;
    uint32_t pos;
    int ret;
    lxb_status_t status;

    if (out_data != NULL) {
        *out_data = NULL;
    }

    if (out_size != NULL) {
        *out_size = 0;
    }

    if ((filepath == NULL) || (*filepath == '\0')) {
        return LXB_STATUS_ERROR_WRONG_ARGS;
    }

    fh = fopen((char *) filepath, "rb");
    if (fh == NULL) {
        return LXB_STATUS_ERROR;
    }

    ret = fseek(fh, 0L, SEEK_END);
    if (ret == -1) {
        status = LXB_STATUS_ERROR_OBJECT_IS_NULL;
        goto close_fh;
    }

    size = ftell(fh);
    if (size == -1) {
        status = LXB_STATUS_ERROR_OBJECT_IS_NULL;
        goto close_fh;
    }

    /* An otf file should contain at least the 12 bytes of the offset table */
    if ((size_t) size < sizeof(lxb_font_offset_table_t)) {
        status = LXB_STATUS_ERROR_TOO_SMALL_SIZE;
        goto close_fh;
    }

    ret = fseek(fh, 0L, SEEK_SET);
    if (ret == -1) {
        status = LXB_STATUS_ERROR_OBJECT_IS_NULL;
        goto close_fh;
    }

    data = (uint8_t *) lexbor_malloc((size_t) size);
    if (data == NULL) {
        status = LXB_STATUS_ERROR_MEMORY_ALLOCATION;
        goto close_fh;
    }

    in_data = data;

    bytes_read = fread(data, 1, (size_t) size, fh);

    fclose(fh);

    if (bytes_read != (size_t) size) {
        status = LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
        goto failed;
    }

    pos = 12;
    if ((size_t) size < pos) {
        status = LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
        goto failed;
    }

    /*
     * Check that the version is correct:
     *
     * - 0x00010000 for TrueType outlines.
     * - 0x4F54544F ("OTTO") for OpenType fonts containing CFF data.
     */
    version = lxb_font_read_u32(&data);
    if ((version != 0x00010000) && (version != 0x4F54544F)) {
        status = LXB_STATUS_ERROR_UNEXPECTED_DATA;
        goto failed;
    }

    mf->is_truetype = (version == 0x00010000);
    mf->num_tables = lxb_font_read_u16(&data);
    lxb_font_read_u16(&data); /* searchRange */
    lxb_font_read_u16(&data); /* entrySelector */
    lxb_font_read_u16(&data); /* rangeShift */

    pos += mf->num_tables * 16;

    if ((uint32_t) size < pos) {
        status = LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
        goto failed;
    }

    if (out_data != NULL) {
        *out_data = in_data;
    }

    if (out_size != NULL) {
        *out_size = (size_t) size;
    }

    return lxb_font_load_table(mf, in_data, size);

  close_fh:

    fclose(fh);
    return status;

  failed:

    lexbor_free(data);
    return status;
}
