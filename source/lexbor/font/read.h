/*
 * Copyright (C) 2019 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef LEXBOR_FONT_READ_H
#define LEXBOR_FONT_READ_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>


lxb_inline int8_t lxb_font_read_8(uint8_t **data)
{
  return (int8_t)*((*data)++);
}

lxb_inline uint8_t lxb_font_read_u8(uint8_t **data)
{
  return (uint8_t)*((*data)++);
}

lxb_inline int16_t lxb_font_read_16(uint8_t **data)
{
  return (int16_t)(lxb_font_read_8(data) << 8 |
                   lxb_font_read_8(data));
}

lxb_inline uint16_t lxb_font_read_u16(uint8_t **data)
{
  return (uint16_t)(lxb_font_read_u8(data) << 8 |
                    lxb_font_read_u8(data));
}

lxb_inline uint32_t lxb_font_read_u32(uint8_t **data)
{
  return (uint32_t)(lxb_font_read_u8(data) << 24 |
                    lxb_font_read_u8(data) << 16 |
                    lxb_font_read_u8(data) << 8  |
                    lxb_font_read_u8(data));
}

lxb_inline int32_t lxb_font_read_32(uint8_t **data)
{
  return (int32_t)(lxb_font_read_u8(data) << 24 |
                   lxb_font_read_u8(data) << 16 |
                   lxb_font_read_u8(data) << 8  |
                   lxb_font_read_u8(data));
}

lxb_inline uint32_t lxb_font_read_u32_as_net(uint8_t **data)
{
  return (uint32_t)(lxb_font_read_u8(data)      |
                   lxb_font_read_u8(data) << 8  |
                   lxb_font_read_u8(data) << 16 |
                   lxb_font_read_u8(data) << 24);
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_FONT_READ_H */
