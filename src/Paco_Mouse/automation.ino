/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.

      Very basic Disk emulation on EEPROM for PacoMouse
      Simple automation for PacoMouse similar to Märklin CS3
*/

#ifdef USE_AUTOMATION

////////////////////////////////////////////////////////////
// ***** EDIT BUFFER *****
////////////////////////////////////////////////////////////

void insertBufEdit (byte pos, byte value) {                         // insert byte on cursor position in edit buffer
  byte n;
  n = EDIT_BUF_SIZE - 1;
  while (n > pos) {
    bufferEdit[n] = bufferEdit[n - 1];
    n--;
  }
  bufferEdit[pos] = value;
}


void deleteBufEdit(byte pos) {                                      // delete byte at cursor position in edit buffer
  while (pos < (EDIT_BUF_SIZE - 1)) {
    bufferEdit[pos] = bufferEdit[pos + 1];
    pos++;
  }
  bufferEdit[EDIT_BUF_SIZE - 1] = OPC_AUTO_END;
}


void newAutoSequence(char *buf) {                                   // creates empty auto sequence leading by name
  byte len, n;
  len = strlen(buf);
  if (len > AUTO_NAME_LNG)
    len = AUTO_NAME_LNG;
  bufferEdit[0] = OPC_AUTO_NAME + len;
  for (n = 0; n < len; n++)
    bufferEdit[n + 1] = buf[n];
  if (!(len & 0x01)) {                                              // space padded if needed
    len++;
    bufferEdit[len] = ' ';
  }
  bufferEdit[len + 1] = OPC_AUTO_END;
  bufferEdit[len + 2] = OPC_AUTO_END;
}


void getNameSequence(char *buf) {                                   // get name from edit buffer
  byte cnt, n;
  buf[0] = 0;
  cnt = bufferEdit[0];
  if (cnt < 16) {
    for (n = 0; n < cnt; n++) {
      buf[n] = bufferEdit[n + 1];
    }
    buf[cnt] = 0;
  }
}


void renameSequence (char *buf) {                                   // rename
  byte bufLen, editLen, len, n;
  len = strlen(buf);
  if (len > AUTO_NAME_LNG)
    len = AUTO_NAME_LNG;
  bufLen = (len | 0x01);
  editLen = bufferEdit[0] | 0x01;
  if (editLen > 15)                                                 // rename only if name exist
    return;
  while (bufLen != editLen) {
    if (editLen > bufLen) {
      deleteBufEdit(0);
      editLen--;
    }
    else {
      insertBufEdit(0, 0);
      editLen++;
    }
  }
  bufferEdit[0] = OPC_AUTO_NAME + len;
  for (n = 0; n < len; n++)
    bufferEdit[n + 1] = buf[n];
  if (!(len & 0x01)) {                                              // space padded if needed
    bufferEdit[len + 1] = ' ';
  }
}


bool isOpcodeEnd (byte pos) {
  if ((bufferEdit[pos] == OPC_AUTO_END) && (bufferEdit[pos + 1] == OPC_AUTO_END))
    return true;
  else
    return false;
}


void setPosEdit(byte num) {
  byte pos;
  editPos = 0;
  if ((bufferEdit[0] & OPC_AUTO_MASK) == OPC_AUTO_NAME)             // skip name
    editPos += (((bufferEdit[0] & 0x0F) | 0x01) + 1);               // padding
  editPos += (num * 2);
}


byte getOpcodeCount() {                                             // get opcode count in the edit buffer
  byte pos, cnt, data;
  pos = 0;
  cnt = 0;
  while (pos < EDIT_BUF_SIZE) {
    if (isOpcodeEnd(pos)) {
      cnt++;
      pos = EDIT_BUF_SIZE;
    }
    else {
      if ((bufferEdit[pos] & OPC_AUTO_MASK) == OPC_AUTO_NAME)
        pos += (((bufferEdit[pos] & 0x0F) | 0x01) + 1);             // padding
      else {
        cnt++;
        pos += 2;
      }
    }
    //DEBUG_MSG("Pos %d: %d", pos, cnt);
  }
  //DEBUG_MSG("Opc cnt: %d", cnt);
  return cnt;
}


////////////////////////////////////////////////////////////
// ***** EEPROM DISK *****
////////////////////////////////////////////////////////////

byte getFileCount() {                                               // get count of files in the disk
  byte n, cnt;
  cnt = 0;
  for (n = 0; n < FAT_SIZE; n++)
    if (!(EEPROM.read(FAT_INI + n) & 0x80))
      cnt++;                                                        // start of file found
  return cnt;
}


byte getEditSectorCount () {                                        // get count of sectors used by edit buffer
  byte pos, cnt;
  pos = 0;
  cnt = 0;
  while (pos < EDIT_BUF_SIZE) {
    //if ((bufferEdit[pos] == OPC_AUTO_END) && (bufferEdit[pos + 1] == OPC_AUTO_END))
    if (isOpcodeEnd(pos))
      pos = EDIT_BUF_SIZE;
    pos += 2;
    cnt += 2;
  }
  return (((cnt - 1) / SECTOR_SIZE) + 1);
}


byte getFileSectorCount (byte posFAT) {                             // get count of sectors used by file
  byte data, nxtSector, cnt;
  cnt = 0;
  data = EEPROM.read(posFAT + FAT_INI);
  while (data != FREE_SECTOR) {
    nxtSector = data & 0x7F;
    cnt++;
    data = (nxtSector == END_SECTOR) ? FREE_SECTOR : EEPROM.read(nxtSector + FAT_INI);
  }
  return cnt;
}


byte getNextFreeSector(byte posFAT) {                               // search FAT for next free sector
  if (posFAT == FREE_SECTOR)
    posFAT = 0;
  else
    posFAT++;
  for (; posFAT < FAT_SIZE; posFAT++)
    if (EEPROM.read(FAT_INI + posFAT) == FREE_SECTOR)
      return posFAT;                                                // free sector found
  return FREE_SECTOR;                                               // disk full
}


byte getFileStart (byte num) {
  byte pos;
  pos = 0;
  while (pos < FAT_SIZE) {
    if (!(EEPROM.read(FAT_INI + pos) & 0x80)) {                     // start of file found
      if (num == 0)
        return pos;
      num--;
    }
    pos++;
  }
  return FREE_SECTOR;
}

void getFileName(byte num, char *buf) {                             // get name of the file
  unsigned int adr;
  byte posFAT, cnt, n;
  buf[0] = 0;                                                       // file not found. No name
  posFAT = getFileStart(num);
  if (posFAT != FREE_SECTOR) {
    adr = (posFAT * SECTOR_SIZE) + DISK_INI;                        // '0000NNNN 0ccccccc' ['0ccccccc 0ccccccc' ... '0ccccccc 0ccccccc']
    cnt = EEPROM.read(adr++);
    if (cnt < 16) {
      for (n = 0; n < cnt; n++) {
        buf[n] = EEPROM.read(adr++);
      }
      buf[cnt] = 0;
    }
  }
}


void deleteFile(byte posFAT) {                                      // delete file
  unsigned int adr;
  byte data;
#if defined(USE_Z21) || defined(USE_ECOS)
  changedEEPROM = false;
#endif
  data = FREE_SECTOR;
  while (data != END_SECTOR) {
    adr = posFAT + FAT_INI;
    data = EEPROM.read(adr);
    posFAT = data & 0x7F;
#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
    EEPROM.update(adr, FREE_SECTOR);
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
    if (EEPROM.read(adr) != FREE_SECTOR) {
      EEPROM.write (adr, FREE_SECTOR);
      changedEEPROM = true;
    }
#endif
    if (data == FREE_SECTOR)
      data = END_SECTOR;
    else
      data = posFAT;
  }
#if defined(USE_Z21) || defined(USE_ECOS)
  if (changedEEPROM)
    EEPROM.commit();
  DEBUG_MSG("Saving EEPROM")
#endif
}


void loadEditBuf(byte posFAT) {                                     // read all file sectors to edit buffer, posFAT: 0..FAT_SIZE
  unsigned int adr;
  byte pos, cnt, data;
  bool reading;
  reading = true;
  pos = 0;
  while (reading && (pos < EDIT_BUF_SIZE)) {
    adr = (posFAT * SECTOR_SIZE) + DISK_INI;
    cnt = 0;
    while (cnt < SECTOR_SIZE) {
      data = EEPROM.read(adr++);
      bufferEdit[pos++] = data;
      cnt++;
    }
    data = EEPROM.read(posFAT + FAT_INI);
    if (data & 0x7F) {                                          // read next sector
      posFAT = data & 0x7F;
    }
    else {                                                      // EOF
      reading = false;
    }
  }

}


void saveEditBuf(byte posFAT) {                                   // save edit buffer to disk, posFAT: 0..FAT_SIZE
  byte sectors[EDIT_BUF_SECT];
  byte cntSectors, usedSectors, nxtSector;
  byte n, i, nxt, cnt;
  byte pos, data;
  unsigned int adr;
#if defined(USE_Z21) || defined(USE_ECOS)
  changedEEPROM = false;
#endif
  usedSectors = getFileSectorCount(posFAT);
  cntSectors = getEditSectorCount();
  for (n = 0; n < usedSectors; n++) {                             // get current used sectors
    adr = posFAT + FAT_INI;
    nxtSector = EEPROM.read(adr);
    sectors[n] = posFAT;
    posFAT = nxtSector & 0x7F;
  }
  if (usedSectors > cntSectors) {                                 // file is shorter now, delete sectors
    for (n = cntSectors; n < usedSectors; n++) {
      adr = FAT_INI + sectors[n];
#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
      EEPROM.update(adr, FREE_SECTOR);
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
      if (EEPROM.read(adr) != FREE_SECTOR) {
        EEPROM.write (adr, FREE_SECTOR);
        changedEEPROM = true;
      }
#endif
    }
  }
  if (usedSectors < cntSectors) {                                 // file is larger now, add new sectors
    nxtSector = FREE_SECTOR;
    for (n = usedSectors; n < cntSectors; n++) {
      nxtSector = getNextFreeSector(nxtSector);
      sectors[n] = nxtSector;
    }
  }
  pos = 0;                                                        // save file
  cnt = START_FILE;
  for (n = 0; n < cntSectors; n++) {
    nxt = (n == (cntSectors - 1)) ? 0 : sectors[n + 1];           // update FAT
    adr = FAT_INI + sectors[n];
#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
    EEPROM.update(adr, nxt | cnt);
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
    if (EEPROM.read(adr) != (nxt | cnt)) {
      EEPROM.write (adr, nxt | cnt);
      changedEEPROM = true;
    }
#endif
    cnt = CONT_FILE;
    adr = (sectors[n] * SECTOR_SIZE) + DISK_INI;                  // save sector data
    for (i = 0; i < SECTOR_SIZE; i++) {
      data = bufferEdit[pos++];
#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
      EEPROM.update(adr++, data);
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
      if (EEPROM.read(adr) != data) {
        EEPROM.write (adr, data);
        changedEEPROM = true;
      }
      adr++;
#endif
    }
  }
#if defined(USE_Z21) || defined(USE_ECOS)
  if (changedEEPROM)
    EEPROM.commit();
  DEBUG_MSG("Saving EEPROM")
#endif
}


////////////////////////////////////////////////////////////
// ***** ACCESSORY FIFO *****
////////////////////////////////////////////////////////////

void initFIFO() {
  lastInFIFO = 0;
  firstOutFIFO = 0;
  cntFIFO = 0;
  stateFIFO = FIFO_CHECK;
}


unsigned int  readFIFO () {
  firstOutFIFO = (firstOutFIFO + 1 ) & 0x1F;                      // next one (hardcoded)
  cntFIFO--;
  return (accessoryFIFO[firstOutFIFO]);
}


void writeFIFO (byte opcode, byte param) {
  unsigned int value;
  lastInFIFO = (lastInFIFO + 1 ) & 0x1F;                          // next one (hardcoded)
  cntFIFO++;
  value = (opcode << 8) + param;
  accessoryFIFO[lastInFIFO] = value;                              // save in FIFO
}


void sendAccessoryFIFO () {
  switch (stateFIFO) {
    case FIFO_CHECK:
      if (cntFIFO > 0) {                                          // activate accessory from FIFO
        lastAccessory = readFIFO();
        moveAccessoryAutomation(lastAccessory, true);
        accessoryTimer = AUTO_ACC_ON;
        stateFIFO = FIFO_OFF;
        DEBUG_MSG("Moving acc. %d", lastAccessory & 0x07FF);
      }
      break;
    case FIFO_OFF:                                                // deactivate accessory
      if (accessoryTimer > 0)
        accessoryTimer--;
      else {
        moveAccessoryAutomation(lastAccessory, false);
        accessoryTimer = AUTO_ACC_OFF;
        stateFIFO = FIFO_WAIT;
      }
      break;
    case FIFO_WAIT:                                               // wait for CDU recharge
      if (accessoryTimer > 0)
        accessoryTimer--;
      else
        stateFIFO = FIFO_CHECK;
      break;
  }
}


////////////////////////////////////////////////////////////
// ***** AUTOMATION *****
////////////////////////////////////////////////////////////


void initAutomation() {
  byte n;
  for (n = 0; n < MAX_AUTO_SEQ; n++) {
    automation[n].myPosFAT = FREE_SECTOR;
    resetAutomation(n);
  }
  currAutomation = MAX_AUTO_SEQ;
  timerAutomation = millis();
  initFIFO();
}


void resetAutomation(byte num) {                                  // reset a running sequence
  automation[num].currPosFAT = automation[num].myPosFAT;
  automation[num].currStep = 0;
  automation[num].opcode = OPC_AUTO_END;
  automation[num].param = OPC_AUTO_END;
}


byte findAutomation (byte posFAT) {                               // find file in running sequences
  byte n;
  for (n = 0; n < MAX_AUTO_SEQ; n++)
    if (automation[n].myPosFAT == posFAT)
      return n;
  return MAX_AUTO_SEQ;
}


bool startAutomation (byte posFAT) {
  byte num;
  if (findAutomation(posFAT) == MAX_AUTO_SEQ) {                   // avoid same sequence running twice
    for (num = 0; num < MAX_AUTO_SEQ; num++) {
      if (automation[num].myPosFAT == FREE_SECTOR) {
        automation[num].myPosFAT = posFAT;
        resetAutomation(num);
        DEBUG_MSG("Start Automation");
        return true;
      }
    }
  }
  return false;                                                   // already started or full
}


void stopAutomation (byte posFAT) {
  byte num;
  num = findAutomation(posFAT);                                   // stop only if running
  if (num != MAX_AUTO_SEQ) {
    automation[num].myPosFAT = FREE_SECTOR;
    if (scrOLED == SCR_AUTO_SELECT)
      updateOLED = true;
    DEBUG_MSG("Stop Automation");
  }
}


void getOpcodeAutomation (byte num) {
  unsigned int adr;
  byte data;
  byte nxtSector;
  if (automation[num].currStep >= SECTOR_SIZE) {                  // check end of sector
    adr = automation[num].currPosFAT + FAT_INI;
    nxtSector = EEPROM.read(adr) & 0x7F;
    if (nxtSector > 0) {                                          // point to next sector
      automation[num].currPosFAT = nxtSector;
      automation[num].currStep = 0;
    }
    else {
      automation[num].myPosFAT = FREE_SECTOR;                     // EOF
      automation[num].opcode = OPC_AUTO_END;
      automation[num].param = OPC_AUTO_END;
      return;
    }
  }
  adr = (automation[num].currPosFAT * SECTOR_SIZE) + DISK_INI;
  adr += automation[num].currStep;
  automation[num].opcode = EEPROM.read(adr++);
  automation[num].param = EEPROM.read(adr);
  data = automation[num].opcode & OPC_AUTO_MASK;
  if ( data == OPC_AUTO_NAME) {
    automation[num].currStep += (((data | 0x01) & 0x0F) + 1);      // padding
  }
  else {
    automation[num].currStep += 2;
  }
}


void processAutomation() {
  unsigned int value;
  byte pos, n;

  if (millis() - timerAutomation > AUTO_INTERVAL) {               // upate timer for automation
    timerAutomation = millis();
    sendAccessoryFIFO();                                          // send pending accessory
    for (n = 0; n < MAX_AUTO_SEQ; n++) {                          // check timed opcodes
      switch (automation[n].opcode & OPC_AUTO_MASK) {
        case OPC_AUTO_DELAY:
        case OPC_AUTO_LOCO_SEL:
        case OPC_AUTO_LOCO_SEL4:
        case OPC_AUTO_LOCO_SEL8:
          if ((automation[n].value & OPC_TIME_MASK) > 0)
            automation[n].value--;
      }
    }
  }
  currAutomation++;
  if (currAutomation >= MAX_AUTO_SEQ)
    currAutomation = 0;
  if (automation[currAutomation].myPosFAT != FREE_SECTOR) {       // only runing sequences
    switch (automation[currAutomation].opcode & OPC_AUTO_MASK) {  // check waiting opcodes (delay, feedback, loco change)
      case OPC_AUTO_DELAY:
      case OPC_AUTO_LOCO_SEL:
      case OPC_AUTO_LOCO_SEL4:
      case OPC_AUTO_LOCO_SEL8:
        if ((automation[currAutomation].value & OPC_TIME_MASK) > 0)     // wait to complete delay
          return;
        break;
      case OPC_AUTO_FBK:                                          // '1110Saaa aaaaaaaa'
#ifdef USE_LOCONET
        if (automation[currAutomation].opcode & 0x08) {           // waiting for occupied
          if (automation[currAutomation].value == 0)
            return;
        }
        else {                                                    // waiting for free
          if (automation[currAutomation].value != 0)
            return;
        }
#endif
#ifdef USE_XPRESSNET
        if (automation[currAutomation].opcode & 0x08) {           // waiting for occupied
          if (!bitRead(automation[currAutomation].value, automation[currAutomation].opcode & 0x07))
            return;
        }
        else {                                                      // waiting for free
          if (!bitRead(automation[currAutomation].value, automation[currAutomation].opcode & 0x07))
            return;
        }
#endif
#ifdef USE_Z21
        if (automation[currAutomation].opcode & 0x08) {           // waiting for occupied
          if (!bitRead(automation[currAutomation].value, automation[currAutomation].opcode & 0x07))
            return;
        }
        else {                                                      // waiting for free
          if (!bitRead(automation[currAutomation].value, automation[currAutomation].opcode & 0x07))
            return;
        }
#endif
#ifdef USE_ECOS
        if (automation[currAutomation].opcode & 0x08) {           // waiting for occupied
          if (!bitRead(automation[currAutomation].value, automation[currAutomation].param & 0x0F))
            return;
        }
        else {                                                      // waiting for free
          if (bitRead(automation[currAutomation].value, automation[currAutomation].param & 0x0F))
            return;
        }
#endif
        break;
    }
    getOpcodeAutomation(currAutomation);
    switch (automation[currAutomation].opcode & OPC_AUTO_MASK) {
      case OPC_AUTO_LOCO_SEL:                                     // loco address 1..9999 (0x270F)
      case OPC_AUTO_LOCO_SEL4:
      case OPC_AUTO_LOCO_SEL8:
        value = (automation[currAutomation].opcode & 0x3F) << 8;
        value += automation[currAutomation].param;
        if (value > 0) {
#ifdef USE_ECOS
          pos = LOCOS_IN_STACK;
          for (n = 0; n < LOCOS_IN_STACK; n++)                    // search ID in roster list
            if (rosterList[n].id == value)
              pos = n;
          if (pos != LOCOS_IN_STACK) {
            releaseLoco();                                        // release old loco ID
            myLoco.address = rosterList[pos].locoAddress;
            myLocoID = value;
            pushLoco(myLocoID);
            infoLocomotora(myLocoID);
          }
#else
          activeShuttle = false;
          myLoco.address = value;                                 // seleccionada nueva loco
          checkLocoAddress();
          pushLoco(myLoco.address);
#ifdef USE_LOCONET
          doDispatchGet = false;
          doDispatchPut = false;
          liberaSlot();                                           // pasa slot actual a COMMON
          //if (typeCmdStation == CMD_DR)                           // For Intellibox II
          //  sendLocoIB();
          infoLocomotora(myLoco.address);                         // busca info nueva loco
#endif
#if defined(USE_Z21) || defined(USE_XPRESSNET)
          infoLocomotora(myLoco.address);                         // busca info nueva loco
#endif
#endif
          automation[currAutomation].value = AUTO_LOCO_CHG;
          if (scrOLED == SCR_SPEED)
            updateAllOLED = true;
        }
        break;
      case OPC_AUTO_LOCO_OPT:
        if (automation[currAutomation].param & 0x80) {
          if (automation[currAutomation].param & 0x40) {          // '10110000 110000dd' Direction   (REV, FWD, CHG, ESTOP)
            switch (automation[currAutomation].param & 0x03) {
              case 0:
                myDir = 0x00;
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
                break;
              case 1:
                myDir = 0x80;
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
                break;
              case 2:
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
                break;
              case 3:
                mySpeed = 1;
                locoOperationSpeed();
                break;
            }
          }
          else {                                                  // '10110000 10Sfffff' Function    (0..28)
            n = automation[currAutomation].param & 0x1F;
            if (automation[currAutomation].param & 0x20)
              bitSet(myFunc.Bits, n);
            else
              bitClear(myFunc.Bits, n);
            funcOperations(n);
          }
        }
        else {                                                    // '10110000 0sssssss' Speed       (0%..100%)
          value = (automation[currAutomation].param * 127) / 100; // 0..100 -> 0..127
          if (value == 1)                                         // avoid E.stop
            value++;
#if defined(USE_LOCONET) || defined(USE_ECOS)
          mySpeed = value;
#endif
#if defined(USE_Z21) || defined(USE_XPRESSNET)
          if (bitRead(mySteps, 2)) {                              // 128 steps
            mySpeed = value;
          }
          else {
            if (bitRead(mySteps, 1)) {                            // 28 steps
              if (value > 0) {
                value >>= 2;
                if (value < 4)
                  value = 4;
              }
              mySpeed =  (value >> 1) & 0x0F;
              bitWrite(mySpeed, 4, bitRead(value, 0));
            }
            else {                                                // 14 steps
              if (value > 0) {
                value >>= 3;
                if (value < 2)
                  value = 2;
              }
              mySpeed = value;
            }
          }
#endif
          locoOperationSpeed();
        }
        if (scrOLED == SCR_SPEED)
          updateSpeedHID();
        break;
      case OPC_AUTO_DELAY:                                        // '1100tttt tttttttt'
        value =  ((automation[currAutomation].opcode & 0x0F) << 8) + automation[currAutomation].param;
        automation[currAutomation].value = value;
        break;
      case OPC_AUTO_ACC:
        writeFIFO(automation[currAutomation].opcode, automation[currAutomation].param);
        break;
      case OPC_AUTO_FBK:
        automation[currAutomation].value = (automation[currAutomation].opcode & 0x08) ? 0x0000 : 0xFFFF;
#ifdef USE_LOCONET
#endif
#ifdef USE_XPRESSNET
        pos = automation[currAutomation].opcode & 0x07;
        infoAccesorio(automation[currAutomation].param, (pos > 3) ? true : false);
#endif
#ifdef USE_Z21
        getFeedbackInfo ((automation[currAutomation].param > 10) ? 1 : 0);
#endif
#ifdef USE_ECOS
        value = ((automation[currAutomation].opcode & 0x07) << 4) + (automation[currAutomation].param >> 4);
        getInfoS88 (value + 100);
#endif
        break;
      case OPC_AUTO_JUMP:
        switch (automation[currAutomation].param) {
          case OPC_AUTO_END:                                      // stop & free sequence
            automation[currAutomation].myPosFAT = FREE_SECTOR;    // main sequence end
            if (scrOLED == SCR_AUTO_SELECT)
              updateOLED = true;
            DEBUG_MSG("End Automation");
            break;
          case OPC_AUTO_LOOP:                                     // Loop. Re-init sequence
            automation[currAutomation].currPosFAT = automation[currAutomation].myPosFAT;
            automation[currAutomation].currStep = 0;
            break;
          default:                                                // start other sequence
            startAutomation(automation[currAutomation].param);
            if (scrOLED == SCR_AUTO_SELECT)
              updateOLED = true;
            break;
        }
        break;
    }
  }
}



void moveAccessoryAutomation (unsigned int cmd, bool activate) {
  unsigned int value;
  byte pos;
  char posChr;
  char buf[40];
  value =  (cmd & 0x07FF) + 1;
  pos = (cmd & 0x0800) ? RECTO : DESVIADO;                             // '1101Paaa aaaaaaaa'
  if (activate) {
#ifdef USE_LOCONET
    lnetRequestSwitch(value, 1, pos - DESVIADO);
#endif
#ifdef USE_XPRESSNET
    enviaAccesorio(value, true, pos - DESVIADO);
#endif
#ifdef USE_Z21
    setTurnout (value, pos - DESVIADO, true);
#endif
#ifdef USE_ECOS
    posChr = (pos - DESVIADO) ? 'g' : 'r';
    snprintf(buf, 40, "set(%d, switch[%d%c])\n", ID_SWMANAGER, value, posChr);           // Directly setting a port.
    sendMsgECOS(buf);
#endif
  }
  else {
#ifdef USE_LOCONET
    lnetRequestSwitch(value, 0, pos - DESVIADO);
#endif
#ifdef USE_XPRESSNET
    enviaAccesorio(value, false, pos - DESVIADO);
#endif
#ifdef USE_Z21
    setTurnout (value, pos - DESVIADO, false);
#endif
#ifdef USE_ECOS
#endif
  }
}


#ifdef DEBUG
void showEditBuf() {
  unsigned int adr, n;
  char buf[15];
  Serial.println (F("Edit:"));
  adr = 0;
  while (adr < EDIT_BUF_SIZE) {
    for (n = 0; n < 16; n++) {
      snprintf(buf, 15, "%02X ", bufferEdit[n + adr]);
      Serial.print(buf);
    }
    Serial.println();
    adr += 16;
  }
  Serial.println();
}
#endif

/*
  // Test functions Arduino Nano

  #include <EEPROM.h>

  void setup() {
  Serial.begin(115200);
  formatDisk();
  EEPROM.update(FAT_INI, 1);
  EEPROM.update(FAT_INI + 1, 0x85);
  EEPROM.update(FAT_INI + 5, 0x80);
  EEPROM.update(DISK_INI + 0x50, 0x80);
  showDisk(256);
  insertBufEdit(3, 0xaa);
  showEditBuf();
  insertBufEdit(3, 0xbb);
  showEditBuf();
  deleteBufEdit(2);
  showEditBuf();
  deleteBufEdit(2);
  showEditBuf();
  delay(1000);
  loadEditBuf(0);
  showEditBuf();
  deleteFile(0);
  showDisk(16);
  newAutoSequence("Ruta");
  showEditBuf();
  }

  void loop() {

  }

  void formatDisk() {
  unsigned int adr, n;
  for (n = 0; n < FAT_SIZE; n++)
    EEPROM.update(FAT_INI + n, FREE_SECTOR);
  }

  void showDisk(unsigned int size) {
  unsigned int adr, n;
  char buf[15];
  Serial.println (F("FAT:"));
  adr = 0;
  while (adr < FAT_SIZE) {
    for (n = 0; n < 16; n++) {
      snprintf(buf, 15, "%02X ", EEPROM.read(FAT_INI + n + adr));
      Serial.print(buf);
    }
    Serial.println();
    adr += 16;
  }
  Serial.println();
  adr = 0;
  while (adr < size) {
    snprintf(buf, 15, "Sector %02X: ", adr >> 4);
    Serial.print(buf);
    for (n = 0; n < 16; n++) {
      snprintf(buf, 15, "%02X ", EEPROM.read(DISK_INI + n + adr));
      Serial.print(buf);
    }
    Serial.println();
    adr += 16;
  }
  Serial.println();
  }



  byte getNextFile(byte posFAT) {
  byte n, cnt;
  cnt = 0;
  for (n = posFAT + 1; n < FAT_SIZE; n++)
    if (!(EEPROM.read(FAT_INI + n) & 0x80))
      return n;                                                     // start of file found
  return FREE_SECTOR;
*/

#endif
