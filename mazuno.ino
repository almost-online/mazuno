#include <U8g2lib.h>  // u8g2 library for drawing on OLED display - needs to be installed in Arduino IDE first
#include <Wire.h>

//U8G2_SH1107_128X128_1_HW_I2C u8g2(U8G2_R0);  // final display, 128x128px [page buffer, size = 128 bytes], HW IIC connection
U8G2_SH1107_PIMORONI_128X128_1_HW_I2C u8g2(U8G2_R0, /*reset=*/U8X8_PIN_NONE);  // final display, 128x128px [page buffer, size = 128 bytes], HW IIC connection

#define JOYSTICK_X A0
#define JOYSTICK_Y A1

#define JOYSTICK_ORIENTATION 0  // 0 - default, 1 - swapped X, 2 - swapped Y, 3 - swapped both

#if JOYSTICK_ORIENTATION & 1
#define JOYSTICK_RIGHT (xVal > 850)
#define JOYSTICK_LEFT (xVal < 150)
#else
#define JOYSTICK_RIGHT (xVal < 150)
#define JOYSTICK_LEFT (xVal > 850)
#endif

#if JOYSTICK_ORIENTATION & 2
#define JOYSTICK_UP (yVal > 850)
#define JOYSTICK_DOWN (yVal < 150)
#else
#define JOYSTICK_UP (yVal < 150)
#define JOYSTICK_DOWN (yVal > 850)
#endif

#define BUZZER_PIN 2


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

#define WIDTH 15   // !should be odd number
#define HEIGHT 13  // !should be odd number

#define BLOCK_SIZE 8
#define MENU_HEIGHT 16
#define NODE_COUNT (WIDTH * HEIGHT)
#define SHIFT_X (SCREEN_WIDTH - WIDTH * BLOCK_SIZE) / 2
#define SHIFT_Y (SCREEN_HEIGHT - HEIGHT * BLOCK_SIZE)

#define STEPS_LIMIT 100

#define SNOWMAN 0x2603 /* hex 2603 Snowman */
#define STAR 0x2605    /* hex 2605 Star */
#define HEART 0x2661   /* hex 2661 Heart */
#define CROSS 0x2617   /* hex 2617 Cross */

#define D_RIGHT 1
#define D_DOWN 2
#define D_LEFT 4
#define D_UP 8

#define MAX_BOARD_SIZE 5
#define NO_PARENT 255

typedef struct {
  byte x, y;     // Node position - less memmory, but faster initialization
  void* parent;  // link to the parent
  char c;        // maze symbol, 0-free, 1 - wall, 2-star....
  char dirs;     // possible direction
} Node;

Node nodes[NODE_COUNT];  // Array of nodes

typedef struct {
  char.level;
  unsigned int score;
} ScoreBoard;

ScoreBoard scoreboard[MAX_BOARD_SIZE] = {};

byte x = 0, y = 1;
byte step_limit = STEPS_LIMIT;
char score[2] = { 0, 0 };  // stars, hearts

ScoreBoard total_score = {0, 0};

void beep(int d = 1) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(d);
  digitalWrite(BUZZER_PIN, LOW);
}

void draw() {
  byte i, j;
  Node n;

  drawHeader();

  u8g2.setFont(u8g2_font_6x12_t_symbols);

  for (i = 0; i < WIDTH; i++) {
    for (j = 0; j < HEIGHT; j++) {
      n = nodes[i + j * WIDTH];
      switch (n.c) {
        case 1:
          // if (j == 0)
          //  write half block
          //  u8g2.drawBox(n.x * BLOCK_SIZE + SHIFT_X, SHIFT_Y + n.y *
          //  BLOCK_SIZE + BLOCK_SIZE / 2, BLOCK_SIZE, BLOCK_SIZE / 2);
          // else
          u8g2.drawBox(n.x * BLOCK_SIZE + SHIFT_X, SHIFT_Y + n.y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
          break;
        case 2:
          u8g2.drawGlyph(n.x * BLOCK_SIZE + SHIFT_X + 1, SHIFT_Y + (n.y + 1) * BLOCK_SIZE - 1, STAR);
          break;
        case 3:
          u8g2.drawGlyph(n.x * BLOCK_SIZE + SHIFT_X + 1, SHIFT_Y + (n.y + 1) * BLOCK_SIZE - 1, HEART);
          break;
        case 4:
          if (score[0] <= 0)
            u8g2.drawGlyph(n.x * BLOCK_SIZE + SHIFT_X + 1, SHIFT_Y + (n.y + 1) * BLOCK_SIZE - 1, CROSS);
          break;
      }
    }
  }

  u8g2.drawGlyph(x * BLOCK_SIZE + SHIFT_X + 1, SHIFT_Y + (y + 1) * BLOCK_SIZE - 1, SNOWMAN);
}

void drawHeader() {
  //  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.setFont(u8g2_font_8x13_t_symbols);
  u8g2.drawGlyph(8, MENU_HEIGHT, SNOWMAN); /* hex 2603 Snowman */
  u8g2.drawGlyph(64, MENU_HEIGHT, STAR);   /* hex 2605 star */
  u8g2.drawGlyph(94, MENU_HEIGHT, HEART);  /* hex 2661 heart */

  u8g2.setCursor(20, MENU_HEIGHT);
  u8g2.print(step_limit > 0 ? step_limit : 0);
  u8g2.setCursor(74, MENU_HEIGHT);
  u8g2.print(score[0] > 0 ? score[0] : 0);
  u8g2.setCursor(104, MENU_HEIGHT);
  u8g2.print(score[1] > 0 ? score[1] : 0);
}

void setup() {
  u8g2.begin();           // begin the u8g2 library
  u8g2.setContrast(255);  // set display contrast/brightness
  u8g2.clearDisplay();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(3));

  startGame();
  // openExit();
}

void calcScore() {
  if (score[2]) {
    while (score[0] > 0 || score[1] > 0 || step_limit > 0) {

      if (step_limit > 0) {
        step_limit--;
        total_score.score++;
      }
      // stars
      else if (score[0] > 0) {
        score[0]--;
        if (total_score.score > 0)
            total_score.score--;
      }
      // extra lives
      else if (score[1] > 0) {
        total_score.score += 2;
        score[1]--;
      }


      u8g2.clearBuffer();
      u8g2.firstPage();

      do {
        drawHeader();
        u8g2.setFont(u8g2_font_8x13_t_symbols);
        u8g2.setCursor(16, MENU_HEIGHT + 30);
        u8g2.print(F("Total score:"));
        u8g2.setFont(u8g2_font_ncenB14_tr);
        u8g2.setCursor(50, MENU_HEIGHT + 60);
        u8g2.print(total_score);
        u8g2.setCursor(30, MENU_HEIGHT + 90);
        u8g2.print(F("Level: "));
        u8g2.print(score[2] + 0);
      } while (u8g2.nextPage());

      beep(1);
      // delay(10);
    }

    // write score to scoreboard
    writeScore(total_score);

    delay(1000);
  }
}

void writeScore(ScoreBoard total_score) {
  char cur_size = sizeof(scoreboard) / sizeof(ScoreBoard);
  char i = 0;

  if (cur_size > 0) {
    for (i = cur_size - 1; i >= 0; i--) {
      if (scoreboard[i].score > total_score.score) break;
      scoreboard[i + 1] = scoreboard[i];
    }
  }
  
  scoreboard[i+0] = total_score;
}

void drawStartLevel() {
  calcScore();

  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setCursor(30, 30);
    u8g2.print(F("Level: "));
    u8g2.print(score[2] + 1);

    if (total_score > 0) {
      u8g2.setFont(u8g2_font_7x13_t_symbols);
      u8g2.setCursor(4, 60);
      u8g2.print(F("Total score: "));
      u8g2.print(total_score);
    }
  } while (u8g2.nextPage());


  beep(3);
  delay(20);
  beep(1);
  delay(2000);
}

void gameOver() {
  calcScore();

  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    u8g2.setFontPosCenter();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setCursor(30, 30);
    u8g2.print(F("Game"));
    u8g2.setCursor(34, 60);
    u8g2.print(F("Over!"));
    u8g2.setFont(u8g2_font_8x13_t_symbols);
    u8g2.setCursor(30, 80);
    u8g2.print(F("Level: "));
    u8g2.print(score[2] + 0);
    u8g2.setFont(u8g2_font_7x13_t_symbols);
    u8g2.setCursor(3, 100);
    u8g2.print(F("Total score: "));
    u8g2.print(total_score);
  } while (u8g2.nextPage());

  beep(1);
  delay(10);
  beep(1);
  delay(20);
  beep(30);
  delay(20);
  beep(1);
  delay(10);
  beep(1);
  delay(20);
  beep(30);

  delay(6000);
  x = 0;
  y = 1;
  total_score = 0;
  step_limit = 0;
  score[0] = 0;
  score[1] = 0;
  score[2] = 0;
  startGame();
}

void openExit() {
  Node* n;
  // get wall nuber
  byte i = random(2, HEIGHT + WIDTH - 1);
  // use for right
  if (i <= HEIGHT) {
    // x = WIDTH-1, y = i
    i = (((i / 2) * 2) - 1);
    n = nodes + (WIDTH - 1) + i * WIDTH;
    n->c = 0;
    // use for bottom
  } else {
    i = ((i / 2) * 2) - HEIGHT;
    // x = i, y = HEIGHT - 1
    n = nodes + i + (HEIGHT - 1) * WIDTH;
    n->c = 0;
  }

  beep(18);
}

void loop() {
  if (step_limit <= 0) gameOver();

  if (score[0] == 0) {
    score[0] = -1;  // the exist is awaliable
    openExit();
  }

  // get end
  if (x == WIDTH || y == HEIGHT) {
    if (x == WIDTH)
      x = 0;
    else
      y = 0;

    startGame();
  }

  // Joystick Control
  int xVal = analogRead(JOYSTICK_X);
  int yVal = analogRead(JOYSTICK_Y);
  Node* n;

  if (JOYSTICK_RIGHT && ((x < WIDTH - 1 && nodes[x + 1 + y * WIDTH].c != 1) || x == WIDTH - 1)) {
    // Right
    if (x == 0) {  // start point
      n = nodes + y * WIDTH;
      n->c = 4;  // set cross
    }
    x++;
    beep(1);
    step_limit--;
  } else if (JOYSTICK_LEFT && x > 0 && nodes[x - 1 + y * WIDTH].c != 1) {
    // Left
    x--;
    beep(1);
    step_limit--;
  } else if (JOYSTICK_DOWN && ((y < HEIGHT - 1 && nodes[x + (y + 1) * WIDTH].c != 1) || y == HEIGHT - 1)) {
    // Down
    if (y == 0) {  // start point
      n = nodes + x;
      n->c = 4;  // set cross
    }
    y++;
    beep(1);
    step_limit--;
  } else if (JOYSTICK_UP && y > 0 && nodes[x + (y - 1) * WIDTH].c != 1) {
    // UP
    y--;
    beep(1);
    step_limit--;
  }

  n = nodes + x + y * WIDTH;
  // when pic the star
  if (n->c == 2) {
    score[0]--;
    beep(2);
    beep(1);
    n->c = 0;
    // when pic the heart
  } else if (n->c == 3) {
    score[1]--;
    beep(1);
    beep(2);
    beep(1);
    n->c = 0;
    // increase steps
    step_limit += 4;
  }

  u8g2.firstPage();  // select the first page of the display (page is 128x8px),
                     // since we are using the page drawing method of the u8g2
                     // library
  do {
    draw();
  } while (u8g2.nextPage());

  // delay(10);
}

void startGame() {
  Node *start, *last;
  drawStartLevel();
  grid_init();

  if (x > 0)
    start = nodes + x + WIDTH;
  else
    start = nodes + 1 + y * WIDTH;

  start->parent = start;
  last = start;
  while ((last = link(last)) != start)
    ;
  // x = 0;
  // y = 1;
  step_limit = STEPS_LIMIT;
}

Node* link(Node* n) {
  // Connect node to a random neigbour
  //  and return next node
  byte x_, y_;
  char dir;
  Node* dest;

  // Nothing
  if (n == NULL) return NULL;

  // while an undefined direction exists
  while (n->dirs) {
    // select a random way
    dir = (1 << (random() % 4));

    // continue when had discover
    if (~n->dirs & dir) continue;

    //  set as discovered
    n->dirs &= ~dir;

    switch (dir) {
      // when Right direction is avaliable
      case D_RIGHT:
        if (n->x + 2 < WIDTH) {
          x_ = n->x + 2;
          y_ = n->y;
        } else
          continue;
        break;

      // Down
      case D_DOWN:
        if (n->y + 2 < HEIGHT) {
          x_ = n->x;
          y_ = n->y + 2;
        } else
          continue;
        break;

      // Left
      case D_LEFT:
        if (n->x - 2 >= 0) {
          x_ = n->x - 2;
          y_ = n->y;
        } else
          continue;
        break;

      // UP
      case D_UP:
        if (n->y - 2 >= 0) {
          x_ = n->x;
          y_ = n->y - 2;
        } else
          continue;
        break;
    }

    // GET node using pointer for speeding up a performance
    dest = nodes + x_ + y_ * WIDTH;

    //  Be sure not a wall
    if (dest->c != 1) {
      if (dest->parent != NULL) continue;

      // set parent as current node
      dest->parent = n;

      // remove wall
      nodes[n->x + (x_ - n->x) / 2 + (n->y + (y_ - n->y) / 2) * WIDTH].c = 0;

      return dest;
    }
  }

  return n->parent;
}

void grid_init() {
  int i, j;
  Node* n;

  // reset score
  score[0] = 0;
  score[1] = 0;
  total_score.level++;

  // init from end to start
  for (i = WIDTH - 1; i >= 0; i--) {
    for (j = HEIGHT - 1; j >= 0; j--) {
      n = nodes + i + j * WIDTH;
      // set clean way
      if (i == x && j == y) {
        n->c = 0;  // clean
        if (x > 0)
          n->dirs = 2;  // Down only
        else
          n->dirs = 1;  // Right only
      } else if (i * j % 2) {
        n->c = (random(1, 4) + 1) % 4;

        if (n->c > 0) {
          if (n->c == 2 && score[0] < score[2])
            score[0]++;
          else if (n->c == 3 && score[1] < 2 * score[2])
            score[1]++;
          else
            n->c = 0;
        };

        n->dirs = 15;
        // start point
      } else {
        n->c = 1;     // wall
        n->dirs = 0;  // (the) Wall
      }
      n->x = i;
      n->y = j;
      n->parent = nullptr;
    }
  }
}