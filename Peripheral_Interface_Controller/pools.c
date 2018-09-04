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

static uint8_t aucReqPoolBuf[128];
static uint8_t aucRspPoolBuf[128];
static uint8_t aucNotifyPoolBuf[128];
static struct Pool prvPool[3] = { 
    [REQ_POOL] = { aucReqPoolBuf, sizeof(aucReqPoolBuf), 0, 0 },
    [RSP_POOL] = { aucRspPoolBuf, sizeof(aucRspPoolBuf), 0, 0 },
    [NOTIFY_POOL] = { aucNotifyPoolBuf, sizeof(aucNotifyPoolBuf), 0, 0 },
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

