/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/


#ifdef USE_XPRESSNET
////////////////////////////////////////////////////////////
// ***** XPRESSNET SOPORTE *****
////////////////////////////////////////////////////////////

void mueveDesvio () {                                       // mueve el desvio
  enviaAccesorio(myTurnout, true, myPosTurnout - DESVIADO);
  accEnviado = true;
}


void infoDesvio(unsigned int num) {
  num--;
  miModulo = num >> 2;
  miAccPos = num & 0x03;
  infoAccesorio(miModulo, bitRead(num, 1));                 // modulo/nibble
}


void showErrorXnet() {                                      // muestra pantalla de error
  if (csStatus & csEmergencyOff)
    errOLED = ERR_OFF;
  if (csStatus & csServiceMode)
    errOLED = ERR_SERV;
  if (csStatus & csEmergencyStop)
    errOLED = ERR_STOP;
  scrOLED = SCR_ERROR;
  updateOLED = true;
}


void showNormalOps() {
  if (scrOLED == SCR_ERROR) {                               // sale de la pantalla de error
    scrOLED = SCR_ENDLOGO;
    updateOLED = true;
  }
}


bool isRecentMM () {                                        // Comprueba central Multimaus reciente
  if ((xnetCS == 0x10) && (highVerMM > 0) && (lowVerMM > 0x02))
    return true;
  else
    return false;
}


#ifdef USE_SET_TIME
void setTime(byte hh, byte mm, byte rate) {
  clockHour = hh;
  clockMin = mm;
  clockRate = rate;
  if (rate > 0) {
    headerXN (0x24);                                        // set clock
    dataXN (0x2B);
    dataXN (hh);
    dataXN (mm);
    dataXN (rate);
    sendXN ();
    /*
    headerXN (0x21);                                        // start clock
    dataXN (0x2C);
    sendXN (0x07);
    */
  }
  else {
    headerXN (0x21);                                        // recommended for rate=0. stop clock
    dataXN (0x2D);
    sendXN ();
  }
}
#endif


////////////////////////////////////////////////////////////
// ***** XPRESSNET DECODE *****
////////////////////////////////////////////////////////////

void procesaXN () {
  byte n, longitud, modulo, dato;

  switch (rxBufferXN[HEADER]) {                             // segun el header byte
    case 0x61:
      switch (rxBufferXN[DATA1]) {
        case 0x01:                                          // Normal operation resumed                   (0x61,0x01,0x60)
          csStatus = csNormalOps;
          showNormalOps();
          break;
        case 0x08:                                          // Z21 LAN_X_BC_TRACK_SHORT_CIRCUIT           (0x61,0x08,XOR)
        case 0x00:                                          // Track power off                            (0x61,0x00,0x61)
          csStatus |= csEmergencyOff;
          showErrorXnet();
          break;
        case 0x02:                                          // Service mode entry                         (0x61,0x02,0x63)
          csStatus |= csServiceMode;
          if (!getResultsSM)                                // show 'Service Mode' if we aren't programming CV
            showErrorXnet();
          break;
        case 0x12:                                          // Programming info. "shortcircuit"           (0x61,0x12,XOR)
        case 0x13:                                          // Programming info. "Data byte not found"    (0x61,0x13,XOR)
          CVdata = 0x0600;
          getResultsSM = false;
          progFinished = true;
          break;
        case 0x81:                                          // Command station busy response              (0x61,0x81,XOR)
          break;
        case 0x1F:                                          // Programming info. "Command station busy"   (0x61,0x1F,XOR)
          getResultsSM = true;
          infoTimer = millis();
          break;
        case 0x82:                                          // Instruction not supported by command station (0x61,0x82,XOR)
          getResultsSM = false;
          if (csStatus & csServiceMode) {
            CVdata = 0x0600;
            progFinished = true;
          }
          break;
      }
      break;
    case 0x81:
      if (rxBufferXN[DATA1] == 0) {                         // Emergency Stop                             (0x81,0x00,0x81)
        csStatus |= csEmergencyStop;
        showErrorXnet();
      }
      break;
    case 0x62:
      if (rxBufferXN[DATA1] == 0x22) {                      // Command station status indication response (0x62,0x22,DATA,XOR)
        csStatus = rxBufferXN[DATA2] & (csEmergencyStop | csEmergencyOff | csServiceMode) ;
        if ((xnetCS >= 0x10) && (rxBufferXN[DATA2] & csProgrammingModeActive))   // Multimaus/Z21 Service Mode
          csStatus |= csServiceMode;
        if (csStatus == csNormalOps)
          showNormalOps();
        else
          showErrorXnet();
      }
      break;
    case 0x63:
      switch (rxBufferXN[DATA1]) {
        case 0x03:                                          // Broadcast "Modellzeit"                     (0x63,0x03,dddhhhhh,s0mmmmmm,XOR) (v4.0)
          clockHour = rxBufferXN[DATA2] & 0x1F;
          clockMin = rxBufferXN[DATA3] & 0x3F;
          clockRate = !bitRead(rxBufferXN[DATA3], 7);
          if (scrOLED == SCR_SPEED) {
            updateExtraOLED = true;
            updateOLED = true;
          }
          break;
        case 0x14:                                          // Service Mode response for Direct CV mode   (0x63,0x1x,CV,DATA,XOR)
        case 0x15:
        case 0x16:
        case 0x17:
          if (rxBufferXN[DATA2] == lastCV) {                // comprobar CV (DR5000)
            lastCV ^= 0x55;
            getResultsSM = false;
            CVdata = rxBufferXN[DATA3];
            progFinished = true;
          }
          break;
        case 0x21:                                          // Command station software version           (0x63,0x21,VER,ID,XOR)
          xnetVersion = rxBufferXN[DATA2];
          xnetCS = rxBufferXN[DATA3];
          if (xnetCS == 0x10)
            askMultimaus = true;
          break;
      }
      break;
    case 0xE3:
      if (rxBufferXN[DATA1] == 0x40) {                      // Locomotive is being operated by another device response  (0xE3,0x40,ADRH,ADRL,XOR)
        if ((rxBufferXN[DATA3] == myLoco.adr[0]) && (rxBufferXN[DATA2] == myLoco.adr[1])) {           // DR5000 workaround
          bitSet(mySteps, 3);
          if (scrOLED == SCR_SPEED) {
            updateOLED = true;
          }
        }
      }
      if (rxBufferXN[DATA1] == 0x52) {                      // Locomotive function info F13..F28          (0xE3,0x52,FNC,FNC,XOR)
        myFunc.Bits &= 0xE0001FFF;
        myFunc.Bits |= ((unsigned long)rxBufferXN[DATA2] << 13);
        myFunc.Bits |= ((unsigned long)rxBufferXN[DATA3] << 21);
        if (scrOLED == SCR_SPEED)
          updateOLED = true;
      }
      break;
    case 0xE4:
      if ((rxBufferXN[DATA1] & 0xF0) == 0x00) {             // Locomotive information normal locomotive   (0xE4,ID,SPD,FKTA,FKTB,XOR)
        mySteps = rxBufferXN[DATA1];                        // '0000BFFF'
        myDir = rxBufferXN[DATA2] & 0x80;                   // 'RVVVVVVV'
        mySpeed = rxBufferXN[DATA2] & 0x7F;
        myFunc.Bits &= 0xFFFFE000;                          // '000FFFFF','FFFFFFFF'
        myFunc.Bits |= ((unsigned long)rxBufferXN[DATA4] << 5);
        myFunc.xFunc[0] |= ((rxBufferXN[DATA3] & 0x0F) << 1);
        bitWrite(myFunc.xFunc[0], 0, bitRead(rxBufferXN[DATA3], 4));
        updateSpeedHID();
      }
      break;
    case 0xE7:
      if ((rxBufferXN[DATA1] & 0xF0) == 0x00) {             // Locomotive function info F13..F20 MM       (0xE7,STP,SPD,FNC,FNC,FNC,0x00,0x00,XOR)
        mySteps = rxBufferXN[DATA1];                        // '0000BFFF'
        myDir = rxBufferXN[DATA2] & 0x80;                   // 'RVVVVVVV'
        mySpeed = rxBufferXN[DATA2] & 0x7F;
        myFunc.Bits &= 0xFE00000;
        myFunc.Bits |= ((unsigned long)rxBufferXN[DATA5] << 13);
        myFunc.Bits |= ((unsigned long)rxBufferXN[DATA4] << 5);
        myFunc.xFunc[0] |= ((rxBufferXN[DATA3] & 0x0F) << 1);
        bitWrite(myFunc.xFunc[0], 0, bitRead(rxBufferXN[DATA3], 4));
        updateSpeedHID();
      }
      /*
        case 0xE6:
        case 0xE8:
        case 0xE9:
        case 0xEA:
        if (rxBufferXN[DATA1] == 0xF1) {                      // Library Entry                              (0xEx,0xF1,ADRH,ADRL,IDX,SIZE,[NAME],XOR)

        }
      */
      break;
    case 0xF3:
      if (rxBufferXN[DATA1] == 0x0A) {                      // Multimaus firmware version                 (0xF3,0x0A,VERH,VERL,XOR)
        highVerMM = rxBufferXN[DATA2];
        lowVerMM = rxBufferXN[DATA3];
      }
      break;
    default:
      if ((rxBufferXN[HEADER] & 0xF0) == 0x40) {            // Feedback broadcast / Accessory decoder information response (0x4X,MOD,DATA,...,XOR)
        for (n = HEADER; n < (rxBytes - 2); n += 2) {
          modulo = rxBufferXN[n + 1];
          dato = rxBufferXN[n + 2];
          if (modulo == miModulo) {                         // Si es mi desvio guarda su posicion
            if (bitRead(dato, 4) == bitRead(miAccPos, 1)) {
              if (bitRead(miAccPos, 0))
                myPosTurnout = (dato >> 2) & 0x03;
              else
                myPosTurnout = dato & 0x03;
              if (scrOLED == SCR_TURNOUT)
                updateOLED = true;
            }
          }
#ifdef USE_AUTOMATION
          for (byte n = 0; n < MAX_AUTO_SEQ; n++) {
            if ((automation[n].opcode & OPC_AUTO_MASK) == OPC_AUTO_FBK) {
              if (modulo == automation[n].param) {
                unsigned int nibble = (dato & 0x10) ? 0x0F : 0xF0;
                automation[n].value &= nibble;
                nibble = (dato & 0x10) ? (dato << 4) : (dato & 0x0F);
                automation[n].value |= nibble;
              }
            }
          }
#endif
          modulo++;
          if (modulo == Shuttle.moduleA)                    // shuttle contacts
            updateShuttleStatus(&Shuttle.statusA, dato);
          if (modulo == Shuttle.moduleB)
            updateShuttleStatus(&Shuttle.statusB, dato);
        }
      }
      break;
  }
}


void  updateShuttleStatus(byte *stat, byte nibble) {
  if (bitRead(nibble, 4)) {
    *stat &= 0x0F;
    *stat |= ((nibble & 0x0F) << 4);
  }
  else {
    *stat &= 0xF0;
    *stat |= (nibble & 0x0F);
  }
}


void getStatus () {
  headerXN (0x21);                                          // Command station status request         (0x21,0x24,0x05)
  dataXN (0x24);
  sendXN();
}


void getVersion () {
  headerXN (0x21);                                          // Command station software version       (0x21,0x21,0x00)
  dataXN (0x21);
  sendXN();
}

void versionMultimaus() {
  headerXN (0xF1);                                          // Multimaus software version             (0xF1,0x0A,XOR)
  dataXN (0x0A);
  sendXN();
}


void getResults () {
  headerXN (0x21);                                          // Request for Service Mode results       (0x21,0x10,0x31)
  dataXN (0x10);
  sendXN();
  //getResultsSM = false;
}


void resumeOperations () {
  headerXN (0x21);                                          // Resume operations request              (0x21,0x81,0xA0)
  dataXN (0x81);
  sendXN();
}


void emergencyOff() {
  headerXN (0x21);                                          // Stop operations request (emergency off)(0x21,0x80,0xA1)
  dataXN (0x80);
  sendXN();
}


void locoOperationSpeed() {                                 // Locomotive speed and direction operations (0xE4,ID,ADRH,ADRL,SPD,XOR)
  headerXN (0xE4);
  if (bitRead(mySteps, 2)) {                                // 128 steps
    dataXN (0x13);
  }
  else {
    if (bitRead(mySteps, 1)) {                              // 28 steps
      dataXN (0x12);
    }
    else {
      dataXN (0x10);                                        // 14 steps
    }
  }
  dataXN (myLoco.adr[1]);
  dataXN (myLoco.adr[0]);
  dataXN (mySpeed | myDir);
  sendXN();
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



void funcOperations (byte fnc) {                            // Function operation instructions        (0xE4,ID,ADRH,ADRL,GRP,XOR)
  byte grp, grpID;

  if (fnc > 20) {
    grpID = 0x28;                                           // F21..F28
    grp =  ((myFunc.xFunc[2] >> 5) & 0x07);
    grp |= (myFunc.xFunc[3] << 3);
  }
  else {
    if (fnc > 12) {
      if (xnetCS == 0x10)
        grpID = 0xF3;                                       // F13..F20 MM                            (0xE4,0xF3,ADH,ADL,F13F20,XOR)
      else
        grpID = 0x23;                                       // F13..F20
      grp =  ((myFunc.xFunc[1] >> 5) & 0x07);
      grp |= (myFunc.xFunc[2] << 3);
    }
    else {
      if (fnc > 8) {
        grpID = 0x22;                                       // F9..F12
        grp =  ((myFunc.xFunc[1] >> 1) & 0x0F);
      }
      else {
        if (fnc > 4) {
          grpID = 0x21;                                     // F5..F8
          grp =  ((myFunc.xFunc[0] >> 5) & 0x07);
          if (bitRead(myFunc.xFunc[1], 0))
            grp |= 0x08;
        }
        else {
          grpID = 0x20;                                     // F0..F4
          grp =  ((myFunc.xFunc[0] >> 1) & 0x0F);
          if (bitRead(myFunc.xFunc[0], 0))
            grp |= 0x10;
        }
      }
    }
  }
  headerXN (0xE4);
  dataXN (grpID);
  dataXN (myLoco.adr[1]);
  dataXN (myLoco.adr[0]);
  dataXN (grp);
  sendXN();
  bitClear(mySteps, 3);                                     // currently operated by me
}


void infoLocomotora (unsigned int loco) {                   // Locomotive information request         (0xE3,0x00,ADRH,ADRL,XOR)
  headerXN (0xE3);
  dataXN (0x00);
  dataXN (highByte(loco));
  dataXN (lowByte (loco));
  sendXN();
  if ((xnetVersion > 0x35) || (xnetCS == 0x10)) {
    headerXN (0xE3);
    if (xnetCS == 0x10)
      dataXN (0xF0);                                       // Locomotive function F13..F20 info MM    (0xE3,0xF0,ADRH,ADRL,XOR)
    else
      dataXN (0x09);                                       // Locomotive function F13..F28 info v3.6  (0xE3,0x09,ADRH,ADRL,XOR)
    dataXN (highByte(loco));
    dataXN (lowByte (loco));
    sendXN();
  }
  getInfoLoco = false;
}


void enviaAccesorio (unsigned int direccion, bool activa, byte posicion) {    // 1..1024
  byte  adr, dato;
  direccion--;                                              // 000000AAAAAAAABB
  adr = (direccion >> 2) & 0x00FF;                          // AAAAAAAA
  dato = ((direccion & 0x0003) << 1) | 0x80;                // 1000xBBx
  if (posicion > 0)
    dato |= 0x01;
  if (activa) {                                             // 1000dBBD
    dato |= 0x08;
  }
  headerXN (0x52);                                          // Accessory Decoder operation request    (0x52,AAAAAAAA,1000dBBD,XOR)
  dataXN (adr);
  dataXN (dato);
  sendXN();
}


void infoAccesorio (byte modulo, bool nibble) {             // Accessory Decoder information request  (0x42,ADR,NIBBLE,XOR)
  headerXN (0x42);
  dataXN (modulo);
  if (nibble)
    dataXN (0x81);
  else
    dataXN (0x80);
  sendXN();
}


void readCV (unsigned int adr, byte stepPrg) {
  if (!modeProg) {                                          // Read only in Direct mode
    if (isRecentMM()) {
      headerXN (0x23);                                      // Multimaus v1.03
      dataXN (0x15);
      adr--;
      dataXN (highByte(adr) & 0x03);
      dataXN (lowByte(adr));
      sendXN();
      lastCV = lowByte(adr) + 1;
    }
    else {
      headerXN (0x22);
      if (xnetVersion > 0x35)
        dataXN (0x18 | (highByte(adr) & 0x03));               // v3.6 & up  CV1..CV1024
      else
        dataXN (0x15);                                        // v3.0       CV1..CV256
      dataXN (lowByte(adr));
      sendXN();
      lastCV = lowByte(adr);
    }
    getResultsSM = true;
    infoTimer = millis();
    progStepCV = stepPrg;
    //DEBUG_MSG("Read CV %d", adr);
  }
}


void writeCV (unsigned int adr, unsigned int data, byte stepPrg) {
  if (modeProg) {
    headerXN (0xE6);                                        // Operations Mode Programming byte mode write request (0xE6,0x30,ADRH,ADRL,0xEC+C,CV,DATA,XOR)
    dataXN (0x30);
    dataXN (myLoco.adr[1]);
    dataXN (myLoco.adr[0]);
    adr--;
    dataXN (0xEC | (highByte(adr) & 0x03));
    dataXN (lowByte(adr));
    dataXN(data);
    sendXN();
  }
  else {
    if (isRecentMM()) {
      headerXN (0x24);                                      // Multimaus v1.03
      dataXN (0x16);
      adr--;
      dataXN (highByte(adr) & 0x03);
      dataXN (lowByte(adr));
      dataXN(data);
      sendXN();
      lastCV = lowByte(adr) + 1;
    }
    else {
      headerXN (0x23);
      if (xnetVersion > 0x35)
        dataXN (0x1C | (highByte(adr) & 0x03));               // v3.6 & up  CV1..CV1024
      else
        dataXN (0x16);                                        // v3.0       CV1..CV256
      dataXN (lowByte(adr));
      dataXN(data);
      sendXN();
      lastCV = lowByte(adr);
    }
    getResultsSM = true;
    infoTimer = millis();
  }
  progStepCV = stepPrg;
  //DEBUG_MSG("Write CV%d = %d", adr, data);
}


#endif
