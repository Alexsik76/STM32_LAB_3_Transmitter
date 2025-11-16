/*
 * ui_feedback.c
 *
 * Created on: Oct 25, 2025
 * Author: osiko
 */

#include "ui_feedback.h"

/**
 * @brief Turns on the LED for feedback (blink start).
 */
static void UI_Blink_Start(void)
{
  // Turn ON the LED (set pin to '0' for Blue Pill)
  // NOTE: This will fail to compile if 'LED_BUILTIN' is not a User Label for PC13
  HAL_GPIO_WritePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Turns off the LED for feedback (blink end).
 */
static void UI_Blink_End(void)
{
    // Turn OFF the LED (set pin to '1' for Blue Pill)
    HAL_GPIO_WritePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin, GPIO_PIN_SET);
}

/**
 * @brief Performs a single, non-blocking 50ms UI blink.
 * @note This function MUST be called from an RTOS task context.
 */
void UI_Blink_Once(void)
{
	UI_Blink_Start();
	vTaskDelay(pdMS_TO_TICKS(50));
	UI_Blink_End();
}

void UI_Blink_Triple(void)
{
	UI_Blink_Start();
	vTaskDelay(pdMS_TO_TICKS(50));
	UI_Blink_End();
	vTaskDelay(pdMS_TO_TICKS(50));
	UI_Blink_Start();
	vTaskDelay(pdMS_TO_TICKS(50));
	UI_Blink_End();
	vTaskDelay(pdMS_TO_TICKS(50));
	UI_Blink_Start();
	vTaskDelay(pdMS_TO_TICKS(50));
	UI_Blink_End();
}
