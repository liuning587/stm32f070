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
#include "stm32f0xx_ll_i2c.h"
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
    } while (ulPoolsGet(EP1_REQ_POOL, (struct PoolEntry*)&xReqPoolEntry) == 0);
    uint32_t len = xReqPoolEntry.ucLen;
    uint32_t reqId = xReqPoolEntry.aucData[0];
    uint8_t *pArg = &xReqPoolEntry.aucData[1];
    xGetReqCb(reqId)(pArg, len - 1);  
    while (ulPoolsGet(EP1_RSP_POOL, (struct PoolEntry*)&xRspPoolEntry)) {        
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

static void prvI2CInit(void) {
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

    LL_GPIO_InitTypeDef GPIO_InitStruct;
    LL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;

    GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);    
    
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

    LL_I2C_EnableClockStretching(I2C1);  
    LL_I2C_DisableOwnAddress1(I2C1);
    LL_I2C_DisableOwnAddress2(I2C1);
    LL_I2C_DisableGeneralCall(I2C1);
    LL_I2C_InitTypeDef I2C_InitStruct = {    
        .PeripheralMode = LL_I2C_MODE_I2C,
        .Timing = 0x200090E,//0x20303E5D,
        .AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE,
        .DigitalFilter = 0,
        .OwnAddress1 = 0,
        .TypeAcknowledge = LL_I2C_ACK,
        .OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT,
    };
    LL_I2C_Init(I2C1, &I2C_InitStruct);
    LL_I2C_DisableAutoEndMode(I2C1); 
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
    vLedInit();
    vRtcInit();
    vApBtnInit();
    vInfoBtnInit();
    vRecoveryBtnInit();        
    prvI2CInit();   	
    vPwrCtrlInit(); 		
    prvUsbDeviceInit();   
    /* Enable Systick interrupt */
	LL_SYSTICK_EnableIT();
}

int main(void) {
    /* Hardware Init */
    prvMspInit();	
    prvBspInit();
    /* Application Init */
    vPoolsInit();     
    /* Application Start */
    prvSetBootReason();  
//    prvWaitFirstReqFromHost();        
    /* Enable Btn Irq */
//    vRecoveryBtnEnableIrq(ENABLE);
//    vInfoBtnEnableIrq(ENABLE);
//    vApBtnEnableIrq(ENABLE);
	for (;;) { /* main loop */    	    	
        __WFI();   
    	struct PoolEntry xReqPoolEntry, xRspPoolEntry, xNotifyPoolEntry, xCtrlReqPoolEntry; 	
		
        while (ulPoolsGet(I2C_REQ_POOL, (struct PoolEntry*)&xCtrlReqPoolEntry)) {        	
            extern void vI2cReqCb(void *pvArg, uint32_t ulSize);
            vI2cReqCb((void *)xCtrlReqPoolEntry.aucData, xCtrlReqPoolEntry.ucLen);
            while (ulPoolsGet(I2C_RSP_POOL, (struct PoolEntry*)&xRspPoolEntry)) {        
        	    vI2cResponse(xRspPoolEntry.aucData, xRspPoolEntry.ucLen);     
    	    } 
    	} 

    	while (ulPoolsGet(EP1_REQ_POOL, (struct PoolEntry*)&xReqPoolEntry)) {        	
        	uint32_t len = xReqPoolEntry.ucLen;
        	uint32_t reqId = xReqPoolEntry.aucData[0];
        	uint8_t *pArg = &xReqPoolEntry.aucData[1];
        	xGetReqCb(reqId)(pArg, len - 1);        	
    	    while (ulPoolsGet(EP1_RSP_POOL, (struct PoolEntry*)&xRspPoolEntry)) {        
        	    vResponse(xRspPoolEntry.aucData, xRspPoolEntry.ucLen);     
    	    } 
    	} 
    	
    	if (ulPoolsGet(EP1_NOTIFY_POOL, (struct PoolEntry*)&xNotifyPoolEntry)) {        
        	vNotify(xNotifyPoolEntry.aucData, xNotifyPoolEntry.ucLen);     
    	}
	}
}

static void prvDefualtSysTick(void) {
    switch (HAL_GetTick() % 3000) {
    case 0:
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
        break;
    case 200:
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
        break;   
    case 300:
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
        break;
    case 500:
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
        break;  
    case 600:
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
        break;
    case 800:
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
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


