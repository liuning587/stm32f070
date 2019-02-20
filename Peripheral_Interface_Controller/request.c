#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_rtc.h"
#include "stm32f0xx_ll_i2c.h"
#include "pools.h"
#include "request.h"
#include <usbd_core.h>
#include <usbd_template.h>
#include <usbd_desc.h>
#include <usbd_template.h>
#include <usbd_template_if.h>
#include "pwrctrl.h"
#include "button.h"
#include "board_info.h"
#include "led.h"

extern USBD_HandleTypeDef USBD_Device;
extern uint8_t USBD_CUSTOM_Transmit(USBD_HandleTypeDef *pdev, uint8_t epNum, const uint8_t *pData, uint32_t ulDataSize);
static uint32_t bIsAlarmSet = 0;

static void prvRsp(const char *pcStr) {
    struct PoolEntry xEntryPut;
    xEntryPut.ucLen = strlen(pcStr) + 1;
    memcpy(xEntryPut.aucData, pcStr, xEntryPut.ucLen);
    ulPoolsPut(EP1_RSP_POOL, &xEntryPut);
}

void vEcho(void *pArg, uint32_t ulLen) {
    struct PoolEntry xEntryPut;
    xEntryPut.ucLen = ulLen;
    memcpy(xEntryPut.aucData, pArg, xEntryPut.ucLen);
    ulPoolsPut(EP1_RSP_POOL, &xEntryPut);
}

static void prvEnterLPM(void) {
    /* Enable PWR clock */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
        
    /*
    *  The Following Wakeup sequence is highly recommended prior to each Standby mode entry
    *  mainly  when using more than one wakeup source this is to not miss any wakeup event.
    *  1. Disable all used wakeup sources,
    *  2. Clear all related wakeup flags,
    *  3. Re-enable all used wakeup sources,
    *  4. Enter the Standby mode.
    */
        
    /* 1. Disable all used wakeup sources: WKUP pin */
    LL_PWR_DisableWakeUpPin(LL_PWR_WAKEUP_PIN1); // Button Int Pin
//        LL_PWR_DisableWakeUpPin(LL_PWR_WAKEUP_PIN2); // Battery Int Pin

    /* 2. Clear all related wakeup flags */
    LL_PWR_ClearFlag_WU();

    /* 3. Re-enable all used wakeup sources */
    LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN1);
//        LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN2);

    /* 4. Enter the Standby mode */
    LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY); // Select STANDBY mode        
    LL_LPM_EnableDeepSleep(); // Set SLEEPDEEP bit of Cortex System Control Register
    __WFI(); // Request Wait For Interrupt
}

void vEnterLPM(void *pArg, uint32_t ulLen) {
    for (int delay = atoi(pArg); delay > 0; delay--) {
        __WFI();
    }

    /* DeInit usb device */
    USBD_DeInit(&USBD_Device);

    /* Power Off all */
    vPwrCtrlExtUsbDev1(PWR_OFF);
    vPwrCtrlExtUsbDev2(PWR_OFF);
    vPwrCtrlEpd(PWR_OFF);
    vPwrCtrlUsbHub(PWR_OFF);
    vPwrCtrlRasberryPi(PWR_OFF);
    
	vLedDarken();

	if (bAnyBtnIsPressed() || bIsAlarmSet == 0) {
		vBtnBackUp();
        NVIC_SystemReset();
    } else  {        
        prvEnterLPM();
    }
}

void vSetEpdPwr(void *pArg, uint32_t ulLen) {
    if (*(uint8_t *)pArg == 0)
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
    /* Set AlarmSet Flag */
    bIsAlarmSet = 1;
    prvRsp("ACK");
}

void vSetLed(void *pArg, uint32_t ulLen) {
    prvRsp(__FUNCTION__);
}

void vDummy(void *pArg, uint32_t ulLen) {
    prvRsp(__FUNCTION__);
}

void vReqsCfgHeldMillis(void *pArg, uint32_t ulLen) {
    char str[8];
    sprintf(str, "%d", (int)ulGetCfgHeldMillis());
    prvRsp(str);
}

void vReqsStaHeldMillis(void *pArg, uint32_t ulLen) {
    char str[8];
    sprintf(str, "%d", (int)ulGetStaHeldMillis());
    prvRsp(str);
}

void vReqsDefHeldMillis(void *pArg, uint32_t ulLen) {
    char str[8];
    sprintf(str, "%d", (int)ulGetDefHeldMillis());
    prvRsp(str);
}

void vReqsInfo(void *pArg, uint32_t ulLen) {
	#define MAX_STR_LEN 63
    char str[MAX_STR_LEN + 1];
	uint32_t infoType = ascii2bcd((char *)pArg, ulLen);
	vGetInfo(str, MAX_STR_LEN, infoType);
    prvRsp(str);
}

pfRequestCb_t xRequestCb[ ] = {
    // A
    vEcho,
    // B
    vReqsInfo,
    // C
    vEnterLPM,
    // D
    vSetEpdPwr,
    // E
    vGetTime,
    // F
    vSetTime,
    // G
    vGetDate,
    // H
    vSetDate,
    // I
    vGetAlarm,
    // J
    vSetAlarm,
    // K
    vSetLed,
	// L
	vReqsCfgHeldMillis,
    // M
    vReqsStaHeldMillis,
    // N
    vReqsDefHeldMillis,
    // O
};

#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))
pfRequestCb_t xGetReqCb(uint32_t ulReqId) {
    return ulReqId <0x41 || ulReqId > (0x40 + COUNTOF(xRequestCb))  ? vDummy : xRequestCb[ulReqId % 0x41];
}

/*
 *  I2C
 *
 **/
#define CMD_ECHO		    0
#define CMD_GET_FUNC		1
#define CMD_SET_DELAY		2
#define CMD_GET_STATUS		3

#define CMD_I2C_IO		    4
#define CMD_I2C_IO_BEGIN	(1<<0)
#define CMD_I2C_IO_END		(1<<1)

/* To determine what functionality is present */
#define I2C_FUNC_I2C			0x00000001
#define I2C_FUNC_10BIT_ADDR		0x00000002
#define I2C_FUNC_PROTOCOL_MANGLING	0x00000004 /* I2C_M_{REV_DIR_ADDR,NOSTART,..} */
#define I2C_FUNC_SMBUS_HWPEC_CALC	0x00000008 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_READ_WORD_DATA_PEC  0x00000800 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA_PEC 0x00001000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_PROC_CALL_PEC	0x00002000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL_PEC 0x00004000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL	0x00008000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_QUICK		0x00010000
#define I2C_FUNC_SMBUS_READ_BYTE	0x00020000
#define I2C_FUNC_SMBUS_WRITE_BYTE	0x00040000
#define I2C_FUNC_SMBUS_READ_BYTE_DATA	0x00080000
#define I2C_FUNC_SMBUS_WRITE_BYTE_DATA	0x00100000
#define I2C_FUNC_SMBUS_READ_WORD_DATA	0x00200000
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA	0x00400000
#define I2C_FUNC_SMBUS_PROC_CALL	0x00800000
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA	0x01000000
#define I2C_FUNC_SMBUS_WRITE_BLOCK_DATA 0x02000000
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK	0x04000000 /* I2C-like block xfer  */
#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK	0x08000000 /* w/ 1-byte reg. addr. */
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK_2	 0x10000000 /* I2C-like block xfer  */
#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK_2 0x20000000 /* w/ 2-byte reg. addr. */
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA_PEC  0x40000000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_WRITE_BLOCK_DATA_PEC 0x80000000 /* SMBus 2.0 */

#define I2C_FUNC_SMBUS_BYTE I2C_FUNC_SMBUS_READ_BYTE | \
                            I2C_FUNC_SMBUS_WRITE_BYTE
#define I2C_FUNC_SMBUS_BYTE_DATA I2C_FUNC_SMBUS_READ_BYTE_DATA | \
                                 I2C_FUNC_SMBUS_WRITE_BYTE_DATA
#define I2C_FUNC_SMBUS_WORD_DATA I2C_FUNC_SMBUS_READ_WORD_DATA | \
                                 I2C_FUNC_SMBUS_WRITE_WORD_DATA
#define I2C_FUNC_SMBUS_BLOCK_DATA I2C_FUNC_SMBUS_READ_BLOCK_DATA | \
                                  I2C_FUNC_SMBUS_WRITE_BLOCK_DATA
#define I2C_FUNC_SMBUS_I2C_BLOCK I2C_FUNC_SMBUS_READ_I2C_BLOCK | \
                                  I2C_FUNC_SMBUS_WRITE_I2C_BLOCK

#define I2C_FUNC_SMBUS_EMUL I2C_FUNC_SMBUS_QUICK | \
                            I2C_FUNC_SMBUS_BYTE | \
                            I2C_FUNC_SMBUS_BYTE_DATA | \
                            I2C_FUNC_SMBUS_WORD_DATA | \
                            I2C_FUNC_SMBUS_PROC_CALL | \
                            I2C_FUNC_SMBUS_WRITE_BLOCK_DATA | \
                            I2C_FUNC_SMBUS_WRITE_BLOCK_DATA_PEC | \
                            I2C_FUNC_SMBUS_I2C_BLOCK

static const uint32_t prvI2cSupports = I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
static uint32_t prvI2cDelay = 10;
static uint32_t prvI2cStatus = 0;

static void prvI2cRsp(const uint8_t *pucData, uint32_t ulLen) {
    struct PoolEntry xEntryPut;
    xEntryPut.ucLen = ulLen;
    memcpy(xEntryPut.aucData, pucData, xEntryPut.ucLen);
    ulPoolsPut(I2C_RSP_POOL, &xEntryPut);
}

#define I2C_M_RD		    0x0001	/* read data, from slave to master, I2C_M_RD is guaranteed to be 0x0001! */
#define I2C_M_TEN		    0x0010	/* this is a ten bit chip address */
#define I2C_M_RECV_LEN		0x0400	/* length will be first received byte */
#define I2C_M_NO_RD_ACK		0x0800	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK	0x1000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR	0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART		0x4000	/* if I2C_FUNC_NOSTART */
#define I2C_M_STOP		    0x8000	/* if I2C_FUNC_PROTOCOL_MANGLING */
struct i2c_msg {
    uint16_t addr; /* slave address			*/
    uint16_t flags;
    uint16_t len; /* msg length				*/
    uint8_t buf[0] __attribute__((aligned(4))); /* pointer to msg data			*/
};

struct i2c_cmd {
    uint8_t ucCmd;
    union {
        uint32_t ulCfg;
        struct i2c_msg xI2cMsg;
    };
} __attribute__((packed));

static void prvI2cIO(struct i2c_msg *p) {
    if (LL_I2C_IsEnabledAutoEndMode(I2C1)) {
        LL_mDelay(1);   // Optional. The delay time depends on the slave device.
        while(!LL_I2C_IsActiveFlag_STOP(I2C1));
        LL_I2C_ClearFlag_STOP(I2C1);
    }

    LL_I2C_SetSlaveAddr(I2C1, p->addr << 1);
    LL_I2C_SetMasterAddressingMode(I2C1, p->flags & I2C_M_TEN ? LL_I2C_ADDRSLAVE_10BIT : LL_I2C_ADDRSLAVE_7BIT);
    LL_I2C_SetTransferRequest(I2C1, p->flags & I2C_M_RD ? LL_I2C_REQUEST_READ : LL_I2C_REQUEST_WRITE);
    LL_I2C_SetTransferSize(I2C1, p->len);
    LL_I2C_GenerateStartCondition(I2C1);
    LL_I2C_EnableAutoEndMode(I2C1);

    if (LL_I2C_GetTransferRequest(I2C1) == LL_I2C_REQUEST_WRITE) {
        for (uint32_t i = 0; i < p->len; i++) {
            LL_I2C_TransmitData8(I2C1, p->buf[i]);
            while (!LL_I2C_IsActiveFlag_TXE(I2C1)) ;
        }
    } else {
#define BUF_LEN (64)
        uint8_t buf[BUF_LEN];
        for (uint32_t j = 0; j < p->len / BUF_LEN; j++) {
            for (uint32_t i = 0; i < BUF_LEN; i++) {
                while (!LL_I2C_IsActiveFlag_RXNE(I2C1)) ;
                buf[i] = LL_I2C_ReceiveData8(I2C1);
            }
            prvI2cRsp((uint8_t *)&buf, BUF_LEN);
        }
        for (uint32_t i = 0; i < p->len % BUF_LEN; i++) {
            while (!LL_I2C_IsActiveFlag_RXNE(I2C1)) ;
            buf[i] = LL_I2C_ReceiveData8(I2C1);
        }
        prvI2cRsp((uint8_t *)&buf, p->len % BUF_LEN);
    }
}

void vI2cReqCb(void *pvArg, uint32_t ulSize) {
    struct i2c_cmd *p = (struct i2c_cmd *)pvArg;
    switch (p->ucCmd) {
    case CMD_ECHO:
        prvI2cRsp(&p->ucCmd, 1);
        break;
    case CMD_SET_DELAY:
        memcpy(&prvI2cDelay, &p->ulCfg, sizeof(prvI2cDelay));
        prvI2cRsp(&p->ucCmd, 1);
        break;
    case CMD_GET_FUNC:
        prvI2cRsp((uint8_t *)&prvI2cSupports, sizeof(prvI2cSupports));
        break;
    case CMD_GET_STATUS:
        prvI2cRsp((uint8_t *)&prvI2cStatus, sizeof(prvI2cStatus));
        break;
    case CMD_I2C_IO:
    case CMD_I2C_IO | CMD_I2C_IO_BEGIN:
    case CMD_I2C_IO | CMD_I2C_IO_END:
    case CMD_I2C_IO | CMD_I2C_IO_BEGIN | CMD_I2C_IO_END:
        prvI2cIO(&p->xI2cMsg);
        break;
    default:
        break;
    }
}