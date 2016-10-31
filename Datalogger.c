/*
 * {Datalogger.c}
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

#include "Datalogger.h"

unsigned char* gp_StartOfDataloggingMemory = (void *) 0x08800;  // Available Memory is found in map file.
unsigned char* gp_EndOfDataloggingMemory = (void *) 0x14000;
unsigned char* gp_StartOfVectorTable = (void *) 0x0FF79;	// 0xFF80 - 1
unsigned char* gp_EndOfVectorTable = (void *) 0x0FFFF;		// 0x10000 - 1

unsigned char p_uc8MemoryFullMessage[] = "\nLogger Memory Full!\n"
										"Current state: Stopped\n"
										"Please Reset.\n"
										"\n";

#pragma NOINIT (gp_StartOfData);
unsigned char* gp_StartOfData;

#pragma NOINIT (gp_EndOfData);
unsigned char* gp_EndOfData;

#pragma NOINIT (gp_ui8StartOfRecordHeader);
unsigned char* gp_ui8StartOfRecordHeader;

#pragma NOINIT (gp_TotalDataLength);
unsigned int gp_TotalDataLength;

#pragma NOINIT (gp_DLFullFlag);
unsigned char gp_DLFullFlag; // Handle buffer loop

void DL_Reset(void)
{
	gp_StartOfData = gp_EndOfDataloggingMemory;
	gp_EndOfData = gp_EndOfDataloggingMemory;
	gp_TotalDataLength = 0;
	gp_DLFullFlag = 0;
}

void DL_Set_Record_Header(unsigned char* p_ui8RecordHeader)
{
	gp_ui8StartOfRecordHeader = p_ui8RecordHeader;
}

void DL_Update_Length(unsigned int ui16AddLength)
{
//TODO Add support for detecting Max_Length
	gp_TotalDataLength += ui16AddLength;

	//update NLEN
	gp_ui8StartOfRecordHeader[0] = ((gp_TotalDataLength + 10) >> 8) & 0xFF;
	gp_ui8StartOfRecordHeader[1] = (gp_TotalDataLength + 10) & 0xFF;

	// Payload Length
	gp_ui8StartOfRecordHeader[6] = ((gp_TotalDataLength + 3) >> 8) & 0xFF;
	gp_ui8StartOfRecordHeader[7] = (gp_TotalDataLength + 3) & 0xFF;
}

//Call all Pointer Update Functions
void DL_Update_Pointers(unsigned int ui16DataLength)
{
	DL_Update_Length(ui16DataLength);

	//will need to make StartofData Helper funciton to handle boundries
	gp_StartOfData -= (unsigned long)ui16DataLength;
}

void DL_Update_Data(unsigned char* p_uc8DataSrc, unsigned int ui16DataLength)
{
	//Buffer is filling form High ("end") to low ("start") for display purposes, so the latest data is always at the top.
	//"End" starts at 0x14000 -> 0x10000.  Then skips the Vector Table (0xFF80-0xFFFF).
	//Runs from 0xFF79 -> to "Start" = gp_StartOfDataloggingMemory
	//Buffer is circular and loops back when it reaches the "Start"

	unsigned long ui16LengthHighSide, ui16LengthLowSide;

	switch(DL_Get_State()){
		case 0:  // pointer is on high side of Vector Table, Not looped yet.
			if((gp_StartOfData - ui16DataLength) > gp_EndOfVectorTable){ // Normal
				DL_Update_Pointers(ui16DataLength);
				//Store Data
				memcpy(gp_StartOfData, p_uc8DataSrc, ui16DataLength);
			}
			else{  		// We're storing across the Vector Table
				//Split the lengths up
				ui16LengthHighSide = gp_StartOfData - gp_EndOfVectorTable - 1;
				ui16LengthLowSide = ui16DataLength - ui16LengthHighSide;

				// point to edge of Vector Table
				DL_Update_Pointers(ui16LengthHighSide);
				// write end of data into "High" side of Vector Table
				memcpy(gp_StartOfData, &p_uc8DataSrc[ui16LengthLowSide], ui16LengthHighSide);

				//Jump pointers to other side of Vector table
				DL_Jump_Pointers(gp_StartOfVectorTable);

				// point to edge of Vector Table
				DL_Update_Pointers(ui16LengthLowSide);
				// fill in data for "Low" side of Vector Table
				memcpy(gp_StartOfData, p_uc8DataSrc, ui16LengthLowSide);
			}
			break;

		case 1:	// Pointer is on low side of Vector Table.  Not looped yet
			if((gp_StartOfData - ui16DataLength) > (gp_StartOfDataloggingMemory + 70)){  // +70 is leaving room for "Memory Full" Message
				DL_Update_Pointers(ui16DataLength);
				//Store Data
				memcpy(gp_StartOfData, p_uc8DataSrc, ui16DataLength);
			}
			else{  		// We're at the start edge of the Buffer

				DL_Update_Pointers(sizeof(p_uc8MemoryFullMessage));
				//Store Data
				memcpy(gp_StartOfData, p_uc8MemoryFullMessage, sizeof(p_uc8MemoryFullMessage));


				gp_DLFullFlag = 1;
			}

			break;
	}
}

void DL_Jump_Pointers(unsigned char * p_StartOfVectorTable){
	gp_StartOfData = p_StartOfVectorTable;
}

unsigned char DL_Get_State(void){
	unsigned char uc8CurrentState = 0;

	if(gp_StartOfData > gp_EndOfVectorTable){
		uc8CurrentState = 0;
	}
	else{
		uc8CurrentState = 1;
	}

	return uc8CurrentState;
}

unsigned char DL_Get_Data(unsigned int ui16E104FileOffset, unsigned int ui16PacketLength, unsigned char ** ul32DLDataPtr, unsigned int * ui16DLDataLength)
{
	unsigned char* gp_StartOfPacket;
	unsigned char uc8JumpCounter = 1;

	switch(DL_Get_State()){
		case 0:  // pointer is on high side of Vector Table, Not looped yet.
			gp_StartOfPacket = gp_StartOfData + ui16E104FileOffset;

			uc8JumpCounter = 1;
			ul32DLDataPtr[0] = gp_StartOfPacket;
			ui16DLDataLength[0] = ui16PacketLength;
			break;

		case 1: // Pointer is on low side of Vector Table.  Not looped yet
			if((gp_StartOfData + ui16E104FileOffset) > gp_StartOfVectorTable){
				//Complete packet is on the High side of Vector Table
				gp_StartOfPacket = ((gp_StartOfData + ui16E104FileOffset) - gp_StartOfVectorTable) + gp_EndOfVectorTable;

				uc8JumpCounter = 1;
				ul32DLDataPtr[0] = gp_StartOfPacket;
				ui16DLDataLength[0] = ui16PacketLength;
			}
			else if((gp_StartOfData + ui16E104FileOffset + ui16PacketLength) > gp_StartOfVectorTable){
				//Packet stretches over Vector table
				ul32DLDataPtr[0] = gp_StartOfData + ui16E104FileOffset;
				ui16DLDataLength[0] = gp_StartOfVectorTable - (gp_StartOfData + ui16E104FileOffset);  //because of my addressing...

				ul32DLDataPtr[1] = gp_EndOfVectorTable + 1;
				ui16DLDataLength[1] = ui16PacketLength - ui16DLDataLength[0];

				uc8JumpCounter = 2;
			}
			else{
				//Entire Packet on low side of buffer
				gp_StartOfPacket = gp_StartOfData + ui16E104FileOffset;

				uc8JumpCounter = 1;
				ul32DLDataPtr[0] = gp_StartOfPacket;
				ui16DLDataLength[0] = ui16PacketLength;
			}

			break;
	}

	return uc8JumpCounter;
}

unsigned char DL_Update(Datalogger_Modes_t Mode, unsigned int ui16Temperature_F, unsigned int ui16Light, unsigned int ui16Humidity)  // Added new Temp measurement to NDEF Message
{
	unsigned char p_uc8AsciiDataPacket[255];
	unsigned char uc8PacketLength = 0;

	if(!gp_DLFullFlag){
		switch(Mode){
		case Temp_Only:
			DL_Print_Timestamp(p_uc8AsciiDataPacket, &uc8PacketLength);
			DL_Print_Temperature(p_uc8AsciiDataPacket, &uc8PacketLength, ui16Temperature_F);
			p_uc8AsciiDataPacket[uc8PacketLength++] = '\n';
			break;

		case Temp_and_Light:
			DL_Print_Timestamp(p_uc8AsciiDataPacket, &uc8PacketLength);
			DL_Print_Temperature(p_uc8AsciiDataPacket, &uc8PacketLength, ui16Temperature_F);
			p_uc8AsciiDataPacket[uc8PacketLength++] = ' ';
			p_uc8AsciiDataPacket[uc8PacketLength++] = '|';
			p_uc8AsciiDataPacket[uc8PacketLength++] = ' ';
			DL_Print_Light(p_uc8AsciiDataPacket, &uc8PacketLength, ui16Light);
			p_uc8AsciiDataPacket[uc8PacketLength++] = '\n';
			break;

		case Temp_and_Humidity:
			DL_Print_Timestamp(p_uc8AsciiDataPacket, &uc8PacketLength);
			DL_Print_Temperature(p_uc8AsciiDataPacket, &uc8PacketLength, ui16Temperature_F);
			p_uc8AsciiDataPacket[uc8PacketLength++] = ' ';
			p_uc8AsciiDataPacket[uc8PacketLength++] = '|';
			p_uc8AsciiDataPacket[uc8PacketLength++] = ' ';
			DL_Print_Humidity(p_uc8AsciiDataPacket, &uc8PacketLength, ui16Humidity);
			p_uc8AsciiDataPacket[uc8PacketLength++] = '\n';
			break;

		case Temp_Light_and_Humidity:
			DL_Print_Timestamp(p_uc8AsciiDataPacket, &uc8PacketLength);
			DL_Print_Temperature(p_uc8AsciiDataPacket, &uc8PacketLength, ui16Temperature_F);
			p_uc8AsciiDataPacket[uc8PacketLength++] = ' ';
			p_uc8AsciiDataPacket[uc8PacketLength++] = '|';
			p_uc8AsciiDataPacket[uc8PacketLength++] = ' ';
			DL_Print_Light(p_uc8AsciiDataPacket, &uc8PacketLength, ui16Light);
			p_uc8AsciiDataPacket[uc8PacketLength++] = ' ';
			p_uc8AsciiDataPacket[uc8PacketLength++] = '|';
			p_uc8AsciiDataPacket[uc8PacketLength++] = ' ';
			DL_Print_Humidity(p_uc8AsciiDataPacket, &uc8PacketLength, ui16Humidity);
			p_uc8AsciiDataPacket[uc8PacketLength++] = '\n';
			break;

		}

		DL_Update_Data(p_uc8AsciiDataPacket, uc8PacketLength);
	}

	return !gp_DLFullFlag;
}


void DL_Print_Timestamp(unsigned char* p_uc8DataPacket, unsigned char* uc8PacketLength)
{
	p_uc8DataPacket[(*uc8PacketLength)++] = '['; //OPEN_BRACKET_ASCII;
	p_uc8DataPacket[(*uc8PacketLength)++] = RTCREG2ASCII((RTCMON >> 4) & 0x0F);
	p_uc8DataPacket[(*uc8PacketLength)++] = RTCREG2ASCII(RTCMON & 0x0F);
	p_uc8DataPacket[(*uc8PacketLength)++] = '/';
	p_uc8DataPacket[(*uc8PacketLength)++] = RTCREG2ASCII((RTCDAY >> 4) & 0x0F);
	p_uc8DataPacket[(*uc8PacketLength)++] = RTCREG2ASCII(RTCDAY & 0x0F);
	p_uc8DataPacket[(*uc8PacketLength)++] = '/';
	p_uc8DataPacket[(*uc8PacketLength)++] = RTCREG2ASCII((RTCYEAR >> 4) & 0x0F);
	p_uc8DataPacket[(*uc8PacketLength)++] = RTCREG2ASCII(RTCYEAR & 0x0F);
	p_uc8DataPacket[(*uc8PacketLength)++] = ' ';
	p_uc8DataPacket[(*uc8PacketLength)++] = RTCREG2ASCII((RTCHOUR >> 4) & 0x0F);
	p_uc8DataPacket[(*uc8PacketLength)++] = RTCREG2ASCII(RTCHOUR & 0x0F);
	p_uc8DataPacket[(*uc8PacketLength)++] = ':';
	p_uc8DataPacket[(*uc8PacketLength)++] = RTCREG2ASCII((RTCMIN >> 4) & 0x0F);
	p_uc8DataPacket[(*uc8PacketLength)++] = RTCREG2ASCII(RTCMIN & 0x0F);
	p_uc8DataPacket[(*uc8PacketLength)++] = ']'; //CLOSED_BRACKET_ASCII;
	p_uc8DataPacket[(*uc8PacketLength)++] = ' ';
}

void DL_Print_Power_Lost(void)
{
	unsigned char p_uc8PowerLostMessage[] = "\nPower was interrupted!\n"
											"Time and Date have been reset.\n"
											"Current state: Stopped\n"
											"\n";

	DL_Update_Data(p_uc8PowerLostMessage, sizeof(p_uc8PowerLostMessage));
}

void DL_Reload_Length(void)
{
	DL_Update_Pointers(0);
}

void DL_Print_Memory_Full(void)
{
	unsigned char p_uc8MemoryFullMessage[] = "\nLogger Memory Full!\n"
											"Current state: Stopped\n"
											"\n";

	DL_Update_Data(p_uc8MemoryFullMessage, sizeof(p_uc8MemoryFullMessage));
}

void DL_Print_Temperature(unsigned char* p_uc8DataPacket, unsigned char * uc8PacketLength, unsigned int u16Temp)
{
	TempDecToAscii(&p_uc8DataPacket[*uc8PacketLength], u16Temp);
	*uc8PacketLength = *uc8PacketLength + 5;
//	g_TempDataFahr = 0;
	p_uc8DataPacket[(*uc8PacketLength)++] = ' ';

	if(g_ui8TemperatureModeFlag == Fahrenheit){
		p_uc8DataPacket[(*uc8PacketLength)++] = 'F';
	}
	else{
		p_uc8DataPacket[(*uc8PacketLength)++] = 'C';
	}
}

void DL_Print_Light(unsigned char* p_uc8DataPacket, unsigned char * uc8PacketLength, unsigned int u16Light)
{
	unsigned char uc_length=0;

	uc_length = LightDecToAscii(&p_uc8DataPacket[*uc8PacketLength], u16Light);
	*uc8PacketLength = *uc8PacketLength + uc_length;

	p_uc8DataPacket[(*uc8PacketLength)++] = ' ';
	p_uc8DataPacket[(*uc8PacketLength)++] = 'l';
	p_uc8DataPacket[(*uc8PacketLength)++] = 'x';
}

void DL_Print_Humidity(unsigned char* p_uc8DataPacket, unsigned char * uc8PacketLength, unsigned int u16Humidity)
{
	unsigned char uc_length=0;

	uc_length = HumidityDecToAscii(&p_uc8DataPacket[*uc8PacketLength], u16Humidity);
	*uc8PacketLength = *uc8PacketLength + uc_length;
	p_uc8DataPacket[(*uc8PacketLength)++] = '%';
	p_uc8DataPacket[(*uc8PacketLength)++] = ' ';
	p_uc8DataPacket[(*uc8PacketLength)++] = 'R';
	p_uc8DataPacket[(*uc8PacketLength)++] = 'H';
}

void TempDecToAscii(unsigned char* Ascii, int Decimal)
{
	unsigned char Hunds, Tens, Ones, Tenths;

	Tenths = (Decimal % 10);
	Ones = ((Decimal / 10) % 10);
	Tens = ((Decimal / 100) % 10);
	Hunds = ((Decimal / 1000) % 10);

	if(Hunds > 0){ *Ascii++ = Hunds | 0x30;}
	else if(ui8TemperatureNegFlag == 1){*Ascii++ = '-';}
	else{*Ascii++ = ' ';}

	*Ascii++ = Tens | 0x30;
	*Ascii++ = Ones | 0x30;
	*Ascii++ = '.';
	*Ascii   = Tenths | 0x30;
}

unsigned char LightDecToAscii(unsigned char* Ascii, unsigned int Decimal)
{
	unsigned char length = 0;
	unsigned char TenThousands, Thousands, Hunds, Tens, Ones;

	Ones = (Decimal % 10);
	Tens = ((Decimal / 10) % 10);
	Hunds = ((Decimal / 100) % 10);
	Thousands = ((Decimal / 1000) % 10);
	TenThousands = ((Decimal / 10000) % 10);

	if(TenThousands){
		*Ascii++ = TenThousands | 0x30;
		*Ascii++ = Thousands | 0x30;
		*Ascii++ = Hunds | 0x30;
		*Ascii++ = Tens | 0x30;
		*Ascii++ = Ones | 0x30;
		length = 5;
	}
	else if(Thousands){
		*Ascii++ = Thousands | 0x30;
		*Ascii++ = Hunds | 0x30;
		*Ascii++ = Tens | 0x30;
		*Ascii++ = Ones | 0x30;
		length = 4;
	}
	else{
		*Ascii++ = Hunds | 0x30;
		*Ascii++ = Tens | 0x30;
		*Ascii++ = Ones | 0x30;
		length = 3;
	}

	return length;
}

unsigned char HumidityDecToAscii(unsigned char* Ascii, unsigned int Decimal)
{
	unsigned char length = 0;
	unsigned char Tens, Ones;

	Ones = (Decimal % 10);
	Tens = ((Decimal / 10) % 10);

	if(Decimal > 99){
		*Ascii++ = '1';
		*Ascii++ = '0';
		*Ascii++ = '0';
		length = 3;
	}
	else{
		*Ascii++ = Tens | 0x30;
		*Ascii++ = Ones | 0x30;
		length = 2;
	}

	return length;
}
