/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/


#ifdef USE_ECOS
////////////////////////////////////////////////////////////
// ***** WIFI SOPORTE *****
////////////////////////////////////////////////////////////

void beginECoS() {
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
    if (!ECoS.connect(wifiSetting.CS_IP, ECoSPort)) {
      DEBUG_MSG("Connection to ECoS failed");
      enterCfgWiFi(CFG_WIFI_IP);
    }
    else {
      ECoS.setNoDelay(true);
      requestViews();
      requestLocoList();
    }
  }
}



////////////////////////////////////////////////////////////
// ***** ECOS SOPORTE *****
////////////////////////////////////////////////////////////

void sendMsgECOS (char *buf) {
  bool recvAnswer;
  ECoS.write(buf);
#ifdef DEBUG
  Serial.print(F("Send: "));
  Serial.println(buf);
#endif
  timeout = millis();
  recvAnswer = false;
  while ((millis() - timeout < 50) && (!recvAnswer))                      // wait answer for 50ms
    recvAnswer = ECoSProcess();
}


void requestViews () {
  snprintf(cmd, 64, "get(%d, info)\n", ID_ECOS);                          // ECoS info
  sendMsgECOS(cmd);
  snprintf(cmd, 64, "request(%d, view)\n", ID_ECOS);                      // ECoS manager (power)
  sendMsgECOS(cmd);
  snprintf(cmd, 64, "request(%d, view)\n", ID_LOKMANAGER);                // Lok manager
  sendMsgECOS(cmd);
  //snprintf(cmd, 64, "request(%d, view)\n", ID_S88MANAGER);                // S88 manager
  //sendMsgECOS(cmd);
  snprintf(cmd, 64, "request(%d, view)\n", ID_S88FEEDBACK);               // S88 feedback
  sendMsgECOS(cmd);
  //snprintf(cmd, 64, "request(%d, view)\n", ID_SWMANAGER);                 // Switch manager
  //sendMsgECOS(cmd);
  snprintf(cmd, 64, "get(%d, status)\n", ID_ECOS);                        // Power status
  sendMsgECOS(cmd);
  requestViewS88();                                                       // S88 modules (shuttle)
}


void requestViewS88() {
  int id;
  id = Shuttle.moduleA + 99;
  if (id != ID_S88FEEDBACK) {                                             // don't repeat module views
    snprintf(cmd, 64, "request(%d, view)\n", id);                         // S88 feedback A
    sendMsgECOS(cmd);
  }
  if (Shuttle.moduleA != Shuttle.moduleB) {
    id = Shuttle.moduleB + 99;
    if (id != ID_S88FEEDBACK) {
      snprintf(cmd, 64, "request(%d, view)\n", id);                       // S88 feedback B
      sendMsgECOS(cmd);
    }
  }
}


void clearRosterList() {
  int pos;
  for (pos = 0; pos < LOCOS_IN_STACK; pos++) {                            // clear roster list
    strlcpy(rosterList[pos].locoName, "", NAME_LNG);
    rosterList[pos].locoAddress = 0;
    rosterList[pos].id = 0;
  }
}

void requestLocoList() {
  snprintf(cmd, 64, "queryObjects(%d, addr, name)\n", ID_LOKMANAGER);     // only address and name, protocol not needed at the moment
  sendMsgECOS(cmd);
}

void mueveDesvio() {
  char pos;
  pos = (myPosTurnout - DESVIADO) ? 'g' : 'r';
  snprintf(cmd, 64, "set(%d, switch[%d%c])\n", ID_SWMANAGER, myTurnout, pos);      // Directly setting a port.
  sendMsgECOS(cmd);
  accEnviado = true;
}


void readCV (unsigned int adr, byte stepPrg) {
  if (!modeProg) {                                                        // Read only in Direct mode
    snprintf(cmd, 64, "request(%d, view)\n", ID_PRGMANAGER);              // Programming manager
    sendMsgECOS(cmd);
    requestedCV = true;
    snprintf(cmd, 64, "set(%d, mode[readdccdirect], cv[%d])\n", ID_PRGMANAGER, adr);
    sendMsgECOS(cmd);
  }
  progStepCV = stepPrg;
}


void writeCV (unsigned int adr, unsigned int data, byte stepPrg) {
  if (modeProg) {
    snprintf(cmd, 64, "request(%d, view)\n", ID_POMMANAGER);              // PoM Programming manager
    sendMsgECOS(cmd);
    snprintf(cmd, 64, "set(%d, mode[writedccpomloco], addr[%d], cv[%d,%d])\n", ID_POMMANAGER, myLoco.address, adr, data); // Operations Mode Programming byte mode write request
  }
  else {
    snprintf(cmd, 64, "request(%d,view)\n", ID_PRGMANAGER);               // Programming manager
    sendMsgECOS(cmd);
    snprintf(cmd, 64, "set(%d, mode[writedccdirect], cv[%d,%d])\n", ID_PRGMANAGER, adr, data);
  }
  requestedCV = true;
  sendMsgECOS(cmd);
  progStepCV = stepPrg;
  DEBUG_MSG("Write CV%d = %d", adr, data);
}


void exitProgramming() {
  if (requestedCV) {
    snprintf(cmd, 64, "release(%d,view)\n", ID_PRGMANAGER);                 // Programming manager
    sendMsgECOS(cmd);
    snprintf(cmd, 64, "release(%d, view)\n", ID_POMMANAGER);                // PoM Programming manager
    sendMsgECOS(cmd);
    requestedCV = false;
  }
}

void releaseLoco() {
  if (myLocoID) {
    snprintf(cmd, 64, "release(%d, control)\n", myLocoID);                // release control
    sendMsgECOS(cmd);
    snprintf(cmd, 64, "release(%d, view)\n", myLocoID);                   // release updates
    sendMsgECOS(cmd);
    myLocoID = 0;
  }
}


void infoLocomotora (unsigned int ID) {
  byte n;
  if (ID > 999) {
    bitClear(mySteps, 3);
    snprintf(cmd, 64, "request(%d, view, control[events])\n", ID);                // View Locomotive data updates
    sendMsgECOS(cmd);
    snprintf(cmd, 64, "get(%d, speed, dir, speedstep)\n", ID);            // Speed, dir & steps
    sendMsgECOS(cmd);
    for (n = 0; n < 29; n++) {
      snprintf(cmd, 64, "get(%d, func[%d])\n", ID, n);                    // Functions
      sendMsgECOS(cmd);
    }
  }
}

void checkControlLoco() {
  if (bitRead(mySteps, 3)) {
    bitClear(mySteps, 3);
    snprintf(cmd, 64, "request(%d, control[events], force)\n", myLocoID);         // force control
    sendMsgECOS(cmd);
  }
}

void changeDirection() {
  checkControlLoco();
  snprintf(cmd, 64, "set(%d, dir[%d])\n", myLocoID, (myDir & 0x80) ? 0 : 1);        // direction (1=rückwärts).
  sendMsgECOS(cmd);
}

void locoOperationSpeed() {
  checkControlLoco();
  snprintf(cmd, 64, "set(%d, speed[%d])\n", myLocoID, mySpeed);           // Speed
  sendMsgECOS(cmd);
}


byte getCurrentStep() {
  return (mySpeedStep);
}


void funcOperations (byte fnc) {
  byte stat;
  checkControlLoco();
  stat = (bitRead(myFunc.Bits, fnc)) ? 1 : 0;
  snprintf(cmd, 64, "set(%d, func[%d,%d])\n", myLocoID, fnc, stat);       // Function
  sendMsgECOS(cmd);
}


void infoDesvio (unsigned int FAdr) {
  snprintf(cmd, 64, "get(%d, switch[%d%c])\n", ID_SWMANAGER, FAdr, 'r');  // Directly read a port.
  sendMsgECOS(cmd);
  snprintf(cmd, 64, "get(%d, switch[%d%c])\n", ID_SWMANAGER, FAdr, 'g');
  sendMsgECOS(cmd);
}


void resumeOperations () {
  snprintf(cmd, 64, "set(%d, go)\n", ID_ECOS);                            // Track on
  sendMsgECOS(cmd);
}

void emergencyOff() {
  snprintf(cmd, 64, "set(%d, stop)\n", ID_ECOS);                          // Track off
  sendMsgECOS(cmd);
}

byte findLocoID (unsigned int id) {
  byte pos, n;
  pos = LOCOS_IN_STACK;
  if (id > 0) {
    for (n = 0; n < LOCOS_IN_STACK; n++)                              // search ID in roster list
      if (rosterList[n].id == id)
        pos = n;
  }
  return pos;
}


void getInfoS88 (unsigned int id) {
  snprintf(cmd, 64, "get(%d, state)\n", id);                            // Ask current state of the S88 module.
  sendMsgECOS(cmd);
}


////////////////////////////////////////////////////////////
// ***** ECOS DECODE *****
////////////////////////////////////////////////////////////

bool ECoSProcess() {
  char chr;
  bool rcvMsg;
  rcvMsg = false;
  while (ECoS.available()) {
    chr = ECoS.read();
    if (chr == '\n') {
      if (inputLength > 0) {
        inputBuffer[inputLength] = 0;
        rcvMsg = ECoSDecode();
      }
      inputLength = 0;
    }
    else {
      inputBuffer[inputLength++] = chr;
      if (inputLength >= BUF_LNG)                                         // line too long, discard
        inputLength = 0;
    }
  }
  yield();
  if (millis() - infoTimer > 1000UL) {                        // Cada segundo
    infoTimer = millis();
    battery = ESP.getVcc ();                                  // Read VCC voltage
    if (battery < LowBattADC)
      lowBATT = true;
  }
  if (progFinished) {                                                     // fin de lectura/programacion CV
    progFinished = false;
    endProg();
  }
  return rcvMsg;
}

//============= Scanning ==================

// Simplified scanning based on C compiler method. https://github.com/DoctorWkt/acwj

char getChar () {                                                         // get a chr form input buffer
  return inputBuffer[posFile++];
}


void putBack(char c) {                                                    // Put back an unwanted character
  putBackChr = c;
  posFile--;                                                              // set reading position back
}


char next() {                                                             // Get the next character from the input stream.
  char c;
  if (putBackChr) {                                                       // Use the character put back if there is one
    c = putBackChr;
    putBackChr = 0;
    posFile++;
  }
  else
    c = getChar();                                                        // Read from input stream
  return c;
}



char skip() {                                                             // Skip past input that we don't need to deal with, i.e. whitespace, newlines. Return the first character we do need to deal with.
  char c;
  c = next();
  while (' ' == c || '\t' == c || '\n' == c || '\r' == c) {
    c = next();
  }
  return (c);
}

void discardLine() {                                                      // ignore rest of the line
  putBackChr = 0;
  inputBuffer[posFile] = 0;
}

int scanInt(char c) {                                                     // Scan and return an integer literal value from the input stream.
  int val;
  val = 0;
  while (isDigit(c)) {                                                    // Convert each character into an int value
    val = val * 10 + (c - '0');
    c = next();
  }
  putBack(c);                                                             // We hit a non-integer character, put it back.
  return val;
}


int scanIdent(int c, char *buf, int lim) {                                // Scan an identifier from the input file and store it in buf[]. Return the identifier's length
  int i = 0;

  while (isalpha(c) || isdigit(c) || (c == '-') || (c == '_')) {          // Allow digits, alpha and underscore -
    if (lim - 1 == i) {                                                   // Error if we hit the identifier length limit, else append to buf[] and get next character
      return (0);
    } else if (i < lim - 1) {
      buf[i++] = c;
    }
    c = next();
  }
  putBack(c);                                                             // We hit a non-valid character, put it back. NUL-terminate the buf[] and return the length
  buf[i] = '\0';
  return (i);
}


int scanStr (char *buf) {                                                 // Scan in a string literal from the input file, and store it in buf[]. Return the length of the string.
  int i, c;
  for (i = 0; i < TEXTLEN - 1; i++) { // Loop while we have enough buffer space
    // Get the next char and append to buf
    // Return when we hit the ending double quote
    if ((c = next()) == '"') {
      buf[i] = 0;
      return (i);
    }
    buf[i] = c;
  }
  // Ran out of buf[] space
  return (0);
}


int ident2HEX (char *buf, int *value) {                                   // convert an idetifier (xNNNN) to hex value
  int val, n;
  char c;
  val = 0;
  n = 1;
  if (buf[0] == 'x') {
    while (buf[n]) {
      c = buf[n++];
      val *= 16;
      if (isDigit(c)) {
        val += (c - '0');
      }
      else {
        c &= 0xDF;
        val += ((c - 'A') + 10);
      }
    }
    *value = val;
    return 1;
  }
  return 0;
}


// Given a word from the input, return the matching keyword token number or 0 if it's not a keyword.
// Switch on the first letter so that we don't have to waste time strcmp()ing against all the keywords.
int keyword(char *s) {
  switch (*s) {
    case 'R':
      if (!strcmp(s, "REPLY"))
        return (T_REPLY);
      break;
    case 'E':
      if (!strcmp(s, "END"))
        return (T_END);
      if (!strcmp(s, "EVENT"))
        return (T_EVENT);
      if (!strcmp(s, "ECoS2"))
        return (T_ECOS2);
      if (!strcmp(s, "ECoS"))
        return (T_ECOS);
      break;
    case 'O':
      if (!strcmp(s, "OK"))
        return (T_OK);
      break;
    case 'C':
      if (!strcmp(s, "CONTROL_LOST"))
        return (T_LOST);
      if (!strcmp(s, "CentralStation"))
        return (T_CS1);
      break;
    case 'G':
      if (!strcmp(s, "GO"))
        return (T_GO);
      break;
    case 'S':
      if (!strcmp(s, "STOP"))
        return (T_STOP);
      if (!strcmp(s, "SHUTDOWN"))
        return (T_SHUTDWN);
      break;
    case 'a':
      if (!strcmp(s, "addr"))
        return (T_ADDR);
      break;
    case 'c':
      if (!strcmp(s, "control"))
        return (T_CONTROL);
      if (!strcmp(s, "cv"))
        return (T_CV);
      break;
    case 'd':
      if (!strcmp(s, "dir"))
        return (T_DIR);
      break;
    case 'e':
      if (!strcmp(s, "error"))
        return (T_ERROR);
      break;
    case 'f':
      if (!strcmp(s, "func"))
        return (T_FUNC);
      if (!strcmp(s, "force"))
        return (T_FORCE);
      break;
    case 'g':
      if (!strcmp(s, "get"))
        return (T_GET);
      break;
    case 'm':
      if (!strcmp(s, "msg"))
        return (T_MSG);
      break;
    case 'n':
      if (!strcmp(s, "name"))
        return (T_NAME);
      break;
    case 'o':
      if (!strcmp(s, "ok"))
        return (T_CVOK);
      if (!strcmp(s, "other"))
        return (T_OTHER);
      break;
    case 'p':
      if (!strcmp(s, "protocol"))
        return (T_PROT);
      break;
    case 'q':
      if (!strcmp(s, "queryObjects"))
        return (T_QOBJ);
      break;
    case 'r':
      if (!strcmp(s, "request"))
        return (T_REQ);
      if (!strcmp(s, "release"))
        return (T_RELEASE);
      break;
    case 's':
      if (!strcmp(s, "status2"))
        return (T_STATUS2);
      if (!strcmp(s, "status"))
        return (T_STATUS);
      if (!strcmp(s, "state"))
        return (T_STATE);
      if (!strcmp(s, "speedstep"))
        return (T_STEPS);
      if (!strcmp(s, "speed"))
        return (T_SPEED);
      if (!strcmp(s, "set"))
        return (T_SET);
      if (!strcmp(s, "switch"))
        return (T_SWITCH);
      break;
    case 'v':
      if (!strcmp(s, "view"))
        return (T_VIEW);
      break;

  }
  return (0);
}

//=========== Lexical ===============


int scan(struct token *t) {                                               // Scan and return the next token found in the input. Return 1 if token valid, 0 if no tokens left.
  char c;

  c = skip();                                                             // Skip whitespace
  switch (c) {                                                            // Determine the token based on the input character
    case 0:                                                               // EOF
      return (0);
    case '(':
      t->token = T_LPARENT;
      t->intvalue = posFile;
      break;
    case ')':
      t->token = T_RPARENT;
      t->intvalue = posFile;
      break;
    case '[':
      t->token = T_LBRACKET;
      t->intvalue = posFile;
      break;
    case ']':
      t->token = T_RBRACKET;
      t->intvalue = posFile;
      break;
    case '<':
      t->token = T_START;
      t->intvalue = posFile;
      break;
    case '>':
      t->token = T_ENDB;
      t->intvalue = posFile;
      break;
    case ',':
      t->token = T_COMMA;
      t->intvalue = posFile;
      break;
    case '#':
      t->token = T_NULL;
      t->intvalue = posFile;
      discardLine();
      break;
    case '"':
      scanStr(Text);
      t->token = T_STRLIT;
      break;
    default:
      if (isDigit(c)) {                                                   // If it's a digit, scan the literal integer value in
        t->intvalue = scanInt(c);
        t->token = T_INTLIT;
        break;
      }
      if (isAlpha(c)) {
        scanIdent(c, Text, TEXTLEN);
        tokenType = keyword(Text);
        if (tokenType) {
          t->token = tokenType;
          break;
        }
        t->token = T_IDENT;                                               // Not a recognised keyword, so it must be an identifier
        t->intvalue = Text[0];
        break;
      }
      return (0);
      break;
  }
  //DEBUG_MSG("Token found: %s", tokstr[T.token]);
  return (1);  // We found a token
}

//----------------------------------------------------------

bool ECoSDecode() {
  bool endMsg;
#ifdef DEBUG
  Serial.print(F("Recv: "));
  Serial.println(inputBuffer);
#endif
  posFile = 0;
  putBackChr = 0;
  endMsg = false;
  while (scan(&T)) {
    switch (msgDecodePhase) {
      case MSG_WAIT:                                                      // wait '<' for start of msg
        if (T.token == T_START)
          msgDecodePhase = MSG_START;
        break;
      case MSG_START:
        switch (T.token) {
          case T_REPLY:                                                   // receiving a REPLY
            msgDecodePhase = MSG_REPLY;
            break;
          case T_EVENT:                                                   // receiving an EVENT
            msgDecodePhase = MSG_EVENT;
            break;
          case T_END:                                                     // receiving END
            msgDecodePhase = MSG_END;
            break;
          default:
            msgDecodePhase = MSG_WAIT;                                    // others not supported
            break;
        }
        idManager = 0;
        break;
      case MSG_END:
        if (T.token == T_INTLIT) {                                        // get the error code
          errCode = T.intvalue;
          discardLine();
          msgDecodePhase = MSG_WAIT;                                      // discard rest of message
          switch (errCode) {
            case 25:                                                      // NERROR_NOCONTROL
              bitSet(mySteps, 3);
              break;
            case 15:                                                      // NERROR_UNKNOWNID
              if (requestedCV) {
                if (progStepCV != PRG_IDLE) {
                  CVdata = 0x0600;
                  progFinished = true;
                }
              }
              break;
          }
          DEBUG_MSG("END Error code: %d", errCode);
        }
        endMsg = true;
        break;
      case MSG_EVENT:
        if (T.token == T_INTLIT)                                          // get the event manager
          idManager = T.intvalue;
        discardLine();
        msgDecodePhase = MSG_EVENTBODY;                                   // discard rest of line
        DEBUG_MSG("Manager ID: %d", idManager);
        if ((idManager == ID_PRGMANAGER) || (idManager == ID_POMMANAGER)) {
          lastNumValue = 0x0600;
        }
        break;
      case MSG_REPLY:                                                     // <REPLY get(1,status)>
        idCommand = T.token;                                              // get ...
        scan(&T);                                                         // (
        scan(&T);                                                         // id manager
        if (T.token == T_INTLIT) {
          idManager = T.intvalue;
          DEBUG_MSG("Reply: %s id: %d", tokstr[idCommand], idManager);
          if ((idManager == ID_LOKMANAGER) && (idCommand == T_QOBJ)) {    // list of loks
            numLoks = 0;
            clearRosterList();
            DEBUG_MSG("Roster list cleared");
          }
          if ((idManager == ID_SWMANAGER) && (idCommand == T_GET)) {      // info of turnout  <REPLY get(11, switch[12g])>
            //DEBUG_MSG("Switch manager get");
            scan(&T);                                                     // ,
            scan(&T);                                                     // switch
            if (T.token == T_SWITCH) {
              //DEBUG_MSG("Get turnout");
              scan(&T);                                                   // [
              scan(&T);                                                   // 12g
              if (T.token == T_INTLIT)                                    // 12
                scan(&T);
              if (T.token == T_IDENT) {                                   // Text is "g" or could be "DCC12g"
                lastTxtChar = Text[strlen(Text) - 1];
                DEBUG_MSG("Info turnout pos: %s - %c", Text, lastTxtChar);
              }
            }
          }
        }
        discardLine();
        msgDecodePhase = MSG_REPLYBODY;
        break;
      case MSG_EVENTBODY:
        switch (T.token) {
          case T_START:
            msgDecodePhase = MSG_START;                                   // End of body
            break;
          case T_INTLIT:
            switch (idManager) {                                          // id
              case ID_ECOS:                                               // ECoS events
                decodeEventECoS();
                break;
              case ID_PRGMANAGER:                                         // Programming events
              case ID_POMMANAGER:
                decodeEventCV();                                          // decodes CV answer
                break;
              default:
                if (idManager == myLocoID) {                               // current loco events
                  //decodeEventLoco();
                  if (T.intvalue == myLocoID)
                    decodeReplyLoco();
                }
                else {
                  if ((idManager >= ID_S88FEEDBACK) && (idManager < (ID_S88FEEDBACK + 32)))           // S88 events
                    decodeEventS88Feedback();
                  else
                    discardLine();
                }
                break;
            }
            break;
          default:                                                        // other types of start of line not supported
            discardLine();
            break;
        }
        break;
      case MSG_REPLYBODY:
        switch (T.token) {
          case T_START:
            msgDecodePhase = MSG_START;                                   // End of body
            break;
          case T_INTLIT:
            switch (idManager) {                                          // id
              case ID_ECOS:                                               // decodes answer to: get(1,...) / request
                if (idCommand == T_GET) {
                  decodeEventECoS();
                }
                else
                  discardLine();
                break;
              case ID_LOKMANAGER:                                         // decodes answer to:  queryObjects(10,...)
                decodeLokManager();
                break;
              case ID_SWMANAGER:
                if (idCommand == T_GET) {                                 // decodes answer to: get(11,..)
                  decodeSwitchManager();
                }
                else
                  discardLine();
                break;
              default:
                if ((idManager >= ID_S88FEEDBACK) && (idManager < (ID_S88FEEDBACK + 32)))
                  decodeEventS88Feedback();
                if (idManager == myLocoID)
                  decodeReplyLoco();                                      // decodes answer to: get(1xxx,...)  / set..
                else
                  discardLine();
                break;
            }
            break;
          default:                                                        // other types of start of line not supported
            discardLine();
            break;
        }
        break;
    }
  }
  return endMsg;
}



void decodeEventECoS () {
  if (T.intvalue == ID_ECOS) {
    scan(&T);
    switch (T.token) {
      case T_STATUS:                                                        // 1 status[GO]
        scan(&T);                                                           // [
        scan(&T);                                                           // GO / STOP / SHUTDOWN
        if (T.token == T_GO) {
          csStatus = 1;
          if (scrOLED == SCR_ERROR) {                                       // sale de la pantalla de error
            scrOLED = SCR_ENDLOGO;
            updateOLED = true;
          }
          DEBUG_MSG("Power On");
        }
        else {
          csStatus = 0;
          scrOLED = SCR_ERROR;
          errOLED = ERR_OFF;
          updateOLED = true;
          DEBUG_MSG("Power Off");
        }
        break;
      case T_CS1:                                                         // 1 CentralStation
      case T_ECOS:                                                        // 1 ECoS
      case T_ECOS2:                                                       // 1 ECoS2
        typeCmdStation = T.token;
        DEBUG_MSG("CS Type: %s", tokstr[typeCmdStation]);
        break;
    }
    discardLine();
  }
}

void decodeEventCV() {
  while (scan(&T)) {
    switch (T.token) {
      case T_INTLIT:
        lastNumValue = T.intvalue;
        break;
      case T_CVOK:
        CVdata = lastNumValue;
        progFinished = true;
        discardLine();
        break;
      case T_ERROR:
        CVdata = 0x0600;
        progFinished = true;
        discardLine();
        break;
    }
  }
}


void decodeEventS88Feedback() {
  idS88 = T.intvalue - 99;                                                // 100 state[0x400]
  scan(&T);
  if (T.token == T_STATE) {                                               // state
    scan(&T);                                                             // [
    scan(&T);                                                             // 0
    scan(&T);                                                             // hex value: x400
    if (T.token == T_IDENT) {
      if (ident2HEX(Text, &stateS88)) {
        if (Shuttle.moduleA == idS88)                                     // only check shuttle contacts
          Shuttle.statusA = stateS88;
        if (Shuttle.moduleB == idS88)
          Shuttle.statusB = stateS88;
#ifdef USE_AUTOMATION
        for (byte n = 0; n < MAX_AUTO_SEQ; n++) {
          if ((automation[n].opcode & OPC_AUTO_MASK) == OPC_AUTO_FBK) {
            int adr = ((automation[n].opcode & 0x07) << 4) + (automation[n].param >> 4) + 1;
            if (adr == idS88) {
              automation[n].value = stateS88;
              DEBUG_MSG("Auto %d: S88", n);
            }
          }
        }
#endif
      }
      DEBUG_MSG("S88 ID: %d State: %s - %04X", idS88, Text, stateS88);
    }
    else {
      DEBUG_MSG("Error HEX conversion");
    }
  }
}


void decodeReplyLoco() {
  int numFunc;
  scan(&T);
  switch (T.token) {                                                      // <REPLY get(1xxx,...  )>  /  <EVENT 1xxx>
    case T_MSG:                                                           // 1018 msg[CONTROL_LOST]
      scan(&T);
      scan(&T);
      if (T.token == T_LOST) {
        bitSet(mySteps, 3);
        DEBUG_MSG("Lost control of %d", myLocoID);
      }
      break;
    case T_CONTROL:                                                       // 1000 control[other]
      scan(&T);
      scan(&T);
      if (T.token == T_OTHER) {
        bitSet(mySteps, 3);
        DEBUG_MSG("Control %d by other", myLocoID);
      }
      break;
    case T_SPEED:                                                         // 1000 speed[6]
      scan(&T);
      scan(&T);
      if (T.token == T_INTLIT) {
        mySpeed = T.intvalue;
        updateSpeedHID();
        DEBUG_MSG("Speed: %d", T.intvalue);
      }
      break;
    case T_STEPS:                                                         // 1000 speedstep[2]
      scan(&T);
      scan(&T);
      if (T.token == T_INTLIT) {
        mySpeedStep = T.intvalue;
        DEBUG_MSG("Steps: %d", T.intvalue);
      }
      break;
    case T_DIR:                                                           // 1000 dir[0]
      scan(&T);
      scan(&T);
      if (T.token == T_INTLIT) {
        myDir = (T.intvalue > 0) ? 0 : 0x80;
        DEBUG_MSG("Dir: %d", T.intvalue);
      }
      break;
    case T_FUNC:                                                          // 1015 func[0, 0]
      scan(&T);
      scan(&T);
      if (T.token == T_INTLIT) {
        numFunc = T.intvalue;
        scan(&T);
        scan(&T);
        if (T.token == T_INTLIT) {
          if (T.intvalue > 0)
            bitSet(myFunc.Bits, numFunc);
          else
            bitClear(myFunc.Bits, numFunc);
          DEBUG_MSG("Func%d: %d", numFunc, T.intvalue);
        }
      }
      break;
  }
  discardLine();
  if (scrOLED == SCR_SPEED)
    updateOLED = true;
}


void parseLokAddrName(int pos) {
  scan(&T);                                                               // 1003 addr[78] name["W. K"]
  switch (T.token) {
    case T_ADDR:
      scan(&T);
      scan(&T);
      if (T.token == T_INTLIT)
        rosterList[pos].locoAddress = T.intvalue;
      scan(&T);
      DEBUG_MSG("Addr: %d", rosterList[pos].locoAddress);
      break;
    case T_NAME:
      scan(&T);
      scan(&T);
      if (T.token == T_STRLIT)
        snprintf(rosterList[pos].locoName, NAME_LNG, "%s", Text);
      scan(&T);
      DEBUG_MSG("Name: %s", rosterList[pos].locoName);
      break;
  }
}


void decodeLokManager () {
  if (idCommand == T_QOBJ) {                                              // answer to queryObjects(10,
    if (numLoks < LOCOS_IN_STACK) {
      rosterList[numLoks].id = T.intvalue;                                // 1003 addr[78] name["W. K"]
      DEBUG_MSG("ID: %d", rosterList[numLoks].id);
      parseLokAddrName(numLoks);                                          // addr
      parseLokAddrName(numLoks);                                          // name
      pushLoco(rosterList[numLoks].id);
      numLoks++;
    }
    discardLine();
  }
  else
    discardLine();
}


void decodeSwitchManager() {
  scan(&T);
  if (T.token == T_SWITCH) {                                             // 11 switch[0]
    scan(&T);
    scan(&T);
    DEBUG_MSG("Pos %c: %d", lastTxtChar, T.intvalue);
    if ((lastTxtChar == 'r') && (T.intvalue == 1))
      myPosTurnout = DESVIADO;
    if ((lastTxtChar == 'g') && (T.intvalue == 1))
      myPosTurnout = RECTO;
    if (scrOLED == SCR_TURNOUT)
      updateOLED = true;
  }
}




#endif
