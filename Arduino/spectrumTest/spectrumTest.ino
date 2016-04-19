//Declare Spectrum Shield pin connections
#define STROBE 4
#define RESET 5
#define DC_One A0
#define DC_Two A1 


//Define spectrum variables
int freq_amp;
int Frequencies_One[7];
int Frequencies_Two[7]; 
int i;

void setupSpectrumShield(){
 /********************Setup Loop*************************/
  

  //Set spectrum Shield pin configurations
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);  
  digitalWrite(STROBE, HIGH);
  digitalWrite(RESET, HIGH);

  //Initialize Spectrum Analyzers
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, HIGH);
  delay(1);
  digitalWrite(STROBE, HIGH);
  delay(1);
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, LOW);
}



void setup() {
 
  setupSpectrumShield();
  
  Serial.begin(9600);
  Serial.println("App Started");
}

void loop() {
  Read_Frequencies();
  Graph_Frequencies();
  delay(200);
}


/*************Pull frquencies from Spectrum Shield****************/
void Read_Frequencies(){
  //Read frequencies for each band
  for (freq_amp = 0; freq_amp<7; freq_amp++)
  {
    Frequencies_One[freq_amp] = analogRead(DC_One);
    Frequencies_Two[freq_amp] = analogRead(DC_Two); 
    digitalWrite(STROBE, HIGH);
    digitalWrite(STROBE, LOW);
  }
}

void Graph_Frequencies(){
   for( i= 0; i<7; i++)
   {
    //Serial.print("C:");
    
    Serial.print(i);
    Serial.print(",");
    
    Serial.print(Frequencies_One[i]);
    Serial.print(",");
    Serial.print(Frequencies_Two[i]);
    
    Serial.print(":");
   }
    Serial.println("");
}


