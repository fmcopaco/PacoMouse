/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/


#ifdef USE_Z21
////////////////////////////////////////////////////////////
// ***** WIFI SOPORTE *****
////////////////////////////////////////////////////////////

void beginZ21() {
  byte line, text, n;
  oled.setInvertMode(true);
  oled.setFont(Sym_Bars);
  for (line = 0; line < 8; line ++) {
    oled.setCursor (0, line);
    for (text = 0; text < 8; text++) {
      oled.print('/');
    }
    yield();
  }
  oled.setFont(Num_32x48);
  oled.setCursor(48, 1);
  oled.print('<');
  oled.setFont(Arial_bold_14);
  oled.setCursor (24, 5);
  printOtherTxt(TXT_CONNECT);
  oled.setInvertMode(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSetting.ssid, wifiSetting.password);
  n = 0;
  while ((WiFi.status() != WL_CONNECTED) && n < 40) {         // tries to connect to router in 20 seconds
    n++;
    delay(500);
    DEBUG_MSG(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_MSG("IP address: %u.%u.%u.%u", WiFi.localIP().operator[](0), WiFi.localIP().operator[](1), WiFi.localIP().operator[](2), WiFi.localIP().operator[](3));
    DEBUG_MSG("Channel: %d", WiFi.channel());
    Udp.begin(z21Port);
    DEBUG_MSG("Now listening UDP port %d", z21Port);
    getStatusZ21();                                           // every x seconds
    getSerialNumber();
    getStatusZ21();
    setBroadcastFlags (0x00000013);                           // Broadcasts and info messages concerning driving and switching, report changes on feedback bus & fast clock
  }
}


////////////////////////////////////////////////////////////
// ***** Z21 SOPORTE *****
////////////////////////////////////////////////////////////

void mueveDesvio() {
  setTurnout (myTurnout, myPosTurnout - DESVIADO, true);
  accEnviado = true;
}


void readCV (unsigned int adr, byte stepPrg) {
  askZ21begin (LAN_X_Header);
  askZ21data (0x23);
  askZ21data (0x11);
  adr--;
  askZ21data ((adr >> 8) & 0xFF);
  askZ21data (adr & 0xFF);
  askZ21xor ();
  sendUDP (0x09);
  waitResultCV = true;
  lastCV = lowByte(adr);
  progStepCV = stepPrg;
  DEBUG_MSG("Read CV %d", adr + 1);
}


void writeCV (unsigned int adr, unsigned int data, byte stepPrg) {
  if (modeProg) {
    askZ21begin (LAN_X_Header);
    askZ21data (0xE6);
    askZ21data (0x30);
    askZ21data (myLoco.adr[1] & 0x3F);
    askZ21data (myLoco.adr[0]);
    adr--;
    askZ21data (0xEC | ((adr >> 8) & 0x03));
    askZ21data (adr & 0xFF);
    askZ21data (data);
    askZ21xor ();
    sendUDP (0x0C);
  }
  else {
    askZ21begin (LAN_X_Header);
    askZ21data (0x24);
    askZ21data (0x12);
    adr--;
    askZ21data ((adr >> 8) & 0xFF);
    askZ21data (adr & 0xFF);
    askZ21data (data);
    askZ21xor ();
    sendUDP (0x09);
    waitResultCV = true;
    lastCV = lowByte(adr);

  }
  progStepCV = stepPrg;
  DEBUG_MSG("Write CV%d = %d", adr + 1, data);
}


void showErrorXnet() {                                        // muestra pantalla de error
  if (csStatus & csTrackVoltageOff)
    errOLED = ERR_OFF;
  if (csStatus & csProgrammingModeActive)
    errOLED = ERR_SERV;
  if (csStatus & csEmergencyStop)
    errOLED = ERR_STOP;
  scrOLED = SCR_ERROR;
  updateOLED = true;
}


void showNormalOps() {
  if (scrOLED == SCR_ERROR) {                                 // sale de la pantalla de error
    scrOLED = SCR_ENDLOGO;
    updateOLED = true;
  }
}

#ifdef USE_SET_TIME
void setTime(byte hh, byte mm, byte rate) {
  clockHour = hh;
  clockMin = mm;
  clockRate = rate;
  askZ21begin (LAN_FAST_CLOCK_CONTROL);                       // set clock
  if (rate > 0) {
    askZ21data (0x24);
    askZ21data (0x2B);
    askZ21data (hh);
    askZ21data (mm);
    askZ21data (rate);
    askZ21xor ();
    sendUDP (0x0A);
    askZ21begin (LAN_FAST_CLOCK_CONTROL);
    askZ21data (0x21);                                        // start clock
    askZ21data (0x2C);
    askZ21xor ();
    sendUDP (0x07);
  }
  else {
    askZ21data (0x21);                                        // recommended for rate=0. stop clock
    askZ21data (0x2D);
    askZ21xor ();
    sendUDP (0x07);
  }
}
#endif

////////////////////////////////////////////////////////////
// ***** Z21 DECODE *****
////////////////////////////////////////////////////////////


void z21Process() {
  int len, packetSize;

  packetSize = Udp.parsePacket();                             // z21 UDP packet
  if (packetSize) {
    len = Udp.read(packetBuffer, packetSize);                 // read the packet into packetBufffer
    ReceiveZ21 (len, packetBuffer);                           // decode received packet
  }
  yield();
  if (millis() - infoTimer > 1000UL) {                        // Cada segundo
    infoTimer = millis();
    pingTimer++;
    if (pingTimer >= PING_INTERVAL) {
      pingTimer = 0;
      if (!(csStatus & csProgrammingModeActive))
        getStatusZ21();
    }
    battery = ESP.getVcc ();                                  // Read VCC voltage
    if (battery < LowBattADC)
      lowBATT = true;
  }
  if (progFinished) {                                         // fin de lectura/programacion CV
    progFinished = false;
    endProg();
  }
  yield();
}

// -------------------------------------------------------------------------------------

void setBroadcastFlags (unsigned long bFlags) {
  askZ21begin (LAN_SET_BROADCASTFLAGS);
  askZ21data (bFlags & 0xFF);
  askZ21data ((bFlags >> 8) & 0xFF);
  askZ21data ((bFlags >> 16) & 0xFF);
  askZ21data ((bFlags >> 24) & 0xFF);
  sendUDP (0x08);
}

void resumeOperations () {                                    // LAN_X_SET_TRACK_POWER_ON
  askZ21begin (LAN_X_Header);
  askZ21data (0x21);
  askZ21data (0x81);
  askZ21xor ();
  sendUDP (0x07);
}


void emergencyOff() {                                         // LAN_X_SET_TRACK_POWER_OFF
  askZ21begin (LAN_X_Header);
  askZ21data (0x21);
  askZ21data (0x80);
  askZ21xor ();
  sendUDP (0x07);
}


void getStatusZ21 () {
  askZ21begin (LAN_X_Header);
  askZ21data (0x21);
  askZ21data (0x24);
  askZ21xor ();
  sendUDP (0x07);
}

void getSerialNumber () {
  askZ21begin (LAN_GET_SERIAL_NUMBER);
  sendUDP (0x04);
}

void infoLocomotora (unsigned int Adr) {
  byte Adr_MSB;
  askZ21begin (LAN_X_Header);
  askZ21data (0xE3);
  askZ21data (0xF0);
  Adr_MSB = (Adr >> 8) & 0x3F;
  if (Adr & 0x3F80)
    Adr_MSB |= 0xC0;
  askZ21data (Adr_MSB);
  askZ21data (Adr & 0xFF);
  askZ21xor ();
  sendUDP (0x08);
}

void locoOperationSpeed() {
  byte Adr_MSB;
  askZ21begin (LAN_X_Header);
  askZ21data (0xE4);
  if (bitRead(mySteps, 2)) {                                  // 128 steps
    askZ21data (0x13);
  }
  else {
    if (bitRead(mySteps, 1)) {                                // 28 steps
      askZ21data (0x12);
    }
    else {
      askZ21data (0x10);                                      // 14 steps
    }
  }
  Adr_MSB = myLoco.adr[1] & 0x3F;
  if (myLoco.address & 0x3F80)
    Adr_MSB |= 0xC0;
  askZ21data (Adr_MSB);
  askZ21data (myLoco.adr[0]);
  askZ21data (mySpeed | myDir);
  askZ21xor ();
  sendUDP (0x0A);
  bitClear(mySteps, 3);                                     // currently operated by me
  if (scrOLED == SCR_SPEED)
    updateOLED = true;
}


byte getCurrentStep() {
  byte currStep;
  if (bitRead(mySteps, 2)) {                                // 128 steps -> 0..126
    if (mySpeed > 1)
      return (mySpeed - 1);
  }
  else {
    if (bitRead(mySteps, 1)) {                              // 28 steps -> 0..28    '---04321' -> '---43210'
      currStep = (mySpeed << 1) & 0x1F;
      bitWrite(currStep, 0, bitRead(mySpeed, 4));
      if (currStep > 3)
        return (currStep - 3);
    }
    else {                                                  // 14 steps -> 0..14
      if (mySpeed > 1)
        return (mySpeed - 1);
    }
  }
  return (0);
}


void funcOperations (byte fnc) {
  byte Adr_MSB;
  askZ21begin (LAN_X_Header);
  askZ21data (0xE4);
  askZ21data (0xF8);
  Adr_MSB = myLoco.adr[1] & 0x3F;
  if (myLoco.address & 0x3F80)
    Adr_MSB |= 0xC0;
  askZ21data (Adr_MSB);
  askZ21data (myLoco.adr[0]);
  if (bitRead(myFunc.Bits, fnc))
    askZ21data (fnc | 0x40);
  else
    askZ21data (fnc);
  askZ21xor ();
  sendUDP (0x0A);
  bitClear(mySteps, 3);                                     // currently operated by me
}


void infoDesvio (unsigned int FAdr) {
  FAdr--;
  askZ21begin (LAN_X_Header);
  askZ21data (0x43);
  askZ21data ((FAdr >> 8) & 0xFF);
  askZ21data (FAdr & 0xFF);
  askZ21xor ();
  sendUDP (0x08);
}


void setTurnout (unsigned int FAdr, int pair, bool active) {
  byte db2;
  FAdr--;
  askZ21begin (LAN_X_Header);
  askZ21data (0x53);
  askZ21data ((FAdr >> 8) & 0xFF);
  askZ21data (FAdr & 0xFF);
  db2 = active ? 0x88 : 0x80;
  if (pair > 0)
    db2 |= 0x01;
  askZ21data (db2);                                         // '10Q0A00P'
  askZ21xor ();
  sendUDP (0x09);
}


void getFeedbackInfo (byte group) {
  askZ21begin (LAN_RMBUS_GETDATA);
  askZ21data (group);
  sendUDP (0x05);
}


void ReceiveZ21 (int len, byte *packet) {                   // get UDP packet, maybe more than one!!
  int  DataLen, isPacket;
#ifdef DEBUG
  Serial.print("\nRX Length: ");
  Serial.println (len);
  for (int i = 0; i < len; i++) {
    Serial.print(packet[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif
  isPacket = 1;
  while (isPacket) {
    DataLen = (packet[DATA_LENH] << 8) + packet[DATA_LENL];
    DecodeZ21 (DataLen, packet);
    if (DataLen >= len) {
      isPacket = 0;
    }
    else {
      packet = packet + DataLen;
      len = len - DataLen;
    }
    yield();
  }
}


void DecodeZ21 (int len, byte *packet) {                      // decode z21 UDP packets
  int Header, DataLen;
  unsigned int FAdr;
  byte group;

  Header = (packet[DATA_HEADERH] << 8) + packet[DATA_HEADERL];
  switch (Header) {
    case  LAN_GET_SERIAL_NUMBER:
      break;
    case LAN_GET_CODE:                                        // FW 1.28
      break;
    case LAN_GET_HWINFO:
      break;
    case LAN_GET_BROADCASTFLAGS:
      break;
    case LAN_GET_LOCOMODE:
      break;
    case LAN_GET_TURNOUTMODE:
      break;
    case LAN_RMBUS_DATACHANGED:
      if (Shuttle.moduleA > 0) {                              // only check shuttle contacts
        if ((packet[4] == 0x01) && (Shuttle.moduleA > 10))
          Shuttle.statusA = packet[Shuttle.moduleA - 6];
        if ((packet[4] == 0x00) && (Shuttle.moduleA < 11))
          Shuttle.statusA = packet[Shuttle.moduleA + 4];
      }
      if (Shuttle.moduleB > 0) {
        if ((packet[4] == 0x01) && (Shuttle.moduleB > 10))
          Shuttle.statusB = packet[Shuttle.moduleB - 6];
        if ((packet[4] == 0x00) && (Shuttle.moduleB < 11))
          Shuttle.statusB = packet[Shuttle.moduleB + 4];
      }
#ifdef USE_AUTOMATION
      for (byte n = 0; n < MAX_AUTO_SEQ; n++) {
        if ((automation[n].opcode & OPC_AUTO_MASK) == OPC_AUTO_FBK) {
          if ((packet[4] == 0x01) && (automation[n].param > 9))
            automation[n].value = packet[automation[n].param - 5];
          if ((packet[4] == 0x00) && (automation[n].param < 10))
            automation[n].value = packet[automation[n].param + 5];
          DEBUG_MSG("RBUS %d", automation[n].value)
        }
      }
#endif
      break;
    case LAN_SYSTEMSTATE_DATACHANGED:
      csStatus = packet[16] & (csEmergencyStop | csTrackVoltageOff | csProgrammingModeActive);    // CentralState
      if (packet[16] & csShortCircuit)
        csStatus |= csTrackVoltageOff;
      break;
    case LAN_RAILCOM_DATACHANGED:
      break;

    case LAN_LOCONET_Z21_TX:                                  // a message has been written to the LocoNet bus by the Z21.
    case LAN_LOCONET_Z21_RX:                                  // a message has been received by the Z21 from the LocoNet bus.
    case LAN_LOCONET_FROM_LAN:                                // another LAN client has written a message to the LocoNet bus via the Z21.
      switch (packet[4]) {
        case 0x83:
          csStatus = csNormalOps;                             // OPC_GPON
          showNormalOps();
          break;
        case 0x82:
          csStatus |= csTrackVoltageOff;                      // OPC_GPOFF
          showErrorXnet();
          break;
      }
      break;
    case LAN_LOCONET_DETECTOR:
      break;
    case LAN_FAST_CLOCK_DATA:                                 // fast clock data FW 1.43
      if (packet[8] & 0x80) {                                 // Stop flag
        clockRate = 0;
      }
      else {
        clockHour = packet[6] & 0x1F;
        clockMin = packet[7] & 0x3F;
        clockRate = packet[9] & 0x3F;
        if (scrOLED == SCR_SPEED) {
          updateExtraOLED = true;
          updateOLED = true;
        }
      }
      DEBUG_MSG("Clock: %d:%d %d", clockHour, clockMin, clockRate);
      break;

    case LAN_X_Header:
      switch (packet[XHEADER]) {
        case 0x43:                                            // LAN_X_TURNOUT_INFO
          FAdr = (packet[DB0] << 8) + packet[DB1] + 1;
          if (FAdr == myTurnout) {
            myPosTurnout = packet[DB2] & 0x03;
            if (scrOLED == SCR_TURNOUT)
              updateOLED = true;
          }
          break;
        case 0x61:
          switch (packet[DB0]) {
            case 0x01:                                        // LAN_X_BC_TRACK_POWER_ON
              csStatus = csNormalOps;
              showNormalOps();
              break;
            case 0x08:                                        // LAN_X_BC_TRACK_SHORT_CIRCUIT
              csStatus |= csShortCircuit;
            case 0x00:                                        // LAN_X_BC_TRACK_POWER_OFF
              csStatus |= csTrackVoltageOff;
              showErrorXnet();
              break;
            case 0x02:                                        // LAN_X_BC_PROGRAMMING_MODE
              csStatus |= csProgrammingModeActive;
              if (!waitResultCV)
                showErrorXnet();
              break;
            case 0x12:                                        // LAN_X_CV_NACK_SC
            case 0x13:                                        // LAN_X_CV_NACK
              CVdata = 0x0600;
              waitResultCV = false;
              progFinished = true;
              break;
            case 0x82:                                        // LAN_X_UNKNOWN_COMMAND
              break;
          }
          break;
        case 0x62:
          switch (packet[DB0]) {
            case 0x22:                                        // LAN_X_STATUS_CHANGED
              csStatus = packet[DB1] & (csEmergencyStop | csTrackVoltageOff | csProgrammingModeActive);
              if (packet[DB1] & csShortCircuit)
                csStatus |= csTrackVoltageOff;
              if (csStatus == csNormalOps)
                showNormalOps();
              else
                showErrorXnet();
              break;
          }
          break;
        case 0x64:
          if (packet[DB0] == 0x14) {                            // LAN_X_CV_RESULT
            if (packet[DB2] == lastCV) {
              lastCV ^= 0x55;
              CVdata = packet[DB3];
              waitResultCV = false;
              progFinished = true;
            }
          }
          break;
        case 0x81:
          if (packet[DB0] == 0) {                               // LAN_X_BC_STOPPED
            csStatus |= csEmergencyStop;
            showErrorXnet();
          }
          break;
        case 0xEF:                                              // LAN_X_LOCO_INFO
          FAdr = ((packet[DB0] << 8) + packet[DB1]) & 0x3FFF;
          if (FAdr == myLoco.address) {
            mySteps = packet[DB2];                              // '0000BFFF'
            myDir = packet[DB3] & 0x80;                         // 'RVVVVVVV'
            mySpeed = packet[DB3] & 0x7F;
            myFunc.Bits &= 0xE0000000;                          // '000FFFFF','FFFFFFFF'
            myFunc.xFunc[0] |= ((packet[DB4] & 0x0F) << 1);
            bitWrite(myFunc.xFunc[0], 0, bitRead(packet[DB4], 4));
            myFunc.Bits |= (unsigned long)(packet[DB5] << 5);
            myFunc.Bits |= (unsigned long)(packet[DB6] << 13);
            myFunc.Bits |= (unsigned long)(packet[DB7] << 21);
            updateSpeedHID();
          }
          break;
      }
      break;

    default:                                                  // Header other
      break;

  }
}


void askZ21begin (unsigned int header) {
  OutData[DATA_HEADERL] = header & 0xFF;
  OutData[DATA_HEADERH] = header >> 8;
  OutPos = XHEADER;
  OutXOR = 0;
}


void askZ21data (byte data) {
  OutData[OutPos++] = data;
  OutXOR ^= data;
}

void askZ21xor () {
  OutData[OutPos] = OutXOR;
}


void sendUDP (int len) {
  OutData[DATA_LENL] = len & 0xFF;
  OutData[DATA_LENH] = len >> 8;
  Udp.beginPacket(wifiSetting.CS_IP, z21Port);
  Udp.write(OutData, len);
  Udp.endPacket();
  yield();
#ifdef DEBUG
  Serial.print("TX: ");
  for (int i = 0; i < len; i++) {
    Serial.print(OutData[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif
}

#endif
