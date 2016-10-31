/*
 * {mcu.h}
 *
 * {Definitions for MCU specific pins and functions}
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

#ifndef MCU_H_
#define MCU_H_

#include "stdio.h"

#include "msp430.h"
#include "RF430CL331H.h"
#include "RF430_Request_Processor.h"
#include "tmp112.h"
#include "opt3001.h"
#include "hdc1000.h"
#include "Datalogger.h"
#include "TIME.h"
#include "RF430_I2C.h"
#include "RTC.h"

#define SENSOR_POWER_ON		P2DIR |= BIT6; P2OUT |= BIT6;
#define SENSOR_POWER_OFF 	P2OUT &= ~BIT6; P2DIR &= ~BIT6; P2REN |= BIT6; PORT_RST_OUT &= ~RST;
#define SENSOR_POWER_DIR	P2DIR
#define SENSOR_POWER_PIN	BIT6

#define RF_FIELD_PIN		BIT5
#define RF_FIELD_DIR		P2DIR
#define RF_FIELD_IFG		P2IFG
#define RF_FIELD_IES		P2IES
#define RF_FIELD_IE			P2IE
#define RF_FIELD_INT_ENABLE		RF_FIELD_IE |= RF_FIELD_PIN;
#define RF_FIELD_INT_DISABLE	RF_FIELD_IE &= ~RF_FIELD_PIN;


unsigned char gui8RfFieldDetected = 0;
unsigned char gui8Rf430Initalized = 0;

void Timer_Init();
void Low_Power_Delay_ms(unsigned int ms);
void Low_Power_Delay_secs(unsigned int secs);
void Update_Time_Counts(void);
void MSP430_Init();
void ResetRF430(void);
void RF430Patch(void);

typedef enum {
	Start,
	Power_Lost,
	Data_logger_Init,
	Wait_For_Command,
	Process_command,
} States_t;


const unsigned char RF430_DEFAULT_DATA[]	=	{  												\
		/*NDEF Tag Application Name*/ 															\
		0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 												\
																								\
		/*Capability Container ID*/ 															\
		0xE1, 0x03, 																			\
		0x00, 0x0F,	/* CCLEN */																	\
		0x20,		/* Mapping version 2.0 */													\
		0x00, 0xF9,	/* MLe (49 bytes); Maximum R-APDU data size */								\
		0x00, 0xF6, /* MLc (52 bytes); Maximum C-APDU data size */								\
		0x04, 		/* Tag, File Control TLV (4 = NDEF file) */									\
		0x06, 		/* Length, File Control TLV (6 = 6 bytes of data for this tag) */			\
		0xE1, 0x04,	/* File Identifier */														\
		0x0B, 0xDF, /* Max NDEF size (3037 bytes of useable memory) */							\
		0x00, 		/* NDEF file read access condition, read access without any security */		\
		0x00, 		/* NDEF file write access condition; write access without any security */	\
																								\
		/* NDEF File ID */ 																		\
		0xE1, 0x04, 																			\
																								\
		/* NDEF File for Hello World  (48 bytes total length) */								\
		0x00, 0x33, /* 0x39 NLEN; NDEF length (3 byte long message) */ 							\
		0xC1, /* Record Header	*/																\
		0x01, /* Type Length	*/																\
		0x00, 0x00, 0x00, 0x2C,  /* Payload length This NLEN - 7 	*/				    		\
		0x54, /* T = text */																	\
		0x02, /* ID length	*/																	\
		0x65, 0x6E, /* 'e', 'n', ID */															\
																								\
		/* Payload	*/																			\
																								\
		/* 'Temperature: '*/																	\
		0x54, 0x65, 0x6d, 0x70, 0x65, 0x72, 0x61, 0x74, 0x75, 0x72, 0x65, 0x3a, 0x20, 			\
		/* ' 80.0 F' */																			\
		0x20, 0x38, 0x30, 0x2E, 0x30, 0x20, 0x46, 0x0A, /* 0x0D, */ 							\
		/* 'Temperature: ' */																	\
		0x54, 0x65, 0x6d, 0x70, 0x65, 0x72, 0x61, 0x74, 0x75, 0x72, 0x65, 0x3a, 0x20, 			\
		/* ' 40.0 C' */																			\
		0x20, 0x34, 0x30, 0x2E, 0x30, 0x20, 0x43												\
};

#pragma NOINIT (p_ucNdefDataBuffer);
unsigned char p_ucNdefDataBuffer[1500]; // FRAM container for holding the NDEF Message

#pragma NOINIT (p_u8NDEF_Header);
unsigned char p_u8NDEF_Header[40];

#pragma NOINIT (g_u16NdefLength);
unsigned int g_u16NdefLength;

#pragma NOINIT (g_u16NdefDataPointer);
unsigned int g_u16NdefDataPointer;

#pragma NOINIT (g_u8MaxLengthFlag);
unsigned char g_u8MaxLengthFlag;

unsigned char read_data[100];

States_t State_t = Start;

#pragma NOINIT (g_Datalogger_Mode_t);
Datalogger_Modes_t g_Datalogger_Mode_t;

unsigned char g_uc8SleepBetweenLoopsFlag = 1; 	// Used to skip sleep state to quickly move between states
unsigned char g_uc8DataloggingEnableFlag = 0;	//Used to start/stop datalogging

#pragma NOINIT (Command_Received_t);
Datalogger_Commands_t Command_Received_t;

#pragma NOINIT (g_ui8ResetStateFlag);
unsigned char g_ui8ResetStateFlag;

#pragma NOINIT (g_ui8TemperatureModeFlag);
Temp_Modes_t g_ui8TemperatureModeFlag;

unsigned char ui8TemperatureNegFlag;

// Polling interval in Minutes
#pragma NOINIT (g_ui8PollingInterval);
unsigned char g_ui8PollingInterval;

unsigned char uc8ReceivedData[3];

unsigned int Sample_Rate = 0X10; // 10 Seconds NOW IN HEX

#endif /* MCU_H_ */








