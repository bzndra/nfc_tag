/*
 * {HDC1000.h}
 *
 * {Driver for HDC1000 Humidy sensor}
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

#include <msp430.h>

#ifndef HDC1000_H_
#define HDC1000_H_

typedef enum {
	TEMPERATURE = 1,
	HUMIDITY,
	BOTH
} HDC_MODE_t;

//DRDY Signal
#define HDC_DRDY_PIN		BIT7
#define HDC_DRDY_PORT_DIR	P4DIR
#define HDC_DRDY_PORT_IN	P4IN
#define HDC_DRDY_SET		(HDC_DRDY_PORT_DIR &= ~HDC_DRDY_PIN);
#define HDC_DRDY			(HDC_DRDY_PORT_IN & HDC_DRDY_PIN)

//HDC Registers
#define HDC_TEMPERATURE_REG		0X00
#define HDC_HUMIDITY_REG		0X01
#define HDC_CONFIG_REG			0X02
#define HDC_SERIAL_ID_1			0XFB
#define HDC_SERIAL_ID_2			0XFC
#define HDC_SERIAL_ID_3			0XFD
#define HDC_MANUFACTOR_ID		0XFE
#define HDC_DEVICE_ID			0XFF

//HDC Config Reg Bits
#define HDC_RST_BIT				BITF
#define HDC_HEAT_BIT			BITD
#define HDC_MODE_BIT			BITC
#define HDC_BTST_BIT			BITB
#define HDC_TRES_14_BIT			0x0000
#define HDC_TRES_11_BIT			BITA
#define HDC_HRES_14_BIT			0x0000
#define HDC_HRES_11_BIT			BIT8
#define HDC_HRES_8_BIT			BIT9

//HDC I2C Slave Address
#define HDC_I2C_SLAVE_ADDR 0x40
//#define HDC_I2C_SLAVE_ADDR 0x41
//#define HDC_I2C_SLAVE_ADDR 0x42
//#define HDC_I2C_SLAVE_ADDR 0x43

//HDC Functions/APIs
void HDC_Init(void);
void HDC_I2C_Write_Register(unsigned char ui8RegAddr, unsigned int ui16RegData);
void HDC_I2C_Trigger_Measurement(HDC_MODE_t t_Mode);
void HDC_I2C_Measurement(HDC_MODE_t t_Mode, unsigned char *pui8MeasurementDataBuf);

//Application Layer Example
void HDC_Get_Measurement(HDC_MODE_t t_Mode, unsigned int *pui8ResultsBuf);
unsigned int HDC_Get_Humidity(void);

#endif /* HDC1000_H_ */
