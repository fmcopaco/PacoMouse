/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/


#ifdef GAME_EXTRA
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)

enum PhasePong {INIT_PONG, WAIT_PONG, RUN_PONG, SERVE_PONG, END_PONG};
enum ballDirection {BALL_UP, BALL_DOWN, BALL_LEFT, BALL_RIGHT};

#define CPU_BAT_X      5
#define PLAYER_BAT_X 122
#define PONG_POINTS   11

byte  gamePhasePong;
byte  pointsPongCPU;
byte  pointsPongPlayer;
bool  winPongCPU;
byte  linePongCPU;
byte  linePongPlayer;
byte  lineBall;
byte  yBatCPU;
byte  xBall;
byte  yBall;
byte  yBallPos;
byte  yBallInc;
byte  ballDirH;
byte  ballDirV;


void  drawPongBat (byte x, byte y, byte *linePad) {
  int scan, line;
  scan = (0xFF << (y & 0x07));        // calculate Bat displacement on screen
  line = y >> 3;
  if (*linePad < line)                // clear Bat raster
    clearPongBat(x, *linePad);
  if (*linePad > line)
    clearPongBat(x, *linePad + 1);
  *linePad = line;
  oled.setCursor (x, line);
  oled.ssd1306WriteRam(lowByte(scan));
  oled.ssd1306WriteRam(lowByte(scan));
  oled.setCursor (x, line + 1);
  oled.ssd1306WriteRam(highByte(scan));
  oled.ssd1306WriteRam(highByte(scan));
}


void clearPongBat (byte x, byte line) {
  oled.setCursor (x, line);
  oled.ssd1306WriteRam(0x00);
  oled.ssd1306WriteRam(0x00);
}

void  drawPongBall (byte x, byte y, byte *linePad) {
  int scan, line;
  scan = (0x03 << (y & 0x07));        // calculate Bat displacement on screen
  line = y >> 3;
  lineBall = line;
  oled.setCursor (x, line);
  oled.ssd1306WriteRam(lowByte(scan));
  oled.ssd1306WriteRam(lowByte(scan));
  if ((y & 0x07) > 5) {
    oled.setCursor (x, line + 1);
    oled.ssd1306WriteRam(highByte(scan));
    oled.ssd1306WriteRam(highByte(scan));
  }
}


void clearPongBall() {
  byte y;
  y = yBall >> 3;
  oled.setCursor (xBall, y);
  oled.ssd1306WriteRam(0x00);
  oled.ssd1306WriteRam(0x00);
  if ((yBall & 0x07) > 5) {
    oled.setCursor (xBall, y + 1);
    oled.ssd1306WriteRam(0x00);
    oled.ssd1306WriteRam(0x00);
  }
}


void drawPongNet() {
  byte i;
  for (i = 0; i < 7; i++) {
    oled.setCursor(63, i);
    oled.ssd1306WriteRam(0x66);
    oled.ssd1306WriteRam(0x66);
  }
}


void drawPongPoints() {
  oled.setCursor(40, 0);
  oled.print(pointsPongCPU);
  oled.print(' ');
  oled.setCursor(80, 0);
  oled.print(pointsPongPlayer);
  oled.print(' ');
}


void moveBatCPU() {
  if (yBall > (yBatCPU + 3)) {
    //if ((yBall - yBatCPU) > 16)
    //yBatCPU++;
    if (yBatCPU < 47)
      yBatCPU++;
  }
  if (yBall < (yBatCPU + 4)) {
    //if ((yBatCPU - yBall) > 16)
    //yBatCPU--;
    if (yBatCPU > 0)
      yBatCPU--;
  }
}

void movePongBall() {
  if (ballDirV == BALL_UP) {
    if (yBallPos < 5)
      ballDirV = BALL_DOWN;
    else
      yBallPos -= yBallInc;
  }
  else {
    if (yBallPos > 216)
      ballDirV = BALL_UP;
    else
      yBallPos += yBallInc;
  }
  if (ballDirH == BALL_RIGHT) {
    xBall++;
    if ((xBall > (PLAYER_BAT_X - 3)) && (xBall < (PLAYER_BAT_X + 2))) {
      checkHitBall(encoderValue);
    }
    if (xBall > 124) {
      pointsPongCPU++;
      winPongCPU = true;
      servePongBall();
      if (pointsPongCPU >= PONG_POINTS)
        gamePhasePong = END_PONG;
    }
  }
  else {
    xBall--;
    if ((xBall < (CPU_BAT_X + 3)) && (xBall > (CPU_BAT_X - 1))) {
      checkHitBall(yBatCPU);
    }
    else {
      if (xBall < 3) {
        pointsPongPlayer++;
        winPongCPU = false;
        servePongBall();
        if (pointsPongPlayer >= PONG_POINTS)
          gamePhasePong = END_PONG;
      }
    }
  }
  yBall = yBallPos >> 2;
}


void checkHitBall(byte y) {
  byte yPos, offset;
  yPos = yBallPos >> 2;
  if ((yPos >= y) && (yPos <= (y + 7))) {
    offset = yPos - y;
    switch (offset) {
      case 0:
      case 7:
        yBallInc = 4;
        break;
      case 1:
      case 6:
        yBallInc = 2;
        break;
      case 2:
      case 5:
        yBallInc = 1;
        break;
      default:
        break;
    }
    ballDirH = (ballDirH == BALL_LEFT) ? BALL_RIGHT : BALL_LEFT;
  }
}

void servePongBall() {
  drawPongPoints();
  if (winPongCPU) {
    xBall = CPU_BAT_X + 10;
    yBall = yBatCPU;
    ballDirH = BALL_RIGHT;
  }
  else {
    xBall = PLAYER_BAT_X - 12;
    yBall = encoderValue;
    ballDirH = BALL_LEFT;
  }
  lineBall = yBall >> 3;
  ballDirV = BALL_DOWN;
  yBallInc = 0;
  yBallPos = yBall << 2;
  timeoutOLED = 1500;
  gamePhasePong = SERVE_PONG;
}


void initGamePong() {
  gamePhasePong = INIT_PONG;
}


void startGamePong() {
  randomSeed(millis());
  oled.clear();
  pointsPongCPU = 0;
  pointsPongPlayer = 0;
  winPongCPU = true;
  encoderMax = 47;
  encoderValue = 24;
  yBatCPU = 24;
  linePongCPU = yBatCPU >> 3;
  linePongPlayer = encoderValue >> 3;
  drawPongNet();
  drawPongBat(CPU_BAT_X, encoderValue, &linePongCPU);
  drawPongBat(PLAYER_BAT_X, yBatCPU, &linePongPlayer);
  servePongBall();
  drawPongBall(xBall, yBall, &lineBall);
  scrOLED = SCR_GAME;
  timeoutOLED = 30;
  updateOLED = true;
  while (digitalRead (pinSwitch) == LOW);        // wait while the user stops pressing the button
}


void drawGamePong() {
  switch (gamePhasePong) {
    case INIT_PONG:
      oled.clear();
      oled.setFont(Arial_bold_14);
      oled.setCursor(50, 2);
      oled.print(F("Pong"));
      gamePhasePong = WAIT_PONG;
      timeoutOLED = ONE_DAY;
      break;
    case WAIT_PONG:
      startGamePong();
      gamePhasePong = RUN_PONG;
      break;
    case RUN_PONG:
      if ((xBall > 60) && (xBall < 66))
        drawPongNet();
      if (yBall < 17)
        drawPongPoints();
      moveBatCPU();
      clearPongBall();
      movePongBall();
      drawPongBall(xBall, yBall, &lineBall);
      drawPongBat(CPU_BAT_X, yBatCPU, &linePongCPU);
      drawPongBat(PLAYER_BAT_X, encoderValue, &linePongPlayer);
      break;
    case SERVE_PONG:
      timeoutOLED = 30;
      gamePhasePong = RUN_PONG;
      break;
    case END_PONG:
      oled.setCursor(26, 3);
      oled.print(F("Game Over"));
      gamePhasePong = WAIT_PONG;
      timeoutOLED = ONE_DAY;
      break;
  }
}


#endif
#endif
