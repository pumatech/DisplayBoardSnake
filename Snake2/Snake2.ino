#include <Adafruit_NeoPixel.h>
#define DI 6
#define ROWS 44
#define COLS 74
#define NUM (ROWS * COLS)
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
#define MAXLEN 100

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM, DI, NEO_GRB + NEO_KHZ800);

// snakeCol and snakeRow are arrays of coordinates for each segment of the snake
// the head is located by headIndex and the other segments come *before* the head
// in reverse order and wrap around to the end of the array if needed

// there are len segments in the snake at any given time

int snakeCol[MAXLEN] = {0};  // initialize to all 0s
int snakeRow[MAXLEN] = {0};  // initialize to all 0s

int headIndex;    // index of snake head
int len;          // currnet snake length
int dir;          // current direction of motion

// location of food (initially in corner)
int foodCol = 0;
int foodRow = 0;

void setup() {
  Serial.begin(9600);

  pixels.begin();
  pixels.setBrightness(128);
  randomSeed(analogRead(5));

  newSnake();
  drawSnake();
}

void loop() {
  checkFood();
  dir = decideDirection();
  moveSnake();
}

// relocates the food to a random location
void moveFood () {
  do {
    foodCol = random(COLS-10) + 5;
    foodRow = random(ROWS-10) + 5;
  }
  while (isInSnake(foodCol, foodRow, true));
}

// creates a short snake in the center of the play area
void newSnake () {
  moveFood();
  for (int i = 0; i < 4; i++) {
    snakeCol[i] = COLS / 2;
    snakeRow[i] = ROWS / 2 - i;
  }
  headIndex = 3;
  len = 4;
  dir = DOWN;
}

// check to see if the snake has eaten the food.  If so, the snake gets longer
// by 1 and the food is moed to a new location
void checkFood() {
  if (snakeCol[headIndex] == foodCol && snakeRow[headIndex] == foodRow) {
    len = min(len + 1, MAXLEN);
    moveFood();
  }
}

// decides which direction to move the snake

int decideDirection () {
  int tryDir;
  int prefs[4] = {0, 0, 0, 0};

  int adjCol = snakeCol[headIndex] == foodCol ? 0 : (foodCol - snakeCol[headIndex]) / abs(foodCol - snakeCol[headIndex]);
  int adjRow = snakeRow[headIndex] == foodRow ? 0 : (foodRow - snakeRow[headIndex]) / abs(foodRow - snakeRow[headIndex]);

  // row is correct - go left or right
  if (adjRow == 0) {
    if (adjCol < 0) {
      prefs[0] = LEFT; prefs[3] = RIGHT;
    }
    else {
      prefs[0] = RIGHT; prefs[3] = LEFT;
    }
    // if can't go left or right, randomly go up/down
    if (random(2)==0) {
      prefs[1] = UP;
      prefs[2] = DOWN;
    }
    else {
      prefs[1] = DOWN;
      prefs[2] = UP;
    }
  }
  // incorrect row - try up/down first then left/right
  else {
    // try up
    if (adjRow < 0) {
      if (adjCol < 0) {
         prefs[0] = UP; prefs[1]=LEFT; 
         if (random(2) == 0) {
            prefs[2] = DOWN; prefs[3] = RIGHT;
         }
         else {
            prefs[2] = RIGHT; prefs[3] = DOWN;
         }
      }
      else {
         prefs[0] = UP; prefs[1]=RIGHT; 
         if (random(2) == 0) {
            prefs[2] = DOWN; prefs[3] = LEFT;
         }
         else {
            prefs[2] = LEFT; prefs[3] = DOWN;
         }
      }
    }
    // try down
    else {
       if (adjCol < 0) {
         prefs[0] = DOWN; prefs[1]=LEFT; 
         if (random(2) == 0) {
            prefs[2] = UP; prefs[3] = RIGHT;
         }
         else {
            prefs[2] = RIGHT; prefs[3] = UP;
         }
      }
      else {
         prefs[0] = DOWN; prefs[1]=RIGHT; 
         if (random(2) == 0) {
            prefs[2] = UP; prefs[3] = LEFT;
         }
         else {
            prefs[2] = LEFT; prefs[3] = UP;
         }
      }
    }
  }

  for (int i=0; i<4; i++) {
    int nextCol = snakeCol[headIndex];
    int nextRow = snakeRow[headIndex];
    if (prefs[i] == UP) nextRow = max(nextRow-1, 0);
    else if (prefs[i] == DOWN) nextRow = min(nextRow+1, ROWS-1);
    else if (prefs[i] == LEFT) nextCol = max (nextCol-1, 0);
    else nextCol = min (nextCol+1, COLS-1);

    if (!isInSnake(nextCol, nextRow, true)) return prefs[i];
  }
  return prefs[0]; // nothing avail - this will crash!
}

// takes a coordinate and checks to see if it is in the snake or not
// used to see if the head has "hit" any part of the snake
boolean isInSnake (int c, int r, boolean includeHead) {
  int start = includeHead? 0: 1;
  for (int i = start; i < len; i++) {
    int index = (headIndex - i + MAXLEN) % MAXLEN;
    if (c == snakeCol[index] && r == snakeRow[index]) {
      return true;
    }
  }
  return false;
}

// moves the snake - no need to shift all the coordinates, just place the next head
// location just after the current head location.  Since the snake is drawn using len
// this will have the effect of adding to the front and removing from the tail
void moveSnake () {
  int nextIndex = (headIndex + 1) % MAXLEN;  // be sure to loop around!
  if (dir == UP) {
    snakeCol[nextIndex] = snakeCol[headIndex];
    snakeRow[nextIndex] = (snakeRow[headIndex] - 1) % ROWS;
  }
  else if (dir == RIGHT) {
    snakeCol[nextIndex] = (snakeCol[headIndex] + 1) % COLS;
    snakeRow[nextIndex] = snakeRow[headIndex];
  }
  else if (dir == DOWN) {
    snakeCol[nextIndex] = snakeCol[headIndex];
    snakeRow[nextIndex] = (snakeRow[headIndex] + 1) % ROWS;
  }
  if (dir == LEFT) {
    snakeCol[nextIndex] = (snakeCol[headIndex] - 1) % COLS;
    snakeRow[nextIndex] = snakeRow[headIndex];
  }
  headIndex = nextIndex;

  drawSnake();
}

// draws the snake and food location
void drawSnake () {

  pixels.clear();
  // draw segments
  for (int i = 1; i < len; i++) {
    int index = (headIndex - i + MAXLEN) % MAXLEN;
    pixels.setPixelColor (getPixel(snakeCol[index], snakeRow[index]), wheel ((int)(i * 255.0 / MAXLEN)));
  }
  // show food
  pixels.setPixelColor (getPixel(foodCol, foodRow), 255, 255, 255);

  // show head
  pixels.setPixelColor (getPixel(snakeCol[headIndex], snakeRow[headIndex]), 255, 255, 255);

  pixels.show();

  // check  to see if the snake has hit itself
  // if so, wait and start over with a small snake
  if (isInSnake (snakeCol[headIndex], snakeRow[headIndex], false)) {
    delay (3000);
    newSnake();
  }
}

/*
  int getPixel (int c, int r) {
  if (r == -1) r = ROWS - 1;
  else if (r == ROWS) r = 0;
  if (c == -1) c = COLS - 1;
  else if (c == COLS) c = 0;
  return r * COLS + ((r % 2 == 0) ? c : (COLS - c - 1));
  }
*/
int getPixel (int c, int r) {
  r = min(r, ROWS-1); 
  c = min(c, COLS-1);
  // even rows go forwards, odd rows go backwards
  return r * COLS + ((r % 2 == 0) ? c : (COLS - c - 1));
}


// mix rgb across a color wheel
// WheelPos is 0 - 255
// returns a blend of 2 colors out of 3 rgb

uint32_t wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
