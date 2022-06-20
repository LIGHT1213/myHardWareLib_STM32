#include "userFlash.h"
#include "string.h"
#include "log.h"
#ifdef USE_RTOS
osMutexId flashMutexHandle;
#endif
//==================================================================================================
//| 函数名称 | flashInit
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 |初始化flash操作
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | NONE
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |
//==================================================================================================
void flashInit()
{
	#ifdef USE_RTOS
	//FLASH操作互斥锁初始化
	osMutexDef(flashMutex);
	flashMutexHandle = osMutexCreate(osMutex(flashMutex));
	#endif
    HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | \
	                       FLASH_FLAG_PGPERR | \
	                       FLASH_FLAG_WRPERR);
	HAL_FLASH_Lock();
}
//==================================================================================================
//| 函数名称 | flashRead
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 |读取flash数据
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | address 读取地址  buf 读取数据 len读取的长度
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 1成功 0失败
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |
//==================================================================================================
uint8_t flashRead(uint32_t address, uint8_t *buf, uint16_t len)
{
	#ifdef USE_RTOS
	osMutexWait(flashMutexHandle,osWaitForever);
	#endif
    if (len == 0 || buf == NULL || address > 0x08010000 || address+len>0x08010000)
    {
		#ifdef USE_RTOS
		osMutexRelease(flashMutexHandle);
		#endif
		return 0; 
    }
	__IO uint16_t* addrBuf=(__IO uint16_t*)( address );
    memcpy(buf,(void *)addrBuf,len);
	#ifdef USE_RTOS
	osMutexRelease(flashMutexHandle);
	#endif
    return 1;
    
}
//==================================================================================================
//| 函数名称 | flashWrite
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 |写入flash数据
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | address 写入地址  buf 写入数据 len 写入的长度
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 1成功 0失败
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |
//==================================================================================================
uint8_t flashWrite(uint32_t address, uint8_t *buf, uint16_t len)
{
	#ifdef USE_RTOS
	osMutexWait(flashMutexHandle,osWaitForever);
	#endif
    if (len == 0 || buf == NULL || address > 0x08010000 || address+len>0x08010000)
    {
		#ifdef USE_RTOS
		osMutexRelease(flashMutexHandle);
		#endif
		return 0; 
    }
    HAL_FLASH_Unlock();               //解锁Flash
    //对Flash进行烧写，FLASH_TYPEPROGRAM_HALFWORD 声明操作的Flash地址的16位的，此外还有32位跟64位的操作，自行翻查HAL库的定义即可
    for( unsigned int i = 0; i < len; i++ )
    {
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address + i, buf[i] ) != HAL_OK )
        {
            HAL_FLASH_Lock();
			#ifdef USE_RTOS
			osMutexRelease(flashMutexHandle);
			#endif
            return 0;
        }
    }
    HAL_FLASH_Lock(); //锁住Flash
	osMutexRelease(flashMutexHandle);
    return 1;
}
//==================================================================================================
//| 函数名称 | flashErase
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 |擦除flash的一个扇区
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | address 扇区号 0-11
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 1成功 0失败
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |
//==================================================================================================
uint8_t flashErase(uint32_t address)
{
	#ifdef USE_RTOS
	osMutexWait(flashMutexHandle,osWaitForever);
	#endif
    if (address > FLASH_SECTOR_11)
    {
		#ifdef USE_RTOS
		osMutexRelease(flashMutexHandle);
		#endif
		logInfo("FLASH_SECTOR_%d erase error",address);
		return 0; 
    }  
    FLASH_EraseInitTypeDef flashEraseInit;
    uint32_t SectorError=0;
    HAL_FLASH_Unlock(); 
    
    
	flashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;       //擦除类型，扇区擦除 
    flashEraseInit.Sector = address;                   //要擦除的扇区
    flashEraseInit.NbSectors = 1;                             //一次只擦除一个扇区
    flashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;      //电压范围，VCC=2.7~3.6V之间!!
    

	if(HAL_FLASHEx_Erase(&flashEraseInit,&SectorError) != HAL_OK)
    {
		#ifdef USE_RTOS
		osMutexRelease(flashMutexHandle);
		#endif

		logInfo("FLASH_SECTOR_%d erase error",address);
        HAL_FLASH_Lock();
        return 0;
	} 
    
	osDelay(2000);
	logInfo("FLASH_SECTOR_%d erase ok",address);
    HAL_FLASH_Lock();
	#ifdef USE_RTOS
	osMutexRelease(flashMutexHandle);
	#endif
    return 1;
    
}