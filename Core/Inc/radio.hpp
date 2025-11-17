#pragma once

#include "nrf24l01p.hpp" // Включаємо наш C++ драйвер
#include "cmsis_os.h"     // Включаємо RTOS API

/**
 * @brief C++ клас-обгортка для всієї логіки радіо.
 */
class MyRadio {
public:
    /**
     * @brief Конструктор.
     * Ініціалізує C++ драйвер nRF24.
     */
    MyRadio();

    /**
     * @brief Головний цикл задачі RTOS для радіо.
     * Викликається з C-обгортки.
     */
    void task();

private:
    /**
     * @brief Екземпляр нашого C++ драйвера.
     */
    Nrf24l01p radio;

    /**
     * @brief Внутрішня функція ініціалізації.
     */
    bool init(void);
};

// --- Глобальний C++ об'єкт ---
extern MyRadio g_radio;

// !!! ВСІ 'extern "C"' ПРОТОТИПИ ТА СЕМАФОРИ ЗВІДСИ ВИДАЛЕНІ !!!
