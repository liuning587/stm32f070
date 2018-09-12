#include "stm32f0xx.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_rtc.h"
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
#include "button.h"
#include "led.h"
#include "rtc.h"

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

void vSysClkConfig(void) {
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
    do {
        __WFI();        
    } while (ulPoolsGet(REQ_POOL, (struct PoolEntry*)&xReqPoolEntry) == 0);
    uint32_t len = xReqPoolEntry.ucLen;
    uint32_t reqId = xReqPoolEntry.aucData[0];
    uint8_t *pArg = &xReqPoolEntry.aucData[1];
    xGetReqCb(reqId)(pArg, len - 1);  
    while (ulPoolsGet(RSP_POOL, (struct PoolEntry*)&xRspPoolEntry)) {        
        vResponse(xRspPoolEntry.aucData, xRspPoolEntry.ucLen);     
    } 
}

static void prvSetBootReason(void) {
    /* Enable PWR clock */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    if (LL_PWR_IsActiveFlag_WU() && LL_PWR_IsActiveFlag_SB()) {
        if (bIsApBtnPressed()) {
            vSetBootReason("AP");
        } else if (bIsInfoBtnPressed()) {
            vSetBootReason("INFO");
        } else if (bIsRecoveryBtnPressed()) {
            vSetBootReason("RECOVERY");
        } else {
            vSetBootReason("RTC");
        }
    } else {
        vSetBootReason("PWR_ON");
    }       
}

void RTC_IRQHandler(void) {
    __BKPT(255);
}

extern void vPwrCtrlInit(void);

static void prvMspInit(void) {
    /* MCU Support Package */
    LL_FLASH_EnablePrefetch();    
    /* SVC_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SVC_IRQn, 0, 0);
    /* PendSV_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);    
}

static void prvBspInit(void) {
    vSysClkConfig();
    vPwrCtrlInit();       
    vLedInit();
    vRtcInit();
    vApBtnInit();
    vInfoBtnInit();
    vRecoveryBtnInit();   
    prvUsbDeviceInit(); 
}

int main(void) {
    /* Hardware Init */
    prvMspInit();
    prvBspInit();
    /* Application Init */
    vPoolsInit();     
    /* Application Start */
    prvSetBootReason();  
    prvWaitFirstReqFromHost();        
    /* Enable Btn Irq */
    vRecoveryBtnEnableIrq(ENABLE);
    vInfoBtnEnableIrq(ENABLE);
    vApBtnEnableIrq(ENABLE);
	for (;;) { /* main loop */    	    	
    	__WFI();   
    	struct PoolEntry xReqPoolEntry, xRspPoolEntry, xNotifyPoolEntry; 	
    	while (ulPoolsGet(REQ_POOL, (struct PoolEntry*)&xReqPoolEntry)) {        	
        	uint32_t len = xReqPoolEntry.ucLen;
        	uint32_t reqId = xReqPoolEntry.aucData[0];
        	uint8_t *pArg = &xReqPoolEntry.aucData[1];
        	xGetReqCb(reqId)(pArg, len - 1);        	
    	    while (ulPoolsGet(RSP_POOL, (struct PoolEntry*)&xRspPoolEntry)) {        
        	    vResponse(xRspPoolEntry.aucData, xRspPoolEntry.ucLen);     
    	    } 
    	} 
    	if (ulPoolsGet(NOTIFY_POOL, (struct PoolEntry*)&xNotifyPoolEntry)) {        
        	vNotify(xNotifyPoolEntry.aucData, xNotifyPoolEntry.ucLen);     
    	}
	}
}

static void prvDefualtSysTick(void) {
    switch (HAL_GetTick() % 3000) {
    case 0:
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
        break;
    case 30:
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
        break;   
    case 120:
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
        break;
    case 150:
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
        break;  
    }
}

static void(*pfSysTickCb)(void) = prvDefualtSysTick;

void* vGetSysTickCallBack(void) {
    return pfSysTickCb;
}

void vSetSysTickCallBack(void *callback) {
    if (callback == NULL) 
        pfSysTickCb = prvDefualtSysTick;
    else
        pfSysTickCb = callback;
}

void HAL_SYSTICK_Callback(void) {
    pfSysTickCb();
}

void SysTick_Handler(void) {
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void USB_IRQHandler(void) {
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

void _Error_Handler(void) {
     __BKPT(255);      
} 


