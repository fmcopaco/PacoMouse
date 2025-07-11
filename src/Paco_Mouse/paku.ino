/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
      
      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/


#ifdef GAME_EXTRA
enum PhasePaku {INIT_PAKU, WAIT_PAKU, RUN_PAKU};

byte gamePhasePaku;
unsigned int currPointsPaku;
unsigned int maxPointsPaku;

#define PAKU_Y       5
#define X_OFFSET    5
#define EATEN     0xFF
#define NUM_DOTS    10
#define ICON_WIDTH  10

byte xPaku;
bool dirPaku;                                               // true: right, false: left
byte xGhost;
byte dirEye;                                                // 0x00: ghost eaten (eye), 0x01: ghost right, 0xFF: ghost left
byte xPower;
byte powerTicks;                                            // duration of power up
byte levelPaku;                                             // multiplier for game score
byte xDots[NUM_DOTS];
byte numDots;                                               // dots left
byte animTicks;

const byte iconPakuOpen[]  = {0xE0, 0xF0, 0xF8, 0xFC, 0x7C, 0x3C, 0x3C, 0x18, 0x18, 0x10,
                              0x03, 0x07, 0x0F, 0x1F, 0x1F, 0x1E, 0x1E, 0x0C, 0x0C, 0x04
                             };
const byte iconPakuMid[]   = {0xE0, 0xF0, 0xF8, 0xFC, 0x7C, 0x7C, 0x3C, 0x38, 0x30, 0x20,
                              0x03, 0x07, 0x0F, 0x1F, 0x1F, 0x1F, 0x1E, 0x0E, 0x06, 0x02
                             };
const byte iconPakuClose[] = {0xE0, 0xF0, 0xF8, 0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF0, 0xE0,
                              0x03, 0x07, 0x0F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0F, 0x07, 0x03
                             };

const byte iconGhostA[]    = {0x00, 0xF0, 0xF8, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xF8, 0x70,
                              0x1F, 0x1F, 0x07, 0x1F, 0x1F, 0x07, 0x1F, 0x1F, 0x07, 0x00
                             };
const byte iconGhostB[]    = {0x00, 0xF0, 0xF8, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xF8, 0x70,
                              0x00, 0x07, 0x1F, 0x1F, 0x07, 0x1F, 0x1F, 0x07, 0x1F, 0x18
                             };

const byte iconPower[]     = {0xC0, 0xE0, 0xF0, 0xF0, 0xE0, 0xC0,
                              0x00, 0x01, 0x03, 0x03, 0x01, 0x00
                             };
const byte iconDot         = 0xC0;
const byte iconEye         = 0x10;


void drawPaku() {
  byte n, i;
  const byte *icon;
  n = (animTicks >> 2) & 0x03;
  oled.setCursor (xPaku + X_OFFSET, PAKU_Y);
  switch (n) {
    case 0:
      icon = iconPakuOpen;
      break;
    case 1:
    case 3:
      icon = iconPakuMid;
      break;
    case 2:
      icon = iconPakuClose;
      break;
  }
  if (!dirPaku)
    icon += (ICON_WIDTH - 1);
  for (i = 0; i < ICON_WIDTH; i++)  {
    if (dirPaku)                                            // draw Paku
      oled.ssd1306WriteRam(*icon++);
    else
      oled.ssd1306WriteRam(*icon--);
  }
  oled.setCursor (xPaku + X_OFFSET, PAKU_Y + 1);
  if (!dirPaku)
    icon += (ICON_WIDTH + ICON_WIDTH);
  for (i = 0; i < ICON_WIDTH; i++)  {
    if (dirPaku)                                            // draw Paku
      oled.ssd1306WriteRam(*icon++);
    else
      oled.ssd1306WriteRam(*icon--);
  }
}


void drawGhost(byte mask) {                                 // mask: 0x00: normal, 0xFF: inverted
  byte i;
  const byte *icon;
  icon = ((animTicks >> 3) & 0x01) ? iconGhostA : iconGhostB;
  oled.setCursor (xGhost + X_OFFSET, PAKU_Y);
  if (dirEye == 0xFF)
    icon += (ICON_WIDTH - 1);
  for (i = 0; i < ICON_WIDTH; i++)  {                                // draw Ghost
    if (dirEye  == 0x01)
      oled.ssd1306WriteRam((*icon++) ^ mask);
    else
      oled.ssd1306WriteRam((*icon--) ^ mask);
  }
  oled.setCursor (xGhost + X_OFFSET, PAKU_Y + 1);
  if (dirEye == 0xFF)
    icon += (ICON_WIDTH + ICON_WIDTH);
  for (i = 0; i < ICON_WIDTH; i++)  {                                // draw Ghost
    if (dirEye  == 0x01)
      oled.ssd1306WriteRam((*icon++) ^ mask);
    else
      oled.ssd1306WriteRam((*icon--) ^ mask);
  }
}


void drawPoints(byte x, unsigned int points) {
  oled.setCursor(x, 0);
  oled.print(points);
  oled.print(' ');
}


void drawDots() {
  byte i, x, n;
  numDots = 0;
  for (i = 0; i < NUM_DOTS; i++) {
    x = xDots[i];
    if (x != EATEN) {
      numDots++;
      oled.setCursor (x + X_OFFSET, PAKU_Y);
      if (x == xPower) {                                    // draw power dot
        oled.setCol(x + X_OFFSET - 2);
        for (n = 0; n < 6; n++)
          oled.ssd1306WriteRam(iconPower[n]);
        oled.setCursor(x + X_OFFSET - 2, PAKU_Y + 1);
        for (n = 6; n < 12; n++)
          oled.ssd1306WriteRam(iconPower[n]);
      }
      else {                                                // draw normal dot
        oled.ssd1306WriteRam(iconDot);
        oled.ssd1306WriteRam(iconDot);
      }
    }
  }
}


void clearIcon(byte x) {
  byte i;
  oled.setCursor (x + X_OFFSET, PAKU_Y);
  for (i = 0; i < ICON_WIDTH; i++)
    oled.ssd1306WriteRam(0x00);
  oled.setCursor (x + X_OFFSET, PAKU_Y + 1);
  for (i = 0; i < ICON_WIDTH; i++)
    oled.ssd1306WriteRam(0x00);
}

void movePaku() {
  clearIcon(xPaku);
  if (updateAllOLED) {                                      // pressed switch, change direction
    updateAllOLED = false;
    dirPaku = !dirPaku;
  }
  xPaku += (dirPaku) ? 0x01 : 0xFF;
  if ((powerTicks > 0) && (!(animTicks & 0x07)))
    xPaku += (dirPaku) ? 0x01 : 0xFF;
  if (xPaku > 100)                                          // escape through edge
    xPaku = 2;
  if (xPaku < 2)
    xPaku = 100;
  drawPaku();
}

void moveGhost() {
  byte inv;
  clearIcon(xGhost);
  if (dirEye == 0) {                                        // defeated
    xGhost = (dirPaku) ? xGhost - 2 : xGhost + 2;
    if ((xGhost > 98) || (xGhost < 2)) {
      xGhost = (xPaku > 50) ? 2 : 98;                       // respawn
      dirEye = (xPaku > 50) ? 0x01 : 0xFF;
    }
    else {
      oled.setCursor(xGhost + 1 + X_OFFSET, PAKU_Y);
      oled.ssd1306WriteRam(iconEye);
      oled.ssd1306WriteRam(0x00);
      oled.ssd1306WriteRam(0x00);
      oled.ssd1306WriteRam(iconEye);
    }
  }
  else {                                                    // alive
    if (powerTicks > 0) {
      dirEye = (xPaku > xGhost) ? 0xFF : 0x01;
    }
    else {
      dirEye = (xPaku > xGhost) ? 0x01 : 0xFF;
      if (!(animTicks & 0x07))
        xGhost += dirEye;
    }
    xGhost += dirEye;
    if (xGhost > 100)                                       // can't go through edge
      xGhost = 100;
    if (xGhost < 2)
      xGhost = 2;
    inv = (powerTicks > 0) ? 0xFF : 0x00;
    drawGhost(inv);
  }
}

void checkCollision() {
  byte i, p, x, d;
  if (powerTicks > 0)
    powerTicks--;
  p = (dirPaku) ? xPaku + 7 : xPaku + 2;
  d = xPower;
  for (i = 0; i < NUM_DOTS; i++) {
    x = xDots[i];
    if (powerTicks > 0) {
      p &= 0xFE;
      x &= 0xFE;
      d &= 0xFE;
    }
    if (p == x) {
      if (p == d) {
        xPower = EATEN;
        powerTicks = 60;
      }
      addPoints(false);
      xDots[i] = EATEN;
    }
  }
  if (dirEye != 0) {                                        // ghost alive
    if (xPaku > xGhost) {
      if (xPaku < (xGhost + 8)) {
        if (powerTicks > 0) {
          dirEye = 0;                                       // ghost eaten
          powerTicks = 0;
          addPoints(true);
        }
        else
          levelPaku = 0;
      }
    }
    else {
      if (xGhost < (xPaku + 8)) {
        if (powerTicks > 0) {
          dirEye = 0;                                       // ghost eaten
          powerTicks = 0;
          addPoints(true);
        }
        else {
          levelPaku = 0;
        }
      }
    }
  }
}

void addPoints(bool eatGhost) {
  currPointsPaku += levelPaku;
  if (eatGhost)                                             // additional points for ghost
    currPointsPaku += 10;
  drawPoints(0, currPointsPaku);
  if (currPointsPaku > maxPointsPaku) {
    maxPointsPaku = currPointsPaku;
    drawPoints (80, maxPointsPaku);
  }
}

void addDots() {
  byte i, pwr;
  pwr = (xPaku > 50) ? byte(random(1, 4)) : byte(random(6, 9));
  for (i = 0; i < NUM_DOTS; i++)
    xDots[i] = (i * 10) + 9;
  xPower = xDots[pwr];
}


void startLevelPaku() {
  byte i;
  levelPaku++;
  oled.clear();
  oled.setCursor(0, PAKU_Y - 1);                            // draw field
  for (i = 0; i < 128; i++)
    oled.ssd1306WriteRam(0x50);
  oled.setCursor(0, PAKU_Y + 2);
  for (i = 0; i < 128; i++)
    oled.ssd1306WriteRam(0x05);
  oled.setFont(Arial_bold_14);
  oled.setCursor(0, 2);
  oled.print('x');
  oled.print(levelPaku);
  oled.setCursor(64, 0);
  oled.print(F("HI"));
  drawPoints(0, currPointsPaku);
  drawPoints(80, maxPointsPaku);
  addDots();
  timeoutOLED = 40 - levelPaku;                             // a little faster every level
}


void startGamePaku() {
  randomSeed(millis());
  currPointsPaku = 0;
  xPaku = 40;
  dirPaku = true;
  xGhost = 100;
  dirEye = 0xFF;
  levelPaku = 0;
  animTicks = 0;
  powerTicks = 0;
  scrOLED = SCR_GAME;
  updateOLED = true;
  updateAllOLED = false;
}


void initGamePaku() {
  gamePhasePaku = INIT_PAKU;
  maxPointsPaku = 0;
}


void drawGamePaku() {
  switch (gamePhasePaku) {
    case INIT_PAKU:
      oled.clear();
      oled.setFont(Arial_bold_14);
      printCfgOption(3, CFG_GAME_PAKU);
      //oled.setCursor(30, 3);
      //oled.print("Paku Paku");
      gamePhasePaku = WAIT_PAKU;
      timeoutOLED = ONE_DAY;
      break;
    case WAIT_PAKU:
      startGamePaku();
      startLevelPaku();
      gamePhasePaku = RUN_PAKU;
      break;
    case RUN_PAKU:
      animTicks++;
      drawDots();
      movePaku();
      checkCollision();
      moveGhost();
      if (numDots == 0) {
        startLevelPaku();                                   // if all eaten, next level
      }
      else {
        if (levelPaku == 0) {                               // Game Over
          oled.setCursor (40, 2);
          oled.print(F("Game Over"));
          timeoutOLED = 3000;
          gamePhasePaku = WAIT_PAKU;
        }
      }
      break;
  }
}





/* ////////////////////////////////////////////////////////////////////////////////

  //Wokwi simulator: https://wokwi.com/projects/348849468083274322
  // PacoMouse loop simulator

  #include <Wire.h>
  #include "SSD1306Ascii.h"
  #include "SSD1306AsciiWire.h"

  SSD1306AsciiWire oled;

  #define ONE_DAY 86400000UL
  #define SCR_GAME 7
  const int pinSwitch     = 2;
  byte scrOLED;
  bool updateOLED, updateAllOLED;
  unsigned long timeOLED, timeoutOLED;
  const unsigned long timeoutButtons = 50;                    // temporizador antirebote
  unsigned long timeButtons;
  byte inputButton;
  byte statusSwitch;


  void setup() {
  pinMode (pinSwitch, INPUT_PULLUP);                           // OLED
  Wire.begin();
  Wire.setClock(400000L);
  oled.begin(&Adafruit128x64, 0x3C);
  initGamePaku();
  }

  void loop() {

  if (updateOLED) {                                          // Actualizar pantalla
    updateOLED = false;
    drawGamePaku();
  }
  if (millis() - timeOLED > timeoutOLED) {
    timeOLED = millis();     // Fin tiempo espera para actualizar OLED
    updateOLED = true;
  }
  if (millis() - timeButtons > timeoutButtons) {             // lectura de boton
    timeButtons = millis();                                   // lee cada cierto tiempo
    inputButton = digitalRead (pinSwitch);                    // comprueba cambio en boton del encoder
    if (statusSwitch != inputButton) {
      statusSwitch = inputButton;
      if (statusSwitch == LOW) {
        updateOLED = true;
        updateAllOLED = true;
      }
    }
  }
  }


  //////////////////////////////////////////////////////////////////////// */


#endif
