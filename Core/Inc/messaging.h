#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Типи команд, які ми можемо відправити потоку дисплея
typedef enum {
    DISPLAY_CMD_SET_STATUS, // Встановити текст статусу (вгорі)
    DISPLAY_CMD_SET_MAIN,   // Встановити головний текст (в центрі)
    DISPLAY_CMD_SHOW_KEY,   // Показати натиснуту клавішу
    DISPLAY_CMD_CLEAR_ALL   // Очистити все
} DisplayCommandType_t;

// Повідомлення, яке ми кладемо в чергу дисплея
typedef struct {
    DisplayCommandType_t command; // Яка команда
    char text[32];          // Буфер для тексту (для SET_STATUS / SET_MAIN)
    char key;               // Буфер для клавіші (для SHOW_KEY)
} DisplayMessage_t;


#ifdef __cplusplus
}
#endif
