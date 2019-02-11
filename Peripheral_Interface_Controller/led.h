#ifndef __LED_H__
#define __LED_H__

void vLedInit(void); 
void vLedTurnOn(uint32_t ulOnTime, uint32_t OffTime, uint32_t ulDuration);
void vLedBrighten(void);
void vLedDarken(void);

#endif // !__LED_H__
