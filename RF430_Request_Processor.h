/*
 * {RF430_Request_Processor.h}
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

#ifndef RF430_REQUEST_PROCESSOR_H_
#define RF430_REQUEST_PROCESSOR_H_

#include "RF430CL331H.h"
#include "Datalogger.h"

enum FileExistType
{
	FileFound 		= 1,
	FileNotFound 	= 0
};

typedef enum {
	None,
	Set_Mode,
	Set_Time,
	Set_Date,
	Set_Temp_Mode,
	Set_Polling_Interval,
	Reset_Command,
	Start_Command,
	Stop_Command,
	Clear_Data_Command
} Datalogger_Commands_t;


extern struct NdefFile_Type NdefFiles[3];
unsigned int ServiceInterrupt(unsigned int flags);
//void AppInit();
enum FileExistType SearchForFile(unsigned char *fileId);
unsigned char ReadDataOnFile(unsigned int selectedFile, unsigned int buffer_start, unsigned int file_offset, unsigned int length);
unsigned int SendDataOnFile(unsigned int selectedFile, unsigned int buffer_start, unsigned int file_offset, unsigned int length);

void ResetDatalogger(void);
void ProcessReceivedFileText(Datalogger_Commands_t *p_Command_Received_t);
void FetchReceivedTime(unsigned char* p_uc8TimeBuffer);
void FetchReceivedDate(unsigned char* p_uc8DateBuffer);
void FetchReceivedPollingInterval(unsigned char* p_uc8DateBuffer);
//void FetchReceivedMode(unsigned char* p_uc8DateBuffer);
void FetchReceivedMode(Datalogger_Modes_t* p_uc8ModeBuffer);
void FetchReceivedTempMode(Temp_Modes_t* p_uc8ModeBuffer);
void E104_Send_File(unsigned int ui16CL331StartAddr, unsigned int ui16E104FileOffset, unsigned int ui16PacketLength);
void E104_Send_File_Record(unsigned int * p_ui16CL331StartAddr, unsigned int * p_ui16E104FileOffset, unsigned int * p_ui16PacketLength);

typedef struct NdefFile_Type
{
	unsigned char FileID[2];
	unsigned char * FilePointer;
	unsigned int FileLength;
}NdefFileType;


#define E104_RECEIVED_PAYLOAD_START_OFFSET 	7
#define E104_RECEIVED_PAYLOAD_START_OFFSET_FOR_S3 	9
#define E104_TOTAL_SIZE 		3000
#define E104_END_OF_RECORD 		12



extern unsigned char FileTextE104[];
#endif /* RF430_REQUEST_PROCESSOR_H_ */
