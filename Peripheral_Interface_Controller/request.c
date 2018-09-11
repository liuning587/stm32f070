#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_rtc.h"
#include "pools.h"
#include "request.h"
#include <usbd_core.h>
#include <usbd_template.h>
#include <usbd_desc.h>
#include <usbd_template.h>
#include <usbd_template_if.h>
#include "pwrctrl.h"

extern USBD_HandleTypeDef USBD_Device;
extern uint8_t  USBD_CUSTOM_Transmit(USBD_HandleTypeDef *pdev, const uint8_t *pData, uint32_t ulDataSize);

char aucBootReason[32];

static void prvRsp(const char *pcStr) {
    struct PoolEntry xEntryPut;
    xEntryPut.ucLen = strlen(pcStr) + 1;
    memcpy(xEntryPut.aucData, pcStr, xEntryPut.ucLen);
    ulPoolsPut(RSP_POOL, &xEntryPut);    
}

void vEcho(void *pArg, uint32_t ulLen) {
    struct PoolEntry xEntryPut;
    xEntryPut.ucLen = ulLen;
    memcpy(xEntryPut.aucData, pArg, xEntryPut.ucLen);
    ulPoolsPut(RSP_POOL, &xEntryPut);
}

void vSetBootReason(const char *pcReason) {
    strcpy(aucBootReason, pcReason);
}

void vGetBootReason(void *pArg, uint32_t ulLen) {
    prvRsp(aucBootReason);
}

void vSetPwr(void *pArg, uint32_t ulLen) {
    vPwrCtrlUsbHub(PWR_OFF);
    vPwrCtrlRasberryPi(PWR_OFF);
    vPwrCtrlEpd(PWR_OFF);
    vPwrCtrlExtUsbDev1(PWR_OFF);
    vPwrCtrlExtUsbDev2(PWR_OFF);
    
    /*  
     *  The Following Wakeup sequence is highly recommended prior to each Standby mode entry
     *  mainly  when using more than one wakeup source this is to not miss any wakeup event.
     *  - Disable all used wakeup sources,
     *  - Clear all related wakeup flags, 
     *  - Re-enable all used wakeup sources,
     *  - Enter the Standby mode.
     */    
    
    /* Enable PWR clock */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  
    /* Disable all used wakeup sources: WKUP pin */
    LL_PWR_DisableWakeUpPin(LL_PWR_WAKEUP_PIN1);    // Btn Int Pin
    LL_PWR_DisableWakeUpPin(LL_PWR_WAKEUP_PIN2);    // Bat Int Pin


    /* Clear all related wakeup flags */
    LL_PWR_ClearFlag_WU();
   
    /* Re-enable all used wakeup sources */
    LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN1); 
    LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN2); 

    
    /* Enter the Standby mode */
    {
        /* Select STANDBY mode */
        LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
        /* Set SLEEPDEEP bit of Cortex System Control Register */
        LL_LPM_EnableDeepSleep();
        /* Request Wait For Interrupt */
        __WFI();
    }
}

void vSetEpdPwr(void *pArg, uint32_t ulLen) {
    if(*(uint8_t *)pArg == 0)
        vPwrCtrlEpd(PWR_OFF);
    else
        vPwrCtrlEpd(PWR_ON);
}


void vGetTime(void *pArg, uint32_t ulLen) {
    char str[32];
    sprintf(str, "%08X\r\n", (int)LL_RTC_TIME_Get(RTC));    
    prvRsp(str);   
}

/* 
 * quick ascii string to hex value 
 * note: only for 8 character hex vale 
 **/
uint32_t ascii2bcd(char *cHexStr, uint32_t ulLen) {
   #define NUMB_COUNT  10
    const uint8_t numb[NUMB_COUNT] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    uint8_t digits[8];
    uint8_t *pDgits = &digits[0];
    uint32_t count = sizeof(digits) > ulLen ? ulLen : sizeof(digits);
    // convert each character to hex value
    for(uint32_t i = 0 ; i < count ; i++) {
        for (uint32_t j = 0; j < NUMB_COUNT; j++) {
            *pDgits = cHexStr[i] == numb[j] ? j : 0xFF;
            if (*pDgits != 0xFF)
                break;
        }
        if (*pDgits == 0xFF)
            break;
        pDgits++;
    }
        
    uint32_t loops = pDgits - &digits[0];
    uint32_t ret = 0;
    for (uint32_t i = 0; i < loops; i++) {
        pDgits--;
        ret |= (uint32_t)(*pDgits * (0x01 << (i * 4)));
    }
    return ret;
}


void vSetTime(void *pArg, uint32_t ulLen) {
	int32_t time = ascii2bcd((char *)pArg, ulLen);

    LL_RTC_TimeTypeDef RTC_TimeStruct;        
    RTC_TimeStruct.TimeFormat = LL_RTC_HOURFORMAT_24HOUR;
    RTC_TimeStruct.Hours = __LL_RTC_GET_HOUR(time);
    RTC_TimeStruct.Minutes = __LL_RTC_GET_MINUTE(time);
    RTC_TimeStruct.Seconds = __LL_RTC_GET_SECOND(time);
    LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_TimeStruct);
    
    LL_PWR_EnableBkUpAccess();
    LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_TimeStruct); 

    LL_PWR_DisableBkUpAccess();
    
    if (LL_RTC_TIME_Get(RTC) == time) {
        prvRsp("ACK");        
    } else {
        prvRsp("NACK");                
    }    
}

//#define LL_RTC_WEEKDAY_MONDAY              ((uint8_t)0x01U) /*!< Monday    */
//#define LL_RTC_WEEKDAY_TUESDAY             ((uint8_t)0x02U) /*!< Tuesday   */
//#define LL_RTC_WEEKDAY_WEDNESDAY           ((uint8_t)0x03U) /*!< Wednesday */
//#define LL_RTC_WEEKDAY_THURSDAY            ((uint8_t)0x04U) /*!< Thrusday  */
//#define LL_RTC_WEEKDAY_FRIDAY              ((uint8_t)0x05U) /*!< Friday    */
//#define LL_RTC_WEEKDAY_SATURDAY            ((uint8_t)0x06U) /*!< Saturday  */
//#define LL_RTC_WEEKDAY_SUNDAY              ((uint8_t)0x07U) /*!< Sunday    */

//#define LL_RTC_MONTH_JANUARY               ((uint8_t)0x01U)  /*!< January   */
//#define LL_RTC_MONTH_FEBRUARY              ((uint8_t)0x02U)  /*!< February  */
//#define LL_RTC_MONTH_MARCH                 ((uint8_t)0x03U)  /*!< March     */
//#define LL_RTC_MONTH_APRIL                 ((uint8_t)0x04U)  /*!< April     */
//#define LL_RTC_MONTH_MAY                   ((uint8_t)0x05U)  /*!< May       */
//#define LL_RTC_MONTH_JUNE                  ((uint8_t)0x06U)  /*!< June      */
//#define LL_RTC_MONTH_JULY                  ((uint8_t)0x07U)  /*!< July      */
//#define LL_RTC_MONTH_AUGUST                ((uint8_t)0x08U)  /*!< August    */
//#define LL_RTC_MONTH_SEPTEMBER             ((uint8_t)0x09U)  /*!< September */
//#define LL_RTC_MONTH_OCTOBER               ((uint8_t)0x10U)  /*!< October   */
//#define LL_RTC_MONTH_NOVEMBER              ((uint8_t)0x11U)  /*!< November  */
//#define LL_RTC_MONTH_DECEMBER              ((uint8_t)0x12U)  /*!< December  */

void vGetDate(void *pArg, uint32_t ulLen) {
    char str[32];
    sprintf(str, "%08X\r\n", (int)LL_RTC_DATE_Get(RTC));
    prvRsp(str);   
}

void vSetDate(void *pArg, uint32_t ulLen) {
    uint32_t date = ascii2bcd((char *)pArg, ulLen);
    
    LL_RTC_DateTypeDef RTC_DateStruct;
    RTC_DateStruct.WeekDay = __LL_RTC_GET_WEEKDAY(date);
    RTC_DateStruct.Month = __LL_RTC_GET_MONTH(date);
    RTC_DateStruct.Day = __LL_RTC_GET_DAY(date);
    RTC_DateStruct.Year = __LL_RTC_GET_YEAR(date);
    
    LL_PWR_EnableBkUpAccess();
    LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_DateStruct);  
    LL_PWR_DisableBkUpAccess();

    if (LL_RTC_DATE_Get(RTC) == date) {
        prvRsp("ACK");        
    } else {
        prvRsp("NACK");                
    }   
}

void vGetAlarm(void *pArg, uint32_t ulLen) {
    char str[32];
    sprintf(str, "%08X\r\n", (int)LL_RTC_ALMA_GetTime(RTC));
    prvRsp(str);   
}

void vSetAlarm(void *pArg, uint32_t ulLen) {
    uint32_t time = ascii2bcd((char *)pArg, ulLen);
    uint32_t format12_24 = LL_RTC_ALMA_TIME_FORMAT_AM;
    uint32_t hh = __LL_RTC_GET_HOUR(time);
    uint32_t mm = __LL_RTC_GET_MINUTE(time);
    uint32_t ss = __LL_RTC_GET_SECOND(time);
    /* Enable PWR clock */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    /* Allow access to Backup */
    LL_PWR_EnableBkUpAccess();
    /* Step 1. Disable the RTC registers Write protection */
    LL_RTC_DisableWriteProtection(RTC);
    /* Step 2. Disable alarm A*/       
    LL_RTC_ALMA_Disable(RTC);
    /* Step 3. Check that the RTC_ALRMAR register can be accessed*/
    while (LL_RTC_IsActiveFlag_ALRAW(RTC) == 0) ;
    /* Step 4. Configure the alarm */   
    LL_RTC_ALMA_DisableWeekday(RTC);
    LL_RTC_ALMA_SetMask(RTC, LL_RTC_ALMA_MASK_DATEWEEKDAY);
    LL_RTC_ALMA_ConfigTime(RTC, format12_24, hh, mm, ss);    
    /* Step 5. Re-enable alarm A & Enable alarm interrupt */
    LL_RTC_ALMA_Enable(RTC); 
    LL_RTC_EnableIT_ALRA(RTC);
    /* Step 6. Enable the RTC registers Write protection*/
    LL_RTC_EnableWriteProtection(RTC); 
    /* Don't allow access to Backup */
    LL_PWR_DisableBkUpAccess();
    
    prvRsp("ACK");        
}

void vSetLed(void *pArg, uint32_t ulLen) {
    prvRsp(__FUNCTION__);     
}

void vDummy(void *pArg, uint32_t ulLen) {
    prvRsp(__FUNCTION__);   
}

pfRequestCb_t xRequestCb[] = { 
        vEcho,          //A
        vGetBootReason, //B
        vSetPwr,        //C
        vSetEpdPwr,     //D
        vGetTime,       //E
        vSetTime,       //F
        vGetDate,       //G
        vSetDate,       //H
        vGetAlarm,      //I
        vSetAlarm,      //J
        vSetLed,
};

#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))
pfRequestCb_t xGetReqCb(uint32_t ulReqId) {
    return ulReqId <0x41 || ulReqId > (0x40 + COUNTOF(xRequestCb))  ? vDummy : xRequestCb[ulReqId%0x41];    
}
