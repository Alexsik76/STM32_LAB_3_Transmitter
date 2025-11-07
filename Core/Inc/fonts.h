#ifndef __FONTS_H__
#define __FONTS_H__

#include <stdint.h>

typedef struct {
    const uint8_t FontWidth;
    uint8_t FontHeight;
    const uint8_t *data;
} FontDef_8bit_t;

extern FontDef_8bit_t Font_6x8;

#endif // __FONTS_H__
