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
#include "button.h"

extern void vSetSysTickCallBack(void *callback);
extern void* vGetSysTickCallBack(void);

/******************************
 *  Key Port: GPIOA
 *  Key Pin: LL_GPIO_PIN_1
 *******************************/
static void prvInfoBtnInit(void) {
     /* Enable GPIOA clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    /* Configure PA0 pin as input floating */
    LL_GPIO_InitTypeDef GPIO_InitStructure;
    LL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_DOWN;
    GPIO_InitStructure.Pin = LL_GPIO_PIN_1;
    LL_GPIO_Init(GPIOA, &GPIO_InitStructure);  
}

static bool prvIsInfoBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_1);
}

/******************************
 *  Ap Port: GPIOA
 *  Ap Pin: LL_GPIO_PIN_2
 *******************************/
static void prvApBtnInit(void) {
     /* Enable GPIOA clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    /* Configure PA0 pin as input floating */
    LL_GPIO_InitTypeDef GPIO_InitStructure;
    LL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_DOWN;
    GPIO_InitStructure.Pin = LL_GPIO_PIN_2;
    LL_GPIO_Init(GPIOA, &GPIO_InitStructure);  

}

static bool prvIsApBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_2);
}

/******************************
 *  Key Port: GPIOA
 *  Key Pin: LL_GPIO_PIN_3
 *******************************/
static void prvKeyBtnInit(void) {
     /* Enable GPIOA clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    /* Configure PA0 pin as input floating */
    LL_GPIO_InitTypeDef GPIO_InitStructure;
    LL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_DOWN;
    GPIO_InitStructure.Pin = LL_GPIO_PIN_3;
    LL_GPIO_Init(GPIOA, &GPIO_InitStructure);  
}

static bool prvIsKeyBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_3);
}

/******************************
 *  WakeUp Port: GPIOA
 *  WakeUp Pin: LL_GPIO_PIN_0
 *******************************/
static void prvWakeUpBtnInit(void) {
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

static void prvWakeUpBtnEnableIrq(FunctionalState xEnable) {
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

static bool prvIsWakeUpBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0);
}

static uint32_t prvGetBtnStatus(void) {
    uint32_t retValue = 0;    
    if (prvIsInfoBtnPressed()) {
        retValue = 0x01 << 0;
    }    
    if (prvIsApBtnPressed()) {
        retValue = 0x01 << 1;
    }    
    if (prvIsKeyBtnPressed()) {
        retValue = 0x01 << 2;
    }    
    return retValue;    
}

static uint32_t prvBtnStatus = 0;
static uint32_t prvBackUpBtnFlasg __attribute__ ((section (".no_init")));
static uint32_t prvBackUpBtnStatus __attribute__ ((section (".no_init"))); 

static void prvWakeUpBtnCb(void) {   
	vSetBtnStatus(prvGetBtnStatus());	
}

/***********************************************
                Public routine 
************************************************/
void EXTI0_1_IRQHandler(void) {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0)) { 
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
        prvWakeUpBtnCb();
    }   
}

void vBtnInit(void) {
    prvInfoBtnInit();
    prvApBtnInit();
    prvKeyBtnInit();
    prvWakeUpBtnInit();        
    
    vSetBtnStatus(prvGetBtnStatus());        
    
    if (LL_RCC_IsActiveFlag_SFTRST() && 
        prvBackUpBtnFlasg == 0xAA33CC55 &&
        prvGetBtnStatus() == 0x00) {
        prvBtnStatus = prvBackUpBtnStatus;
    }   
    
    prvBackUpBtnStatus = prvBackUpBtnFlasg = 0;
}

void vBtnEnableIrq(FunctionalState xEnable) {
    prvWakeUpBtnEnableIrq(xEnable);
}

uint32_t ulGetBtnStatus(void) {
	uint32_t retval = prvBtnStatus;
    prvBtnStatus = 0;
    return retval;    
}

void vSetBtnStatus(uint32_t status) {
	prvBtnStatus = status;
}

void vBackUpBtnStatus(uint32_t status) {
    prvBackUpBtnFlasg = 0xAA33CC55;
    prvBackUpBtnStatus = status;
}

