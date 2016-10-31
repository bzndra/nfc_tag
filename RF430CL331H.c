/*
 * {RF430CL331H.c}
 *
 * {Contains low level functions for interfacing the RF430CL331}
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

#include "RF430CL331H.h"

unsigned char RxData[2] = {0,0};
unsigned char TxData[2] = {0,0};
unsigned char TxAddr[2] = {0,0};


unsigned char patch_looping[] =  {  0xB2, 0xF0, 0xFF, 0xFB, 0x00, 0x07, 0xB2, 0xF0, 0xFF, 0xFD, 0x00, 0x07, 0xA2, 0xC3, 0x00, 0x07,
									0x08, 0x3C, 0xB2, 0xF0, 0xFF, 0xFB, 0x00, 0x07, 0xB2, 0xF0, 0xFF, 0xFD, 0x00, 0x07, 0xA2, 0xC3,
									0x00, 0x07, 0xB2, 0xB0, 0x04, 0x04, 0x00, 0x07, 0xF4, 0x23, 0x30, 0x41, 0x0F, 0x12, 0x0E, 0x12,
									0xB2, 0xB0, 0x00, 0x01, 0x00, 0x07, 0x08, 0x24, 0x3E, 0x40, 0x1D, 0x00, 0x1F, 0x42, 0xB2, 0x2A,
									0x6E, 0x9F, 0x02, 0x20, 0x92, 0x43, 0x2E, 0x2A, 0x3E, 0x41, 0x3F, 0x41, 0x30, 0x41};


void RF430CL331H_Init(void)
{
	CL331H_I2C_Init();
	// DATA_TRANSACTION_INT_ENABLE - general NDEF interrupt - should always be enabled
	// EXTRA_DATA_IN_INT_ENABLE - enables the read pre-fetch option, code in this project can handle this if enabled,
	//								speeds up read speeds 10-30% depending on mobile
	// FIELD_REMOVED_INT_ENABLE - sends an interrupt if the RF430 has been removed from the RF field
	// GENERIC_ERROR_INT_ENABLE - this will send an interrupt in some cases of RF430 resets
	CL331H_Write_Register(INT_ENABLE_REG, DATA_TRANSACTION_INT_ENABLE + FIELD_REMOVED_INT_ENABLE + GENERIC_ERROR_INT_ENABLE);


    //DUPLEX_ON_WRITE - automatically acks UpdateBinary requests - speeds up write speeds 10-30% depending on mobile
	CL331H_Write_Register(CONTROL_REG,  INT_ENABLE + INTO_DRIVE + INTO_HIGH + RF_ENABLE);

	// sets the command timeouts, generally should stay at these settings
	CL331H_Write_Register(SWTX_INDEX, 0x3B);
	CL331H_Write_Register(TIMER_DELAY, 300);

	//prepare interrupt detection
	PORT_INTO_IFG &= ~INTO; //clear any pending flags
	PORT_INTO_IE |= INTO;	//enable interrupt
}

void ResetRF430(void){

	CL331H_I2C_Init();

	//RST RF430 (in case board is still powered but the MSP430 reset for some reason - MSP430 RST button pushed for example)
	PORT_RST_SEL0 &= ~RST;
	PORT_RST_SEL1 &= ~RST;
	PORT_RST_DIR |= RST;

	//configure pin for INTO interrupts
	PORT_INTO_DIR &= ~INTO; 		//input
	PORT_INTO_OUT &= ~INTO; 		//output low for pullup
	PORT_INTO_REN |= INTO; 			//internal pullup resistor  (external pull down on this board)
	PORT_INTO_IFG &= ~INTO; 		//clear interrupt flag
	PORT_INTO_IES &= ~INTO; 		//fire interrupt on low-to-high transition since INTO is setup High

	//Setup the CL331H I2C Ready input
	CL331H_READY_DIR &= ~CL331H_READY_PIN;		//Set I2C_READY singal to input on MSP430

	//Setup the CL331H I2C Stop input
	CL331H_STOP_DIR &= ~CL331H_STOP_PIN;			//Set I2C_READY singal to input on MSP430

	//RST RF430: This clears NDEF Message, sets RF disabled, and resets PUPI
	PORT_RST_OUT &= ~RST; 				//RF430CL330H device in reset
	Low_Power_Delay_ms(1);
	PORT_RST_OUT |= RST; 				//release the RF430CL330H

	Low_Power_Delay_ms(5);

	while(!(CL331H_Read_Register(STATUS_REG) & READY)); //wait until READY bit has been set

	RF430Patch();						// Patch Errata
}

void RF430Patch(void){
	//Correction Patch.  Fixes some Errata

	//Add to initial section after status byte returns okay
	CL331H_Write_Register(TEST_MODE_REG, TEST_MODE_KEY);   	//unlock test mode
	CL331H_Write_Register(CONTROL_REG, TEST430_ENABLE);    	//enable test mode, now have to use actual addresses
	Write_Continuous (0x2AD0, (unsigned char *) &patch_looping, sizeof(patch_looping));		//write the patch to memory
	CL331H_Write_Register(0x2A90, 0x2AFC); 	            	//looping patch (at ISO_ISR)
	CL331H_Write_Register(0x2AAE, 0x2AD0); 	            	//wait time extension patch (at send swtx)
	CL331H_Write_Register(0x2A66, 0x0000); 		        	//fix by init
	CL331H_Write_Register(0x27B8, 0);                      	//exit test mode (CONTROL_REG is at real address 0x2814)
	CL331H_Write_Register(TEST_MODE_REG, 0);               	//lock test reg
}

void RF430UpdateLength( unsigned char Length)
{
	if(!g_u8MaxLengthFlag){
		g_u16NdefLength += Length;
	}

	FileTextE104[NDEF_CC_LENGTH_POINTER]   = 0x00FF & (g_u16NdefLength >> 8);	// Updating NLEN in Record Header
	FileTextE104[NDEF_CC_LENGTH_POINTER+1] = 0x00FF & g_u16NdefLength;
	FileTextE104[NDEF_CC_LENGTH_POINTER+6] = 0x00FF & ((g_u16NdefLength-7) >> 8);
	FileTextE104[NDEF_CC_LENGTH_POINTER+7] = 0x00FF & (g_u16NdefLength-7);
}

void CL331H_I2C_Init(){
	UCB0CTL1 = UCSWRST;	            			//Software reset enabled
	UCB0CTLW1 = 0;
	UCB0CTLW0 = UCMODE_3  + UCMST + UCSYNC;	//I2C mode, Master mode, sync, transmitter
	UCB0CTLW0 |= UCSSEL_2;                    		// SMCLK = 8MHz

	UCB0BRW = 30; 									// Baudrate = SMLK/40 = 200kHz

	UCB0I2CSA  = RF430_I2C_SLAVE_ADDR;					// Set Slave Address
	UCB0CTL1  &= ~UCSWRST;
	UCB0IE |= UCTXIE + UCRXIE + UCSTPIE + UCNACKIE;
}

// Reads the register at reg_addr, returns the result
unsigned int CL331H_Read_Register(unsigned int reg_addr)
{
	unsigned char ui8TxAddr[2] = {0,0};
    unsigned char ui8RxData[2] = {0,0};
	unsigned int ui16ReturnData;

	CL331H_I2C_Init();

	while(!CL331H_READY);		//wait until it is safe to transmit
	__no_operation();

	ui8TxAddr[0] = reg_addr >> 8; 		// MSB of address
	ui8TxAddr[1] = reg_addr & 0xFF; 	// LSB of address

	if(RF430_I2C_Write_Restart_Read((unsigned char *)ui8TxAddr, 2, (unsigned char *)ui8RxData, 2) != FAIL){
		ui16ReturnData = ui8RxData[1] << 8 | ui8RxData[0];
	}
	else{
		__no_operation();
	}
	return ui16ReturnData;
}

//reads the register at reg_addr, returns the result
unsigned int Read_Register_BIP8(unsigned int reg_addr)
{
	CL331H_I2C_Init();

	unsigned char BIP8 = 0;
	TxAddr[0] = reg_addr >> 8; 		// MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	// LSB of address

	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = 0x0002;
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation

	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[0];
	BIP8 ^= TxAddr[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[1];
	BIP8 ^= TxAddr[1];

	while(!(UCB0IFG & UCBCNTIFG));
	UCB0CTL1 &= ~UCTR; 				// I2C read operation
	UCB0CTL1 |= UCTXSTT; 			// Repeated start

	while(!(UCB0IFG & UCRXIFG0));
	RxData[0] = UCB0RXBUF;
	BIP8 ^= RxData[0];
	while(!(UCB0IFG & UCRXIFG0));
	RxData[1] = UCB0RXBUF;
	BIP8 ^= RxData[1];

	UCB0CTLW0 |= UCTXSTP; 			// Send stop after next RX
	while(!(UCB0IFG & UCRXIFG0));	// Receive BIP8
	if(BIP8 != UCB0RXBUF)			// Compare to known value

		__no_operation();			// Breakpoint encase BIP8 doesn't match

	while (!(UCB0IFG & UCSTPIFG));  // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

	return RxData[0] << 8 | RxData[1];
}

//Continuous read data_length bytes and store in the area "read_data"
void Read_Continuous(unsigned int reg_addr, unsigned char* read_data, unsigned int data_length)
{

	CL331H_I2C_Init();

	TxAddr[0] = reg_addr >> 8; 		// MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	// LSB of address

	while(!CL331H_READY);		//wait until it is safe to transmit

	RF430_I2C_Write_Restart_Read((unsigned char  *)TxAddr, 2, (unsigned char *)read_data, data_length);

	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent

}

//writes the register at reg_addr with value
void CL331H_Write_Register(unsigned int reg_addr, unsigned int value)
{
	unsigned char ui8TxData[4] = {0,0,0,0};

	CL331H_I2C_Init();

	ui8TxData[0] = reg_addr >> 8; 		// MSB of address
	ui8TxData[1] = reg_addr & 0xFF; 	// LSB of address
	ui8TxData[2] = value & 0xFF;
	ui8TxData[3] = value >> 8;

	while(!CL331H_READY);		//wait until it is safe to transmit

	RF430_I2C_Write((unsigned char  *)ui8TxData, 4, START);

	RF430_I2C_Send_Stop();

	{
		unsigned int delay = 10;
		while ((UCB0CTL1 & UCTXSTP) && delay)             // Ensure stop condition got sent, added timeout due to infinite loop when it should not be  AK 10-27-2013
		{
			__delay_cycles(10);
			delay--;
		}
	}
}


//writes the register at reg_addr with value
void Write_Register_BIP8(unsigned int reg_addr, unsigned int value)
{
	unsigned char BIP8 = 0;

	CL331H_I2C_Init();

	TxAddr[0] = reg_addr >> 8; 		//MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	//LSB of address
	TxData[0] = value >> 8;
	TxData[1] = value & 0xFF;

	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = 0x0005;
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		//start i2c write operation

	//write the address
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[0];
	BIP8 ^= TxAddr[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[1];
	BIP8 ^= TxAddr[1];

	//write the data
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxData[0];
	BIP8 ^= TxData[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxData[1];
	BIP8 ^= TxData[1];

	//send BIP8 byte
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = BIP8;

	while (!(UCB0IFG & UCBCNTIFG));
	UCB0CTL1 |= UCTXSTP;
	while (!(UCB0IFG & UCSTPIFG));     // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

}

//writes the register at reg_addr and incrementing addresses with the data at "write_data" of length data_length
void Write_Continuous(unsigned int reg_addr, unsigned char* write_data, unsigned int data_length)
{
	CL331H_I2C_Init();

    while(!CL331H_READY);		//wait until it is safe to transmit

    TxAddr[0] = reg_addr >> 8; 		//MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	//LSB of address

    RF430_I2C_Write((unsigned char *)TxAddr, 2, START);

    RF430_I2C_Write((unsigned char  *)write_data, data_length, CONTINUE);

    RF430_I2C_Send_Stop();

    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
}
