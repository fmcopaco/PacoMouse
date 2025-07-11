/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/


#ifdef USE_LOCONET
////////////////////////////////////////////////////////////
// ***** LOCONET SOPORTE *****
////////////////////////////////////////////////////////////

uint8_t getLnMsgSize(volatile lnMsg* Msg) {
  return ((Msg->sz.command & 0x60) == 0x60) ? Msg->sz.mesg_size : ((Msg->sz.command & 0x60) >> 4) + 2;
}

void send4byteMsg (byte opcode, byte slot, byte val) {      // Envia un mensaje de 4 bytes
  SendPacket.data[0] = opcode;
  SendPacket.data[1] = slot;
  SendPacket.data[2] = val;
  lnetSend( &SendPacket);
}

void resumeOperations() {
  SendPacket.data[0] = OPC_GPON;                            //  Track Power On
  lnetSend( &SendPacket);
}

void emergencyOff() {
  SendPacket.data[0] = OPC_GPOFF;                            //  Track Power Off
  lnetSend( &SendPacket);
}

void mueveDesvio() {                                        // mueve el desvio
  lnetRequestSwitch(myTurnout, 1, myPosTurnout - DESVIADO);
  accEnviado = true;
}

void infoDesvio(unsigned int addr) {                        // busca la posicion del desvio
  accStateReq = true;
  addr--;
  send4byteMsg(OPC_SW_STATE, (addr & 0x7F), ((addr >> 7) & 0x0F));
}

void lnetRequestSwitch (unsigned int addr, byte output, byte dir) {
  byte adrH, adrL;
  adrH = ((--addr >> 7) & 0x0F);
  adrL = addr & 0x7F;
  if (output)
    adrH |= OPC_SW_REQ_OUT;                                 // output on
  if (dir)
    adrH |= OPC_SW_REQ_DIR;                                 // direction closed/thrown
  send4byteMsg(OPC_SW_REQ, adrL, adrH);
}

void infoLocomotora (unsigned int address) {
  byte adrH, adrL;
  adrH = (address >> 7) & 0x7F;
  adrL = address & 0x7F;
  send4byteMsg(OPC_LOCO_ADR, adrH, adrL);                   // REQ loco ADR, expect <E7>SLOT READ
}

byte getCurrentStep() {
  byte maxStep, calcStep;
  maxStep = stepsLN[mySlot.state & 0x07];
  if (mySpeed > 1) {
    if (maxStep == 128) {                              // 128 steps -> 0..126
      return (mySpeed - 1);
    }
    else {
      if (maxStep == 28) {                             // 28 steps -> 0..28
        calcStep = ((mySpeed - 2) << 1) / 9;
        return (calcStep + 1);
      }
      else {                                           // 14 steps -> 0..14
        calcStep = (mySpeed - 2) / 9;
        return (calcStep + 1);
      }
    }
  }
  return (0);
}

void nullMoveSlot (byte slot) {
  send4byteMsg(OPC_MOVE_SLOTS, slot, slot);                 // NULL MOVE, expect <E7>SLOT READ or LACK
  pingTimer = millis();
}

void dispatchGet () {
  send4byteMsg(OPC_MOVE_SLOTS, 0, 0);                       // Slot move from slot 0, expect <E7>SLOT READ or LACK
  doDispatchGet = true;
  doDispatchPut = false;
}

void dispatchPut () {
  if (mySlot.num > 0) {
    send4byteMsg(OPC_SLOT_STAT1, mySlot.num, mySlot.state & ~(STAT1_SL_BUSY));    // put slot in Common state
    send4byteMsg(OPC_MOVE_SLOTS, mySlot.num, 0);            // Slot move to slot 0, expect <E7>SLOT READ or LACK
    mySlot.num = 0;
    doDispatchPut = true;
  }
}

void liberaSlot() {
  if (mySlot.num > 0) {
    send4byteMsg(OPC_SLOT_STAT1, mySlot.num, mySlot.state & ~(STAT1_SL_BUSY));    //
    //DEBUG_MSG("Liberando slot %d", mySlot.num);
  }
  mySlot.num = 0;
  myFunc.Bits = 0;
}

void locoOperationSpeed() {                                 // Envia velocidad
  if (mySlot.num > 0) {
    send4byteMsg(OPC_LOCO_SPD, mySlot.num, mySpeed);
    //DEBUG_MSG("Operation Speed: %d", mySpeed);
  }
}

void changeDirectionF0F4() {                                // Envia sentido y F0..F4
  byte fnc;
  if (mySlot.num > 0) {
    fnc =  ((myFunc.xFunc[0] >> 1) & 0x0F);
    if (bitRead(myFunc.xFunc[0], 0))
      fnc |= 0x10;
    if (myDir & 0x80)
      fnc |= 0x20;
    send4byteMsg(OPC_LOCO_DIRF, mySlot.num, fnc);
    //DEBUG_MSG("Dir: %d FncF0F4 %d", myDir >> 7, fnc & 0x1F);
  }
}

void funcOperations (byte fnc) {                            //(variantes posibles: Digitrax, Uhlenbrock y DR5000 - https://wiki.rocrail.net/doku.php?id=loconet:lnpe-parms-en)
  byte data;
  if (mySlot.num > 0) {
    switch (fnc) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
        changeDirectionF0F4();                              // LNPE
        break;
      case 5:
      case 6:
      case 7:
      case 8:
        data =  ((myFunc.xFunc[0] >> 5) & 0x07);            // LNPE
        if (bitRead(myFunc.xFunc[1], 0))
          data |= 0x08;
        send4byteMsg(OPC_LOCO_SND, mySlot.num, data);
        //DEBUG_MSG("FncF5F8 %d", data);
        break;
      default:
        switch (typeCmdStation) {
          case CMD_DR:                                      // DR5000 / IB II
            if (fnc > 12) {
              changeFuncULHI(fnc);
            }
            else {
              data =  ((myFunc.xFunc[1] >> 1) & 0x0F);
              send4byteMsg(0xA3, mySlot.num, data);
              //DEBUG_MSG("DR5000-FncF9F12 %d",  data);
            }
            break;
          case CMD_ULI:                                     // Uhlenbrock
            changeFuncULHI(fnc);
            break;
          case CMD_DIG:                                     // Digitrax
            changeFuncIMM(fnc);
            break;
        }
        break;
    }
  }
}


void changeFuncULHI(byte fnc) {
  byte arg4;
  SendPacket.data[0] = OPC_UHLI_FUN;                        // Uhlenbrock
  SendPacket.data[1] = 0x20;
  SendPacket.data[2] = mySlot.num;
  switch (fnc) {                                            // ---87654 32109876 54321098 76543210
    case 12:
    case 20:
    case 28:
      SendPacket.data[3] = 0x05;
      arg4 = bitRead(myFunc.Bits, 12) << 4;
      arg4 |= bitRead(myFunc.Bits, 20) << 5;
      arg4 |= bitRead(myFunc.Bits, 28) << 6;
      SendPacket.data[4] = arg4 & 0x70;                    // F12,F20,F28
      break;
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
      SendPacket.data[3] = 0x08;
      arg4 =  ((myFunc.xFunc[1] >> 5) & 0x07);
      arg4 |= (myFunc.xFunc[2] << 3);
      SendPacket.data[4] = arg4 & 0x7F;                   // F13..F19
      break;
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
      SendPacket.data[3] = 0x09;
      arg4 =  ((myFunc.xFunc[2] >> 5) & 0x07);
      arg4 |= (myFunc.xFunc[3] << 3);
      SendPacket.data[4] = arg4 & 0x7F;                   // F21..F27
      break;
    default:
      SendPacket.data[3] = 0x07;
      arg4 = (byte)(myFunc.Bits >> 5);
      SendPacket.data[4] = arg4 & 0x7F;                   // F5..F11
      break;
  }
  lnetSend(&SendPacket);
}


void changeFuncIMM(byte fnc) {
  byte data, adrH, adrL;
  adrH = (myLoco.address >> 7) & 0x7F;                          // Digitrax
  adrL = myLoco.adr[0] & 0x7F;
  SendPacket.data[0] = OPC_IMM_PACKET;
  SendPacket.data[1] = 0x0B;
  SendPacket.data[2] = 0x7F;
  if (fnc > 12) {
    if (fnc > 20)
      data = (byte)(myFunc.Bits >> 21) & 0xFF;
    else
      data = (byte)(myFunc.Bits >> 13) & 0xFF;
    if (adrH) {
      SendPacket.data[3] = 0x44;                                // REPS: D4,5,6=#IM bytes,D3=0(reserved); D2,1,0=repeat CNT
      SendPacket.data[4] = (bitRead(myLoco.adr[0], 7)) ? 0x07 : 0x05; // DHI
      if (bitRead(data, 7))
        SendPacket.data[4] |= 0x08;
      SendPacket.data[5] = myLoco.adr[1] | 0x40;                // IM1
      SendPacket.data[6] = adrL;                                // IM2
      if (fnc > 20)
        SendPacket.data[7] = 0x5F;                              // IM3
      else
        SendPacket.data[7] = 0x5E;                              // IM3
      SendPacket.data[8] = data & 0x7F;                         // IM4
    }
    else {
      SendPacket.data[3] = 0x34;                                // REPS: D4,5,6=#IM bytes,D3=0(reserved); D2,1,0=repeat CNT
      SendPacket.data[4] = (bitRead(data, 7)) ? 0x06 : 0x02;    // DHI
      SendPacket.data[5] = adrL;                                // IM1
      if (fnc > 20)
        SendPacket.data[6] = 0x5F;                              // IM2   [110-11111]
      else
        SendPacket.data[6] = 0x5E;                              // IM2   [110-11110]
      SendPacket.data[7] = data & 0x7F;                         // IM3
      SendPacket.data[8] = 0x00;                                // IM4
    }
    SendPacket.data[9] = 0x00;                                  // IM5
    lnetSend(&SendPacket);
  }
  else {
    data = (byte)(myFunc.Bits >> 9) & 0x0F;
    if (adrH) {
      SendPacket.data[3] = 0x34;                                // REPS: D4,5,6=#IM bytes,D3=0(reserved); D2,1,0=repeat CNT
      SendPacket.data[4] = (bitRead(myLoco.adr[0], 7)) ? 0x07 : 0x05; // DHI
      SendPacket.data[5] = myLoco.adr[1] | 0x40;                // IM1
      SendPacket.data[6] = adrL;                                // IM2
      SendPacket.data[7] = 0x20 | data;                         // IM3    [101SDDDD]
    }
    else  {
      SendPacket.data[3] = 0x24;
      SendPacket.data[4] = 0x02;
      SendPacket.data[5] = adrL;
      SendPacket.data[6] = 0x20 | data;
      SendPacket.data[7] = 0x00;
    }
    SendPacket.data[8] = 0x00;                                  // IM4
    SendPacket.data[9] = 0x00;                                  // IM5
    lnetSend(&SendPacket);
    //DEBUG_MSG("Digitrax-FncF9F12 %d", fnc);
  }
}

void lnetTimers() {
  if (millis() - pingTimer > PING_INTERVAL) {               // Refresca velocidad para mantener slot en IN_USE
    if (mySlot.num > 0) {
      pingTimer = millis();
      send4byteMsg(OPC_LOCO_SPD, mySlot.num, mySpeed);
      //DEBUG_MSG("Refresing speed %d", mySpeed);
    }
  }
  if (clockRate > 0) {                                      // Actualiza fast clock interno
    if (millis() - clockTimer > clockInterval) {
      clockTimer = millis();
      clockMin++;
      if (clockMin > 59) {
        clockMin = 0;
        clockHour++;
      }
      if (clockHour > 23)
        clockHour = 0;
      if (scrOLED == SCR_SPEED) {
        updateExtraOLED = true;
        updateOLED = true;
      }
    }
  }
#ifdef  USE_SET_TIME
  if (cfgMenu[CFG_SYNC].value > 0) {
    if (millis() - syncTimer > SYNC_TIME) {               // temporizador para SYNC
      sendSYNC();
    }
  }
#endif
#if defined(USE_LNWIFI)
  yield();
  if (millis() - infoTimer > 1000UL) {                        // Cada segundo
    infoTimer = millis();
    battery = ESP.getVcc ();                                  // Read VCC voltage
    if (battery < LowBattADC)
      lowBATT = true;
  }
#endif
}


#ifdef USE_SET_TIME
void setTime(byte hh, byte mm, byte rate) {
  clockHour = hh;
  clockMin = mm;
  clockRate = rate;
  SendPacket.data[0]  = OPC_WR_SL_DATA;                       // Fast clock
  SendPacket.data[1]  = 0x0E;
  SendPacket.data[2]  = 0x7B;
  SendPacket.data[3]  = rate;                                 // clock rate
  SendPacket.data[4]  = 0x7F;
  SendPacket.data[5]  = 0x79;
  SendPacket.data[6]  = mm + 0x43;
  SendPacket.data[7]  = mySlot.trk;                           // trk
  SendPacket.data[8]  = hh + 0x68;
  SendPacket.data[9]  = 0x00;                                 // day
  SendPacket.data[10] = 0x40;                                 // control
  SendPacket.data[11] = 0x00;                                 // ID
  SendPacket.data[12] = 0x00;
  lnetSend(&SendPacket);
}


void sendSYNC () {                                          // Envia SYNC
  SendPacket.data[0]  = OPC_RQ_SL_DATA;
  SendPacket.data[1]  = 0x7B;
  SendPacket.data[2]  = 0x00;
  lnetSend(&SendPacket);
  syncTimer = millis();
}
#endif


////////////////////////////////////////////////////////////
// ***** LOCONET DECODE *****
////////////////////////////////////////////////////////////

void lnetDecode (lnMsg * LnPacket) {
  unsigned int adr;
  byte slotStatus, i, n;

#ifdef DEBUG
  uint8_t msgLen = getLnMsgSize(LnPacket);              // imprime paquete
  Serial.print(F("Decoding: "));
  for (uint8_t x = 0; x < msgLen; x++) {
    uint8_t val = LnPacket->data[x];
    if (val < 16)
      Serial.print('0');
    Serial.print(val, HEX);
    Serial.print(' ');
  }
  Serial.println();
#endif
  switch (LnPacket->sz.command) {
    case OPC_LOCO_SPD:
      if (LnPacket->lsp.slot == mySlot.num) {           // cambio en la velocidad
        mySpeed = LnPacket->lsp.spd;
        pingTimer = millis();
        updateSpeedHID();
      }
      break;
    case OPC_LOCO_DIRF:
      if (LnPacket->ldf.slot == mySlot.num) {           // cambio en el sentido o F0..F4
        myDir = (LnPacket->ldf.dirf << 2);
        myFunc.xFunc[0] &= 0xE0;
        myFunc.xFunc[0] |= ((LnPacket->ldf.dirf & 0x0F) << 1);
        if (LnPacket->ldf.dirf & 0x10)
          myFunc.xFunc[0] |= 0x01;
        if (scrOLED == SCR_SPEED)
          updateOLED = true;
      }
      break;
    case OPC_LOCO_SND:
      if (LnPacket->ls.slot == mySlot.num) {            // cambio en F5..F8
        myFunc.xFunc[0] &= 0x1F;
        myFunc.xFunc[1] &= 0xFE;
        myFunc.xFunc[0] |= (LnPacket->ls.snd << 5);
        if (LnPacket->ls.snd & 0x08)
          myFunc.xFunc[1] |= 0x01;
#if  (FUNC_SIZE == BIG)
        if ((scrOLED == SCR_SPEED) && (scrFunc == FNC_5_8))
          updateOLED = true;
#endif
#if  (FUNC_SIZE == SMALL)
        if ((scrOLED == SCR_SPEED) && (scrFunc == FNC_0_9))
          updateOLED = true;
#endif
      }
      break;
    case OPC_LOCO_F9F12:                                // DR5000 - Intellibox-II
      if (LnPacket->ls.slot == mySlot.num) {            // cambio en F9..F12
        myFunc.xFunc[1] &= 0xE1;
        myFunc.xFunc[1] |= (LnPacket->ls.snd << 1);
#if  (FUNC_SIZE == BIG)
        if ((scrOLED == SCR_SPEED) && (scrFunc == FNC_9_12))
          updateOLED = true;
#endif
#if  (FUNC_SIZE == SMALL)
        if (scrOLED == SCR_SPEED) {
          if ((scrFunc == FNC_0_9) || (scrFunc == FNC_10_19))
            updateOLED = true;
        }
#endif
      }
      break;
    case OPC_UHLI_FUN:
      if ((LnPacket->data[1] == 0x20) && (LnPacket->data[2] == mySlot.num)) {   // Used by Intellibox-I for F0-F28 and Intellibox-II for F13-F28
        if (LnPacket->data[3] == 0x07) {                // F5..F11          // Used only by Intellibox-I ("one") version 2.x
          myFunc.Bits &= 0xFFFFF01F;
          myFunc.Bits |= ((unsigned long)(LnPacket->data[4] & 0x7F) << 5);
#if  (FUNC_SIZE == BIG)
          slotStatus = 0x06;
#endif
#if  (FUNC_SIZE == SMALL)
          slotStatus = 0x03;
#endif
        }
        if (LnPacket->data[3] == 0x05) {                // F12,F20,F28      // Common to Intellibox-I and -II
          bitWrite(myFunc.Bits, 12, bitRead(LnPacket->data[4], 4));
          bitWrite(myFunc.Bits, 20, bitRead(LnPacket->data[4], 5));
          bitWrite(myFunc.Bits, 28, bitRead(LnPacket->data[4], 6));
#if  (FUNC_SIZE == BIG)
          slotStatus = 0x54;
#endif
#if  (FUNC_SIZE == SMALL)
          slotStatus = 0x06;
#endif
        }
        if (LnPacket->data[3] == 0x08) {                // F13..F19         // Common to Intellibox-I and -II
          myFunc.Bits &= 0xFFF01FFF;
          myFunc.Bits |= ((unsigned long)(LnPacket->data[4] & 0x7F) << 13); // ---87654 32109876 54321098 76543210
#if  (FUNC_SIZE == BIG)
          slotStatus = 0x18;
#endif
#if  (FUNC_SIZE == SMALL)
          slotStatus = 0x02;
#endif
        }
        if (LnPacket->data[3] == 0x09) {                // F21..F27         // Common to Intellibox-I and -II
          myFunc.Bits &= 0xF01FFFFF;
          myFunc.Bits |= ((unsigned long)(LnPacket->data[4] & 0x7F) << 21);
#if  (FUNC_SIZE == BIG)
          slotStatus = 0x60;
#endif
#if  (FUNC_SIZE == SMALL)
          slotStatus = 0x04;
#endif
        }
        if (LnPacket->data[3] == 0x06) {                // F0..F4           // Used only by Intellibox-I ("one") version 2.x
          myFunc.Bits &= 0xFFFFFFE0;
          myFunc.Bits |= ((unsigned long)(LnPacket->data[4] & 0x0F) << 1);
          bitWrite(myFunc.Bits, 0, bitRead(LnPacket->data[4], 4));
          //myDir = (LnPacket->data[3] << 2);
#if  (FUNC_SIZE == BIG)
          slotStatus = 0x01;                                                // 1:F0..F4, 2:F5..F8, 4:F9..F12, 8:F13..F16, 16:F17..F20, 32:F21..F24, 64:F25..F28
#endif
#if  (FUNC_SIZE == SMALL)
          slotStatus = 0x01;                                                // 1:F0..F9, 2:F10..F19, 4:F20..F28
#endif
        }
        i = (scrOLED == SCR_SPEED) ? (1 << scrFunc) : 0;
        if (i & slotStatus)
          updateOLED = true;
      }
      break;
#ifdef USE_DECODE_IMM
    case OPC_IMM_PACKET:
      if ((LnPacket->sp.mesg_size == 0x0B) && (LnPacket->sp.val7f == 0x7F)) {
        if (bitRead(LnPacket->sp.dhi, 0)) {
          if (bitRead(LnPacket->sp.im1, 6)) {           // im1:1 1LLLLLL im2: L LLLLLLL
            adr = (bitRead(LnPacket->sp.dhi, 1)) ? LnPacket->sp.im2 | 0x80 : LnPacket->sp.im2;
            adr |= ((LnPacket->sp.im1 & 0x3F) << 8);
            if (adr == myLoco.address) {
              if (bitRead(LnPacket->sp.dhi, 2)) {       // im3: 1 XXXFFFF
                i = (bitRead(LnPacket->sp.dhi, 3)) ? LnPacket->sp.im4 | 0x80 : LnPacket->sp.im4;
                decodeFuncIMM (LnPacket->sp.im3, i);
              }
            }
          }
        }
        else {
          if (LnPacket->sp.im1 == myLoco.address) {     // im1: 0 LLLLLLL
            if (bitRead(LnPacket->sp.dhi, 1)) {         // im2: 1 XXXFFFF
              i = (bitRead(LnPacket->sp.dhi, 2)) ? LnPacket->sp.im3 | 0x80 : LnPacket->sp.im3;
              decodeFuncIMM (LnPacket->sp.im2, i);
            }
          }
        }
      }
      break;
#endif
    case OPC_INPUT_REP:
      adr = (LnPacket->ir.in1 | ((LnPacket->ir.in2 & 0x0F) << 7));
      adr <<= 1;
      adr += (LnPacket->ir.in2 & OPC_INPUT_REP_SW) ? 2 : 1;
      lnetNotifySensor(adr, LnPacket->ir.in2 & OPC_INPUT_REP_HI);
      break;
    case OPC_SW_REQ:
      adr = (LnPacket->srq.sw1 | ((LnPacket->srq.sw2 & 0x0F) << 7));
      adr++;
      lnetNotifySwitchRequest(adr, LnPacket->srq.sw2 & OPC_SW_REQ_OUT, LnPacket->srq.sw2 & OPC_SW_REQ_DIR);
      break;
    case OPC_SW_REP:
      adr = (LnPacket->srp.sn1 | ((LnPacket->srp.sn2 & 0x0F) << 7));
      adr++;
      if (LnPacket->srp.sn2 & OPC_SW_REP_INPUTS) {
        isMyTurnout(adr, LnPacket->srp.sn2 & OPC_SW_REP_HI, LnPacket->srp.sn2 & OPC_SW_REP_SW);
      }
      else {
        lnetNotifySwitchOutputsReport(adr, LnPacket->srp.sn2 & OPC_SW_REP_CLOSED, LnPacket->srp.sn2 & OPC_SW_REP_THROWN);
      }
      break;
    case OPC_GPON:
      scrOLED = SCR_ENDLOGO;
      bitSet(mySlot.trk, 0);
      updateOLED = true;
#ifdef USE_PHONE
      phoneCallEnd();
#endif
      break;
    case OPC_GPOFF:
      bitClear(mySlot.trk, 0);
      scrOLED = SCR_ERROR;
      errOLED = ERR_OFF;
      updateOLED = true;
#ifdef USE_PHONE
      phoneCallEnd();
#endif
      break;
    case OPC_SL_RD_DATA:                                // informacion de un slot
      adr = (LnPacket->sd.adr2 << 7) + LnPacket->sd.adr;
      if (doDispatchGet && (LnPacket->sd.slot < 0x79)) {  // valid slot 1..120
        myLoco.address = adr;
      }
      if ((adr == myLoco.address) && (LnPacket->sd.slot < 0x79) && (!doDispatchPut)) {    // valid slot 1..120
        //DEBUG_MSG("Slot read for ADDR:%d", adr);
        mySlot.num = LnPacket->sd.slot;                 // es  mi locomotora, guarda slot
        mySlot.state = LnPacket->sd.stat;
        mySlot.trk = LnPacket->sd.trk;
        slotStatus = (LnPacket->sd.stat >> 4) & 0x03;
        //DEBUG_MSG("Slot % d STATUS: % d", mySlot.num, slotStatus);
        mySpeed = LnPacket->sd.spd;                     // actualiza velocidad
        myDir = (LnPacket->sd.dirf << 2) & 0x80;        // actualiza sentido
        myFunc.Bits &= 0xFFFFFE00;
        myFunc.xFunc[0] |= ((LnPacket->sd.dirf & 0x0F) << 1);
        myFunc.xFunc[0] |= ((LnPacket->sd.dirf >> 4) & 0x01);
        myFunc.xFunc[0] |= ((LnPacket->sd.snd & 0x07) << 5);
        bitWrite(myFunc.Bits, 8, bitRead(LnPacket->sd.snd, 3));
        //fnc = ((unsigned long)(LnPacket->sd.dirf & 0x0F) << 1) | ((unsigned long)(LnPacket->sd.snd & 0x0F) << 5);   // actualiza funciones F0..F8
        //myFunc.Bits = (myFunc.Bits & 0xFFFFFE00) | fnc | ((LnPacket->sd.dirf >> 4) & 0x01);
        if (slotStatus != STAT_IN_USE) {
          nullMoveSlot (mySlot.num);                    // si el slot no se usa, tomo el control para que refresque
        }
        if (doDispatchGet) {
          doDispatchGet = false;
          checkLocoAddress();
          pushLoco(myLoco.address);                     // guarda en stack
          updateEEPROM (EE_ADRH, myLoco.adr[1]);       // guarda nueva direccion en EEPROM
          updateEEPROM (EE_ADRL, myLoco.adr[0]);
          optOLED = OPT_SPEED;
          enterMenuOption();
        }
        else {
          updateSpeedHID();
        }
      }
      if (LnPacket->sd.slot == 0x7B) {                  // FAST Clock
        if (LnPacket->fc.clk_cntrl & 0x40) {            // bit 6 = 1; data is valid clock info
          setFastClock(LnPacket);
        }
      }
      if (LnPacket->sd.slot == 0x7C) {                  // Programmer Task Final reply
        if (progStepCV != PRG_IDLE) {
          mySlot.trk = LnPacket->pt.trk;
          CVdata = LnPacket->pt.data7 | (bitRead(LnPacket->pt.cvh, 1) << 7);
          CVdata |= ((LnPacket->pt.pstat & 0x0F) << 8);
          endProg();
        }
      }
      break;
    case OPC_WR_SL_DATA:
      if ((mySlot.num > 0) && (LnPacket->sd.slot == mySlot.num)) {    // Cambios en mi slot
        infoLocomotora(myLoco.address);                               // do it with read slot
      }
      if (LnPacket->sd.slot == 0x7C) {                  // Programmer Task Start
        //CVdata = LnPacket->pt.cvl | bitRead((LnPacket->pt.cvh, 1) << 6);
      }
      if (LnPacket->sd.slot == 0x7B) {                  // FAST Clock
        //if (LnPacket->fc.clk_cntrl & 0x40) {            // bit 6 = 1; data is valid clock info. JMRI sends only EF 0E 7B ... (OPC_WR_SL_DATA) with clk_cntrl == 0
        setFastClock(LnPacket);
        //}
      }
      break;
    case OPC_MOVE_SLOTS:
      if ((LnPacket->sm.src == mySlot.num) && (LnPacket->sm.dest == mySlot.num)) {
        // me quieren robar el slot. He sido yo?
      }
      break;
    case OPC_LONG_ACK:
      //DEBUG_MSG("LACK Opcode: %x resp: %x", LnPacket->lack.opcode | 0x80, LnPacket->lack.ack1);
      if (LnPacket->lack.opcode == (OPC_SW_STATE & 0x7F)) {
        lnetNotifyLongAckSwState(LnPacket->data[1], LnPacket->data[2]);
      }
      if (doDispatchGet) {
        if ((LnPacket->lack.opcode == (OPC_MOVE_SLOTS & 0x7F)) && (LnPacket->lack.ack1 == 0)) {
          doDispatchGet = false;
          optOLED = OPT_LOCO;
          enterMenuOption();
          //DEBUG_MSG("LACK Move Slots");
        }
      }
      if (progStepCV != PRG_IDLE) {
        if (LnPacket->lack.opcode == (OPC_WR_SL_DATA & 0x7F)) {
          switch (LnPacket->lack.ack1) {
            case 0x7F:                                  // Function NOT implemented, no reply.
            case 0x00:                                  // Programmer BUSY , task aborted, no reply.
              CVdata = 0x0600;                          // show ERR
            case 0x40:                                  // Task accepted blind NO <E7> reply at completion
              endProg();
              break;
            case 0x01:                                  // Task accepted , <E7> reply at completion.
              break;
          }
        }
      }
#ifdef USE_LNCV
      if (scrOLED == SCR_LNCV) {
        if (LnPacket->lack.opcode == (OPC_IMM_PACKET & 0x7F) && (LnPacket->lack.ack1 != 0x7F)) {  // error writing LNCV
          updateExtraOLED = true;
          updateOLED = true;
        }
      }
#endif
      break;
    case OPC_SLOT_STAT1:
      if ((mySlot.num > 0) && (LnPacket->ss.slot == mySlot.num)) {    // Cambios en mi slot
        //if (!acquireLoco)
        //  infoLocomotora(myLoco.address);
      }
      break;
    case OPC_RQ_SL_DATA:
      /*
        if (LnPacket->sr.slot == 0x7B) {                  // FAST Clock SYNC
          clockTimer = millis();                          // reset local sub-minute phase counter
        }
      */
      break;
    case  OPC_PEER_XFER:
#ifdef USE_LNCV
      //  [E5 0F 05 49 4B 1F 01 2F 13 00 00 01 00 00 31]  (LNCV) READ_CV_REPLY from module (Article #5039):
      if ((LnPacket->ub.DSTL == 'I') && (LnPacket->ub.DSTH == 'K')) {
        if ((LnPacket->ub.SRC == 0x05) && (LnPacket->ub.ReqId == LNCV_REQID_CFGREAD)) {
          for (i = 0; i < 7; i++) {                     // read bits in PXCT1
            if (bitRead(LnPacket->ub.PXCT1, i)) {
              bitSet(LnPacket->ub.payload.D[i], 7);
            }
          }
          artNum  = LnPacket->ub.payload.data.deviceClass;
          numLNCV = LnPacket->ub.payload.data.lncvNumber;
          valLNCV = LnPacket->ub.payload.data.lncvValue;
          if (numLNCV == 0)
            modNum = valLNCV;
          if (scrOLED == SCR_LNCV)
            updateOLED = true;
        }
        /*
          if ((LnPacket->ub.SRC == 0x00) && (LnPacket->ub.ReqId == 0x0B)) {  // Virtual loco
          for (i = 0; i < 7; i++) {                     // read bits in PXCT1
            if (bitRead(LnPacket->ub.PXCT1, i)) {
              bitSet(LnPacket->ub.payload.D[i], 7);
            }
          }
          myID = LnPacket->ub.payload.data.lncvNumber;
          virtualLoco = LnPacket->ub.payload.data.lncvValue;
          }
        */
      }
#endif
      break;
  }
}



void setFastClock(lnMsg * LnPacket) {
  if (LnPacket->fc.clk_rate != clockRate) {                 // 0 = Freeze clock, 1 = normal, 10 = 10:1 etc. Max is 0x7f
    clockRate = LnPacket->fc.clk_rate;                      // calcula nuevo intervalo interno
    if (clockRate > 0)
      clockInterval = 60000UL / (unsigned long)clockRate;   // calcula intervalo para un minuto
  }                                                         // [EF 0E 7B 01 7F 79 43 07 68 1B 40 00 00 15] JMRI: 00:00 DAY 27
  clockMin =  LnPacket->fc.mins_60 - 0x43;                  // 256 - minutes   ???
  clockHour = LnPacket->fc.hours_24 - 0x68;                 // 256 - hours     ???
  clockTimer = millis();
  DEBUG_MSG("CLOCK %d:%d R:%d", clockHour, clockMin, clockRate);
  if (scrOLED == SCR_SPEED) {
    updateExtraOLED = true;
    updateOLED = true;
  }
}


#ifdef USE_DECODE_IMM
void decodeFuncIMM (byte cmd, byte data) {
  /*
    if ((cmd & 0x60) == 0x00) {                               // 100D-DDDD  F0-F4
      myFunc.xFunc[0] &= 0xE0;
      myFunc.xFunc[0] |= ((cmd & 0x0F) << 1);
      if (cmd & 0x10)
        myFunc.xFunc[0] |= 0x01;
      cmd = 0x80;
    }
    if ((cmd & 0x70) == 0x30) {                               // 1011-FFFF  F5-F8
      myFunc.xFunc[0] &= 0x1F;
      myFunc.xFunc[0] |= (cmd << 5);
      if (cmd & 0x10)
        myFunc.xFunc[1] |= 0x01;
      else
        myFunc.xFunc[1] &= 0xFE;
      cmd = 0x80;
    }
  */
  if ((cmd & 0x70) == 0x20) {                               // 1010-FFFF  F9-F12
    myFunc.xFunc[1] &= 0xE1;
    myFunc.xFunc[1] |= ((cmd & 0x0F) << 1);
    cmd = 0x80;
  }
  if (cmd == 0x5E) {                                        // 1101-1110  DDDDDDDD F13-F20
    myFunc.Bits &= 0xFFE01FFF;
    myFunc.Bits |= ((unsigned long)(data) << 13);
    cmd = 0x80;
  }
  if (cmd == 0x5F) {                                        // 1101-1111  DDDDDDDD F21-F28
    myFunc.Bits &= 0xE01FFFFF;
    myFunc.Bits |= ((unsigned long)(data) << 21);
    cmd = 0x80;
  }
  if ((scrOLED == SCR_SPEED) && (cmd == 0x80))
    updateOLED = true;
}
#endif

/*  Program slot
   <0xEF>,<0E>,<7C>,<PCMD>,<0>,<HOPSA>,<LOPSA>,<TRK>;<CVH>,<CVL>,<DATA7>,<0>,<0>,<CHK>

  PCMD
  D7 -0
  D6 -Write/Read , 1= Write, 0=Read
  D5 -Byte Mode , 1= Byte operation, 0=Bit operation (if possible)
  D4 -TY1 Programming Type select bit
  D3 -TY0 Prog type select bit
  D2 -Ops Mode, 1=Ops Mode on Mainlines, 0=Service Mode on Programming Track
  D1 -0 reserved
  D0 -0-reserved

  Direct Read   '00101000'  0x28
  Direct Write  '01101000'  0x68
  PoM Write     '01100100'  0x64


  Intellibox II & DR5000:
  Program start
  0xE5, 0x07, 0x01, 0x49, 0x42, 0x41, CHK
  Program end
  0xE5, 0x07, 0x01, 0x49, 0x42, 0x40, CHK
  Write/Read CV
  Answer with standard Slot read 124

  0xED, 0x1F, 0x01, 0x49, 0x42, PXCT1, CMD, CVL, CVH, VAL, PXCT2, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, CHK
  PXCT1:  0 1 1 1 VAL.7 0 CVL.7 1
  PXCT2:  0 1 1 1   0   0   0   0  ??
  CMD:    Program Dir:  0x72 Read, 0x71: Write
          Program MM:   0x62 R / 0x63 W   ??
          Program Reg:  0x6C R / 0x6D W   ??
          Program Page: 0x6E R / 0x6F W   ??

  Intellibox II:
  0xED, 0x1F, 0x01, 0x49, 0x42, PXCT1, CMD, ADL, ADH, CVL, PXCT2, CVH, VAL, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, CHK
    PXCT1:  0 1 1 1 CVL.7 0 ADL.7 1
    PXCT2:  0 1 1 1   0   0 VAL.7 0
    CMD:    Program POM:  0x5E


  JMRI
  [ED 1F 01 49 42 71 72 01 00 00 70 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 65] Read CV1
  [ED 1F 01 49 42 73 72 00 00 00 70 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 66] Read CV128
  [ED 1F 01 49 42 71 71 01 00 00 70 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 66] Write CV1=0
  [ED 1F 01 49 42 71 5E 01 00 01 70 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 48] Write CV1=0 POM ADR=1
  [ED 1F 01 49 42 71 71 01 00 55 70 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 33] Write CV1=55
  [ED 1F 01 49 42 79 71 01 00 2A 70 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 44] Write CV1=170
  [ED 1F 01 49 42 71 5E 01 00 01 70 00 55 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 1D] Write CV1=55 POM
  [ED 1F 01 49 42 71 5E 01 00 01 72 00 2A 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 60] Write CV1=170 POM ADR=1
  [ED 1F 01 49 42 73 5E 03 00 01 72 00 2A 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 60] Write CV1=170 POM ADR=131
  [ED 1F 01 49 42 7B 5E 03 00 00 72 00 2A 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 69] Write CV128=170 POM ADR=131


*/

void readCV (unsigned int adr, byte stepPrg) {
  byte cvh;
  if (!modeProg) {                                          // Read only in Direct mode
    clearPacketUlhi();
#ifdef USE_PRG_UHLI
    if (typeCmdStation == CMD_DR) {
      progUhli(UHLI_PRG_START);                             // Intellibox II format
      SendPacketUhli.data[0] = OPC_IMM_PACKET;
      SendPacketUhli.data[1] = 0x1F;
      SendPacketUhli.data[2] = 0x01;
      SendPacketUhli.data[3] = 'I';
      SendPacketUhli.data[4] = 'B';
      SendPacketUhli.data[5] = 0x71 | (bitRead(adr, 7) << 1);
      SendPacketUhli.data[6] = 0x72;
      SendPacketUhli.data[7] = adr & 0x7F;
      SendPacketUhli.data[8] = (adr >> 8) & 0x03;
      SendPacketUhli.data[10] = 0x70;
      SendPacketUhli.data[15] = 0x10;
    }
    else
#endif
    {
      SendPacketUhli.data[0] = OPC_WR_SL_DATA;
      SendPacketUhli.data[1] = 0x0E;
      SendPacketUhli.data[2] = 0x7C;                        // Slot 0x7C
      SendPacketUhli.data[3] = 0x28;                        // PCMD Direct Read
      //    SendPacketUhli.data[4] = 0x00;
      //    SendPacketUhli.data[5] = 0x00;                  // HOPSA Loco address
      //    SendPacketUhli.data[6] = 0x00;                  // LOPSA
      SendPacketUhli.data[7] = mySlot.trk;                  // TRK
      adr--;
      cvh = bitRead(adr, 7) | (bitRead(adr, 8) << 4) | (bitRead(adr, 9) << 5);
      SendPacketUhli.data[8] = cvh;                         // CVH <0,0,CV9,CV8 - 0,0, D7,CV7>
      SendPacketUhli.data[9] = adr & 0x7F;                  // CVL
      //    SendPacketUhli.data[10] = 0x00;                 // DATA7
      //    SendPacketUhli.data[11] = 0x00;
      //    SendPacketUhli.data[12] = 0x00;
    }
    lnetSend( &SendPacketUhli.lnMsgCV);
    //DEBUG_MSG("Read CV %d", adr);
    progStepCV = stepPrg;
  }
}



void writeCV (unsigned int adr, unsigned int data, byte stepPrg) {
  byte cvh;
  clearPacketUlhi();
#ifdef USE_PRG_UHLI
  if (typeCmdStation == CMD_DR) {
    progUhli(UHLI_PRG_START);                               // Intellibox II format
    SendPacketUhli.data[0] = OPC_IMM_PACKET;
    SendPacketUhli.data[1] = 0x1F;
    SendPacketUhli.data[2] = 0x01;
    SendPacketUhli.data[3] = 'I';
    SendPacketUhli.data[4] = 'B';
    if (modeProg) {
      SendPacketUhli.data[5] = 0x71 | (bitRead(myLoco.address, 7) << 1) | (bitRead(adr, 7) << 3);
      SendPacketUhli.data[6] = 0x5E;
      SendPacketUhli.data[7] = myLoco.address & 0x7F;
      SendPacketUhli.data[8] = (myLoco.address >> 8) & 0x3F;
      SendPacketUhli.data[9] = adr & 0x7F;
      SendPacketUhli.data[10] = 0x70 | (bitRead(data, 7) << 1);
      SendPacketUhli.data[11] = (adr >> 8) & 0x03;
      SendPacketUhli.data[12] = data & 0x7F;
    }
    else {
      SendPacketUhli.data[5] = 0x71 | (bitRead(adr, 7) << 1) | (bitRead(data, 7) << 3);
      SendPacketUhli.data[6] = 0x71;
      SendPacketUhli.data[7] = adr & 0x7F;
      SendPacketUhli.data[8] = (adr >> 8) & 0x03;
      SendPacketUhli.data[9] = data & 0x7F;
      SendPacketUhli.data[10] = 0x70;
    }
    SendPacketUhli.data[15] = 0x10;
  }
  else
#endif
  {
    SendPacketUhli.data[0] = OPC_WR_SL_DATA;                // Write in Direct mode or POM
    SendPacketUhli.data[1] = 0x0E;
    SendPacketUhli.data[2] = 0x7C;                          // Slot 0x7C
    //  SendPacketUhli.data[4] = 0x00;
    if (modeProg) {
      SendPacketUhli.data[3] = 0x64;                        // PCMD PoM Write
      SendPacketUhli.data[5] = (myLoco.address >> 7) & 0x7F;// HOPSA Loco address
      SendPacketUhli.data[6] = myLoco.address & 0x7F;       // LOPSA
    }
    else {
      SendPacketUhli.data[3] = 0x68;                        // PCMD Direct Write
      //    SendPacketUhli.data[5] = 0x00;                  // HOPSA Loco address
      //    SendPacketUhli.data[6] = 0x00;                  // LOPSA
    }
    SendPacketUhli.data[7] = mySlot.trk;                    // TRK
    adr--;
    cvh = bitRead(adr, 7) | (bitRead(adr, 8) << 4) | (bitRead(adr, 9) << 5) | (bitRead(data, 7) << 1);
    SendPacketUhli.data[8] = cvh;                           // CVH <0,0,CV9,CV8 - 0,0, D7,CV7>
    SendPacketUhli.data[9] = adr & 0x7F;                    // CVL
    SendPacketUhli.data[10] = data & 0x7F;                  // DATA7
    //  SendPacketUhli.data[11] = 0x00;
    //  SendPacketUhli.data[12] = 0x00;
  }
  lnetSend( &SendPacketUhli.lnMsgCV);
  progStepCV = stepPrg;
  //DEBUG_MSG("Write CV%d = %d", adr, data);
}

#ifdef USE_PRG_UHLI
void progUhli (byte mode) {
  if (typeCmdStation == CMD_DR) {                           // Intellibox II program task start or end
    SendPacket.data[0] = OPC_PEER_XFER;
    SendPacket.data[1] = 0x07;
    SendPacket.data[2] = 0x01;
    SendPacket.data[3] = 'I';
    SendPacket.data[4] = 'B';
    SendPacket.data[5] = mode;
    lnetSend( &SendPacket);
  }
}
#endif

void clearPacketUlhi() {                                    // Borra paquete largo para Intellibox II
  for (byte i = 0; i < 31; i++)
    SendPacketUhli.data[i] = 0x00;
}


////////////////////////////////////////////////////////////
// ***** LOCONET CALLBACK *****
////////////////////////////////////////////////////////////

void lnetNotifyLongAckSwState(uint8_t d1, uint8_t d2) {     // Respuesta a reportSwitch()
  if (accStateReq) {
    myPosTurnout = (d2 & 0x20) ? RECTO : DESVIADO;
    accStateReq = false;
    updateOLED = true;
    DEBUG_MSG("LACK SW State %02x", d2);
  }
#ifdef USE_PHONE
  if (doPhoneStatus) {
    doPhoneStatus = false;
    phonePhase = (d2 & 0x20) ? PHONE_BUSY : PHONE_CALLING;  // green/red
    updateOLED = true;
  }
#endif
}

void lnetNotifySwitchRequest(uint16_t Address, uint8_t Output, uint8_t Direction) {   // Alguien mueve un desvio (OPC_SW_REQ)
  byte i;
  isMyTurnout(Address, Output, Direction);
  //DEBUG_MSG("SW %d REQUEST: %d-%d", Address, Direction, Output);

#ifdef USE_PHONE

  if (Address != PHONE_BASE) {
    for (i = 1; i < MAX_STATION; i++) {                     // check if colateral station pick up the phone
      if (Address == (phoneBook[i] + PHONE_BASE)) {
        if ((phonePhase == PHONE_IDLE) && Direction)        // GREEN
          lastStationPickUp = Address;
        //bitWrite(phonePBX, i, Direction);                   // or hangs
      }
    }
    switch (phonePhase) {
      case PHONE_IDLE:
        if (Direction) {                                    // GREEN
          if (Address == myStation ) {                      // someone is calling me
            originateStation = lastStationPickUp;
            if (originateStation > 0)
              phonePhase = PHONE_RING;
          }
        }
        break;
      case WAIT_ANSWER:
        if (!Direction) {                                   // RED
          if (Address == myStation) {                       // accept train
            phonePhase = CALL_ACCEPTED;
          }
          else {
            if (Address == callingStation)                  // Hung. Stop train
              phonePhase = CALL_REJECTED;
          }
        }
        break;
      case WAIT_TRAIN:
        if (!Direction) {                                   // RED
          if (Address == myStation) {                       // dispatch get train
            phonePhase = GET_TRAIN;
          }
        }
        break;
      case WAIT_USER:
        if (!Direction) {                                   // RED
          if (Address == myStation) {                       // dispatch get train
            phoneCallEnd();
            showMenu();
            updateOLED = true;
          }
        }
        break;
    }
  }
#endif
}


void lnetNotifySwitchOutputsReport(uint16_t Address, uint8_t ClosedOutput, uint8_t ThrownOutput) {    // Turnout SENSOR state REPORT (OPC_SW_REP)
  if (Address == myTurnout) {                                                                     // input levels for turnout feedback
    myPosTurnout = NO_MOVIDO;
    if (ClosedOutput)
      myPosTurnout = RECTO;
    if (ThrownOutput)
      myPosTurnout = DESVIADO;
    if (scrOLED == SCR_TURNOUT)
      updateOLED = true;
  }
  //DEBUG_MSG("SW %d OUT REPORT: %d-%d", Address, ClosedOutput, ThrownOutput);
}
/*
  void notifySwitchReport(uint16_t Address, uint8_t Output, uint8_t Direction) {                    // Turnout SENSOR state REPORT (OPC_SW_REP)
  isMyTurnout(Address, Output, Direction);                                                        // current OUTPUT levels
  //DEBUG_MSG("SW %d REPORT: %d-%d", Address, Direction, Output);
  }
*/

void isMyTurnout(uint16_t Address, uint8_t Output, uint8_t Direction) {       // Info desvio, comprobar si es el mio
  if (Address == myTurnout) {
    if (Direction)
      myPosTurnout = RECTO;
    else
      myPosTurnout = DESVIADO;
    if ((scrOLED == SCR_TURNOUT) && Output)
      updateOLED = true;
  }
}


void lnetNotifySensor(uint16_t Address, uint8_t State) {        // General SENSOR input codes (OPC_INPUT_REP)
  unsigned int adr;
  adr = (Shuttle.moduleA << 8) + Shuttle.inputA;
  if (adr == Address)
    Shuttle.statusA = State;
  adr = (Shuttle.moduleB << 8) + Shuttle.inputB;
  if (adr == Address)
    Shuttle.statusB = State;
#ifdef USE_AUTOMATION
  for (byte n = 0; n < MAX_AUTO_SEQ; n++) {
    if ((automation[n].opcode & OPC_AUTO_MASK) == OPC_AUTO_FBK) {
      adr = ((automation[n].opcode & 0x07) << 8) + automation[n].param;
      if (adr == Address)
        automation[n].value = State;
    }
  }
#endif
}



/*
   LocoNet support in the IB:
  In addition to the above, there are other LocoNet messages which are specific to the Intellibox and which are used for communication between
  the two main CPU's of the IB (SPU and KPU) as well as for other purposes. These messages have header OPC_IMM_PACKET or OPC_PEER_XFER with a length
  of either 7, 15 or 31 bytes (see below for doc on a group of those messages with length = 15 bytes).

  [E5 07 01 49 42 41 56]  Uhlenbrock IB-COM / Intellibox II Start Programming Track.  OPC_PEER_XFER
  [82 7D]  Set Global (Track) Power to 'OFF'. OPC_GPOFF
  [ED 1F 01 49 42 71 72 03 00 00 70 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 67]  Read CV in Direct Byte Mode from PT for Uhlenbrock IB-COM / Intellibox - CV: 3.
  [B4 6D 01 27] Long_ACK: Uhlenbrock IB-COM / Intellibox II CV programming request was accepted.   Not issued in Daisy II
  [E7 0E 7C 00 00 00 00 07 00 00 0E 00 00 63]  Programming Response: Uhlenbrock IB-COM / Intellibox II Programming Read Was Successful: CV1 value 14 (0x0E, 00001110b).

  [E5 07 01 49 42 41 56]  Uhlenbrock IB-COM / Intellibox II Start Programming Track. OPC_PEER_XFER
  [82 7D]  Set Global (Track) Power to 'OFF'. OPC_GPOFF
  [ED 1F 01 49 42 71 72 08 00 00 70 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 6C]  Read CV in Direct Byte Mode from PT for Uhlenbrock IB-COM / Intellibox - CV: 8
  [B4 6D 01 27]  Long_ACK: Uhlenbrock IB-COM / Intellibox II CV programming request was accepted.
  [E7 0E 7C 00 00 00 72 06 02 00 11 00 00 0D]  Programming Response: Uhlenbrock IB-COM / Intellibox II Programming Read Was Successful: CV1 value 145 (0x91, 10010001b).

  <0xED>,<0B>,<7F>,<REPS>,<DHI>,<IM1>,<IM2>,<IM3>,<IM4>,<IM5>,<CHK>    OPC_IMM_PACKET

  <0xED>,<1F>,<01>,<49>,<42>,<PCXT1>,<CMD>,<CVL>,<CVH>,<VALL>,<70>,<00>,<IM?>,<IM?>,<IM?>,<10>,<IM?>,<IM?>,
    0     1    2   3     4    5      6     7     8     9     10    11    12    13    14  15
              SRC DSTL DSTH PCXT1

  SRC  0 = master, 1 = KPU, 2 = DAISY, 3 = TB or FRED, 4 = IB-Switch, 5 = LocoNet modules, 70h..7Eh = reserved
  DSTL/H destination (addressed) device, 0/0 = broadcast
          "I"/"B" = Intellibox (SPU: the 'main' CPU of the Intellibox)
          "I"/"K" = Intellibox (KPU: the 'user interface' CPU of the Intellibox)
          0..15/"T" = Twin-Box
          "I"/"S" = IB-Switch
          "D"/"Y" = DAISY throttle
  PXCT1  0, D7.7, D6.7, D5.7, D4.7, D3.7, D2.7, D1.7

  CMD: 6C: READ REG, 6D: WRITE REG, 6E: READ PAGE, 6F: WRITE PAGE, 71: WRITE DIRECT, 70,72: READ DIRECT
  CVB: BIT1:CVL.7, BIT3:VALL.7

  <0xED>,<1F>,<01>,<49>,<42>,<PCXT1>,<CMD>,<ADRL>,<ADRH>,<CVL>,<VALH>,<CVH>,<VALL>,<IM?>,<IM?>,<IM?>,<IM?>,<IM?>,
    0     1    2   3     4    5      6     7     8     9     10     11     12    13    14    15

  CMD: 5E: POM
  CVB: BIT3: CVL.7, BIT1: VALL.7

  [BE 09 52 1A] IB Request Loco slot
  [E6 15 00 02 20 52 09 07 00 00 00 00 00 00 00 00 00 00 00 00 72]  Read extended slot (Write reply): slot 2 stat 32 addr 1234 speed 0.
         SLT STAT ADL ADH


  LNCV

  JMRI -> DR5039
  [ED 0F 01 05 00 21 41 2F 13 00 00 01 00 00 44]  (LNCV) MOD_PROG_START command to module #1 (Article #5039)
  [E5 0F 05 49 4B 1F 01 2F 13 00 00 01 00 00 31]  (LNCV) READ_CV_REPLY from module (Article #5039):
  CV0 value = 1
  [ED 0F 01 05 00 21 01 2F 13 00 00 01 00 00 04]  (LNCV) READ_CV request to module #1 (Article #5039):
  Read CV0
  [E5 0F 05 49 4B 1F 01 2F 13 00 00 01 00 00 31]  (LNCV) READ_CV_REPLY from module (Article #5039):
  CV0 value = 1
  [ED 0F 01 05 00 20 01 2F 13 00 00 01 00 00 05]  (LNCV) WRITE_CV request to module (Article #5039):
  Set value in CV0 to: 1
  [B4 6D 7F 59]  LONG_ACK: the Send IMM Packet command was accepted.
  [E5 0F 01 05 00 21 01 2F 13 00 00 01 00 40 4C]  (LNCV) MOD_PROG_END command to module #1 (Article #5039)

  JMRI -> 5088RC
  [ED 0F 01 05 00 21 41 60 13 00 00 01 00 00 0B]  (LNCV) MOD_PROG_START command to module #1 (Article #5088)
  [E5 0F 05 49 4B 1F 41 60 13 00 00 01 00 00 3E]  (LNCV) READ_CV_REPLY from module (Article #5088):
  CV0 value = 1
  [ED 0F 01 05 00 21 01 60 13 00 00 01 00 00 4B]  (LNCV) READ_CV request to module #1 (Article #5088):
  Read CV0
  [E5 0F 05 49 4B 1F 01 60 13 00 00 01 00 00 7E]  (LNCV) READ_CV_REPLY from module (Article #5088):
  CV0 value = 1
  [ED 0F 01 05 00 20 01 60 13 00 00 01 00 00 4A]  (LNCV) WRITE_CV request to module (Article #5088):
  Set value in CV0 to: 1
  [B4 6D 7F 59]  LONG_ACK: the Send IMM Packet command was accepted.
  [E5 0F 01 05 00 21 01 60 13 00 00 01 00 40 03]  (LNCV) MOD_PROG_END command to module #1 (Article #5088)

  TWIN CENTER
  [ED 0F 01 05 00 21 33 7F 7F 00 00 7F 7F 00 0B]  (LNCV) READ_CV request to module #65535 (Article #65535):     // Enter LNCV Menu
  Read CV0
  [ED 0F 01 05 00 21 41 60 13 00 00 01 00 00 0B]  (LNCV) MOD_PROG_START command to module #1 (Article #5088)    // Art: 5088 Module: 1
  [E5 0F 05 49 4B 1F 41 60 13 00 00 01 00 00 3E]  (LNCV) READ_CV_REPLY from module (Article #5088):
  CV0 value = 1
  [ED 0F 01 05 00 21 01 60 13 00 00 01 00 00 4B]  (LNCV) READ_CV request to module #1 (Article #5088):          // Enter LNCV0
  Read CV0
  [E5 0F 05 49 4B 1F 01 60 13 00 00 01 00 00 7E]  (LNCV) READ_CV_REPLY from module (Article #5088):
  CV0 value = 1
  [ED 0F 01 05 00 20 01 60 13 00 00 01 00 00 4A]  (LNCV) WRITE_CV request to module (Article #5088):            // LNCV0 = 1
  Set value in CV0 to: 1
  [B4 6D 7F 59]  LONG_ACK: the Send IMM Packet command was accepted.
  [E5 0F 01 05 00 21 03 7F 7F 00 00 01 00 40 72]  (LNCV) General ALL_PROG_END command                           // Menu

   TWIN CENTER
  [ED 0F 01 05 00 21 33 7F 7F 00 00 7F 7F 00 0B]  (LNCV) READ_CV request to module #65535 (Article #65535):      // Enter LNCV menu  Art.-Nr:
  Read CV0
  [ED 0F 01 05 00 21 41 2F 13 00 00 01 00 00 44]  (LNCV) MOD_PROG_START command to module #1 (Article #5039)     // Art: 5039 Module: 1
  [E5 0F 05 49 4B 1F 01 2F 13 00 00 01 00 00 31]  (LNCV) READ_CV_REPLY from module (Article #5039):
  CV0 value = 1
  [ED 0F 01 05 00 21 01 2F 13 00 00 01 00 00 04]  (LNCV) READ_CV request to module #1 (Article #5039):           // LNCV0
  Read CV0
  [E5 0F 05 49 4B 1F 01 2F 13 00 00 01 00 00 31]  (LNCV) READ_CV_REPLY from module (Article #5039):
  CV0 value = 1
  [ED 0F 01 05 00 20 01 2F 13 00 00 01 00 00 05]  (LNCV) WRITE_CV request to module (Article #5039):             // LNCV0=1
  Set value in CV0 to: 1
  [B4 6D 7F 59]  LONG_ACK: the Send IMM Packet command was accepted.
  [E5 0F 01 05 00 21 03 7F 7F 00 00 01 00 40 72]  (LNCV) General ALL_PROG_END command                            // Menu//Back

  TWIN CENTER
  [ED 0F 01 05 00 21 33 7F 7F 00 00 7F 7F 00 0B]  (LNCV) READ_CV request to module #65535 (Article #65535):      // Enter Menu
  Read CV0
  [E5 0F 05 49 4B 1F 01 60 13 00 00 01 00 00 7E]  (LNCV) READ_CV_REPLY from module (Article #5088):
  CV0 value = 1
  [ED 0F 01 05 00 21 01 60 13 00 00 01 00 00 4B]  (LNCV) READ_CV request to module #1 (Article #5088):           // LNCV0
  Read CV0
  [E5 0F 05 49 4B 1F 01 60 13 00 00 01 00 00 7E]  (LNCV) READ_CV_REPLY from module (Article #5088):
  CV0 value = 1
  [ED 0F 01 05 00 20 01 60 13 00 00 01 00 00 4A]  (LNCV) WRITE_CV request to module (Article #5088):             // LNCV0
  Set value in CV0 to: 1
  [B4 6D 7F 59]  LONG_ACK: the Send IMM Packet command was accepted.
  [E5 0F 01 05 00 21 03 7F 7F 00 00 01 00 40 72]  (LNCV) General ALL_PROG_END command

  JMRI Article empty
  [ED 0F 01 05 00 21 73 7F 7F 00 00 7F 7F 00 4B]  (LNCV) General ALL_PROG_START command
  [ED 0F 01 05 00 21 73 7F 7F 00 00 7F 7F 00 4B]  (LNCV) General ALL_PROG_START command
  [E5 0F 05 49 4B 1F 41 60 13 00 00 01 00 00 3E]  (LNCV) READ_CV_REPLY from module (Article #5088):
  CV0 value = 1
  [E5 0F 01 05 00 21 33 7F 7F 00 00 7F 7F 40 43]  (LNCV) General ALL_PROG_END command




  OPC_LONG_ACK            0xB4  ; Long acknowledge
  OPC_PEER_XFER           0xE5  ; Move 8 bytes PEER to PEER
  OPC_IMM_PACKET          0xED  ; Send n-byte packet immediate

  LNCV_SRC_KPU            0x01  ; KPU is, e.g., an IntelliBox
  LNCV_SRC_MODULE         0x05
  LNCV_IB_KPU_DSTL        'I'
  LNCV_IB_KPU_DSTH        'K'

  LNCV_REQID_CFGREAD      0x1F
  LNCV_REQID_CFGWRITE     0x20
  LNCV_REQID_CFGREQUEST   0x21

  LNCV_FLAG_PRON          0x80
  LNCV_FLAG_PROFF         0x40
  LNCV_FLAG_RO            0x01

  ARTNUML                 0xAF  ; Article Number: 5039 (0x13AF)
  ARTNUMH                 0x13

  <0xE5><0x0F><SRC><DESTL><DESTH><ID><PCXT1><CLASSL><CLASSH><LNCVL><LNCVH><VALL><VALH><FLAGS><CHK>   Answer
     ED    0F   01     05     00  21     41      2F      13     00     00    01    00     00   44
     E5    0F   05     49     4B  1F     01      2F      13     00     00    01    00     00   31
     ED    0F   01     05     00  21     41      60      13     00     00    01    00     00   0B

*/


#ifdef USE_LNCV

void sendLNCV (byte id, byte flags) {
  byte i;
  SendPacket.data[0]  = (flags == LNCV_FLAG_PROFF) ? OPC_PEER_XFER : OPC_IMM_PACKET;
  SendPacket.data[1]  = 0x0F;
  SendPacket.data[2]  = 0x01;
  SendPacket.data[3]  = 0x05;
  SendPacket.data[4]  = 0x00;
  SendPacket.data[5]  = id;
  SendPacket.data[6]  = 0x00;                   // PXCT1
  SendPacket.data[7]  = lowByte(artNum);
  SendPacket.data[8]  = highByte(artNum);
  SendPacket.data[9]  = lowByte(numLNCV);
  SendPacket.data[10] = highByte(numLNCV);
  SendPacket.data[11] = lowByte(valLNCV);
  SendPacket.data[12] = highByte(valLNCV);
  SendPacket.data[13] = flags;
  for (i = 0; i < 7; i++) {                     // set bits in PXCT1
    if (SendPacket.data[7 + i] & 0x80) {
      bitSet(SendPacket.data[6], i);
      bitClear(SendPacket.data[7 + i], 7);
    }
  }
  lnetSend( &SendPacket);
}


/*
  // [  ED    0F   01    49     42   0D    00    48    01   00   00    00    00      00    53]    ask loco 328 (0x148)
  //    ED    0F   01    49     42   0D    00    06    00   00   00    00    00      00    1C     ask loco 6
  // <0xE5><0x0F><SRC><DESTL><DESTH><ID><PCXT1><ADRL><ADRH><STP><???><VIRTL><VIRTH><???><CHK>   Answer
  //    E5    0F  00     49     4B   0B    04    06    00   52   00    00    00      00    4C     loco 6 in 28 steps
  //    E5    0F  00     49     4B   0B    04    09    00   52   11    09    00      04    5F     loco 9 in 28 steps
  //    E5    0F  00     49     4B   0B    00    03    00   53   51    03    00      00    1E     loco 3 in 128 steps

  // STP:  0xD3-128steps, 0xD2-28steps
  // VIRT: Virtual address (slot contains this address)
  void sendLocoIB() {
  byte i;
  SendPacket.data[0]  = OPC_IMM_PACKET;
  SendPacket.data[1]  = 0x0F;
  SendPacket.data[2]  = 0x01;
  SendPacket.data[3]  = 'I';
  SendPacket.data[4]  = 'B';
  SendPacket.data[5]  = 0x0D;
  SendPacket.data[6]  = 0x00;                   // PXCT1
  SendPacket.data[7]  = lowByte(myLoco.address);
  SendPacket.data[8]  = highByte(myLoco.address);
  SendPacket.data[9]  = 0;
  SendPacket.data[10] = 0;
  //SendPacket.data[9]  = lowByte(myID);
  //SendPacket.data[10] = highByte(myID);
  SendPacket.data[11] = 0;
  SendPacket.data[12] = 0;
  SendPacket.data[13] = 0;
  for (i = 0; i < 7; i++) {                     // set bits in PXCT1
    if (SendPacket.data[7 + i] & 0x80) {
      bitSet(SendPacket.data[6], i);
      bitClear(SendPacket.data[7 + i], 7);
    }
  }
  lnetSend( &SendPacket);
  }
*/
#endif


/*
  Reading Slot #0 can be used to identify the Intellibox. In fact, it can be
  used to identify all of our 'LocoNet master' devices: one can tell that it is
  an Intellibox, a DAISY or a Power 2 in analog mode by looking at the answer
  of a Slot #0 read request. One would find that ID1 and ID2 have these values:
  ID1 = 'I', ID2 = 'B'.
  Furthermore, the value of the Speed byte tells the "level" of the LocoNet
  driver of the Command Station. The current firmware of the Intellibox (IB),
  of the DAISY and of the Power2 (in analog mode) report the Speed byte as
  having value 2 (this basically means: the LocoNet driver supports our FRED
  - used in the "intelligent, 4 locos" mode).
  Now, if one needs to find out exactly to which of our Command Stations one
  is connected to (Intellibox, DAISY, Power2 in analog mode - or Digitrax),
  then a Slot write of Slot #0 has to be performed (e.g. using the very values
  one just read from that Slot):
  - a Power 2 in analog mode does not reply at all
  - an IB/TC replies with LACK, error = 0Eh (OPC_WR_SL_DATA length)
  - a DAISY replies with LACK, error = 1
  - a Digitrax Chief replies with LACK 'Ok' (7Fh)
  One can tell a Digitrax CS also by looking at ID1 and ID2 of the Slot #0 read.

  Detect Hardware type (Read Slot 0)
  0xE7, 0x0E, 0x00, 0x00, 0x00, 0x02, 0x00, 0x07, 0x00, 0x00, 0x00, 0x49, 0x42, 0x18  "Intellibox / TwinCenter"               ADR: 0    ID: 'IB'  SPD: 2
  0xE7, 0x0E, 0x00, 0x03, 0x00, 0x03, 0x00, 0x06, 0x08, 0x00, 0x00, 0x49, 0x42, 0x13  "DR5000"                                ADR: 0    ID: 'IB'  SPD: 3
  0xE7, 0x0E, 0x00, 0x03, 0x00, 0x03, 0x00, 0x07, 0x08, 0x00, 0x00, 0x49, 0x42, 0x12  "YD7001"                                ADR: 0    ID: 'IB'  SPD: 3
  0xE7, 0x0E, 0x00, 0x02, 0x42, 0x03, 0x00, 0x07, 0x00, 0x00, 0x15, 0x49, 0x42, 0x4C  "Intellibox II / IB-Basic / IB-Com"     ADR: 'B'  ID: 'IB'  SPD: 3
  0xE7, 0x0E, 0x00, 0x02, 0x42, 0x03, 0x00, 0x06, 0x00, 0x00, 0x15, 0x49, 0x42, 0x4D  "System Control 7"                      ADR: 'B'  ID: 'IB'  SPD: 3
  0xE7, 0x0E, 0x00, 0x02, 0x42, 0x03, 0x00, 0x07, 0x00, 0x00, 0x15, 0x49, 0x42, 0x4C  "Daisy II Tillig"                       ADR: 'B'  ID: 'IB'  SPD: 3
  0xE7, 0x0E, 0x00, 0x00, 0x44, 0x02, 0x00, 0x07, 0x00, 0x59, 0x01, 0x49, 0x42, 0x04  "Daisy"                                 ADR: 'DY' ID: 'IB'  SPD: 2
  0xE7, 0x0E, 0x00, 0x00, 0x4C, 0x01, 0x00, 0x07, 0x00, 0x49, 0x02, 0x49, 0x42, 0x1C  "Adapter 63820"                         ADR: 'LI' ID: 'IB'  SPD: 1
  0xE7, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x5A, 0x21, 0x6A  "Z21 Black"                             ADR: 0    ID: 'Z'21 SPD: 0
  0xE7, 0x0E, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11  "Digitrax Chief"                        ADR: 0    ID: 0     SPD: 0
  0xe7, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11  DCS100
  0xe7, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12  DCS200
  0xe7, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12  DCS50
  0xe7, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52  DCS52
  0xe7, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x25, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30  DT200
  0xe7, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x51  DCS210
  0xe7, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x11, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40  DCS240
  0xe7, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x20, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x71  DCS240+
  0xe7, 0x0e, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16  DB150                                   ADR: 0    ID: 0     SPD: 4
  0xe7, 0x0e, 0x00, 0x33, 0x0e, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x2d  DCS51                                   ADR: 0    ID: 1     SPD: 0
  0xE7, 0x0E, 0x00, 0x00, 0x4E, 0x10, 0x01, 0x07, 0x00, 0x4C, 0x00, 0x4A, 0x46, 0x09  NanoL                                   ADR: 'NL' ID: 'JF'  SPD: 16
  0xB4, 0x3B, 0x00, 0x70                                                              RB1110                                  not supported

              SLOT  STAT1 ADRL  SPD   DIRF  TRK   STAT2 ADH   SND   ID1   ID2   CHK

*/

/*
  Software version:  ED 0F 01 49 42 06 00 00 00 00 00 00 00 00 11
            DR5000:  E5 0F 00 49 42 08 00 00 20 15 10 14 08 00 2F -> v1.6.2
            YD7001:  E5 0F 00 49 42 08 00 00 20 15 10 14 08 00 2F ->
        TwinCenter:  E5 0F 00 49 4B 08 00 00 11 13 10 14 0F 00 16 -> v1.100
               IB2:  E5 0F 00 49 4B 08 00 26 10 01 01 01 00 00 28 -> v1.025 - 1.026
             Daisy:  E5 0F 00 49 4B 08 00 02 10 00 00 00 00 00 0D -> 1002 (LNCV 90)
            RB1110:  B4 7D 7F 49                                  -> not supported


     Serial number:  ED 0F 01 49 42 07 00 00 00 00 00 00 00 00 10
            DR5000:  E5 0F 00 49 4B 09 00 10 00 03 07 72 00 00 78 ->
            YD7001:  E5 0F 00 49 4B 09 00 10 00 03 07 72 00 00 78 ->
        TwinCenter:  E5 0F 00 49 4B 09 00 20 00 00 59 31 00 00 56 -> 2000005931
               IB2:  E5 0F 00 49 4B 09 00 12 00 01 60 60 00 00 0D -> 1200016060
             Daisy:  E5 0F 00 49 4B 09 00 13 00 00 06 05 00 00 0E ->
            RB1110:  B4 7D 7F 49                                  -> not supported

*/

/*
   Timbre. Phone Bell, Cloche, Campainha, Telefonklingel, Campanello, Telefoon bel
*/

#endif
