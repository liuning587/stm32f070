/**
  ******************************************************************************
  * @file    usbd_template_core.h
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   Header file for the usbd_template_core.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/ 
#ifndef __USB_TEMPLATE_CORE_H
#define __USB_TEMPLATE_CORE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_TEMPLATE
  * @brief This file is the header file for usbd_template_core.c
  * @{
  */ 


/** @defgroup USBD_TEMPLATE_Exported_Defines
  * @{
  */ 
#define TEMPLATE_EPIN_ADDR                 0x81
#define TEMPLATE_EPIN_SIZE                 0x10

#define USB_TEMPLATE_CONFIG_DESC_SIZ       64

/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */ 



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 
typedef struct USBD_CUSTOM_Itf
{
  int8_t (* Init)			(void);
  int8_t (* DeInit)			(void);
  int8_t (* Control)		(void *, uint32_t );
  int8_t (* Receive)		(uint8_t *, uint32_t *);  
  int8_t (* ReceiveI2cCmd)	(uint8_t *, uint32_t *);  

} USBD_CUSTOM_ItfTypeDef;
     
typedef struct {
	uint8_t sa;
	uint8_t rw;
	uint8_t txLen;
	uint8_t rxLen;
	uint8_t txBuf[64];
	uint8_t rxBuf[64];
} I2C_Adapter_HandleTypeDef;  

typedef struct {
	uint8_t  CmdData[64];
	uint8_t  CmdOpCode;
	uint8_t  CmdLength;    
	
	uint8_t  *RxBuffer;  
	uint8_t  *TxBuffer;   
	uint32_t RxLength;
	uint32_t TxLength;      
	__IO uint32_t TxState;     
	__IO uint32_t RxState;    
	
	uint8_t  *I2cRxBuffer;  
	uint8_t  *I2cTxBuffer;   
	uint32_t I2cRxLength;
	uint32_t I2cTxLength;   
	__IO uint32_t I2cTxState;     
	__IO uint32_t I2cRxState;  
} USBD_CUSTOM_HandleTypeDef; 


extern USBD_ClassTypeDef  USBD_TEMPLATE_ClassDriver;
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
#define CUSTOM_OUT_EP 0x01
#define CUSTOM_IN_EP 0x81
#define I2C_OUT_EP 0x02
#define I2C_IN_EP 0x82
#define USB_EP_COUNT 0x04
uint8_t  USBD_CUSTOM_RegisterInterface(USBD_HandleTypeDef   *pdev, USBD_CUSTOM_ItfTypeDef *fops);
uint8_t  USBD_CUSTOM_Transmit(USBD_HandleTypeDef *pdev, uint8_t epNum, const uint8_t *pData, uint32_t ulDataSize);
/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif  /* __USB_TEMPLATE_CORE_H */
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
