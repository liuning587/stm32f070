#include "stm32f0xx.h"
#include "pools.h"
#include <string.h>
/*********************************************************************
*
*	   Private data/functions
*/

/////////////////////////////////////////////////////////////////////////
// select a pool with data 
//  (if multiple pools are with data, select the first one)
static int8_t prvPoolsGetSelect(struct Pool axPools[]) {
    for(int8_t cIdx=0; axPools[cIdx].xCb.ulSize != 0; cIdx++) {
		if(ulCircBufCount(&(axPools[cIdx].xCb))) return cIdx;
	}
	return -1;
}

static uint8_t aucEp1ReqPoolBuf[128];
static uint8_t aucEp1RspPoolBuf[128];
static uint8_t aucEp1NotifyPoolBuf[128];
static uint8_t aucCtrlPoolBuf[128];
static uint8_t aucEp2ReqPoolBuf[128];
static uint8_t aucEp2RspPoolBuf[128];
static struct Pool prvPool[5] = { 
    [EP1_REQ_POOL] = { aucEp1ReqPoolBuf, sizeof(aucEp1ReqPoolBuf), 0, 0 },
    [EP1_RSP_POOL] = { aucEp1RspPoolBuf, sizeof(aucEp1RspPoolBuf), 0, 0 },
    [EP1_NOTIFY_POOL] = { aucEp1NotifyPoolBuf, sizeof(aucEp1NotifyPoolBuf), 0, 0 },
    [I2C_REQ_POOL] = { aucEp2ReqPoolBuf, sizeof(aucEp2ReqPoolBuf), 0, 0 },
    [I2C_RSP_POOL] = { aucEp2RspPoolBuf, sizeof(aucEp2RspPoolBuf), 0, 0 },
};

/*********************************************************************
*
*       API
*
* (for n producers, 1 consumer)
* (all pools share the same get-semaphore)
*/
void vPoolsInit(void) {
    // TODO... nothing
}

uint32_t ulPoolsGet(uint32_t ulIdx, struct PoolEntry *pxEntry) {
    __disable_irq();
    uint32_t ret = 0;
    if (ulCircBufCount(&prvPool[ulIdx].xCb) > 0) {
        ret = pxEntry->ucLen = ucCircBufReadByte(&prvPool[ulIdx].xCb);
        if (pxEntry->ucLen != 0) {
            vCircBufRead(&prvPool[ulIdx].xCb, pxEntry->aucData, pxEntry->ucLen);
        }
    }
    __enable_irq();
    return ret;
}

uint32_t ulPoolsPut(uint32_t ulIdx, struct PoolEntry *pxEntry) {
    __disable_irq();
    uint32_t ret = 0;
    if (ulCircBufSpace(&prvPool[ulIdx].xCb) > (pxEntry->ucLen)) {		
        vCircBufWrite(&prvPool[ulIdx].xCb, (uint8_t *)pxEntry, pxEntry->ucLen + 1);
        ret = pxEntry->ucLen + 1;
    }
    __enable_irq();
    return ret;
}

void vPoolsFlush(uint32_t ulIdx) {
    __disable_irq();
	vCircBufFlush(&prvPool[ulIdx].xCb);
    __enable_irq();
}

