/*
 * {RF430_Request_Processor.c}
 *
 * {Handles interupts and memory flow for RF430CL331}
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

#include "RF430_Request_Processor.h"
#include "msp430.h"

#pragma NOINIT (NdefFiles);
struct NdefFile_Type NdefFiles[3];

#pragma NOINIT (g_u8Second);
unsigned char g_u8Second;

#pragma NOINIT (g_u8Minute);
unsigned char g_u8Minute;

#pragma NOINIT (g_u8Hour);
unsigned char g_u8Hour;

#pragma NOINIT (g_u8Day);
unsigned char g_u8Day;

#pragma NOINIT (g_u8Month);
unsigned char g_u8Month;

#pragma NOINIT (g_u8Year);
unsigned char g_u8Year;

#pragma NOINIT (g_u8PollingInterval);
unsigned char g_u8PollingInterval;
unsigned char g_u8PollingIntervalLength;

#pragma NOINIT (g_u8Mode);
Datalogger_Modes_t g_u8Mode;

#pragma NOINIT (g_u8TempMode);
Temp_Modes_t g_u8TempMode;

#pragma NOINIT (NumberOfFiles);
unsigned int NumberOfFiles;

const unsigned char CCFILE_TEXT_DEFAULT_DATA[]	=  {										\
	0x00, 0x0F,	/* CCLEN */																		\
	0x20,		/* Mapping version 2.0 */														\
	0x00, 0xF9,	/* MLe (49 bytes); Maximum R-APDU data size */									\
	0x00, 0xF6, /* MLc (52 bytes); Maximum C-APDU data size */									\
	0x04, 		/* Tag, File Control TLV (4 = NDEF file) */										\
	0x06, 		/* Length, File Control TLV (6 = 6 bytes of data for this tag) */				\
	0xE1, 0x04,	/* File Identifier */															\
	0xFF, 0xFE, /* Max NDEF size (3037 bytes of useable memory) */								\
	0x00, 		/* NDEF file read access condition, read access without any security */			\
	0x00 		/* NDEF file write access condition; write access without any security */		\
	};//CC file text

#define E104_DEFAULT_DATA_RECORD_LENGTH 12

unsigned char E104_DEFAULT_DATA_RECORD[] = {						\
	0x00, 0x0A, /* 0x39 NLEN; NDEF length (3 byte long message) */	\
	0xC1, /* Record Header	*/										\
	0x01, /* Type Length	*/										\
	0x00, 0x00, 0x00, 0x03,  /* Payload length This NLEN - 7 	*/	\
	0x54, /* T = text */											\
	0x02, /* ID length	*/											\
	0x65, 0x6E, /* 'e', 'n', ID */									\
};

unsigned char E104_DEFAULT_DATA[] ="\nTI's Datalogger Demo!\n"
"\n"
"Default Settings:\n"
"    Mode: Temperature Only (F)\n"
"    Time: 12:00:00 AM\n"
"    Date: 01/01/15\n"
"    Polling Interval: 10 Minutes\n"
"    Default State: Stopped\n"
"\n"
"Control Commands\n"
"    ST - Start\n"
"    SP - Stop\n"
"    CD - Clear Data\n"
"    RE - Reset\n"
"\n"
"Config Commands\n"
"    TI hh:mm:ss - Set Time\n"
"    DA mm/dd/yy - Date\n"
"    PI xxx - Set Polling Interval (minutes)\n"
"    TM x - Temp Mode: 'F' or 'C'\n"
"    MO x - Set Mode (0-3)\n"
"        0: Temperature\n"
"        1: Temperature and Light\n"
"        2: Temperature and Humidity\n"
"        3: Temperature, Light, and Humidity\n";

#pragma NOINIT (CCFileText);
/*const*/ unsigned char CCFileText[25];

#pragma NOINIT (FileTextE104);
unsigned char FileTextE104[E104_TOTAL_SIZE];

//setting up pointers for a revoling buffer
#pragma NOINIT (g_pui8E104StartOfPayload);
unsigned char * g_pui8E104StartOfPayload;

#pragma NOINIT (g_pui8E104EndOfPayload);
unsigned char * g_pui8E104EndOfPayload;

//#pragma NOINIT (SelectedFile);
unsigned int SelectedFile = 0;		//the file that is currently selected

// Used to receive commands and filter writes before updating Data Memory
unsigned char ReceivedFileTextPrebufCC[30];
unsigned char ReceivedFileTextPrebuf[100];

////Initializes the file management
//void AppInit()
//{
////Not working....  Somehow resets?
//
//	//TODO Check if has been previously powered.  If so, choose state accordingly.
//
////	if((NdefFiles[0].FileID[0] != 0xE1) || (NdefFiles[0].FileID[1] != 0x03) || (NdefFiles[1].FileID[0] != 0xE1)){ //  || (g_u16NdefDataPointer > 1400))
//	// Just Programed
//		ResetDatalogger();
////	}
////	else{  // Re-powered
////		RF430_Power_Lost_Message();  		// Indicate Power was lost for unkown time
////	}
//
//	NumberOfFiles = 2; 			//the number if NDEF files available
//	SelectedFile = 0;			//default to CC file
//}

void ResetDatalogger(void){
	DL_Reset();

	NdefFiles[0].FileID[0] = 0xE1;
	NdefFiles[0].FileID[1] = 0x03;
	NdefFiles[0].FilePointer = (unsigned char *)CCFileText;
	memcpy(NdefFiles[0].FilePointer, CCFILE_TEXT_DEFAULT_DATA, 15); 	// LOADING DEFAULT CC DATA
	NdefFiles[0].FileLength = 0;		//?

	// Init Ndef file info
	NdefFiles[1].FileID[0] = 0xE1;
	NdefFiles[1].FileID[1] = 0x04;
	NdefFiles[1].FilePointer = gp_StartOfData;
//	memcpy(NdefFiles[1].FilePointer, E104_DEFAULT_DATA_RECORD, E104_DEFAULT_DATA_RECORD_LENGTH);

	DL_Set_Record_Header(E104_DEFAULT_DATA_RECORD);
	DL_Update_Data(E104_DEFAULT_DATA, sizeof(E104_DEFAULT_DATA)-1);

	g_u16NdefLength = 10;  			// Empty NDEF Message length
	g_u16NdefDataPointer = NDEF_DATA_POINTER_START_LOCATION;
	g_u8MaxLengthFlag = 0;			// NDEF Max Length Flag

	NumberOfFiles = 2; 			//the number if NDEF files available
	SelectedFile = 0;			//default to CC file
}

//Service an interrupt from the RF430
unsigned int ServiceInterrupt(unsigned int flags)
{
	unsigned int interrupt_serviced = 0;
	unsigned int TagWritten = 0;

	if(flags & GENERIC_ERROR_INT_FLAG) 									//check if the there was a reset on the RF430
	{
		interrupt_serviced |= GENERIC_ERROR_INT_FLAG;
		 CL331H_Write_Register(INT_FLAG_REG, interrupt_serviced); 				//clear this flag
	}

	if(flags & FIELD_REMOVED_INT_FLAG) 									//check if the tag was removed
	{
		interrupt_serviced |= FIELD_REMOVED_INT_FLAG;					//clear this flag later
		 CL331H_Write_Register(INT_FLAG_REG, interrupt_serviced); //ACK the flags to clear
	}

	if(flags & DATA_TRANSACTION_INT_FLAG) //check if the tag was read
	{
		unsigned int status;
		unsigned int ret;
		status = CL331H_Read_Register(STATUS_REG); //read the status register to determine the nature of the interrupt

		switch (status & APP_STATUS_REGS)
		{
			// NDEF File Select Request is coming from the mobile/reader - response to the request is determined here
			// based on whether the file exists in our file database
			case FILE_SELECT_STATUS:
			{
				unsigned int file_id;
				file_id = CL331H_Read_Register(NDEF_FILE_ID);						// determine the file identifier of the select request
				ret = SearchForFile((unsigned char *)&file_id);				// determine if the file exists on the host controller
				interrupt_serviced |= DATA_TRANSACTION_INT_FLAG;			//clear this flag later
				if (ret == FileFound)
				{
					 CL331H_Write_Register(INT_FLAG_REG, interrupt_serviced); 		//ACK the flags to clear
					 CL331H_Write_Register(HOST_RESPONSE, INT_SERVICED_FIELD + FILE_EXISTS_FIELD);			//indicate to the RF430 that the file exist
				}
				else
				{
					 CL331H_Write_Register(INT_FLAG_REG, interrupt_serviced); 		//ACK the flags to clear
					 CL331H_Write_Register(HOST_RESPONSE, INT_SERVICED_FIELD + FILE_DOES_NOT_EXIST_FIELD);	// the file does not exist
				}
				break;
			}

			//NDEF ReadBinary request has been sent by the mobile / reader
			case FILE_REQUEST_STATUS:
			{
				unsigned int buffer_start;
				unsigned int file_offset;
				unsigned int file_length;
				buffer_start = CL331H_Read_Register(NDEF_BUFFER_START);		// where to start writing the file info in the RF430 buffer (0-2999)
				file_offset = CL331H_Read_Register(NDEF_FILE_OFFSET);			// what part of the file to start sending
				file_length = CL331H_Read_Register(NDEF_FILE_LENGTH);			// how much of the file starting at offset to send
																		// we can send more than requested, called caching
																		// as long as we write back into the length register how
																		// much we sent it
				interrupt_serviced |= DATA_TRANSACTION_INT_FLAG;					//clear this flag later
				//can have bounds check for the requested length

				file_length = SendDataOnFile(SelectedFile, buffer_start, file_offset, file_length);

				 CL331H_Write_Register(NDEF_FILE_LENGTH, file_length);  		// how much was actually written
				 CL331H_Write_Register(INT_FLAG_REG, interrupt_serviced); 		// ACK the flags to clear
				 CL331H_Write_Register(HOST_RESPONSE, INT_SERVICED_FIELD);		// indicate that we have service the request

				break;
			}

			// NDEF UpdateBinary request
			case FILE_AVAILABLE_STATUS:
			{
				unsigned int buffer_start;
				unsigned int file_offset;
				unsigned int file_length;
				interrupt_serviced |= DATA_TRANSACTION_INT_FLAG;			// clear this flag later
				buffer_start = CL331H_Read_Register(NDEF_BUFFER_START);			// where to start in the RF430 buffer to read the file data (0-2999)
				file_offset = CL331H_Read_Register(NDEF_FILE_OFFSET);				// the file offset that the data begins at
				file_length = CL331H_Read_Register(NDEF_FILE_LENGTH);				// how much of the file is in the RF430 buffer

				//can have bounds check for the requested length
				TagWritten = ReadDataOnFile(SelectedFile, buffer_start, file_offset, file_length);
				CL331H_Write_Register(INT_FLAG_REG, interrupt_serviced); 			// ACK the flags to clear
				CL331H_Write_Register(HOST_RESPONSE, INT_SERVICED_FIELD);			// the interrup has been serviced

				break;
			}
		}
	}

	// This is the pre-fetch interrupt
	// If this is enabled with the interrupt enable, the RF430 will request more read data when it is sending the previous
	// ReadBinary request.  The goal is to have enough data in the RF430 buffer for the next ReadBinary request so that
	// it responds the ReadBinary request automatically, with no interrupt.
	else if(flags & EXTRA_DATA_IN_FLAG) //check if the tag was read
	{
		#define AMOUNT_DATA_TO_SENT_EARLY		255							// data to add to the buffer while transmit is happening
		#define MAX_TAG_BUFFER_SPACE			2998						// actually 3000 but to avoid any possibility of an issue

		unsigned int buffer_start;
		unsigned int file_offset;
		unsigned int file_length = 0;

		interrupt_serviced |= EXTRA_DATA_IN_FLAG;							// clear the interrupt flag

		if (SelectedFile == 1)   											// allows only prefetch on NDEF files, no Capability container ones
		{
			buffer_start = CL331H_Read_Register(NDEF_BUFFER_START);
			file_offset = CL331H_Read_Register(NDEF_FILE_OFFSET);
			//		file_length = CL331H_Read_Register(NDEF_FILE_LENGTH);

			if ((buffer_start + AMOUNT_DATA_TO_SENT_EARLY) >= MAX_TAG_BUFFER_SPACE)
			{
				// can't fill the buffer anymore
				// do no fill.  New data request interrupt will come later.
				__no_operation();
			}
			else
			{
				//!!!!range check on file needs to be done here!!!
				//can have bounds check for the requested length
				file_length = SendDataOnFile(SelectedFile, buffer_start, file_offset, AMOUNT_DATA_TO_SENT_EARLY); //255 is enough for atleast one packet
			}
		}
		 CL331H_Write_Register(NDEF_FILE_LENGTH, file_length);  						// how much was actually written
		 CL331H_Write_Register(INT_FLAG_REG, interrupt_serviced);						// ACK the flags to clear
		 CL331H_Write_Register(HOST_RESPONSE, EXTRA_DATA_IN_SENT_FIELD);				// interrupt was serviced
	}

	return TagWritten;
}

//Select request
//find if we have the file
enum FileExistType SearchForFile(unsigned char *fileId)
{
	unsigned int i;
	enum FileExistType ret = FileNotFound; // 0 means not found, 1 mean found

	for (i = 0; i < NumberOfFiles; i++)
	{
		if (NdefFiles[i].FileID[0] == fileId[0] && NdefFiles[i].FileID[1] == fileId[1])
		{
			ret = FileFound;
			SelectedFile = i;  // the index of the selected file in the array
			break;
		}
	}
	return ret;
}

//Tag is requesting data on the selected file
//This would be a result of the ReadBinary NDEF command
//return how much written
unsigned int SendDataOnFile(unsigned int selectedFile, unsigned int buffer_start, unsigned int file_offset, unsigned int length)
{
	//TODO make this function support revoling buffer.
	unsigned int ret_length;


	ret_length = length;

	if(selectedFile == 0){ // 0xE103 Capability file
		Write_Continuous (buffer_start, (unsigned char *)&NdefFiles[selectedFile].FilePointer[file_offset], length);
	}
	else if(selectedFile == 1){ // 0xE104 Data file
		E104_Send_File(buffer_start, file_offset, length);
	}

	return ret_length;

	//Regs[NDEF_FILE_LENGTH_INDEX]  = Regs[NDEF_FILE_LENGTH_INDEX]
	//do not change right now, we are only sending as much as has been requested
	//if we wanted to send data than requested, we would update the Regs[NDEF_FILE_LENGTH_INDEX] register to a higher value
}

void E104_Send_File(unsigned int ui16CL331StartAddr, unsigned int ui16E104FileOffset, unsigned int ui16PacketLength)
{
	unsigned char * ul32DLDataPtr[3];
	unsigned int ui16DLDataLength[3];
	unsigned char uc8DLJumpCount = 0;
	unsigned int i = 0;

	E104_Send_File_Record(&ui16CL331StartAddr, &ui16E104FileOffset, &ui16PacketLength);

	if(ui16PacketLength > 0){

		//Update length for payload data
		//TODO Add support for looping buffer
		__no_operation();
		uc8DLJumpCount = DL_Get_Data(ui16E104FileOffset, ui16PacketLength, ul32DLDataPtr,  ui16DLDataLength);

		for(i=0;i<uc8DLJumpCount;i++){
			Write_Continuous (ui16CL331StartAddr, ul32DLDataPtr[i], ui16DLDataLength[i]);
			ui16CL331StartAddr += ui16DLDataLength[i];
		}
	}

	// Providing the Payload
}

void E104_Send_File_Record(unsigned int * p_ui16CL331StartAddr, unsigned int * p_ui16E104FileOffset, unsigned int * p_ui16PacketLength)
{
	unsigned int ui16LengthDifference = 0;

	if(*p_ui16E104FileOffset < E104_DEFAULT_DATA_RECORD_LENGTH){  //Reading Record

		ui16LengthDifference = E104_DEFAULT_DATA_RECORD_LENGTH - *p_ui16E104FileOffset;

		if((*p_ui16E104FileOffset + *p_ui16PacketLength) < E104_DEFAULT_DATA_RECORD_LENGTH ){  // Reading part of Record only
			Write_Continuous (*p_ui16CL331StartAddr, &E104_DEFAULT_DATA_RECORD[*p_ui16E104FileOffset], *p_ui16PacketLength);
			*p_ui16PacketLength = 0;
		}
		else{  // Reading some of record, and some payload
			Write_Continuous (*p_ui16CL331StartAddr, &E104_DEFAULT_DATA_RECORD[*p_ui16E104FileOffset], ui16LengthDifference);

			*p_ui16CL331StartAddr += ui16LengthDifference;
			*p_ui16PacketLength -= ui16LengthDifference;
			*p_ui16E104FileOffset = 0;
		}
	}
	else{
		*p_ui16E104FileOffset -= E104_DEFAULT_DATA_RECORD_LENGTH;
	}
}

//Tag has data to be read out on the selected file
//This would be a result of the UpdateBinary NDEF command
unsigned char ReadDataOnFile(unsigned int selectedFile, unsigned int buffer_start, unsigned int file_offset, unsigned int length)
{
//	unsigned int * e104_l = (unsigned int *)&E104_Length;
	unsigned char uc8DataReceivedFlag = 0;

	if(length < 100){
//		Read_Continuous(buffer_start, (unsigned char *)&NdefReceivedFiles[selectedFile].FilePointer[file_offset], length);
		Read_Continuous(buffer_start, ReceivedFileTextPrebuf, length);
		uc8DataReceivedFlag = 1;
//		ReceivedFileTextPrebuf
	}

	if (selectedFile == 1){
		__no_operation();
	}


	return uc8DataReceivedFlag;
	//Regs[NDEF_FILE_LENGTH_INDEX]  = Regs[NDEF_FILE_LENGTH_INDEX]
	//do not change right now, we are only sending as much as has been requested
	//if we wanted to send data than requested, we would update the Regs[NDEF_FILE_LENGTH_INDEX] register to a higher value
//	NdefFiles[selectedFile].FileLength = offset + index;  //not safe here if random update binary writes are done by reader
}


void ProcessReceivedFileText(Datalogger_Commands_t *p_Command_Received_t)
{
	unsigned char i=0;
	*p_Command_Received_t = None;

	if((ReceivedFileTextPrebuf[0] == 0x00) && (ReceivedFileTextPrebuf[1] == 0x00)){  // Work around for Samsung Galaxy S3 which write in the data in a different location
		for(i=0;i<20;i++){
			ReceivedFileTextPrebuf[i]=ReceivedFileTextPrebuf[i+2];
		}
	}


	if(((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'R') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'r')) &&
		((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'E') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'e'))){
		*p_Command_Received_t = Reset_Command;
	}
	else if(((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'S') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 's')) &&
			((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'T') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 't'))){
		*p_Command_Received_t = Start_Command;
	}
	else if(((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'S') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 's')) &&
			((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'P') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'p'))){
		*p_Command_Received_t = Stop_Command;
	}
	else if(((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'C') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'c')) &&
			((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'D') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'd'))){
		*p_Command_Received_t = Clear_Data_Command;
	}
	else if(((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'M') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'm')) &&
			((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'O') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'o'))){
		*p_Command_Received_t = Set_Mode;

		switch( ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+3] & ~0x30 ){
			case 0:
				g_u8Mode = Temp_Only;
				break;

			case 1:
				g_u8Mode = Temp_and_Light;
				break;

			case 2:
				g_u8Mode = Temp_and_Humidity;
				break;

			case 3:
				g_u8Mode = Temp_Light_and_Humidity;
				break;

			default:
				g_u8Mode = Temp_Only;
				break;
		}
	}
	else if(((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'T') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 't')) &&
			((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'M') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'm'))){
		*p_Command_Received_t = Set_Temp_Mode;

		if( (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+3] == 'C') ||  (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+3] == 'c') ){
			g_u8TempMode = Celcius;
		}
		else{
			g_u8TempMode = Fahrenheit;
		}
	}
	else if(((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'T') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 't')) &&
			((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'I') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'i'))){
		*p_Command_Received_t = Set_Time;

		g_u8Hour = (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+3] & ~0x30) << 4;
		g_u8Hour |= (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+4] & ~0x30);
		if(((g_u8Hour & 0xF0) > 0x20) || ((g_u8Hour & 0x0F) > 0x09)){ g_u8Hour = 0x00;}		// Filter Inputs

		g_u8Minute = (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+6] & ~0x30) << 4;
		g_u8Minute |= (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+7] & ~0x30);
		if(((g_u8Minute & 0xF0) > 0x50) || ((g_u8Minute & 0x0F) > 0x09)){ g_u8Minute = 0x00;}		// Filter Inputs

		g_u8Second = (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+9] & ~0x30) << 4;
		g_u8Second |= (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+10] & ~0x30);
		if(((g_u8Second & 0xF0) > 0x50) || ((g_u8Second & 0x0F) > 0x09)){ g_u8Second = 0x00;}		// Filter Inputs
	}
	else if(((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'D') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'd')) &&
			((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'A') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'a'))){
		*p_Command_Received_t = Set_Date;

		g_u8Month = (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+3] & ~0x30) << 4;
		g_u8Month |= (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+4] & ~0x30);
		if(((g_u8Month & 0xF0) > 0x10) || ((g_u8Month & 0x0F) > 0x09)){ g_u8Month = 0x12;}		// Filter Inputs

		g_u8Day = (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+6] & ~0x30) << 4;
		g_u8Day |= (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+7] & ~0x30);
		if(((g_u8Day & 0xF0) > 0x50) || ((g_u8Day & 0x0F) > 0x09)){ g_u8Day = 0x00;}		// Filter Inputs

		g_u8Year = (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+9] & ~0x30) << 4;
		g_u8Year |= (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+10] & ~0x30);
		if(((g_u8Year & 0xF0) > 0x50) || ((g_u8Year & 0x0F) > 0x09)){ g_u8Year = 0x00;}		// Filter Inputs
	}
	else if(((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'P') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] == 'p')) &&
			((ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'I') || (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] == 'i'))){
		*p_Command_Received_t = Set_Polling_Interval;

		g_u8PollingIntervalLength = ReceivedFileTextPrebuf[2] - E104_RECEIVED_PAYLOAD_START_OFFSET + 1;
		switch(g_u8PollingIntervalLength){
		case 1:
			g_u8PollingInterval = ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+3] & ~0x30;
			break;
		case 2:
			g_u8PollingInterval = 10*(ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+3] & ~0x30);
			g_u8PollingInterval += ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+4] & ~0x30;
			break;
		case 3:
			g_u8PollingInterval = 100*(ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+3] & ~0x30);
			g_u8PollingInterval += 10*(ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+4] & ~0x30);
			g_u8PollingInterval += ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+5] & ~0x30;
			break;
		}

//		g_u8PollingInterval = (ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+3] & ~0x30)
	}

	//Clear out command
	ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET] = 0;
	ReceivedFileTextPrebuf[E104_RECEIVED_PAYLOAD_START_OFFSET+1] = 0;
}

void FetchReceivedTime(unsigned char* p_uc8TimeBuffer)
{
	*p_uc8TimeBuffer++ = g_u8Hour;
	*p_uc8TimeBuffer++ = g_u8Minute;
	*p_uc8TimeBuffer = g_u8Second;
}

void FetchReceivedDate(unsigned char* p_uc8DateBuffer)
{
	*p_uc8DateBuffer++ = g_u8Month;
	*p_uc8DateBuffer++ = g_u8Day;
	*p_uc8DateBuffer = g_u8Year;
}

void FetchReceivedPollingInterval(unsigned char* p_uc8PollingIntervalBuffer)
{
	*p_uc8PollingIntervalBuffer = g_u8PollingInterval;
}

void FetchReceivedMode(Datalogger_Modes_t* p_uc8ModeBuffer)
{
//	switch(g_u8Mode){
//	case
//	}
//	g_u8Mode = Temp_Only; // for now
	*p_uc8ModeBuffer = g_u8Mode;
}

void FetchReceivedTempMode(Temp_Modes_t* p_uc8TempModeBuffer)
{
//	switch(g_u8Mode){
//	case
//	}
//	g_u8Mode = Temp_Only; // for now
	*p_uc8TempModeBuffer = g_u8TempMode;
}




