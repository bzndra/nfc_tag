/*
 * {OPT3001.h}
 *
 * {Driver for OPT3001 Light sensor}
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

#ifndef OPT3001_H_
#define OPT3001_H_

#include "msp430.h"

void I2C_OPT_Init(void);
unsigned int I2C_OPT_Read_Register(unsigned int reg_addr);
void I2C_OPT_Write_Register(unsigned int reg_addr, unsigned int value);

#define OPTI2CADDR  0x44 // Address pin tied low
//#define OPTI2CADDR  0x45 // Address pin tied Hi
//#define OPTI2CADDR  0x46 // Address pin tied to SDA
//#define OPTI2CADDR  0x47 // Address pin tied to SCL

//Register definations
#define RESULTREG 		 0x00
#define CONFIGREG 	 	 0x01
#define LOWLIMITREG 	 0x02
#define HIGHLIMITREG 	 0x03
#define MANUFACTUREREG   0x7E
#define DEVICEID		 0x7F

#define DEVICEIDVALUE    0x3001
#define MANUFACTUREVALUE 0x5449

//Register Bits setting
//Configuration Register
#define FC0 0x0001 // fault count lsb
#define FC1 0x0002 // fault count msb - These two
#define ME	0x0004 // Mask expononent field
#define POL	0x0008 // Interrupt polarity field 0 = pulled low IRQ. 1 =pulled High IRQ
#define L	0x0010 // Latch Field
#define FL	0x0020 // Flag low field - Indicates the value is lower then a specefied level of interest
#define FH	0x0040 // Flag high field - Indicates the value is higher then a specefied level of interest
#define CRF	0x0080 // Conversion ready field
#define OVF	0x0100 // light overflow field
#define M0  0x0200 // conversion mode register
#define M1  0x0400
#define CT  0x0800 // Conversion time - 0 = 100ms, 1 = 800ms. Longer time yields lower noise.
#define RN0 0x1000
#define RN1 0x2000
#define RN2 0x4000
#define RN3 0x8000
#define RN_Auto RN3|RN2

// Modes of operation

#define SHUTDOWN 	0  // default
#define SINGLESHOT 	1
#define CONTINIOUS 	2

//Functional Prototypes
void SetConfigurationRegister(unsigned int configValue);
void SetComparisonLowLimit(unsigned int limitValue);
void SetComparisonHighLimit(unsigned int limitValue);
unsigned int GetManufactureID(void);
unsigned int GetDeviceID(void);
unsigned int GetConfigurationRegister(void);
void SetConversionMode(char mode);
unsigned int GetLuxValue(void);
void StartLuxMeasurement(void);

#endif /* OPT3001_H_ */
