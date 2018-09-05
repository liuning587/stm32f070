#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_bus.h"

void vLedInit(void) {
    /* Enable GPIOA clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    /* Configure PA.00 pin as input floating */
    LL_GPIO_InitTypeDef GPIO_InitStructure;
    LL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_LOW;
    GPIO_InitStructure.Pin = LL_GPIO_PIN_5;
    LL_GPIO_Init(GPIOA, &GPIO_InitStructure);
    for (int i = 0; i < 8; i++) {
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
        LL_mDelay(50);
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
        LL_mDelay(50);
    }
}