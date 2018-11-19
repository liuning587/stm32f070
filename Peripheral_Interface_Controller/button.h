#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdbool.h>

void vBtnInit(void);
void vBtnEnableIrq(FunctionalState xEnable);
uint32_t ulGetBtnStatus(void);
void vSetBtnStatus(uint32_t status);
void vBackUpBtnStatus(uint32_t status);

#endif // !__BUTTON_H__
