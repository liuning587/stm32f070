#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdbool.h>
void vBtnInit(void);
void vBtnBackUp(void);
bool bAnyBtnIsPressed(void);
uint32_t ulGetCfgHeldMillis(void);
uint32_t ulGetStaHeldMillis(void);
uint32_t ulGetDefHeldMillis(void);

#endif // !__BUTTON_H__
