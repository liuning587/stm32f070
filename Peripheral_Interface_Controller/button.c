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

/******************************
 *  Cfg Port: GPIOA
 *  Cfg Pin: LL_GPIO_PIN_1
 *******************************/
static void prvCfgBtnHwInit(void) {
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

static bool prvIsCfgBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_1);
}

/******************************
 *  State Port: GPIOA
 *  State Pin: LL_GPIO_PIN_2
 *******************************/
static void prvStaBtnHwInit(void) {
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

static bool prvIsStaBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_2);
}

/******************************
 *  Defult Port: GPIOA
 *  Defult Pin: LL_GPIO_PIN_3
 *******************************/
static void prvDefBtnHwInit(void) {
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

static bool prvIsDefBtnPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_3);
}

/******************************
 *  WakeUp Port: GPIOA
 *  WakeUp Pin: LL_GPIO_PIN_0
 *******************************/
static void prvWakeUpBtnHwInit(void) {
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

static void prvWakeUpBtnIrqEnable(FunctionalState xEnable) {
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

struct button {
	uint32_t ulHeldMillis;
	uint32_t ulLastHeldMillis;
	bool bPreState;
	bool bCurState;
};

static struct button prvCfgBtn;
static struct button prvStaBtn;
static struct button prvDefBtn;
static struct button prvBackUpCfgBtn __attribute__ ((section (".no_init")));
static struct button prvBackUpStaBtn __attribute__ ((section (".no_init")));
static struct button prvBackUpDefBtn __attribute__ ((section (".no_init")));
static uint32_t prvBackUpFlag __attribute__ ((section (".no_init")));
/***********************************************
                Public routine 
************************************************/
static void prvButtonHeldDetect(struct button *pxBtn, bool bIsBtnPressed) {
	pxBtn->bCurState = bIsBtnPressed;
	
	if (pxBtn->bPreState == false && pxBtn->bCurState == false) {
		pxBtn->ulHeldMillis = 0;
	} else if (pxBtn->bPreState == true && pxBtn->bCurState == false) {
		pxBtn->ulLastHeldMillis = pxBtn->ulHeldMillis > 50 ? pxBtn->ulHeldMillis : pxBtn->ulLastHeldMillis;			
	} else if (pxBtn->bCurState == true) {
		pxBtn->ulHeldMillis++;
	}
	
	pxBtn->bPreState = pxBtn->bCurState;	
}

void vButtonHeldDetectCb(void) {
	prvButtonHeldDetect(&prvCfgBtn, prvIsCfgBtnPressed());
	prvButtonHeldDetect(&prvStaBtn, prvIsStaBtnPressed());
	prvButtonHeldDetect(&prvDefBtn, prvIsDefBtnPressed());
}
	
uint32_t ulGetCfgHeldMillis(void) {
	uint32_t ret = prvCfgBtn.ulLastHeldMillis;
	prvCfgBtn.ulLastHeldMillis = 0;
	return ret;
}

uint32_t ulGetStaHeldMillis(void) {
	uint32_t ret = prvStaBtn.ulLastHeldMillis;
	prvStaBtn.ulLastHeldMillis = 0;
	return ret;
}

uint32_t ulGetDefHeldMillis(void) {
	uint32_t ret = prvDefBtn.ulLastHeldMillis;
	prvDefBtn.ulLastHeldMillis = 0;
	return ret;
}

void vBtnInit(void) {
	prvCfgBtnHwInit();
	prvStaBtnHwInit();
	prvDefBtnHwInit();
	prvWakeUpBtnHwInit();        
        
	if (LL_RCC_IsActiveFlag_SFTRST() && prvBackUpFlag == 0xAA33CC55) {
		prvCfgBtn = prvBackUpCfgBtn;
		prvStaBtn = prvBackUpStaBtn;
		prvDefBtn = prvBackUpDefBtn;
	}   
	
	memset(&prvBackUpCfgBtn, 0x00, sizeof(prvBackUpCfgBtn));
	memset(&prvBackUpStaBtn, 0x00, sizeof(prvBackUpStaBtn));
	memset(&prvBackUpDefBtn, 0x00, sizeof(prvBackUpDefBtn));
	prvBackUpFlag = 0;
	
	prvWakeUpBtnIrqEnable(ENABLE);
}

bool bAnyBtnIsPressed(void) {
	return prvCfgBtn.bCurState || prvStaBtn.bCurState || prvDefBtn.bCurState;
}

void vBtnBackUp(void) {
    prvBackUpFlag = 0xAA33CC55;
	prvBackUpCfgBtn = prvCfgBtn;
	prvBackUpStaBtn = prvStaBtn;
	prvBackUpDefBtn = prvDefBtn;
}

void EXTI0_1_IRQHandler(void) {	
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0)) { 
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
    }   
}

