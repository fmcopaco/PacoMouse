/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/


////////////////////////////////////////////////////////////
// ***** LECTURA TECLADO *****
////////////////////////////////////////////////////////////


void readKeypad() {
  byte inputButton;

  inputButton = keypad.getKey();                            // lee teclado
  if (inputButton != NO_KEY) {
    keyValue = inputButton;
    keyOn = true;
  }
  else {
    if (accEnviado) {                                       // desactiva desvio al soltar la tecla
      if (keypad.getState() == RELEASED) {
        accEnviado = false;
#ifdef USE_LOCONET
        lnetRequestSwitch(myTurnout, 0, myPosTurnout - DESVIADO);
#endif
#ifdef USE_XPRESSNET
        enviaAccesorio(myTurnout, false, myPosTurnout - DESVIADO);
#endif
#ifdef USE_Z21
        setTurnout (myTurnout, myPosTurnout - DESVIADO, false);
#endif
#ifdef USE_ECOS
#endif
      }
    }
  }
}


////////////////////////////////////////////////////////////
// ***** CONTROL TECLADO *****
////////////////////////////////////////////////////////////

void controlKeypad() {
  byte maxFnc, pos;
  unsigned int value;
  keyOn = false;
  /*
    #ifdef DEBUG
    if (isTeclaNumerica()) {
      DEBUG_MSG("Numero: %d", keyValue - 1);
    }
    if (keyValue == K_MENU) {
      DEBUG_MSG("MENU");
    }
    if (keyValue == K_ENTER) {
      DEBUG_MSG("ENTER");
    }
    if (keyValue == K_A) {
      DEBUG_MSG("STOP");
    }
    if (keyValue == K_B) {
      DEBUG_MSG("LOCO");
    }
    if (keyValue == K_C) {
      DEBUG_MSG("UP / RECTO");
    }
    if (keyValue == K_D) {
      DEBUG_MSG("DOWN / DESVIADO");
    }
    #endif
  */
  if ((keyValue == K_A) && notLocked()) {                   // Tecla A: Emergency off
#ifdef USE_LOCONET
    if (bitRead(mySlot.trk, 0))
      emergencyOff();                                       //  Track Power Off
    else
      resumeOperations();                                   //  Track Power On
#endif
#ifdef USE_XPRESSNET
    if (csStatus & csEmergencyOff)
      resumeOperations();                                   //  Track Power On
    else
      emergencyOff();                                       //  Track Power Off
#endif
#ifdef USE_Z21
    if (csStatus & csTrackVoltageOff)
      resumeOperations();                                   //  Track Power On
    else
      emergencyOff();                                       //  Track Power Off
#endif
#ifdef USE_ECOS
    if (csStatus > 0)
      emergencyOff();                                       //  Track Power Off
    else
      resumeOperations();                                   //  Track Power On
#endif
  }

  if (keyValue == K_B) {                                      // Tecla B: Enter Loco
#ifdef USE_LOCONET
    if (bitRead(mySlot.trk, 0))
#endif
#if defined(USE_Z21) || defined(USE_XPRESSNET)
      if (csStatus == csNormalOps)
#endif
#ifdef USE_ECOS
        if (csStatus == 1)
#endif
        { // only in normal operations
          if (scrOLED == SCR_LOCO) {
            if (notLockedOption(LOCK_TURNOUT)) {
              optOLED = OPT_TURNOUT;
              enterMenuOption();
            }
          }
          else {
            if (notLockedOption(LOCK_SEL_LOCO)) {
              optOLED = OPT_LOCO;
              enterMenuOption();
            }
            else {
              if (notLockedOption(LOCK_TURNOUT)) {
                optOLED = OPT_TURNOUT;
                enterMenuOption();
              }
            }
          }
        }
  }
  switch (scrOLED)  {
    case SCR_MENU:
      if ((keyValue == K_MENU) || (keyValue == K_D)) {      // MENU / D: siguiente opcion de menu
        optOLED++;
        if (optOLED >= MENU_ITEMS)
          optOLED = 0;
        encoderValue = optOLED;
        updateOLED = true;
      }
      if (keyValue == K_C) {                                // C: anterior opcion de menu
        if (optOLED > 0)
          optOLED--;
        else
          optOLED = MENU_ITEMS - 1;
        encoderValue = optOLED;
        updateOLED = true;
      }
      if (keyValue == K_ENTER) {                             // ENTER: entra opcion menu
#if USE_LOCONET
        if ((mySlot.num == 0) && (optOLED == OPT_SPEED))
          optOLED = OPT_LOCO;
#endif
        enterMenuOption();
      }
      break;
    case SCR_SELCV:
#ifdef USE_XPRESSNET
      if (keyValue == K_MENU) {
        if (csStatus & csServiceMode)
          resumeOperations();
      }
#endif
#ifdef USE_Z21
      if (keyValue == K_MENU) {
        if (csStatus & csProgrammingModeActive)
          resumeOperations();
      }
#endif
#ifdef USE_ECOS
      if (keyValue == K_MENU) {
        exitProgramming();
      }
#endif
      isKeyMenu();
      if (keyValue == K_ENTER)
        enterPrgOption();
      if (keyValue == K_C) {
        if (prgOLED > 0)
          prgOLED--;
        else
          prgOLED = PRG_ITEMS - 1;
        encoderValue = prgOLED;
        updateOLED = true;
      }
      if (keyValue == K_D) {
        prgOLED++;
        if (prgOLED >=  PRG_ITEMS)
          prgOLED = 0;
        encoderValue = prgOLED;
        updateOLED = true;
      }
      break;
    case SCR_SPEED:
      isKeyMenu();
#if  (FUNC_SIZE == BIG)
      maxFnc = FNC_25_28;
      if ((keyValue == K_ENTER) || (keyValue == K_C)) {     // proximo grupo de funciones
        scrFunc++;
        if (scrFunc > maxFnc)
          scrFunc = FNC_0_4;
        updateAllOLED = true;
        showExtra();
      }
      if (keyValue == K_D) {                                // previo grupo de funciones
        if (scrFunc == FNC_0_4)
          scrFunc = maxFnc;
        else {
          scrFunc--;
        }
        updateAllOLED = true;
        showExtra();
      }
#endif
#if  (FUNC_SIZE == SMALL)
      maxFnc = FNC_20_28;
      if ((keyValue == K_ENTER) || (keyValue == K_C)) {     // proximo grupo de funciones
        scrFunc++;
        if (scrFunc > maxFnc)
          scrFunc = FNC_0_9;
        updateAllOLED = true;
        showExtra();
      }
      if (keyValue == K_D) {                                // previo grupo de funciones
        if (scrFunc == FNC_0_9)
          scrFunc = maxFnc;
        else {
          scrFunc--;
        }
        updateAllOLED = true;
        showExtra();
      }
#endif
      if (isTeclaNumerica()) {                              // activa/desactiva funcion
#if  (FUNC_SIZE == BIG)
        switch (keyValue) {
          case K_NUM0:
            myFunc.Bits ^= bit(0);                          // Luz
            funcOperations(0);                              // Luz
            updateOLED = true;
            break;
          case K_NUM1:
          case K_NUM2:
          case K_NUM3:
          case K_NUM4:
            myFunc.Bits ^= bit((keyValue - K_NUM0) + (scrFunc << 2));
            funcOperations((keyValue - K_NUM0) + (scrFunc << 2));
            updateOLED = true;
            break;
          case K_NUM5:
          case K_NUM6:
          case K_NUM7:
          case K_NUM8:
            if (scrFunc == FNC_0_4) {
              myFunc.Bits ^= bit(keyValue - K_NUM0);
              funcOperations(keyValue - K_NUM0);
            }
            break;
          case K_NUM9:
            showBigClock = !showBigClock;
            updateAllOLED = true;
            showExtra();
            break;
        }
#endif
#if  (FUNC_SIZE == SMALL)
        if ((keyValue == K_NUM9) && (scrFunc == FNC_20_28)) {
          showBigClock = !showBigClock;
          updateAllOLED = true;
          showExtra();
        }
        else {
          myFunc.Bits ^= bit((keyValue - K_NUM0) + (scrFunc * 10));
          funcOperations((keyValue - K_NUM0) + (scrFunc * 10));
          updateOLED = true;
        }
#endif
      }
      break;
    case SCR_LOCO:
      isKeyMenu();
      if (keyValue == K_ENTER) {                            // entrada numero locomotora
        scrLocoGet();
      }
#ifndef USE_ECOS
      if (isTeclaNumerica()) {
        inputDigit();
        scrLoco = calcInputValue();
        showExtra();
      }
#endif
      if (keyValue == K_C) {
        if (encoderValue < (LOCOS_IN_STACK - 1))
          encoderValue++;
        if ((encoderValue > 0) && (locoStack[encoderValue] == 0))
          encoderValue--;
        showStackLoco();
      }
      if (keyValue == K_D) {
        if (encoderValue > 0)
          encoderValue--;
        showStackLoco();
      }
      break;
#ifdef USE_LOCONET
    case SCR_DISPATCH:
      isKeyMenu();
      if (keyValue == K_ENTER) {
        selectDispatch();
      }
      checkKeyUpD ();
      checkKeyDownC ();
      break;
#endif
    case SCR_TURNOUT:
      isKeyMenu();
      if (isTeclaNumerica()) {
        inputDigit();
        if (myInput.digit[THOUSANDS] > 1)                   // maximo 1999
          myInput.digit[THOUSANDS] = 0;
        myTurnout = calcInputValue();
        if (myInput.packed > 0) {
          infoDesvio(myTurnout);                            // busca la posicion del desvio
        }
        else {
          myPosTurnout = NO_MOVIDO;
        }
        showExtra();
      }
      if (keyValue == K_ENTER) {                            // cambia posicion desvio
        if (myTurnout > 0) {
          if (myPosTurnout == NO_MOVIDO)
            myPosTurnout = lastPosTurnout;
          myPosTurnout = (myPosTurnout == RECTO) ? DESVIADO : RECTO;
          lastPosTurnout = myPosTurnout;
          mueveDesvio();
          clearInputDigit();
#if defined(USE_ECOS)
          updateOLED = true;
#endif
        }
      }
      if (keyValue == K_C) {                                // incrementa desvio
        if (myTurnout < 1999) {
          myTurnout++;
          myPosTurnout = NO_MOVIDO;
          infoDesvio(myTurnout);                            // busca la posicion del desvio
          clearInputDigit();
          showExtra();                                      // update number in OLED
        }
      }
      if (keyValue == K_D) {                                // decrementa desvio
        if (myTurnout > 1) {
          myTurnout--;
          myPosTurnout = NO_MOVIDO;
          infoDesvio(myTurnout);                            // busca la posicion del desvio
          clearInputDigit();
          showExtra();                                      // update number in OLED
        }
      }
      /*
            if (keyValue == K_C) {                                // cambia desvio a recto
              if (myTurnout > 0) {
                myPosTurnout = RECTO;
                mueveDesvio();
                clearInputDigit();
              }
            }
            if (keyValue == K_D) {                                // cambia desvio a desviado
              if (myTurnout > 0) {
                myPosTurnout = DESVIADO;
                mueveDesvio();
                clearInputDigit();
              }
            }
      */
      break;
    case SCR_WAIT:
      if (keyValue == K_MENU) {
#if defined(USE_Z21) || defined(USE_XPRESSNET)
        if (csStatus != csNormalOps)
          resumeOperations();
#endif
#ifdef USE_ECOS
        if (requestedCV)
          exitProgramming();
#endif
        showMenu();
#ifdef USE_LOCONET
        doDispatchGet = false;
        lnetProg = false;
        if (!bitRead(mySlot.trk, 0))
          showEmergencyOff();
#ifdef USE_PHONE
        if (phonePhase == WAIT_TRAIN)
          phonePhase = CALL_REJECTED;
#endif
#endif
      }
      break;
    case SCR_CFG:
      if (keyValue == K_MENU) {
        if (changeCfgOption) {
          changeCfgOption = false;
          encoderMax = MAX_CFG_ITEMS - 1;
          encoderValue = cfgOLED;
          cfgMenu[CFG_CONTR_VAL].value = readEEPROM(EE_CONTRAST);
          oled.setContrast(cfgMenu[CFG_CONTR_VAL].value);
        }
        else {
          showMenu();
        }
        updateOLED = true;
      }
      if (keyValue == K_ENTER) {
        saveCfgOption();
      }
      if (changeCfgOption) {
#ifdef USE_XNWIRE
        if ((cfgOLED == CFG_CONTR_VAL) || (cfgOLED == CFG_XBUS_DIR))
#endif
#if defined(USE_LOCONET) || defined(USE_Z21) || defined(USE_ECOS) || defined(USE_XNWIFI)
          if (cfgOLED == CFG_CONTR_VAL)
#endif
          {
            if (keyValue == K_C)                            // correct numeric option
              keyValue = K_D;
            else {
              if (keyValue == K_D)
                keyValue = K_C;
            }
          }
      }
      if (keyValue == K_D) {
        if (encoderValue < encoderMax)
          encoderValue++;
        //updateOLED = true;
        controlEncoder();
      }
      if (keyValue == K_C) {
        if (encoderValue != 0)
          encoderValue--;
        //updateOLED = true;
        controlEncoder();
      }
      break;
    case SCR_TURNTABLE:
      isKeyMenu();
      if (keyValue == K_ENTER) {
        if (modeTurntable) {
          if (destTrack > 0) {
            switch (typeTurntable) {
              case TT_F6915:
                myTurnout = destTrack + baseAdrTurntable + baseOffset;
                break;
              default:
                myPosTurnout = (destTrack & 0x01) ? DESVIADO : RECTO;
                myTurnout = (destTrack - 1) >> 1;
                myTurnout = myTurnout + baseAdrTurntable + baseOffset + 4;
                break;
            }
            mueveDesvio();
            clearInputDigit();
            updateExtraOLED = true;
          }
          else {
            updateAllOLED = true;
            modeTurntable = false;
          }
          updateOLED = true;
        }
        else {
          modeTurntable = true;
          updateAllOLED = true;
          updateOLED = true;
        }
      }
      if (isTeclaNumerica()) {
        if (modeTurntable) {
          inputDigit();
          destTrack = (myInput.digit[TENS] * 10) + myInput.digit[UNITS];
          if (typeTurntable != TT_F6915) {
            if (destTrack > 24) {
              clearInputDigit();
              destTrack = 0;
            }
          }
          updateOLED = true;
        }
        else {
          switch (keyValue) {
            case K_NUM1:                                    // Spoke Left
              myPosTurnout = RECTO;
              if (typeTurntable != TT_F6915) {
                myTurnout = baseAdrTurntable + baseOffset + 2;
                mueveDesvio();
              }
              showExtra();
              break;
            case K_NUM2:                                    // Turn 180º
              if (typeTurntable != TT_F6915) {
                myPosTurnout = RECTO;
                myTurnout = baseAdrTurntable + baseOffset + 1;
              }
              else {
                myTurnout = baseAdrTurntable + baseOffset;
              }
              mueveDesvio();
              showExtra();
              break;
            case K_NUM3:                                    // Spoke Right
              myPosTurnout = DESVIADO;
              if (typeTurntable != TT_F6915) {
                myTurnout = baseAdrTurntable + baseOffset + 2;
                mueveDesvio();
              }
              showExtra();
              break;
          }
        }
      }
      break;
    case SCR_CV:
      if (keyValue == K_MENU) {
        updateOLED = true;
        if (enterCVdata) {
          clearInputDigit();
          updateAllOLED = true;
          enterCVdata = false;
        }
        else {
          showPrgMenu();
        }
      }
      if (keyValue == K_ENTER) {
        if (enterCVdata && (CVdata > 255))                  // no escribir valores erroneos, volver a leer CV
          enterCVdata = false;
        if (enterCVdata) {                                  // Write CV
#ifdef USE_LOCONET
          if (modeProg) {
            enterCVdata = false;
            updateAllOLED = true;
          }
          else {
            showWait();
          }
#endif
#if defined(USE_Z21) || defined(USE_XPRESSNET)
          if (modeProg) {
            enterCVdata = false;
            //clearInputDigit();
            updateAllOLED = true;
            //updateOLED = true;
          }
          else {
            showWait();
          }
#endif
#ifdef USE_ECOS
          showWait();
#endif
          writeCV(CVaddress, CVdata, PRG_CV);
          updateOLED = true;
        }
        else {
          if (CVaddress > 0) {                              // Read CV
            if (!modeProg) {                                // Program track
              showWait();
              readCV(CVaddress, PRG_CV);
            }
            else {
              CVdata = 0x0600;
            }
            enterCVdata = true;
            //clearInputDigit();
            updateAllOLED = true;
            updateOLED = true;
          }
        }
        clearInputDigit();
      }
      if (isTeclaNumerica()) {
        inputDigit();
        if (enterCVdata) {
          myInput.digit[THOUSANDS] = 0;
          if (modeBit) {
            maxFnc = keyValue - K_NUM0;
            if (maxFnc < 8) {
              CVdata &= 0x00FF;
              CVdata ^= bit(maxFnc);
            }
          }
          else {
            CVdata = maxInputValue(255);
          }
        }
        else {
#ifdef USE_LOCONET
          CVmax = 1024;
#endif
#ifdef USE_XPRESSNET
          if (isRecentMM()) {                               // Multimaus v1.03
            CVmax = 1024;
          }
          else {
            if (xnetVersion > 0x35)                         // Otros:
              CVmax = 1024;                                 // Xpressnet v3.6
            else
              CVmax = 256;                                  // Xpressnet v3.0
          }
#endif
#ifdef USE_Z21
          CVmax = 1024;
#endif
#ifdef USE_ECOS
          CVmax = 1024;
#endif
          CVaddress = maxInputValue(CVmax);
        }
        updateOLED = true;
      }
      break;
    case SCR_DIRCV:
      if (keyValue == K_MENU)
        showPrgMenu();
      if (isTeclaNumerica()) {
        inputDigit();
        decoAddress = calcInputValue();
      }
      updateOLED = true;
      if (keyValue == K_ENTER) {
        if (decoAddress > 0) {
          showWait();
          if (decoAddress >
#ifdef USE_LOCONET
              127
#endif
#ifdef USE_XPRESSNET
              99
#endif
#ifdef USE_Z21
              shortAddress
#endif
#ifdef USE_ECOS
              127
#endif
             )
          {
            CVaddress = 17;
            writeCV(CVaddress, highByte(decoAddress) | 0xC0, PRG_WR_CV17);
          }
          else {
            CVaddress = 1;
            writeCV(CVaddress, decoAddress & 0x7F, PRG_WR_CV1);
          }
        }
      }
      break;
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
    case SCR_SSID:
      changeCfgOption = false;
      isKeyMenu();
      if (keyValue == K_ENTER) {
        saveSSID(scrSSID);
        enterCfgWiFi(CFG_WIFI_PWD);
      }
      if (keyValue == K_C) {
        if (scrSSID < networks) {
          scrSSID++;
          encoderValue = scrSSID;
        }
      }
      if (keyValue == K_D) {
        if (scrSSID > 0) {
          scrSSID--;
          encoderValue = scrSSID;
        }
      }
      break;
    case SCR_PASSWORD:
      changeCfgOption = false;
      isKeyMenu();
      if (keyValue == K_ENTER) {
        savePassword();
        showMenu();
        updateOLED = true;
      }
      checkKeyUpC ();
      checkKeyDownD ();
      break;
    case SCR_IP_ADDR:
      changeCfgOption = false;
      isKeyMenu();
      if (keyValue == K_ENTER) {
        saveIP();
        showMenu();
        updateOLED = true;
      }
      checkKeyUpC ();
      checkKeyDownD ();
      break;
#if defined(USE_LNWIFI)
    case SCR_PORT:
      changeCfgOption = false;
      isKeyMenu();
      if (keyValue == K_ENTER) {
        wifiSetting.port = scrPort;
        saveServer();
        showMenu();
        updateOLED = true;
      }
      if (isTeclaNumerica()) {
        input5Digit();
        scrPort = calc5InputValue();
        updateOLED = true;
      }
      break;
#endif
#endif
#ifdef GAME_EXTRA
    case SCR_GAME:
      isKeyMenu();
      break;
#endif
    case SCR_SHUTTLE:
      if (shuttleConfig) {
        if (keyValue == K_MENU) {
          shuttleConfig = false;
        }
        if (keyValue == K_ENTER) {
          switch (optShuttle) {
            case TIME_SHUTT:
              updateEEPROM(EE_TIME, Shuttle.time);
              break;
            case CONTACT_A:
              updateEEPROM(EE_CONT_A, Shuttle.inputA);
#ifdef USE_LOCONET
              updateEEPROM(EE_MOD_A, Shuttle.moduleA);
#endif
              break;
            case CONTACT_B:
              updateEEPROM(EE_CONT_B, Shuttle.inputB);
#ifdef USE_LOCONET
              updateEEPROM(EE_MOD_B, Shuttle.moduleB);
#endif
              break;
#if defined(USE_XPRESSNET) || defined(USE_Z21)|| defined(USE_ECOS)
            case MODULE_A:
              updateEEPROM(EE_MOD_A, Shuttle.moduleA);
              break;
            case MODULE_B:
              updateEEPROM(EE_MOD_B, Shuttle.moduleB);
              break;
#endif
          }
          clearInputDigit();
          optShuttle++;
          if (optShuttle > TIME_SHUTT)
            optShuttle = SET_SHUTT;
        }
        if (isTeclaNumerica()) {
          inputDigit();
          Shuttle.statusA = 0;
          Shuttle.statusB = 0;
          switch (optShuttle) {
            case SET_SHUTT:
              if (keyValue == K_NUM1)
                activeShuttle = !activeShuttle;
              if (activeShuttle) {
                suttleCurrentState = TRAVELLING;
#ifdef USE_ECOS
                bitSet(mySteps, 3);                         // force control
#endif
              }
              else
                suttleCurrentState = DEACTIVATED;
              break;
            case TIME_SHUTT:
              Shuttle.time = maxInputValue(255);
              break;
#ifdef USE_LOCONET
            case CONTACT_A:
              value =  maxInputValue(2048);                 // 1..2048
              Shuttle.moduleA = highByte(value);
              Shuttle.inputA = lowByte(value);
              break;
            case CONTACT_B:
              value =  maxInputValue(2048);
              Shuttle.moduleB = highByte(value);
              Shuttle.inputB = lowByte(value);
              break;
#endif
#ifdef USE_XPRESSNET
            case CONTACT_A:
              Shuttle.inputA = maxInputValue(8);            // 1..8
              break;
            case CONTACT_B:
              Shuttle.inputB = maxInputValue(8);
              break;
            case MODULE_A:
              Shuttle.moduleA = maxInputValue(128);         // 1..128
              break;
            case MODULE_B:
              Shuttle.moduleB = maxInputValue(128);
              break;
#endif
#ifdef USE_Z21
            case CONTACT_A:
              Shuttle.inputA = maxInputValue(8);            // 1..8
              break;
            case CONTACT_B:
              Shuttle.inputB = maxInputValue(8);
              break;
            case MODULE_A:
              Shuttle.moduleA = maxInputValue(20);          // 1..20
              break;
            case MODULE_B:
              Shuttle.moduleB = maxInputValue(20);
              break;
#endif
#ifdef USE_ECOS
            case CONTACT_A:
              Shuttle.inputA = maxInputValue(16);            // 1..16
              break;
            case CONTACT_B:
              Shuttle.inputB = maxInputValue(16);
              break;
            case MODULE_A:
              Shuttle.moduleA = maxInputValue(32);          // 1..32
              break;
            case MODULE_B:
              Shuttle.moduleB = maxInputValue(32);
              break;
#endif
          }
        }
      }
      else {
        isKeyMenu();
        if (keyValue == K_ENTER) {
          optShuttle = SET_SHUTT;
          shuttleConfig = true;
        }
      }
      updateOLED = true;
      break;
#ifdef USE_PHONE
    case SCR_PHONE:
      if (isKeyMenu()) {
        switch (phonePhase) {
          case WAIT_ANSWER:
            //phoneCallEnd();                                                   // force end of call
            lnetRequestSwitch(callingStation , 1, DESVIADO - DESVIADO);   // hung other phone. RED
            lnetRequestSwitch(callingStation , 0, DESVIADO - DESVIADO);
          case PHONE_BUSY:
          case WAIT_USER:
            lnetRequestSwitch(myStation , 1, DESVIADO - DESVIADO);        // hung my phone. RED
            lnetRequestSwitch(myStation , 0, DESVIADO - DESVIADO);
            break;
        }
        phoneCallEnd();
      }
      if (keyValue == K_ENTER) {
        if (phonePhase ==  WAIT_USER) {
          showWait();
          lnetRequestSwitch(originateStation, 1, DESVIADO - DESVIADO);  // accept transfer. RED
          lnetRequestSwitch(originateStation, 0, DESVIADO - DESVIADO);
          phonePhase = WAIT_TRAIN;
        }
      }
      break;
    case SCR_STATION:
      isKeyMenu();
      if (keyValue == K_ENTER)
        callToStation();
      checkKeyUpD ();
      checkKeyDownC ();
      break;
    case SCR_PHONEBOOK:
      isKeyMenu();
      if (keyValue == K_ENTER)
        enterPhoneNumber();
      break;
    case SCR_PHONE_NUM:
      isKeyMenu();
      if (keyValue == K_ENTER) {
        savePhoneNumber();
      }
      checkKeyUpC ();
      checkKeyDownD ();
      break;
#endif
#ifdef USE_LNCV
    case SCR_LNCV:
      if (keyValue == K_MENU) {
        if (optLNCV == LNCV_VAL) {
          clearInputDigit();
          optLNCV = LNCV_ADR;
        }
        else {
          //showMenu();
          showPrgMenu();
          numLNCV = 0;
          valLNCV = modNum;
          sendLNCV(LNCV_REQID_CFGREQUEST, LNCV_FLAG_PROFF);
        }
        updateOLED = true;
      }
      if (keyValue == K_ENTER) {
        switch (optLNCV) {
          case LNCV_ART:
            if (artNum == 65535) {
              numLNCV = 0;
              valLNCV = 65535;
              sendLNCV(LNCV_REQID_CFGREQUEST, 0);
              timeoutLNCV();      // wait a little for response
            }
            else {
              optLNCV = LNCV_MOD;
              clearInputDigit();
              updateOLED = true;
            }
            break;
          case LNCV_MOD:
            numLNCV = 0;
            valLNCV = modNum;
            sendLNCV (LNCV_REQID_CFGREQUEST, LNCV_FLAG_PRON);
            optLNCV = LNCV_ADR;
            clearInputDigit();
            timeoutLNCV();      // wait a little for response
            break;
          case LNCV_ADR:
            optLNCV = LNCV_VAL;
            sendLNCV (LNCV_REQID_CFGREQUEST, 0);
            clearInputDigit();
            timeoutLNCV();      // wait a little for response
            break;
          case LNCV_VAL:
            sendLNCV (LNCV_REQID_CFGWRITE, 0);
            clearInputDigit();
            timeoutLNCV();      // wait a little for response
            break;
        }
      }
      if (isTeclaNumerica()) {
        unsigned int val;
        input5Digit();
        val = calc5InputValue();
        switch (optLNCV) {
          case LNCV_ART:
            artNum = val;
            break;
          case LNCV_MOD:
            modNum = val;
            break;
          case LNCV_ADR:
            numLNCV = val;
            break;
          case LNCV_VAL:
            valLNCV = val;
            break;
        }
        updateOLED = true;
      }
#endif
#ifdef USE_AUTOMATION
    case SCR_AUTO_SELECT:
      isKeyMenu();
      if (keyValue == K_NUM1) {                           // play/stop sequence
        pos = getFileStart(encoderValue);
        if (findAutomation(pos) == MAX_AUTO_SEQ)
          startAutomation(pos);                           // play
        else
          stopAutomation(pos);                            // stop
        updateOLED = true;
      }
      if (keyValue == K_NUM2) {                           // add sequence
        isNewFile = true;
        scrName[0] = 0;
        enterChangeName();
      }
      if (keyValue == K_NUM3) {                           // edit name sequence
        if (fileCount > 0) {
          isNewFile = false;
          fileOLED = encoderValue;
          getFileName(fileOLED, scrName);
          enterChangeName();
        }
      }
      checkKeyUpC ();
      checkKeyDownD ();
      break;
    case SCR_AUTO_NAME:
      isKeyMenu();
      if ((keyValue == K_ENTER) || (keyValue == K_NUM3)) {
        if (scrNameLng > 0) {                             // valid name
          if (isNewFile) {
            newAutoSequence(scrName);                     // create a sequence with new name
          }
          else {
            loadEditBuf(getFileStart(fileOLED));          // load sequence & rename it
            renameSequence(scrName);
          }
          setPosEdit(0);
          editingSequence = true;
          encoderValue = 0;
          encoderMax = getOpcodeCount() - 1;
          scrOLED = SCR_AUTO_EDIT;
          updateAllOLED = true;
          updateOLED = true;
#ifdef DEBUG
          showEditBuf();
#endif
        }
      }
      if (keyValue == K_NUM1) {                           // delete file
        if (! isNewFile) {
          scrOLED = SCR_AUTO_DELETE;
          updateOLED = true;
        }
      }
      checkKeyUpC ();
      checkKeyDownD ();
      break;
    case SCR_AUTO_DELETE:
      isKeyMenu();
      if (keyValue == K_NUM2) {                           // delete
        if (editingSequence) {
          deleteBufEdit(editPos);
          deleteBufEdit(editPos);
          encoderMax = getOpcodeCount() - 1;
          scrOLED = SCR_AUTO_EDIT;
          updateAllOLED = true;
          updateOLED = true;
#ifdef DEBUG
          showEditBuf();
#endif
        }
        else {
          deleteFile(getFileStart(fileOLED));             // delete file
          fileOLED = 0;
          fileCount = getFileCount();
          optOLED = OPT_AUTOMATION;
          enterMenuOption();
        }
      }
      if (keyValue == K_NUM3) {                           // cancel
        if (editingSequence)
          scrOLED = SCR_AUTO_EDIT;
        else
          scrOLED = SCR_AUTO_NAME;
        updateAllOLED = true;
        updateOLED = true;
      }
      break;
    case SCR_AUTO_EDIT:
      if (editingOpcode) {
        switch (keyValue) {
          case K_MENU:
            bufferEdit[editPos] = oldOpcode;
            bufferEdit[editPos + 1] = oldParam;
          case K_ENTER:
            encoderMax = getOpcodeCount() - 1;
            encoderValue = editingPosition;
            editingOpcode = false;
            editingJump = false;
#ifdef USE_ECOS
            editingLoco = false;
#endif
            updateOLED = true;
            break;
          default:
            if (isTeclaNumerica()) {                      // change parameter
              inputDigit();
              enterOpcodeParameter();
              updateOLED = true;
            }
            break;
        }
      }
      else {
        isKeyMenu();
        if (keyValue == K_NUM3) {
          if (isOpcodeEnd(editPos)) {                     // End opcode, save file
            if (isNewFile)
              pos = getNextFreeSector(FREE_SECTOR);       // find space in FAT for new file
            else
              pos = getFileStart(fileOLED);               // get start of edited file
            saveEditBuf(pos);                             // save
            fileOLED = 0;
            fileCount = getFileCount();
            optOLED = OPT_AUTOMATION;
            enterMenuOption();
          }
          else {                                          // other opcode, edit
            enterEditOpcode();
          }
        }
        if (keyValue == K_NUM1) {                         // delete
          if (! isOpcodeEnd(editPos)) {
            scrOLED = SCR_AUTO_DELETE;
            updateOLED = true;
          }
        }
        if (keyValue == K_NUM2) {                         // add
          if (!isOpcodeEnd(editPos)) {
            editPos += 2;
            encoderValue++;
          }
          insertBufEdit(editPos, lowByte(defaultOpcode[0]));
          insertBufEdit(editPos, highByte(defaultOpcode[0]));
          enterEditOpcode();
#ifdef DEBUG
          showEditBuf();
          Serial.println(editPos);
#endif
        }
      }
      break;
#endif
#ifdef USE_SET_TIME
    case SCR_TIME:
      isKeyMenu();
      if (keyValue == K_ENTER) {
        setTime(scrHour, scrMin, scrRate);
        showMenu();
        updateOLED = true;
      }
      break;
#endif
  }
}


#ifdef USE_AUTOMATION
void enterChangeName() {
  scrNameLng = strlen(scrName);
  encoderMax = 95;
  encoderValue = 0x21;
  scrOLED = SCR_AUTO_NAME;
  updateAllOLED = true;
  updateOLED = true;
}


void enterEditOpcode() {
  oldOpcode = bufferEdit[editPos];
  oldParam = bufferEdit[editPos + 1];
  editingOpcode = true;
  editingPosition = encoderValue;
  encoderValue = 0;
  encoderMax = 14;
  clearInputDigit();
  updateOLED = true;
}


void enterOpcodeParameter() {
  byte opcode, param, n;
  unsigned int value;
  opcode = bufferEdit[editPos];
  param = bufferEdit[editPos + 1];
  value = calcInputValue();
  switch (opcode & OPC_AUTO_MASK) {
#if defined(USE_LOCONET) || defined(USE_XPRESSNET) || defined(USE_Z21)
    case OPC_AUTO_LOCO_SEL:
    case OPC_AUTO_LOCO_SEL4:
    case OPC_AUTO_LOCO_SEL8:
      param = lowByte(value);                             // loco
      opcode = OPC_AUTO_LOCO_SEL | highByte(value);
      break;
#endif
    case OPC_AUTO_LOCO_OPT:
      if (bitRead(param, 7)) {
        if (!bitRead(param, 6)) {
          value = maxInputValue(28);
          param &= 0xE0;
          param |= value;
        }
      }
      else {
        param = maxInputValue(100);
      }
      break;
    case OPC_AUTO_DELAY:
      value = maxInputValue(4000);
      param = lowByte(value);                             // time
      opcode = OPC_AUTO_DELAY | highByte(value);
      break;
    case OPC_AUTO_ACC:
      value = maxInputValue(2048);
      if (value > 0) {
        value--;
        opcode &= 0xF8;                                     // accessory
        opcode |= highByte(value);
        param = lowByte(value);
      }
      break;
    case OPC_AUTO_FBK:                                      // feedback
#ifdef USE_LOCONET
      value = maxInputValue(2048);
      if (value > 0) {
        value--;
        opcode &= 0xF8;                                     // accessory
        opcode |= highByte(value);
        param = lowByte(value);
      }
      break;
#endif
#ifdef USE_XPRESSNET
      if (myInput.digit[UNITS] > 8)
        value = (myInput.digit[CENTS] * 100) + (myInput.digit[TENS] * 10) + myInput.digit[UNITS] - 1;
      else
        value = (myInput.digit[THOUSANDS] * 100) + (myInput.digit[CENTS] * 10) + myInput.digit[TENS] - 1;
      if (value > 127) {
        value = 0;
        myInput.packed &= 0x000F;
      }
      param = value;
      value = myInput.digit[UNITS] - 1;
      if (value > 7)
        value = 0;
      opcode &= 0xF8;
      opcode |= value;
#endif
#ifdef USE_Z21
      if (myInput.digit[UNITS] > 8)
        value = (myInput.digit[TENS] * 10) + myInput.digit[UNITS] - 1;
      else
        value = (myInput.digit[CENTS] * 10) + myInput.digit[TENS] - 1;
      if (value > 19) {
        value = 0;
        myInput.packed &= 0x000F;
      }
      param = value;
      value = myInput.digit[UNITS] - 1;
      if (value > 7)
        value = 0;
      opcode &= 0xF8;
      opcode |= value;
#endif
#ifdef USE_ECOS
      if (myInput.digit[TENS] > 1) {
        value = myInput.digit[UNITS] - 1;
        if (value > 15) {
          value = 0;
        }
        param = value;
        value = (myInput.digit[CENTS] * 10) + myInput.digit[TENS]  - 1;
        if (value > 31)
          value = 0;
      }
      else {
        value = (myInput.digit[TENS] * 10) + myInput.digit[UNITS] - 1;
        if (value > 15) {
          value = 0;
        }
        param = value;
        value = (myInput.digit[THOUSANDS] * 10) + myInput.digit[CENTS]  - 1;
        if (value > 31)
          value = 0;
      }
      opcode &= 0xF8;
      if (value > 15)
        opcode |= 0x01;
      param |= ((value & 0x0F) << 4);
#endif
      break;
      //case OPC_AUTO_JUMP:
      //  break;
  }
  bufferEdit[editPos] = opcode;
  bufferEdit[editPos + 1] = param;
}


#endif

void checkKeyUpC () {
  if (keyValue == K_C) {
    if (encoderValue < encoderMax)
      encoderValue++;
    updateOLED = true;
  }
}

void checkKeyDownD () {
  if (keyValue == K_D) {
    if (encoderValue != 0)
      encoderValue--;
    updateOLED = true;
  }
}

void checkKeyUpD () {
  if (keyValue == K_D) {
    if (encoderValue < encoderMax)
      encoderValue++;
    updateOLED = true;
  }
}

void checkKeyDownC () {
  if (keyValue == K_C) {
    if (encoderValue != 0)
      encoderValue--;
    updateOLED = true;
  }
}

/*
  void checkKeyUpKeyDown () {
  if (keyValue == K_C) {
  if (encoderValue < encoderMax)
  encoderValue++;
  updateOLED = true;
  }
  if (keyValue == K_D) {
  if (encoderValue != 0)
  encoderValue--;
  updateOLED = true;
  return true;
  }
  }
*/

bool isTeclaNumerica() {                                    // comprueba si la tecla es un numero
  if ((keyValue >= K_NUM0) && (keyValue <= K_NUM9))
    return true;
  else
    return false;
}


bool isKeyMenu() {                                          // comprueba si es la tecla MENU
  if (keyValue == K_MENU) {
    showMenu();
    updateOLED = true;
    return true;
  }
  return false;
}


#ifdef USE_LNCV
void timeoutLNCV() {
  timeOLED = millis();
  timeoutOLED = 2000;      // wait a little for response
}
#endif


void showExtra() {
  updateExtraOLED = true;
  updateOLED = true;
}
