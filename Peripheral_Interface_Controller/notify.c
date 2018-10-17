#include "stm32f0xx_ll_utils.h"
#include "pools.h"
#include "notify.h"
#include <usbd_core.h>
#include <usbd_template.h>
#include <usbd_desc.h>
#include <usbd_template.h>
#include <usbd_template_if.h>

extern USBD_HandleTypeDef USBD_Device;
void vNotify(void *pArg, uint32_t ulLen) {
    USBD_CUSTOM_Transmit(&USBD_Device, CUSTOM_IN_EP, pArg, ulLen); 
}
