/*
 * {RF430CL331H.h}
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

#ifndef RF430_EXAMPLE_H_
#define RF430_EXAMPLE_H_

#include "msp430.h"
#include "RF430_I2C.h"
#include "TIME.h"
#include "RF430_Request_Processor.h"

//void Rf430_I2C_Init(void);
unsigned int CL331H_Read_Register(unsigned int reg_addr);
unsigned int Read_Register_BIP8(unsigned int reg_addr);
void Read_Continuous(unsigned int reg_addr, unsigned char* read_data, unsigned int data_length);

void CL331H_Write_Register(unsigned int reg_addr, unsigned int value);
void Write_Continuous(unsigned int reg_addr, unsigned char* write_data, unsigned int data_length);
void Write_Register_BIP8(unsigned int reg_addr, unsigned int value);

void CL331H_I2C_Init();
void RF430CL331H_Init(void);
void RF430UpdateLength( unsigned char Length);

void ResetRF430(void);
void RF430Patch(void);

extern unsigned char p_ucNdefDataBuffer[1500]; // FRAM container for holding the NDEF Message (and data logging)  // 68 22 byte samples
extern unsigned char p_u8NDEF_Header[40];
extern unsigned int g_u16NdefDataPointer;
extern unsigned int g_u16NdefLength;
extern unsigned char g_u8MaxLengthFlag;

extern unsigned char* gp_StartOfDataloggingMemory;  // TODO can be extended lower to creat more memory space
extern unsigned char* gp_EndOfDataloggingMemory;
extern unsigned char* gp_StartOfVectorTable;
extern unsigned char* gp_EndOfVectorTable;

extern unsigned char* gp_StartOfData;
extern unsigned char* gp_EndOfData;
extern unsigned int gp_TotalDataLength;


#define NDEF_CC_LENGTH_POINTER 0
#define NDEF_CC_TOTAL_LENGTH 38
#define NDEF_DATA_POINTER_START_LOCATION 12

#define RF430_CORRECTION_LENGTH 112

extern unsigned int g_TempDataFahr;
extern unsigned int g_TempDataCel;
extern char g_TempNegFlagCel;
extern char g_TempNegFlagFahr;

extern unsigned char g_u8Seconds;
extern unsigned char g_u8Minutes;
extern unsigned char g_u8Hours;
extern unsigned char g_u8Days;


//INTO
#define PORT_INTO_IN	P2IN
#define PORT_INTO_OUT	P2OUT
#define PORT_INTO_DIR	P2DIR
#define PORT_INTO_SEL0	P2SEL0
#define PORT_INTO_SEL1	P2SEL1
#define PORT_INTO_REN	P2REN
#define PORT_INTO_IE	P2IE
#define PORT_INTO_IES	P2IES
#define PORT_INTO_IFG	P2IFG
#define INTO			BIT2

//RST
#define PORT_RST_OUT	P4OUT
#define PORT_RST_DIR	P4DIR
#define PORT_RST_SEL0	P4SEL0
#define PORT_RST_SEL1
#define RST	BIT4

//Buttons
#define PORT_BUTTON_IN		P4IN
#define PORT_BUTTON_OUT		P4OUT
#define PORT_BUTTON_DIR 	P4DIR
#define PORT_BUTTON_SEL0	P4SEL0
#define PORT_BUTTON_SEL1	P4SEL1
#define PORT_BUTTON_REN		P4REN
#define PORT_BUTTON_IE		P4IE
#define PORT_BUTTON_IES		P4IES
#define PORT_BUTTON_IFG		P4IFG
#define S1	BIT0
#define S2	BIT1

// RF430 I2C READY
#define CL331H_READY_IN	P3IN
#define CL331H_READY_OUT	P3OUT
#define CL331H_READY_DIR	P3DIR
#define CL331H_READY_SEL0	P3SEL0
#define CL331H_READY_SEL1	P3SEL1
#define CL331H_READY_REN	P3REN
#define CL331H_READY_IE	P3IE
#define CL331H_READY_IES	P3IES
#define CL331H_READY_IFG	P3IFG
#define CL331H_READY_PIN	BIT4
#define CL331H_READY		(CL331H_READY_IN & CL331H_READY_PIN)

#define CL331H_STOP_DIR	P3DIR
#define CL331H_STOP_PIN	BIT5

//LEDs
#define PORT_LED_OUT	P4OUT
#define PORT_LED_DIR	P4DIR
#define LED_ORANGE		BIT5
#define LED_RED			BIT6


//define the values for Granite's registers we want to access
#define CONTROL_REG 			0xFFFE
#define STATUS_REG				0xFFFC
#define INT_ENABLE_REG			0xFFFA
#define INT_FLAG_REG			0xFFF8
#define CRC_RESULT_REG			0xFFF6
#define CRC_LENGTH_REG			0xFFF4
#define CRC_START_ADDR_REG		0xFFF2
#define COMM_WD_CTRL_REG		0xFFF0
#define VERSION_REG				0xFFEE //contains the software version of the ROM
#define NDEF_FILE_ID			0xFFEC
#define HOST_RESPONSE			0xFFEA
#define NDEF_FILE_LENGTH		0xFFE8
#define NDEF_FILE_OFFSET		0xFFE6
#define NDEF_BUFFER_START		0xFFE4
#define TEST_FUNCTION_REG   	0xFFE2
#define TEST_MODE_REG			0xFFE0
#define SWTX_INDEX				0xFFDE
#define TIMER_DELAY				0xFFDC
#define CUSTOM_RESPONSE_REG		0xFFDA

//define the different virtual register bits
//CONTROL_REG bits
#define SW_RESET		BIT0
#define RF_ENABLE		BIT1
#define INT_ENABLE		BIT2
#define INTO_HIGH		BIT3
#define INTO_DRIVE		BIT4
#define BIP8_ENABLE		BIT5
#define STANDBY_ENABLE	BIT6
#define TEST430_ENABLE	BIT7

//STATUS_REG bits
#define READY			BIT0
#define CRC_ACTIVE		BIT1
#define RF_BUSY			BIT2

#define APP_STATUS_REGS			BIT4 + BIT5 + BIT6
#define FILE_SELECT_STATUS		BIT4
#define FILE_REQUEST_STATUS		BIT5
#define FILE_AVAILABLE_STATUS	BIT5 + BIT4

//INT_ENABLE_REG bits
#define EOR_INT_ENABLE		BIT1
#define EOW_INT_ENABLE		BIT2
#define CRC_INT_ENABLE		BIT3
#define BIP8_ERROR_INT_ENABLE		BIT4
#define DATA_TRANSACTION_INT_ENABLE	BIT5
#define GENERIC_ERROR_INT_ENABLE	BIT7
#define FIELD_REMOVED_INT_ENABLE 	BIT6
#define EXTRA_DATA_IN_INT_ENABLE	BIT8

//INT_FLAG_REG bits
#define EOR_INT_FLAG				BIT1
#define EOW_INT_FLAG				BIT2
#define CRC_INT_FLAG				BIT3
#define BIP8_ERROR_INT_FLAG			BIT4
#define DATA_TRANSACTION_INT_FLAG	BIT5
#define FIELD_REMOVED_INT_FLAG	 	BIT6
#define GENERIC_ERROR_INT_FLAG		BIT7
#define EXTRA_DATA_IN_FLAG			BIT8


//COMM_WD_CTRL_REG bits
#define WD_ENABLE	BIT0
#define TIMEOUT_PERIOD_2_SEC	0
#define TIMEOUT_PERIOD_32_SEC	BIT1
#define TIMEOUT_PERIOD_8_5_MIN	BIT2
#define TIMEOUT_PERIOD_MASK		BIT1 + BIT2 + BIT3

#define TEST_MODE_KEY 0x004E


//Host response index
#define INT_SERVICED_FIELD 			BIT0
#define FILE_EXISTS_FIELD			BIT1
#define CUSTOM_RESPONSE_FIELD		BIT2
#define EXTRA_DATA_IN_SENT_FIELD	BIT3
#define FILE_DOES_NOT_EXIST_FIELD	0


#define RF430_I2C_SLAVE_ADDR 0x0018


#endif /* RF430_EXAMPLE_H_ */
