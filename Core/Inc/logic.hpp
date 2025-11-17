#pragma once
#include "main.h"

// Перелік наших режимів роботи (про які ви казали)
typedef enum {
    MODE_NORMAL_KEYS, // Режим 1: Передача клавіш
    MODE_SERVO_ARROWS, // Режим 2: Управління серво (стрілки)
    MODE_SEND_PRESET_ABC // Режим 3: Передача "Abc"
} SystemMode_t;


class LogicTask {
public:
    LogicTask();

    /**
     * @brief Головний цикл задачі RTOS для логіки.
     * Викликається з C-обгортки.
     */
    void task();

private:
    // Поточний стан (режим) системи
    SystemMode_t current_mode;

    // --- Приватні методи-обробники ---
    void handle_mode_normal_keys(char key);
    void handle_mode_servo_arrows(char key);
    void handle_mode_send_preset(char key);

    // --- Приватні методи-"відправники" ---
    void send_display_status(const char* text);
    void send_display_main(const char* text);
    void send_display_key(char key);
    void send_radio_packet(uint8_t* data, uint8_t len);
};

// !!! ВСІ 'extern "C"' ПРОТОТИПИ ЗВІДСИ ВИДАЛЕНІ !!!
