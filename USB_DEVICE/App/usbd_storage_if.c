/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_storage_if.c
  * @version        : v2.0_Cube
  * @brief          : Memory management layer.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_storage_if.h"

/* USER CODE BEGIN INCLUDE */
#include "DiskImage.h"
#include <string.h>
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @defgroup USBD_STORAGE
  * @brief Usb mass storage device module
  * @{
  */

/** @defgroup USBD_STORAGE_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */
typedef union{
	uint8_t		u8[4];
	int8_t		i8[4];
	uint16_t	u16[2];
	int16_t		i16[2];
	uint32_t	u32;
	int32_t		i32;
	float		f;
}_uWork;
/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Defines
  * @brief Private defines.
  * @{
  */

#define STORAGE_LUN_NBR                  1
#define STORAGE_BLK_NBR                  96
#define STORAGE_BLK_SIZ                  0x200

/* USER CODE BEGIN PRIVATE_DEFINES */
#define MESSAGGE_START 		(0xA00+19)


/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN INQUIRY_DATA_FS */
/** USB Mass storage Standard Inquiry Data. */
const int8_t STORAGE_Inquirydata_FS[] = {/* 36 */

  /* LUN 0 */
  0x00,
  0x80,
  0x02,
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,
  0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product      : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0' ,'1'                      /* Version      : 4 Bytes */
};
/* USER CODE END INQUIRY_DATA_FS */

/* USER CODE BEGIN PRIVATE_VARIABLES */
//uint8_t MSCbuffer[STORAGE_BLK_NBR*STORAGE_BLK_SIZ];
uint8_t MSCbuffer[16*STORAGE_BLK_SIZ];
uint8_t bufPage[1024], statePage;
uint8_t aBinFile;
uint32_t binFileSize;
uint32_t binFileStart;
uint32_t programAddressStart;
char strAux[64];

_uWork w;

FLASH_EraseInitTypeDef EraseInitStruct;

const char msgStart[] = "Nothing to say                  ";
const char msgERROR[] = "Firmware UPDATE ERROR           ";
const char msgOk[]    = "Firmware UPDATE OK              ";
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */
static const uint16_t BUFMASKTX = 2047;
extern uint8_t txBuf[2048];
extern uint16_t iw;
/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t STORAGE_Init_FS(uint8_t lun);
static int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
static int8_t STORAGE_IsReady_FS(uint8_t lun);
static int8_t STORAGE_IsWriteProtected_FS(uint8_t lun);
static int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun_FS(void);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
//static void BootLoader(void);
static uint8_t FlashPage(uint32_t pageAddress);
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_StorageTypeDef USBD_Storage_Interface_fops_FS =
{
  STORAGE_Init_FS,
  STORAGE_GetCapacity_FS,
  STORAGE_IsReady_FS,
  STORAGE_IsWriteProtected_FS,
  STORAGE_Read_FS,
  STORAGE_Write_FS,
  STORAGE_GetMaxLun_FS,
  (int8_t *)STORAGE_Inquirydata_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes over USB FS IP
  * @param  lun:
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Init_FS(uint8_t lun)
{
  /* USER CODE BEGIN 2 */
	memset(MSCbuffer, '\0', 8192);

	memcpy(MSCbuffer, DiskImage0, 512);
	memcpy(&MSCbuffer[1*512], DiskImage1, sizeof(DiskImage1));
	memcpy(&MSCbuffer[3*512], DiskImage3, sizeof(DiskImage3));
	memcpy(&MSCbuffer[4*512], DiskImage4, sizeof(DiskImage4));
	memcpy(&MSCbuffer[5*512], DiskImage5, sizeof(DiskImage5));

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.NbPages = 1;
	EraseInitStruct.PageAddress = programAddressStart;

	aBinFile = 0;

  return (USBD_OK);
  /* USER CODE END 2 */
}

/**
  * @brief  .
  * @param  lun: .
  * @param  block_num: .
  * @param  block_size: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  /* USER CODE BEGIN 3 */
	*block_num  = STORAGE_BLK_NBR;
	*block_size = STORAGE_BLK_SIZ;
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsReady_FS(uint8_t lun)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  /* USER CODE BEGIN 6 */
	if(blk_addr < 16)
		memcpy(buf, &MSCbuffer[blk_addr*STORAGE_BLK_SIZ], blk_len*STORAGE_BLK_SIZ);
	else
		memset(buf, 0, blk_len*STORAGE_BLK_SIZ);


  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  /* USER CODE BEGIN 7 */
	if(blk_addr < 16)
		memcpy(&MSCbuffer[blk_addr*STORAGE_BLK_SIZ], buf, blk_len*STORAGE_BLK_SIZ);

	if(blk_addr == 3){
		uint16_t l = blk_addr*STORAGE_BLK_SIZ;
		uint16_t k;

		for(int i=0; i<16; i++){
			k = l+i*32;
			if((MSCbuffer[k+0]!=0xE5 && MSCbuffer[k+0]!=0x00) && MSCbuffer[k+8]=='B' && MSCbuffer[k+9]=='I' && MSCbuffer[k+10]=='N'){

				w.u8[0] = MSCbuffer[k+26];
				w.u8[1] = MSCbuffer[k+27];
				w.u8[2] = 0;
				w.u8[3] = 0;

				w.u8[0] = MSCbuffer[k+28];
				w.u8[1] = MSCbuffer[k+29];
				w.u8[2] = MSCbuffer[k+30];
				w.u8[3] = MSCbuffer[k+31];
				binFileSize = w.u32;

				aBinFile = 0;
				if(binFileSize != 0){
					aBinFile = 1;
					statePage = 0;
					binFileStart = programAddressStart;
				}

				sprintf(strAux, "WBIN: %4u -- %4u -- %6u\r\n", (unsigned int)blk_addr, (unsigned int)blk_len, (unsigned int)binFileSize);

				for(int i=0; i<30; i++){
					txBuf[iw++] = strAux[i];
					iw &= BUFMASKTX;
				}
			}
		}
	}
	else{
		if(aBinFile && binFileSize!=0){

			switch (statePage) {
				case 0://Add first 512 bytes to bufPage
					memcpy(bufPage, buf, 512);
					statePage = 1;

					if(binFileSize <= 512){
						memset(&bufPage[512], 0xFF, 512);
						statePage = 0;
					}
					break;
				case 1:
					memcpy(&bufPage[512], buf, 512);
					statePage = 0;
					break;
			}


			if(statePage == 0){
				if(FlashPage(binFileStart) == 0){
					memcpy(&MSCbuffer[MESSAGGE_START], msgERROR, sizeof(msgERROR));
					sprintf(strAux, "PAGE ERROR: %8X\r\n", (unsigned int)binFileStart);
					for(int i=0; i<22; i++){
						txBuf[iw++] = strAux[i];
						iw &= BUFMASKTX;
					}
					aBinFile = 0;
				}

				sprintf(strAux, "PAGE: %8X %8X\r\n", (unsigned int)binFileStart, *((unsigned int *)&bufPage[0]));
				for(int i=0; i<25; i++){
					txBuf[iw++] = strAux[i];
					iw &= BUFMASKTX;
				}
				binFileStart += 1024;
			}

			binFileSize -= blk_len*STORAGE_BLK_SIZ;
			if(binFileSize & 0x80000000)
				binFileSize = 0;

//			sprintf(strAux, "WBLK: %4u -- %4u -- %6u\r\n", (unsigned int)blk_addr, (unsigned int)blk_len, (unsigned int)binFileSize);
//			for(int i=0; i<30; i++){
//				txBuf[iw++] = strAux[i];
//				iw &= BUFMASKTX;
//			}
			if(binFileSize == 0){
				aBinFile = 0;
				sprintf(strAux, "PAGE    OK: %8X\r\n", (unsigned int)binFileStart);
				for(int i=0; i<22; i++){
					txBuf[iw++] = strAux[i];
					iw &= BUFMASKTX;
				}
				memcpy(&MSCbuffer[MESSAGGE_START], msgOk, sizeof(msgOk));

//				sprintf(strAux, "WEND: %4u -- %4u -- %6u\r\n", (unsigned int)blk_addr, (unsigned int)blk_len, (unsigned int)binFileSize);
//				for(int i=0; i<30; i++){
//					txBuf[iw++] = strAux[i];
//					iw &= BUFMASKTX;
//				}
//
//				memcpy(&MSCbuffer[MESSAGGE_START], msgOk, 32);
			}
		}

	}

  return (USBD_OK);
  /* USER CODE END 7 */
}

/**
  * @brief  .
  * @param  None
  * @retval .
  */
int8_t STORAGE_GetMaxLun_FS(void)
{
  /* USER CODE BEGIN 8 */
  return (STORAGE_LUN_NBR - 1);
  /* USER CODE END 8 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
static uint8_t FlashPage(uint32_t pageAddress){
	int i;

	EraseInitStruct.PageAddress = pageAddress;

	HAL_FLASH_Unlock();
	if(HAL_FLASHEx_Erase(&EraseInitStruct, &w.u32) != HAL_OK){
		HAL_FLASH_Lock();
		return 0;
	}
	for(i=0; i<1024; i+=4){
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, pageAddress, *((uint32_t *)&bufPage[i])) != HAL_OK){
			break;
		}
		pageAddress += 4;
	}
	HAL_FLASH_Lock();
	if(i != 1024)
		return 0;
	return 1;
}

//static void BootLoader(void)
//{
//
//  /* Check if There is a New OTA */
//  if(data32==OTA_MAGIC_NUM) {
//    /* Make the OTA */
//    /* Configure the System clock */
//    SystemClock_Config();
//
//    /* Reset the First Flash Section */
//    {
//      FLASH_EraseInitTypeDef EraseInitStruct;
//      uint32_t SectorError = 0;
//
//      EraseInitStruct.TypeErase    = FLASH_TYPEERASE_PAGES;
//      EraseInitStruct.Banks = FLASH_BANK_1;
//      EraseInitStruct.PageAddress = PROG_SECTOR_START;
//      EraseInitStruct.NbPages = 1;
//
//      /* Unlock the Flash to enable the flash control register access *************/
//      HAL_FLASH_Unlock();
//
//      if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
//        /* Error occurred while sector erase.
//          User can add here some code to deal with this error.
//          SectorError will contain the faulty sector and then to know the code error on this sector,
//          user can call function 'HAL_FLASH_GetError()'
//          FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
//        while(1);
//      }
//
//      /* Lock the Flash to disable the flash control register access (recommended
//      to protect the FLASH memory against possible unwanted operation) *********/
//      HAL_FLASH_Lock();
//    }
//
//    /* Make the OTA */
//    {
//      int32_t WriteIndex;
//      uint32_t ProgAddress = (uint32_t  ) PROG_ADDRESS_START;
//
//      /* Unlock the Flash to enable the flash control register access *************/
//      HAL_FLASH_Unlock();
//
//      for(WriteIndex=0;WriteIndex<MAX_PROG_SIZE;WriteIndex+=4){
//        if (HAL_FLASH_Program(TYPEPROGRAM_WORD, ProgAddress+WriteIndex,OTAAddress[WriteIndex>>2]) != HAL_OK){
//          /* Error occurred while writing data in Flash memory.
//             User can add here some code to deal with this error
//             FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
//          while(1);
//        }
//      }
//
//      /* Lock the Flash to disable the flash control register access (recommended
//       to protect the FLASH memory against possible unwanted operation) *********/
//      HAL_FLASH_Lock();
//    }
//
//    /* Reset the Second Half of the Flash except where is stored the License Manager*/
//    {
//      FLASH_EraseInitTypeDef EraseInitStruct;
//      uint32_t SectorError = 0;
//
//      /* Unlock the Flash to enable the flash control register access *************/
//      HAL_FLASH_Unlock();
//
//      /* Reset the Second half Flash */
//      EraseInitStruct.TypeErase    = TYPEERASE_SECTORS;
//      EraseInitStruct.VoltageRange = VOLTAGE_RANGE_3;
//      EraseInitStruct.Sector       = OTA_SECTOR_START;
//      EraseInitStruct.NbSectors    = OTA_NUM_SECTORS_128K;
//
//      if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK){
//        /* Error occurred while sector erase.
//          User can add here some code to deal with this error.
//          SectorError will contain the faulty sector and then to know the code error on this sector,
//          user can call function 'HAL_FLASH_GetError()'
//          FLASH_ErrorTypeDef errorcode = HAL_FLASH_GetError(); */
//        while(1);
//      }
//
//      /* Lock the Flash to disable the flash control register access (recommended
//       to protect the FLASH memory against possible unwanted operation) *********/
//      HAL_FLASH_Lock();
//    }
//
//    /* System Reboot */
//    HAL_NVIC_SystemReset();
//  } else {
//    /* Jump To Normal boot */
////    typedef  void (*pFunction)(void);
////
////    pFunction JumpToApplication;
////    uint32_t JumpAddress;
//
//    /* reset all interrupts to default */
//   // __disable_irq();
//
//    /* Jump to system memory */
//    JumpAddress = *(__IO uint32_t*) (PROG_ADDRESS_START + 4);
//    JumpToApplication = (pFunction) JumpAddress;
//    /* Initialize user application's Stack Pointer */
//    __set_MSP(*(__IO uint32_t*) PROG_ADDRESS_START);
//    JumpToApplication();
//  }
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

