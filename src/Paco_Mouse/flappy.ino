/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/


#ifdef GAME_EXTRA

enum PhaseFlappy {INIT_FLAP, WAIT_FLAP, RUN_FLAP};

#define BIRD_X      32
#define BIRD_FLOOR  54

byte gamePhaseFlappy;
int currPoints;
int maxPoints;
int xWall[2];
byte yWall[2];
int8_t yBird;
int8_t momentum;
byte birdLine;
bool wing;

const unsigned int wingUp[]   = {0x1c, 0x24, 0x66, 0xa5, 0x99, 0x81, 0xa7, 0xf9, 0x75, 0x72, 0x7c, 0x30};
const unsigned int wingDown[] = {0x70, 0x4c, 0x4a, 0xa9, 0x99, 0x81, 0xa7, 0xf9, 0x75, 0x72, 0x7c, 0x30};


void moveBird() {
  if (digitalRead (pinSwitch) == LOW)     // if switch pressed go up
    momentum = -4;
  momentum++;                             // gravity force
  yBird += momentum;
  if (yBird < 0)                          // check top of screen
    yBird = 0;
  if (yBird > BIRD_FLOOR) {               // flying near the floor
    yBird = BIRD_FLOOR;
    momentum = -2;
  }
  if ((momentum < 0) && wing) {
    drawBird(wingDown);                   // flying animation
    wing = !wing;
  }
  else {
    drawBird(wingUp);                     // falling or flying animation
    wing = true;
  }
}


void drawBird(const unsigned int *bird) {
  byte line, offset, i;
  byte lower[12], upper[12];
  int scan;
  offset = yBird & 0x07;
  for (i = 0; i < 12; i++) {              // calculate bird displacement on screen
    scan = bird[i] << offset;
    upper[i] = lowByte(scan);
    lower[i] = highByte(scan);
  }
  line = yBird >> 3;
  if (birdLine < line)                    // clear bird raster
    clearBird(birdLine);
  if (birdLine > line)
    clearBird(birdLine + 1);
  birdLine = line;
  oled.setCursor (BIRD_X, line);
  for (i = 0; i < 12; i++)                // draw displaced bird
    oled.ssd1306WriteRam(upper[i]);
  oled.setCursor (BIRD_X, line + 1);
  for (i = 0; i < 12; i++)
    oled.ssd1306WriteRam(lower[i]);
}


void clearBird(byte line) {
  oled.setCursor (BIRD_X, line);
  for (byte i = 0; i < 12; i++)            // clear bird raster
    oled.ssd1306WriteRam(0x00);
}


void moveWall() {
  byte i;
  for (i = 0; i < 2 ; i++) {
    drawWall(xWall[i], yWall[i], 0xFF);     // draw wall on new position
    drawWall(xWall[i] + 12, yWall[i], 0x00);
    if (xWall[i] == -12) {
      xWall[i] = 128;                       // wall hit edge, reset with new gap position
      yWall[i] = random(6);
    }
    if (xWall[i] == BIRD_X)                 // bird passed the gap
      printPointsFlappy(true);
    if (
      (BIRD_X + 12 > xWall[i] && BIRD_X < xWall[i] + 12) // level with wall
      &&
      ((yBird >> 3) < yWall[i] || ((yBird + 8) >> 3) > (yWall[i] + 2)) // not level with the gap
    )
      gameOverFlappy();
    xWall[i] -= 4;
  }
}


void drawWall (int xPos, byte yPos, byte pattern) {
  byte i;                                   // draw wall with gap
  if ((xPos < 125) && (xPos >= 0)) {
    for (i = 0; i < 8; i++) {
      oled.setCursor(xPos, i);
      if (i < yPos)
        drawWallBrick (pattern);
      if (i > (yPos + 2))                  // lower wall
        drawWallBrick (pattern);
    }
  }
}

void drawWallBrick (byte pattern) {
  oled.ssd1306WriteRam(pattern);
  oled.ssd1306WriteRam(pattern);
  oled.ssd1306WriteRam(pattern);
  oled.ssd1306WriteRam(pattern);
}

void printPointsFlappy(bool newPoint) {
  if (newPoint) {
    currPoints++;                             // every 4 points go a little faster
    if (!(currPoints & 0x0003))
      timeoutOLED -= 30;
  }
  oled.setCursor (BIRD_X + 24, 0);
  oled.print(currPoints);
}

void gameOverFlappy() {
  if (currPoints > maxPoints)                 // set max score
    maxPoints = currPoints;
  oled.setCursor(16, 3);
  oled.print(F("Game Over"));
  printPointsFlappy(false);
  timeoutOLED = 3000;
  gamePhaseFlappy = INIT_FLAP;
}


void initGameFlappy() {
  gamePhaseFlappy = INIT_FLAP;
  maxPoints = 0;
}


void startGameFlappy() {
  randomSeed(millis());
  oled.clear();
  currPoints = 0;
  yBird = 32;                                   // init positions
  momentum = -4;
  xWall[0] = 128;
  yWall[0] = 2;
  xWall[1] = 192;
  yWall[1] = 3;
  scrOLED = SCR_GAME;
  timeoutOLED = 240;
  updateOLED = true;
  while (digitalRead (pinSwitch) == LOW);        // wait while the user stops pressing the button
}


void drawGameFlappy() {
  switch (gamePhaseFlappy) {
    case INIT_FLAP:
      oled.clear();
      yBird = 42;
      drawBird(wingDown);
      oled.setFont(Arial_bold_14);
      printCfgOption(2, CFG_GAME_FLAPPY);
      oled.setCursor(60, 5);
      oled.print(maxPoints);
      gamePhaseFlappy = WAIT_FLAP;
      timeoutOLED = ONE_DAY;
      break;
    case WAIT_FLAP:
      startGameFlappy();
      gamePhaseFlappy = RUN_FLAP;
      break;
    case RUN_FLAP:
      moveBird();
      moveWall();
      break;
  }
}


#endif
