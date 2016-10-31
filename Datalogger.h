/*
 * {Datalogger.h}
 *
 * {Controllers Datalogging Memory Flow and APIs}
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

#ifndef DATALOGGER_H_
#define DATALOGGER_H_

#include "msp430.h"

typedef enum {
	Temp_Only,
	Temp_and_Light,
	Temp_and_Humidity,
	Temp_Light_and_Humidity
} Datalogger_Modes_t;

typedef enum {
	Fahrenheit = 0,
	Celcius
} Temp_Modes_t;

extern Temp_Modes_t g_ui8TemperatureModeFlag;

extern unsigned char ui8TemperatureNegFlag;

void DL_Reset(void);
void DL_Set_Record_Header(unsigned char* p_ui8RecordHeader);
void DL_Update_Length(unsigned int ui16AddLength);
void DL_Update_Data_Pointers(unsigned int ui16DataLength);
void DL_Update_Data(unsigned char* p_uc8DataToBeStored, unsigned int ui16DataLength);
unsigned char DL_Update(Datalogger_Modes_t Mode, unsigned int ui16Temperature_F, unsigned int ui16Light, unsigned int ui16Humidity );
void DL_Print_Timestamp(unsigned char* p_uc8DataPacket, unsigned char* uc8PacketLength);
void DL_Print_Temperature(unsigned char* p_uc8DataPacket, unsigned char * uc8PacketLength, unsigned int u16Temp);
void DL_Print_Light(unsigned char* p_uc8DataPacket, unsigned char * uc8PacketLength, unsigned int u16Light);
void DL_Print_Humidity(unsigned char* p_uc8DataPacket, unsigned char * uc8PacketLength, unsigned int u16Humidity);
void DL_Print_Power_Lost(void);
void DL_Reload_Length(void);
void DL_Print_Memory_Full(void);
void TempDecToAscii(unsigned char* Ascii, int Decimal);
unsigned char LightDecToAscii(unsigned char* Ascii, unsigned int Decimal);
unsigned char HumidityDecToAscii(unsigned char* Ascii, unsigned int Decimal);
void DL_Jump_Pointers(unsigned char * p_StartOfVectorTable);
unsigned char DL_Get_State(void);
unsigned char DL_Get_Data(unsigned int ui16E104FileOffset, unsigned int ui16PacketLength, unsigned char ** ul32DLDataPtr, unsigned int * ui16DLDataLength);



#define DEC2ASCII(x) (((x) % 10) | 0x30)
#define RTCREG2ASCII(x) (x | 0x30)

#endif /* DATALOGGER_H_ */
