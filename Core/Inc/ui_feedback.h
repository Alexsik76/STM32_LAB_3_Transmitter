// ui_feedback.h

#ifndef INC_UI_FEEDBACK_H_
#define INC_UI_FEEDBACK_H_

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

// Блок, який говорить C++-компілятору:
// "Ці функції є C-функціями і не мають спотворюватися."
#ifdef __cplusplus
extern "C" {
#endif

void UI_Blink_Once(void); // <<< ТЕПЕР ОГОЛОШЕНО ЯК C-ФУНКЦІЯ

#ifdef __cplusplus
}
#endif


#endif /* INC_UI_FEEDBACK_H_ */
