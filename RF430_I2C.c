/*
 * {RF430_I2C.c}
 *
 * {I2C Interface for RF430CL331 Dynamic NFC Tag}
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

#include "RF430_I2C.h"

unsigned char ui8RxData[2] = {0,0};
unsigned char ui8TxData[4] = {0,0,0,0};
unsigned char ui8TxAddr[4] = {0,0};

unsigned char  *PTxData;                     // Pointer to TX data
unsigned char *PRxData;                     // Pointer to RX data
unsigned int TXByteCtr;
unsigned int RXByteCtr;

unsigned char RF430_I2C_State;
unsigned char RF430_I2C_Start = 0;			//AK 10-23-2013


//------------------------------------------------------------------------------
//
// RF430 I2C Write
//
//------------------------------------------------------------------------------
void RF430_I2C_Write(unsigned char  * data, unsigned int length, unsigned int cont)
{
    PTxData = (unsigned char  *)data;      // TX array start address

    TXByteCtr = length;

    if (cont == START) {
        RF430_I2C_Start = SET;					// need a way of knowing when a start condition has been set in the ISR AK 10-23-2013
        UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    }
    else {
        UCB0CTL1 |= UCTR;                       // I2C TX
        UCB0IFG |= UCTXIFG;                     // Kickoff IFG since START not sent
    }

    __bis_SR_register(LPM_MODE + GIE);     // Enter LPM, enable interrupts
    __no_operation();                       // Remain in LPM until all data
                                            // is TX'd
    RF430_I2C_Start = CLEAR;				// this should be cleared in the interrupt, but to be sure it is also cleared here, AK-1023-2013

    if (!(RF430_I2C_State == I2C_NACK_RCVD))
    {
        while (TXByteCtr != 0);          // Ensure stop condition got sent
    }

}


unsigned char RF430_I2C_Write_Restart_Read(unsigned char  * tx_data, unsigned int tx_length, unsigned char* rx_data, unsigned int rx_length)
{
    RF430_I2C_State = I2C_TRANSMIT;

    PTxData = (unsigned char  *)tx_data;     // TX array start address
    TXByteCtr = tx_length;

    RF430_I2C_Start = SET;					// need a way of knowing when a start condition has been set in the ISR AK 10-23-2013
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C Master Transmitter Mode, Generate START Condition

    if(RF430_I2C_State == I2C_NACK_RCVD)    // If RF430 NACKed
    {
        return FAIL;
    }

    __bis_SR_register(LPM_MODE + GIE);     // Enter LPM, enable interrupts
    __no_operation();                       // Remain in LPM until all data

    RF430_I2C_Start = CLEAR;				// this should be cleared in the interrupt, but to be sure it is also cleared here, AK-1023-2013

    if(RF430_I2C_State == I2C_NACK_RCVD)    // If RF430 NACKed
    {
        return FAIL;
    }

    PRxData = (unsigned char *)rx_data;           // Start of RX buffer
    RXByteCtr = rx_length;                          // Load RX byte counter

    UCB0CTL1 &= ~UCTR;                      // I2C Read
    RF430_I2C_Start = CLEAR;				// only needed on transmit and start condition --AK 10-23-2013
    UCB0CTL1 |= UCTXSTT;                    // I2C start condition

    __bis_SR_register(LPM_MODE + GIE);     // Enter LPM, enable interrupts
                                            // Remain in LPM until all data
                                            // is RX'd
    RF430_I2C_Start = CLEAR;				// this should be cleared in the interrupt, but to be sure it is also cleared here, AK-1023-2013
    __no_operation();                       // Set breakpoint >>here<< and
                                            // read out the RxBuffer buffer
    return PASS;
}

//------------------------------------------------------------------------------
//
// RF430 I2C Send Stop
//
//------------------------------------------------------------------------------
void RF430_I2C_Send_Stop()
{
	__delay_cycles(100);		//AK 10-23-2013
	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	__enable_interrupt();
	while (!(UCB0CTL1 & UCTXSTP));             // Ensure stop condition got sent
	__disable_interrupt();
}

//------------------------------------------------------------------------------
//
// USCI B0 ISR - RF430 I2C
//
//------------------------------------------------------------------------------
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
  switch(__even_in_range(UCB0IV,12))
  {
  case  0: break;                           // Vector  0: No interrupts
  case  2: break;                           // Vector  2: ALIFG
  case  4: //break;                         // Vector  4: NACKIFG
    RF430_I2C_State = I2C_NACK_RCVD;
    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
    UCB0IFG = 0;                            // Clear All USCI_B0 flags
    __bic_SR_register_on_exit(LPM_MODE);    // Exit LPM
    break;
  case  6: break;                           // Vector  6: STTIFG
  case  8: break;                           // Vector  8: STPIFG
  case 0x16:                                  // Vector 10: RXIFG
    RXByteCtr--;                            // Decrement RX byte counter
    if (RXByteCtr)
    {
      *PRxData++ = UCB0RXBUF;               // Move RX data to address PRxData
      if (RXByteCtr == 1)
        UCB0CTL1 |= UCTXSTP;                // Generate I2C stop condition on next RX
    }
    else
    {
      *PRxData = UCB0RXBUF;                 // Move final RX data to PRxData
      __bic_SR_register_on_exit(LPM_MODE); // Exit active CPU
    }
    break;
  case 0x18:                                  // Vector 12: TXIFG
    if (TXByteCtr)                          // Check TX byte counter
    {
      unsigned int delay = 0;
      UCB0TXBUF = *PTxData++;               // Load TX buffer
      TXByteCtr--;                          // Decrement TX byte counter
      if(TXByteCtr == 1)
      {
    	  while((delay < 9000) && (UCB0CTL1 & UCTXSTT) && (RF430_I2C_Start == SET)) //only go into this loop when a start condition has been requested AK 10-23-2013
    	  {
    		  __delay_cycles(40);
    		  delay += 40;
    	  }
		  if((UCB0CTL1 & UCTXSTT) && (RF430_I2C_Start == SET))            // Still Waiting?  --only when start condition is expected, AK 10-23-2013
		  {
			  RF430_I2C_State = I2C_NACK_RCVD;
			  __bic_SR_register_on_exit(LPM_MODE); // Exit LPM
		  }
      }
    }
    else
    {
      UCB0IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
      __bic_SR_register_on_exit(LPM_MODE); // Exit LPM
    }
    RF430_I2C_Start = CLEAR;			//Clear after the first TX, start has been passed		//AK 10-23-2013
    break;
  default:
    __no_operation();
    break;
  }
}

