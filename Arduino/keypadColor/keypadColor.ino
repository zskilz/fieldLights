#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
/* @file keypadColor.pde
|| @version 1.0
|| @author Alexander Brevig
|| @contact alexanderbrevig@gmail.com
||
|| @description
|| | Demonstrates the simplest use of the matrix Keypad library.
|| #
*/
#include <Keypad.h>


#define PIN 10
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 7, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(54, PIN, NEO_RGB + NEO_KHZ400);


//===============================================================
// Tweak here... 
//===============================================================
const byte L = 74;//74;//74;
const byte H = 57;//57;//57;
const byte G = 24;//24;
const int G1_offset = 0;
const int G2_offset = 0;
//===============================================================


struct COL{
  byte r;
  byte g;
  byte b;
  COL(byte _r,
  byte _g,
  byte _b){
    r = _r;
    g = _g;
    b = _b;
  };
  COL(const COL &c){
  r = c.r;
  g= c.g;
  b = c.b;
  }
  uint32_t get(){
    return _Color(r,g,b);
  }
};


//===============================================================
// Tweak Team colors here... (r,g,b) [0..255]
//===============================================================
COL teamCol1(255,0,0);
COL teamCol2(255,255,0);  
//===============================================================

struct STATE {
  Adafruit_NeoPixel *p1;
  Adafruit_NeoPixel *p2;
  Adafruit_NeoPixel *p3;
  Adafruit_NeoPixel *p4;
  
  
  unsigned long t;
  
  bool pos;
  bool dir;
  
  int idleStateInd;
  STATE(){
    p1 = new Adafruit_NeoPixel(L*2+H, 10, NEO_RGB + NEO_KHZ400);
    p2 = new Adafruit_NeoPixel(H, 11, NEO_RGB + NEO_KHZ400);
    p3 = new Adafruit_NeoPixel(H, 12, NEO_RGB + NEO_KHZ400);
    p4 = new Adafruit_NeoPixel(L*2+H, 13, NEO_RGB + NEO_KHZ400);
    
    pos = true;
    dir = false;
  };
  void init(){
    p1->begin();
    p1->show();
    p2->begin();
    p2->show();
    p3->begin();
    p3->show();
    p4->begin();
    p4->show();
  };
  
  void show(){
    p1->show();
    p2->show();
    p3->show();
    p4->show();
  }
};


typedef void (*ColorFn)(struct STATE &s);

STATE state;

uint32_t _Color(byte r, byte g, byte b){

  return state.p1->Color(r,g,b);

}

  
byte blend(byte r , byte src1, byte src2){
  return byte((src1*(256-r)+src2*r)/256);
}  
 
struct COL blend(byte r, struct COL &c1, struct COL &c2){
  return COL(blend(r,c1.r,c2.r), blend(r,c1.g,c2.g), blend(r,c1.b,c2.b));
}
  
struct COL mod(const struct COL &c1, byte m){
  
  return COL(byte(c1.r*m/256),byte(c1.g*m/256),byte(c1.b*m/256));
}

byte playRamp(byte r, struct STATE &s, int waves = 2, float speed = 2){
  int term = s.t * speed ;
  term += map((waves*(s.dir?r:-r))%255,0,255,0,1000);
  byte _t = byte(((term%1000)/1000.0)*256) ; 
  return  byte(_t);
}


byte playWave(byte r, struct STATE &s, int waves = 2, float speed = 2){
  byte _t = playRamp( r,s, waves,  speed);
  return  byte((_t>127?(127-_t%128):_t)+16);
}



void mapToOuter(int i, struct STATE &s, uint32_t c){
  if(i<(L*2+H)){
    s.p1->setPixelColor(i,c);
  } else if(i>=(L*2+H)){
    s.p4->setPixelColor((L*4+H*2)-i-1,c);
  } 
  
}
  
void mapToGoal(int i, struct STATE &s, uint32_t c){
  Adafruit_NeoPixel *p1 = s.pos?s.p1:s.p4;
  Adafruit_NeoPixel *p2 = s.pos?s.p2:s.p3;
  if(i<H){
      p2->setPixelColor(H-i-1,c);  
  }else {
      p1->setPixelColor((L-G)+(i-H-1),c); 
  }
}  
  
void mapToSides(int i, struct STATE &s, uint32_t c){
   if(i<L){
     s.p1->setPixelColor(i+L+H,c);
     s.p1->setPixelColor(L-i-1,c);  
   }else if(i>=L){
     s.p4->setPixelColor(i-L,c);
     s.p4->setPixelColor((L*2+H)-(i-L),c);
   }
}

const int halfOuterL = (L * 2 + H) ;
const int outerL = halfOuterL * 2;
const int goalL = (G+H)*2;


void idleRainbow(struct STATE &s){
  int r, t = ((s.t%1000)/1000.0)*255;
  for (int i = 0; i < outerL; i++) {
    r = map(i , 0 , outerL,0,255);
    uint32_t tc = Wheel((r+t)%255);
    mapToOuter(i, s, tc);
  }
  for(int i = 0 ; i < H; i++){   
    r = map(i , 0 , H,0,255);
    uint32_t tc = Wheel((r+t)%255);
    s.p2->setPixelColor(i,tc);
    s.p3->setPixelColor(i,tc);
  }
}


  
  uint32_t red = COL(255,0,0).get();
  uint32_t white = COL(255,255,255).get(); 
  

void idleTheatreChase(struct STATE &s){
  int r, t = ((s.t%1000)/1000.0)*255;
  for (int q=0; q < 3; q++) {
    for (int i=0; i < outerL; i=i+3) {
      r = map(i , 0 , outerL,0,255);
      mapToOuter(i+q,s, white);    //turn every third pixel on
    }
    s.show();
   
    delay(30);
   
    for (int i=0; i < outerL; i=i+3) {
      mapToOuter(i+q,s, 0);        //turn every third pixel off
    }
  }
}


void idleTheatreChaseRainbow(struct STATE &s){
  int r, t = ((s.t%1000)/1000.0)*255;
  for (int q=0; q < 3; q++) {
    for (int i=0; i < outerL; i=i+3) {
      r = map(i , 0 , outerL,0,255);
      //byte wave = playWave(r,s,4);
      uint32_t tc = Wheel((r+t)%255);
      mapToOuter(i+q,s, tc);    //turn every third pixel on
    }
    s.show();
   
    delay(30);
   
    for (int i=0; i < outerL; i=i+3) {
      mapToOuter(i+q,s, 0);        //turn every third pixel off
    }
  }
}

void idleWipeFX(struct STATE &s){
  static uint16_t redWipeInd = 0;
  int r;
  int _t = (((s.t%1000)/1000.0)*255) ; 
  redWipeInd = (redWipeInd + 1) % outerL;
  r = map(redWipeInd , 0 , outerL,0,255);
 
  uint32_t tc = Wheel((r+_t)%255);
  mapToOuter(redWipeInd,s, tc);
  
}

//Create scrolling red and white candy cane stripes.
//Try adjusting the width in pixels for various results.
//Value "sets" should evenly divide into strand length
//void CandyCane  (int sets,int width,int wait) {
  
void idleCandyCane(struct STATE &s){
  int L;
  int width = 8;
  int _t = (s.t)/1000.0*50 ; 
  for(int i=0;i< outerL;i++) {
    L=outerL-i-1;
    if ( ((i+_t) % (width*2) )<width)
      mapToOuter(L,s,red);
    else
      mapToOuter(L,s,white);
  };
};
 
//Create sets of random white or gray pixels
void idleRandomWhite (struct STATE &s) {
  byte V;
  for(int i=0;i< outerL;i++) {
      V=random(255);
      mapToOuter(i,s,COL(V,V,V).get());
  }
};
//Create sets of random colors
void idleRandomColor (struct STATE &s) {
  for(int i=0;i< outerL;i++) {
      mapToOuter(i,s,COL(random(255),random(255),random(255)).get());
  }
};

void idleKnightRider(struct STATE &s) {
  int L,k;
  bool dir = true;
  int width = 1;
  int _t = int((s.t)/1000.0*20)%(outerL*2) ; 
  if( _t > outerL){
    dir = false;
    _t=outerL-(_t%outerL);
  }
  
  for(int i=0;i< outerL;i++) {
     L=outerL-i-1;
    if ( ((L+width) > _t)&&((L-width) < _t ))
      mapToOuter(L,s,red);
    else
      mapToOuter(L,s,0);
  }
};

ColorFn idleColorFn[] = {&idleRainbow,&idleTheatreChase,&idleTheatreChaseRainbow,&idleWipeFX,&idleCandyCane,&idleRandomWhite,&idleRandomColor,&idleKnightRider};
int numIdleStates = sizeof(idleColorFn)/sizeof(ColorFn);

void idle(struct STATE &s){
  (*idleColorFn[s.idleStateInd])(s);
}


ColorFn colorFn = &idle;

void dataLines(struct STATE &s){
    int r;
      for (int i = 0; i < (L * 2 + H) ; i++) {
        r = map(i , 0 , L * 2 + H,0,255);
        byte wave = playWave(r,s,4);
        s.p1->setPixelColor(i,wave,0,0);
        s.p4->setPixelColor(i,wave,wave,0);
       
      }
      for(int i = 0 ; i < H; i++){
        
        r = map(i , 0 ,H,0,255);
         byte wave = playWave(r,s,4);
        s.p2->setPixelColor(i,0,wave,0);
        s.p3->setPixelColor(i,0,wave,wave);
      }
}

void play(struct STATE &s){
    int r;
    COL t1 = s.pos?teamCol1:teamCol2;
    COL t2 = s.pos?teamCol2:teamCol1;
      for (int i = 0; i < L*2 ; i++) {
         r = map(i ,0, L*2,0,255);
        //  COL c(playWave(r,s,1),
        //  playWave(r, s, 32, 0.5),
        //  playWave(r, s, 32, 0.25));
         mapToSides(i, s, mod(COL(blend(r,t1,t2)),playWave(r,s,16)).get());
      }
      for(int i = 0 ; i < H; i++){
        
         byte wave = playWave(0,s,1);
        COL s1 = mod(t1,wave);
        COL s2 = mod(t2,wave);
        s.p2->setPixelColor(i,s1.r,s1.g,s1.b);
        s.p3->setPixelColor(i,s2.r,s2.g,s2.b);
      }
}


void turnover(struct STATE &s){
    int r;
    static unsigned long startTime = -1;
    if(startTime == -1)startTime=s.t;
    if (s.t > (startTime + 3000)) {
        startTime = -1;
        colorFn = &play;
        s.dir = !s.dir;
      }
    for (int i = 0; i < outerL ; i++) {
       r = map(i ,0, outerL,0,255);
       mapToOuter(i, s, mod(COL(blend(playWave(r,s,16),teamCol1,teamCol2)),playWave(r,s,32)).get());
    }
    for(int i = 0 ; i < H; i++){
      
      uint32_t s1 = mod(COL(blend(playWave(r,s,16),teamCol1,teamCol2)),playWave(r,s,32)).get();
      s.p2->setPixelColor(i,s1);
      s.p3->setPixelColor(i,s1);
    }
}


void goal(struct STATE &s){
    int r;
    static unsigned long startTime = -1;
    if(startTime == -1)startTime=s.t;
    if (s.t > (startTime + 3000)) {
        startTime = -1;
        colorFn = &play;
        s.pos = !s.pos;
      }
      
      
      play(s);
      for (int i = 0; i < goalL; i++) {
       r = map(i ,0, goalL,0,255);
        mapToGoal(i,s,blend(playWave(r, s, 4, 5), teamCol1, teamCol2).get());
      }
}


void test(struct STATE &s){
  int r;
      for (int i = 0; i < (G+H)*2 ; i++) {
         r = map(i, 0, L, 0, 255);
         COL c = mod(COL(blend(r,teamCol1,teamCol2)),playWave(r,s,16));
         mapToGoal(i,s,c.get());
      }
      
}




// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return _Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return _Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return _Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}


// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.


void setup() {
  // put your setup code here, to run once:
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  state.init();


}


    
void loop() {
  char key = keypad.getKey();
    
  state.t = millis();
  
  if (key){
   switch(key){
     case '0' :    
       colorFn = &dataLines;
       break;
     case '1' : 
       state.dir = !state.dir;
       break;
     case '2' : 
       state.pos = !state.pos;
       break;
     case '3' : 
       colorFn = &turnover;
       break;
     case '4' : 
       colorFn = &goal;
       break;
     case '5' :
       break;
     case '6' : 
       break;
     case '7' : 
       colorFn = &play;
       break;
     case '8' : 
       colorFn = &idle;
       break;
     case '9' :
       colorFn = &test; 
       break;
     case '*' : 
       state.idleStateInd = (state.idleStateInd +1)%numIdleStates;
       break;
     case '#' : 
       state.idleStateInd--;
       state.idleStateInd = state.idleStateInd<0?numIdleStates-1:state.idleStateInd;
       
       break;
       
   }
  }
  
  (*colorFn)(state);
  state.show();
}


