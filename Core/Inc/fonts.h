#ifndef __FONTS_H__
#define __FONTS_H__

#include <stdint.h>

/**
 * @brief Font structure for 8-bit fonts (like 6x8)
 */
typedef struct {
    const uint8_t FontWidth;    // Font width in pixels
    uint8_t FontHeight;   		// Font height in pixels
    const uint8_t *data;  		// Pointer to 8-bit font data
} FontDef_8bit_t;

/**
 * @brief Font structure for 16-bit fonts (like 11x18)
 * @note  This is from the trusted driver you provided.
 */
typedef struct {
    const uint8_t FontWidth;    /*!< Font width in pixels */
    uint8_t FontHeight;   		/*!< Font height in pixels */
    const uint16_t *data; 		/*!< Pointer to 16-bit font data */
} FontDef_t;


// --- Exported Fonts ---

// Our existing 8-bit font
extern FontDef_8bit_t Font_6x8;

// The new 16-bit fonts from the trusted driver
extern FontDef_t Font_7x10;
extern FontDef_t Font_11x18;
extern FontDef_t Font_16x26;


#endif // __FONTS_H__
