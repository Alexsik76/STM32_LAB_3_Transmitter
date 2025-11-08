#pragma once

// C-обгортка для RTOS-задачі
#ifdef __cplusplus
extern "C" {
#endif

void keypad_task(void *argument);
char keypad_get_key(void); // Функція для отримання натиснутої клавіші

#ifdef __cplusplus
}
#endif

// C++ частина
#ifdef __cplusplus

#include "main.h"

class MyKeypad
{
public:
    MyKeypad();     // Конструктор
    void init(void);    // Ініціалізація
    void scan_and_update(void);    // Головна функція сканування

private:
    // Мапа символів
    const char key_map[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    // Піни рядків та стовпців згідно вашого робочого коду
    const uint16_t row_pins[4] = {GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11}; // PA8-PA11
    const uint16_t col_pins[4] = {GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15}; // PB12-PB15
};

#endif // __cplusplus
