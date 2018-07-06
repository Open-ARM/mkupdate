/*********************************************************************************
*
*  Copyright (C) 2014 Hisilicon Technologies Co., Ltd.  All rights reserved. 
*
*  This program is confidential and proprietary to Hisilicon Technologies Co., Ltd.
*  (Hisilicon), and may not be copied, reproduced, modified, disclosed to
*  others, published or used, in whole or in part, without the express prior
*  written permission of Hisilicon.
*
***********************************************************************************/

/*******************************************************************************/

#ifndef __LOADERDB_H__
#define __LOADERDB_H__

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif

typedef unsigned char           HI_U8;
typedef unsigned char           HI_UCHAR;
typedef unsigned short          HI_U16;
typedef unsigned int            HI_U32;
typedef unsigned long           HI_ULONG;

typedef signed char             HI_S8;
typedef short                   HI_S16;
typedef int                     HI_S32;
typedef long                    HI_SLONG;

#ifndef _M_IX86
typedef unsigned long long      HI_U64;
typedef long long               HI_S64;
#else
typedef __int64                 HI_U64;
typedef __int64                 HI_S64;
#endif

typedef char                    HI_CHAR;
typedef char*                   HI_PCHAR;

typedef float                   HI_FLOAT;
typedef double                  HI_DOUBLE;
/*typedef void                    HI_VOID;*/
#define HI_VOID         void

typedef unsigned long           HI_SIZE_T;
typedef unsigned long           HI_LENGTH_T;

typedef HI_U32                  HI_HANDLE;

typedef unsigned int            HI_PHYS_ADDR_T;

#ifdef CONFIG_ARCH_LP64_MODE
typedef unsigned long long      HI_VIRT_ADDR_T;
#else
typedef unsigned int            HI_VIRT_ADDR_T;
#endif

typedef enum {
    HI_FALSE    = 0,
    HI_TRUE     = 1,
} HI_BOOL;

/*************************** Structure Definition ****************************/
#define HIFILE_D_MAXNUM_FLASHMAP                (64)
#define HIFILE_D_MAXNUM_MANUFACTURER            (3)
#define HIFILE_D_MAXLEN_PARTNAME                (64)
#define HIFILE_D_MAXLEN_HEADER                	(16*1024)

/****** Macro Functions definition ********/

/** @} */  /*! <!-- Macro Definition End */

/*************************** Structure Definition *****************************/
/** \addtogroup      HIFILE_PARSE */
/** @{ */  /** <!-- [HIFILE_PARSE] */

/****** Enumeration definition ************/

/****** Structure definition **************/

typedef struct tagHIFILE_FLASHMAP_S
{
    HI_U32      u32ImageID;                 /**< Image identify */
    HI_U32      u32ImageLength;             /**< Image length */
    HI_U32      u32ImageCRC;                /**< Image CRC value */
    HI_U32      u32ImageOffset;             /**< Image Address offset */
    HI_U64      u64PartitionStartAddr;      /**< Address in the flash */
    HI_U64      u64PartitionEndAddr;        /**< The end address of flash partition */
    HI_U32      u32FlashType;               /**< Flash type */
    HI_U32      u32FlashIndex;              /**< Flash chip select */

    HI_U32      u32PartVersion;             /**< Partition Version */
    HI_U32      u32PartVerifyTag;           /**< Partition Verify Tag */
    HI_U8       au8PartName[HIFILE_D_MAXLEN_PARTNAME]; /**< Partition Name */

    HI_U8*      pu8ImageData;

    HI_U32      u32ImageVersion;
    HI_BOOL     bNeedUpgrade;

} HIFILE_FLASHMAP_S;

typedef struct tagHIFILE_MANUINFO_S
{
    HI_U32      u32ManufacturerID;          /**< Manufacturer ID*/
    HI_U32      u32HardwareVersion;         /**< Hardware version*/
    HI_U32      u32SoftwareVersion;         /**< Software version*/
    HI_U32      u32SerialNumberStart;       /**< Start serial number*/
    HI_U32      u32SerialNumberEnd;         /**< End serial number*/
    HI_U32      u32DownloadType;            /**< USB upgrade download mode */
    HI_U32      u32Reserved;                /**< Reserved for future expansion */
    HI_U32      u32ImageLength;             /**< Total image length */
    HI_U16      u16FlashMapNum;             /**< Total flashmap count */
    HIFILE_FLASHMAP_S astFlashMap[HIFILE_D_MAXNUM_FLASHMAP];   /**< flashmap information*/
} HIFILE_MANUINFO_S;


typedef struct tagHIFILE_DOCUMENT_S
{
    HI_U32      u32HeaderLen;               /**< Length of the file head */
    HI_U32      u32FileLen;                 /**< Length of file */
    HI_U16      u16ManuNum;                 /**< Count of Manufacturer */
    HIFILE_MANUINFO_S   astManuInfo[HIFILE_D_MAXNUM_MANUFACTURER];  /**< Manufacturer information */
} HIFILE_DOCUMENT_S;

typedef enum hiHI_FLASH_TYPE_E
{
	HI_FLASH_TYPE_SPI_0,
	HI_FLASH_TYPE_NAND_0,
	HI_FLASH_TYPE_EMMC_0,
	HI_FLASH_TYPE_SD,

	HI_FLASH_TYPE_BUTT
} HI_FLASH_TYPE_E;

#ifdef __cplusplus
 #if __cplusplus
}

 #endif
#endif

#endif

