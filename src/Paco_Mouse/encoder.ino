/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/

////////////////////////////////////////////////////////////
// ***** LECTURA ENCODER *****
////////////////////////////////////////////////////////////

#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
ISR (PCINT2_vect)                                           // gestionar PCINT para D0 a D7   // interrupción encoder
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
//ICACHE_RAM_ATTR void encoderISR ()                          // ESP core version 2.x.x       // interrupción encoder
IRAM_ATTR void encoderISR ()                                // ESP core version 3.x.x
#endif
{
  cli();
  outA = digitalRead (pinOutA);
  outB = digitalRead (pinOutB);
  if (outA != copyOutA) {                                   // evitamos rebotes
    copyOutA = outA;
#ifdef SENSITIVE
    if ( outB != copyOutB) {
      copyOutB = outB;
      if (outA == outB)                                     // comprueba sentido de giro
        encoderValue = (encoderValue < encoderMax) ? ++encoderValue : encoderMax ;  // CW, hasta maximo
      else
        encoderValue = (encoderValue > 0) ? --encoderValue : 0;                     // CCW, hasta 0
      encoderChange = true;
    }
#else
    if (copyOutB == 0x80) {
      copyOutB = outB;
    }
    else {
      if ( outB != copyOutB) {
        copyOutB = 0x80;
        if (outA == outB)                                   // comprueba sentido de giro
          encoderValue = (encoderValue < encoderMax) ? ++encoderValue : encoderMax ;  // CW, hasta maximo
        else
          encoderValue = (encoderValue > 0) ? --encoderValue : 0;                     // CCW, hasta 0
        encoderChange = true;
      }
    }
#endif
  }
  sei();
}


void readButtons () {
  byte inputButton;

  timeButtons = millis();                                   // lee cada cierto tiempo
  inputButton = digitalRead (pinSwitch);                    // comprueba cambio en boton del encoder
  if (statusSwitch != inputButton) {
    statusSwitch = inputButton;
    if (statusSwitch == LOW)
      switchOn = true;
  }
}


////////////////////////////////////////////////////////////
// ***** CONTROL ENCODER *****
////////////////////////////////////////////////////////////

void controlEncoder() {                                     // encoder movement
  encoderChange = false;
  switch (scrOLED)  {
    case SCR_LOGO:
      showMenu();
      updateOLED = true;
      break;
    case SCR_ERROR:
      break;
    case SCR_MENU:                                          // display desired option
      if (encoderValue >= MENU_ITEMS) {
        encoderMax = MENU_ITEMS - 1;
        encoderValue = 0;
      }
      optOLED = encoderValue;
      updateOLED = true;
      break;
    case SCR_SELCV:                                          // display desired programming option
      if (encoderValue >= PRG_ITEMS) {
        encoderMax = PRG_ITEMS - 1;
        encoderValue = 0;
      }
      prgOLED = encoderValue;
      updateOLED = true;
      break;
#ifdef USE_PHONE
    case SCR_PHONE:                                         // Control loco en llamada telefonica
#endif
    case SCR_SHUTTLE:                                       // Control loco en menu lanzadera
    case SCR_TURNTABLE:                                     // Control loco en menu plataforma giratoria
    case SCR_TURNOUT:                                       // Control loco en menu desvios
    case SCR_SPEED:                                         // change speed
      updateMySpeed();
      break;
    case SCR_LOCO:
      if ((encoderValue > 0) && (locoStack[encoderValue] == 0))
        encoderValue--;
      showStackLoco();
      break;
#ifdef USE_LOCONET
#ifdef USE_PHONE
    case SCR_PHONE_NUM:
    case SCR_PHONEBOOK:
    case SCR_STATION:
#endif
    case SCR_DISPATCH:
      updateOLED = true;
      break;
#endif
    case SCR_CFG:
      if (changeCfgOption) {
        switch (cfgOLED) {
          case CFG_CONTR_VAL:
            oled.setContrast(encoderValue);
            cfgMenu[CFG_CONTR_VAL].value = encoderValue;
            updateOLED = true;
            break;
          case CFG_STOP_VEL0:
          case CFG_STOP_EMERG:
          case CFG_SHUNTING:
            cfgOLED = CFG_STOP_VEL0 + encoderValue;
            updateOLED = true;
            break;
#ifdef USE_LOCONET
          case CFG_CMD_DR:
          case CFG_CMD_ULI:
          case CFG_CMD_DIG:
            cfgOLED = CFG_CMD_DR + encoderValue;
            updateOLED = true;
            break;
#endif
#ifdef USE_XNWIRE
          case CFG_XBUS_DIR:
            cfgMenu[CFG_XBUS_DIR].value = encoderValue;
            updateOLED = true;
            break;
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
          case CFG_WIFI_SSID:
          case CFG_WIFI_PWD:
          case CFG_WIFI_IP:
            cfgOLED = CFG_WIFI_SSID + encoderValue;
            updateOLED = true;
            break;
#endif
#ifdef USE_Z21
          case CFG_SHORT_99:
          case CFG_SHORT_127:
            cfgOLED = CFG_SHORT_99 + encoderValue;
            updateOLED = true;
            break;
#endif
#if defined(USE_LNWIFI)
          case CFG_WIFI_PORT:
          case CFG_LBSERVER:
          case CFG_BINARY:
            cfgOLED = CFG_WIFI_PORT + encoderValue;
            updateOLED = true;
            break;
#endif
          case CFG_TT_KB14:
          case CFG_TT_KB15:
          case CFG_TT_TCTRL:
#if defined(USE_Z21) || defined(USE_XPRESSNET)
          case CFG_TT_ROCO:
#endif
            cfgOLED = CFG_TT_KB14 + encoderValue;
            updateOLED = true;
            break;
#ifdef  USE_SET_TIME
          case CFG_SET_TIME:
#ifdef USE_LOCONET
          case CFG_SYNC:
#endif
            cfgOLED = CFG_SET_TIME + encoderValue;
            updateOLED = true;
            break;
#endif
          case CFG_LOCK_LOCO:
          case CFG_LOCK_TURNOUT:
          case CFG_LOCK_PROG:
            cfgOLED = CFG_LOCK_LOCO + encoderValue;
            updateOLED = true;
            break;
#ifdef GAME_EXTRA
          case CFG_GAME_TIC:
          case CFG_GAME_SNAKE:
          case CFG_GAME_FLAPPY:
          case CFG_GAME_PAKU:
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
          case CFG_GAME_PONG:
#endif
            cfgOLED = CFG_GAME_TIC + encoderValue;
            updateOLED = true;
            break;
#endif
#ifdef  USE_PHONE
          case CFG_PHONE_ME:
          case CFG_PHONE_STAT:
            cfgOLED = CFG_PHONE_ME + encoderValue;
            updateOLED = true;
            break;
#endif
        }
      }
      else {
        if (encoderValue == 0)                              // Evita seleccionar titulo
          encoderValue = 1;
        if (cfgOLED != encoderValue) {
          if (encoderValue >= MAX_CFG_ITEMS) {
            encoderMax = MAX_CFG_ITEMS - 1;
            encoderValue = 1;
          }
          cfgOLED = encoderValue;
          updateOLED = true;
        }
      }
      break;
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
    case SCR_SSID:
      scrSSID = encoderValue;
      updateOLED = true;
      //DEBUG_MSG("Encoder: %d", encoderValue);
      break;
    case SCR_PASSWORD:
      updateOLED = true;
      break;
    case SCR_IP_ADDR:
      scrIP[scrPosIP] = encoderValue;
      updateOLED = true;
      break;
#endif
#ifdef GAME_EXTRA
    case SCR_GAME:
      updateOLED = true;
      break;
#endif
#ifdef USE_AUTOMATION
    case SCR_AUTO_SELECT:
    case SCR_AUTO_NAME:
      updateOLED = true;
      break;
    case SCR_AUTO_EDIT:
#ifdef USE_ECOS
      if (editingOpcode) {
        if (editingLoco) {
          if ((encoderValue > 0) && (locoStack[encoderValue] == 0))
            encoderValue--;
          bufferEdit[editPos] = highByte(locoStack[encoderValue]) | OPC_AUTO_LOCO_SEL;    // ID
          bufferEdit[editPos + 1] = lowByte(locoStack[encoderValue]);
        }
        else {
          if (editingJump) {
            bufferEdit[editPos + 1] = getFileStart(encoderValue);
          }
          else {
            bufferEdit[editPos] = highByte(defaultOpcode[encoderValue]);
            bufferEdit[editPos + 1] = lowByte(defaultOpcode[encoderValue]);
          }
        }
      }
#else
      if (editingOpcode) {
        if (editingJump) {
          bufferEdit[editPos + 1] = getFileStart(encoderValue);
        }
        else {
          bufferEdit[editPos] = highByte(defaultOpcode[encoderValue]);
          bufferEdit[editPos + 1] = lowByte(defaultOpcode[encoderValue]);
        }
      }
#endif
      else {
        setPosEdit(encoderValue);
      }
      updateOLED = true;
      break;
#endif
#ifdef USE_SET_TIME
    case SCR_TIME:
      switch (scrPosTime) {
        case 0:
          scrHour = encoderValue;
          break;
        case 2:
          scrMin = encoderValue;
          break;
        case 5:
          scrRate = encoderValue;
          break;
      }
      updateOLED = true;
      break;
#endif
  }
}


void controlSwitch() {                                      // encoder switch
  switchOn = false;
  switch (scrOLED)  {
    case SCR_LOGO:
      showMenu();
      updateOLED = true;
      break;
    case SCR_ERROR:
      resumeOperations();                                   //  Track Power On
      break;
#ifdef USE_PHONE
    case SCR_PHONE:
#endif
    case SCR_SHUTTLE:                                       // Control loco en menu lanzadera
    case SCR_TURNTABLE:                                     // Control loco en menu plataforma giratoria
    case SCR_TURNOUT:                                       // Control loco en menu desvios
    case SCR_SPEED:                                         // Control speed
      if (encoderValue > 0) {
        encoderValue = 0;
        if (stopMode > 0)
          mySpeed = 1;
        else
          mySpeed = 0;
        locoOperationSpeed();
      }
      else {
        myDir ^= 0x80;
#ifdef USE_LOCONET
        changeDirectionF0F4();
#endif
#ifdef USE_XPRESSNET
        locoOperationSpeed();
#endif
#ifdef USE_Z21
        locoOperationSpeed();
#endif
#ifdef USE_ECOS
        changeDirection();
#endif
      }
      updateOLED = true;
      break;
    case SCR_MENU:
#if USE_LOCONET
      if ((mySlot.num == 0) && (optOLED == OPT_SPEED))
        optOLED = OPT_LOCO;
#endif
      enterMenuOption();
      break;
    case SCR_SELCV:
      enterPrgOption();
      break;
    case SCR_LOCO:
      scrLocoGet();
      break;
#ifdef USE_LOCONET
    case SCR_DISPATCH:
      selectDispatch();
      break;
#endif
    case SCR_CFG:
      saveCfgOption();
      break;
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
    case SCR_SSID:
      changeCfgOption = false;
      saveSSID(scrSSID);
      enterCfgWiFi(CFG_WIFI_PWD);
      break;
    case SCR_PASSWORD:
      if (encoderValue > 0) {
        if (scrPwdLng < 64) {
          scrPwd[scrPwdLng++] = encoderValue + 32;
          scrPwd[scrPwdLng] = 0;
        }
      }
      else {
        if (scrPwdLng > 0) {
          scrPwd[--scrPwdLng] = 0;
        }
      }
      updateOLED = true;
      //DEBUG_MSG("Enc: %d %s", scrPwdLng, scrPwd);
      break;
    case SCR_IP_ADDR:
      scrPosIP++;
      if (scrPosIP > 3)
        scrPosIP = 0;
      encoderValue = scrIP[scrPosIP];
      updateOLED = true;
      break;
#endif
#ifdef GAME_EXTRA
    case SCR_GAME:
      updateOLED = true;
      updateAllOLED = true;
      break;
#endif
#ifdef USE_PHONE
    case SCR_STATION:
      callToStation();
      break;
    case SCR_PHONEBOOK:
      enterPhoneNumber();
      break;
    case SCR_PHONE_NUM:
      savePhoneNumber();
      break;
#endif
#ifdef USE_AUTOMATION
    case SCR_AUTO_NAME:
      if (encoderValue > 0) {
        if (scrNameLng < 13) {
          scrName[scrNameLng++] = encoderValue + 32;
          scrName[scrNameLng] = 0;
        }
      }
      else {
        if (scrNameLng > 0) {
          scrName[--scrNameLng] = 0;
        }
      }
      updateOLED = true;
      break;
    case SCR_AUTO_EDIT:
      if (editingOpcode) {
        if (fileCount > 0) {
          if ((bufferEdit[editPos] == OPC_AUTO_JUMP) && (bufferEdit[editPos + 1] != OPC_AUTO_LOOP)) {
            editingJump = true;
            encoderValue = 0;
            encoderMax = fileCount - 1;
            DEBUG_MSG("Editing Jump")
          }
#ifdef USE_ECOS
          byte opc = bufferEdit[editPos] & OPC_AUTO_MASK;
          if ((opc == OPC_AUTO_LOCO_SEL) || (opc == OPC_AUTO_LOCO_SEL4) || (opc == OPC_AUTO_LOCO_SEL8)) {
            editingLoco = true;
            encoderValue = 0;
            encoderMax = LOCOS_IN_STACK - 1;
            DEBUG_MSG("Editing Loco")
          }
#endif
        }
      }
      break;
#endif
#ifdef USE_SET_TIME
    case SCR_TIME:
      switch (scrPosTime) {
        case 0:
          scrPosTime = 2;
          encoderValue = scrMin;
          encoderMax = 59;
          break;
        case 2:
          scrPosTime = 5;
          encoderValue = scrRate;
#ifdef USE_Z21
          encoderMax = 63;
#endif
#ifdef USE_XPRESSNET
          encoderMax = 31;
#endif
#ifdef USE_LOCONET
          encoderMax = 127;
#endif
          break;
        case 5:
          scrPosTime = 0;
          encoderValue = scrHour;
          encoderMax = 23;
          break;
      }
      updateOLED = true;
      break;
#endif
  }
}
