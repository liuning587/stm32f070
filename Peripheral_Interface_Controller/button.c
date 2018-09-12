#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_rcc.h"    
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_hal.h"
#include <stdbool.h>
#include <string.h>
#include "pools.h"
#include "request.h"
#include "response.h"
#include "notify.h"

void vRecoveryBtnInit(void) {
     /* Enable GPIOB clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    /* Configure PA0 pin as input floating */ 
    LL_GPIO_InitTypeDef GPIO_InitStructure;
    LL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_DOWN;
    GPIO_InitStructure.Pin = LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOB, &GPIO_InitStructure);  
    /* Enable and set EXTI lines 4 to 15 Interrupt to the lowest priority */
    NVIC_SetPriority(EXTI4_15_IRQn, 2);    
	NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void vRecoveryBtnEnableIrq(FunctionalState xEnable) {
    /* Enable SYSCF clock */
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE15);
    /* Configure EXTI Line 5 */ 
    LL_EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_15;
    EXTI_InitStruct.LineCommand = xEnable;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);  
}

bool bIsRecoveryBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_15);
}

void vRecoveryBtnCb(void) {
    static uint32_t tick = 0;
    if (HAL_GetTick() - tick > 200) {
        struct PoolEntry xEntryPut;
        xEntryPut.ucLen = sizeof("RECOVERY");
        memcpy(xEntryPut.aucData, "RECOVERY", xEntryPut.ucLen);
        ulPoolsPut(NOTIFY_POOL, &xEntryPut);
        tick = HAL_GetTick();
    }
}

void vApBtnInit(void) {
     /* Enable GPIOA clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    /* Configure PA0 pin as input floating */
    LL_GPIO_InitTypeDef GPIO_InitStructure;
    LL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_DOWN;
    GPIO_InitStructure.Pin = LL_GPIO_PIN_0;
    LL_GPIO_Init(GPIOA, &GPIO_InitStructure);  
    /* Enable and set EXTI lines 0 to 1 Interrupt to the lowest priority */
    NVIC_SetPriority(EXTI0_1_IRQn, 2);
    NVIC_EnableIRQ(EXTI0_1_IRQn);
}

void vApBtnEnableIrq(FunctionalState xEnable) {
    /* Enable SYSCF clock */
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);
    /* Configure EXTI Line 0 */ 
    LL_EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0;
    EXTI_InitStruct.LineCommand = xEnable;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
}

bool bIsApBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0);
}

void vApBtnCb(void) {
    static uint32_t tick = 0;
    if (HAL_GetTick() - tick > 200) {
        struct PoolEntry xEntryPut;
        xEntryPut.ucLen = sizeof("AP");
        memcpy(xEntryPut.aucData, "AP", xEntryPut.ucLen);
        ulPoolsPut(NOTIFY_POOL, &xEntryPut);
        tick = HAL_GetTick();
    }
}

void vInfoBtnInit(void) {
    /* Enable GPIOC clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    /* Configure PC13 pin as Input */ 
    LL_GPIO_InitTypeDef GPIO_InitStructure;
    LL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_DOWN;
    GPIO_InitStructure.Pin = LL_GPIO_PIN_13;
    LL_GPIO_Init(GPIOC, &GPIO_InitStructure);    
    /* Enable and set EXTI lines 4 to 15 Interrupt to the lowest priority */
    NVIC_SetPriority(EXTI4_15_IRQn, 2);    
	NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void vInfoBtnEnableIrq(FunctionalState xEnable) {
    /* Enable SYSCF clock */
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);
    /* Configure EXTI Line 13 */ 
    LL_EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_13;
    EXTI_InitStruct.LineCommand = xEnable;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
}

bool bIsInfoBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13);
}

static void prvInfoPinCb(void) {
    switch (HAL_GetTick() % 750) {
    case 0:
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
        break;
    case 25:
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
        break;   
    default:
        break;  
    }

}

extern void vSetSysTickCallBack(void *callback);
extern void* vGetSysTickCallBack(void);
void vInfoBtnCb(void) {
    static uint32_t tick = 0;
    if (HAL_GetTick() - tick > 200) {
        struct PoolEntry xEntryPut;
        xEntryPut.ucLen = sizeof("INFO");
        memcpy(xEntryPut.aucData, "INFO", xEntryPut.ucLen);
        ulPoolsPut(NOTIFY_POOL, &xEntryPut);
        tick = HAL_GetTick();
        vSetSysTickCallBack(vGetSysTickCallBack() == prvInfoPinCb ? NULL : prvInfoPinCb);
    }
}

void EXTI0_1_IRQHandler(void) {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0)) { 
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
        vApBtnCb();
    }   
}


void EXTI4_15_IRQHandler(void) {
    /* EXTI line interrupt detected */
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13)) { 
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        vInfoBtnCb();
        return;
    }   
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_15)) { 
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_15);
        vRecoveryBtnCb();
        return;
    }   
}