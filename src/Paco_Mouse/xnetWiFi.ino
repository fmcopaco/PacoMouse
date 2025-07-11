/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/

#ifdef USE_XNWIFI
////////////////////////////////////////////////////////////
// ***** WIFI SOPORTE *****
////////////////////////////////////////////////////////////

void beginXnWiFi() {
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
  while ((WiFi.status() != WL_CONNECTED) && n < 40) {       // tries to connect to router in 20 seconds
    n++;
    delay(500);
    DEBUG_MSG(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_MSG("IP address: %u.%u.%u.%u", WiFi.localIP().operator[](0), WiFi.localIP().operator[](1), WiFi.localIP().operator[](2), WiFi.localIP().operator[](3));
    DEBUG_MSG("Channel: %d", WiFi.channel());
    if (!XnWiFi.connect(wifiSetting.CS_IP, XnetPort)) {
      DEBUG_MSG("Connection to Xpressnet failed");
      enterCfgWiFi(CFG_WIFI_IP);
    }
    else {
      rxIndice = FRAME1;
      XnWiFi.setNoDelay(true);
      getVersion();                                         // pide la version del Xpressnet
      getStatus();                                          // pide estado de la central
    }
  }
}


void headerXN (byte header) {
  txBytes = HEADER;                                         // coloca header en el buffer
  txXOR = header;
  txBuffer[txBytes++] = header;
  txBuffer[FRAME1] = 0xFF;
  txBuffer[FRAME2] = 0xFE;
}


void dataXN (byte dato) {
  txBuffer[txBytes++] = dato;                               // coloca dato en el buffer
  txXOR ^= dato;
}


void sendXN () {
  bool recvAnswer;
  txBuffer[txBytes++] = txXOR;                              // coloca XOR byte en el buffer
#ifdef DEBUG
  Serial.print(F("TX: "));
  for (uint8_t x = 0; x < txBytes; x++) {
    uint8_t val = txBuffer[x];
    if (val < 16)
      Serial.print('0');
    Serial.print(val, HEX);
    Serial.print(' ');
  }
  Serial.println();
#endif
  XnWiFi.write((byte *)&txBuffer[FRAME1], txBytes);         // envia paquete xpressnet
  timeout = millis();
  recvAnswer = false;
  while ((millis() - timeout < 500) && (!recvAnswer))       // wait answer for 500ms
    recvAnswer = xnetReceive();
}


bool xnetReceive() {
  bool getAnswer;
  getAnswer = false;
  while (XnWiFi.available()) {
    rxData = XnWiFi.read();
    //DEBUG_MSG("%d-%02X", rxIndice, rxData);
    switch (rxIndice) {
      case FRAME1:
        rxBufferXN[FRAME1] = rxData;
        if (rxData == 0xFF)                                 // 0xFF... Posible inicio de paquete
          rxIndice = FRAME2;
        break;
      case FRAME2:
        rxBufferXN[FRAME2] = rxData;
        switch (rxData) {
          case 0xFF:                                        // 0xFF 0xFF... FRAME2 puede ser FRAME1 (inicio de otro paquete)
            break;
          case 0xFE:                                        // 0xFF 0xFE... Inicio paquete correcto
          case 0xFD:                                        // 0xFF 0xFD... Inicio paquete de broadcast correcto
            rxIndice = HEADER;
            rxXOR = 0;
            break;
          default:                                          // 0xFF 0xXX... No es inicio de paquete
            rxIndice = FRAME1;
            break;
        }
        break;
      default:
        rxBufferXN[rxIndice++] = rxData;
        rxXOR ^= rxData;
        if (((rxBufferXN[HEADER] & 0x0F) + 4) == rxIndice) {  // si se han recibido todos los datos indicados en el paquete
          if (rxXOR == 0) {                                   // si el paquete es correcto
            rxBytes = rxIndice;
#ifdef DEBUG
            Serial.print(F("RX: "));
            for (uint8_t x = 0; x < rxBytes; x++) {
              uint8_t val = rxBufferXN[x];
              if (val < 16)
                Serial.print('0');
              Serial.print(val, HEX);
              Serial.print(' ');
            }
            Serial.println();
#endif
            procesaXN();                                      // nuevo paquete recibido, procesarlo
            getAnswer = true;
          }
          rxIndice = FRAME1;                                  // proximo paquete
        }
        break;
    }
  }
  return getAnswer;
}

void xpressnetProcess () {                                  // procesa Xpressnet
  xnetReceive();
  if (getInfoLoco && (csStatus == csNormalOps))
    infoLocomotora(myLoco.address);
  if (millis() - infoTimer > 1000UL) {                      // Cada segundo
    infoTimer = millis();
    if (getResultsSM)                                       // Resultados de CV pendientes
      getResults();                                         // pide resultados
    else {
      if (bitRead(mySteps, 3))                              // Loco controlada por otro mando
        getInfoLoco = true;                                 // pide info locomotora
      if (askMultimaus) {                                   // pide info Multimaus
        askMultimaus = false;
        versionMultimaus();
      }
      if ((scrOLED == SCR_TURNOUT) && (xnetCS > 0x10))      // Z21 info desvios
        infoDesvio (myTurnout);
    }
  }
  if (progFinished) {                                       // fin de lectura/programacion CV
    progFinished = false;
    endProg();
  }
  if (millis() - pingTimer > PING_INTERVAL) {               // Refresca para mantener la conexion
    pingTimer = millis();
    getStatus();                                            // pide estado de la central
  }
}


#endif
