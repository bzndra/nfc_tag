/*
 * {main.c}
 *
 * {Cold Chain Datalogger Demo High Level Flow}
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

//******************************************************************************
//
//  Description: This firmware demonstrates Cold Chain data logging.
//
//
// Built with CCSv6.0.1
//******************************************************************************

//?********Release Version 1.0 ***********?//


#include "mcu.h"
unsigned char Cmd = 0;	//Command byte for SPI mode
unsigned char read_complete = 0;
unsigned char rx_byte_count = 0;
unsigned char tx_byte_count = 0;
unsigned int Results[11] = {0,0,0,0,0,0,0,0,0,0,0};

unsigned char u8Ticks = 0;

unsigned int g_TempDataFahr;
unsigned int g_TempDataCel;
char g_TempNegFlagCel = 0;
char g_TempNegFlagFahr = 0;

unsigned char button_pressed = 0;
unsigned char uc8_CL331H_INTO_flag = 0;
unsigned char g_uc8PollingWakeupFlag = 0;


extern unsigned char gp_DLFullFlag; // Handle buffer loop

void main (void)
{
	volatile unsigned int test = 0;
	volatile unsigned int flags = 0;
	unsigned int ui16Temperature;
	unsigned int ui16Light;
	unsigned int ui16Humidity;

	MSP430_Init();

	SENSOR_POWER_OFF;
	RF_FIELD_INT_ENABLE;

	RTC_Init();

	while (1)
    {
		switch(State_t){
			case Start:
				if((NdefFiles[0].FileID[0] != 0xE1) || (NdefFiles[0].FileID[1] != 0x03) ||
				   (NdefFiles[1].FileID[0] != 0xE1) || (NdefFiles[1].FileID[1] != 0x04)){
					// First Time Programed
					State_t = Data_logger_Init;
				}
				else{  // Power was lost
					State_t = Power_Lost;
				}
				break;

			case Power_Lost:
				if(g_ui8ResetStateFlag){
					DL_Reload_Length();
				}
				else{
					DL_Print_Power_Lost();  		// Indicate Power was lost for unkown time
				}

				State_t = Wait_For_Command;
				break;

			case Data_logger_Init:		//Set default Settings
				ResetDatalogger();

				g_Datalogger_Mode_t = Temp_Only;
				g_ui8PollingInterval = 10;  // Polling interval in Minutes
				g_uc8DataloggingEnableFlag = 0;

				g_ui8TemperatureModeFlag = Fahrenheit;

				g_uc8PollingWakeupFlag = 0;				// Take First Sample
				Command_Received_t = None;
				g_ui8ResetStateFlag = 1;

				RTC_Init();  // Reset Time

				State_t = Wait_For_Command;
				break;

			case Wait_For_Command:
				__no_operation();
		    	if(uc8_CL331H_INTO_flag)
		    	{
					flags = CL331H_Read_Register(INT_FLAG_REG); //read the flag register to check if a read or write occurred
					__no_operation();
					do
		    		{
						if (flags)
						{
							if(ServiceInterrupt(flags)){
								State_t = Process_command;
								g_uc8SleepBetweenLoopsFlag = 0;
							}
						}
		    			//interrupts may have occured while servicing a prior interrupt, check again to make sure all are serviced
						flags = CL331H_Read_Register(INT_FLAG_REG); //read the flag register to check if a read or write occurred
		    		}while(flags);

		        	uc8_CL331H_INTO_flag = 0; //we have serviced INTO
		    		//re-enable INTO
		    		PORT_INTO_IFG &= ~INTO;
		    		PORT_INTO_IE |= INTO;

					__no_operation();
		    	}
				break;

			case Process_command:

				g_uc8SleepBetweenLoopsFlag = 1;
				ProcessReceivedFileText(&Command_Received_t);

				switch(Command_Received_t){
					case Reset_Command:
						State_t = Data_logger_Init;
						g_ui8ResetStateFlag = 1;
						break;

					case Start_Command:
						g_uc8DataloggingEnableFlag = 1;  		// Start Datalogging
						g_uc8PollingWakeupFlag = 1;				// Take First Sample
						g_ui8ResetStateFlag = 0;

						RTC_Set_Alarm(g_ui8PollingInterval);	// Start Wakeup timer from this point
						State_t = Wait_For_Command;
						break;

					case Stop_Command:
						g_uc8DataloggingEnableFlag = 0;  		// Stop Datalogging
						RTC_Disable_Alarm();
						State_t = Wait_For_Command;
						g_ui8ResetStateFlag = 0;
						break;

					case Clear_Data_Command:
						ResetDatalogger();
						State_t = Wait_For_Command;
						g_ui8ResetStateFlag = 0;
						break;

					case Set_Time:
						FetchReceivedTime(uc8ReceivedData);
						RTC_Set_Time(uc8ReceivedData);
						State_t = Wait_For_Command;
						g_ui8ResetStateFlag = 0;
						break;

					case Set_Date:
						FetchReceivedDate(uc8ReceivedData);
						RTC_Set_Date(uc8ReceivedData);
						State_t = Wait_For_Command;
						g_ui8ResetStateFlag = 0;
						break;

					case Set_Polling_Interval:
						FetchReceivedPollingInterval(uc8ReceivedData);
						g_ui8PollingInterval = uc8ReceivedData[0];
						RTC_Set_Alarm(g_ui8PollingInterval);
						g_ui8ResetStateFlag = 0;
						//TODO Update Polling Interval with new data

						State_t = Wait_For_Command;
						break;

					case Set_Mode:
						FetchReceivedMode(&g_Datalogger_Mode_t);
						g_ui8ResetStateFlag = 0;

						State_t = Wait_For_Command;
						break;

					case Set_Temp_Mode:
						FetchReceivedTempMode(&g_ui8TemperatureModeFlag);
						g_ui8ResetStateFlag = 0;

						State_t = Wait_For_Command;
						break;

					default:  // Unknown command
						State_t = Wait_For_Command;
						break;
				}

				Command_Received_t = None;
				break;
		}

		if(g_uc8DataloggingEnableFlag && g_uc8PollingWakeupFlag)
		{
			//Flag from RTC
			g_uc8PollingWakeupFlag = 0;

			if(!gp_DLFullFlag){
				SENSOR_POWER_ON;
				Low_Power_Delay_ms(30);

				switch(g_Datalogger_Mode_t){
				case Temp_Only:
					TMP_Get_Temp(&ui16Temperature, &ui8TemperatureNegFlag, g_ui8TemperatureModeFlag);

					DL_Update(g_Datalogger_Mode_t, ui16Temperature, 0, 0);
					break;

				case Temp_and_Light:
					TMP_Get_Temp(&ui16Temperature, &ui8TemperatureNegFlag, g_ui8TemperatureModeFlag);

					StartLuxMeasurement();
					Low_Power_Delay_ms(125);
					ui16Light = GetLuxValue();

					DL_Update(g_Datalogger_Mode_t, ui16Temperature, ui16Light, 0);
					break;

				case Temp_and_Humidity:
					TMP_Get_Temp(&ui16Temperature, &ui8TemperatureNegFlag, g_ui8TemperatureModeFlag);

					ui16Humidity = HDC_Get_Humidity();

					DL_Update(g_Datalogger_Mode_t, ui16Temperature, 0, ui16Humidity);
					break;

				case Temp_Light_and_Humidity:
					TMP_Get_Temp(&ui16Temperature, &ui8TemperatureNegFlag, g_ui8TemperatureModeFlag);

					StartLuxMeasurement();
					Low_Power_Delay_ms(125);
					ui16Light = GetLuxValue();

					ui16Humidity = HDC_Get_Humidity();

					DL_Update(g_Datalogger_Mode_t, ui16Temperature, ui16Light, ui16Humidity);
					break;
				}
			}
			else{
				//Data Logger Memory is full.  Stop Dataloggin
				g_uc8DataloggingEnableFlag = 0;
				RTC_Disable_Alarm();
			}
		}

		if(gui8RfFieldDetected){
			if(!gui8Rf430Initalized){ // Only want to reset/init RF430 once each power cycle.
				SENSOR_POWER_ON;
				ResetRF430();
				RF430CL331H_Init();

				gui8Rf430Initalized = 1;
			}
		}
		else{
			//Set RF430 Reset low
			// Need "Low power settings function"

			if(gui8Rf430Initalized){
				P2OUT &= ~BIT6;
				Low_Power_Delay_ms(5);
			}

			gui8Rf430Initalized = 0;
			SENSOR_POWER_OFF;
		}

		if(g_uc8SleepBetweenLoopsFlag){
			__bis_SR_register(LPM3_bits + GIE); //go to low power mode and enable interrupts. Here we are waiting for an RF read or write
    		__no_operation();
		}
		else{
			g_uc8SleepBetweenLoopsFlag = 1;
		}

    	flags = 0;

    	gui8RfFieldDetected = 0;
    	RF_FIELD_INT_ENABLE;
    }
}


void Low_Power_Delay_secs(unsigned int secs){
	unsigned int i = 0;

	for(i=0; i<secs; i++){
		Low_Power_Delay_ms(1000);
	}
}

void j(){
	WDTCTL = WDTPW + WDTHOLD;		// Turn off Watch Dog

//	//**** Drive unused pins low, for lowest power  ***//
	P1DIR = 0x37;
	P2DIR = 0x9F;
	P3DIR = 0xFF;
	P4DIR = 0xFF;

	P1OUT = 0x00;
	P2OUT = 0x00;
	P3OUT = 0x00;
	P4OUT = 0x00;
//	//*************************************************//

	// Setting up pins for LF Osc for RTC
	PJOUT = 0x00;
	PJSEL0 |= BIT4 | BIT5;
	PJDIR = 0xFFFF;

	// Clock Setup
	PMMCTL0 = PMMPW;				// Open PMM Module
	PM5CTL0 &= ~LOCKLPM5;			// Clear locked IO Pins

	CSCTL0_H = 0xA5;
	CSCTL1 |= DCOFSEL0 + DCOFSEL1;          	// Set max. DCO setting = 8MHz
	CSCTL2 = SELA__VLOCLK + SELS_3 + SELM_3;      	// set ACLK = DCO, VLOCLK = 10kHz
	CSCTL3 = DIVA__1 + DIVS__1 + DIVM__1;      	// set VLOCLK / 1  SMCLK = dco / 32 = 250kHz
	CSCTL4 &= ~LFXTOFF;                       // Enable LFXT1

	do
	{
		CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
		SFRIFG1 &= ~OFIFG;
	}while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
	CSCTL0_H = 0;                             // Lock CS registers

	// Configure Vcc pins for I2C
	P2DIR |= 0x40;					//P2.6 as output
	P2OUT |= 0x40;					//set P2.6 as high

	PORT_I2C_SEL0 &= ~(SCL + SDA);
	PORT_I2C_SEL1 |= SCL + SDA;

	//RST RF430 (in case board is still powered but the MSP430 reset for some reason - MSP430 RST button pushed for example)
	PORT_RST_SEL0 &= ~RST;
	PORT_RST_SEL1 &= ~RST;
	PORT_RST_OUT &= ~RST; 				//RF430CL330H device in reset
	PORT_RST_DIR |= RST;

	//configure pin for INTO interrupts
	PORT_INTO_DIR &= ~INTO; 		//input
	PORT_INTO_OUT &= ~INTO; 		//output low for pull down
	PORT_INTO_REN |= INTO; 			//Enable internal pull down resistor  (external pull down on this board)
	PORT_INTO_IFG &= ~INTO; 		//clear interrupt flag
	PORT_INTO_IES &= ~INTO; 		//fire interrupt on low-to-high transition since INTO is setup High

	//Setup the CL331H I2C Ready input
	CL331H_READY_DIR &= ~CL331H_READY_PIN;		//Set I2C_READY singal to input on MSP430

	//Setup the CL331H I2C Stop input
	CL331H_STOP_DIR &= ~CL331H_STOP_PIN;			//Set I2C_READY singal to input on MSP430


	__enable_interrupt();  // Enable interrupts globally

	//configure interrupt for RF Field Detection
	RF_FIELD_DIR &= ~RF_FIELD_PIN; 		//input
	RF_FIELD_IFG &= ~RF_FIELD_PIN; 		//clear interrupt flag
	RF_FIELD_IES &= ~RF_FIELD_PIN; 	//fire interrupt on low-to-high transition since INTO is setup High

	//Setup control from sensor power
	SENSOR_POWER_DIR |= SENSOR_POWER_PIN;

	RTC_Init();
	Timer_Init();
}

// ISR
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
	__no_operation();
	//INTO interrupt fired
	if(PORT_INTO_IFG & INTO)
	{
		uc8_CL331H_INTO_flag = 1;

		PORT_INTO_IE &= ~INTO; //disable INTO
		PORT_INTO_IFG &= ~INTO; //clear interrupt flag

		LPM3_EXIT;
	}
	else if(RF_FIELD_IFG & RF_FIELD_PIN){
		//RF field is present, enable RF430CL33x

		//SET RF FIELD INTERRUPT FLAG
		gui8RfFieldDetected = 1;

		//DISABLE RF FIELD INTERRUPT
		RF_FIELD_INT_DISABLE;
		RF_FIELD_IFG &= ~RF_FIELD_PIN;

		LPM3_EXIT; //???
	}
}

/*****************Timer Functions*********************/
void Timer_Init(){
	//Timer A
	TA1CCTL0 = CCIE;	                        // TACCR0 interrupt enabled
	TA1CCR0 = 10;
	TA1CTL = TASSEL__ACLK;           			// ACLK = 10 kHz

	//Timer B
	TB0CCTL0 = CCIE;                          	// TBCCR0 interrupt enabled
	TB0CCR0 = 32767;							// Fire at 1 Hz (for time)
	TB0CTL = TBSSEL__ACLK;           			// ACLK = 10KHz, up mode
	TB0CTL |= MC__UP;
}

// Timer A1 interrupt service routine
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0_ISR(void)
{
	// Can update time here
    TA1CTL &= ~(MC_3);		//stops the Timer
	LPM3_EXIT;
}

// Timer B1 "Time" interrupt service routine
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void)
{
//	Update_Time_Counts();
	LPM3_EXIT;
}
/******************************************************/





