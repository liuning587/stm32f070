/**
  ******************************************************************************
  * @file    usbd_template.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                TEMPLATE Class  Description
  *          ===================================================================
  *          
  *
  *
  *
  *           
  *      
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
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
#include "usbd_template.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "stm32f0xx_ll_utils.h"
#define COUNTOF(x)  (sizeof(x)/sizeof(x[0]))
#define CUSTOM_DATA_EP_IN_PACKET_SIZE   64
#define CUSTOM_DATA_EP_OUT_PACKET_SIZE  64
/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_TEMPLATE 
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


static uint8_t  USBD_CUSTOM_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_CUSTOM_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_CUSTOM_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  *USBD_CUSTOM_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_CUSTOM_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_CUSTOM_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_CUSTOM_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_CUSTOM_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_CUSTOM_EP0_TxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_CUSTOM_SOF (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_CUSTOM_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_CUSTOM_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

/**
  * @}
  */ 

/** @defgroup USBD_CUSTOM_Private_Variables
  * @{
  */ 

USBD_ClassTypeDef  USBD_CUSTOM_ClassDriver = 
{
  USBD_CUSTOM_Init,
  USBD_CUSTOM_DeInit,
  USBD_CUSTOM_Setup,
  USBD_CUSTOM_EP0_TxReady,  
  USBD_CUSTOM_EP0_RxReady,
  USBD_CUSTOM_DataIn,
  USBD_CUSTOM_DataOut,
  USBD_CUSTOM_SOF,
  USBD_CUSTOM_IsoINIncomplete,
  USBD_CUSTOM_IsoOutIncomplete,      
  USBD_CUSTOM_GetCfgDesc,
  USBD_CUSTOM_GetCfgDesc, 
  USBD_CUSTOM_GetCfgDesc,
  USBD_CUSTOM_GetDeviceQualifierDesc,
};

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4   
#endif

struct usbdEndpointDesc {
    /*Endpoint Descriptor*/
    uint8_t bLength; /* bLength: Endpoint Descriptor size */
    uint8_t bDescriptorType; /* bDescriptorType: Endpoint */
    uint8_t bEndpointAddress; /* bEndpointAddress */
    uint8_t bmAttributes; /* bmAttributes: Interrupt */
    uint16_t wMaxPacketSize; /* wMaxPacketSize: */
    uint8_t bInterval; /* bInterval: */ 
}__attribute__((packed));

struct usbdCustomIfDesc {
    /*Interface Descriptor*/
    uint8_t bLength; /* bLength: Interface Descriptor size */
    uint8_t bDescriptorType; /* bDescriptorType: Interface */
    /* Interface descriptor type */
    uint8_t bInterfaceNumber; /* bInterfaceNumber: Number of Interface */
    uint8_t bAlternateSetting; /* bAlternateSetting: Alternate setting */
    uint8_t bNumEndpoints; /* bNumEndpoints: One endpoints used */
    uint8_t bInterfaceClass; /* bInterfaceClass: Communication Interface Class */
    uint8_t bInterfaceSubClass; /* bInterfaceSubClass: Abstract Control Model */
    uint8_t bInterfaceProtocol; /* bInterfaceProtocol: Common AT commands */
    uint8_t iInterface; /* iInterface: */
} __attribute__((packed));

struct usbdCustomCfgDesc {
    /*Configuration Descriptors*/
    uint8_t bLength; /* bLength: Configuation Descriptor size */
    uint8_t bDescriptorType; /* bDescriptorType: Configuration */
    uint16_t wTotalLength; /* wTotalLength: Bytes returned */
    uint8_t bNumInterfaces;/*bNumInterfaces: 1 interface*/
    uint8_t bConfigurationValue;/*bConfigurationValue: Configuration value*/
    uint8_t iConfiguration;/*iConfiguration: Index of string descriptor describing the configuration*/
    uint8_t bmAttributes;/*bmAttributes: D7 Reserved, set to 1. (USB 1.0 Bus Powered), D6 Self Powered, D5 Remote Wakeup, D4..0 Reserved, set to 0.*/
    uint8_t bMaxPower;/*MaxPower 100 mA: this current is used for detecting Vbus*/ 
    struct usbdCustomIfDesc xUsbdCustomIfDesc;
    struct usbdEndpointDesc xUsbdEp[2];
} __attribute__((packed));

#define CUSTOM_OUT_EP 0x01  
#define CUSTOM_IN_EP 0x81  
struct usbdCustomCfgDesc xUsbdCustomCfgDesc = {
    /*Configuration Descriptors*/
    .bLength = USB_LEN_CFG_DESC,
    .bDescriptorType = USB_DESC_TYPE_CONFIGURATION,
    .wTotalLength = sizeof(struct usbdCustomCfgDesc),
    .bNumInterfaces = 0x01,
    .bConfigurationValue = 0x01,
    .iConfiguration = 0x00,
    .bmAttributes =  0xC0, /* Self Powered */  
    .bMaxPower = 0x32, /* 100mA = 0x32 2mA */ 
    /*Interface Descriptor*/
    .xUsbdCustomIfDesc = { 
        .bLength = USB_LEN_IF_DESC, /* bLength: Interface Descriptor size */
        .bDescriptorType = USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
        .bInterfaceNumber = 0x00, /* bInterfaceNumber: Number of Interface */
        .bAlternateSetting = 0x00, /* bAlternateSetting: Alternate setting */
        .bNumEndpoints = 0x02, /* bNumEndpoints: One endpoints used */
        .bInterfaceClass = 0x00, /* bInterfaceClass: Communication Interface Class */
        .bInterfaceSubClass = 0x00, /* bInterfaceSubClass: Abstract Control Model */
        .bInterfaceProtocol = 0x00, /* bInterfaceProtocol: Common AT commands */
        .iInterface = 0x00, /* iInterface: */
    },
    .xUsbdEp = { 
        /*Endpoint OUT Descriptor*/
        {
            .bLength = USB_LEN_EP_DESC, /* bLength: Endpoint Descriptor size */
            .bDescriptorType = USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: Endpoint */
            .bEndpointAddress = CUSTOM_OUT_EP, /* bEndpointAddress */
            .bmAttributes = USBD_EP_TYPE_BULK, /* bmAttributes: Interrupt */
            .wMaxPacketSize = CUSTOM_DATA_EP_OUT_PACKET_SIZE, /* wMaxPacketSize: */
            .bInterval = 0x00, /* bInterval: */ 
        },
        /*Endpoint IN Descriptor*/
        {
            .bLength = USB_LEN_EP_DESC, /* bLength: Endpoint Descriptor size */
            .bDescriptorType = USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: Endpoint */
            .bEndpointAddress = CUSTOM_IN_EP, /* bEndpointAddress */
            .bmAttributes = USBD_EP_TYPE_BULK, /* bmAttributes: Interrupt */
            .wMaxPacketSize = CUSTOM_DATA_EP_IN_PACKET_SIZE, /* wMaxPacketSize: */
            .bInterval = 0x00, /* bInterval: */ 
        }
    },
    
};


  
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4   
#endif
/* USB Standard Device Descriptor */
static uint8_t USBD_CUSTOM_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */ 
uint8_t prvDataOut[CUSTOM_DATA_EP_OUT_PACKET_SIZE];
uint8_t prvDataIn[CUSTOM_DATA_EP_IN_PACKET_SIZE];
/** @defgroup USBD_CUSTOM_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_CUSTOM_Init
  *         Initialize the TEMPLATE interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CUSTOM_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
    /* Open EP IN */
    USBD_LL_OpenEP(pdev, CUSTOM_IN_EP, USBD_EP_TYPE_BULK, CUSTOM_DATA_EP_IN_PACKET_SIZE);
    /* Open EP OUT */
    USBD_LL_OpenEP(pdev, CUSTOM_OUT_EP, USBD_EP_TYPE_BULK, CUSTOM_DATA_EP_OUT_PACKET_SIZE);
    pdev->pClassData = USBD_malloc(sizeof(USBD_CUSTOM_HandleTypeDef));
    if (pdev->pClassData == NULL) {
        return USBD_FAIL;
    } 
   
    USBD_CUSTOM_HandleTypeDef   *hCustom = (USBD_CUSTOM_HandleTypeDef*) pdev->pClassData;
    /* Init  physical Interface components */
    ((USBD_CUSTOM_ItfTypeDef *)pdev->pUserData)->Init();
    /* Init Xfer states */
    hCustom->TxState = 0;
    hCustom->TxBuffer = prvDataIn;
    hCustom->RxState = 0;
    hCustom->RxBuffer = prvDataOut;
    /* Prepare Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev, CUSTOM_OUT_EP, hCustom->RxBuffer, CUSTOM_DATA_EP_OUT_PACKET_SIZE);
        
    return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_Init
  *         DeInitialize the TEMPLATE layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CUSTOM_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
    /* Open EP IN */
    USBD_LL_CloseEP(pdev, CUSTOM_IN_EP);  
    /* Open EP OUT */
    USBD_LL_CloseEP(pdev, CUSTOM_OUT_EP);  
    /* DeInit  physical Interface components */
    if (pdev->pClassData != NULL) {
        ((USBD_CUSTOM_ItfTypeDef *)pdev->pUserData)->DeInit();
        USBD_free(pdev->pClassData);
        pdev->pClassData = NULL;
    }  
    return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_Setup
  *         Handle the TEMPLATE specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_CUSTOM_Setup(USBD_HandleTypeDef *pdev, 
    USBD_SetupReqTypedef *req) {
    __BKPT(255);
    return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_GetCfgDesc 
  *         return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CUSTOM_GetCfgDesc(uint16_t *length) {    
    *length = xUsbdCustomCfgDesc.wTotalLength;
    return (uint8_t  *)&xUsbdCustomCfgDesc;
}

/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_CUSTOM_DeviceQualifierDescriptor (uint16_t *length) {
  *length = sizeof (USBD_CUSTOM_DeviceQualifierDesc);
  return USBD_CUSTOM_DeviceQualifierDesc;
}

/**
  * @brief  USBD_CUSTOM_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_CUSTOM_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
    USBD_CUSTOM_HandleTypeDef   *hCustom = (USBD_CUSTOM_HandleTypeDef*) pdev->pClassData;  
    if (pdev->pClassData != NULL) {    
        hCustom->TxState = 0;
        return USBD_OK;
    } else {
        return USBD_FAIL;
    }
    return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_CUSTOM_EP0_RxReady(USBD_HandleTypeDef *pdev) {
    USBD_CUSTOM_HandleTypeDef   *hCustom = (USBD_CUSTOM_HandleTypeDef*) pdev->pClassData;
  
    if ((pdev->pUserData != NULL) && (hCustom->CmdOpCode != 0xFF)) {
        ((USBD_CUSTOM_ItfTypeDef *)pdev->pUserData)->Control(hCustom->CmdOpCode,
                                                          (uint8_t *)hCustom->data,
                                                          hCustom->CmdLength);
        hCustom->CmdOpCode = 0xFF; 
    }
    return USBD_OK;
}
/**
  * @brief  USBD_CUSTOM_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_CUSTOM_EP0_TxReady(USBD_HandleTypeDef *pdev) {
    return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_CUSTOM_SOF(USBD_HandleTypeDef *pdev)
{
    __NOP();
    return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_CUSTOM_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    __BKPT(255);
    return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_CUSTOM_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum) {

  return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_CUSTOM_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
    USBD_CUSTOM_HandleTypeDef   *hCustom = (USBD_CUSTOM_HandleTypeDef*) pdev->pClassData;  
    /* Get the received data length */
    hCustom->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);
    /* USB data will be immediately processed, this allow next USB traffic being 
    NAKed till the end of the application Xfer */
    if (pdev->pClassData != NULL)  {
        ((USBD_CUSTOM_ItfTypeDef *)pdev->pUserData)->Receive(hCustom->RxBuffer, &hCustom->RxLength);
        /* Prepare Out endpoint to receive next packet */
        hCustom->RxState = 0;
        USBD_LL_PrepareReceive(pdev, CUSTOM_OUT_EP, hCustom->RxBuffer, CUSTOM_DATA_EP_OUT_PACKET_SIZE);
        return USBD_OK;
    }
    else {
        return USBD_FAIL;
    }
}
/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_CUSTOM_GetDeviceQualifierDesc(uint16_t *length) {
    *length = sizeof(USBD_CUSTOM_DeviceQualifierDesc);
    return USBD_CUSTOM_DeviceQualifierDesc;
}

/**
* @brief  USBD_CUSTOM_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t  USBD_CUSTOM_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CUSTOM_ItfTypeDef *fops) {
    uint8_t  ret = USBD_FAIL;
  
    if (fops != NULL)
    {
        pdev->pUserData = fops;
        ret = USBD_OK;    
    } 
    return ret;
}
/**
  * @}
  */ 
uint8_t  USBD_CUSTOM_Transmit(USBD_HandleTypeDef *pdev, const uint8_t *pData, uint32_t ulDataSize) {
    USBD_CUSTOM_HandleTypeDef   *hCustom = (USBD_CUSTOM_HandleTypeDef*) pdev->pClassData;  
    uint32_t quotient = ulDataSize / CUSTOM_DATA_EP_IN_PACKET_SIZE;
    uint32_t remainder = ulDataSize % CUSTOM_DATA_EP_IN_PACKET_SIZE;  
    uint32_t timeout = 0xFFFF;
    const uint8_t *p = pData;
    
    if (pdev == NULL || hCustom == NULL) {
        return USBD_FAIL;
    }
    
    for (int i = 0; i < quotient; i++) {
        timeout = 0xFFFF;
        while (hCustom->TxState) {
            if (--timeout == 0) {
                __BKPT(255);
                return USBD_FAIL;
            }
        }
        hCustom->TxState  = 1;
        memcpy(hCustom->TxBuffer, p, CUSTOM_DATA_EP_IN_PACKET_SIZE);
        USBD_LL_Transmit(pdev, CUSTOM_IN_EP, hCustom->TxBuffer, CUSTOM_DATA_EP_IN_PACKET_SIZE);
        p += CUSTOM_DATA_EP_IN_PACKET_SIZE;
    }
    
    if (remainder > 0) {
        timeout = 0xFFFF;
        while (hCustom->TxState) {
            if (--timeout == 0) {
                __BKPT(255);
                return USBD_FAIL;
            }
        }
        hCustom->TxState  = 1;
        memcpy(hCustom->TxBuffer, p, remainder);
        USBD_LL_Transmit(pdev, CUSTOM_IN_EP, hCustom->TxBuffer, remainder);
    }
    return USBD_OK;
}

/**
  * @}
  */ 


/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
