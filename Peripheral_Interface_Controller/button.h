#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdbool.h>

void vDefaultBtnInit(void);
void vInfoBtnInit(void);
void vApBtnInit(void);

bool bIsDefaultBtnPressed(void);
bool bIsInfoBtnPressed(void);
bool bIsApBtnPressed(void);

void vDefaultBtnCb(void);
void vInfoBtnCb(void);
void vApBtnCb(void);

#endif // !__BUTTON_H__
