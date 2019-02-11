#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_pwr.h"

extern void vSetSysTickCallBack(void *callback);

static uint32_t prvOnTime;
static uint32_t prvOffTime;
static uint32_t prvDuration;

static void prvLedCb(void) {
	uint32_t curTick = HAL_GetTick();
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);

	if (curTick > prvDuration) {
		vSetSysTickCallBack(NULL);	
		return;
	} 
	
	if (curTick % (prvOnTime + prvOffTime) < prvOnTime) {
		LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
	} 
} 

void vLedTurnOn(uint32_t ulOnTime, uint32_t OffTime, uint32_t ulDuration) {
	prvOnTime = ulOnTime;
	prvOffTime = OffTime;
	prvDuration = HAL_GetTick() + ulDuration;	
	vSetSysTickCallBack(prvLedCb);	
}

void vLedBrighten(void) {
	for (int i = 0; i < 10; i++) {
		vLedTurnOn(i, 10 - i, 1 * 100);
		uint32_t cur = HAL_GetTick();
		while (HAL_GetTick() - cur < 100) {
			__WFI();
		} 
	}
}

void vLedDarken(void) {
	for (int i = 0; i < 10; i++) {
		vLedTurnOn(10-i, i , 1 * 100);
		uint32_t cur = HAL_GetTick();
		while (HAL_GetTick() - cur < 100) {
			__WFI();
		} 
	}
}

void vLedInit(void) {
    /* Enable GPIOA clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	/* Set PA4, PA5 as High to disable led */
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4 | LL_GPIO_PIN_5);
    /* Configure PA4, PA5 pin as output */
    LL_GPIO_InitTypeDef GPIO_InitStructure;
    LL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_LOW;
    GPIO_InitStructure.Pin = LL_GPIO_PIN_4 | LL_GPIO_PIN_5;
    LL_GPIO_Init(GPIOA, &GPIO_InitStructure);
}