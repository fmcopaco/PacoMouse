/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/

#ifdef USE_LNWIFI

void beginLnWiFi() {
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
  while ((WiFi.status() != WL_CONNECTED) && n < 40) {                     // tries to connect to router in 20 seconds
    n++;
    delay(500);
    DEBUG_MSG(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_MSG("IP address: %u.%u.%u.%u", WiFi.localIP().operator[](0), WiFi.localIP().operator[](1), WiFi.localIP().operator[](2), WiFi.localIP().operator[](3));
    DEBUG_MSG("Channel: %d", WiFi.channel());
    if (!LnWiFi.connect(wifiSetting.CS_IP, wifiSetting.port)) {
      DEBUG_MSG("Connection to Loconet over TCP/IP failed");
      enterCfgWiFi(CFG_WIFI_IP);
    }
    else {
      LnWiFi.setNoDelay(true);
    }
  }
}


void lnetSend (lnMsg * Msg) {
  byte n, pos, chk, nibble;
  byte msgLng;
  char msgStr[120];

  chk = 0xFF;
  msgLng = getLnMsgSize(Msg);
  //msgLng = ((Msg->sz.command & 0x60) == 0x60) ? Msg->sz.mesg_size : ((Msg->sz.command & 0x60) >> 4) + 2;
  if (isLBServer) {
    strcpy(msgStr, "SEND ");                                        // Loconet over TCP/IP LBServer
    pos = 5;
    for (n = 0; n < msgLng - 1; n++) {
      chk ^= Msg->data[n];
      nibble = Msg->data[n] >> 4;
      msgStr[pos++] = (nibble > 9) ? nibble + 0x37 : nibble + 0x30;
      nibble = Msg->data[n] & 0x0F;
      msgStr[pos++] = (nibble > 9) ? nibble + 0x37 : nibble + 0x30;
      msgStr[pos++] = ' ';
    }
    nibble = chk >> 4;
    msgStr[pos++] = (nibble > 9) ? nibble + 0x37 : nibble + 0x30;
    nibble = chk & 0x0F;
    msgStr[pos++] = (nibble > 9) ? nibble + 0x37 : nibble + 0x30;
    msgStr[pos++] = '\r';
    msgStr[pos++] = '\n';
    msgStr[pos++] = '\0';
    LnWiFi.write(msgStr);
    DEBUG_MSG(msgStr);
  }
  else {
    for (n = 0; n < msgLng - 1; n++)                                    // Loconet over TCP/IP Binary
      chk ^= Msg->data[n];
    Msg->data[n] = chk;
    LnWiFi.write((byte *)&Msg->data[0], msgLng);
    /*
      pos = 0;                                                          // Loconet over TCP/IP Binary
      for (n = 0; n < msgLng - 1; n++) {
      chk ^= Msg->data[n];
      //msgStr[pos++] = Msg->data[n];
      LnWiFi.write(Msg->data[n]);
      DEBUG_MSG("%02X", Msg->data[n])
      }
      //msgStr[pos] = chk;
      LnWiFi.write(chk);
      DEBUG_MSG("%02X", chk)
    */
  }
}


void lnetReceive() {
  char rxByte;
  byte lng;
  while (LnWiFi.available()) {
    rxByte = LnWiFi.read();
    if (isLBServer) {                                               // Loconet over TCP/IP LBServer
#ifdef DEBUG
      Serial.print(rxByte);
#endif
      switch (rcvStrPhase) {
        case WAIT_TOKEN:                                            // wait for RECEIVE token
          if (rxByte == 'R') {                                      // Possible match: RECEIVE. veRsion, bREak, eRRoR Checksum, eRRoR line, eRRoR message / No match: send, sent,timestamp
            rcvStrPos = 0;
            rcvStr[rcvStrPos++] = rxByte;
            rcvStrPhase = RECV_TOKEN;
          }
          break;
        case RECV_TOKEN:
          switch (rxByte) {
            case 'E':                                 // RECEIVE valid characters
            case 'C':
            case 'I':
            case 'V':
            case ' ':
              rcvStr[rcvStrPos++] = rxByte;
              if (rcvStrPos == 8) {
                if (! strncmp(rcvStr, "RECEIVE ", 8)) {
                  rcvStrPhase = RECV_PARAM;
                  rcvStrPos = 0;
                  RecvPacket.data[rcvStrPos] = 0;
                  //DEBUG_MSG(" - RECEIVE token detected!")
                }
                else
                  rcvStrPhase = WAIT_TOKEN;
              }
              break;
            default:                                 // RECEIVE invalid characters
              rcvStrPhase = WAIT_TOKEN;
              break;
          }
          break;
        case RECV_PARAM:
          switch (rxByte) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
              RecvPacket.data[rcvStrPos] = (RecvPacket.data[rcvStrPos] << 4) + (rxByte & 0x0F);
              break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
              RecvPacket.data[rcvStrPos] = (RecvPacket.data[rcvStrPos] << 4) + (rxByte & 0x0F) + 9;
              break;
            case '\n':
              rcvStrPhase = WAIT_TOKEN;
              //DEBUG_MSG("Message received!")
              lnetDecode(&RecvPacket);
#ifdef DEBUG
              rcvStrPos = ((RecvPacket.sz.command & 0x60) == 0x60) ? RecvPacket.sz.mesg_size : ((RecvPacket.sz.command & 0x60) >> 4) + 2;             // imprime paquete
              DEBUG_MSG("LN Lng: % d", rcvStrPos)
              Serial.print(F("RX: "));
              for (uint8_t x = 0; x < rcvStrPos; x++) {
                uint8_t val = RecvPacket.data[x];
                if (val < 16)
                  Serial.print('0');
                Serial.print(val, HEX);
                Serial.print(' ');
              }
              Serial.println();
#endif
              break;
            case ' ':
              rcvStrPos++;
              if (rcvStrPos > 31)
                rcvStrPhase = WAIT_TOKEN;       // message too long, discard
              else
                RecvPacket.data[rcvStrPos] = 0;
              break;
          }
          break;
      }
    }
    else {                                                        // Loconet over TCP/IP Binary
      if (rxByte & 0x80) {
        rcvStrPos = 1;
        RecvPacket.sz.command = rxByte;
      }
      else {
        RecvPacket.data[rcvStrPos++] = rxByte;
        lng = ((RecvPacket.sz.command & 0x60) == 0x60) ? RecvPacket.sz.mesg_size : ((RecvPacket.sz.command & 0x60) >> 4) + 2;
        if (lng > sizeof(RecvPacket))                              // discard message too long
          rcvStrPos = 2;
        if (rcvStrPos == lng) {
          //DEBUG_MSG("Message received!")
          lnetDecode(&RecvPacket);
#ifdef DEBUG
          //DEBUG_MSG("LN Lng: % d Pkt: % d", rcvStrPos, lng)
          Serial.print(F("RX: "));
          for (uint8_t x = 0; x < lng; x++) {
            uint8_t val = RecvPacket.data[x];
            if (val < 16)
              Serial.print('0');
            Serial.print(val, HEX);
            Serial.print(' ');
          }
          Serial.println();
#endif
        }
      }
    }
  }
}


void lnetProcess() {
  lnetReceive();
  lnetTimers();
}

#endif
