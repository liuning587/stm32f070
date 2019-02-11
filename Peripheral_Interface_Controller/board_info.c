#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_rcc.h"    
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_hal.h"
#include "board_info.h"
#include <string.h>
/* .info @ 0x100*/

#define MAX_INFO_STR_LEN 48
#define MAX_TYPE_NUMB  8
#define FW_VER 0
#define UNDEF_1 1
#define UNDEF_2 2
#define UNDEF_3 3
#define UNDEF_4 4
#define UNDEF_5 5
#define UNDEF_6 6
#define UNDEF_7 7

struct Info {
	const char xlist[MAX_TYPE_NUMB][MAX_INFO_STR_LEN];
};

/*
 * note: max of info string is 63 char
 **/
static struct Info prvInfo __attribute__((section(".info"), used)) = {
	.xlist[FW_VER]  = "FW v1.0",
	.xlist[UNDEF_1] = "UNDEF_1",
	.xlist[UNDEF_2] = "UNDEF_2",
	.xlist[UNDEF_3] = "UNDEF_3",
	.xlist[UNDEF_4] = "UNDEF_4",
	.xlist[UNDEF_5] = "UNDEF_5",
	.xlist[UNDEF_6] = "UNDEF_6",
	.xlist[UNDEF_7] = "UNDEF_7",
};

void vGetInfo(char *pcInfo, uint32_t ulMaxStrLen, uint32_t ulInfoType) {	
	const char *p = ulInfoType > MAX_TYPE_NUMB ? "Unkown Type" : prvInfo.xlist[ulInfoType];
	strcpy(pcInfo, strlen(p) > ulMaxStrLen ? "No enough mem." : p);
}