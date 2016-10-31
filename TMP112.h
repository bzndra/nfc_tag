/*
 * {TMP112.h}
 *
 * {Driver for TMP112 Temperature Sensor}
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#ifndef TMP103_H_
#define TMP103_H_

#include "msp430.h"
#include "RF430_I2C.h"

void TMP_I2C_Init(void);
unsigned int TMP_I2C_Read_Register(unsigned char reg_addr);
void TMP_Get_Temp(unsigned int* ui16TempReturn, unsigned char* uc8NegFlagReturn, unsigned char uc8ModeFlag );
void Temp_Register_write(void);

extern unsigned int g_TempDataFahr;
extern unsigned int g_TempDataCel;
extern char g_TempNegFlagCel;
extern char g_TempNegFlagFahr;

#define TMP112_I2C_ADDR 	0x0048

//TMP112 Register Addresses
#define TMP112_TEMP_REG		0x00
#define TMP112_CONF_REG 	0x01
#define TMP112_TLOW_REG		0x02
#define TMP112_THIGH_REG 	0x03

//TMP112 Configuration Register Bits
#define TMP112_CONFIG_OS		BITF
#define TMP112_CONFIG_RES		(BITE | BITD)
#define TMP112_CONFIG_FAULT_0	0X00
#define TMP112_CONFIG_FAULT_1	BITB
#define TMP112_CONFIG_FAULT_2	BITC
#define TMP112_CONFIG_FAULT_3	(BITB | BITC)
#define TMP112_CONFIG_POL		BITA
#define TMP112_CONFIG_TM		BIT9
#define TMP112_CONFIG_SD		BIT8
#define TMP112_CONFIG_CR_0		0X00			//.25 Hz
#define TMP112_CONFIG_CR_1		BIT6			//  1 Hz
#define TMP112_CONFIG_CR_2		BIT7			//  4 Hz (default)
#define TMP112_CONFIG_CR_3		(BIT6 | BIT7)	//  8 Hz
#define TMP112_CONFIG_AL		BIT5
#define TMP112_CONFIG_EM		BIT4



#endif /* TMP103_H_ */
















