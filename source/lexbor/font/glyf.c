/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/base.h"
#include "lexbor/font/base.h"
#include "lexbor/font/read.h"


static lxb_status_t
lxb_font_simple_glyph_load(lxb_font_t *mf, uint8_t *data,
                           glyph_t *glyph, size_t size, uint32_t pos)
{


#define SG(field_) glyph->simple_glyph.field_

#define SG_ALLOC(field_, count_, type_)                                        \
    do {                                                                       \
        SG(field_) = LXB_FONT_ALLOC(count_, type_);                            \
        if (SG(field_) == NULL) {                                              \
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;                         \
        }                                                                      \
    }                                                                          \
    while (0)


    uint8_t *flags_data;
    uint16_t points_count;
    uint16_t flags_count;
    uint16_t data_count;
    uint16_t xcoord_count;
    uint16_t ycoord_count;
    uint16_t xcoord_size;
    uint16_t ycoord_size;
    uint16_t i;
    uint8_t flag;

    pos += 2 * glyph->number_of_contours + 2;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    SG_ALLOC(end_pts_of_contours, glyph->number_of_contours, uint16_t);

    for (i = 0; i < glyph->number_of_contours; i++) {
        SG(end_pts_of_contours)[i] = lxb_font_read_u16(&data);
    }

    points_count = SG(end_pts_of_contours)[glyph->number_of_contours - 1] + 1;

    SG(instruction_length) = lxb_font_read_u16(&data);

    if (SG(instruction_length) > 0) {
        SG_ALLOC(instructions, SG(instruction_length), uint8_t);

        for (i = 0; i < SG(instruction_length); i++) {
            SG(instructions)[i] = lxb_font_read_u8(&data);
        }
    }

    /*
     * Calculate the number of elements of the 'flags' array
     * and the size for coordinates  */
    flags_data = data;
    flags_count = 0;
    data_count = 0;
    xcoord_count = 0;
    ycoord_count = 0;
    printf(" * pcount : %d %p\n", points_count, glyph);
    fflush(stdout);
    for (i = 0; i < points_count; i++) {
        flag = lxb_font_read_u8(&flags_data);
        flags_count++;
        printf(" * fcount : %d\n", flags_count);
        data_count++;
        xcoord_size = (flag & LXB_FONT_X_SHORT_VECTOR) ? 1 : 2;
        ycoord_size = (flag & LXB_FONT_Y_SHORT_VECTOR) ? 1 : 2;
        xcoord_count += xcoord_size;
        ycoord_count += ycoord_size;
        if (flag & LXB_FONT_REPEAT_FLAG) {
            uint8_t repeats;

            repeats = lxb_font_read_u8(&flags_data);
            printf(" * repeats : %d\n", repeats);
            fflush(stdout);
            data_count++;
            for (; repeats > 0; repeats--) {
                flags_count++;
                //printf(" * fcount : %d\n", flags_count);
                xcoord_count += xcoord_size;
                ycoord_count += ycoord_size;
            }
        }
    }

    printf(" * final fcount : %d\n", flags_count);
    fflush(stdout);

    SG(flags_count) = flags_count;
    pos += data_count + xcoord_count + ycoord_count;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    printf(" * final fcount : %d\n", SG(flags_count));
    fflush(stdout);
    /* Fill the 'flags' and (x|y) coordinates arrays */
    SG_ALLOC(flags, SG(flags_count), uint8_t);
    SG_ALLOC(x_coordinates, SG(flags_count), uint16_t);
    SG_ALLOC(y_coordinates, SG(flags_count), uint16_t);

#if 0
    flags_count = 0;
    for (i = 0; i < points_count; i++) {
        SG(flags)[flags_count] = lxb_font_read_u8(&data);
        flag = SG(flags)[flags_count];

        /* X coordinate. */
        if (flag & LXB_FONT_X_SHORT_VECTOR) {
            if (flag & LXB_FONT_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
                SG(x_coordinates)[flags_count] = lxb_font_read_u8(&data);
            }
            else {
                SG(x_coordinates)[flags_count] = lxb_font_read_8(&data);
            }
        }
        else {
            if (flag & LXB_FONT_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
                SG(x_coordinates)[flags_count] = SG(x_coordinates)[flags_count - 1];
            }
            else {
                SG(x_coordinates)[flags_count] = lxb_font_read_16(&data);
            }
        }

        /* Y coordinate. */
        if (flag & LXB_FONT_Y_SHORT_VECTOR) {
            if (flag & LXB_FONT_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
                SG(y_coordinates)[flags_count] = lxb_font_read_u8(&data);
            }
            else {
                SG(y_coordinates)[flags_count] = lxb_font_read_8(&data);
            }
        }
        else {
            if (flag & LXB_FONT_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
                SG(x_coordinates)[flags_count] = SG(y_coordinates)[flags_count - 1];
            }
            else {
                SG(y_coordinates)[flags_count] = lxb_font_read_16(&data);
            }
        }

        flags_count++;

        if (flag & LXB_FONT_REPEAT_FLAG) {
            //uint16_t j;
            uint8_t repeats;

            repeats = lxb_font_read_u8(&data);
            printf(" * repeats : %d\n", repeats);
            fflush(stdout);
            for (; repeats > 0; repeats--) {
                SG(flags)[flags_count] = flag;

                /* X coordinate. */
                if (flag & LXB_FONT_X_SHORT_VECTOR) {
                    if (flag & LXB_FONT_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
                        SG(x_coordinates)[flags_count] = lxb_font_read_u8(&data);
                    }
                    else {
                        SG(x_coordinates)[flags_count] = lxb_font_read_8(&data);
                    }
                }
                else {
                    if (flag & LXB_FONT_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
                        printf("fcount : %d / %d\n", flags_count, SG(flags_count));
                        fflush(stdout);
                        SG(x_coordinates)[flags_count] = SG(x_coordinates)[flags_count - 1];
                    }
                    else {
                        SG(x_coordinates)[flags_count] = lxb_font_read_16(&data);
                    }
                }

                /* Y coordinate. */
                if (flag & LXB_FONT_Y_SHORT_VECTOR) {
                    if (flag & LXB_FONT_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
                        SG(y_coordinates)[flags_count] = lxb_font_read_u8(&data);
                    }
                    else {
                        SG(y_coordinates)[flags_count] = lxb_font_read_8(&data);
                    }
                }
                else {
                    if (flag & LXB_FONT_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
                        SG(x_coordinates)[flags_count] = SG(y_coordinates)[flags_count - 1];
                    }
                    else {
                        SG(y_coordinates)[flags_count] = lxb_font_read_16(&data);
                    }
                }

                flags_count++;
            }
        }
    }
#else
    /* flags_count = 0; */
    /* for (i = 0; i < points_count; i++) { */
    /*     flag = lxb_font_read_u8(&data); */

    /*     flags_count++; */

    /*     if (flag & LXB_FONT_REPEAT_FLAG) { */
    /*         //uint16_t j; */
    /*         uint8_t repeats; */

    /*         repeats = lxb_font_read_u8(&data); */
    /*         printf(" * repeats : %d\n", repeats); */
    /*         fflush(stdout); */
    /*         for (; repeats > 0; repeats--) { */
    /*             flags_count++; */
    /*         } */
    /*     } */
    /* } */
    /* printf(" * final fcount 2 : %d\n", flags_count); */
#endif

    return LXB_STATUS_OK;


#undef SG

#undef SG_ALLOC


}

static lxb_status_t
lxb_font_composite_glyph_load(lxb_font_t *mf, uint8_t *data,
                              glyph_t *glyph, size_t size, uint32_t pos)
{


#define CG(field_) glyph->composite_glyph.field_

#define CG_ALLOC(field_, count_, type_)                                        \
    do {                                                                       \
        CG(field_) = LXB_FONT_ALLOC(count_, type_);                            \
        if (CG(field_) == NULL) {                                              \
            return LXB_STATUS_ERROR_MEMORY_ALLOCATION;                         \
        }                                                                      \
    }                                                                          \
    while (0)


    pos += 4;
    if (size < pos) {
        return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
    }

    CG(flags) = lxb_font_read_u16(&data);
    CG(glyph_index) = lxb_font_read_u16(&data);

    if (CG(flags) & LXB_FONT_ARG_1_AND_2_ARE_WORDS) {
        pos += 4;
        if (size < pos) {
            return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
        }

        if (CG(flags) & LXB_FONT_ARGS_ARE_XY_VALUES) {
            CG(argument1) = lxb_font_read_16(&data);
            CG(argument2) = lxb_font_read_16(&data);
        }
        else {
            CG(argument1) = lxb_font_read_u16(&data);
            CG(argument2) = lxb_font_read_u16(&data);
        }
    }
    else {
        pos += 2;
        if (size < pos) {
            return LXB_STATUS_ERROR_INCOMPLETE_OBJECT;
        }

        if (CG(flags) & LXB_FONT_ARGS_ARE_XY_VALUES) {
            CG(argument1) = lxb_font_read_8(&data);
            CG(argument2) = lxb_font_read_8(&data);
        }
        else {
            CG(argument1) = lxb_font_read_u8(&data);
            CG(argument2) = lxb_font_read_u8(&data);
        }
    }

    return LXB_STATUS_OK;


#undef CG

#undef CG_ALLOC


}

lxb_font_table_glyf_t *
lxb_font_table_glyf(lxb_font_t *mf, uint8_t* font_data, size_t size)
{
    lxb_font_table_glyf_t *table;
    uint8_t *data;
    glyph_t *glyph;
    uint32_t offset;
    uint32_t pos;
    uint16_t num_glyphs;
    uint16_t i;

    table = LXB_FONT_ALLOC(1, lxb_font_table_glyf_t);
    if (table == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    num_glyphs = mf->table_maxp->num_glyphs;
    if (num_glyphs == 0) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
    }
    table->glyphs = LXB_FONT_ALLOC(num_glyphs, glyph_t);
    if (table->glyphs == NULL) {
        return lxb_font_failed(mf, LXB_STATUS_ERROR_MEMORY_ALLOCATION);
    }

    memset(table->glyphs, 0, sizeof(glyph_t));

    for (i = 0; i < num_glyphs; i++) {
        offset = mf->cache.tables_offset[LXB_FONT_TKEY_GLYF];
        offset += mf->table_loca->offsets[i];
        data = font_data + offset;
        glyph = table->glyphs + i;

        pos = offset + 10;
        if (size < pos) {
            return lxb_font_failed(mf, LXB_STATUS_ERROR_INCOMPLETE_OBJECT);
        }

        /* Parse the header. */
        glyph->number_of_contours = lxb_font_read_16(&data);

        glyph->x_min = lxb_font_read_16(&data);
        glyph->y_min = lxb_font_read_16(&data);
        glyph->x_max = lxb_font_read_16(&data);
        glyph->y_max = lxb_font_read_16(&data);

        /* FIXME: mraw problem with simple glyphes ? */
#if 1
        /* Parse simple glyph. */
        if (glyph->number_of_contours >= 0) {
            lxb_status_t status;

            status = lxb_font_simple_glyph_load(mf, data, glyph, size, pos);
            if (status != LXB_STATUS_OK) {
                return lxb_font_failed(mf, status);
            }
        }
        /* Parse composite glyph. */
        else {
            lxb_status_t status;

            status = lxb_font_composite_glyph_load(mf, data, glyph, size, pos);
            if (status != LXB_STATUS_OK) {
                return lxb_font_failed(mf, status);
            }
        }
#endif
    }

    return table;
}
