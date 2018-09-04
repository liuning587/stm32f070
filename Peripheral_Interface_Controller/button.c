
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_rcc.h"    
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_exti.h"
#include <usbd_core.h>
#include <usbd_template.h>
#include <usbd_desc.h>
#include <usbd_template.h>
#include <usbd_template_if.h>
#include <stdbool.h>
#include "pools.h"
#include "request.h"
#include "response.h"
#include "notify.h"

void vDefaultBtnInit(void) {
     // TODO...
}

bool bIsDefaultBtnPressed(void) {
    // TODO...
    return false;        
}

void vDefaultBtnCb(void) {
    static uint32_t tick = 0;
    if (HAL_GetTick() - tick > 50) {
        struct PoolEntry xEntryPut;
        xEntryPut.ucLen = sizeof("DEFAULT");
        memcpy(xEntryPut.aucData, "DEFAULT", xEntryPut.ucLen);
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
        /* Enable SYSCF clock */
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);
    /* Configure EXTI Line 0 */ 
    LL_EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    /* Enable and set EXTI lines 0 to 1 Interrupt to the lowest priority */
    NVIC_SetPriority(EXTI0_1_IRQn, 2);
}

bool bIsApBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0);
}

void vApBtnCb(void) {
    static uint32_t tick = 0;
    if (HAL_GetTick() - tick > 50) {
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
    /* Enable SYSCF clock */
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);
    /* Configure EXTI Line 13 */ 
    LL_EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_13;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    /* Enable and set EXTI lines 4 to 15 Interrupt to the lowest priority */
    NVIC_SetPriority(EXTI4_15_IRQn, 2);
}

bool bIsInfoBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13);
}

void vInfoBtnCb(void) {
    static uint32_t tick = 0;
    if (HAL_GetTick() - tick > 50) {
        struct PoolEntry xEntryPut;
        xEntryPut.ucLen = sizeof("INFO");
        memcpy(xEntryPut.aucData, "INFO", xEntryPut.ucLen);
        ulPoolsPut(NOTIFY_POOL, &xEntryPut);
        tick = HAL_GetTick();
    }
}

void EXTI0_1_IRQHandler(void) {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0)) { 
        NVIC_DisableIRQ(EXTI0_1_IRQn);
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
        vApBtnCb();
    }   
}
void EXTI4_15_IRQHandler(void) {
    /* EXTI line interrupt detected */
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13)) { 
        NVIC_DisableIRQ(EXTI4_15_IRQn);
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        vInfoBtnCb();
    }   

}