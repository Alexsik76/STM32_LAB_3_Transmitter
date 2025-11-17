/*
 * ui_feedback.c
 * (Оновлено для CMSIS-OS)
 */

#include <ui_feedback.hpp>
#include "cmsis_os.h" // <<< 1. ПІДКЛЮЧАЄМО CMSIS API
// #include "FreeRTOS.h" // <<< 2. ВИДАЛЯЄМО NATIVE API
// #include "task.h"

/**
 * @brief Turns on the LED for feedback (blink start).
 */
static void UI_Blink_Start(void)
{
  HAL_GPIO_WritePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Turns off the LED for feedback (blink end).
 */
static void UI_Blink_End(void)
{
    HAL_GPIO_WritePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin, GPIO_PIN_SET);
}

/**
 * @brief Performs a single, 50ms UI blink.
 */
void UI_Blink_Once(void)
{
	UI_Blink_Start();
    osDelay(50); // <<< 3. ЗАМІНЕНО vTaskDelay() НА osDelay()
	UI_Blink_End();
}

/**
 * @brief Performs a triple UI blink.
 */
void UI_Blink_Multi(int count = 3)
{
	for (int i = 0; i < count; ++i)
	{
		UI_Blink_Once();
		osDelay(50);
	}
}
