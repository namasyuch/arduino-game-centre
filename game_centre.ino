#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define DATA_PIN 11
#define CS_PIN   10
#define CLK_PIN  13

MD_Parola display = MD_Parola(
  HARDWARE_TYPE,
  DATA_PIN,
  CLK_PIN,
  CS_PIN,
  MAX_DEVICES
);

// Get the actual matrix controller
MD_MAX72XX *matrix;

// Joystick
const int joyX = A0;
const int joyY = A1;
const int button = 3;

// Games
#define MENU 0
#define CATCH 1
#define SNAKE 2
#define DODGE 3
#define STACK 4

int game = MENU;
int menuSelection = CATCH;

// TEXT

void showText(String text)
{
  display.displayClear();

  display.displayText(
    text.c_str(),
    PA_CENTER,
    60,
    500,
    PA_SCROLL_LEFT,
    PA_SCROLL_LEFT
  );

  while (!display.displayAnimate())
  {
  }

  display.displayClear();
}

// BUTTON

void waitButtonRelease()
{
  while (digitalRead(button) == LOW)
  {
    delay(10);
  }
}

// MENU

void displayMenuSelection()
{
  if (menuSelection == CATCH)
    showText("CATCH DOT");

  else if (menuSelection == SNAKE)
    showText("SNAKE");

  else if (menuSelection == DODGE)
    showText("DODGE");

  else if (menuSelection == STACK)
    showText("STACK");
}


void menuLoop()
{
  int y = analogRead(joyY);

  if (y < 350)
  {
    menuSelection++;

    if (menuSelection > STACK)
      menuSelection = CATCH;

    displayMenuSelection();
    delay(250);
  }

  if (y > 700)
  {
    menuSelection--;

    if (menuSelection < CATCH)
      menuSelection = STACK;

    displayMenuSelection();
    delay(250);
  }

  if (digitalRead(button) == LOW)
  {
    waitButtonRelease();

    displayMenuSelection();
    delay(300);

    showText("GET READY");
    delay(300);

    game = menuSelection;
  }
}

// GAME OVER

void showGameOver(int score)
{
  showText("GAME OVER");

  String scoreText = "SCORE " + String(score);
  showText(scoreText);

  delay(300);

  game = MENU;
  menuSelection = CATCH;

  displayMenuSelection();
}

// CATCH DOT

int playerX;
int playerY;

int targetX;
int targetY;

int catchScore;

unsigned long catchStartTime;


void drawCatch()
{
  matrix->clear();

  matrix->setPoint(
    playerY,
    playerX,
    true
  );

  matrix->setPoint(
    targetY,
    targetX,
    true
  );

  matrix->update();
}


void newCatchTarget()
{
  targetX = random(0, 32);
  targetY = random(0, 8);

  if (targetX == playerX &&
      targetY == playerY)
  {
    newCatchTarget();
  }
}


void startCatch()
{
  playerX = 2;
  playerY = 4;

  catchScore = 0;

  catchStartTime = millis();

  newCatchTarget();

  while (true)
  {
    int x = analogRead(joyX);
    int y = analogRead(joyY);

    if (x < 350)
      playerX--;

    if (x > 700)
      playerX++;

    if (y < 350)
      playerY--;

    if (y > 700)
      playerY++;

    playerX = constrain(playerX, 0, 31);
    playerY = constrain(playerY, 0, 7);

    if (playerX == targetX &&
        playerY == targetY)
    {
      catchScore++;
      newCatchTarget();
    }

    drawCatch();

    if (millis() - catchStartTime >= 30000)
    {
      matrix->clear();
      matrix->update();

      showGameOver(catchScore);
      return;
    }

    delay(120);
  }
}

// SNAKE

#define MAX_SNAKE 100

int snakeX[MAX_SNAKE];
int snakeY[MAX_SNAKE];

int snakeLength;

int snakeFoodX;
int snakeFoodY;

int snakeDirX;
int snakeDirY;

int snakeScore;


void newSnakeFood()
{
  snakeFoodX = random(0, 32);
  snakeFoodY = random(0, 8);

  for (int i = 0; i < snakeLength; i++)
  {
    if (snakeX[i] == snakeFoodX &&
        snakeY[i] == snakeFoodY)
    {
      newSnakeFood();
      return;
    }
  }
}


bool snakeHitSelf(int x, int y)
{
  for (int i = 1; i < snakeLength; i++)
  {
    if (snakeX[i] == x &&
        snakeY[i] == y)
    {
      return true;
    }
  }

  return false;
}


void drawSnake()
{
  matrix->clear();

  for (int i = 0; i < snakeLength; i++)
  {
    matrix->setPoint(
      snakeY[i],
      snakeX[i],
      true
    );
  }

  matrix->setPoint(
    snakeFoodY,
    snakeFoodX,
    true
  );

  matrix->update();
}


void startSnake()
{
  snakeLength = 3;

  snakeX[0] = 10;
  snakeY[0] = 4;

  snakeX[1] = 9;
  snakeY[1] = 4;

  snakeX[2] = 8;
  snakeY[2] = 4;

  snakeDirX = 1;
  snakeDirY = 0;

  snakeScore = 0;

  newSnakeFood();

  while (true)
  {
    int x = analogRead(joyX);
    int y = analogRead(joyY);

    // LEFT
    if (x < 350 && snakeDirX != 1)
    {
      snakeDirX = -1;
      snakeDirY = 0;
    }

    // RIGHT
    if (x > 700 && snakeDirX != -1)
    {
      snakeDirX = 1;
      snakeDirY = 0;
    }

    // UP
    if (y < 350 && snakeDirY != 1)
    {
      snakeDirX = 0;
      snakeDirY = -1;
    }

    // DOWN
    if (y > 700 && snakeDirY != -1)
    {
      snakeDirX = 0;
      snakeDirY = 1;
    }

    int newX = snakeX[0] + snakeDirX;
    int newY = snakeY[0] + snakeDirY;

    // WALL
    if (newX < 0 ||
        newX >= 32 ||
        newY < 0 ||
        newY >= 8)
    {
      showGameOver(snakeScore);
      return;
    }

    // SELF
    if (snakeHitSelf(newX, newY))
    {
      showGameOver(snakeScore);
      return;
    }

    bool ateFood =
      (newX == snakeFoodX &&
       newY == snakeFoodY);

    if (ateFood)
    {
      if (snakeLength < MAX_SNAKE)
        snakeLength++;

      snakeScore++;

      newSnakeFood();
    }

    // MOVE BODY
    for (int i = snakeLength - 1; i > 0; i--)
    {
      snakeX[i] = snakeX[i - 1];
      snakeY[i] = snakeY[i - 1];
    }

    snakeX[0] = newX;
    snakeY[0] = newY;

    drawSnake();

    int speed = 180 - snakeScore * 5;

    if (speed < 70)
      speed = 70;

    delay(speed);
  }
}

// DODGE

int dodgePlayerX;
int dodgePlayerY;

int obstacleX;
int obstacleY;

int dodgeScore;

unsigned long dodgeLastMove;


void drawDodge()
{
  matrix->clear();

  matrix->setPoint(
    dodgePlayerY,
    dodgePlayerX,
    true
  );

  matrix->setPoint(
    obstacleY,
    obstacleX,
    true
  );

  matrix->update();
}


void startDodge()
{
  dodgePlayerX = 3;
  dodgePlayerY = 4;

  obstacleX = 31;
  obstacleY = random(0, 8);

  dodgeScore = 0;

  dodgeLastMove = millis();

  while (true)
  {
    int y = analogRead(joyY);

    if (y < 350)
      dodgePlayerY--;

    if (y > 700)
      dodgePlayerY++;

    dodgePlayerY = constrain(
      dodgePlayerY,
      0,
      7
    );

    int speed = 250 - dodgeScore * 8;

    if (speed < 70)
      speed = 70;

    if (millis() - dodgeLastMove >= speed)
    {
      dodgeLastMove = millis();

      obstacleX--;

      if (obstacleX < 0)
      {
        obstacleX = 31;
        obstacleY = random(0, 8);

        dodgeScore++;
      }
    }

    if (obstacleX == dodgePlayerX &&
        obstacleY == dodgePlayerY)
    {
      showGameOver(dodgeScore);
      return;
    }

    drawDodge();

    delay(30);
  }
}

// STACK

byte stackBoard[8][32];

int pieceX;
int pieceY;

int pieceType;

int stackScore;

int fallSpeed;

unsigned long lastFall;


// 7 TETRIS-STYLE PIECES

const byte shapes[7][4][4] =
{
  // I
  {
    {1,1,1,1},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0}
  },

  // O
  {
    {1,1,0,0},
    {1,1,0,0},
    {0,0,0,0},
    {0,0,0,0}
  },

  // L
  {
    {1,0,0,0},
    {1,0,0,0},
    {1,1,0,0},
    {0,0,0,0}
  },

  // J
  {
    {0,1,0,0},
    {0,1,0,0},
    {1,1,0,0},
    {0,0,0,0}
  },

  // T
  {
    {1,1,1,0},
    {0,1,0,0},
    {0,0,0,0},
    {0,0,0,0}
  },

  // S
  {
    {0,1,1,0},
    {1,1,0,0},
    {0,0,0,0},
    {0,0,0,0}
  },

  // Z
  {
    {1,1,0,0},
    {0,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
  }
};


void clearBoard()
{
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 32; x++)
    {
      stackBoard[y][x] = 0;
    }
  }
}


bool canMovePiece(int dx, int dy)
{
  for (int y = 0; y < 4; y++)
  {
    for (int x = 0; x < 4; x++)
    {
      if (shapes[pieceType][y][x])
      {
        int nx = pieceX + x + dx;
        int ny = pieceY + y + dy;

        if (nx < 0 || nx >= 32)
          return false;

        if (ny >= 8)
          return false;

        if (ny >= 0 &&
            stackBoard[ny][nx])
        {
          return false;
        }
      }
    }
  }

  return true;
}


void drawStack()
{
  matrix->clear();

  // Existing blocks
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 32; x++)
    {
      if (stackBoard[y][x])
      {
        matrix->setPoint(
          y,
          x,
          true
        );
      }
    }
  }

  // Current piece
  for (int y = 0; y < 4; y++)
  {
    for (int x = 0; x < 4; x++)
    {
      if (shapes[pieceType][y][x])
      {
        int px = pieceX + x;
        int py = pieceY + y;

        if (px >= 0 &&
            px < 32 &&
            py >= 0 &&
            py < 8)
        {
          matrix->setPoint(
            py,
            px,
            true
          );
        }
      }
    }
  }

  matrix->update();
}


void placePiece()
{
  for (int y = 0; y < 4; y++)
  {
    for (int x = 0; x < 4; x++)
    {
      if (shapes[pieceType][y][x])
      {
        int px = pieceX + x;
        int py = pieceY + y;

        if (px >= 0 &&
            px < 32 &&
            py >= 0 &&
            py < 8)
        {
          stackBoard[py][px] = 1;
        }
      }
    }
  }
}


void clearRows()
{
  for (int y = 7; y >= 0; y--)
  {
    bool full = true;

    for (int x = 0; x < 32; x++)
    {
      if (!stackBoard[y][x])
      {
        full = false;
        break;
      }
    }

    if (full)
    {
      for (int yy = y; yy > 0; yy--)
      {
        for (int x = 0; x < 32; x++)
        {
          stackBoard[yy][x] =
            stackBoard[yy - 1][x];
        }
      }

      for (int x = 0; x < 32; x++)
      {
        stackBoard[0][x] = 0;
      }

      stackScore += 10;

      y++;
    }
  }
}


bool createNewPiece()
{
  pieceType = random(0, 7);

  pieceX = 14;
  pieceY = 0;

  if (!canMovePiece(0, 0))
  {
    return false;
  }

  return true;
}


void startStack()
{
  clearBoard();

  stackScore = 0;

  fallSpeed = 500;

  if (!createNewPiece())
  {
    showGameOver(stackScore);
    return;
  }

  lastFall = millis();

  while (true)
  {
    int x = analogRead(joyX);

    // LEFT
    if (x < 350)
    {
      if (canMovePiece(-1, 0))
        pieceX--;

      delay(100);
    }

    // RIGHT
    if (x > 700)
    {
      if (canMovePiece(1, 0))
        pieceX++;

      delay(100);
    }

    // PRESS = DROP
    if (digitalRead(button) == LOW)
    {
      waitButtonRelease();

      while (canMovePiece(0, 1))
      {
        pieceY++;
      }

      placePiece();

      clearRows();

      fallSpeed -= 10;

      if (fallSpeed < 100)
        fallSpeed = 100;

      if (!createNewPiece())
      {
        showGameOver(stackScore);
        return;
      }

      lastFall = millis();
    }

    // AUTO FALL
    if (millis() - lastFall >= fallSpeed)
    {
      lastFall = millis();

      if (canMovePiece(0, 1))
      {
        pieceY++;
      }
      else
      {
        placePiece();

        clearRows();

        fallSpeed -= 10;

        if (fallSpeed < 100)
          fallSpeed = 100;

        if (!createNewPiece())
        {
          showGameOver(stackScore);
          return;
        }
      }
    }

    drawStack();

    delay(20);
  }
}

// SETUP

void setup()
{
  pinMode(button, INPUT_PULLUP);

  display.begin();

  display.setIntensity(3);

  display.displayClear();

  // Get matrix controller
  matrix = display.getGraphicObject();

  randomSeed(analogRead(A3));

  game = MENU;
  menuSelection = CATCH;

  // Startup
  showText("WELCOME");
  delay(300);

  showText("GAME CENTRE");
  delay(300);

  displayMenuSelection();
}

// MAIN LOOP

void loop()
{
  if (game == MENU)
  {
    menuLoop();
  }

  else if (game == CATCH)
  {
    startCatch();
  }

  else if (game == SNAKE)
  {
    startSnake();
  }

  else if (game == DODGE)
  {
    startDodge();
  }

  else if (game == STACK)
  {
    startStack();
  }
}
