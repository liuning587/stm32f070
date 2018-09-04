#include "stm32f0xx_ll_utils.h"
#include "pools.h"
#include "request.h"
#include <usbd_core.h>
#include <usbd_template.h>
#include <usbd_desc.h>
#include <usbd_template.h>
#include <usbd_template_if.h>

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

    prvRsp(__FUNCTION__);   
}

void vSetEpdPwr(void *pArg, uint32_t ulLen) {
    prvRsp(__FUNCTION__);    
}

void vGetTime(void *pArg, uint32_t ulLen) {
    prvRsp(__FUNCTION__);   
}

void vSetTime(void *pArg, uint32_t ulLen) {
    prvRsp(__FUNCTION__);   
}

void vGetAlarm(void *pArg, uint32_t ulLen) {
    prvRsp(__FUNCTION__);   
}

void vSetAlarm(void *pArg, uint32_t ulLen) {
    prvRsp(__FUNCTION__);     
}

void vSetLed(void *pArg, uint32_t ulLen) {
    prvRsp(__FUNCTION__);     
}

void vDummy(void *pArg, uint32_t ulLen) {
    prvRsp(__FUNCTION__);   
}

pfRequestCb_t xRequestCb[] = { 
        vEcho,
        vGetBootReason,
        vSetPwr,
        vSetEpdPwr,
        vGetTime,
        vSetTime,
        vGetAlarm,
        vSetAlarm,
        vSetLed,
};

pfRequestCb_t xGetReqCb(uint32_t ulReqId) {
    return ulReqId <0x41 || ulReqId > 0x48 ? vDummy : xRequestCb[ulReqId%0x41];    
}
