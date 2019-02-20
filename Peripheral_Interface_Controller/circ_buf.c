#include "circ_buf.h"
#include <string.h>

uint32_t ulCircBufCount(const CircBuf_t *pxCb) {
	return ((pxCb->ulHead - pxCb->ulTail) & (2*pxCb->ulSize-1));
}

uint32_t ulCircBufSpace(const CircBuf_t *pxCb) {
	return pxCb->ulSize - ulCircBufCount(pxCb);
}

void vCircBufWriteByte(CircBuf_t *pxCb, uint8_t ucElem) {
	pxCb->pucBuf[pxCb->ulHead & (pxCb->ulSize-1)] = ucElem;
	pxCb->ulHead++;
	pxCb->ulHead &= (2*pxCb->ulSize-1);
}

uint8_t ucCircBufReadByte(CircBuf_t *pxCb) {
	uint8_t ucElem;
	ucElem = pxCb->pucBuf[pxCb->ulTail & (pxCb->ulSize-1)];
	pxCb->ulTail++;
	pxCb->ulTail &= (2*pxCb->ulSize-1);
	return ucElem;
}

uint32_t ulCircBufCountToEnd(const CircBuf_t *pxCb) {
	uint32_t ulTailtoEnd = pxCb->ulSize - pxCb->ulTail&(pxCb->ulSize-1);
	uint32_t ulTailtoHead = ulCircBufCount(pxCb);
	return (ulTailtoHead<ulTailtoEnd ? ulTailtoHead : ulTailtoEnd);
}

uint32_t ulCircBufSpaceToEnd(const CircBuf_t *pxCb) {
	uint32_t ulHeadtoEnd = pxCb->ulSize - pxCb->ulHead&(pxCb->ulSize-1);
	uint32_t ulHeadtoTail = ulCircBufSpace(pxCb);
	return (ulHeadtoTail<ulHeadtoEnd ? ulHeadtoTail : ulHeadtoEnd);
}

void vCircBufWrite(CircBuf_t *pxCb, const uint8_t *pucBuf, uint32_t ulLen) {
	uint32_t ulSpaceToEnd = ulCircBufSpaceToEnd(pxCb);
	if(ulLen > ulSpaceToEnd) {
		memcpy(&pxCb->pucBuf[pxCb->ulHead & (pxCb->ulSize-1)], pucBuf, ulSpaceToEnd);
		memcpy(&pxCb->pucBuf[0], pucBuf+ulSpaceToEnd, ulLen-ulSpaceToEnd);
	} else {
		memcpy(&pxCb->pucBuf[pxCb->ulHead & (pxCb->ulSize-1)], pucBuf, ulLen);
	}
	pxCb->ulHead += ulLen;
	pxCb->ulHead &= (2*pxCb->ulSize-1);
}

void vCircBufRead(CircBuf_t *pxCb, uint8_t *pucBuf, uint32_t ulLen) {
	uint32_t ulCountToEnd = ulCircBufCountToEnd(pxCb);
	if(ulLen > ulCountToEnd) {
		memcpy(pucBuf, &pxCb->pucBuf[pxCb->ulTail & (pxCb->ulSize-1)], ulCountToEnd);
		memcpy(pucBuf+ulCountToEnd, &pxCb->pucBuf[0], ulLen-ulCountToEnd);
	} else {
		memcpy(pucBuf, &pxCb->pucBuf[pxCb->ulTail & (pxCb->ulSize-1)], ulLen);
	}
	pxCb->ulTail += ulLen;
	pxCb->ulTail &= (2*pxCb->ulSize-1);
}

void vCircBufFlush(CircBuf_t *pxCb) {
    pxCb->ulTail = pxCb->ulHead = 0;
}
