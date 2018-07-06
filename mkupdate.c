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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>       
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "update.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define HIFILE_D_MAGICNUM       0x4C4F4144
#define HIFILE_D_HEARDCRC_POS   4
#define HIFILE_D_HEARDCRC_LEN   4
#define HIFILE_D_PARTITION_TAG_LEN   8
#define HIFILE_D_ALIGN_LEN   16


#define HI_LOADER_PROTOCOL_VER3
#define HI_IMAGE_FILE_ALIGN_SUPPORT

#define HI_WRITE_64BIT(pcByte, Result, Length)   \
    {                                           \
    	(pcByte)[0] = ((Result) >> 56) & 0xFF;	\
		(pcByte)[1] = ((Result) >> 48) & 0xFF;	\
		(pcByte)[2] = ((Result) >> 40) & 0xFF;	\
		(pcByte)[3] = ((Result) >> 32) & 0xFF;	\
		(pcByte)[4] = ((Result) >> 24) & 0xFF;	\
		(pcByte)[5] = ((Result) >> 16) & 0xFF;	\
		(pcByte)[6] = ((Result) >> 8) & 0xFF;		\
		(pcByte)[7] = (Result) & 0xFF;			\
        (pcByte) += 8;                          \
        (Length) += 8;                          \
    }

#define HI_WRITE_32BIT(pcByte, Result, Length)   \
    {                                           \
		(pcByte)[0] = ((Result) >> 24) & 0xFF;	\
		(pcByte)[1] = ((Result) >> 16) & 0xFF;	\
		(pcByte)[2] = ((Result) >> 8) & 0xFF; 	\
		(pcByte)[3] = (Result) & 0xFF;			\
        (pcByte) += 4;                          \
        (Length) += 4;                          \
    }

#define HI_WRITE_24BIT(pcByte, Result, Length)   \
    {                                           \
		(pcByte)[0] = ((Result) >> 16) & 0xFF;	\
		(pcByte)[1] = ((Result) >> 8) & 0xFF; 	\
		(pcByte)[2] = (Result) & 0xFF;			\
        (pcByte) += 3;                          \
        (Length) += 3;                          \
    }

#define HI_WRITE_16BIT(pcByte, Result, Length)   \
    {                                           \
		(pcByte)[0] = ((Result) >> 8) & 0xFF; 	\
		(pcByte)[1] = (Result) & 0xFF;			\
        (pcByte) += 2;                          \
        (Length) += 2;                          \
    }

#define HI_WRITE_08BIT(pcByte, Result, Length)   \
    {                                           \
		(pcByte)[0] = (Result) & 0xFF;			\
        (pcByte) += 1;                          \
        (Length) += 1;                          \
    }

#define HI_SKIP_BYTES(pcByte, ByteCount, Length)                        \
    {                                                                   \
        (pcByte) = (pcByte) + (ByteCount);                              \
        (Length) += (ByteCount);                                        \
    }

#define HI_WRITE_BYTES(pcByte, ByteCount, Buffer,Length)    \
    {                                                                   \
        if (0 < (ByteCount))                                            \
        {                                                               \
            memcpy((pcByte), (Buffer), ByteCount);                  \
            (pcByte) += (ByteCount);                                    \
            (Length) += (ByteCount);                                    \
        }                                                               \
    }


static const unsigned int aCrc32_tab[] =
{
    0x00000000L, 0x04c11db7L, 0x09823b6eL, 0x0d4326d9L, 0x130476dcL, 0x17c56b6bL,
    0x1a864db2L, 0x1e475005L, 0x2608edb8L, 0x22c9f00fL, 0x2f8ad6d6L, 0x2b4bcb61L,
    0x350c9b64L, 0x31cd86d3L, 0x3c8ea00aL, 0x384fbdbdL, 0x4c11db70L, 0x48d0c6c7L,
    0x4593e01eL, 0x4152fda9L, 0x5f15adacL, 0x5bd4b01bL, 0x569796c2L, 0x52568b75L,
    0x6a1936c8L, 0x6ed82b7fL, 0x639b0da6L, 0x675a1011L, 0x791d4014L, 0x7ddc5da3L,
    0x709f7b7aL, 0x745e66cdL, 0x9823b6e0L, 0x9ce2ab57L, 0x91a18d8eL, 0x95609039L,
    0x8b27c03cL, 0x8fe6dd8bL, 0x82a5fb52L, 0x8664e6e5L, 0xbe2b5b58L, 0xbaea46efL,
    0xb7a96036L, 0xb3687d81L, 0xad2f2d84L, 0xa9ee3033L, 0xa4ad16eaL, 0xa06c0b5dL,
    0xd4326d90L, 0xd0f37027L, 0xddb056feL, 0xd9714b49L, 0xc7361b4cL, 0xc3f706fbL,
    0xceb42022L, 0xca753d95L, 0xf23a8028L, 0xf6fb9d9fL, 0xfbb8bb46L, 0xff79a6f1L,
    0xe13ef6f4L, 0xe5ffeb43L, 0xe8bccd9aL, 0xec7dd02dL, 0x34867077L, 0x30476dc0L,
    0x3d044b19L, 0x39c556aeL, 0x278206abL, 0x23431b1cL, 0x2e003dc5L, 0x2ac12072L,
    0x128e9dcfL, 0x164f8078L, 0x1b0ca6a1L, 0x1fcdbb16L, 0x018aeb13L, 0x054bf6a4L,
    0x0808d07dL, 0x0cc9cdcaL, 0x7897ab07L, 0x7c56b6b0L, 0x71159069L, 0x75d48ddeL,
    0x6b93dddbL, 0x6f52c06cL, 0x6211e6b5L, 0x66d0fb02L, 0x5e9f46bfL, 0x5a5e5b08L,
    0x571d7dd1L, 0x53dc6066L, 0x4d9b3063L, 0x495a2dd4L, 0x44190b0dL, 0x40d816baL,
    0xaca5c697L, 0xa864db20L, 0xa527fdf9L, 0xa1e6e04eL, 0xbfa1b04bL, 0xbb60adfcL,
    0xb6238b25L, 0xb2e29692L, 0x8aad2b2fL, 0x8e6c3698L, 0x832f1041L, 0x87ee0df6L,
    0x99a95df3L, 0x9d684044L, 0x902b669dL, 0x94ea7b2aL, 0xe0b41de7L, 0xe4750050L,
    0xe9362689L, 0xedf73b3eL, 0xf3b06b3bL, 0xf771768cL, 0xfa325055L, 0xfef34de2L,
    0xc6bcf05fL, 0xc27dede8L, 0xcf3ecb31L, 0xcbffd686L, 0xd5b88683L, 0xd1799b34L,
    0xdc3abdedL, 0xd8fba05aL, 0x690ce0eeL, 0x6dcdfd59L, 0x608edb80L, 0x644fc637L,
    0x7a089632L, 0x7ec98b85L, 0x738aad5cL, 0x774bb0ebL, 0x4f040d56L, 0x4bc510e1L,
    0x46863638L, 0x42472b8fL, 0x5c007b8aL, 0x58c1663dL, 0x558240e4L, 0x51435d53L,
    0x251d3b9eL, 0x21dc2629L, 0x2c9f00f0L, 0x285e1d47L, 0x36194d42L, 0x32d850f5L,
    0x3f9b762cL, 0x3b5a6b9bL, 0x0315d626L, 0x07d4cb91L, 0x0a97ed48L, 0x0e56f0ffL,
    0x1011a0faL, 0x14d0bd4dL, 0x19939b94L, 0x1d528623L, 0xf12f560eL, 0xf5ee4bb9L,
    0xf8ad6d60L, 0xfc6c70d7L, 0xe22b20d2L, 0xe6ea3d65L, 0xeba91bbcL, 0xef68060bL,
    0xd727bbb6L, 0xd3e6a601L, 0xdea580d8L, 0xda649d6fL, 0xc423cd6aL, 0xc0e2d0ddL,
    0xcda1f604L, 0xc960ebb3L, 0xbd3e8d7eL, 0xb9ff90c9L, 0xb4bcb610L, 0xb07daba7L,
    0xae3afba2L, 0xaafbe615L, 0xa7b8c0ccL, 0xa379dd7bL, 0x9b3660c6L, 0x9ff77d71L,
    0x92b45ba8L, 0x9675461fL, 0x8832161aL, 0x8cf30badL, 0x81b02d74L, 0x857130c3L,
    0x5d8a9099L, 0x594b8d2eL, 0x5408abf7L, 0x50c9b640L, 0x4e8ee645L, 0x4a4ffbf2L,
    0x470cdd2bL, 0x43cdc09cL, 0x7b827d21L, 0x7f436096L, 0x7200464fL, 0x76c15bf8L,
    0x68860bfdL, 0x6c47164aL, 0x61043093L, 0x65c52d24L, 0x119b4be9L, 0x155a565eL,
    0x18197087L, 0x1cd86d30L, 0x029f3d35L, 0x065e2082L, 0x0b1d065bL, 0x0fdc1becL,
    0x3793a651L, 0x3352bbe6L, 0x3e119d3fL, 0x3ad08088L, 0x2497d08dL, 0x2056cd3aL,
    0x2d15ebe3L, 0x29d4f654L, 0xc5a92679L, 0xc1683bceL, 0xcc2b1d17L, 0xc8ea00a0L,
    0xd6ad50a5L, 0xd26c4d12L, 0xdf2f6bcbL, 0xdbee767cL, 0xe3a1cbc1L, 0xe760d676L,
    0xea23f0afL, 0xeee2ed18L, 0xf0a5bd1dL, 0xf464a0aaL, 0xf9278673L, 0xfde69bc4L,
    0x89b8fd09L, 0x8d79e0beL, 0x803ac667L, 0x84fbdbd0L, 0x9abc8bd5L, 0x9e7d9662L,
    0x933eb0bbL, 0x97ffad0cL, 0xafb010b1L, 0xab710d06L, 0xa6322bdfL, 0xa2f33668L,
    0xbcb4666dL, 0xb8757bdaL, 0xb5365d03L, 0xb1f740b4L
};

#define DO1(u32Crc, buf) u32Crc = (u32Crc << 8) ^ aCrc32_tab[((u32Crc >> 24) ^ *(buf)++) & 0xff];
#define DO2(u32Crc, buf) DO1(u32Crc, buf) DO1(u32Crc, buf)
#define DO4(u32Crc, buf) DO2(u32Crc, buf) DO2(u32Crc, buf)
#define DO8(u32Crc, buf) DO4(u32Crc, buf) DO4(u32Crc, buf)

static HI_U32 COM_CRC32(HI_U32 u32Crc, const HI_U8* pu8Data, HI_U32 u32Len)
{
    HI_U8* pu8Temp = (HI_U8*) pu8Data;

    if (NULL == pu8Data)
    {
        return 0;
    }

    while (8 <= u32Len)
    {
        DO8(u32Crc, pu8Temp);
        u32Len -= 8;
    }

    if (0 < u32Len)
    {
        do
        {
            DO1(u32Crc, pu8Temp);
        }
        while (--u32Len);
    }

    return u32Crc;
}

static void GetDir(char *filename,char *dir_p)
{
	int i;
	strcpy(dir_p,filename);
	
	i = strlen(dir_p) - 1;
	while(i >= 0)
	{
		if(dir_p[i] == '/')
		{
			dir_p[i+1] = 0;
			break;
		}
		i --;
	}
}
static void free_res(HIFILE_MANUINFO_S *astManuInfo_p)
{
	int j;
	HIFILE_FLASHMAP_S *pstFlashMap;
	for (j = 0; j < astManuInfo_p->u16FlashMapNum; ++j)
	{
		pstFlashMap = &astManuInfo_p->astFlashMap[j];
		if (pstFlashMap->pu8ImageData){
			free(pstFlashMap->pu8ImageData);
		}
	}
}
static int parse_partitions(char *filepath_p, HIFILE_MANUINFO_S *astManuInfo_p)
{
	FILE *hf = NULL;
	FILE *hp = NULL;
	char buff[1024]={0};
	char tmp_buff[1024]={0};
	char tmp_file[1024]={0};
	char *p, *end;
	struct stat filestat;
	int ret;
	HI_U32		Offset = 0;

	if (astManuInfo_p == NULL){
        printf("invalid params!\n");
        return -1;
	}
	if((hf = fopen(filepath_p, "r")) == NULL)
	{
        printf("open %s error!\n",filepath_p);
        return -1;
	}

	while(1)
	{
		if (fgets(buff, 1024 - 1, hf) == NULL)
		{
			break;
		}

		if ((p=strstr(buff,"PartitionName")))
		{
			memset(&astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum],0,sizeof(HIFILE_FLASHMAP_S));
			p += strlen("PartitionName");
			p += 2;
			end = strstr(p,"\"");
			memcpy(astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].au8PartName,p,end-p);
			//printf("au8PartName = %s\n",astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].au8PartName);

			if ((p=strstr(buff,"FlashType")) == NULL){
				printf("no flash type find!\n");
				goto do_error;
			}
			p += strlen("FlashType");
			p += 2;
			end = strstr(p,"\"");
			memset(tmp_buff,0,sizeof(tmp_buff));
			memcpy(tmp_buff,p,end-p);	
			
			if (!strcmp(tmp_buff,"emmc")){
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u32FlashType = HI_FLASH_TYPE_EMMC_0;
			}else if (!strcmp(tmp_buff,"spi")){
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u32FlashType = HI_FLASH_TYPE_SPI_0;
			}else if (!strcmp(tmp_buff,"nand")){
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u32FlashType = HI_FLASH_TYPE_NAND_0;
			}else if (!strcmp(tmp_buff,"sd")){
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u32FlashType = HI_FLASH_TYPE_SD;			
			}else{
				printf("no flash type find!\n");
				goto do_error;
			}

			if ((p=strstr(buff,"Start")) == NULL){
				printf("can't find start address!\n");
				goto do_error;
			}
			
			p += strlen("Start");
			p += 2;
			end = strstr(p,"\"");
			memset(tmp_buff,0,sizeof(tmp_buff));
			memcpy(tmp_buff,p,end-p);
			p = strstr(tmp_buff,"M");
			if ((p = strstr(tmp_buff,"M"))) {
				*p = 0;
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u64PartitionStartAddr = atoi(tmp_buff)*1024*1024;
			}else if ((p = strstr(tmp_buff,"K"))){
				*p = 0;
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u64PartitionStartAddr = atoi(tmp_buff)*1024;
			}
			else{
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u64PartitionStartAddr = atoi(tmp_buff);
			}
			//printf("u64PartitionStartAddr = %llx\n",astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u64PartitionStartAddr);
			if ((p=strstr(buff,"Length")) == NULL){
				printf("can't find patition length!\n");
				goto do_error;
			}			
			p += strlen("Length");
			p += 2;
			end = strstr(p,"\"");
			memset(tmp_buff,0,sizeof(tmp_buff));
			memcpy(tmp_buff,p,end-p);	
			if ((p = strstr(tmp_buff,"M"))) {
				*p = 0;
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u64PartitionEndAddr = (HI_U32)(atoll(tmp_buff)*1024*1024)+astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u64PartitionStartAddr;
			}else if ((p = strstr(tmp_buff,"K"))){
				*p = 0;
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u64PartitionEndAddr = atoll(tmp_buff)*1024+astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u64PartitionStartAddr;
			}
			else{
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u64PartitionEndAddr = atoll(tmp_buff)+astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u64PartitionStartAddr;
			}

			//printf("u64PartitionEndAddr = %llx\n",astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u64PartitionEndAddr);
			
			if ((p=strstr(buff,"SelectFile")) == NULL){
				printf("can't find SelectFile tag!\n");
				goto do_error;
			}			
			p += strlen("SelectFile");
			p += 2;
			end = strstr(p,"\"");
			memset(tmp_buff,0,sizeof(tmp_buff));
			memcpy(tmp_buff,p,end-p);
			if (tmp_buff[0] == 0){
				//printf("no SelectFile file find!\n");
				continue;
			}
			
			GetDir(filepath_p,tmp_file);
			strcat(tmp_file,tmp_buff);
			//printf("SelectFile = %s, realpath  = %s\n",tmp_buff,tmp_file);
			
			if (lstat(tmp_file, &filestat) != 0)
			{
				printf("no patition file %s find!\n",tmp_file);
				goto do_error;
			}
			
			astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u32ImageLength = filestat.st_size;

			astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].pu8ImageData = malloc(filestat.st_size);
			if (astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].pu8ImageData == NULL){
				printf("patition malloc fail!\n");
				goto do_error;
			}
			if((hp = fopen(tmp_file, "r")) == NULL)
			{
				printf("open %s error!",tmp_file);
				goto do_error;
			}
			
			ret = fread(astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].pu8ImageData ,1 ,filestat.st_size ,hp);
			if (ret != filestat.st_size){
				printf("read %s error!",tmp_file);
				goto do_error;
			}
			astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u32ImageOffset = Offset;
			astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u32ImageCRC = COM_CRC32(0xFFFFFFFFL, 
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].pu8ImageData,
				astManuInfo_p->astFlashMap[astManuInfo_p->u16FlashMapNum].u32ImageLength);
			
			Offset += (filestat.st_size+HIFILE_D_PARTITION_TAG_LEN);
			astManuInfo_p->u16FlashMapNum ++;
			}
	}
		
	fclose(hf);
	return 0;
do_error:	
	free_res(astManuInfo_p);
	fclose(hf);
	return -1;
}

static int calc_info(HIFILE_DOCUMENT_S *FileInof_p)
{
	int i,j,len;
	int header_len = 0;
	int partition_image_len = 0;
	HIFILE_MANUINFO_S	*astManuInfo_p;
	HIFILE_FLASHMAP_S *pstFlashMap;

	header_len += 12;
#if defined (HI_LOADER_PROTOCOL_VER2)
	header_len += 4;
#endif
	header_len += 4;
	header_len += 2;

	for (i = 0; i < FileInof_p->u16ManuNum; ++i){

		astManuInfo_p = &FileInof_p->astManuInfo[i];
		header_len += 30;
		for (j = 0; j < astManuInfo_p->u16FlashMapNum; ++j)
		{
			pstFlashMap = &astManuInfo_p->astFlashMap[j];

			header_len += 8;

#if defined (HI_LOADER_PROTOCOL_VER3) || defined (HI_LOADER_PROTOCOL_VER2)
			header_len += 16;
#elif defined(HI_LOADER_PROTOCOL_VER1)
			header_len += 8;
#endif
			header_len += 8;
#if defined (HI_LOADER_PROTOCOL_VER3)
			header_len += 8;
			len = strlen(pstFlashMap->au8PartName);
			header_len += 2;
			header_len += len;
			header_len += 2;
#endif
			partition_image_len += (pstFlashMap->u32ImageLength+8);
		}
	}
	
	FileInof_p->u32FileLen = header_len + partition_image_len;
#if defined (HI_IMAGE_FILE_ALIGN_SUPPORT)
	if ((FileInof_p->u32FileLen%HIFILE_D_ALIGN_LEN)){
		FileInof_p->u32FileLen = (FileInof_p->u32FileLen+HIFILE_D_ALIGN_LEN)/HIFILE_D_ALIGN_LEN*HIFILE_D_ALIGN_LEN;
	}
#endif	
	FileInof_p->u32HeaderLen = header_len;
	return 0;
}

#define HW_DEFARGS 		"00000003-00000001-01010101"
#define UPDATE_DEFFILE_PATH 	"./usb_update.bin"

int main(int argc,char* argv[])
{	
    HI_U8 *ac8HeadBuf = NULL;
    HI_U8 *ac8TmpBuf = NULL;
    FILE *fp = NULL;
	int ret,len,oc = 0;
	int i,j,length = 0;
	HI_S8 hw_defstr[64] = {0};
	HI_S8 update_deffile[1024] = {0};
	HI_S8 filepath[1024] = {0};
	HIFILE_DOCUMENT_S FileInof;
	HIFILE_MANUINFO_S	*astManuInfo_p;
	HIFILE_FLASHMAP_S *pstFlashMap;
	HI_U32 header_crc;
    HI_U8 ac8header_tag[8] = {0};
	int total_write = 0;
	
	if (argc < 2) {
		printf("Usage: mkupdate:\n"
		       "       [-s stbidstring -f partitionfile]\n");
    	printf("Example:./mkupdate -s 00000003-00000001-01010101 -f ./emmc_partitions.xml -d ./usb_update.bin\n");
		return -1;
	}

	while ((oc = getopt(argc, argv, "s:f:d:")) != -1) {
		switch (oc) {
		case 's':
			memcpy(hw_defstr, optarg, strlen(optarg)+1);
			break;
		case 'f':
			memcpy(filepath, optarg, strlen(optarg)+1);
			//printf("filepath is %s\n", filepath);
			break;
		case 'd':
			memcpy(update_deffile, optarg, strlen(optarg)+1);
			//printf("update_deffile is %s\n", update_deffile);
			break;
		default:
			break;
		}
	}

	memset(&FileInof,0,sizeof(FileInof));
	FileInof.u16ManuNum = 1;
	astManuInfo_p = &FileInof.astManuInfo[0];
	
	sscanf(hw_defstr, "%x-%x-%x", &astManuInfo_p->u32ManufacturerID, &astManuInfo_p->u32HardwareVersion, &astManuInfo_p->u32SoftwareVersion);	
	astManuInfo_p->u32SerialNumberStart = 0x0;
	astManuInfo_p->u32SerialNumberEnd = 0xFFFFFFFF;
	
	ret = parse_partitions(filepath,astManuInfo_p);
	if (ret < 0){
        printf("parse %s error!\n",filepath);
		return -1;
	}
	
	calc_info(&FileInof);

	ac8HeadBuf = ac8TmpBuf = malloc(HIFILE_D_MAXLEN_HEADER);
	memset(ac8TmpBuf,0,HIFILE_D_MAXLEN_HEADER);
	
	HI_WRITE_32BIT(ac8TmpBuf,HIFILE_D_MAGICNUM,length);
	
	HI_SKIP_BYTES(ac8TmpBuf,4,length);
	HI_WRITE_32BIT(ac8TmpBuf, FileInof.u32HeaderLen, length);
#if defined (HI_LOADER_PROTOCOL_VER2)
	HI_SKIP_BYTES(ac8TmpBuf, 4, length);
#endif
	HI_WRITE_32BIT(ac8TmpBuf, FileInof.u32FileLen, length);

	HI_WRITE_16BIT(ac8TmpBuf,FileInof.u16ManuNum,length);

	for (i = 0; i < FileInof.u16ManuNum; ++i){

		astManuInfo_p = &FileInof.astManuInfo[i];
		astManuInfo_p->u32DownloadType = 1;
			
		HI_WRITE_32BIT(ac8TmpBuf, astManuInfo_p->u32ManufacturerID, length);
		HI_WRITE_32BIT(ac8TmpBuf, astManuInfo_p->u32HardwareVersion, length);
		HI_WRITE_32BIT(ac8TmpBuf, astManuInfo_p->u32SoftwareVersion, length);
		HI_WRITE_32BIT(ac8TmpBuf, astManuInfo_p->u32SerialNumberStart, length);
		HI_WRITE_32BIT(ac8TmpBuf, astManuInfo_p->u32SerialNumberEnd, length);
		HI_WRITE_32BIT(ac8TmpBuf, astManuInfo_p->u32DownloadType, length);
		HI_WRITE_32BIT(ac8TmpBuf, astManuInfo_p->u32Reserved, length);
		HI_WRITE_16BIT(ac8TmpBuf, astManuInfo_p->u16FlashMapNum, length);
	
		for (j = 0; j < astManuInfo_p->u16FlashMapNum; ++j)
		{
			pstFlashMap = &astManuInfo_p->astFlashMap[j];

			HI_WRITE_32BIT(ac8TmpBuf, pstFlashMap->u32ImageLength, length);

			HI_WRITE_32BIT(ac8TmpBuf, pstFlashMap->u32ImageOffset+FileInof.u32HeaderLen, length);
#if defined (HI_LOADER_PROTOCOL_VER3) || defined (HI_LOADER_PROTOCOL_VER2)
			HI_WRITE_64BIT(ac8TmpBuf, pstFlashMap->u64PartitionStartAddr, length);
			HI_WRITE_64BIT(ac8TmpBuf, pstFlashMap->u64PartitionEndAddr, length);
#elif defined(HI_LOADER_PROTOCOL_VER1)
			HI_WRITE_32BIT(ac8TmpBuf, pstFlashMap->u64PartitionStartAddr, length);
			HI_WRITE_32BIT(ac8TmpBuf, pstFlashMap->u64PartitionEndAddr, length);
#endif
			HI_WRITE_32BIT(ac8TmpBuf, pstFlashMap->u32FlashType, length);
			HI_WRITE_32BIT(ac8TmpBuf, pstFlashMap->u32FlashIndex, length);

#if defined (HI_LOADER_PROTOCOL_VER3)
			HI_WRITE_32BIT(ac8TmpBuf, pstFlashMap->u32PartVersion, length);
			HI_WRITE_32BIT(ac8TmpBuf, pstFlashMap->u32PartVerifyTag, length);
			len = strlen(pstFlashMap->au8PartName);
			HI_WRITE_16BIT(ac8TmpBuf, len, length);
			HI_WRITE_BYTES(ac8TmpBuf, len, pstFlashMap->au8PartName, length);
			HI_WRITE_16BIT(ac8TmpBuf, 0, length);
			HI_SKIP_BYTES(ac8TmpBuf, 0, length);
#endif
		}

	}

	header_crc = COM_CRC32(0xFFFFFFFFL, ac8HeadBuf, FileInof.u32HeaderLen);
	ac8TmpBuf = ac8HeadBuf + HIFILE_D_HEARDCRC_POS;
	HI_WRITE_32BIT(ac8TmpBuf, header_crc, length);

    fp = fopen(update_deffile, "w+b");
    if(NULL == fp)
    {
        printf("open %s error!\n",update_deffile);
		goto  do_exit;
    }

    ret = fwrite(ac8HeadBuf, 1, FileInof.u32HeaderLen, fp);
	if (ret != FileInof.u32HeaderLen){
        printf("write header error!\n");
        goto  do_exit;
	}
	total_write += FileInof.u32HeaderLen;
	
	for (i = 0; i < FileInof.u16ManuNum; ++i){

		astManuInfo_p = &FileInof.astManuInfo[i];
		
		for (j = 0; j < astManuInfo_p->u16FlashMapNum; ++j)
		{
			pstFlashMap = &astManuInfo_p->astFlashMap[j];
			ac8TmpBuf = ac8header_tag;
			HI_WRITE_32BIT(ac8TmpBuf, pstFlashMap->u32ImageLength, length);
			HI_WRITE_32BIT(ac8TmpBuf, pstFlashMap->u32ImageCRC, length);
			ret = fwrite(ac8header_tag, 1, 8, fp);
			if (ret != 8){
		        printf("write partition tag error!\n");
				goto  do_exit;
			}			
			
			total_write += 8;
			ret = fwrite(pstFlashMap->pu8ImageData, 1, pstFlashMap->u32ImageLength, fp);
			if (ret != pstFlashMap->u32ImageLength){
		        printf("write partition data error!\n");
				goto  do_exit;
			}			
			total_write += pstFlashMap->u32ImageLength;
		}
	}

	if (FileInof.u32FileLen > total_write){
		ac8TmpBuf = malloc(FileInof.u32FileLen-total_write);
		if (ac8TmpBuf == NULL){
			printf("malloc %dBytes error!\n",FileInof.u32FileLen-total_write);
			goto  do_exit;
		}
		memset(ac8TmpBuf,0xFF,FileInof.u32FileLen-total_write);
		ret = fwrite(ac8TmpBuf, 1, FileInof.u32FileLen-total_write, fp);
		if (ret != (FileInof.u32FileLen-total_write)){
	        printf("write align data error!\n");
			free(ac8TmpBuf);
			goto  do_exit;
		}		

		free(ac8TmpBuf);
	}

do_exit:
	if (ac8HeadBuf)
		free(ac8HeadBuf);
	astManuInfo_p = &FileInof.astManuInfo[0];
	free_res(astManuInfo_p);
	if (fp){
		fclose(fp);
	}
	return 0;
}

#ifdef  __cplusplus
}
#endif

