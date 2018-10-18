/**
  ******************************************************************************
  * @file    usbd_cdc_if_template.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   Generic media access Layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "usbd_template_if.h"
#include "pools.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_CUSTOM
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CUSTOM_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_Private_Defines
  * @{
  */
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_Private_FunctionPrototypes
  * @{
  */

static int8_t CUSTOM_Init(void);
static int8_t CUSTOM_DeInit(void);
static int8_t CUSTOM_Control(void* pvReq, uint32_t ulSize);
static int8_t CUSTOM_Receive(uint8_t* pbuf, uint32_t *Len);
static int8_t CUSTOM_ReceiveI2cCmd(uint8_t* pbuf, uint32_t *Len);

USBD_CUSTOM_ItfTypeDef USBD_CUSTOM_Template_fops =
{
    CUSTOM_Init,
    CUSTOM_DeInit,
    CUSTOM_Control,
    CUSTOM_Receive,
    CUSTOM_ReceiveI2cCmd,
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CUSTOM_Init
  *         Initializes the CUSTOM media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_Init(void) {
    /*
       Add your initialization code here
     */
    return (0);
}

/**
  * @brief  CUSTOM_DeInit
  *         DeInitializes the CUSTOM media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_DeInit(void) {
    /*
       Add your deinitialization code here
     */
    return (0);
}

/**
  * @brief  CUSTOM_Control
  *         Manage the CUSTOM class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */


static int8_t CUSTOM_Control(void* pvReq, uint32_t ulSize) {
    return USBD_OK;
}

/**
  * @brief  CUSTOM_Receive
  *         Data received over USB OUT endpoint are sent over CUSTOM interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint untill exiting this function. If you exit this function
  *         before transfer is complete on CUSTOM interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */

static int8_t CUSTOM_Receive(uint8_t* Buf, uint32_t *Len) {
    struct PoolEntry xPoolEntry;
    xPoolEntry.ucLen = *Len;
    memcpy((void *)xPoolEntry.aucData, Buf, *Len);
    ulPoolsPut(EP1_REQ_POOL, (struct PoolEntry*)&xPoolEntry);
    return USBD_OK;
}

/**
  * @}
  */
static int8_t CUSTOM_ReceiveI2cCmd(uint8_t* Buf, uint32_t *Len) {
	struct PoolEntry xPoolEntry;
	xPoolEntry.ucLen = *Len;
	memcpy((void *)xPoolEntry.aucData, Buf, *Len);
	ulPoolsPut(I2C_REQ_POOL, (struct PoolEntry*)&xPoolEntry);
	return USBD_OK;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/