/*
 * {OPT3001.c}
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

#include "OPT3001.h"

const float LSBValues[12] = {0.01, 0.02, 0.04, 0.08, 0.16, 0.32, 0.64,1.28,2.56,5.12,10.24,20.48};

void SetConfigurationRegister(unsigned int configValue)
{
	I2C_OPT_Write_Register(CONFIGREG,configValue);
}


void SetComparisonLowLimit(unsigned int limitValue)
{
	I2C_OPT_Write_Register(LOWLIMITREG,limitValue);
}


void SetComparisonHighLimit(unsigned int limitValue)//Should return 0x5449 or 21577 in decimal
{
	I2C_OPT_Write_Register(HIGHLIMITREG,limitValue);
}


unsigned int GetManufactureID(void)
{
	return I2C_OPT_Read_Register(MANUFACTUREREG); //Should return 0x3001h or 12289 in decimal
}

unsigned int GetDeviceID(void)
{
	return I2C_OPT_Read_Register(DEVICEID);
}

unsigned int GetConfigurationRegister(void)
{
	return I2C_OPT_Read_Register(CONFIGREG);
}

void SetConversionMode(char mode)
{
	unsigned int tempValue = GetConfigurationRegister();
	tempValue &= ~(M0 + M1); // clear the mode set bits
	switch(mode)
	{
		case SHUTDOWN:
			break;
		case SINGLESHOT:
			tempValue |= M0;
			break;
		case CONTINIOUS:
			tempValue |= M1;
			break;
		default:
			break;
	}
	SetConfigurationRegister(tempValue);
}

// this assumes that exponent values are enabeled.
void StartLuxMeasurement(void)
{
	I2C_OPT_Init();

	I2C_OPT_Write_Register(CONFIGREG, M0 | RN_Auto );  // Single Conversion, auto scaled
}

unsigned int GetLuxValue(void)
{
	unsigned int tempResult = I2C_OPT_Read_Register(RESULTREG);
	unsigned int expononentValue = tempResult &0xF000; // expononent is stored in the upper 4 bits
	unsigned int fractionalValue = tempResult & 0x0FFF; // fractional value is stored in the bottom 12 bits
	float LSBSize = LSBValues[(expononentValue >>12)];	// get the LSB value in the look up table
	float luxValue = LSBSize * (float)fractionalValue;

	//Edge Cases
	if(luxValue > 65535){
		luxValue = 65534;
	}

	if(luxValue < 1){
		luxValue = 0;
	}

	return (unsigned int)luxValue;
}

void I2C_OPT_Init(void)
{
	UCB0CTL1 = UCSWRST;	            			//Software reset enabled
	UCB0CTLW1 = 0;
	UCB0CTLW0 = UCMODE_3  + UCMST + UCSYNC;	//I2C mode, Master mode, sync, transmitter
	UCB0CTLW0 |= UCSSEL_2;                    		// SMCLK = 8MHz

	UCB0BRW = 30; 									// Baudrate = SMLK/40 = 200kHz

	UCB0I2CSA  = 0x0044;					// Set Slave Address
	UCB0CTL1  &= ~UCSWRST;
	UCB0IE = 0;

}

// Reads the register at reg_addr, returns the result
unsigned int I2C_OPT_Read_Register(unsigned int reg_addr)
{
	unsigned char ui8RxData[2] = {0,0};
	unsigned char ui8TxAddr[4] = {0,0};

	ui8TxAddr[1] = reg_addr & 0xFF;

	UCB0CTL1  |= UCSWRST;
	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = 0x0001;
	UCB0CTL1  &= ~UCSWRST;

	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation

	UCB0TXBUF = ui8TxAddr[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = ui8TxAddr[1];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0CTL1 &= ~UCTR; 				// I2C read operation
	UCB0CTL1 |= UCTXSTT; 			// Repeated start
	while(!(UCB0IFG & UCRXIFG0));
	ui8RxData[0] = UCB0RXBUF;
	UCB0CTLW0 |= UCTXSTP; 			// Send stop after next RX
	while(!(UCB0IFG & UCRXIFG0));
	ui8RxData[1] = UCB0RXBUF;
	while (!(UCB0IFG & UCSTPIFG));  // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

	return ui8RxData[0] << 8 | ui8RxData[1];
}

//writes the register at reg_addr with value
void I2C_OPT_Write_Register(unsigned int reg_addr, unsigned int value)
{
	unsigned char ui8TxData[4] = {0,0,0,0};
	unsigned char ui8TxAddr[4] = {0,0};

	ui8TxAddr[0] = reg_addr & 0xFF; 		// MSB of address
	ui8TxData[0] = value >> 8;
	ui8TxData[1] = value & 0xFF;

	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = 0x0003;
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation
	//write the address
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = ui8TxAddr[0];
	//write the data
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = ui8TxData[0];

	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = ui8TxData[1];

	while (!(UCB0IFG & UCBCNTIFG));
	UCB0CTL1 |= UCTXSTP;
	while (!(UCB0IFG & UCSTPIFG)); 	// Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;
}

