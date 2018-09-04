
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

extern PCD_HandleTypeDef hpcd_USB_FS;
extern USBD_ClassTypeDef USBD_CUSTOM_ClassDriver;
USBD_HandleTypeDef USBD_Device;

static void Error_Handler(void) {
    __BKPT(255);        
}

static void prvUsbDeviceInit(void) {
    USBD_Init(&USBD_Device, &FS_Desc, 0);
    USBD_RegisterClass(&USBD_Device, &USBD_CUSTOM_ClassDriver);
    USBD_CUSTOM_RegisterInterface(&USBD_Device, &USBD_CUSTOM_Template_fops);
	USBD_Start(&USBD_Device);
}

static void prvLedInit(void) {
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
}

static void prvDefaultPinInit(void) {
     // TODO...
}

static bool prvIsDefaultPinPressed(void) {
    // TODO...
    return false;        
}

static void prvDefaultPinCb(void) {
    static uint32_t tick = 0;
    if (HAL_GetTick() - tick > 50) {
        struct PoolEntry xEntryPut;
        xEntryPut.ucLen = sizeof("DEFAULT");
        memcpy(xEntryPut.aucData, "DEFAULT", xEntryPut.ucLen);
        ulPoolsPut(NOTIFY_POOL, &xEntryPut);
        tick = HAL_GetTick();
    }
}

static void prvApPinInit(void) {
     /* Enable GPIOA clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    /* Configure PA0 pin as input floating */
    LL_GPIO_InitTypeDef GPIO_InitStructure;
    LL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;
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
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);
    /* Enable and set EXTI lines 0 to 1 Interrupt to the lowest priority */
    NVIC_SetPriority(EXTI0_1_IRQn, 2);
}

static bool prvIsApPinPressed(void) {
    return LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0);
}

static void prvApPinCb(void) {
    static uint32_t tick = 0;
    if (HAL_GetTick() - tick > 50) {
        struct PoolEntry xEntryPut;
        xEntryPut.ucLen = sizeof("AP");
        memcpy(xEntryPut.aucData, "AP", xEntryPut.ucLen);
        ulPoolsPut(NOTIFY_POOL, &xEntryPut);
        tick = HAL_GetTick();
    }
}

static void prvInfoPinInit(void) {
    /* Enable GPIOC clock */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    /* Configure PC13 pin as Input */ 
    LL_GPIO_InitTypeDef GPIO_InitStructure;
    LL_GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
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
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);
    /* Enable and set EXTI lines 4 to 15 Interrupt to the lowest priority */
    NVIC_SetPriority(EXTI4_15_IRQn, 2);
}

static bool prvIsInfoPinPressed(void) {
    return !LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13);
}

static void  prvInfoPinCb(void) {
    static uint32_t tick = 0;
    if (HAL_GetTick() - tick > 50) {
        struct PoolEntry xEntryPut;
        xEntryPut.ucLen = sizeof("INFO");
        memcpy(xEntryPut.aucData, "INFO", xEntryPut.ucLen);
        ulPoolsPut(NOTIFY_POOL, &xEntryPut);
        tick = HAL_GetTick();
    }
}

void SystemClock_Config(void) {
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    if (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1) {
        Error_Handler();  
    }
    LL_RCC_HSE_Enable();

    /* Wait till HSE is ready */
    while (LL_RCC_HSE_IsReady() != 1);
    
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLL_MUL_6, LL_RCC_PREDIV_DIV_1);

    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1) ;
    
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);
    
    LL_Init1msTick(48000000);

    LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);

    LL_SetSystemCoreClock(48000000);

    LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_PLL);

    /* SysTick_IRQn interrupt configuration */
    NVIC_SetPriority(SysTick_IRQn, 0);

}

static void prvWaitFirstReqFromHost(void) {
    uint32_t ret;
    struct PoolEntry xReqPoolEntry, xRspPoolEntry;   
    while (ulPoolsGet(REQ_POOL, (struct PoolEntry*)&xReqPoolEntry) == 0) {
        __WFI();
    }
    uint32_t len = xReqPoolEntry.ucLen;
    uint32_t reqId = xReqPoolEntry.aucData[0];
    uint8_t *pArg = &xReqPoolEntry.aucData[1];
    xGetReqCb(reqId)(pArg, len - 1);  
    if (ulPoolsGet(RSP_POOL, (struct PoolEntry*)&xRspPoolEntry)) {        
        vResponse(xRspPoolEntry.aucData, xRspPoolEntry.ucLen);     
    } 
}

static void prvSetBootReason(void) {
    if (prvIsApPinPressed()) {
        vSetBootReason("AP");
    } else if (prvIsInfoPinPressed()) {
        vSetBootReason("INFO");
    } else if (prvIsDefaultPinPressed()) {
        vSetBootReason("DEFAULT");
    } else {
        vSetBootReason("RTC");
    }    
}

int main(void) {
    /* Hardware Init */
	HAL_Init();
	SystemClock_Config();
    prvUsbDeviceInit();
    prvLedInit();
    prvApPinInit();
    prvInfoPinInit();
    prvDefaultPinInit();    
    /* Application Init */
    vPoolsInit();     
    /* Application Start */
    prvSetBootReason();  
    prvWaitFirstReqFromHost();
	for (;;) { /* main loop */    	
    	NVIC_EnableIRQ(EXTI0_1_IRQn);
    	NVIC_EnableIRQ(EXTI4_15_IRQn);    	
    	__WFI();   
    	struct PoolEntry xReqPoolEntry, xRspPoolEntry, xNotifyPoolEntry; 	
    	while (ulPoolsGet(REQ_POOL, (struct PoolEntry*)&xReqPoolEntry)) {        	
        	uint32_t len = xReqPoolEntry.ucLen;
        	uint32_t reqId = xReqPoolEntry.aucData[0];
        	uint8_t *pArg = &xReqPoolEntry.aucData[1];
        	xGetReqCb(reqId)(pArg, len - 1);        	
    	    if (ulPoolsGet(RSP_POOL, (struct PoolEntry*)&xRspPoolEntry)) {        
        	    vResponse(xRspPoolEntry.aucData, xRspPoolEntry.ucLen);     
    	    } 
    	} 
    	if (ulPoolsGet(NOTIFY_POOL, (struct PoolEntry*)&xNotifyPoolEntry)) {        
        	vNotify(xNotifyPoolEntry.aucData, xNotifyPoolEntry.ucLen);     
    	}
	}
}

void HAL_SYSTICK_Callback(void) {
    static uint32_t tick = 0;
    if (HAL_GetTick() - tick > 250) {
        tick = HAL_GetTick();
        LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_5);
    }
}

void SysTick_Handler(void) {
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void USB_IRQHandler(void) {
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

void EXTI0_1_IRQHandler(void) {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0)) { 
        NVIC_DisableIRQ(EXTI0_1_IRQn);
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
        prvApPinCb();
    }   
}
void EXTI4_15_IRQHandler(void) {
    /* EXTI line interrupt detected */
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13)) { 
        NVIC_DisableIRQ(EXTI4_15_IRQn);
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        prvInfoPinCb();
    }   
}

void _Error_Handler(void) {
     __BKPT(255);      
} 


