#pragma once
#ifndef NEOCHROME_H
#define NEOCHROME_H
//http://wiki.multimedia.cx/index.php?title=Neochrome
#ifdef WIN32
#define uint8_t BYTE
#define uint16_t WORD
#endif

#pragma pack(push, 8)

struct neochrome_file{
 uint16_t flags;                      /* always 0 */
 uint16_t resolution;                 /* 0 = low (320x200x16), 1 = medium (640x200x4), 2 = high (640x400x2) */
 uint16_t palette[16];                /* 9-bit RGB 00000RRR0GGG0BBB */
 uint8_t filename[12];                /* 8 '.' 3 */
 uint16_t color_animation_limits;
 uint16_t color_animation_speeddir;
 uint16_t color_animation_steps;
 uint16_t x_offset;                   /* always 0 */
 uint16_t y_offset;                   /* always 0 */
 uint16_t width;
 uint16_t height;
 uint16_t reserved[33];
 uint16_t data[16000];
} ;

#pragma pack(pop)

#ifdef WIN32
#undef uint8_t
#undef uint16_t
#endif

#endif//#ifndef NEOCHROME_H