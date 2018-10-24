#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_bus.h"
#include "pwrctrl.h"



void vPwrCtrlUsbHub(uint32_t ulPowerOn) {
    if (ulPowerOn == PWR_ACTIVE) {
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_6);
    } else {
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);
    }
}


void vPwrCtrlRasberryPi(uint32_t ulPowerOn) {    
    if (ulPowerOn == PWR_ACTIVE) {
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7);
    } else {
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);
    }
}

void vPwrCtrlEpd(uint32_t ulPowerOn) {
    if (ulPowerOn == PWR_ACTIVE) {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_0);
    } else {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0);
    }
}

void vPwrCtrlExtUsbDev1(uint32_t ulPowerOn) {
    if (ulPowerOn == PWR_ACTIVE) {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_1);
    } else {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_1);
    }
}

void vPwrCtrlExtUsbDev2(uint32_t ulPowerOn) {
    if (ulPowerOn == PWR_ACTIVE) {
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_2);
    } else {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_2);
    }
}


void vPwrCtrlInit(void) {
    /* Enable GPIOA, GPIOB clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    
    
    /* Configure PA6, PA7, PB0, PB1, PB2 pin as output floating 
    * HUB_PWR_ON: PA6
    * PI_PWR_ON: PA7  
    * EPD_PWR_ON: PB0
    * EXT_USB1_PWR_ON: PB1  
    * EX_USB2_PWR_ON: PB2
    **/  
    LL_GPIO_InitTypeDef GPIO_InitStructure;
    LL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_LOW;
    
    GPIO_InitStructure.Pin = LL_GPIO_PIN_6 | LL_GPIO_PIN_7; 
    LL_GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2; 
    LL_GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    vPwrCtrlRasberryPi(PWR_ON);
    vPwrCtrlUsbHub(PWR_ON);
    vPwrCtrlEpd(PWR_ON);
    vPwrCtrlExtUsbDev1(PWR_ON);
    vPwrCtrlExtUsbDev2(PWR_ON);
}