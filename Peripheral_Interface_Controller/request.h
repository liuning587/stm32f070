#ifndef __REQUEST_H__
#define __REQUEST_H__
			
#include "stm32f0xx.h"

typedef void(*pfRequestCb_t)(void *pArg, uint32_t ulLen);
pfRequestCb_t xGetReqCb(uint32_t ulReqId);
void vSetBootReason(const char *pcReason);
#endif // !__REQUEST_H__