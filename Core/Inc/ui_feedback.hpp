// ui_feedback.h

#ifndef INC_UI_FEEDBACK_HPP_
#define INC_UI_FEEDBACK_HPP_

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

// C-wrapper block to ensure C++ compatibility.
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Performs a single, non-blocking UI blink (e.g., LED).
 */
void UI_Blink_Once(void);
void UI_Blink_Triple(int);

#ifdef __cplusplus
}
#endif


#endif /* INC_UI_FEEDBACK_HPP_ */
