#include "display.hpp"
#include "display_protocol.hpp"
#include "rtos_tasks.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>
#include "ssd1306.h"
#include "fonts.h"

// --- Глобальні об'єкти ---
extern I2C_HandleTypeDef hi2c1;
MyDisplay g_display(&hi2c1);

// =============================================================================
// ЦЕЙ БЛОК (extern "C") - ЄДИНЕ МІСЦЕ, ЯКЕ ВІДРІЗНЯЄТЬСЯ НА TX І RX
// =============================================================================
extern "C" {


extern osMessageQueueId_t displayQueueHandleHandle;
extern osSemaphoreId_t i2cTxDoneSemHandle; 



void display_run_task(void) {
   
    g_display.setup(displayQueueHandleHandle, i2cTxDoneSemHandleHandle);
    
    
    g_display.task();
}

void display_init(void) {
   
}

} // extern "C"
// =============================================================================


// --- C++ Implementation 

MyDisplay::MyDisplay(I2C_HandleTypeDef *hi2c)
{
    this->hi2c = hi2c;
    this->current_key = 0;
    this->queueHandle = NULL; // Захист від дурня
    this->i2cSemHandle = NULL;
    memset(this->main_text, 0, sizeof(this->main_text));
    memset(this->status_text, 0, sizeof(this->status_text));
    strncpy(this->status_text, "Boot...", sizeof(this->status_text) - 1);
}

// Реалізація методу setup
void MyDisplay::setup(osMessageQueueId_t queue, osSemaphoreId_t i2c_sem)
{
    this->queueHandle = queue;
    this->i2cSemHandle = i2c_sem;
}

bool MyDisplay::init()
{
    osDelay(100);
    return ssd1306_Init() == 0;
}

void MyDisplay::task(void)
{
    // Перевірка, чи викликали setup
    if (this->queueHandle == NULL) {
        vTaskDelete(NULL);
    }

    if (!this->init()) {
    }

    DisplayMessage_t msg; 
    this->set_status_text("Booting...");
    this->update_screen_internal();
    osDelay(500);
    this->set_status_text("Ready");
    this->set_main_text(""); // Очищуємо центр
    this->update_screen_internal();
    while (1)
    {
        // Використовуємо переданий хендл this->queueHandle замість глобального
        if (osMessageQueueGet(this->queueHandle, &msg, NULL, osWaitForever) == osOK)
        {
            bool needs_redraw = false;

            switch (msg.command)
				{
				case DISP_CMD_SET_STATUS:
					this->set_status_text(msg.text);
					needs_redraw = true;
					break;

				case DISP_CMD_SET_MAIN_TEXT:
					this->set_main_text(msg.text);
					this->current_key = 0;
					needs_redraw = true;
					break;

				case DISP_CMD_SHOW_KEY:
					 this->on_key_press(msg.key);
					 this->set_main_text("");
					 needs_redraw = true;
					 break;

				case DISP_CMD_CLEAR:
					 this->clear_all();
					 needs_redraw = true;
					 break;
				}
            if (needs_redraw) {
                this->update_screen_internal();
            }
        }
    }
}


void MyDisplay::update_screen_internal()
{
    ssd1306_Fill(Black);

    // 1. Статус бар (дрібний шрифт 6x8)
    // Тут використовуємо звичайний WriteString
    if (this->status_text[0] != '\0') {
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString(this->status_text, (FontDef_8bit_t*)&Font_6x8, White);
    }

    // 2. Головний текст (ВЕЛИКИЙ шрифт 11x18)
    // Тут використовуємо WriteString_Large
    if (this->main_text[0] != '\0') 
    {
        // Центруємо для шрифту шириною 11 пікселів
        int len = strlen(this->main_text);
        int px_width = len * 11; 
        
        int x_pos = (128 - px_width) / 2;
        if (x_pos < 0) x_pos = 0;

        ssd1306_SetCursor(x_pos, 20);
        
        // ВИКЛИКАЄМО ПРАВИЛЬНУ ФУНКЦІЮ БЕЗ КАСТИНГУ!
        ssd1306_WriteString_Large(this->main_text, &Font_11x18, White);
    }
    else if (this->current_key != 0) 
    {
        // Одна літера по центру
        ssd1306_SetCursor(54, 20); 
        char str[2] = {this->current_key, '\0'};
        
        ssd1306_WriteString_Large(str, &Font_11x18, White); 
    }

    // 3. DMA Update
    ssd1306_UpdateScreenDMA();
    
    if (this->i2cSemHandle != NULL) {
        osSemaphoreAcquire(this->i2cSemHandle, 100);
    } else {
        osDelay(10);
    }
}

void MyDisplay::set_status_text(const char* text) {
    strncpy(this->status_text, text, sizeof(this->status_text) - 1);
}
void MyDisplay::set_main_text(const char* text) {
    strncpy(this->main_text, text, sizeof(this->main_text) - 1);
}
void MyDisplay::on_key_press(char key) {
    this->current_key = key;
}
void MyDisplay::clear_all()
{
    this->status_text[0] = '\0';
    this->main_text[0] = '\0';
    this->current_key = 0;
}
