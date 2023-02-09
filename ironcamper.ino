// Tim Eisenmann - Nextion Display für Wohnkabine 
// v0.3
/* Arduino Pro Micro
 * Sensor Außen D4
 * Sensor Innen D5
 * Display Serial RX9 TX8
 * Sensor Spannung A0
 * Sensor Stromstärke A1
 */

#include <Wire.h>
#include <SoftwareSerial.h>
#include <DHT.h>;
#include <DS3231.h>

//DHT21 außen
#define DHTPIN 4           
#define DHTTYPE DHT21       
DHT dhta(DHTPIN, DHTTYPE);   //Initialize DHT sensor for normal 16mhz Arduino

//DHT22 innen
#define DHTPIN 5            
#define DHTTYPE DHT22       
DHT dhti(DHTPIN, DHTTYPE);   //Initialize DHT sensor for normal 16mhz Arduino

DS3231  rtc(SDA, SCL);

#define SENSOR_Spannung A0  //Spannungssensor
#define SENSOR_Strom A1     //Stromsensor

#define OPERATING_VOLTAGE 4.1f   //Referenzspannung
const float R1 = 30000.0f;  //R1
const float R2 = 7500.0f;  //R2
const float MAX_VIN = OPERATING_VOLTAGE;


int lufta,lufti;  //Luftfeuchtigkeit
float tempa,tempi; //Temperatur
float messwertv; //Spannungssensor Messwert
float messwerts;  //Stromsensor Messwert
float v;  //Spannung
int vval; //Spannung Leiste
double asensor; //Stromstärke
double acalc;
double a;
float w; //Watt



const byte rxPin = 9;
const byte txPin = 8;

// Set up a new SoftwareSerial object
SoftwareSerial mySerial (rxPin, txPin);


void setup() {

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  mySerial.begin(9600);
  dhta.begin();
  dhti.begin();

  pinMode(SENSOR_Spannung, INPUT);
  pinMode(SENSOR_Strom, INPUT);

  Serial.begin(9600);
  
}


void loop() {
  
  String cmd;
  cmd += "\"";
  
//Sensor außen---------------------------------------------------------------
  lufta = dhta.readHumidity();
  tempa= dhta.readTemperature();
  String stringtempa;
  stringtempa = String(tempa,1);  

//Sensor innen---------------------------------------------------------------
  lufti = dhti.readHumidity();
  tempi= dhti.readTemperature();
  String stringtempi;
  stringtempi = String(tempi,1); 
  
//Temp außen---------------------------------------------------------------
  mySerial.print("tempa.txt=" + cmd + stringtempa +  cmd);
  nextionsend();

//Luftfeuchtigkeit außen---------------------------------------------------------------
  mySerial.print("lufta.txt=" + cmd + lufta + "%" + cmd);
  nextionsend();


//Temp innen---------------------------------------------------------------
  mySerial.print("tempi.txt=" + cmd + stringtempi +  cmd);
  nextionsend();


//Luftfeuchtigkeit innen---------------------------------------------------------------
  mySerial.print("lufti.txt=" + cmd + lufti + "%" + cmd);
  nextionsend();

//Spannung Batterie---------------------------------------------------------------
  float vin = analogRead(SENSOR_Spannung) * (MAX_VIN / 1024.0f);
  float valc = (vin * (R1 + R2)) / R2; 
     
  mySerial.print("spannung.txt=" + cmd + (valc) + "V" + cmd);
  nextionsend();

  vval =map(valc,10,14.4,0,100);
  if (vval <=0){
    vval=0;
  }
  mySerial.print("bat.val=");
  mySerial.print(vval);  //muss int sein
  nextionsend();

  
  if (valc <=13){
  mySerial.print("p2.pic=");
  mySerial.print(8);
  nextionsend();} else {
  mySerial.print("p2.pic=");
  mySerial.print(7);
  nextionsend();
  }


//Stromstärke Batterie---------------------------------------------------------------

  asensor = analogRead(SENSOR_Strom);
  acalc= (asensor/100)/2;
  a=(acalc-2.5)/0.065;
  if (a<=0) a=0;
  //Serial.println(acalc);
  mySerial.print("ah.txt=" + cmd + a + "A" + cmd);
  nextionsend();
  
//Watt---------------------------------------------------------------
  w = valc + a;
  if (a<=0) w=0;
  mySerial.print("watt.txt=" + cmd + w + "Watt" + cmd);
  nextionsend();



}

void nextionsend(){
  mySerial.write(0xFF);
  mySerial.write(0xFF);
  mySerial.write(0xFF);
  }

void spannung(){

}

void strom(){
  messwerts=analogRead(SENSOR_Strom);
  a= (messwerts - 2.5) / 0.066;
  
}
