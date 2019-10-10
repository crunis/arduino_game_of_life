#include <LedControl.h>   //LedControl library: https://www.electronoobs.com/ledcontrol.php
#include <math.h>
#include <stdlib.h>
#include <assert.h>

// old avg cps is 22
// screen2 is 31
// screen is 28

const int numLedDevices = 4;      // number of MAX7219s used 
LedControl lc=LedControl(5,4,3,numLedDevices);//DATA | CLK | CS/LOAD | number of matrices
unsigned long tmark;
const int CYCLES = 150;
const int SX = 30;
const int SY = 6;
const int BLANK_Y = 0;

char screen[SY][SX+2];
char nscreen[SY+2][SX+2];

long count=0;


void setup() {
  basic_setup();
//  clear_screen_memset();
//  random_screen();
  set_glider(0,0);
//  print_screen_row(&screen[0][0], SY, SX, SX+2, BLANK_Y);
  print_screen_row(&nscreen[0][0], SY+2, SX+2, SX+2, BLANK_Y);
//  Serial.println(count_neig2(0,0));
}


void loop() {
  life();
}

void setXY(int row, int col, byte val){
  int disp = 3 - col / 8;
  int real_col = col % 8;
  lc.setLed(disp, row, real_col, val);
}

void basic_setup(){
  Serial.begin (9600); 
  Serial.println("Starting....");
  for (int x=0; x<numLedDevices; x++)
  {
    lc.shutdown(x,false);       //The MAX72XX is in power-saving mode on startup
    lc.setIntensity(x,3);       // Set the brightness to default value
    lc.clearDisplay(x);
  }
  tmark = millis();
}


//////////////////////////////////////////////////////////////
// User functions
//////////////////////////////////////////////////////////////


void life(){
  next_screen_fast();
  // print_screen_row(&screen[0][0], SY, SX, SX+2, BLANK_Y);
  print_screen_row(&nscreen[0][0], SY+2, SX+2, SX+2, BLANK_Y);
  count+=1;
  if (!(count % CYCLES)){
    Serial.println(millis()-tmark);
    Serial.print("Cycles per second: ");
    Serial.println( CYCLES * 1000.0 / (millis()-tmark));
    delay(3000);
    random_screen();
//    print_screen_row(&screen[0][0], SY, SX, SX+2, BLANK_Y);
    print_screen_row(&nscreen[0][0], SY+2, SX+2, SX+2, BLANK_Y);
    delay(3000);
    tmark = millis();
  }
  delay(100);
}


int get_screen(int row, int col){
  row = (row + SY) % SY;
  col = (col + SX) % SX;
  return screen[row][col];
}

void clear_screen_xy(){
  for (int i=0; i<SY; i++){
    for (int j=0; j<SX; j++){
      screen[i][j] = 0;
    }
  }
}

void clear_screen_memset(){
  memset(&screen[0][0], 0, sizeof(screen));
}

void print_screen_xy(){
  for (int i=0; i<SY; i++){
    for (int j=0; j<SX; j++){
      setXY(i,j, screen[i][j]);
    }
  }
}

void print_screen_row(char* screen, int sy, int sx, int sx_mem, int blank_y){
  for (int disp=0; disp<numLedDevices; disp++){
    for (int row=0; row<sy; row++){
      int res = 0;
      for (int col=0; col<8 && (col+disp*8)<sx; col++){
          if ( *(screen + (row * sx_mem + disp*8 +col))){
            res += (1 << (7-col));
          }
      }
      lc.setRow(3-disp, blank_y + row, res);
    }
  }
}

void set_pi(int uf, int of){
  screen[uf][0+of] = 1;
  screen[uf][1+of] = 1;
  screen[uf][2+of] = 1;
  screen[1+uf][0+of] = 1;
  screen[1+uf][2+of] = 1;
  screen[2+uf][0+of] = 1;
  screen[2+uf][2+of] = 1;
}

void set_glider(int uf, int of){
  screen[uf]  [1+of] = 1;
  screen[1+uf][2+of] = 1;
  screen[2+uf][of]   = 1;
  screen[2+uf][1+of] = 1;
  screen[2+uf][2+of] = 1;
}

void set_sema(int row_off, int col_off){
  screen[row_off]  [col_off+1] = 1;
  screen[row_off+1][col_off+1] = 1;
  screen[row_off+2][col_off+1] = 1;
}


void random_screen(){
  for (int i=0; i<SY; i++){
    for (int j=0; j<SX; j++){
      screen[i][j] = (rand() < RAND_MAX/2);
    }  
  }
}

int count_neig(int row,int col){
  int neig = 0;
  for (int i=row-1; i<=row+1; i++){
    for (int j=col-1; j<=col+1; j++){
      if (i!=row || j!=col) {
        neig += get_screen(i,j);
      }
    }
  }
  return neig;
}

void dump_to_nscreen(){
  memcpy(&nscreen[1][1], &screen[0][0], sizeof(screen));
  memcpy(&nscreen[0][1], &screen[SY-1][0], SX);
  memcpy(&nscreen[SY+1][1], &screen[0][0], SX);
  for(int i=0; i<SY; i++){
    nscreen[i+1][0] = screen[i][SX-1];
    nscreen[i+1][SX+1] = screen[i][0];
  }
  nscreen[0][0] = screen[SY-1][SX-1];
  nscreen[0][SX+1] = screen[SY-1][0];
  nscreen[SY+1][0] = screen[0][SX-1];
  nscreen[SY+1][SX+1] = screen[0][0];
}

int count_neig_fast(int row,int col){
  int neig = 0;
  char *p = &nscreen[row+1][col+1];
  
  p -= SX+2+1;
  
  for (int i=0; i<3; i++){
    for (int j=0; j<3; j++){
      if (!(i==1 && j==1) && *p){
        neig += 1;
      }
      p++;
    }
    p += (SX + 2) - 3;
  }

  return neig;
}

int next_state(int row, int col, int (*count_neig_f(int, int))){
  int neig = count_neig_f(row, col);
  if (screen[row][col]){
    if (neig<2 || neig>3){
      return 0;
    }
    return 1;
  } else {
    if (neig == 3){
      return 1;
    }
    return 0;
  }
}

int next_screen(){
  char nscreen[8][32];
  for (int i=0; i<8; i++){
    for (int j=0; j<32; j++){
      nscreen[i][j] = next_state(i,j, count_neig);
    }
  }
  for (int i=0; i<8; i++){
    for (int j=0; j<32; j++){
      screen[i][j] = nscreen[i][j];
    }
  }
}


int next_screen_fast(){
  dump_to_nscreen();
  
  for (int i=0; i<SY; i++){
    for (int j=0; j<SX; j++){
      screen[i][j] = next_state(i,j, count_neig_fast);
    }
  }

}
