/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/


#ifdef GAME_EXTRA

#define SNAKE_MAX 25                                                          // Max. length of snake

enum thingSnake {ERASE, SNAKE, FOOD, BOARD};
enum PhaseSnake {INIT, WAIT, RUN};

typedef struct
{
  byte x;
  byte y;
} coord;

coord snake[SNAKE_MAX + 2];
coord foodCoord;
byte snakeLength = 2;
byte directions[4][2] = {{1, 0}, {0, 1}, {255, 0}, {0, 255}};
byte dirIndex;
byte gamePhase;
bool onBorder, borderFull;
byte gameLevel;


void drawThing (byte x, byte y, byte thing) {
  byte i;
  oled.setFont(Sym_Bars);
  oled.setCursor(x << 3, y);
  switch (thing) {
    case ERASE:
      oled.print('.');
      break;
    case SNAKE:
      oled.print('@');
      break;
    case FOOD:
      oled.print('A');
      break;
    case BOARD:
      for (i = 0; i < 127; i++)
        oled.ssd1306WriteRam(0x01);
      for (i = 0; i < 8; i++) {
        oled.setCursor(0, i);
        oled.ssd1306WriteRam(0xFF);
        oled.setCol(127);
        oled.ssd1306WriteRam(0xFF);
      }
      oled.setCursor(1, 7);
      for (i = 1; i < 127; i++)
        oled.ssd1306WriteRam(0x80);
      borderFull = true;
      break;
  }
}


void initSnake() {
  snakeLength = 2;
  snake[0] = {2, (byte) random(1, 7)};
  snake[1] = {1, snake[0].y};
  drawThing(snake[0].x, snake[0].y, SNAKE);
  drawThing(snake[1].x, snake[1].y, SNAKE);
  dirIndex = 0;
}


void putFood() {
  bool foodOK = false;
  while (!foodOK)                                                             //Make sure the food doesnt fall on top of the snake
  {
    foodCoord = {byte(random(1, 15)), byte(random(1, 7))};
    foodOK = true;
    for (byte i = 0; i < snakeLength; i++)
    {
      if (foodCoord.y == snake[i].y && foodCoord.x == snake[i].x)
      {
        foodOK = false;
        break;
      }
    }
  }
  drawThing(foodCoord.x, foodCoord.y, FOOD);
}


bool moveSnake() {
  byte x, y;
  bool onFood;
  dirIndex = (dirIndex + encoderValue - 1) & 0x03;                            // Modify direction according encoder
  encoderValue = 1;
  onBorder = false;
  x = snake[0].x + directions[dirIndex][0];                                   // Calculate the new coordinates
  y = snake[0].y + directions[dirIndex][1];
  if (x > 15 || x == 0xFF || y > 7 || y == 0xFF)                              // If out of bounds, exit and lose.
    return false;
  drawThing(x, y, SNAKE);
  onFood = (x == foodCoord.x && y == foodCoord.y) ? true : false;
  for (byte i = snakeLength; i != 0; --i) {                                    // Shift all the snake coords back to make space for the head
    snake[i] = snake[i - 1];
    if (!onFood && x == snake[i].x && y == snake[i].y)                        //If the new head contacts any snake coord, exit and lose
      return false;
    if ((snake[i].x == 0) || (snake[i].y == 0) || (snake[i].x == 15) || (snake[i].y == 7))
      onBorder = true;
  }
  snake[0].x = x;                                                             // If nothing wrong, set the new head of the snake.
  snake[0].y = y;
  if ((x == 0) || (x == 15) || (y == 0) || (y == 7))
    onBorder = true;
  if (!onFood)   {
    drawThing(snake[snakeLength].x, snake[snakeLength].y, ERASE);             // If no food, erase tail
  }
  else
  {
    snakeLength++;                                                            // Else dont erase tail, increment length of snake,
    putFood();                                                                // and put a new food
  }
  if (onBorder)
    borderFull = false;
  else {
    if (!borderFull)
      drawThing(0, 0, BOARD);
  }
  return true;
}

void initGameSnake() {
  gamePhase = INIT;
  gameLevel = 0;
}

void startGame() {
  randomSeed(millis());
  oled.clear();
  drawThing(0, 0, BOARD);
  initSnake();
  putFood();
  timeoutOLED = 500 - (unsigned int)(gameLevel << 6);
  encoderMax = 2;
  encoderValue = 1;
  scrOLED = SCR_GAME;
  updateOLED = true;
}


void drawGameSnake() {
  switch (gamePhase) {
    case INIT:
      oled.clear();
      oled.setFont(Arial_bold_14);
      printCfgOption(3, CFG_GAME_SNAKE);
      gamePhase = WAIT;
      timeoutOLED = ONE_DAY;
      break;
    case WAIT:
      startGame();
      gamePhase = RUN;
      break;
    case RUN:
      if (moveSnake()) {
        if (snakeLength == SNAKE_MAX)
          winGame(true);
      }
      else
        winGame(false);
      break;
  }
}


void winGame(bool win) {
  oled.clear();
  oled.setFont(Arial_bold_14);
  oled.setCursor(30, 3);
  if (win) {
    oled.print(F("Snakes: "));
    gameLevel = (gameLevel + 1) & 0x07;
    oled.print(gameLevel);
  }
  else {
    oled.print(F("Game Over"));
    gameLevel = 0;
  }
  timeoutOLED = 3000;
  gamePhase = INIT;
}
#endif
