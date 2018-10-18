#ifndef _POOLS_H_
#define _POOLS_H_

#include "circ_buf.h"

struct Pool {
    CircBuf_t xCb;
};

struct PoolEntry {
	uint8_t ucLen;
	uint8_t aucData[63];
} __attribute__((packed));

/*
 * the API is designed for multiple pools
 * many producers put data into n pools
 * but only one consumer get data from the pools
 * 
 * ps:
 * the parameter axPools is an array of Pool_t
 * for ulPoolGet(), if not only one pool has data, the frist pool is selected (axPools[0] first, then [1], [2], ...)
 */
#define	EP1_REQ_POOL	0	/* requests */
#define	EP1_RSP_POOL	1	/* requests */
#define	EP1_NOTIFY_POOL	2	/* response */
#define	I2C_REQ_POOL	3	/* requests */
#define	I2C_RSP_POOL	4	/* requests */

void vPoolsInit(void);
uint32_t ulPoolsGet(uint32_t ulIdx, struct PoolEntry *pxEntry);
uint32_t ulPoolsPut(uint32_t ulIdx, struct PoolEntry *pxEntry);
void vPoolsFlush(uint32_t ulIdx);

#endif
