#include "stm32f0xx.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_rtc.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_cortex.h"


#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_pwr.h"
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
#include "button.h"
#include "led.h"
#include "rtc.h"


void vRtcInit(void) {
    if (!LL_PWR_IsActiveFlag_SB()) {
        /* Enable PWR clock */
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
        /* Allow access to Backup, reset RTC Domain */
        LL_PWR_EnableBkUpAccess();
        /* Reset backup domain */
        LL_RCC_ForceBackupDomainReset();
        LL_RCC_ReleaseBackupDomainReset();
        
        LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_HIGH);
        LL_RCC_LSE_Enable();
                
        /* Wait till LSE is ready */
        while (LL_RCC_LSE_IsReady() != 1);
        LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
        LL_RCC_EnableRTC();
        
        LL_RTC_InitTypeDef RTC_InitStruct;
        LL_RTC_StructInit(&RTC_InitStruct);      
        LL_RTC_Init(RTC, &RTC_InitStruct);

        LL_RTC_TimeTypeDef RTC_TimeStruct;        
        RTC_TimeStruct.Hours = 0;
        RTC_TimeStruct.Minutes = 0;
        RTC_TimeStruct.Seconds = 0;
        LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_TimeStruct);
        
        LL_RTC_DateTypeDef RTC_DateStruct;
        RTC_DateStruct.WeekDay = LL_RTC_WEEKDAY_MONDAY;
        RTC_DateStruct.Month = LL_RTC_MONTH_JANUARY;
        RTC_DateStruct.Day = 0x01;
        RTC_DateStruct.Year = 0x18;
        LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_DateStruct);
        
        /* Deny access to Backup, reset RTC Domain */
        LL_PWR_DisableBkUpAccess();
    }
}