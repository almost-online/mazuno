#include <Wire.h>
#include <U8g2lib.h>  // u8g2 library for drawing on OLED display - needs to be installed in Arduino IDE first


U8G2_SH1107_PIMORONI_128X128_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);  // final display, 128x128px [page buffer, size = 128 bytes], HW IIC connection

#define JOYSTICK_X A0
#define JOYSTICK_Y A1
#define BUZZER_PIN 2

#define screen_width 128
#define screen_height 128

#define width 15 // should be odd number
#define height 13 // should be odd number

#define block_size 8
#define menu_height 14
#define NODE_COUNT (width * height)
#define shift_x (screen_width - width * block_size) / 2
#define shift_y (screen_height - height * block_size)

#define STEPS_LIMIT 100


typedef struct
{
  byte x, y;      //Node position - less memmory, but faster initialization
  void *parent;  //link to the parent
  char c;        //maze symbol, 0-free, 1 - wall, 2-star....
  char dirs;     //possible direction
} Node;

Node nodes[NODE_COUNT];  //Array of nodes

byte x, y;
byte step_limit = STEPS_LIMIT;
char score[2] = { 0, 0 };

void beep(int d = 1) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(d);
  digitalWrite(BUZZER_PIN, LOW);
}

void draw() {
  byte i, j;
  Node n;

  //  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.setFont(u8g2_font_8x13_t_symbols);
  u8g2.drawGlyph(8, menu_height, 0x2603);  /* hex 2603 Snowman */
  u8g2.drawGlyph(54, menu_height, 0x2605); /* hex 2605 Star */
  u8g2.drawGlyph(94, menu_height, 0x2661); /* hex 2661 Heart */

  u8g2.setCursor(20, menu_height);
  u8g2.print(step_limit > 0 ? step_limit : 0);
  u8g2.setCursor(64, menu_height);
  u8g2.print(score[0] > 0 ? score[0] : 0);
  u8g2.setCursor(104, menu_height);
  u8g2.print(score[1] > 0 ? score[1] : 0);

  // draw walls
  // u8g2.drawBox(0, menu_height, (width+2)*block_size, block_size);
  // u8g2.drawBox(0, (height+1)*block_size + menu_height, (width+2)*block_size, block_size);
  // u8g2.drawBox(0, menu_height + 2*block_size, block_size, height*block_size);
  // u8g2.drawBox((width+1)*block_size,  block_size + menu_height, block_size, (height - 1) * block_size);
  u8g2.setFont(u8g2_font_6x12_t_symbols);

  for (i = 0; i < width; i++) {
    for (j = 0; j < height; j++) {
      n = nodes[i + j * width];
      switch (n.c) {
        case 1:
          // if (j == 0)
          //  write half block
          //  u8g2.drawBox(n.x * block_size + shift_x, shift_y + n.y * block_size + block_size / 2, block_size, block_size / 2);
          //else
            u8g2.drawBox(n.x * block_size + shift_x, shift_y + n.y * block_size, block_size, block_size);
          break;
        case 2:
          //u8g2.drawCircle(n.x*block_size+block_size/2 + shift, menu_height+n.y*block_size+block_size/2, block_size/2-2);
          u8g2.drawGlyph(n.x * block_size + shift_x + 1, shift_y + (n.y + 1) * block_size - 1, 0x2605); /* hex 2605 Star */
          break;
        case 3:
          //u8g2.drawDisc(n.x*block_size+block_size/2 + shift, menu_height+n.y*block_size+block_size/2, block_size/2-2);
          u8g2.drawGlyph(n.x * block_size + shift_x + 1, shift_y + (n.y + 1) * block_size - 1, 0x2661); /* hex 2661 Heart */
          break;
      }
    }
  }

  u8g2.drawGlyph(x * block_size + shift_x + 1, shift_y + (y + 1) * block_size - 1, 0x2603); /* hex 2603 Snowman */
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
}

void gameOver() {
  u8g2.clearBuffer();
  u8g2.firstPage();
  do {
    u8g2.setFontPosCenter();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setCursor(30, 30);
    u8g2.print(F("Game"));
    u8g2.setCursor(34, 60);
    u8g2.print(F("Over!"));
  } while (u8g2.nextPage());

  digitalWrite(BUZZER_PIN, HIGH);
  delay(10);
  digitalWrite(BUZZER_PIN, LOW);
  delay(10);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(20);
  digitalWrite(BUZZER_PIN, LOW);
  delay(20);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(30);
  digitalWrite(BUZZER_PIN, LOW);
  delay(10);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(20);
  digitalWrite(BUZZER_PIN, LOW);

  delay(5000);
  startGame();
}

void openExit() {
  
  Node *n;
  // get wall nuber
  byte i = random(1, height);
  // if even
  if (i % 2) {
    // x = width-1, y = i
    n = nodes + width - 1 + i * width;
    n->c = 0;
    n->dirs = 1;  // Allow UP

  } else {
    // get even nuber
    i = ((random(1, width) / 2) * 2) + 1;
    // x = i, y = height - 1
    n = nodes + i + (height - 1) * width;
    n->c = 0;
    n->dirs = 8;  // Allow Right
  }
  beep(18);
}

void loop() {
  if (step_limit <= 0)
    gameOver();

  if (score[0] == 0) {
    score[0] = -1;  // the exist is awaliable
    openExit();
  }

  // get end
  if (x == width - 1 || y == height - 1) {
    startGame();
  }

  //Joystick Control
  int xVal = analogRead(JOYSTICK_X);
  int yVal = analogRead(JOYSTICK_Y);

  if (xVal < 150 && x < width - 1 && nodes[x + 1 + y * width].c != 1) {  // Right
    x++;
    beep(2);
    step_limit--;
  } else if (xVal > 850 && x > 0 && nodes[x - 1 + y * width].c != 1) {  // Left
    x--;
    beep(1);
    step_limit--;
  } else if (yVal > 850 && y < height - 1 && nodes[x + (y + 1) * width].c != 1) {  // Down
    y++;
    beep(2);
    step_limit--;
  } else if (yVal < 150 && y > 0 && nodes[x + (y - 1) * width].c != 1) {  // UP
    y--;
    beep(1);
    step_limit--;
  }

  Node *n;
  n = nodes + x + y * width;
  // when pic the star
  if (n->c == 2) {
    score[0]--;
    beep(4);
    beep(6);
    n->c = 0;
    // when poc the heart
  } else if (n->c == 3) {
    score[1]--;
    beep(6);
    beep(4);
    beep(5);
    n->c = 0;
    // increase steps
    step_limit += 4;
  }


  u8g2.firstPage();  // select the first page of the display (page is 128x8px), since we are using the page drawing method of the u8g2 library
  do {
    draw();
  } while (u8g2.nextPage());

  //delay(10);
}

void startGame() {
  Node *start, *last;

  grid_init();

  start = nodes + 1 + width;
  start->parent = start;
  last = start;
  while ((last = link(last)) != start)
    ;
  x = 0;
  y = 1;
  step_limit = STEPS_LIMIT;
}

Node *link(Node *n) {
  //Connect node to a random neigbour
  // and return next node
  byte x, y;
  char dir;
  Node *dest;

  //Nothing
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
      case 1:
        if (n->x + 2 < width) {
          x = n->x + 2;
          y = n->y;
        } else continue;
        break;

      // Down
      case 2:
        if (n->y + 2 < height) {
          x = n->x;
          y = n->y + 2;
        } else continue;
        break;

      // Left
      case 4:
        if (n->x - 2 >= 0) {
          x = n->x - 2;
          y = n->y;
        } else continue;
        break;

      // UP
      case 8:
        if (n->y - 2 >= 0) {
          x = n->x;
          y = n->y - 2;
        } else continue;
        break;
    }

    // GET node using pointer for speeding up a performance
    dest = nodes + x + y * width;

    //  Be sure not a wall
    if (dest->c != 1) {
      if (dest->parent != NULL) continue;

      // set parent as current node
      dest->parent = n;

      // remove wall
      nodes[n->x + (x - n->x) / 2 + (n->y + (y - n->y) / 2) * width].c = 0;

      return dest;
    }
  }

  return n->parent;
}

void grid_init() {
  int i, j;
  Node *n;

  // reset score
  score[0] = 0;
  score[1] = 0;

  for (i = 0; i < width; i++) {
    for (j = 0; j < height; j++) {
      n = nodes + i + j * width;
      // set clean way
      if (i * j % 2) {
        n->c = (random(1, 4) + 1) % 4;
        if (n->c > 0)
          score[n->c - 2]++;

        n->dirs = 15;
        // start point
      } else if (i == 0 && j == 1) {
        n->c = 0; // clean
        n->dirs = 1;  // Right only
      } else {
        n->c = 1; // wall
        n->dirs = 0;  // (the) Wall
      }
      n->x = i;
      n->y = j;
      n->parent = nullptr;
    }
  }
}
