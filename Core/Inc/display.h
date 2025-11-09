#pragma once

// Цей блок потрібен, щоб "подружити" C++ код з main.c
#ifdef __cplusplus
extern "C" {
#endif
#include "FreeRTOS.h"
#include "semphr.h"
// Функції-обгортки, які буде бачити C-код (наш main.c)
void display_init(void);
void display_task(void *argument);

extern SemaphoreHandle_t g_i2c_tx_done_sem;
#ifdef __cplusplus
}
#endif

// Ця частина коду буде видима тільки для C++
#ifdef __cplusplus

// Підключаємо згенеровані HAL-драйвери
#include "main.h"

// Оголошуємо наш C++ клас
class MyDisplay
{
public:
    MyDisplay(I2C_HandleTypeDef *hi2c); // Конструктор
    bool init(void);                    // Метод ініціалізації
    void update_screen_DMA(void);       // Метод оновлення (неблокуючий)

private:
    I2C_HandleTypeDef *hi2c;            // Вказівник на наш I2C
};

#endif // __cplusplus
