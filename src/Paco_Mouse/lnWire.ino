#ifdef USE_LNWIRE
/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/

/****************************************************************************
    Copyright (C) 2009 to 2013 Alex Shepherd
    Copyright (C) 2020 Damian Philipp

    Portions Copyright (C) Digitrax Inc.
    Portions Copyright (C) Uhlenbrock Elektronik GmbH

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *****************************************************************************
     DESCRIPTION
    This module provides functions that manage the sending and receiving of LocoNet packets.
    
    Retailed and adapted to PacoMouse to reduce memory space

 *****************************************************************************/


// Common defines
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define LN_BIT_PERIOD               (F_CPU / 16666)
#define LN_TMR_PRESCALER            1
#define LN_TIMER_TX_RELOAD_ADJUST   106               //  14,4 us delay borrowed from FREDI sysdef.h
#define LN_TX_RETRIES_MAX           25

#define LN_RX_PORT  PINB
#define LN_RX_DDR   DDRB
#ifdef PB0                                            // bug/missing defines (some hardware core's ioXX.h files define one or the other)
#define LN_RX_BIT   PB0
#else
#define LN_RX_BIT   PORTB0
#endif

#define LN_SB_SIGNAL          TIMER1_CAPT_vect
#define LN_TMR_SIGNAL         TIMER1_COMPA_vect

#define LN_SB_INT_ENABLE_REG  TIMSK1                  //Timer/Counter1 Interrupt Mask Register
#define LN_SB_INT_ENABLE_BIT  ICIE1                   //Timer/Counter1, Input Capture Interrupt Enable
#define LN_SB_INT_STATUS_REG  TIFR1                   //Timer/Counter1 Interrupt Flag Register
#define LN_SB_INT_STATUS_BIT  ICF1                    //Timer/Counter1, Input Capture Flag

#define LN_SB_INT_ENABLE_REG  TIMSK1                  //Timer/Counter1 Interrupt Mask Register
#define LN_SB_INT_ENABLE_BIT  ICIE1                   //Timer/Counter1, Input Capture Interrupt Enable

#define LN_TMR_INT_ENABLE_REG TIMSK1
#define LN_TMR_INT_STATUS_REG TIFR1
#define LN_TMR_INT_ENABLE_BIT OCIE1A
#define LN_TMR_INT_STATUS_BIT OCF1A
#define LN_TMR_INP_CAPT_REG   ICR1                    // [BA040319] added defines for:
#define LN_TMR_OUTP_CAPT_REG  OCR1A                   // ICR1, OCR1A, TCNT1, TCCR1B
#define LN_TMR_COUNT_REG      TCNT1                   // and replaced their occurence in
#define LN_TMR_CONTROL_REG    TCCR1B                  // the code.
#define LN_INIT_COMPARATOR() { TCCR1A = 0; TCCR1B = 0x01; }    // no prescaler, normal mode

#define LN_SW_UART_SET_TX_LOW(LN_TX_PORT, LN_TX_BIT)  LN_TX_PORT |= (1 << LN_TX_BIT)    // to pull down LN line to drive low level
#define LN_SW_UART_SET_TX_HIGH(LN_TX_PORT, LN_TX_BIT) LN_TX_PORT &= ~(1 << LN_TX_BIT)   // master pull up will take care of high LN level
#define IS_LN_COLLISION()  (((LN_TX_PORT >> LN_TX_BIT) & 0x01) == ((LN_RX_PORT >> LN_RX_BIT) & 0x01))

#define LN_ST_IDLE            0   // net is free for anyone to start transmission
#define LN_ST_CD_BACKOFF      1   // timer interrupt is counting backoff bits
#define LN_ST_TX_COLLISION    2   // just sending break after creating a collision
#define LN_ST_TX              3   // transmitting a packet
#define LN_ST_RX              4   // receiving bytes

#define LN_COLLISION_TICKS 15
#define LN_TX_RETRIES_MAX  25

// CD Backoff starts after the Stop Bit (Bit 9) and has a minimum or 20 Bit Times
// but initially starts with an additional 20 Bit Times
#define   LN_CARRIER_TICKS      20  // carrier detect backoff - all devices have to wait this
#define   LN_MASTER_DELAY        6  // non master devices have to wait this additionally
#define   LN_INITIAL_PRIO_DELAY 20                                      // initial attempt adds priority delay
#define   LN_BACKOFF_MIN      (LN_CARRIER_TICKS + LN_MASTER_DELAY)      // not going below this
#define   LN_BACKOFF_INITIAL  (LN_BACKOFF_MIN + LN_INITIAL_PRIO_DELAY)  // for the first normal tx attempt
#define   LN_BACKOFF_MAX      (LN_BACKOFF_INITIAL + 10)                 // lower priority is not supported

// The Start Bit period is a full bit period + half of the next bit period
// so that the bit is sampled in middle of the bit
#define LN_TIMER_RX_START_PERIOD    LN_BIT_PERIOD + (LN_BIT_PERIOD / 2)
#define LN_TIMER_RX_RELOAD_PERIOD   LN_BIT_PERIOD
#define LN_TIMER_TX_RELOAD_PERIOD   LN_BIT_PERIOD

volatile uint8_t  lnState ;
volatile uint8_t  lnBitCount ;
volatile uint8_t  lnCurrentByte ;
volatile uint16_t lnCompareTarget ;

volatile lnMsg    * volatile lnTxData ;
volatile uint8_t  lnTxIndex ;
volatile uint8_t  lnTxLength ;
volatile uint8_t  lnTxSuccess ;   // this boolean flag as a message from timer interrupt to send function

volatile uint8_t  *txPort;
uint8_t           txPin;

#define LN_TX_PORT *txPort
#define LN_TX_BIT txPin

volatile uint8_t  uartFIFO[128];
volatile uint8_t  lastInUART;
volatile uint8_t  firstOutUART;
volatile uint8_t  cntUART;
lnMsg    uartBuf ;
uint8_t  uartIndex ;


/**************************************************************************

   Start Bit Interrupt Routine

   DESCRIPTION
   This routine is executed when a falling edge on the incoming serial
   signal is detected. It disables further interrupts and enables
   timer interrupts (bit-timer) because the UART must now receive the
   incoming data.

 **************************************************************************/

ISR(LN_SB_SIGNAL)
{
  // Disable the Input Comparator Interrupt
  cbi( LN_SB_INT_ENABLE_REG, LN_SB_INT_ENABLE_BIT );

  // Get the Current Timer1 Count and Add the offset for the Compare target
  lnCompareTarget = LN_TMR_INP_CAPT_REG + LN_TIMER_RX_START_PERIOD ;
  LN_TMR_OUTP_CAPT_REG = lnCompareTarget ;

  // Clear the current Compare interrupt status bit and enable the Compare interrupt
  sbi(LN_TMR_INT_STATUS_REG, LN_TMR_INT_STATUS_BIT) ;
  sbi(LN_TMR_INT_ENABLE_REG, LN_TMR_INT_ENABLE_BIT) ;

  // Set the State to indicate that we have begun to Receive
  lnState = LN_ST_RX ;

  // Reset the bit counter so that on first increment it is on 0
  lnBitCount = 0;
}


/**************************************************************************

   Timer Tick Interrupt

   DESCRIPTION
   This routine coordinates the transmition and reception of bits. This
   routine is automatically executed at a rate equal to the baud-rate. When
   transmitting, this routine shifts the bits and sends it. When receiving,
   it samples the bit and shifts it into the buffer.

 **************************************************************************/
ISR(LN_TMR_SIGNAL)     /* signal handler for timer0 overflow */
{
  // Advance the Compare Target by a bit period
  lnCompareTarget += LN_TIMER_RX_RELOAD_PERIOD;
  LN_TMR_OUTP_CAPT_REG = lnCompareTarget;

  lnBitCount++;                // Increment bit_counter
  if ( lnState == LN_ST_RX ) { // Are we in RX mode
    if ( lnBitCount < 9)  {  // Are we in the Stop Bits phase
      lnCurrentByte >>= 1;
      if ( bit_is_set(LN_RX_PORT, LN_RX_BIT)) {
        lnCurrentByte |= 0x80;
      }
      return ;
    }
    // Clear the Start Bit Interrupt Status Flag and Enable ready to
    // detect the next Start Bit
    sbi( LN_SB_INT_STATUS_REG, LN_SB_INT_STATUS_BIT ) ;
    sbi( LN_SB_INT_ENABLE_REG, LN_SB_INT_ENABLE_BIT ) ;
    // If the Stop bit is not Set then we have a Framing Error
    if ( bit_is_clear(LN_RX_PORT, LN_RX_BIT) ) {
      // ERROR_LED_ON();
    }
    else { // Put the received byte in the buffer
      // // // // // addByteLnBuf( lnRxBuffer, lnCurrentByte ) ;
      uartWriteFIFO(lnCurrentByte);
    }
    lnBitCount = 0 ;
    lnState = LN_ST_CD_BACKOFF ;
  }
  if ( lnState == LN_ST_TX ) {  // Are we in the TX State
    // To get to this point we have already begun the TX cycle so we need to
    // first check for a Collision.
    if ( IS_LN_COLLISION() ) {       // Collision?
      lnBitCount = 0 ;
      lnState = LN_ST_TX_COLLISION ;
      // ERROR_LED_ON();
    }
    else if ( lnBitCount < 9) {        // Send each Bit
      if ( lnCurrentByte & 0x01 ) {
        LN_SW_UART_SET_TX_HIGH(LN_TX_PORT, LN_TX_BIT);
      }
      else {
        LN_SW_UART_SET_TX_LOW(LN_TX_PORT, LN_TX_BIT);
      }
      lnCurrentByte >>= 1;
    }
    else if ( lnBitCount ==  9) {      // Generate stop-bit
      LN_SW_UART_SET_TX_HIGH(LN_TX_PORT, LN_TX_BIT);
    }
    else if ( ++lnTxIndex < lnTxLength ) { // Any more bytes in buffer
      // Setup for the next byte
      lnBitCount = 0 ;
      lnCurrentByte = lnTxData->data[ lnTxIndex ] ;
      // Begin the Start Bit
      LN_SW_UART_SET_TX_LOW(LN_TX_PORT, LN_TX_BIT);
      // Get the Current Timer1 Count and Add the offset for the Compare target
      // added adjustment value for bugfix (Olaf Funke)
      lnCompareTarget = LN_TMR_COUNT_REG + LN_TIMER_TX_RELOAD_PERIOD - LN_TIMER_TX_RELOAD_ADJUST;
      LN_TMR_OUTP_CAPT_REG = lnCompareTarget ;
    }
    else {
      // Successfully Sent all bytes in the buffer
      // so set the Packet Status to Done
      lnTxSuccess = 1 ;
      // Now copy the TX Packet into the RX Buffer
      for (lnCurrentByte = 0; lnCurrentByte < lnTxLength; lnCurrentByte++)  
        uartWriteFIFO(lnTxData->data[ lnCurrentByte ]);
      // // // // // addMsgLnBuf( lnRxBuffer, lnTxData ); 
      // Begin CD Backoff state
      lnBitCount = 0 ;
      lnState = LN_ST_CD_BACKOFF ;
    }
  }
  // Note we may have got here from a failed TX cycle, if so BitCount will be 0
  if ( lnState == LN_ST_TX_COLLISION ) {
    if ( lnBitCount == 0 ) {
      // Pull the TX Line low to indicate Collision
      LN_SW_UART_SET_TX_LOW(LN_TX_PORT, LN_TX_BIT);
      // ERROR_LED_ON();
    }
    else if ( lnBitCount >= LN_COLLISION_TICKS ) {
      // Release the TX Line
      LN_SW_UART_SET_TX_HIGH(LN_TX_PORT, LN_TX_BIT);
      // ERROR_LED_OFF();
      lnBitCount = 0 ;
      lnState = LN_ST_CD_BACKOFF ;
    }
  }
  if ( lnState == LN_ST_CD_BACKOFF ) {
    if ( lnBitCount == 0 ) {
      // Even though we are waiting, other nodes may try and transmit early
      // so Clear the Start Bit Interrupt Status Flag and Enable ready to
      // detect the next Start Bit
      sbi( LN_SB_INT_STATUS_REG, LN_SB_INT_STATUS_BIT ) ;
      sbi( LN_SB_INT_ENABLE_REG, LN_SB_INT_ENABLE_BIT ) ;
    }
    else if ( lnBitCount >= LN_BACKOFF_MAX ) {
      // declare network to free after maximum backoff delay
      lnState = LN_ST_IDLE ;
      cbi( LN_TMR_INT_ENABLE_REG, LN_TMR_INT_ENABLE_BIT ) ;
    }
  }
}


LN_STATUS sendLocoNetPacketTry(lnMsg *TxData, unsigned char ucPrioDelay) {
  uint8_t  CheckSum ;
  uint8_t  CheckLength ;

  lnTxLength = getLnMsgSize( TxData ) ;
  // First calculate the checksum as it may not have been done
  CheckLength = lnTxLength - 1 ;
  CheckSum = 0xFF ;
  for ( lnTxIndex = 0; lnTxIndex < CheckLength; lnTxIndex++ ) {
    CheckSum ^= TxData->data[ lnTxIndex ] ;
  }
  TxData->data[ CheckLength ] = CheckSum ;
  // clip maximum prio delay
  if (ucPrioDelay > LN_BACKOFF_MAX) {
    ucPrioDelay = LN_BACKOFF_MAX;
  }
  // if priority delay was waited now, declare net as free for this try
  cli();  // disabling interrupt to avoid confusion by ISR changing lnState while we want to do it
  if (lnState == LN_ST_CD_BACKOFF) {
    if (lnBitCount >= ucPrioDelay) {  // Likely we don't want to wait as long as
      lnState = LN_ST_IDLE;     // the timer ISR waits its maximum delay.
      cbi( LN_TMR_INT_ENABLE_REG, LN_TMR_INT_ENABLE_BIT ) ;
    }
  }
  sei();  // a delayed start bit interrupt will happen now,
  // a delayed timer interrupt was stalled
  // If the Network is not Idle, don't start the packet
  if (lnState == LN_ST_CD_BACKOFF) {
    if (lnBitCount < LN_CARRIER_TICKS) {  // in carrier detect timer?
      return LN_CD_BACKOFF;
    }
    else {
      return LN_PRIO_BACKOFF;
    }
  }
  if ( lnState != LN_ST_IDLE ) {
    return LN_NETWORK_BUSY;  // neither idle nor backoff -> busy
  }
  // We need to do this with interrupts off.
  // The last time we check for free net until sending our start bit
  // must be as short as possible, not interrupted.
  cli() ;
  // Before we do anything else - Disable StartBit Interrupt
  cbi( LN_SB_INT_ENABLE_REG, LN_SB_INT_ENABLE_BIT ) ;
  if (bit_is_set(LN_SB_INT_STATUS_REG, LN_SB_INT_STATUS_BIT)) {
    // first we disabled it, than before sending the start bit, we found out
    // that somebody was faster by examining the start bit interrupt request flag
    sbi( LN_SB_INT_ENABLE_REG, LN_SB_INT_ENABLE_BIT ) ;
    sei() ;  // receive now what our rival is sending
    return LN_NETWORK_BUSY;
  }
  LN_SW_UART_SET_TX_LOW(LN_TX_PORT, LN_TX_BIT);        // Begin the Start Bit
  // Get the Current Timer1 Count and Add the offset for the Compare target
  // added adjustment value for bugfix (Olaf Funke)
  lnCompareTarget = LN_TMR_COUNT_REG + LN_TIMER_TX_RELOAD_PERIOD - LN_TIMER_TX_RELOAD_ADJUST;
  LN_TMR_OUTP_CAPT_REG = lnCompareTarget ;
  sei() ;  // Interrupts back on ...
  lnTxData = TxData ;
  lnTxIndex = 0 ;
  lnTxSuccess = 0 ;
  // Load the first Byte
  lnCurrentByte = TxData->data[ 0 ] ;
  // Set the State to Transmit
  lnState = LN_ST_TX ;
  // Reset the bit counter
  lnBitCount = 0 ;
  // Clear the current Compare interrupt status bit and enable the Compare interrupt
  sbi(LN_TMR_INT_STATUS_REG, LN_TMR_INT_STATUS_BIT) ;
  sbi(LN_TMR_INT_ENABLE_REG, LN_TMR_INT_ENABLE_BIT) ;
  while (lnState == LN_ST_TX) {
    // now busy wait until the interrupts do the rest
  }
  if (lnTxSuccess) {
    return LN_DONE;
  }
  if (lnState == LN_ST_TX_COLLISION) {
    return LN_COLLISION;
  }
  return LN_UNKNOWN_ERROR; // everything else is an error
}


LN_STATUS  lnetSend (lnMsg* pPacket) {
  unsigned char ucTry;
  LN_STATUS enReturn;
  unsigned char ucWaitForEnterBackoff;
  uint8_t ucPrioDelay;

  ucPrioDelay = LN_BACKOFF_INITIAL;
  for (ucTry = 0; ucTry < LN_TX_RETRIES_MAX; ucTry++)   {
    // wait previous traffic and than prio delay and than try tx
    ucWaitForEnterBackoff = 1;  // don't want to abort do/while loop before
    do                          // we did not see the backoff state once
    {
      enReturn = sendLocoNetPacketTry(pPacket, ucPrioDelay);
      if (enReturn == LN_DONE)  // success?
        return LN_DONE;
      if (enReturn == LN_PRIO_BACKOFF)
        ucWaitForEnterBackoff = 0; // now entered backoff -> next state != LN_BACKOFF is worth incrementing the try counter
    } while ((enReturn == LN_CD_BACKOFF) ||                             // waiting CD backoff
             (enReturn == LN_PRIO_BACKOFF) ||                           // waiting master+prio backoff
             ((enReturn == LN_NETWORK_BUSY) && ucWaitForEnterBackoff)); // or within any traffic unfinished
    // failed -> next try going to higher prio = smaller prio delay
    if (ucPrioDelay > LN_BACKOFF_MIN)
      ucPrioDelay--;
  }
  return LN_RETRY_ERROR;
}


void beginLnWire() {
  pinMode(pinLN_TXD, OUTPUT);
  // Not figure out which Port bit is the Tx Bit from the Arduino pin number
  uint8_t  bitMask = digitalPinToBitMask(pinLN_TXD);
  uint8_t  bitMaskTest = 0x01;
  uint8_t  bitNum = 0;
  uint8_t  port = digitalPinToPort(pinLN_TXD);
  txPort = portOutputRegister(port);
  while (bitMask != bitMaskTest)
    bitMaskTest = 1 << ++bitNum;
  txPin = bitNum;
  // Set the RX line to Input
  cbi( LN_RX_DDR, LN_RX_BIT ) ;
  // Set the TX line to Inactive
  LN_SW_UART_SET_TX_HIGH(LN_TX_PORT, LN_TX_BIT);
  LN_INIT_COMPARATOR();
  lnState = LN_ST_IDLE ;
  //Clear StartBit Interrupt flag
  sbi( LN_SB_INT_STATUS_REG, LN_SB_INT_STATUS_BIT );
  //Enable StartBit Interrupt
  sbi( LN_SB_INT_ENABLE_REG, LN_SB_INT_ENABLE_BIT );
  // Set Timer Clock Source
  LN_TMR_CONTROL_REG = (LN_TMR_CONTROL_REG & 0xF8) | LN_TMR_PRESCALER;
  uartInitFIFO();
}


////////////////////////////////////////////////////////////
// ***** BUFFER *****
////////////////////////////////////////////////////////////

void uartInitFIFO() {
  lastInUART = 0;
  firstOutUART = 0;
  cntUART = 0;
  uartIndex = 0;
}


uint8_t uartReadFIFO () {
  firstOutUART = (firstOutUART + 1 ) & 0x7F;                      // next one (hardcoded)
  cntUART--;
  return (uartFIFO[firstOutUART]);
}


void uartWriteFIFO (uint8_t value) {
  lastInUART = (lastInUART + 1 ) & 0x7F;                          // next one (hardcoded)
  cntUART++;
  uartFIFO[lastInUART] = value;                                   // save in FIFO
}


lnMsg* lnetReceive() {
  byte newByte, chk;
  while (cntUART > 0) {
    newByte  = uartReadFIFO();
    if (newByte & 0x80) {                                           // new opcode
      uartIndex = 0;
      chk = 0xFF;
    }
    uartBuf.data[uartIndex++] = newByte ;
    chk ^= newByte;
    if (uartIndex > 1) {
      if (uartIndex == getLnMsgSize(&uartBuf)) {
        uartIndex = 0;                                            // message received
        return (chk == 0) ? &uartBuf : NULL;                      // return packet if checksum is good
      }
    }
  }
  return NULL;
}





void lnetProcess() {
  RecvPacket = lnetReceive();
  if (RecvPacket)
    lnetDecode(RecvPacket);
  lnetTimers();
}



#endif
