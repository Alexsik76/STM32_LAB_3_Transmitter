#ifndef DISPLAY_PROTOCOL_H
#define DISPLAY_PROTOCOL_H

#include <stdint.h>

typedef enum {
    DISP_CMD_SET_STATUS,
    DISP_CMD_SET_MAIN_TEXT,
    DISP_CMD_SHOW_KEY, // Для гарного відображення однієї літери
    DISP_CMD_CLEAR
} DisplayCommand_t;

typedef struct {
    DisplayCommand_t command;
    char text[32]; // Текст повідомлення
    char key;      // Окремий символ (якщо треба показати кнопку)
} DisplayMessage_t;

#endif