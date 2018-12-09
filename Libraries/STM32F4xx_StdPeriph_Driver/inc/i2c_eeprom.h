#ifndef __i2c_eeprom_H
#define __i2c_eeprom_H
#include "stm32f4xx.h"
#include "basicFunc.h"

#define I2C_EEPROM I2C2
#define EEPROM_ADD_24c64n 0xA0
#define EE_Write	0xA0
#define EE_Read		0xA1

#define I2C_PageSize 32

void I2C2_Configuration(void);
void I2C_ByteWrite(u8 rowAdd, u8 colAdd, u8* pBuffer);
void I2C_PageWrite(u16 add, u8* pBuffer, u8 NumByteToWrite);

void I2C_BufferWrite(u16 add, u8* pBuffer, u16 NumByteToWrite);
void I2C_BufferRead(u16 add, u8 *pBuffer, u16 NumByteToRead);



#endif
