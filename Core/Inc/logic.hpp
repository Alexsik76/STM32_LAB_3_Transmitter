#pragma once
#include "main.h"
#include "radio_protocol.hpp"    // Тут живуть RadioPacket і SystemMode_t (MODE_KEYPAD...)
#include "display_protocol.hpp" // Тут живе DisplayCommand_t

class LogicTask {
public:
    LogicTask();

    /**
     * @brief Головний цикл
     */
    void task();

private:
    // Тепер цей тип береться з radio_protocol.h
    SystemMode_t current_mode;

    // Методи, які ми реалізували в logic.cpp
    void update_local_display();
    void send_to_display(DisplayCommand_t cmd, const char* text, char key = 0);
};
