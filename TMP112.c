/*
 * {TMP112.c}
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


#include "tmp112.h"

signed char TemperatureData = 0;

void TMP_I2C_Init(void){
	UCB0CTL1 |= UCSWRST;	            			//Software reset enabled
	UCB0CTLW0 = UCMODE_3  + UCMST + UCSYNC + UCTR;	//I2C mode, Master mode, sync, transmitter
	UCB0CTLW0 |= UCSSEL_2;                    		// SMCLK = 8MHz

	UCB0BRW = 30; 									// Baudrate = SMLK/40 = 200kHz

	UCB0I2CSA  = TMP112_I2C_ADDR;					// Set Slave Address
	UCB0IE = 0;
	UCB0CTL1 &= ~UCSWRST;
}


unsigned int TMP_I2C_Read_Register(unsigned char reg_addr)
{
	unsigned int ui16ReturnData = 0;
	unsigned char ui8RxData[2];

	UCB0CTL1  |= UCSWRST;
	UCB0CTLW1 = UCASTP_2;  // generate STOP condition.
	UCB0TBCNT = 0x0001;
	UCB0CTL1  &= ~UCSWRST;

	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = reg_addr & 0xFF;
	while(!(UCB0IFG & UCBCNTIFG));

	UCB0CTL1 &= ~UCTR;

	UCB0CTLW1 = UCASTP_2;  			// generate STOP condition.
	UCB0TBCNT = 0x0002;
	UCB0CTL1 |= UCTXSTT; 			// Repeated start
	while(!(UCB0IFG & UCRXIFG0));
	ui8RxData[1] = UCB0RXBUF;
	while(!(UCB0IFG & UCRXIFG0));
	ui8RxData[0] = UCB0RXBUF;
	while (!(UCB0IFG & UCSTPIFG));  // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

	ui16ReturnData = (0x0FF0 & (ui8RxData[1] << 4)) | (0x0F & (ui8RxData[0] >> 4 ));

	return ui16ReturnData;
}


void TMP_Get_Temp(unsigned int* ui16TempReturn, unsigned char* uc8NegFlagReturn, unsigned char uc8ModeFlag ){

	unsigned long pTempData[2] = {0, 0};

	g_TempNegFlagFahr = 0;

	TMP_I2C_Init();
	pTempData[0] = TMP_I2C_Read_Register(TMP112_TEMP_REG);

	if(!(pTempData[0] & 0x800)){ // Sign bit.  If +
		g_TempDataCel = (pTempData[0] * 625)/1000;
		g_TempNegFlagCel = 0;
	}  //else is -
	else{
		pTempData[0] = (~pTempData[0]) & 0xFFF;
		g_TempDataCel = (pTempData[0] * 625)/1000;
		g_TempNegFlagCel = 1;
	}

	if(g_TempNegFlagCel){
		g_TempDataFahr  = 320 - ((g_TempDataCel * 9)/5);

		if(g_TempDataCel > 176){  // 176 == 17.6 C == 0 F
			g_TempNegFlagFahr = 1;
			g_TempDataFahr = (~g_TempDataFahr) + 1;
		}
	}
	else{
		g_TempDataFahr  = 320 + ((g_TempDataCel * 9)/5);
	}

	if(uc8ModeFlag){ // Celcius
		*ui16TempReturn = g_TempDataCel;
		*uc8NegFlagReturn = g_TempNegFlagCel;
	}
	else{	//Fahrenheit
		*ui16TempReturn = g_TempDataFahr;
		*uc8NegFlagReturn = g_TempNegFlagFahr;
	}
}

void Temp_Register_write(void){

}




























