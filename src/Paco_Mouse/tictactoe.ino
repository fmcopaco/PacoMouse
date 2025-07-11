/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
      
      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/


#ifdef GAME_EXTRA

enum thingTic {EMPTY, CROSS, CIRCLE = 4};
enum PhaseTic {INIT_TIC, WAIT_TIC, PLAYER, THINKING, MOVING, END};

byte board [9];
byte boxPos[9][2] = {{0, 0}, {24, 0}, {48, 0}, {0, 3}, {24, 3}, {48, 3}, {0, 6}, {24, 6}, {48, 6}};
byte lines [8][3] = {{0, 4, 8}, {2, 4, 6}, {0, 1, 2}, {6, 7, 8}, {0, 3, 6}, {2, 5, 8}, {3, 4, 5}, {1, 4, 7}};
byte answer[9] = {4, 7, 4, 5, 8, 3, 4, 1, 4};

// 0 1 2 Board positions
// 3 4 5
// 6 7 8

#define TIC_NONE 0xFF

byte currCursor;
byte gamePhaseTic;
bool gameTurn;
byte myMove;
byte xPoints, oPoints;


void drawBox (byte box) {
  char thing;
  oled.setFont(Sym_16x16);
  oled.setCursor (boxPos[box][0], boxPos[box][1]);
  switch (board[box]) {
    case CROSS:
      thing = (box == currCursor) ? '7' : '9';
      break;
    case CIRCLE:
      thing = (box == currCursor) ? '+' : '-';
      break;
    case EMPTY:
      thing = (box == currCursor) ? '6' : '/';
      break;
  }
  oled.print(thing);
  DEBUG_MSG("\n%d %d %d\n%d %d %d\n%d %d %d\n%d-%c", board[0], board[1], board[2], board[3], board[4], board[5], board[6], board[7], board[8], box, thing);
}


void drawBoard() {
  byte i;
  oled.setCursor(0, 2);
  for (i = 0; i < 64; i++)
    oled.ssd1306WriteRam(0x18);
  oled.setCursor(0, 5);
  for (i = 0; i < 64; i++)
    oled.ssd1306WriteRam(0x18);
  for (i = 0; i < 8; i++) {
    oled.setCursor(18, i);
    oled.ssd1306WriteRam(0xFF);
    oled.ssd1306WriteRam(0xFF);
    oled.setCol(43);
    oled.ssd1306WriteRam(0xFF);
    oled.ssd1306WriteRam(0xFF);
  }
  oled.setFont(Sym_16x16);
  oled.setCursor(75, 2);
  oled.print ('9');
  oled.setCursor(75, 5);
  oled.print('-');
}


void printPoints() {
  byte i, points;
  oled.setFont(Arial_bold_14);
  for (byte i = 2; i < 6; i += 3) {
    oled.setCursor(100, i);
    points = (i == 2) ? xPoints : oPoints;
    oled.print (points);
    oled.print (' ');
  }
}


void clearBoard() {
  byte i;
  for (i = 0; i < 9; i++)
    board[i] = EMPTY;
}


byte checkBoard (byte seq) {
  byte box, pos, i, j;
  for (i = 0; i < 8; i++) {                                                   // check all possible lines for a sequence
    pos = 0;
    for (j = 0; j < 3; j++) {
      box = lines[i][j];
      pos += board[box];
    }
    if (pos == seq)
      return i;
  }
  return TIC_NONE;
}


bool checkWin (byte player) {
  byte result, i;
  result = checkBoard(player + player + player);
  if (result != TIC_NONE) {
    for (i = 0; i < 3; i++) {                                                 // tic-tac-toe
      currCursor = lines[result][i];
      drawBox(currCursor);
    }
    if (player == CROSS)
      xPoints++;
    else
      oPoints++;
    return true;
  }
  else
    return false;
}


bool canMove() {
  for (byte i = 0; i < 9; i++)
    if (board[i] == EMPTY)
      return true;
  gamePhaseTic = END;                                                       // can't move, no winner
  updateOLED = true;
  return false;
}


void initGameTic() {
  xPoints = 0;
  oPoints = 0;
  gamePhaseTic = INIT_TIC;
  gameTurn = false;
}


void startGameTic() {
  randomSeed(millis());
  oled.clear();
  clearBoard();
  drawBoard();
  printPoints();
  currCursor = TIC_NONE;
  encoderMax = 8;
  encoderValue = 0;
  scrOLED = SCR_GAME;
  updateAllOLED = false;
  updateOLED = true;
}


void tryMove (byte seq) {
  byte result, box;
  result = checkBoard(seq);
  if (result != TIC_NONE) {                                                       // try move if sequence found
    box = lines[result][0];
    if (board[box] == EMPTY) {
      myMove = box;
    }
    else {
      box = lines[result][2];
      if (board[box] == EMPTY)
        myMove = box;
      else
        myMove = lines[result][1];
    }
  }
}


void basicMove() {
  byte i, things, pos;
  things = 0;
  for (i = 0; i < 9; i++)
    if (board[i] == EMPTY)                                                    // look for an empty box
      myMove = i;
    else {
      things++;                                                               // thing position
      pos = i;
    }
  if (things == 0)                                                            // first move O
    myMove = xPoints & 0x06;
  if (things == 1)                                                            // answer to first move X
    myMove = answer[pos];
}


void drawGameTic() {
  byte pos;
  switch (gamePhaseTic) {
    case INIT_TIC:
      oled.clear();
      oled.setFont(Arial_bold_14);
      printCfgOption(3, CFG_GAME_TIC);
      gamePhaseTic = WAIT_TIC;
      timeoutOLED = ONE_DAY;
      break;
    case WAIT_TIC:
      startGameTic();
      gamePhaseTic = gameTurn ? THINKING : PLAYER;
      break;
    case PLAYER:
      if (canMove()) {
        if (currCursor != encoderValue) {
          pos = currCursor;
          if (pos != TIC_NONE) {
            currCursor = TIC_NONE;
            drawBox(pos);
          }
          currCursor = encoderValue;
          drawBox(encoderValue);
        }
        if (updateAllOLED) {
          if (board[encoderValue] == EMPTY) {
            currCursor = TIC_NONE;
            board[encoderValue] = CROSS;
            drawBox(encoderValue);
            gamePhaseTic = checkWin(CROSS) ? END : THINKING;
            updateOLED = true;
          }
          updateAllOLED = false;
        }
      }
      break;
    case THINKING:
      if (canMove()) {
        basicMove();                                                          // first and basic moves
        tryMove(CIRCLE);                                                      // improve my position
        tryMove(CROSS + CROSS);                                               // block opponent
        tryMove(CIRCLE + CIRCLE);                                             // win the game
        gamePhaseTic = MOVING;
        timeoutOLED = 1000;
      }
      break;
    case MOVING:
      currCursor = TIC_NONE;
      board[myMove] = CIRCLE;
      drawBox(myMove);
      gamePhaseTic = checkWin(CIRCLE) ? END : PLAYER;
      timeoutOLED = ONE_DAY;
      updateAllOLED = false;
      updateOLED = true;
      break;
    case END:
      gameTurn = !gameTurn;
      printPoints();
      gamePhaseTic = WAIT_TIC;
      timeoutOLED = 4000;
      break;
  }
}

/*
  void basicMove() {
  byte i;
  bool betterMove;
  betterMove = false;
  for (i = 0; i < 9; i++)                                                     // look for an empty box
    if (board[i] == EMPTY) {
      switch (i) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 8:
          betterMove = true;
        default:
          myMove = i;
          break;
      }
    }
    //DEBUG_MSG("Basic: %d",myMove);
  if (betterMove) {                                                           // recalculate if exist a better move
    while (betterMove) {
      myMove = random(5);
      if (myMove == 1)
        myMove = 6;
      if (myMove == 3)
        myMove = 8;
      if (board[myMove] == EMPTY)
        betterMove = false;
    }
    //DEBUG_MSG("Better: %d",myMove);
  }
  }
*/
#endif
