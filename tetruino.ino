// Project: Arduino Tetris
// Author: Milan Jelic (Jelka)

#define DEBUG

// Display
#include <Adafruit_ST7735.h>

// display pins
const int TFT_CS  = 10;
const int TFT_RST = -1; // use Arduino reset pin
const int TFT_RS  =  8;

// Use hardware SPI pins: MOSI -> pin 11; SCLK -> pin 13
Adafruit_ST7735 display = Adafruit_ST7735(TFT_CS, TFT_RS, TFT_RST);

// Joystick
const int JoyXPin = A1;
const int JoyYPin = A0;
const int JoyBtnPin = 7;
const uint8_t JoyXFlip = 1;
const uint8_t JoyYFlip = 0;

// UI
const int BOARD_BACKGROUND_COLOR = ST7735_BLACK;
const int SIDE_PANEL_BORDER_COLOR = 0x8410;
const int FONT_SIZE = 7; // when text size = 1, then it's 7px high
const int FONT_WIDTH = 5; // when text size = 1, then it's 5px wide
const int SCORE_POS_Y = 5+FONT_SIZE+ 2*8 +10 +FONT_SIZE+5; // 2*5 is 2*SQUARE_SIZE, but it's declared underneath...
                                                           // and i want this to be here and not below... *:D

// Game
const int SQUARE_SIZE = 8;                       // width of a single square in pixels
const int BOARD_WIDTH = 80;                      // width and height
const int BOARD_HEIGHT = 160;                    //  of the board in pixels
const int BOARD_COLS = BOARD_WIDTH/SQUARE_SIZE;  // number of columns
const int BOARD_ROWS = BOARD_HEIGHT/SQUARE_SIZE; // number of rows
uint8_t board[BOARD_ROWS][BOARD_COLS];           // 2D board array, stores shape_id+1 of placed tetrominoes; (y,x) format

const uint8_t TETROMINOES_SHAPES[7][2][4] = {
  // Straight
  {
    {1,1,1,1}
  },
  // Block
  {
    {1,1},
    {1,1}
  },
  // T-Shape
  {
    {1,1,1},
    {0,1,0}
  },
  // L-Shape
  {
    {0,0,1},
    {1,1,1}
  },
  // Reversed L-Shape
  {
    {1,1,1},
    {0,0,1}
  },
  // Left Zig-Zag
  {
    {1,1,0},
    {0,1,1}
  },
  // Right Zig-Zag
  {
    {0,1,1},
    {1,1,0}
  }
};

const uint16_t TETROMINOES_BASE_COLORS[7] = {
  ST7735_CYAN,
  ST7735_YELLOW,
  ST7735_MAGENTA,
  ST7735_ORANGE,
  ST7735_BLUE,
  ST7735_RED,
  ST7735_GREEN
};

// 50% of base colors
const uint16_t TETROMINOES_SHADE_COLORS[7] = {
  0x0410,
  0x8400,
  0x8010,
  0x8200,
  0x0010,
  0x8000,
  0x0400
};

struct Tetromino {
  uint8_t pos_x = BOARD_COLS/2 - 2;    // board column
  uint8_t pos_y = 0;                   // board row
  uint8_t rotation = 0;                // 0 through 3 times 90 degrees (clockwise)
  uint8_t shape_id = 0;                // one of possible shapes
};

Tetromino tetromino;
uint8_t next_id = 0;
uint16_t score = 0;

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Debug");
  #endif

  // display init
  display.initR(INITR_BLACKTAB);
  display.fillScreen(ST7735_BLACK);

  // set the Joystick button as input (with pullup)
  pinMode(JoyBtnPin, INPUT_PULLUP);

  // draw the main menu
  // matrix that holds shape_id+1 of the color of a square in the logo
  /*const uint8_t logo[5][38] = {
    //  T           E           T           R            U           I          N          O
    {1,1,1,1, 0, 1,1,1,1, 0, 1,1,1,1, 0, 0,0,0,0, 0, 1,0,0,0,1, 0, 0,0,0, 0, 6,6,0,0, 0, 0,0,0},
    {0,0,1,0, 0, 3,0,0,0, 0, 0,0,1,0, 0, 1,5,5,5, 0, 1,0,0,0,1, 0, 3,3,3, 0, 1,6,6,0, 0, 5,5,0},
    {0,0,1,0, 0, 3,3,0,0, 0, 0,0,1,0, 0, 1,0,0,5, 0, 1,0,0,0,1, 0, 0,3,0, 0, 1,0,4,4, 0, 5,0,5},
    {0,0,1,0, 0, 3,0,0,0, 0, 0,0,1,0, 0, 1,0,0,0, 0, 1,5,0,0,1, 0, 0,3,0, 0, 1,0,0,4, 0, 5,0,5},
    {0,0,1,0, 0, 1,1,1,1, 0, 0,0,1,0, 0, 1,0,0,0, 0, 0,5,5,5,0, 0, 3,3,3, 0, 1,0,0,4, 0, 0,5,5}
  }; // simplified version of "Tetruino"; draw with 3x3 px each square*/
  const uint8_t logo[8][61] = {
    //         T                 E                   T                 R               U              I               N               O
    {0,7,7,0,0,6,6,3,3,3, 0, 2,2,5,5,5, 0, 0,7,7,0,0,6,6,3,3,3, 0, 0,6,5,5,5, 0, 2,2,0,0,5,5, 0, 1,1,1,1,7,7, 0, 0,3,0,0,0,0, 0, 0,0,4,5,0,0},
    {7,7,5,5,5,1,6,6,3,0, 0, 2,2,6,0,5, 0, 7,7,5,5,5,1,6,6,3,0, 0, 6,6,0,0,5, 0, 2,2,0,0,5,0, 0, 0,2,2,7,7,0, 0, 3,3,0,0,0,1, 0, 4,4,4,5,5,5},
    {0,0,0,0,5,1,0,0,0,0, 0, 0,6,6,0,0, 0, 0,0,0,0,5,1,0,0,0,0, 0, 6,3,0,2,2, 0, 4,4,0,0,5,6, 0, 0,2,2,5,5,0, 0, 7,3,2,2,0,1, 0, 2,2,0,0,0,1},
    {0,0,0,0,4,1,0,0,0,0, 0, 0,6,4,4,4, 0, 0,0,0,0,4,1,0,0,0,0, 0, 3,3,0,2,2, 0, 0,4,0,0,6,6, 0, 0,0,4,5,0,0, 0, 7,7,2,2,6,1, 0, 2,2,0,0,0,1},
    {0,0,0,0,4,1,0,0,0,0, 0, 0,7,4,0,0, 0, 0,0,0,0,4,1,0,0,0,0, 0, 1,3,4,4,4, 0, 7,4,0,0,6,0, 0, 0,0,4,5,0,0, 0, 0,7,0,6,6,1, 0, 0,6,0,0,0,1},
    {0,0,0,0,4,4,0,0,0,0, 0, 3,7,7,0,0, 0, 0,0,0,0,4,4,0,0,0,0, 0, 1,0,4,7,0, 0, 7,7,0,0,3,0, 0, 0,0,4,4,0,0, 0, 0,5,0,6,4,4, 0, 6,6,0,0,0,1},
    {0,0,0,0,2,2,0,0,0,0, 0, 3,3,7,0,0, 0, 0,0,0,0,2,2,0,0,0,0, 0, 1,0,0,7,7, 0, 0,7,0,3,3,3, 0, 0,3,6,6,0,0, 0, 0,5,0,0,0,4, 0, 6,3,3,3,7,7},
    {0,0,0,0,2,2,0,0,0,0, 0, 3,1,1,1,1, 0, 0,0,0,0,2,2,0,0,0,0, 0, 1,0,0,0,7, 0, 0,1,1,1,1,0, 0, 3,3,3,6,6,0, 0, 5,5,0,0,0,4, 0, 0,0,3,7,7,0}
  }; // detailed version, draw with 2x2 px each square
     // generated with Tetris Font by Erik Demaine and Martin Demaine

  // draw the logo
  for(uint8_t y = 0; y<8; y++){
    for(uint8_t x = 0; x<61; x++){
      if(logo[y][x])
        display.drawRect(2+x*2, y*2+30, 2, 2, TETROMINOES_BASE_COLORS[logo[y][x]-1]);
    }
  }

  // draw the play button
  display.drawRect(32, 100, 64, 24, ST7735_WHITE);
  display.setTextWrap(false);
  display.setTextColor(ST7735_WHITE);
  display.setTextSize(2);
  display.setCursor(32+10, 100+5);
  display.print("PLAY");

  // wait for play button in main menu
  while(digitalRead(JoyBtnPin)){

  }

  // start the game

  display.fillScreen(BOARD_BACKGROUND_COLOR);

  // draw side panel border
  display.drawFastVLine(BOARD_WIDTH, 0, 160, SIDE_PANEL_BORDER_COLOR);

  // set text settings
  display.setTextSize(1);

  // write the side panel text
  // "NEXT:"
  display.setCursor(BOARD_WIDTH+5, 5);
  display.print("NEXT:");
  // "SCORE:"
  display.setCursor(BOARD_WIDTH+5, 5+FONT_SIZE +2*SQUARE_SIZE +10);
  display.print("SCORE:");
  display.setCursor(BOARD_WIDTH+5, SCORE_POS_Y);
  display.print("00000");

  // generate the first two tetrominoes
  randomSeed(millis());
  tetromino.shape_id = random(0,7);
  next_id = random(0,7);

  // draw the next tetromino that will drop
  for(uint8_t y=0; y<2; y++){
    for(uint8_t x=0; x<4; x++){
      if(TETROMINOES_SHAPES[next_id][y][x])
        drawSquare(BOARD_WIDTH+5+x*SQUARE_SIZE, 5+FONT_SIZE+5+y*SQUARE_SIZE, next_id);
    }
  }

  drawTetromino();
}

uint16_t moveTime = 1000; // 1000ms; time between two falls, player can move and rotate here

void loop() {
  // delay(moveTime) but with sideways movement, rotation and fast fall
  unsigned long start_time = millis();
  while((millis() - start_time) < moveTime){
    // check input
    int8_t joy_readingX = readJoyX();
    int8_t joy_readingY = readJoyY();
    
    if(joy_readingX > 10 or joy_readingX < -10){
      // move sideways
      
      uint8_t next_x = tetromino.pos_x;
      if(joy_readingX > 10){
        next_x += 1;
      }
      else if(joy_readingX < -10){
        next_x -= 1;
      }

      // offset to the right when the tetromino goes right
      // otherwise the left most side will come to the edge of the screen with the rest hanging over
      // (it equals the size of shape so that tetromino.pos_x+offset = BOARD_COLS)
      // e.g. straight piece on position 6 will take places [6,7,8,9] all inside BOARD_COLS=10
      uint8_t offsetR = 0;
      switch(tetromino.rotation){
        default:
          break;
        case 0:
        case 2:
          if(tetromino.shape_id == 0)
            offsetR = 4;
          else if(tetromino.shape_id == 1)
            offsetR = 2;
          else
            offsetR = 3;
          break;
        case 1:
        case 3:
          if(tetromino.shape_id == 0)
            offsetR = 1;
          else
            offsetR = 2;
          break;
      }
      
      // check for collision
      uint8_t move_collided = 0;
      if((joy_readingX < 0 and tetromino.pos_x == 0) or (joy_readingX > 0 and tetromino.pos_x+offsetR >= BOARD_COLS)){
        move_collided = 1;
      }

      // loop through the tetromino's shape
      for(int8_t y = 1; y >= 0; y--){
        for(uint8_t x = 0; x < 4; x++){
          // check if there is a square in this part of the tetromino's shape
          if(TETROMINOES_SHAPES[tetromino.shape_id][y][x]){
            // check for already placed square in the future placement of the tetronimo
            switch(tetromino.rotation){
              default:
              case 0:
                if(board[y+tetromino.pos_y][x+next_x])
                  move_collided = 1;
                break;
              case 1:
                {
                  // negative offset because the straight piece has only FIRST row and empty second
                  // so it would be placed one square to the right
                  uint8_t neg_offset = 0;
                  if(tetromino.shape_id == 0)
                    neg_offset = 1;
                  
                  if(board[x+tetromino.pos_y][1-y+next_x-neg_offset])
                    move_collided = 1;
                }
                break;
              case 2:
                // the code block is needed because the declared variables are interfering with case 3
                {
                  // negative offset because shape array has width 4,
                  // but not every shape has 4 squares in width
                  uint8_t neg_offsetX = 0;
                  if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
                    neg_offsetX = 1;
                  else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
                    neg_offsetX = 2;
          
                  // negative offset because straight piece has height 1
                  uint8_t neg_offsetY = 0;
                  if(tetromino.shape_id == 0)
                    neg_offsetY = 1;
                  
                  if(board[1-y+tetromino.pos_y-neg_offsetY][3-x+next_x-neg_offsetX])
                    move_collided = 1;
                }
                break;
              case 3:
                // the code block is needed because the declared variables are interfering with case 2
                {
                  // negative offset because shape array has width 4,
                  // but not every shape has 4 squares in width
                  uint8_t neg_offsetY = 0;
                  if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
                    neg_offsetY = 1;
                  else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
                    neg_offsetY = 2;
                  
                  if(board[3-x+tetromino.pos_y-neg_offsetY][y+next_x])
                    move_collided = 1;
                }
                break;
            }
          }
        }
      }

      // update (move) and redraw the tetromino
      if(!move_collided){
        // fill the previous position of the tetromino with background color
        eraseTetromino();
    
        // update the tetromino
        tetromino.pos_x = next_x;
    
        // redraw the tetromino
        drawTetromino();
      }
    }
    else if(joy_readingY > 10){
      // fast fall
      
      uint8_t next_y = tetromino.pos_y + 1;
      uint8_t neg_offsetY = 0;
    
      // check for collision
      uint8_t fall_collided = 0;
      // loop through the tetromino's shape
      for(int8_t y = 1; y >= 0; y--){
        for(uint8_t x = 0; x < 4; x++){
          // check if there is a square in this part of the tetromino's shape
          if(TETROMINOES_SHAPES[tetromino.shape_id][tetromino.rotation % 2 ? x : y][tetromino.rotation % 2 ? y : x]){
            // check for floor
            if(next_y + (tetromino.rotation % 2 ? x : y) - neg_offsetY >= BOARD_ROWS){
                fall_collided = 1;
            }
            else{
              // check for already placed square in the future placement of the tetronimo
              switch(tetromino.rotation){
                default:
                case 0:
                  if(board[y+next_y][x+tetromino.pos_x])
                    fall_collided = 1;
                  break;
                case 1:
                  {
                    // negative offset because the straight piece has only FIRST row and empty second
                    // so it would be placed one square to the right
                    uint8_t neg_offset = 0;
                    if(tetromino.shape_id == 0)
                      neg_offset = 1;
                    
                    if(board[x+next_y][1-y+tetromino.pos_x-neg_offset])
                      fall_collided = 1;
                  }
                  break;
                case 2:
                  // the code block is needed because the declared variables are interfering with case 3
                  {
                    // negative offset because shape array has width 4,
                    // but not every shape has 4 squares in width
                    uint8_t neg_offsetX = 0;
                    if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
                      neg_offsetX = 1;
                    else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
                      neg_offsetX = 2;
            
                    // negative offset because straight piece has height 1
                    uint8_t neg_offsetY = 0;
                    if(tetromino.shape_id == 0)
                      neg_offsetY = 1;
                    
                    if(board[1-y+next_y-neg_offsetY][3-x+tetromino.pos_x-neg_offsetX])
                      fall_collided = 1;
                  }
                  break;
                case 3:
                  // the code block is needed because the declared variables are interfering with case 2
                  {
                    // negative offset because shape array has width 4,
                    // but not every shape has 4 squares in width
                    uint8_t neg_offsetY = 0;
                    if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
                      neg_offsetY = 1;
                    else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
                      neg_offsetY = 2;
                    
                    if(board[3-x+next_y-neg_offsetY][y+tetromino.pos_x])
                      fall_collided = 1;
                  }
                  break;
              }
            }
          }
        }
      }

      // update (move) and redraw the tetromino
      if(!fall_collided){
        // fill the previous position of the tetromino with background color
        eraseTetromino();
    
        // update the tetromino
        tetromino.pos_y = next_y;
    
        // redraw the tetromino
        drawTetromino();
      }
    }
    else if(joy_readingY < -10){
      // rotate

      uint8_t next_rotation = tetromino.rotation;
      if(tetromino.rotation < 3){
        next_rotation += 1;
      }
      else{
        next_rotation = 0;
      }

      // offset to the right when the tetromino rotates on the right side
      // otherwise the left most side will come to the edge of the screen with the rest hanging over
      // (it equals the size of shape -1 so that tetromino.pos_x+offset = BOARD_COLS-1)
      // e.g. straight piece on position 6 will take places [6,7,8,9] all inside BOARD_COLS=10
      // (copied from above :D and changed so that offsetR is 1 less then the above code)
      uint8_t offsetR = 0;
      switch(next_rotation){
        default:
          break;
        case 0:
        case 2:
          if(tetromino.shape_id == 0)
            offsetR = 3;
          else if(tetromino.shape_id == 1)
            offsetR = 1;
          else
            offsetR = 2;
          break;
        case 1:
        case 3:
          if(tetromino.shape_id != 0)
            offsetR = 2;
          break;
      }
      
      // check for collision after rotate
      uint8_t collided = 0;
      if(tetromino.pos_x+offsetR >= BOARD_COLS)
        collided = 1;
      
      // loop through the tetromino's shape
      for(int8_t y = 0; y < 2; y++){
        for(uint8_t x = 0; x < 4; x++){
          // check if there is a square in this part of the tetromino's shape
          if(TETROMINOES_SHAPES[tetromino.shape_id][y][x]){
            switch(next_rotation){
              default:
              case 0:
                if(board[y+tetromino.pos_y][x+tetromino.pos_x])
                  collided = 1;
                break;
              case 1:
                {
                  // negative offset because the straight piece has only FIRST row and empty second
                  // so it would be placed one square to the right
                  uint8_t neg_offset = 0;
                  if(tetromino.shape_id == 0)
                    neg_offset = 1;
                  
                  if(board[x+tetromino.pos_y][1-y+tetromino.pos_x-neg_offset]){
                    collided = 1;
                  }
                }
                break;
              case 2:
                // the code block is needed because the declared variables are interfering with case 3
                {
                  // negative offset because shape array has width 4,
                  // but not every shape has 4 squares in width
                  uint8_t neg_offsetX = 0;
                  if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
                    neg_offsetX = 1;
                  else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
                    neg_offsetX = 2;
          
                  // negative offset because straight piece has height 1
                  uint8_t neg_offsetY = 0;
                  if(tetromino.shape_id == 0)
                    neg_offsetY = 1;
                  
                  if(board[1-y+tetromino.pos_y-neg_offsetY][3-x+tetromino.pos_x-neg_offsetX])
                    collided = 1;
                }
                break;
              case 3:
                // the code block is needed because the declared variables are interfering with case 2
                {
                  // negative offset because shape array has width 4,
                  // but not every shape has 4 squares in width
                  uint8_t neg_offsetY = 0;
                  if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
                    neg_offsetY = 1;
                  else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
                    neg_offsetY = 2;
                  
                  if(board[3-x+tetromino.pos_y-neg_offsetY][y+tetromino.pos_x])
                    collided = 1;
                }
                break;
            }
          }
        }
      }

      if(!collided){
        eraseTetromino();

        tetromino.rotation = next_rotation;
  
        drawTetromino();
      }
    }

    delay(150);
  }
  
  // due to gravity should fall down
  uint8_t next_y = tetromino.pos_y + 1;

  // check for collision
  uint8_t fall_collided = 0;
  uint8_t neg_offsetX = 0;
  uint8_t neg_offsetY = 0;
  switch(tetromino.rotation){
    default:
      break;
    case 1:
      if(tetromino.shape_id == 0){
        neg_offsetX = 1;
      }
      break;
    case 2:
      if(tetromino.shape_id > 1){
        neg_offsetX = 1;
      }
      else if(tetromino.shape_id == 1){
        neg_offsetX = 2;
      }
      break;
  }
  
  // loop through the tetromino's shape
  for(int8_t y = 0; y < 2; y++){
    for(uint8_t x = 0; x < 4; x++){
      // check if there is a square in this part of the tetromino's shape
      if(TETROMINOES_SHAPES[tetromino.shape_id][y][x]){
            // check for floor
        if(next_y + (tetromino.rotation % 2 ? x : y) - neg_offsetY >= BOARD_ROWS){
            fall_collided = 1;
        }
        else{
          // check for already placed square in the future placement of the tetronimo
          switch(tetromino.rotation){
            default:
            case 0:
              if(board[y+next_y][x+tetromino.pos_x])
                fall_collided = 1;
              break;
            case 1:
              {
                // negative offset because the straight piece has only FIRST row and empty second
                // so it would be placed one square to the right
                uint8_t neg_offset = 0;
                if(tetromino.shape_id == 0)
                  neg_offset = 1;
                
                if(board[x+next_y][1-y+tetromino.pos_x-neg_offset])
                  fall_collided = 1;
              }
              break;
            case 2:
              // the code block is needed because the declared variables are interfering with case 3
              {
                // negative offset because shape array has width 4,
                // but not every shape has 4 squares in width
                uint8_t neg_offsetX = 0;
                if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
                  neg_offsetX = 1;
                else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
                  neg_offsetX = 2;
        
                // negative offset because straight piece has height 1
                uint8_t neg_offsetY = 0;
                if(tetromino.shape_id == 0)
                  neg_offsetY = 1;
                
                if(board[1-y+next_y-neg_offsetY][3-x+tetromino.pos_x-neg_offsetX])
                  fall_collided = 1;
              }
              break;
            case 3:
              // the code block is needed because the declared variables are interfering with case 2
              {
                // negative offset because shape array has width 4,
                // but not every shape has 4 squares in width
                uint8_t neg_offsetY = 0;
                if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
                  neg_offsetY = 1;
                else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
                  neg_offsetY = 2;
                
                if(board[3-x+next_y-neg_offsetY][y+tetromino.pos_x])
                  fall_collided = 1;
              }
              break;
          }
        }
      }
    }
  }

  // update (move) and redraw the tetronimo
  if(!fall_collided){
    // fill the previous position of the tetromino with background color
    eraseTetromino();

    // update the tetromino
    tetromino.pos_y = next_y;

    // redraw the tetromino
    drawTetromino();
  }
  else {
    // put this tetromino on to the 'board'
    switch(tetromino.rotation){
      default:
      case 0:
        for(int8_t y = 0; y<2; y++){
          for(int8_t x = 0; x<4; x++){
            if(TETROMINOES_SHAPES[tetromino.shape_id][y][x]){
              board[y+tetromino.pos_y][x+tetromino.pos_x] = tetromino.shape_id+1;
            }
          }
        }
        break;
      case 1:
        {
          // negative offset because the straight piece has only FIRST row and empty second
          // so it would be placed one square to the right
          uint8_t neg_offset = 0;
          if(tetromino.shape_id == 0)
            neg_offset = 1;
          
          for(int8_t y = 0; y<2; y++){
            for(int8_t x = 0; x<4; x++){
              if(TETROMINOES_SHAPES[tetromino.shape_id][y][x])
                board[x+tetromino.pos_y][1-y+tetromino.pos_x-neg_offset] = tetromino.shape_id+1;
            }
          }
        }
        break;
      case 2:
        // the code block is needed because the declared variables are interfering with case 3
        {
          // negative offset because shape array has width 4,
          // but not every shape has 4 squares in width
          uint8_t neg_offsetX = 0;
          if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
            neg_offsetX = 1;
          else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
            neg_offsetX = 2;
  
          // negative offset because straight piece has height 1
          uint8_t neg_offsetY = 0;
          if(tetromino.shape_id == 0)
            neg_offsetY = 1;
          
          for(int8_t y = 0; y<2; y++){
            for(int8_t x = 3; x>=0; x--){
              if(TETROMINOES_SHAPES[tetromino.shape_id][y][x])
                board[1-y+tetromino.pos_y-neg_offsetY][3-x+tetromino.pos_x-neg_offsetX] = tetromino.shape_id+1;
            }
          }
        }
        break;
      case 3:
        // the code block is needed because the declared variables are interfering with case 2
        {
          // negative offset because shape array has width 4,
          // but not every shape has 4 squares in width
          uint8_t neg_offsetY = 0;
          if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
            neg_offsetY = 1;
          else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
            neg_offsetY = 2;
          
          for(int8_t y = 0; y<2; y++){
            for(int8_t x = 0; x<4; x++){
              if(TETROMINOES_SHAPES[tetromino.shape_id][y][x])
                board[3-x+tetromino.pos_y-neg_offsetY][y+tetromino.pos_x] = tetromino.shape_id+1;
            }
          }
        }
        break;
    }

    // check for filled rows
    uint8_t found_filled = 0;

    // check once, then repeat if found one, because there could be multiple
    // if found nothing, no need to repeat check
    do {
      found_filled = 0;
      
      for(int8_t row = BOARD_ROWS-1; row >= 0; row--){
        uint8_t empty = 0; // was there an empty square?
        for(uint8_t col = 0; col < BOARD_COLS; col++){
          if(!board[row][col]){
            empty = 1;
            break;
          }
        }
        if(!empty){
          // no empty square? then mark found filled, increase the score and run the animation + pull the tip down
          found_filled = 1;

          // increase and print the score
          score++;
          display.fillRect(BOARD_WIDTH+5, SCORE_POS_Y, 5*FONT_WIDTH+5, FONT_SIZE, BOARD_BACKGROUND_COLOR);
          display.setCursor(BOARD_WIDTH+5, SCORE_POS_Y);
          char buf[6]; // it's 6 even tho only 5 digits, because of null terminator
          sprintf(buf, "%05u", score);
          display.print(buf);

          // lower the moveTime
          // (if it isn't 100ms or less for better playability)
          if(moveTime > 100)
            moveTime -= 10;
          
          // blinking row animation
          
            // white row
            display.fillRect(0,row*SQUARE_SIZE, BOARD_WIDTH,SQUARE_SIZE, ST7735_WHITE);
            delay(100);
            // in-color row
            for(uint8_t col = 0; col < BOARD_COLS; col++){
              drawSquare(col*SQUARE_SIZE, row*SQUARE_SIZE, board[row][col]-1);
            }
            delay(100);
  
            // white row
            display.fillRect(0,row*SQUARE_SIZE, BOARD_WIDTH,SQUARE_SIZE, ST7735_WHITE);
            delay(100);
            // in-color row
            for(uint8_t col = 0; col < BOARD_COLS; col++){
              drawSquare(col*SQUARE_SIZE, row*SQUARE_SIZE, board[row][col]-1);
            }
            delay(100);
  
            // white row
            display.fillRect(0,row*SQUARE_SIZE, BOARD_WIDTH,SQUARE_SIZE, ST7735_WHITE);
            delay(100);

          // pull everything above one row downwards
          uint8_t pull_empty = 0; // if the last row was empty, stop; set to 0 every non-empty row
          uint8_t pull_row = row;
          while(!pull_empty){
            pull_empty = 1;
            pull_row--;
            for(uint8_t col = 0; col < BOARD_COLS; col++){
              board[pull_row+1][col] = board[pull_row][col];
              if(board[pull_row][col]){
                pull_empty = 0;
              }
            }
          }

          // redraw everything that was just pulled down
          uint8_t redraw_row = row;
          while(redraw_row > pull_row){     // redraw until the empty row (found above)
            for(uint8_t col = 0; col < BOARD_COLS; col++){
              if(board[redraw_row][col])
                drawSquare(col*SQUARE_SIZE, redraw_row*SQUARE_SIZE, board[redraw_row][col]-1);
              else{
                display.fillRect(col*SQUARE_SIZE, redraw_row*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BOARD_BACKGROUND_COLOR);
              }
            }
            redraw_row--;
          }
          
          break;
        }
      }
    } while(found_filled);

    // check if the board is filled and show game over
    // (because of other code and it's execution order the first row is always empty)
    uint8_t filled = 0;
    for(uint8_t col=0; col<BOARD_COLS; col++){
      if(board[1][col])
        filled = 1;
    }

    // game over
    if(filled){
      display.setCursor(BOARD_WIDTH+10, 100);
      display.print("GAME");
      display.setCursor(BOARD_WIDTH+10, 101+FONT_SIZE);
      display.print("OVER");
      while(true){}
    }
    
    // generate a new tetromino
    tetromino.pos_x = BOARD_COLS/2 - 2;
    tetromino.pos_y = 0;
    tetromino.rotation = 0;
    tetromino.shape_id = next_id;
    next_id = random(0, 7);

    // remove the previous next tetromino drawing
    display.fillRect(BOARD_WIDTH+5, 5+FONT_SIZE+5, 4*SQUARE_SIZE, 2*SQUARE_SIZE, BOARD_BACKGROUND_COLOR);
    // draw the next tetromino that will drop
    for(uint8_t y=0; y<2; y++){
      for(uint8_t x=0; x<4; x++){
        if(TETROMINOES_SHAPES[next_id][y][x])
          drawSquare(BOARD_WIDTH+5+x*SQUARE_SIZE, 5+FONT_SIZE+5+y*SQUARE_SIZE, next_id);
      }
    }
  }
}

// read joystick x axis and return value between -100 and 100
int readJoyX(){
  int min_i, max_i;
  if(!JoyXFlip){
    min_i = 0;
    max_i = 1023;
  } else {
    min_i = 1023;
    max_i = 0;
  }
  return map(analogRead(JoyXPin), min_i,max_i, -100,100);
}

// read joystick y axis and return value between -100 and 100
int readJoyY(){
  int min_i, max_i;
  if(!JoyYFlip){
    min_i = 0;
    max_i = 1023;
  } else {
    min_i = 1023;
    max_i = 0;
  }
  return map(analogRead(JoyYPin), min_i,max_i, -100,100);
}

void drawTetromino(){
  switch(tetromino.rotation){
    default:
    case 0:
      for(int8_t y = 0; y<2; y++){
        for(int8_t x = 0; x<4; x++){
          if(TETROMINOES_SHAPES[tetromino.shape_id][y][x])
            drawSquare((x+tetromino.pos_x)*SQUARE_SIZE,(y+tetromino.pos_y)*SQUARE_SIZE, tetromino.shape_id);
        }
      }
      break;
    case 1:
      {
        // negative offset because the straight piece has only FIRST row and empty second
        // so it would be placed one square to the right
        uint8_t neg_offset = 0;
        if(tetromino.shape_id == 0)
          neg_offset = 1;
        
        for(int8_t y = 0; y<2; y++){
          for(int8_t x = 0; x<4; x++){
            if(TETROMINOES_SHAPES[tetromino.shape_id][y][x])
              drawSquare((1-y+tetromino.pos_x-neg_offset)*SQUARE_SIZE,(x+tetromino.pos_y)*SQUARE_SIZE, tetromino.shape_id);
          }
        }
      }
      break;
    case 2:
      // the code block is needed because the declared variables are interfering with case 3
      {
        // negative offset because shape array has width 4,
        // but not every shape has 4 squares in width
        uint8_t neg_offsetX = 0;
        if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
          neg_offsetX = 1;
        else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
          neg_offsetX = 2;

        // negative offset because straight piece has height 1
        uint8_t neg_offsetY = 0;
        if(tetromino.shape_id == 0)
          neg_offsetY = 1;
        
        for(int8_t y = 0; y<2; y++){
          for(int8_t x = 3; x>=0; x--){
            if(TETROMINOES_SHAPES[tetromino.shape_id][y][x])
              drawSquare((3-x+tetromino.pos_x-neg_offsetX)*SQUARE_SIZE,(1-y+tetromino.pos_y-neg_offsetY)*SQUARE_SIZE, tetromino.shape_id);
          }
        }
      }
      break;
    case 3:
      // the code block is needed because the declared variables are interfering with case 2
      {
        // negative offset because shape array has width 4,
        // but not every shape has 4 squares in width
        uint8_t neg_offsetY = 0;
        if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
          neg_offsetY = 1;
        else if(tetromino.shape_id == 1) // check for block (order of TETROMINOES_SHAPES matters!)
          neg_offsetY = 2;
        
        for(int8_t y = 0; y<2; y++){
          for(int8_t x = 0; x<4; x++){
            if(TETROMINOES_SHAPES[tetromino.shape_id][y][x])
              drawSquare((y+tetromino.pos_x)*SQUARE_SIZE,(3-x+tetromino.pos_y-neg_offsetY)*SQUARE_SIZE, tetromino.shape_id);
          }
        }
      }
      break;
  }
}

void eraseTetromino(){
  switch(tetromino.rotation){
    default:
    case 0:
      for(int8_t y = 0; y<2; y++){
        for(int8_t x = 0; x<4; x++){
          if(TETROMINOES_SHAPES[tetromino.shape_id][y][x]){
            display.fillRect((x+tetromino.pos_x)*SQUARE_SIZE,(y+tetromino.pos_y)*SQUARE_SIZE, SQUARE_SIZE,SQUARE_SIZE, BOARD_BACKGROUND_COLOR);
          }
        }
      }
      break;
    case 1:
      {
        // negative offset because the straight piece has only FIRST row and empty second
        // so it would be placed one square to the right
        uint8_t neg_offset = 0;
        if(tetromino.shape_id == 0)
          neg_offset = 1;
        
        for(int8_t y = 0; y<2; y++){
          for(int8_t x = 0; x<4; x++){
            if(TETROMINOES_SHAPES[tetromino.shape_id][y][x]){
              display.fillRect((1-y+tetromino.pos_x-neg_offset)*SQUARE_SIZE,(x+tetromino.pos_y)*SQUARE_SIZE, SQUARE_SIZE,SQUARE_SIZE, BOARD_BACKGROUND_COLOR);
            }
          }
        }
      }
      break;
    case 2:
      // the code block is needed because the declared variables are interfering with case 3
      {
        // negative offset because shape array has width 4,
        // but not every shape has 4 squares in width
        uint8_t neg_offsetX = 0;
        if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
          neg_offsetX = 1;
        else if(tetromino.shape_id > 0) // check for block (order of TETROMINOES_SHAPES matters!)
          neg_offsetX = 2;

        // negative offset because straight piece has height 1
        uint8_t neg_offsetY = 0;
        if(tetromino.shape_id == 0)
          neg_offsetY = 1;
        
        for(int8_t y = 0; y<2; y++){
          for(int8_t x = 3; x>=0; x--){
            if(TETROMINOES_SHAPES[tetromino.shape_id][y][x]){
              display.fillRect((3-x+tetromino.pos_x-neg_offsetX)*SQUARE_SIZE,(1-y+tetromino.pos_y-neg_offsetY)*SQUARE_SIZE, SQUARE_SIZE,SQUARE_SIZE, BOARD_BACKGROUND_COLOR);
            }
          }
        }
      }
      break;
    case 3:
      // the code block is needed because the declared variables are interfering with case 2
      {
        // negative offset because shape array has width 4,
        // but not every shape has 4 squares in width
        uint8_t neg_offsetX = 0;
        if(tetromino.shape_id > 1) // check for anything other then block and straight (order of TETROMINOES_SHAPES matters!)
          neg_offsetX = 1;
        else if(tetromino.shape_id > 0) // check for block (order of TETROMINOES_SHAPES matters!)
          neg_offsetX = 2;
        
        for(int8_t y = 0; y<2; y++){
          for(int8_t x = 0; x<4; x++){
            if(TETROMINOES_SHAPES[tetromino.shape_id][y][x]){
              display.fillRect((y+tetromino.pos_x)*SQUARE_SIZE,(3-x+tetromino.pos_y-neg_offsetX)*SQUARE_SIZE, SQUARE_SIZE,SQUARE_SIZE, BOARD_BACKGROUND_COLOR);
            }
          }
        }
      }
      break;
  }
}

// draws one square with the specified color index inside TETROMINOES_BASE_COLORS
// for small SQUARE_SIZE
/*void drawSquare(uint8_t x, uint8_t y, uint8_t color_id){
  display.fillRect(x+1, y+1, SQUARE_SIZE-2, SQUARE_SIZE-2, TETROMINOES_BASE_COLORS[color_id]); // center
  display.drawFastHLine(x, y, SQUARE_SIZE, ST7735_WHITE); // top
  display.drawFastHLine(x, y+SQUARE_SIZE-1, SQUARE_SIZE, TETROMINOES_SHADE_COLORS[color_id]); // bottom
  display.drawFastVLine(x, y+1, SQUARE_SIZE-2, TETROMINOES_SHADE_COLORS[color_id]); // left
  display.drawFastVLine(x+SQUARE_SIZE-1, y+1, SQUARE_SIZE-2, ST7735_WHITE); // right
}*/
// for bigger SQUARE_SIZE
void drawSquare(uint8_t x, uint8_t y, uint8_t color_id){
  display.fillRect(x+2, y+2, SQUARE_SIZE-4, SQUARE_SIZE-4, TETROMINOES_BASE_COLORS[color_id]); // center
  display.drawFastHLine(x, y, SQUARE_SIZE, ST7735_WHITE); // top 1
  display.drawFastHLine(x+1, y+1, SQUARE_SIZE-1, ST7735_WHITE); // top 2
  display.drawFastHLine(x, y+SQUARE_SIZE-1, SQUARE_SIZE, TETROMINOES_SHADE_COLORS[color_id]); // bottom 1
  display.drawFastHLine(x, y+SQUARE_SIZE-2, SQUARE_SIZE-1, TETROMINOES_SHADE_COLORS[color_id]); // bottom 2
  display.drawFastVLine(x, y+1, SQUARE_SIZE-2, TETROMINOES_SHADE_COLORS[color_id]); // left 1
  display.drawFastVLine(x+1, y+2, SQUARE_SIZE-4, TETROMINOES_SHADE_COLORS[color_id]); // left 2
  display.drawFastVLine(x+SQUARE_SIZE-1, y+1, SQUARE_SIZE-2, ST7735_WHITE); // right 1
  display.drawFastVLine(x+SQUARE_SIZE-2, y+2, SQUARE_SIZE-4, ST7735_WHITE); // right 2
}
