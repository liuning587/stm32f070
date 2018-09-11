#ifndef __PWR_CTRL_H__
#define __PWR_CTRL_H__

#include "stm32f0xx.h"

#define PWR_ACTIVE  (0x01)
#define PWR_ON      (PWR_ACTIVE)
#define PWR_OFF     (!PWR_ACTIVE)

void vPwrCtrlInit(void);
void vPwrCtrlUsbHub(uint32_t ulPowerOn);
void vPwrCtrlRasberryPi(uint32_t ulPowerOn);
void vPwrCtrlEpd(uint32_t ulPowerOn);
void vPwrCtrlExtUsbDev1(uint32_t ulPowerOn);
void vPwrCtrlExtUsbDev2(uint32_t ulPowerOn);

#endif // !__PWR_CTRL_H__
