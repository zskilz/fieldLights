#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
/* @file keypadColor.pde
|| @version 0.1.0
|| @author Petrus J. Pretorius
|| @contact zskil@gmail.com
||
|| @description
|| | Field Light Adafruid LED controller project for a Ultamite Disc field. 
|| | used keypas sourcy by Alexander Brevig and some snippets from Adafruit examples.
|| #
*/
//#include <Keypad.h>


//#define PIN 10
//const byte ROWS = 4; //four rows
//const byte COLS = 3; //three columns
//char keys[ROWS][COLS] = {
//  {'1','2','3'},
//  {'4','5','6'},
//  {'7','8','9'},
//  {'*','0','#'}
//};
//byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
//byte colPins[COLS] = {8, 7, 6}; //connect to the column pinouts of the keypad
//
//Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(54, PIN, NEO_RGB + NEO_KHZ400);


//Declare Spectrum Shield pin connections
#define STROBE 4
#define RESET 5
#define DC_One A0
#define DC_Two A1 


void setupSpectrumShield(){
  
  //Set spectrum Shield pin configurations
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);  
  digitalWrite(RESET, LOW);
  digitalWrite(STROBE, HIGH);
}

//===============================================================
// Tweak here... 
//===============================================================
const byte L = 29;//74;//74;
const byte H = 1;//57;//57;
const byte G = 1;//24;
const int G1_offset = 0;
const int G2_offset = 0;
//===============================================================

// forward declaration..
uint32_t _Color(byte r, byte g, byte b);

struct COL{
  byte r;
  byte g;
  byte b;
  COL(){
    r = 0;
    g = 0;
    b = 0;
  };
  COL(uint32_t c){    
    r = (uint8_t)(c >> 16);
    g = (uint8_t)(c >>  8);
    b = (uint8_t)c;
  };
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

// the 4 output pins for the field
const int OUTPUT_PINS[] = {10,11,12,13};

const float threshHold = 0.1;
const int G_brightness = 10;
const boolean useSoundCapture = true;


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
    p1 = new Adafruit_NeoPixel(L*2+H, OUTPUT_PINS[0], NEO_GRB + NEO_KHZ800);
    p2 = new Adafruit_NeoPixel(H, OUTPUT_PINS[1], NEO_GRB + NEO_KHZ800);
    p3 = new Adafruit_NeoPixel(H, OUTPUT_PINS[2], NEO_GRB + NEO_KHZ800);
    p4 = new Adafruit_NeoPixel(L*2+H, OUTPUT_PINS[3], NEO_GRB + NEO_KHZ800);

    p1->setBrightness(G_brightness);
    p2->setBrightness(G_brightness);
    p3->setBrightness(G_brightness);
    p4->setBrightness(G_brightness);
    
    pos = true;
    dir = false;
  };
  
  void show(){
    p1->show();
    p2->show();
    p3->show();
    p4->show();
  }
  
  void clear(){
    p1->begin();
    p2->begin();
    p3->begin();
    p4->begin();
  }
  
  void init(){
    setupSpectrumShield();
    this->clear();
    this->show();
  };

  int freq_amp;
  
  int FL[7];
  int FR[7];
  //the following variables should be used as modulators in the functions of type ColorFn.
  float low =1.0, mid =1.0, high =1.0;

/*************Pull frquencies from Spectrum Shield****************/
  void Read_Frequencies(){
    digitalWrite(RESET, HIGH); 
    digitalWrite(RESET, LOW);
    //Read frequencies for each band
    for (freq_amp = 0; freq_amp<7; freq_amp++)
    {
      
      digitalWrite(STROBE, LOW);
      delayMicroseconds(30);
      FL[freq_amp] = analogRead(DC_One);
      FR[freq_amp] = analogRead(DC_Two); 
      digitalWrite(STROBE, HIGH);
    }
  
  // combine L+R and overlap.
    low = ((FL[0]+FL[1]+FL[2] /*+ FR[0]+FR[1]+FR[2] )/6*/ )/3) / 1024.0;
    mid = ((FL[2]+FL[3]+FL[4] /*+ FR[2]+FR[3]+FR[4] )/6*/)/3) / 1024.0;
    high = ((FL[4]+FL[5]+FL[6] /*+ FR[4]+FR[5]+FR[6] )/6*/)/3) / 1024.0;
  
    low = low < threshHold ? 0.0 : low*low;
    mid = mid < threshHold ? 0.0 : mid*mid*mid;
    high = high < threshHold ? 0.0 : high*high*high;
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
  COL tc;

  
  
  for (int i = 0; i < outerL; i++) {
    r = map(i , 0 , outerL,0,255);
    tc = COL(Wheel((r+t)%255));
 
    tc = COL(
      byte(tc.r * s.low),
      byte(tc.g * s.mid),
      byte(tc.b * s.high));
    
    mapToOuter(i, s, tc.get());
  }

  uint32_t _tc;
  for(int i = 0 ; i < H; i++){   
    r = map(i , 0 , H,0,255);
    tc = COL(Wheel((r+t)%255));
    _tc = COL(
      byte(tc.r * s.low),
      byte(tc.g * s.mid),
      byte(tc.b * s.high)).get();
    s.p2->setPixelColor(i,_tc);
    s.p3->setPixelColor(i,_tc);
  }
}


  
uint32_t red = COL(255,0,0).get();
uint32_t white = COL(255,255,255).get(); 

void idleTheatreChase(struct STATE &s){
  int r, t = ((s.t%1000)/1000.0)*255;
  uint32_t ct = COL(255*s.low,255*s.mid,255*s.high).get();
  for (int i=0; i < outerL; i++) {
    r = map(i , 0 , outerL,0,255);
    if((r*32+t*4)%255>(255/3)){
      
    mapToOuter(i,s, 0);    //turn every third pixel on
    }else{
      
    mapToOuter(i,s, ct);    //turn every third pixel on
    }
  }
}

void idleTheatreChaseRainbow(struct STATE &s){
  int r, t = ((s.t%1000)/1000.0)*255;
  //uint32_t ct = COL(255*s.low,255*s.mid,255*s.high).get();
  COL tc;
  for (int i=0; i < outerL; i++) {
    r = map(i , 0 , outerL,0,255);
    
    if((r*32+t*4)%255>(255/3)){  
      mapToOuter(i,s, 0);   
    }else{
      tc = COL(Wheel((r+t)%255));
      tc = COL(tc.r*s.low,tc.g*s.mid,tc.b*s.high);
      mapToOuter(i,s, tc.get());   
    }
  }
}




void allWhite(struct STATE &s){

  
  uint32_t ct = COL(255*s.low,255*s.mid,255*s.high).get();

  for (int i=0; i < outerL; i++) {
   
      mapToOuter(i,s, ct);    //turn every third pixel on
    
  }
  
//  static uint16_t redWipeInd = 0;
//  int r;
//  int _t = (((s.t%1000)/1000.0)*255) ; 
//  redWipeInd = (redWipeInd + 1) % outerL;
//  r = map(redWipeInd , 0 , outerL,0,255);
// 
//  uint32_t tc = Wheel((r+_t)%255);
//  mapToOuter(redWipeInd,s, tc);
  
}

//Create scrolling red and white candy cane stripes.
//Try adjusting the width in pixels for various results.
//Value "sets" should evenly divide into strand length
//void CandyCane  (int sets,int width,int wait) {
  
void idleCandyCane(struct STATE &s){
  int L;
  int width = 8;
  float ave = (s.mid+s.low+s.high)/3.0;
  uint32_t _red = COL(255*s.low,170*((s.mid+s.high)/2.0),0).get();
  uint32_t _white = COL(255*ave,255*ave,255*ave).get();
  int _t = int((s.t)/1000.0*50) % outerL; 
  for(int i=0;i< outerL;i++) {
    L=outerL-i-1;
    if ( ((i+_t) % (width*2) )<width)
      mapToOuter(L,s,_red);
    else
      mapToOuter(L,s,_white);
  };
  for(int i = 0 ; i < H; i++){
    L=H-i-1;
    if ( ((i+_t) % (width*2) )<width){
      
      s.p2->setPixelColor(i,_red);
      s.p3->setPixelColor(i,_red);
    }else{
      
      s.p2->setPixelColor(i,_white);
      s.p3->setPixelColor(i,_white);
    }
  }
};
 
//Create sets of random white or gray pixels
void idleRandomWhite (struct STATE &s) {
  byte V;
  for(int i=0;i< outerL;i++) {
      V=random(255);
      mapToOuter(i,s,COL(V*s.low,V*s.mid,V*s.high).get());
  };
  for(int i = 0 ; i < H; i++){
    
    V=random(255);
    uint32_t s1 = COL(V*s.low,V*s.mid,V*s.high).get();
    s.p2->setPixelColor(i,s1);
    s.p3->setPixelColor(i,s1);
  }
};
//Create sets of random colors
void idleRandomColor (struct STATE &s) {
  for(int i=0;i< outerL;i++) {
    mapToOuter(i,s,COL(random(255)*s.low,random(255)*s.mid,random(255)*s.high).get());
  }
  uint32_t s1;
  for(int i = 0 ; i < H; i++){
    
     byte wave = playWave(0,s,1);
    s1 = COL(random(255)*s.low,random(255)*s.mid,random(255)*s.high).get();
    s.p2->setPixelColor(i,s1);
    s.p3->setPixelColor(i,s1);
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

ColorFn idleColorFn[] = {&idleRainbow,&idleTheatreChase,&idleTheatreChaseRainbow,&allWhite,&idleCandyCane,&idleRandomWhite,&idleRandomColor};
int numIdleStates = sizeof(idleColorFn)/sizeof(ColorFn);

void idle(struct STATE &s){
  (*idleColorFn[s.idleStateInd])(s);
}


ColorFn colorFn =  &idle;

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
       mapToSides(i, s, mod(COL(blend(r,t1,t2)),playWave(r,s,16)).get());
    }
    byte wave = playWave(0,s,16); 
    uint32_t s1 = mod(t1,wave).get();
    uint32_t s2 = mod(t2,wave).get();
    for(int i = 0 ; i < H; i++){
      s.p4->setPixelColor(i,s1);
      s.p3->setPixelColor(i,s2);
      s.p1->setPixelColor(L+i,s1);
      s.p2->setPixelColor(L+i,s2);
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
  int r, t = ((s.t%1000)/1000.0)*255;;
  for (int i = 0; i < (G+H)*2 ; i++) {
     r = map(i, 0, L, 0, 255);
     uint32_t tc = Wheel((r+t)%255);
     //uint32_t tc = mod(COL(blend(r,teamCol1,teamCol2)),playWave(r,s,16)).get();
     mapToGoal(i,s,tc);
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

unsigned long _St = millis();

void clearAll(){
    for (int i = 0; i < (L * 2 + H) ; i++) {
      state.p1->setPixelColor(i,0);
      state.p4->setPixelColor(i,0);
     
    }
    for(int i = 0 ; i < H; i++){
      state.p2->setPixelColor(i,0);
      state.p3->setPixelColor(i,0);
    }

}

const unsigned long stateChangeInterval =  1000*20;//600000;//(1000*60*10);
    
void loop() {
  //char key = keypad.getKey();
    
  state.t = millis();
  if(useSoundCapture)
    state.Read_Frequencies();
  
  if(state.t - _St >= stateChangeInterval){
    _St = state.t;
    
    state.clear();
    state.idleStateInd = (state.idleStateInd +1)%numIdleStates;
  }
  
//  if (key){
//   switch(key){
//     case '0' :    
//       colorFn = &dataLines;
//       break;
//     case '1' : 
//       state.dir = !state.dir;
//       break;
//     case '2' : 
//       state.pos = !state.pos;
//       break;
//     case '3' : 
//        state.clear();
//       colorFn = &turnover;
//       break;
//     case '4' : 
//       colorFn = &goal;
//       break;
//     case '5' :
//        state.clear();
//        clearAll();
//       colorFn = &test; 
//       break;
//     case '6' : 
//       break;
//     case '7' : 
//     
//        state.clear();
//       colorFn = &play;
//       break;
//     case '8' : 
//        state.clear();
//       colorFn = &idle;
//       break;
//     case '9' :
//       colorFn = &idleCandyCane; 
//       break;
//     case '*' : 
//        state.clear();
//       state.idleStateInd = (state.idleStateInd +1)%numIdleStates;
//       break;
//     case '#' : 
//        state.clear();
//       state.idleStateInd--;
//       state.idleStateInd = state.idleStateInd<0?numIdleStates-1:state.idleStateInd;
//       
//       break;
//       
//   }
//  }
//  
  (*colorFn)(state);
  state.show();
}


