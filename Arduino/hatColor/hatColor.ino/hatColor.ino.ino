#include <Adafruit_NeoPixel.h>
//#include <avr/power.h>
#include <assert.h>
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
const byte L = 240; // how many bulbs?
const byte percentageDark = 90;//should be less than 100 but bigger than 80. (abuot 48 LEDs MAX)
const int G_brightness = 10;//
const byte chaseSpeed = 4;
const byte numBands = 8;
const unsigned long stateChangeInterval =  1000*20;//600000;//(1000*60*10);

//===============================================================


uint32_t _Color(byte r, byte g, byte b);

const byte D = 255*percentageDark/100;

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

struct STATE {
  Adafruit_NeoPixel *p1;
  
  
  unsigned long t;

  
  int idleStateInd;
  STATE(){
    p1 = new Adafruit_NeoPixel(L, 10, NEO_RGB + NEO_KHZ400);
    p1->setBrightness(G_brightness);
  };
  
  void show(){
    p1->show();
  }
  
  void clear(){
    p1->begin();
  }
  
  void init(){
    this->clear();
    this->show();
  };
  
  
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




void idleRainbow(struct STATE &s){
  int r, t = ((s.t%1000)/1000.0)*255;
  for (int i = 0; i < L; i++) {
    r = map(i , 0 , L,0,255);
    if((r*numBands/2+t*chaseSpeed*2)%255>D){
      s.p1->setPixelColor(i,0);    //turn every third pixel on
    }else{ 
      uint32_t tc = Wheel((r+t)%255);
      s.p1->setPixelColor(i,tc);    //turn every third pixel on
    }
  }
 
}


  
  uint32_t red = COL(255,0,0).get();
  uint32_t white = COL(255,255,255).get(); 
  

void idleTheatreChase(struct STATE &s){
  int r, t = ((s.t%1000)/1000.0)*255;
  for (int i=0; i < L; i++) {
    r = map(i , 0 , L,0,255);
    if((r*numBands+t*chaseSpeed)%255>D){
      s.p1->setPixelColor(i,0);    //turn every third pixel on
    }else{ 
      s.p1->setPixelColor(i,white);    //turn every third pixel on
    }
  }
}

void idleTheatreChaseRainbow(struct STATE &s){
  int r, t = ((s.t%1000)/1000.0)*255;
  for (int i=0; i < L; i++) {
    r = map(i , 0 , L,0,255);
    
    if((r*numBands+t*chaseSpeed)%255>D){  
      s.p1->setPixelColor(i, 0);   
    }else{
      uint32_t tc = Wheel((r+t)%255);
      s.p1->setPixelColor(i, tc);   
    }
  }
}



//Create scrolling red and white candy cane stripes.
//Try adjusting the width in pixels for various results.
//Value "sets" should evenly divide into strand length
//void CandyCane  (int sets,int width,int wait) {
  
void idleCandyCane(struct STATE &s){
  int r, t = ((s.t%1000)/1000.0)*255;
  
  for(int i=0;i< L;i++) {
    r = map(i , 0 , L,0,255);
    if((r*numBands+t*chaseSpeed)%255>D){  
      s.p1->setPixelColor(L,0);
    }else{
      if((r*16+t*2)%255>D)
        s.p1->setPixelColor(L,white);
       else
        s.p1->setPixelColor(L,red);
    }
  };
};
 
//Create sets of random white or gray pixels
void idleRandomWhite (struct STATE &s) {
  int r, t = ((s.t%1000)/1000.0)*255;
  byte V;
  for(int i=0;i< L;i++) {
      r = map(i , 0 , L,0,255);
      if((r*numBands/2+t*chaseSpeed/2)%255>D){  
        s.p1->setPixelColor(i, 0);   
      }else{
        V=random(255);
        s.p1->setPixelColor(i,COL(V,V,V).get());
      }
  };
};
//Create sets of random colors
void idleRandomColor (struct STATE &s) {
  
  int r, t = ((s.t%1000)/1000.0)*255;
  for(int i=0;i< L;i++) {
    
      r = map(i , 0 , L,0,255);
      if((r*numBands/2+t*chaseSpeed/2)%255>D){  
        s.p1->setPixelColor(i, 0);   
      }else{
        s.p1->setPixelColor(i,COL(random(255),random(255),random(255)).get());
      }
     
  }

};

void idleKnightRider(struct STATE &s) {
  int width = 2;
  float _speed = 20;
  int _t = int((s.t)/1000.0*_speed)%(L*2) ; 
  if( _t > L){
    _t=L-(_t%L);
  }
  
  for(int i=0;i< L;i++) {
    if ( ((i+width) > _t)&&((i-width) < _t ))
      s.p1->setPixelColor(i,red);
    else
      s.p1->setPixelColor(i,0);
  }
};

ColorFn idleColorFn[] = {&idleRainbow,&idleTheatreChase,&idleTheatreChaseRainbow,&idleCandyCane,&idleRandomWhite,&idleRandomColor,&idleKnightRider};
int numIdleStates = sizeof(idleColorFn)/sizeof(ColorFn);

void idle(struct STATE &s){
  (*idleColorFn[s.idleStateInd])(s);
}


ColorFn colorFn = &idle;

byte playRamp(byte r, struct STATE &s, int waves = 2, float speed = 2){
  int term = s.t * speed ;
  term += map((waves*r)%255,0,255,0,1000);
  byte _t = byte(((term%1000)/1000.0)*256) ; 
  return  byte(_t);
}


byte playWave(byte r, struct STATE &s, int waves = 2, float speed = 2){
  byte _t = playRamp( r,s, waves,  speed);
  return  byte((_t>127?(127-_t%128):_t)+16);
}


void dataLines(struct STATE &s){
    int r;
    for (int i = 0; i < (L) ; i++) {
      r = map(i , 0 , L,0,255);
      byte wave = playWave(r,s,4);
      s.p1->setPixelColor(i,wave,0,0);
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


  // NOTE: comment below if you want to experiment with more bulbs
  assert((percentageDark<100)&&(percentageDark>80));

  state.init();


}

unsigned long _St = millis();

void clearAll(){
    for (int i = 0; i < (L) ; i++) {
      state.p1->setPixelColor(i,0);
    }
}

 
    
void loop() {
  state.t = millis();
  
  if(state.t >  (_St + stateChangeInterval)){
    _St = state.t;
    
    state.clear();
    state.idleStateInd = (state.idleStateInd +1)%numIdleStates;
  }
  
  
  (*colorFn)(state);
  state.show();
}


