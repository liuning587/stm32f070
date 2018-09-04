#ifndef __CIRC_BUF_H__
#define __CIRC_BUF_H__

#include <stdint.h>

typedef struct {
    uint8_t * const pucBuf;
	const uint32_t ulSize;
	volatile uint32_t ulHead;
	volatile uint32_t ulTail;
} CircBuf_t;

uint32_t ulCircBufCount(const CircBuf_t *pxCb);
uint32_t ulCircBufSpace(const CircBuf_t *pxCb);
void vCircBufWriteByte(CircBuf_t *pxCb, uint8_t ucElem);
uint8_t ucCircBufReadByte(CircBuf_t *pxCb);

uint32_t ulCircBufCountToEnd(const CircBuf_t *pxCb);
uint32_t ulCircBufSpaceToEnd(const CircBuf_t *pxCb);
void vCircBufWrite(CircBuf_t *pxCb, const uint8_t *pucBuf, uint32_t ulLen);
void vCircBufRead(CircBuf_t *pxCb, uint8_t *pucBuf, uint32_t ulLen);
void vCircBufFlush(CircBuf_t *pxCb);

#endif
