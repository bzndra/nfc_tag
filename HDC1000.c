/*
 * {HDC1000.c}
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

#include "HDC1000.h"

void HDC_Init(void){
	HDC_DRDY_SET;
	//configure eUSCI for I2C
	UCB0CTL1 |= UCSWRST;	            			//Software reset enabled
	UCB0CTLW0 |= UCMODE_3  + UCMST + UCSYNC + UCTR;	//I2C mode, Master mode, sync, transmitter
	UCB0CTLW0 |= UCSSEL_2;                    		// SMCLK = 8MHz

	UCB0BRW = 30; 						// Baudrate = SMLK/80 = 100kHz

	UCB0I2CSA  = HDC_I2C_SLAVE_ADDR;				//slave address - determined by pins E0, E1, and E2 on the RF430CL330H
	UCB0CTL1  &= ~UCSWRST;
}

void HDC_I2C_Write_Register(unsigned char ui8RegAddr, unsigned int ui16RegData)
{
	UCB0CTLW1 = UCASTP_1;  			// Send Stop Cond after 3 bytes (1 Reg addr, and 2 RegData bytes)
	UCB0TBCNT = 0x0003;
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation

	while(!(UCB0IFG & UCTXIFG0));	//write the address
	UCB0TXBUF = ui8RegAddr;

	while(!(UCB0IFG & UCTXIFG0));	//write the data
	UCB0TXBUF = ui16RegData >> 8;

	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = ui16RegData & 0xFF;

	while (!(UCB0IFG & UCBCNTIFG));
	UCB0CTL1 |= UCTXSTP;
	while (!(UCB0IFG & UCSTPIFG)); 	// Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;
}

void HDC_I2C_Trigger_Measurement(HDC_MODE_t t_Mode)
{
	unsigned char ui8RegByte = 0;

	if(t_Mode ==  HUMIDITY){
		ui8RegByte = HDC_HUMIDITY_REG;
	}
	else{
		ui8RegByte = HDC_TEMPERATURE_REG;
	}

	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = 1;
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation

	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = ui8RegByte;
	while(!(UCB0IFG & UCTXIFG0));

	UCB0CTL1 &= ~UCTR; 				// I2C read operation
	UCB0CTL1 |= UCTXSTT;

	while(!(UCB0IFG & UCNACKIFG));

	UCB0CTL1  |= UCSWRST;

}

void HDC_I2C_Measurement(HDC_MODE_t t_Mode, unsigned char *pui8MeasurementData)
{
	unsigned char ui8ModeByteCount;
	unsigned int i = 0;

	if(t_Mode == BOTH){
		ui8ModeByteCount = 3;
	}
	else{		// Just one measurement, either Temp or Humidity
		ui8ModeByteCount = 1;
	}

	UCB0CTL1  |= UCSWRST;
	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = 0x0001;
	UCB0CTL1  &= ~UCSWRST;

	UCB0CTL1 &= ~UCTR; 				// I2C read operation
	UCB0CTL1 |= UCTXSTT; 			// Repeated start
	for(i=0; i<ui8ModeByteCount; i++){
		while(!(UCB0IFG & UCRXIFG0));
		*pui8MeasurementData++ = UCB0RXBUF;
	}

	UCB0CTLW0 |= UCTXSTP; 			// Send stop after next RX
	while(!(UCB0IFG & UCRXIFG0));
	*pui8MeasurementData++ = UCB0RXBUF;
	while (!(UCB0IFG & UCSTPIFG));  // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;
}

void HDC_Get_Measurement(HDC_MODE_t t_Mode, unsigned int *pui8ResultsBuf)
{
	unsigned char ui8MeasurementDataBuf[4], i=0;
	unsigned long ui32MathBuffer;

	HDC_Init();

	if(t_Mode == BOTH){						// Configure HDC1000
		HDC_I2C_Write_Register(HDC_CONFIG_REG, HDC_MODE_BIT|HDC_TRES_14_BIT|HDC_HRES_14_BIT);
	}
	else{
		HDC_I2C_Write_Register(HDC_CONFIG_REG, HDC_TRES_14_BIT|HDC_HRES_14_BIT);
	}

	HDC_I2C_Trigger_Measurement(t_Mode);	//trigger data capture

	while(HDC_DRDY);  						// Active low Data Ready signal

	HDC_I2C_Measurement(t_Mode, ui8MeasurementDataBuf);		// Get Data

	//Convert Data
	_nop();
	_nop();

	switch(t_Mode){
	case BOTH:
	case TEMPERATURE:
		ui32MathBuffer = ui8MeasurementDataBuf[i] << 8 | ui8MeasurementDataBuf[i+1];
		ui32MathBuffer = ui32MathBuffer * 165;
		ui32MathBuffer = ui32MathBuffer >> 16;
		*pui8ResultsBuf++ = ui32MathBuffer - 40;

		if(t_Mode != BOTH){
			break;
		}
		else{
			i += 2;
		}

	case HUMIDITY:
		ui32MathBuffer = ui8MeasurementDataBuf[i] << 8 | ui8MeasurementDataBuf[i+1];
		*pui8ResultsBuf = (ui32MathBuffer * 100) >> 16;
		break;
	}

}

unsigned int HDC_Get_Humidity(void)
{
	unsigned char ui8MeasurementDataBuf[2];
	unsigned long ui32MathBuffer = 0;

	HDC_Init();

	HDC_I2C_Write_Register(HDC_CONFIG_REG, HDC_TRES_14_BIT|HDC_HRES_14_BIT);

	HDC_I2C_Trigger_Measurement(HUMIDITY);	//trigger data capture

	while(HDC_DRDY);  						// Active low Data Ready signal

	HDC_I2C_Measurement(HUMIDITY, ui8MeasurementDataBuf);		// Get Data

	//Convert Data
	_nop();
	_nop();

	ui32MathBuffer = ((ui8MeasurementDataBuf[0] << 8) | ui8MeasurementDataBuf[1]) & 0x0000FFFF;

	_nop();

	ui32MathBuffer = (ui32MathBuffer * 100) >> 16;

	return (unsigned int)ui32MathBuffer;
}



