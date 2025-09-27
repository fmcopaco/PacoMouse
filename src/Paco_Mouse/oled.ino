/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/


////////////////////////////////////////////////////////////
// ***** OLED *****
////////////////////////////////////////////////////////////

void beginHID() {
  pinMode (pinSCL, INPUT_PULLUP);                           // HID (Human Interface Device)
  pinMode (pinSDA, INPUT_PULLUP);
  pinMode (pinOutA, INPUT_PULLUP);                          // Encoder
  pinMode (pinOutB, INPUT_PULLUP);
  pinMode (pinSwitch, INPUT_PULLUP);
#if defined(__AVR__) && defined(USE_TTP229)
#ifdef POWER_TOUCH
  pinMode(pinTOUCH_GND, OUTPUT);
  digitalWrite(pinTOUCH_GND, LOW);
  pinMode(pinTOUCH_VCC, OUTPUT);
  digitalWrite(pinTOUCH_VCC, HIGH);
#endif
#endif
  oled.begin(&OLED_TYPE_MAIN, I2C_ADDRESS_OLED);            // OLED
#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
  oled.setI2cClock(400000UL);
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  Wire.setClock(400000UL);
#endif
  oled.setContrast(cfgMenu[CFG_CONTR_VAL].value);
  updateOLED = true;
  copyOutA = digitalRead (pinOutA);                         // init encoder ISR
  //copyOutB = digitalRead (pinOutB);
  copyOutB = 0x80;
#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
  *digitalPinToPCMSK(pinOutA) |= bit (digitalPinToPCMSKbit(pinOutA));   // activar pin en PCMSK para D0 a D7
  PCIFR  |= bit (digitalPinToPCICRbit(pinOutA));                        // limpiar flag de la interrupcion en PCIFR
  PCICR  |= bit (digitalPinToPCICRbit(pinOutA));                        // activar interrupcion para el grupo en PCICR
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  attachInterrupt (digitalPinToInterrupt (pinOutA), encoderISR, CHANGE);
  keypad.begin();
#endif
}



void showMenu() {
  scrOLED = SCR_MENU;                                       // muestra el menu
  encoderMax = MENU_ITEMS - 1;
  encoderValue = optOLED;
}


void showPrgMenu() {                                        // muestra menu de programacion
  scrOLED = SCR_SELCV;
  encoderMax = PRG_ITEMS - 1;
  encoderValue = prgOLED;
  updateOLED = true;
#ifdef USE_LOCONET
  lnetProg = false;
  if (!bitRead(mySlot.trk, 0))
    showEmergencyOff();
#endif
#ifdef USE_XPRESSNET
  if (csStatus & csServiceMode)
    resumeOperations();
#endif
#ifdef USE_Z21
  if (csStatus & csProgrammingModeActive)
    resumeOperations();
#endif
#ifdef USE_ECOS
  exitProgramming();
#endif
}


void showWait() {                                           // muestra pantalla de espera
  oled.clear();
  scrOLED = SCR_WAIT;
  updateOLED = true;
}


void showStackLoco() {                                      // muestra numero de loco del stack
#ifdef USE_ECOS
  scrLoco = locoStack[encoderValue];                        // ID
#else
  scrLoco = locoStack[encoderValue] & 0x3FFF;               // address
#endif
  clearInputDigit();
  updateExtraOLED = true;
  updateOLED = true;
}


void showOLED() {
  byte line, text, spd;                                     // muestra la pantalla principal actual
  unsigned int adr;
  char name[20];

  updateOLED = false;
  timeOLED = millis();
  switch (scrOLED) {
    case SCR_LOGO:
#ifdef USE_SSD1309
      oled.setInvertMode(false);
#else
      oled.setInvertMode(true);                             // Hello world!
#endif
      oled.setFont(Arial_bold_14);
      for (line = 0; line < 8; line += 2) {
        oled.setCursor (0, line);
        for (text = 0; text < 17; text++) {
          oled.write(' ');
        }
      }
      oled.setCursor (8, 0);
      oled.set2X();
      oled.print(F("Paco"));
      oled.set1X();
      oled.setCursor (16, 3);
      oled.print(F("Mouse"));
      oled.setCursor (5, 6);
      oled.print(VER_STR);
#ifdef USE_SSD1309
      oled.setInvertMode(true);
#endif
      oled.setCursor (83, 0);
      oled.setFont(Sym_44x64);
      oled.print('+');
      oled.setInvertMode(false);
      scrOLED = SCR_ENDLOGO;
      timeoutOLED = 3000;
      updateExtraOLED = true;
      updateAllOLED = true;
      break;
    case SCR_ENDLOGO:                                       // Go to first screen
#ifdef USE_LOCONET
      if (mySlot.num > 0)
        optOLED = OPT_SPEED;
      else
        optOLED = OPT_LOCO;
      enterMenuOption();
      if (!bitRead(mySlot.trk, 0)) {
        scrOLED = SCR_ERROR;
        errOLED = ERR_OFF;
      }
#ifdef USE_LNWIFI
      if (WiFi.status() != WL_CONNECTED) {
        enterCfgWiFi(CFG_WIFI_SSID);
      }
#endif
#endif
#ifdef USE_XPRESSNET
      if (csStatus == csNormalOps) {
        optOLED = OPT_SPEED;
        enterMenuOption();
      }
      else {
        showErrorXnet();
      }
#ifdef USE_XNWIFI
      if (WiFi.status() != WL_CONNECTED) {
        enterCfgWiFi(CFG_WIFI_SSID);
      }
#endif
#endif
#ifdef USE_Z21
      if (WiFi.status() != WL_CONNECTED) {
        enterCfgWiFi(CFG_WIFI_SSID);
      }
      else {
        if (csStatus == csNormalOps) {
          optOLED = OPT_SPEED;
          enterMenuOption();
        }
        else {
          showErrorXnet();
        }
      }
#endif
#ifdef USE_ECOS
      if (WiFi.status() != WL_CONNECTED) {
        enterCfgWiFi(CFG_WIFI_SSID);
      }
      else {
        if (csStatus == 1) {
          adr = LOCOS_IN_STACK;
          for (line = 0; line < LOCOS_IN_STACK; line++)                    // search ID in roster list
            if (rosterList[line].id == myLocoID)
              adr = line;
          if (adr != LOCOS_IN_STACK) {
            optOLED = OPT_SPEED;
            myLoco.address = rosterList[adr].locoAddress;
            pushLoco(myLocoID);
            infoLocomotora(myLocoID);
          }
          else
            optOLED = OPT_LOCO;
          enterMenuOption();
        }
        else {
          scrOLED = SCR_ERROR;
          errOLED = ERR_OFF;
          updateOLED = true;
        }
      }
#endif
      break;
    case SCR_MENU:
      oled.clear();                                         // Opciones del menu
      oled.setFont(Arial_bold_14);
      text = (optOLED > 3) ? optOLED - 3 : 0;
      for (line = 0; line < 4; line++) {
        oled.setCursor (20, line * 2);
        printMenuTxt(text + line);
      }
      line = (optOLED > 3) ? 6 : optOLED * 2;
      oled.setCursor (4, line);
      printCursor();
      break;
    case SCR_SELCV:
      oled.clear();                                         // Opciones de programacion
      oled.setFont(Arial_bold_14);
      text = (prgOLED > 3) ? prgOLED - 3 : 0;
      for (line = 0; line < 4; line++) {
        oled.setCursor (20, line * 2);
        printPrgTxt(text + line);
      }
      line = (prgOLED > 3) ? 6 : prgOLED * 2;
      oled.setCursor (4, line);
      printCursor();
      break;
    case SCR_ERROR:                                         // Errores
      oled.clear();
      oled.setFont(Arial_bold_14);
      oled.setCursor (20, 2);
      switch (errOLED) {
        case ERR_OFF:
          oled.print(F("Emergency"));
          oled.setCursor (45, 5);
          oled.print(F("OFF"));
          break;
#if defined(USE_Z21) || defined(USE_XPRESSNET)
        case ERR_STOP:
          oled.print(F("Emergency"));
          oled.setCursor (45, 5);
          oled.print(F("Stop"));
          break;
        case ERR_SERV:
          oled.print(F("Service"));
          oled.setCursor (45, 5);
          oled.print(F("Mode"));
          break;
#endif
        default:
          oled.print(F("Err#"));
          oled.print(errOLED);
          break;
      }
      timeoutOLED = ONE_DAY;
      break;
    case SCR_SPEED:
      if (updateAllOLED) {                                  // Control locomotora
        oled.clear();
        adr = myLoco.address & 0x3FFF;                      // Numero de locomotora
        if ((clockRate > 0) && showBigClock) {
          oled.setFont(Arial_bold_14);
        }
        else {
#ifdef USE_ECOS
          line = findLocoID(myLocoID);
          oled.setFont(Arial_bold_14);                    // nombre de la locomotora
          if (line != LOCOS_IN_STACK) {
            for (text = 0; text < NAME_LNG; text++)
              oled.print(rosterList[line].locoName[text]);
          }
          oled.setCursor(40, 2);
          if (adr < 1000)
            oled.setCol(46);
          if (adr < 100)
            oled.setCol(52);
          if (adr < 10)
            oled.setCol(58);
          oled.setFont(Sym_Bars);
          oled.print(F("<,"));
          oled.setFont(Arial_bold_14);
#else
          oled.setFont(Num_15x31);
          oled.setCol(40);
          if (adr < 1000)
            oled.setCol(48);
          if (adr < 100)
            oled.setCol(56);
          if (adr < 10)
            oled.setCol(64);
#endif
        }
        oled.print(adr);
        oled.setCursor (38, 4);
        oled.ssd1306WriteRam(0xFF);
        oled.setCol (104);
        oled.ssd1306WriteRam(0xFF);
        scrSpeed = 0;
        oled.setFont(Arial_bold_14);
#if  (FUNC_SIZE == BIG)
        oled.setCursor(0, 6);
        if (scrFunc > FNC_0_4) {                            // funciones
          oled.print('F');
          oled.print((scrFunc << 2) + 1);
        }
        else {
          oled.print(F("F0"));
        }
        oled.print(F("-F"));
        oled.print((scrFunc << 2) + 4);
#endif
#if  (FUNC_SIZE == SMALL)
        oled.setCursor(4, 6);
        oled.print('F');
        if (scrFunc > 0)
          oled.print(scrFunc);
#endif
        updateAllOLED = false;
      }
      if (updateExtraOLED) {                                // hora
        if (clockRate > 0) {
          if (showBigClock) {
            oled.setFont(Num_15x31);
            oled.setCursor(32, 0);
          }
          else {
            oled.setFont(Arial_bold_14);
            oled.setCursor(0, 0);
          }
          if (clockHour < 10)
            oled.print('0');
          oled.print(clockHour);
          if (clockMin < 10)
            oled.print(F(":0"));
          else
            oled.print(':');
          oled.print(clockMin);
          if (!showBigClock)
            oled.print(' ');
        }
        updateExtraOLED = false;
      }
      spd = calcBarSpeed(encoderValue);
      printBarSpeed(scrSpeed, spd);                         // barra velocidad
      oled.setFont(Sym_Bars);
#if (SHOW_STEPS == LEFT)
      oled.setCursor(16, 4);
      printCurrentStep();
#endif
#if (SHOW_STEPS == RIGHT)
      oled.setCursor(108, 4);
      printCurrentStep();
#endif
      oled.setCursor (0, 3);
      if (activeShuttle)
        oled.print('B');                                    // shuttle active
      else
        oled.print('/');
#if defined(USE_Z21) || defined(USE_XPRESSNET) || USE_ECOS
      oled.setCursor (0, 3);
      if (bitRead(mySteps, 3))                              // control by other
        oled.print('=');
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
      oled.setCol (16);                                     // low battery
      if (lowBATT) {
        oled.print('>');
      }
      else {
        if (WiFi.status() != WL_CONNECTED)                  // no wifi connection
          oled.print('?');
      }
#endif
      oled.setFont(Num_15x31);                              // direccion
      oled.setCursor (112, 0);
#if (CHANGE_DIR == SWITCH_3P)
      if (dirValue == 1)
        oled.print (';');                                   // stopped
      else
#endif
      {
#ifdef USE_LOCONET
        if (myDir & 0x80)
          oled.print ('<');                                 // forward
        else
          oled.print ('>');                                 // reverse
#endif
#ifdef USE_XPRESSNET
        if (myDir & 0x80)
          oled.print ('>');                                 // forward
        else
          oled.print ('<');                                 // reverse
#endif
#ifdef USE_Z21
        if (myDir & 0x80)
          oled.print ('>');                                 // forward
        else
          oled.print ('<');                                 // reverse
#endif
#ifdef USE_ECOS
        if (myDir & 0x80)
          oled.print ('>');                                 // forward
        else
          oled.print ('<');                                 // reverse
#endif
      }
      printFunctions();                                     // funciones
      timeoutOLED = ONE_DAY;
      break;
    case SCR_LOCO:
#ifdef USE_ECOS
      oled.setFont(Arial_bold_14);
      oled.clear();
      text = findLocoID(scrLoco);
      if (text == LOCOS_IN_STACK) {
        oled.setCursor (24, 0);                             // address not found, print select loco
        printOtherTxt(TXT_SEL_LOCO);
      }
      else {
        oled.print(rosterList[text].locoName);               // ID found, print loco name
      }
      updateAllOLED = false;
      oled.setFont(Num_15x31);                              // numero locomotora
      if (updateExtraOLED) {
        oled.clear(32, 95, 3, 6);
        updateExtraOLED = false;
      }
      if (scrLoco > 0) {
        oled.setCursor(32, 3);                              // centra numero
        if (rosterList[text].locoAddress < 1000)
          oled.setCol(40);
        if (rosterList[text].locoAddress < 100)
          oled.setCol(48);
        if (rosterList[text].locoAddress < 10)
          oled.setCol(56);
        oled.print (rosterList[text].locoAddress);
      }
      else {
        oled.setCursor(56, 3);                              // ningun numero seleccionado
        oled.print('?');
      }
#else
      if (updateAllOLED) {
        oled.setFont(Arial_bold_14);
        oled.clear();
        oled.setCursor (24, 0);
        printOtherTxt(TXT_SEL_LOCO);
        updateAllOLED = false;
      }
      oled.setFont(Num_15x31);                              // numero locomotora
      if (updateExtraOLED) {
        oled.clear(32, 95, 3, 6);
        updateExtraOLED = false;
      }
      if (scrLoco > 0) {
        oled.setCursor(32, 3);                              // centra numero
        if (scrLoco < 1000)
          oled.setCol(40);
        if (scrLoco < 100)
          oled.setCol(48);
        if (scrLoco < 10)
          oled.setCol(56);
        oled.print (scrLoco);
      }
      else {
        oled.setCursor(56, 3);                              // ningun numero seleccionado
        oled.print('?');
      }
#endif
      timeoutOLED = ONE_DAY;
      break;
#ifdef USE_LOCONET
    case SCR_DISPATCH:                                      // Opciones de Dispatch
      oled.setFont(Arial_bold_14);
      oled.clear();
      oled.setCursor (20, 0);
      oled.print (F("DISPATCH"));
      oled.setCursor (20, 2);
      oled.print(F("GET "));
      printOtherTxt(TXT_LOCO);
      oled.setCursor (20, 4);
      if (mySlot.num > 0) {
        oled.print(F("PUT "));
        printOtherTxt(TXT_LOCO);
        oled.print(myLoco.address);
      }
      else {
        printMenuTxt(OPT_LOCO);
      }
      oled.setCursor (4, (encoderValue << 1) + 2);
      printCursor();
#ifdef USE_PHONE
      if (phoneBook[PHONE_ME] > 0) {
        oled.setCursor (20, 6);
        oled.print(';');                            // phone
        oled.setFont(Arial_bold_14);
        oled.print(F("  "));
        printOtherTxt(TXT_STATION);
      }
#endif
      break;
#endif
    case SCR_TURNOUT:                                       // Desvios
      if (updateAllOLED) {
        oled.setFont(Arial_bold_14);
        oled.clear();
        oled.setCursor (24, 0);
        printOtherTxt(TXT_SEL_TURN);
        updateAllOLED = false;
      }
      oled.setFont(Num_15x31);                              // numero desvio
      if (updateExtraOLED) {
        oled.clear(24, 71, 3, 6);
        oled.setCol(72);
        if (myTurnout > 0) {
          if (myTurnout > 9)
            oled.setCol(56);
          if (myTurnout > 99)
            oled.setCol(40);
          if (myTurnout > 999)
            oled.setCol(24);
          oled.print(myTurnout);
        }
        else {
          oled.setCursor(72, 3);
          oled.print('?');
        }
        updateExtraOLED = false;
      }
      oled.setCol(96);
      switch (myPosTurnout) {
        case RECTO:
          oled.print('-');
          break;
        case DESVIADO:
          oled.print('+');
          break;
        default:
          oled.print(',');
          break;
      }
      timeoutOLED = ONE_DAY;
      break;
    case SCR_WAIT:                                          // Espera respuesta
      estadoPuntos <<= 1;
      if (!bitRead(estadoPuntos, 7))
        bitSet(estadoPuntos, 0);
      oled.setFont(Num_15x31);
      oled.setCursor(56, 2);
      oled.print('?');
      oled.setFont(Sym_16x16);
      for (line = 0; line < 8; line++) {
        oled.setCursor(posPuntos[line][0], posPuntos[line][1]);
        if (bitRead(estadoPuntos, line))
          oled.print(',');
        else
          oled.print('/');
      }
      timeoutOLED = 300;
      break;
    case SCR_CFG:                                           // Configuracion
      //DEBUG_MSG("Opc. CFG: %d", cfgOLED);                   // 1..9     0 1 2 3 4 5 6 7 8 9 10
      oled.clear();
      if (cfgOLED < (MAX_CFG_ITEMS - 3))
        text = cfgOLED + 3;
      else
        text = MAX_CFG_ITEMS;
      for (line = cfgOLED - 1; line < text; line++) {
        printCfgOption((line - (cfgOLED - 1)) << 1, line);
      }
      oled.setCursor (4, 2);
      if ((cfgMenu[cfgOLED].type != TCFG_OPT) && (!changeCfgOption)) {
        oled.setFont(Arial_bold_14);
        oled.print('>');
        timeoutOLED = ONE_DAY;
      }
      else {
        printCursor();
      }
      break;
    case SCR_TURNTABLE:
      if (updateAllOLED) {                                  // Plataforma giratoria
        oled.clear();
        oled.setFont(Num_32x48);
        if (modeTurntable) {
          oled.setCol(64);
          oled.print(F(":;"));
          oled.setFont(Arial_bold_14);
          oled.setCursor(0, 6);
          printOtherTxt(TXT_DEST_TRK);
        }
        else {
          oled.setCol(32);
          oled.print(F(":;"));
          oled.setFont(Sym_16x16);
          oled.setCursor (24, 6);
          oled.print(F("3/5/0"));
        }
        updateAllOLED = false;
      }
      if (modeTurntable) {
        oled.setFont(Num_15x31);                            // numero salida
        oled.setCursor(16, 1);
        if (destTrack > 0) {
          if (destTrack < 10)
            oled.print(';');
          oled.print(destTrack);
        }
        else {
          oled.print(F(";?"));
        }
      }
      oled.setFont(Sym_Bars);
      oled.setCursor(0, 0);
      if (updateExtraOLED) {
        oled.print('<');
        timeoutOLED = 750;
        updateExtraOLED = false;
      }
      else {
        oled.print('/');
        timeoutOLED = ONE_DAY;
      }
      break;
    case SCR_CV:
      if (updateAllOLED) {                                  // Programar CV
        oled.clear();
        oled.setFont(Arial_bold_14);
        if (modeProg) {
          oled.print(F(" PoM"));                            // POM
          oled.setCol(50);
          oled.print(F("Loco: "));
          adr = myLoco.address & 0x3FFF;
          oled.print(adr);
        }
        else {
          oled.print(F("Prog"));                            // Programming track
        }
        if (enterCVdata) {                                  // CV data selected
          oled.setCursor(0, 4);
          oled.print(F("CV"));
          oled.print(CVaddress);
          oled.print(':');
          if (!modeProg)
            printNameCV();
        }
        else {
          oled.set2X();                                     // CV address selected
          oled.setCursor(8, 4);
          oled.print(F("CV"));
          oled.set1X();
        }
        oled.setFont(Sym_Bars);                             // track
        oled.setCursor(8, 2);
        oled.print('<');
        updateAllOLED = false;
        //if (modeBit)
        //  oled.print('+');
      }
      oled.setFont(Num_15x31);
      oled.clear(50, 128, 2, 7);
      if (enterCVdata) {
        oled.setCursor(58, 2);
        if (CVdata > 255) {                                 // Error in result
          oled.print(F("???"));
        }
        else {
          if (CVdata < 100)
            oled.setCol(66);
          if (CVdata < 10)
            oled.setCol(74);
          oled.print(CVdata);                               // result
          oled.setFont(Arial_bold_14);
          oled.setCursor(58, 6);
          for (line = 0; line < 8; line++) {                // show active bits
            if (bitRead(CVdata, 7 - line))
              oled.print(7 - line);
            else
              oled.print('-');
          }
        }
      }
      else {
        oled.setCursor(64, 3);
        if (CVaddress > 0) {                                // CV address
          if (CVaddress < 1000)
            oled.setCol(72);
          if (CVaddress < 100)
            oled.setCol(80);
          if (CVaddress < 10)
            oled.setCol(88);
          oled.print(CVaddress);
        }
        else {
          oled.print('?');
        }
      }
      timeoutOLED = ONE_DAY;
      break;
    case SCR_DIRCV:
      if (updateAllOLED) {                                  // Programar CV Direccion loccomotora
        oled.clear();
        oled.setFont(Arial_bold_14);
        oled.print(F("Prog"));                              // Programming track
        oled.setCol(50);
        printPrgTxt(PRG_DIR);
        oled.setFont(Sym_Bars);                             // track
        oled.setCursor(8, 2);
        oled.print('<');
        updateAllOLED = false;
      }
      oled.setFont(Num_15x31);
      oled.clear(32, 95, 4, 7);
      if (decoAddress > 0) {                                // CV address
        if (decoAddress < 1000)
          oled.setCol(40);
        if (decoAddress < 100)
          oled.setCol(48);
        if (decoAddress < 10)
          oled.setCol(56);
        oled.print(decoAddress);
      }
      else {
        oled.setCol(56);
        oled.print('?');
      }
      timeoutOLED = ONE_DAY;
      break;
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
    case SCR_SSID:                                          // SSID list
      oled.clear();
      oled.setFont(Arial_bold_14);
      text = (scrSSID > 3) ? scrSSID - 3 : 0;
      for (line = 0; line < 4; line++) {
        oled.setCursor (16, line * 2);
        snprintf (name, 18, WiFi.SSID(text + line).c_str());
        oled.print (name);
#ifdef DEBUG
        DEBUG_MSG("%d: %s", scrSSID + line, name);
        adr = WiFi.encryptionType(text + line);
        switch (adr) {
          case 5:
            Serial.println(F("ENC_TYPE_WEP  - WEP"));
            break;
          case 2:
            Serial.println(F("ENC_TYPE_TKIP - WPA / PSK"));
            break;
          case 4:
            Serial.println(F("ENC_TYPE_CCMP - WPA2 / PSK"));
            break;
          case 7:
            Serial.println(F("ENC_TYPE_NONE - open network"));
            break;
          case 8:
            Serial.println(F("ENC_TYPE_AUTO - WPA / WPA2 / PSK"));
            break;
        }
#endif
      }
      line = (scrSSID > 3) ? 6 : scrSSID * 2;
      oled.setCursor (0, line);
      printCursor();
      break;
    case SCR_PASSWORD:                                      // WiFi password
      oled.setFont(Arial_bold_14);
      if (updateAllOLED) {
        oled.clear();
        oled.print(F("SSID:"));
        oled.setCursor(0, 2);
        oled.print(wifiSetting.ssid);
        oled.setCursor(0, 4);
        oled.print(F("Password:"));
        updateAllOLED = false;
      }
      oled.clear(0, 127, 6, 7);
      text = (scrPwdLng > 11) ? scrPwdLng - 12 : 0;         // pos char inicial
      for (line = text; line < scrPwdLng; line++)
        oled.print(scrPwd[line]);
      oled.setInvertMode(true);
      oled.print((char)(encoderValue + 32));
      oled.setInvertMode(false);
      timeoutOLED = ONE_DAY;
      break;
    case SCR_IP_ADDR:                                     // Z21 IP address
      if (updateAllOLED) {
        oled.clear();
        oled.setFont(Arial_bold_14);
#ifdef USE_Z21
        oled.print(F("Z21 IP:"));
#endif
#ifdef USE_ECOS
        oled.print(F("ECoS IP:"));
#endif
#ifdef USE_LNWIFI
        oled.print(F("LNET IP:"));
#endif
#ifdef USE_XNWIFI
        oled.print(F("XNET IP:"));
#endif
        updateAllOLED = false;
      }
      oled.clear(0, 127, 3, 6);
      for (text = 0; text < 4; text++) {
        if (text == scrPosIP) {
          oled.setRow(3);
          oled.set2X();
          oled.setInvertMode(true);
          oled.print(scrIP[text]);
          oled.set1X();
          oled.setInvertMode(false);
        }
        else {
          oled.setRow(4);
          oled.print(scrIP[text]);
        }
        oled.setRow(4);
        if (text != 3)
          oled.print('.');
      }
      timeoutOLED = ONE_DAY;
      break;
#if defined(USE_LNWIFI)
    case SCR_PORT:
      oled.setFont(Arial_bold_14);
      if (updateAllOLED) {
        oled.clear();
        printConfigTxt(CFG_WIFI_PORT);
        updateAllOLED = false;
      }
      oled.setFont(Num_15x31);
      oled.clear(24, 128, 3, 6);
      if (scrPort < 10000)
        oled.setCol(32);
      if (scrPort < 1000)
        oled.setCol(40);
      if (scrPort < 100)
        oled.setCol(48);
      if (scrPort < 10)
        oled.setCol(56);
      oled.print(scrPort);
      timeoutOLED = ONE_DAY;
      break;
#endif
#endif
#ifdef GAME_EXTRA
    case SCR_GAME:
      switch (cfgOLED) {
        case CFG_GAME_TIC:
          drawGameTic();
          break;
        case CFG_GAME_SNAKE:
          drawGameSnake();
          break;
        case CFG_GAME_FLAPPY:
          drawGameFlappy();
          break;
        case CFG_GAME_PAKU:
          drawGamePaku();
          break;
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
        case CFG_GAME_PONG:
          drawGamePong();
          break;
#endif
      }
      break;
#endif
    case SCR_SHUTTLE:
      if (updateAllOLED) {
        oled.clear();
        oled.setFont(Sym_16x16);
        oled.setCursor(24, 2);
        oled.print(',');
        oled.setCursor(24, 4);
        oled.print(',');
        oled.setCursor(24, 6);
        oled.print(':');
        oled.setFont(Arial_bold_14);
        oled.setCursor(24, 0);
        oled.print(myLoco.address & 0x3FFF);
        updateAllOLED = false;
      }
      oled.setFont(Sym_16x16);
      oled.setCursor(80, 0);
      switch (suttleCurrentState) {
        case A_TO_B:
          oled.print (F("000"));    //>>>
          break;
        case B_TO_A:
          oled.print (F("333"));    //<<<
          break;
        case BRAKE_A:
          oled.print (F("4/3"));    //STA>
          break;
        case BRAKE_B:
          oled.print (F("0/4"));    //<STA
          break;
        case STATION_A:
          oled.print (F("4//"));    //STA
          break;
        case STATION_B:
          oled.print (F("//4"));    // STA
          break;
        case TRAVELLING:
          oled.print (F("310"));    //<0>
          break;
        case DEACTIVATED:
          oled.print (F("///"));    //nothing
      }
      markOptionShuttle(0, 0, SET_SHUTT);
      if (activeShuttle)
        oled.print('2');
      else
        oled.print('1');
      oled.setFont(Arial_bold_14);
      markOptionShuttle(48, 6, TIME_SHUTT);
      oled.print (Shuttle.time);
      line = oled.col();
      cleanOptionShuttle();
      oled.setCol(line);
      oled.print ('s');
#ifdef USE_LOCONET
      markOptionShuttle(48, 2, CONTACT_A);
      oled.print((Shuttle.moduleA << 8) + Shuttle.inputA);
      cleanOptionShuttle();
      markOptionShuttle(48, 4, CONTACT_B);
      oled.print((Shuttle.moduleB << 8) + Shuttle.inputB);
      cleanOptionShuttle();
#endif
#if defined(USE_Z21) || defined(USE_XPRESSNET) || defined(USE_ECOS)
      markOptionShuttle(48, 2, MODULE_A);
      oled.print(Shuttle.moduleA);
      line = oled.col();
      cleanOptionShuttle();
      oled.setCol(line);
      oled.print('.');
      markOptionShuttle(line + 4, 2, CONTACT_A);
      oled.print(Shuttle.inputA);
      cleanOptionShuttle();
      markOptionShuttle(48, 4, MODULE_B);
      oled.print(Shuttle.moduleB);
      line = oled.col();
      cleanOptionShuttle();
      oled.setCol(line);
      oled.print('.');
      markOptionShuttle(line + 4, 4, CONTACT_B);
      oled.print(Shuttle.inputB);
      cleanOptionShuttle();
#endif
      break;
#ifdef USE_PHONE
    case SCR_PHONE:
      if (updateAllOLED) {
        oled.clear();
        updateAllOLED = false;
        oled.setFont(Sym_16x16);
        oled.setCursor(56, 0);                              // draw Big phone & hanged phone
        oled.print(';');
        oled.setCursor(0, 6);
        oled.print('=');
      }
      oled.clear(20, 127, 4, 5); // draw status
      oled.setFont(Arial_bold_14);
      switch (phonePhase) {
        case PHONE_CHECK:
          //printOutgoingCall();                              // >
          oled.setFont(Sym_16x16);                          // >
          oled.setCursor (72, 0);
          oled.print('0');
          break;
        case PHONE_BUSY:
          printOtherTxt(TXT_PHONE_BUSY);                    // "Busy"
          //printOutgoingCall();                              // >
          break;
        case PHONE_IDLE:
          printOtherTxt(TXT_STOP_TRAIN);                    // "Stop Train"
          oled.clear(72, 88, 0, 1);                         // delete '>'
          break;
        case PHONE_CALLING:
        //printOutgoingCall();                              // >
        //oled.setCursor(20,4);
        case WAIT_ANSWER:
          printPhoneContact(callingStation);                // "Estacion B"
          break;
        case WAIT_USER:
          printPhoneContact(originateStation);              // "Estacion A"
          oled.setFont(Sym_16x16);                          // >
          oled.setCursor (40, 0);
          oled.print('0');
          oled.setCursor (100, 6);
          oled.print('<');                                  // pick up phone
          break;
      }
      break;
    case SCR_PHONEBOOK:
    case SCR_STATION:
      oled.setFont(Arial_bold_14);
      oled.clear();
      for (line = 0; line < (MAX_STATION - 1); line++) {
        oled.setCursor (20, line << 1);
        printOtherTxt(TXT_STATION);                         // "Estacion"
        oled.print(' ');
        oled.print ((char) ('A' + line));                   // A..D
      }
      oled.setCursor (4, (encoderValue << 1));
      oled.setFont(Sym_16x16);
      oled.print ('0');
      timeoutOLED = ONE_DAY;
      break;
    case SCR_PHONE_NUM:
      oled.setFont(Arial_bold_14);
      oled.clear();
      if (cfgMenu[CFG_PHONE].value == MAX_STATION)
        printConfigTxt(CFG_PHONE_ME);
      else {
        printOtherTxt(TXT_STATION);                         // "Estacion"
        oled.print(' ');
        oled.print ((char) ('A' + cfgMenu[CFG_PHONE].value));   // A..D
      }
      oled.setFont(Num_15x31);
      oled.setCursor(40, 3);
      oled.print (PHONE_BASE / 100);                        // print cents
      if (encoderValue > 0) {
        if (encoderValue < 10)
          oled.print('0');
        oled.print(encoderValue);                         // 701
      }
      else
        oled.print(F("??"));                                // 7??
      break;
#endif
#ifdef USE_LNCV
    case SCR_LNCV:
      oled.clear();
      oled.setFont(Arial_bold_14);
      oled.print(F("LNCV "));
      if (optLNCV > LNCV_MOD) {
        oled.print(artNum);
        oled.print('-');
        oled.print(modNum);
      }
      switch (optLNCV) {
        case LNCV_MOD:
          oled.setCursor(0, 6);
          oled.print(F("Mod-num: "));
          oled.print(modNum);
        case LNCV_ART:
          oled.setCursor(0, 3);
          oled.print(F("Art-num: "));
          oled.print(artNum);
          break;
        case LNCV_ADR:
          oled.setCursor(0, 5);
          oled.print(F("LNCV: "));
          oled.setFont(Num_15x31);
          oled.setCursor(48, 3);
          //oled.clear(48, 128, 2, 7);
          oled.print(numLNCV);
          break;
        case LNCV_VAL:
          oled.setCursor(0, 2);
          if (numLNCV < 1000)
            oled.setCol(8);
          oled.print(numLNCV);
          oled.setFont(Num_15x31);
          oled.setCursor(48, 3);
          if (updateExtraOLED) {
            oled.print('?');
            updateExtraOLED = false;
          }
          else {
            oled.print(valLNCV);
          }
          break;
      }
      timeoutOLED = ONE_DAY;
      break;
#endif
#ifdef USE_AUTOMATION
    case SCR_AUTO_SELECT:
      if (updateAllOLED) {
        oled.clear();
        oled.setFont(Arial_bold_14);
        printOtherTxt(TXT_AUTOMATION);                      // "Automation"
        oled.setFont(Auto_16x16);
        oled.setCursor(56, 6);
        oled.print('D');                                    // add
        updateAllOLED = false;
      }
      if (fileCount > 0) {                                  // files on disk
        showFileSelect(encoderValue + 1, fileCount);
        oled.setFont(Arial_bold_14);
        oled.clear(0, 127, 3, 4);
        getFileName(encoderValue, name);                    // sequence name
        oled.print(name);
        oled.setFont(Auto_16x16);
        oled.setCursor(0, 6);
        spd = findAutomation(getFileStart(encoderValue));   // running?
        oled.print((spd != MAX_AUTO_SEQ) ? 'C' : 'B');
        oled.setCol(112);
        oled.print('E');                                    // edit
      }
      else {                                                // disk empty
        showFileSelect(0, 0);
        oled.setFont(Auto_16x16);
        oled.setCursor(0, 3);
        oled.print('A');
      }
      timeoutOLED = ONE_DAY;
      break;
    case SCR_AUTO_NAME:
      if (updateAllOLED) {
        oled.clear();
        oled.setFont(Arial_bold_14);
        printOtherTxt(TXT_AUTO_NAME);                       // "Name:"
        oled.setFont(Auto_16x16);
        oled.setCursor(112, 6);
        oled.print('V');                                  // edit sequence
        if (!isNewFile) {
          oled.setCol(0);
          oled.print('F');                                  // delete
        }
        updateAllOLED = false;
      }
      oled.setFont(Arial_bold_14);
      oled.clear(0, 127, 3, 4);
      text = (scrNameLng > 11) ? scrNameLng - 12 : 0;       // pos char inicial
      for (line = text; line < scrNameLng; line++)
        oled.print(scrName[line]);
      oled.setInvertMode(true);
      oled.print((char)(encoderValue + 32));
      oled.setInvertMode(false);
      timeoutOLED = ONE_DAY;
      break;
    case SCR_AUTO_EDIT:
      if (updateAllOLED) {
        oled.clear();
        oled.setFont(Arial_bold_14);
        getNameSequence(name);
        oled.print(name);
        updateAllOLED = false;
      }
      oled.setFont(Auto_16x16);
      if (editingOpcode) {
        oled.clear(0, 127, 6, 7);
      }
      else {
        if (isOpcodeEnd(editPos)) {
          oled.clear(0, 15, 6, 7);
          oled.setCol(56);
          oled.print('D');                                  // add
          oled.setCol(112);
          oled.print('H');                                  // save
        }
        else {
          oled.setCursor(0, 6);
          oled.print('F');                                  // delete
          oled.setCol(56);
          oled.print('D');                                  // add
          oled.setCol(112);
          oled.print('E');                                  // edit
        }
      }
      oled.setCursor(0, 3);
      oled.print(getIconOpcode());
      printParameterOpcode();
      if (!editingOpcode)
        showFileSelect(encoderValue + 1, encoderMax + 1);
      break;
    case SCR_AUTO_DELETE:
      oled.clear();
      oled.setFont(Arial_bold_14);
      printOtherTxt(TXT_AUTO_DELETE);                       // "Delete?"
      oled.setCursor(0, 3);
      if (editingSequence) {
        oled.setFont(Auto_16x16);
        oled.print(getIconOpcode());
        printParameterOpcode();
      }
      else {
        getFileName(fileOLED, scrName);
        oled.print(scrName);
      }
      oled.setFont(Auto_16x16);
      oled.setCursor(56, 6);
      oled.print('F');                                      // delete
      oled.setCol(112);
      oled.print('G');                                      // cancel
      timeoutOLED = ONE_DAY;
      break;
#endif
#ifdef USE_SET_TIME
    case SCR_TIME:
      if (updateAllOLED) {
        oled.clear();
        oled.setFont(Arial_bold_14);
        printConfigTxt(CFG_TIME);
        updateAllOLED = false;
      }
      oled.clear(0, 127, 3, 6);
      for (text = 0; text < 6; text++) {
        if (text == scrPosTime) {
          oled.setRow(3);
          oled.set2X();
          oled.setInvertMode(true);
        }
        else {
          oled.setRow(4);
          oled.set1X();
          oled.setInvertMode(false);
        }
        switch (text) {
          case 0:
            if (scrHour < 10)
              oled.print('0');
            oled.print(scrHour);
            break;
          case 1:
            if (scrPosTime == 5)
              oled.print(':');
            else
              oled.print(" : ");
            break;
          case 2:
            if (scrMin < 10)
              oled.print('0');
            oled.print(scrMin);
            break;
          case 3:
            oled.print("   ");
            break;
          case 4:
            if (scrRate > 0)
              oled.print("1:");
            break;
          case 5:
            if (scrRate > 0)
              oled.print(scrRate);
            else
              oled.print("STOP");
            break;
        }
      }
      oled.set1X();
      oled.setInvertMode(false);
      timeoutOLED = ONE_DAY;
      break;
#endif
  }
}


#ifdef USE_AUTOMATION
void showFileSelect(byte curr, byte total) {
  char dizaine, unit;
  oled.setFont(Sym_Bars);
  oled.clear(100, 127, 2, 2);                               // 1 / 15
  if (curr < 10)
    oled.print(',');
  oled.print(curr);
  oled.print('D');
  if (total < 10)
    oled.print(',');
  oled.print(total);
}


char getIconOpcode() {
  byte opcode, param;
  char chr;
  opcode = bufferEdit[editPos];
  param = bufferEdit[editPos + 1];
  //DEBUG_MSG("Opcode edit pos: %d - %02x %02x", editPos, opcode, param)
  switch (opcode & OPC_AUTO_MASK) {
    case OPC_AUTO_LOCO_SEL:
    case OPC_AUTO_LOCO_SEL4:
    case OPC_AUTO_LOCO_SEL8:
      chr = 'M';                                            // loco
      break;
    case OPC_AUTO_LOCO_OPT:
      if (bitRead(param, 7)) {
        if (bitRead(param, 6))
          chr = 'Q' + (param & 0x03);                       //direction
        else
          chr = (param & 0x20) ? 'P' : 'O';                 // fon / foff
      }
      else
        chr = 'N';                                          // speed
      break;
    case OPC_AUTO_DELAY:
      chr = 'U';
      break;
    case OPC_AUTO_ACC:
      chr = bitRead(opcode, 3) ? 'J' : 'I';
      break;
    case OPC_AUTO_FBK:
      chr = bitRead(opcode, 3) ? 'L' : 'K';
      break;
    case OPC_AUTO_JUMP:
      switch (param) {
        case OPC_AUTO_END:
          chr = 'C';
          DEBUG_MSG("OPC END")
          break;
        case OPC_AUTO_LOOP:
          chr = 'W';
          break;
        default:                                            // start other sequence
          chr = 'V';
          break;
      }
      break;
  }
  return chr;
}


void printParameterOpcode() {
  byte opcode, param, n;
  unsigned int value;
  oled.setFont(Arial_bold_14);
  oled.clear(20, 127, 3, 4);
  opcode = bufferEdit[editPos];
  param = bufferEdit[editPos + 1];
  switch (opcode & OPC_AUTO_MASK) {
    case OPC_AUTO_LOCO_SEL:
    case OPC_AUTO_LOCO_SEL4:
    case OPC_AUTO_LOCO_SEL8:
      value = ((opcode & 0x3F) << 8) + param;               // loco
#ifdef USE_ECOS
      for (n = 0; n < LOCOS_IN_STACK; n++) {
        if (value == rosterList[n].id) {
          oled.print(rosterList[n].locoName);
          return;
        }
      }
#endif
      break;
    case OPC_AUTO_LOCO_OPT:
      if (bitRead(param, 7)) {
        if (bitRead(param, 6))
          return;                                           // direction
        else
          value = param & 0x1F;                             // function
      }
      else {
        oled.print(param);                                  // speed
        oled.print('%');
        return;
      }
      break;
    case OPC_AUTO_DELAY:
      value = ((opcode & 0x0F) << 8) + param;               // time
      oled.print(value / 10);
      oled.print('.');
      oled.print(value % 10);
      oled.print('s');
      return;
      break;
    case OPC_AUTO_ACC:
      value = (((opcode & 0x03) << 8) + param) + 1;         // accessory
      break;
    case OPC_AUTO_FBK:                                      // feedback
#ifdef USE_LOCONET
      value = (((opcode & 0x07) << 8) + param) + 1;         // 1..2048
      break;
#endif
#ifdef USE_XPRESSNET
      oled.print(param + 1);                                // 1.1..128.8
      oled.print('.');
      value = (opcode & 0x07) + 1;
#endif
#ifdef USE_Z21
      oled.print(param + 1);                                // 1.1..20.8
      oled.print('.');
      value = (opcode & 0x07) + 1;
#endif
#ifdef USE_ECOS
      value = param >> 4;                                   // 1.1..32.16
      if (bitRead(opcode, 0))
        value += 16;
      oled.print(value + 1);
      oled.print('.');
      value = (param & 0x0F) + 1;
      if (value < 10)
        oled.print('0');
#endif
      break;
    case OPC_AUTO_JUMP:
      if (param < OPC_AUTO_LOOP) {                          // sequence
        value = (param * SECTOR_SIZE) + DISK_INI;
        opcode = EEPROM.read(value++);
        if (opcode < 16) {
          for (n = 0; n < opcode; n++)                      // name
            oled.print((char)EEPROM.read(value++));
        }
      }
      return;
      break;
  }
  oled.print(value);
}
#endif


void markOptionShuttle(byte x, byte y, byte opt) {
  oled.setCursor(x, y);
  oled.setInvertMode(false);
  if (shuttleConfig && (optShuttle == opt))
    oled.setInvertMode(true);
}


void cleanOptionShuttle() {
  oled.setInvertMode(false);
  oled.print(F("     "));
}


byte calcBarSpeed(byte encoder) {                           // calcula barra para posicion encoder segun los pasos
#ifdef USE_LOCONET
  byte steps;

  steps = getMaxStepLnet();
  if (steps == 128) {
    return encoder;                                         // 0..63
  }
  else {
    if (steps == 28)
      return (encoder << 1);                                // 0..31
    else
      return (encoder << 2);                                // 0..15
  }
#endif
#if defined(USE_Z21) || defined(USE_XPRESSNET)
  if (bitRead(mySteps, 2)) {                                // 0..63
    return encoder;
  }
  else if (bitRead(mySteps, 1)) {                           // 0..31
    return (encoder << 1);

  }
  else {                                                    // 0..15
    return (encoder << 2);
  }
#endif
#ifdef USE_ECOS
  return encoder;
#endif
}


void  printBarSpeed(byte barScr, byte barSpd) {             // imprime barra velocidad
  byte diff;
  //DEBUG_MSG("barScr: %d  barSpd: %d", barScr, barSpd);

  oled.setRow(4);
  if (barSpd > barScr) {                                    // aumentar
    diff = barSpd - barScr;
    oled.setCol (barSpd - diff + 40);
    while (diff--)
      oled.ssd1306WriteRam(0xFF);
  }
  if (barSpd < barScr) {                                    // disminuir
    diff = barScr - barSpd;
    oled.setCol (barScr - diff + 40);
    while (diff--)
      oled.ssd1306WriteRam(0x00);
  }
  scrSpeed = barSpd;
}


void printCurrentStep() {
  byte currStep;
  currStep = getCurrentStep();
  if (currStep < 100)
    oled.print(',');
  if (currStep < 10)
    oled.print(',');
  oled.print(currStep);
}


void printCursor() {
  oled.setFont(Sym_16x16);
  oled.print ('0');
  timeoutOLED = ONE_DAY;
}


#if  (FUNC_SIZE == BIG)
void printFunctions() {
  byte i;
  oled.setFont(Sym_16x16);                                  // funciones en pantalla
  if (scrFunc > FNC_0_4) {
    oled.setCursor(64, 6);
    for (i = 1; i < 5; i++)
      printFuncButton((scrFunc << 2) + i);
  }
  else {
    oled.setCursor(48, 6);
    for (i = 0; i < 5; i++)
      printFuncButton(i);
  }
}

void printFuncButton (byte numFunc) {
  if (myFunc.Bits & bit(numFunc))                           // Estado funcion
    oled.print ('+');
  else
    oled.print ('-');
}
#endif
#if  (FUNC_SIZE == SMALL)
void printFunctions() {
  byte i;
  oled.setFont(Sym_Bars);
  oled.setCursor(28, 6);
  oled.print(F("0C1C2C3C4C5C6C7C8"));
  if (scrFunc != FNC_20_28)
    oled.print(F("C9"));
  oled.setCursor(26, 7);
  for (i = 0; i < 9; i++)
    printFuncButton(i + (scrFunc * 10));
  if (scrFunc != FNC_20_28)
    printFuncButton(9 + (scrFunc * 10));
}

void printFuncButton (byte numFunc) {
  if (myFunc.Bits & bit(numFunc))                           // Estado funcion
    oled.print ('+');
  else
    oled.print ('-');
}
#endif

void printNameCV () {                                       // imprime el nombre de las CV conocidas
  byte num;
  num = 0;
  oled.setCursor(38, 0);
  switch (CVaddress) {
    case 1:
    case 17:
    case 18:
    case 19:
      num = PRG_DIR;
      break;
    case 2:
      num = PRG_CV2;
      break;
    case 3:
      num = PRG_CV3;
      break;
    case 4:
      num = PRG_CV4;
      break;
    case 5:
      num = PRG_CV5;
      break;
    case 6:
      num = PRG_CV6;
      break;
    case 541:
    case 29:
      num = PRG_CV29;
      break;
    case 520:
    case 8:
      switch (CVdata) {                                     // imprime el fabricante conocido
        case 13:
          oled.print(F("DIY"));
          break;
        case 74:
          oled.print(F("PpP"));
          break;
        case 42:
          oled.print(F("Digikeijs"));
          break;
        case 151:
          oled.print(F("ESU"));
          break;
        case 145:
          oled.print(F("Zimo"));
          break;
        case 99:
          oled.print(F("Lenz"));
          break;
        case 97:
          oled.print(F("D&H"));
          break;
        case 157:
          oled.print(F("Kuehn"));
          break;
        case 62:
          oled.print(F("Tams"));
          break;
        case 85:
          oled.print(F("Uhlenbrock"));
          break;
        case 134:
          oled.print(F("Lais"));
          break;
        case 129:
          oled.print(F("Digitrax"));
          break;
        case 161:
          oled.print(F("Roco"));
          break;
        case 109:
          oled.print(F("Viessmann"));
          break;
        case 78:
          oled.print(F("TrainOmatic"));
          break;
        case 117:
          oled.print(F("CT Elektronik"));
          break;
        default:
          num = PRG_FAB;
          break;
      }
      break;
  }
  if (num > 0)
    printPrgTxt(num);
}

#ifdef USE_PHONE
void printPhoneContact (int adr) {
  byte i;
  for (i = 1; i < MAX_STATION; i++)
    if (adr == (phoneBook[i] + PHONE_BASE)) {
      printOtherTxt(TXT_STATION);                           // "Estacion"
      oled.print(' ');
      oled.print ((char) ('@' + i));                        // A..D
    }
}

/*
  void printOutgoingCall() {
  oled.setFont(Sym_16x16);                                  // >
  oled.setCursor (72, 0);
  oled.print('0');
  }
*/
#endif


void printConfigTxt(byte num) {                             // imprime texto configuracion
  char chr;
  byte lng;
  lng = strlen_P(cfgMsg[num]);
  for (byte k = 0; k < lng; k++) {
    chr = pgm_read_byte_near(cfgMsg[num] + k);
    oled.print(chr);
  }
}


void printMenuTxt(byte num) {                               // imprime texto menu
  char chr;
  byte lng;
  lng = strlen_P(menuMsg[num]);
  for (byte k = 0; k < lng; k++) {
    chr = pgm_read_byte_near(menuMsg[num] + k);
    oled.print(chr);
  }
}


void printOtherTxt(byte num) {                               // imprime otros textos
  char chr;
  byte lng;
  lng = strlen_P(otherMsg[num]);
  for (byte k = 0; k < lng; k++) {
    chr = pgm_read_byte_near(otherMsg[num] + k);
    oled.print(chr);
  }
}


void printPrgTxt(byte num) {                                // imprime textos programacion
  char chr;
  byte lng;
  lng = strlen_P(prgMsg[num]);
  for (byte k = 0; k < lng; k++) {
    chr = pgm_read_byte_near(prgMsg[num] + k);
    oled.print(chr);
  }
}


void printCfgOption (byte row, byte num) {                  // imprime opcion configuracion
  oled.setFont(Arial_bold_14);
  oled.setCursor(40, row);
  switch (cfgMenu[num].type) {
    case TCFG_TIT:
      oled.setCol(0);
      printConfigTxt(num);
      break;
    case TCFG_TXT:
      printConfigTxt(num);
      break;
    case TCFG_OPT:
      oled.setCol(20);
      printConfigTxt(num);
      break;
    case TCFG_NUM:
      oled.print(cfgMenu[num].value);
      break;
    case TCFG_RAD:
      printConfigTxt(num);
      oled.setFont(Sym_16x16);
      oled.setCol(20);
      if (cfgMenu[num].value)
        oled.print(',');
      else
        oled.print('.');
      break;
    case TCFG_CHK:
      printConfigTxt(num);
      oled.setFont(Sym_16x16);
      oled.setCol(20);
      if (cfgMenu[num].value)
        oled.print('2');
      else
        oled.print('1');
      break;
  }
}
