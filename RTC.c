/*
 * {RTC.c}
 *
 * {Manages Real Time Clock for Datalogging}
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

#include "RTC.h"

void RTC_Init(void)
{

	// Configure RTC_B
	RTCCTL01 = RTCBCD | RTCHOLD | RTCTEV_0 | RTCTEVIE | RTCAIE;  	// BCD mode, RTC hold, enable RTC

	RTCHOUR = 0x00;                           	// Hour = 0x12
	RTCMIN = 0x00;                            	// Minute = 0x00
	RTCSEC = 0x00;                           	// Seconds = 0x00
	RTCYEAR = 0x2015;      						// Year = 0x2015
	RTCMON = 0x01;              				// Month = 0x01 = Jan
	RTCDAY = 0x01;                       		// Day = 0x01 = 1st

	RTCAMIN = 0x00;
	RTCAHOUR = 0x00;
	RTCADAY = 0x00;
	RTCADOW = 0x00;

	RTCCTL01 &= ~RTCHOLD;                     // Start RTC calendar mode
}

void RTC_Set_Time(unsigned char* uc8TimeBuffer)
{
	// Configure RTC_B
	RTCCTL01 |= RTCHOLD;  					// RTC hold

	RTCHOUR = *uc8TimeBuffer++;                           // Hour = 0x11
	RTCMIN = *uc8TimeBuffer++;                            // Minute = 0x59
	RTCSEC = *uc8TimeBuffer;                            // Seconds = 0x30

	RTCCTL01 &= ~RTCHOLD;                     // Start RTC calendar mode
}

void RTC_Set_Date(unsigned char* uc8TimeBuffer)
{
  // Configure RTC_B
	RTCCTL01 |= RTCHOLD;  					// RTC hold

	RTCMON = *uc8TimeBuffer++;              // Month = 0x10 = October
	RTCDAY = *uc8TimeBuffer++;              // Day = 0x07 = 7th
	RTCYEAR = *uc8TimeBuffer;      			// Year = 0x2011

	RTCCTL01 &= ~RTCHOLD;                   // Start RTC calendar mode
}

void RTC_Set_Alarm(unsigned char uc8PollingInterval)
{
	unsigned int Temperary_time = 0;

	// Configure RTC_B
	RTCCTL01 |= RTCHOLD;  					// RTC hold

	Temperary_time = (((RTCMIN >> 4) & 0x0F) * 10) + (RTCMIN & 0x0F);
	Temperary_time += uc8PollingInterval;

	if(Temperary_time > 59){
		Temperary_time -= 60;
		RTCAMIN = (Temperary_time / 10) << 4;
		RTCAMIN |= (Temperary_time % 10);
	}
	else{
		RTCAMIN = (Temperary_time / 10) << 4;
		RTCAMIN |= (Temperary_time % 10);
	}

	RTCAMIN |= RTCAE;

	RTCCTL01 &= ~RTCHOLD;                     // Start RTC calendar mode
}

void RTC_Disable_Alarm()
{
	// Configure RTC_B
	RTCCTL01 |= RTCHOLD;  					// RTC hold

	RTCAMIN &= ~RTCAE;

	RTCCTL01 &= ~RTCHOLD;                     // Start RTC calendar mode
}

#pragma vector=RTC_VECTOR
__interrupt void RTCISR(void)
{
  switch (__even_in_range(RTCIV, RTCIV_RTCOFIFG)){
    case RTCIV_NONE: break;
    case RTCIV_RTCRDYIFG: break;
    case RTCIV_RTCTEVIFG:		// Should fire and be here once ever minute
    	__no_operation();
    	LPM3_EXIT;

      break;
    case RTCIV_RTCAIFG:		// Alarm Flag
    	RTC_Set_Alarm(g_ui8PollingInterval);
    	g_uc8PollingWakeupFlag = 1;
    	LPM3_EXIT;

    	break;
    case RTCIV_RT0PSIFG: break;
    case RTCIV_RT1PSIFG: break;
    case RTCIV_RTCOFIFG: break;
  }
}

