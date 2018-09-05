#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdbool.h>

void vRecoveryBtnInit(void);
void vInfoBtnInit(void);
void vApBtnInit(void);

bool bIsRecoveryBtnPressed(void);
bool bIsInfoBtnPressed(void);
bool bIsApBtnPressed(void);

void vRecoveryBtnCb(void);
void vInfoBtnCb(void);
void vApBtnCb(void);

#endif // !__BUTTON_H__
