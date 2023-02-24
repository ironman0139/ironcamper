// Tim Eisenmann - Nextion Display für Wohnkabine 
// v0.8
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
#include <RTClib.h>;

RTC_DS3231 rtc;


const float SAMPLE_INTERVAL = 0.1; // Abtastintervall in Sekunden
const float MAXIMUM_CAPACITY = 92000.0; // Maximale Kapazität des Akkus in mAh
const float MAXIMUM_VOLTAGE = 12.85; // Maximale Spannung des Akkus in Volt
const float MINIMUM_VOLTAGE = 11.9; // Minimale Spannung des Akkus in Volt (entspricht leerem Akku)
const float VOLTAGE_DIFFERENCE = MAXIMUM_VOLTAGE - MINIMUM_VOLTAGE; // Spannungsdifferenz zwischen vollen und leeren Akkus
float accumulated_current = 0.0; // Kumulierter Strom in Ampere-Sekunden
float accumulated_capacity = 0.0; // Kumulierte Kapazität in mAh
float remaining_capacity = 0.0; // Verbleibende Kapazität in mAh
float current1 = 0.0; // Aktueller Strom in Ampere
float voltage1 = 0.0; // Aktuelle Spannung in Volt
float battery_life = 0.0;

float version_ironcamper = 0.81;


//DHT21 außen
#define DHTPINa 4           
#define DHTTYPEa DHT21       
DHT dhta(DHTPINa, DHTTYPEa);   //Initialize DHT sensor for normal 16mhz Arduino
float dhta_offset = 0.0f;

//DHT22 innen
#define DHTPIN 5            
#define DHTTYPE DHT22       
DHT dhti(DHTPIN, DHTTYPE);   //Initialize DHT sensor for normal 16mhz Arduino
float dhti_offset = 0.0f;


#define SPANNUNG_SENSOR A2  //Spannungssensor
float MESSWERT_Spannung; //Spannungssensor Messwert
float v;  //Spannung


#define STROM_SENSOR A1     //Stromsensor
float MESSWERT_Strom;  //Stromsensor Messwert

#define OPERATING_VOLTAGE 5.0f   //Referenzspannung
const float R1 = 30000.0f;  //R1
const float R2 = 7500.0f;  //R2
const float MAX_VIN = OPERATING_VOLTAGE;

const float STROM_korrektur = 0.065;  //Stromsensor Korrekturwert


int lufta,lufti;  //Luftfeuchtigkeit
float tempa,tempi; //Temperatur


float VOLT_average;
int vval; //Spannung Leiste
double asensor; //Stromstärke
double acalc;
double a;
int w; //Watt
float STROM_average; // Mittelwert Stromsensor
int h,m,dd,mm,yy;
String datum;
int test = 0;
float prozent;


const byte rxPin = 9;
const byte txPin = 8;
String daten_display = "";

// Set up a new SoftwareSerial object
SoftwareSerial nextion (rxPin, txPin);


void setup() {

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  nextion.begin(9600);
  dhta.begin();
  dhti.begin();
  rtc.begin();



  pinMode(SPANNUNG_SENSOR, INPUT);
  pinMode(STROM_SENSOR, INPUT);

  Serial.begin(9600);

     // This line sets the RTC with an explicit date & time, for example to set
     // January 21, 2014 at 3am you would call:
     // rtc.adjust(DateTime(2023, 2, 22, 22, 33, 0));
  
}


void loop() {
  
  String cmd;
  cmd += "\"";


  nextion.print("version.txt=" + cmd + " v" + version_ironcamper + cmd);
  nextionsend();


  
//Sensor außen---------------------------------------------------------------
  lufta = dhta.readHumidity();
  tempa= dhta.readTemperature() + dhta_offset;
  String stringtempa;
  stringtempa = String(tempa,1);  

//Sensor innen-------------------------------------------------------------------
  lufti = dhti.readHumidity() + dhti_offset;
  tempi= dhti.readTemperature();
  String stringtempi;
  stringtempi = String(tempi,1); 
  
//Temp außen--------------------------------------------------------------------
  nextion.print("tempa.txt=" + cmd + stringtempa +  cmd);
  nextionsend();

//Luftfeuchtigkeit außen---------------------------------------------------------------
  nextion.print("lufta.txt=" + cmd + lufta + "%" + cmd);
  nextionsend();


//Temp innen---------------------------------------------------------------
  nextion.print("tempi.txt=" + cmd + stringtempi +  cmd);
  nextionsend();


//Luftfeuchtigkeit innen---------------------------------------------------------------
  nextion.print("lufti.txt=" + cmd + lufti + "%" + cmd);
  nextionsend();

//Spannung Batterie---------------------------------------------------------------
  float MESSWERT_Spannung = analogRead(SPANNUNG_SENSOR) * (MAX_VIN / 1024.0f);
  float valc = (MESSWERT_Spannung * (R1 + R2)) / R2;

   float VOLT_average = 0;
     for (int ii=0; ii < 10; ii++) {
     VOLT_average += valc;
     delay(10);
   }
   
  VOLT_average /= 10.0f;
  String voltage = String(VOLT_average, 1);

    
  nextion.print("spannung.txt=" + cmd + (voltage) + "V" + cmd);
  nextionsend();

 // Serial.println(voltage);

  valc =valc*100;
  //Serial.println(valc);
  vval=valc;
  vval =map(valc,1000,1300,0,100);
  //Serial.println(vval);
  if (vval <=0){
    vval=0;
  }

 if (vval >=100){
    vval=100;
  }
  
  //Serial.println(vval);
 // Serial.println(valc);
  nextion.print("bat.val=");
  nextion.print(vval);  //muss int sein
  nextionsend();

  if (valc <= 1210){
  nextion.print("bat.pco=");
  nextion.print(63488);  //muss int sein
  nextionsend();
  }

  else if (valc <= 1250){
  nextion.print("bat.pco=");
  nextion.print(65504);  //muss int sein
  nextionsend();
  }
 

  else if (valc >= 1265){
  nextion.print("bat.pco=");
  nextion.print(2016);  //muss int sein
  nextionsend();
  }



  
  if (valc <=1300){
  nextion.print("p2.pic=");
  nextion.print(8);
  nextionsend();} else {
  nextion.print("p2.pic=");
  nextion.print(7);
  nextionsend();
  }

  VOLT_average = VOLT_average*10;
  prozent = map(VOLT_average, 119, 128, 40, 100);
  prozent = constrain(prozent, 0, 100);


  nextion.print("t4.txt=" + cmd + prozent + "%" + cmd);
  nextionsend();
  


//Stromstärke Batterie---------------------------------------------------------------


 STROM_average = 0;
 for (int i=0; i < 100; i++) {
 STROM_average = STROM_average + analogRead(STROM_SENSOR);
 }
 STROM_average = STROM_average/100;

 // asensor = analogRead(SENSOR_Strom);
  acalc= (STROM_average/100)/2;
  a=(acalc-2.5)/STROM_korrektur-1.01;
  if (a<=0) a=0;
 // Serial.println(acalc);
  nextion.print("ah.txt=" + cmd + a + " A" + cmd);
  nextionsend();


  
//Watt---------------------------------------------------------------
  w = (valc/100) * a;
  if (a<=0) w=0;
  w=w,1;
  nextion.print("watt.txt=" + cmd + w + " Watt" + cmd);
  nextionsend();

//Uhr---------------------------------------------------------------
DateTime now = rtc.now();
h = now.hour();
m = now.minute();
yy = now.year();
mm = now.month();
dd = now.day();

String hourStr = (h < 10 ? "0" : "") + String(h);
String minuteStr = (m < 10 ? "0" : "") + String(m);
String yearStr = String(yy);
String monthStr = (mm < 10 ? "0" : "") + String(mm);
String dayStr = (dd < 10 ? "0" : "") + String(dd);

nextion.print("uhr.txt=" + cmd + hourStr + ":" + minuteStr + cmd);
nextionsend();

nextion.print("t8.txt=" + cmd + dayStr + "." + monthStr + "." + yearStr + cmd);
nextionsend();

nextion.print("h.txt=" + cmd + h + cmd);
nextionsend();

nextion.print("m.txt=" + cmd + m + cmd);
nextionsend();

nextion.print("dd.txt=" + cmd + dd + cmd);
nextionsend();

nextion.print("mm.txt=" + cmd + mm + cmd);
nextionsend();

nextion.print("yy.txt=" + cmd + yy + cmd);
nextionsend();



//Schalter Test-----------------------------------------------
  if (nextion.available() > 0) {
    String commandh = nextion.readStringUntil('0xFF');
    //Serial.print("Received: ");
   // Serial.println(command);
    //delay(10);
    String myStringh = commandh;
  if (myStringh.indexOf("hplus") >= 0) {
   //Serial.println("Substring found!");
  // Serial.println(myString);
    h++;
    rtc.adjust(DateTime(rtc.now().year(), rtc.now().month(), rtc.now().day(), h, rtc.now().minute(), rtc.now().second()));
  
  //Serial.println(test);
    nextion.print("h.txt=" + cmd + h +  cmd);
    nextionsend();
  }
    
  else if (myStringh.indexOf("hminus") >= 0) {
   //Serial.println("Substring found!");
  // Serial.println(myString);
    h--;
    rtc.adjust(DateTime(rtc.now().year(), rtc.now().month(), rtc.now().day(), h, rtc.now().minute(), rtc.now().second()));
  
  //Serial.println(test);
    nextion.print("h.txt=" + cmd + h +  cmd);
    nextionsend();
  }

  else if (myStringh.indexOf("mplus") >= 0) {
   //Serial.println("Substring found!");
  // Serial.println(myString);
    m++;
    rtc.adjust(DateTime(rtc.now().year(), rtc.now().month(), rtc.now().day(),rtc.now().hour(), m , rtc.now().second()));
  
  //Serial.println(test);
    nextion.print("m.txt=" + cmd + m +  cmd);
    nextionsend();
  }

    else if (myStringh.indexOf("mminus") >= 0) {
   //Serial.println("Substring found!");
  // Serial.println(myString);
    m--;
    rtc.adjust(DateTime(rtc.now().year(), rtc.now().month(), rtc.now().day(), rtc.now().hour(), m , rtc.now().second()));
  
  //Serial.println(test);
    nextion.print("m.txt=" + cmd + m +  cmd);
    nextionsend();
  }

    else if (myStringh.indexOf("ddplus") >= 0) {
   //Serial.println("Substring found!");
  // Serial.println(myString);
    dd++;
    rtc.adjust(DateTime(rtc.now().year(), rtc.now().month(), dd , rtc.now().hour(), rtc.now().minute() , rtc.now().second()));
     //Serial.println(test);
    nextion.print("dd.txt=" + cmd + dd +  cmd);
    nextionsend();
  
  }

    else if (myStringh.indexOf("ddminus") >= 0) {
   //Serial.println("Substring found!");
//   Serial.println(myString);
    dd--;
    rtc.adjust(DateTime(rtc.now().year(), rtc.now().month(), dd , rtc.now().hour(), rtc.now().minute() , rtc.now().second()));
    nextion.print("dd.txt=" + cmd + dd +  cmd);
    nextionsend();;
  }

    else if (myStringh.indexOf("mmplus") >= 0) {
   Serial.println("Substring found!");
   //Serial.println(myStringh);
    mm++;
    rtc.adjust(DateTime(rtc.now().year(), mm, rtc.now().day(), rtc.now().hour(), rtc.now().minute() , rtc.now().second()));
  
    nextion.print("mm.txt=" + cmd + mm +  cmd);
    nextionsend();
  }



 }


//Coulomb count ------------------------------------------------------------------------------

  // Lese den Stromsensor aus und berechne den aktuellen Strom
  current1 = a;
  
  // Lese den Spannungssensor aus und berechne die aktuelle Spannung
  voltage1 = voltage.toFloat();
 // voltage1 = 12.5;
  
  // Berechne den kumulierten Strom
  accumulated_current += current1 * SAMPLE_INTERVAL;
  
  // Berechne die kumulierte Kapazität
  accumulated_capacity = accumulated_current / 3600.0 * 1000.0 / MAXIMUM_CAPACITY * 100.0;
  
  // Berechne die verbleibende Kapazität basierend auf der Spannung
  float voltage_ratio = (voltage1 - MINIMUM_VOLTAGE) / VOLTAGE_DIFFERENCE;
  remaining_capacity = MAXIMUM_CAPACITY * voltage_ratio;

  battery_life = (voltage.toFloat()*(MAXIMUM_CAPACITY/1000)/w)*60;

  
  
  // Gib die aktuellen Werte aus
  /*Serial.print("Strom: ");
  Serial.print(current1);
  Serial.print(" A, Spannung: ");
  Serial.print(voltage1);
  Serial.print(" V, Kumulierter Strom: ");
  Serial.print(accumulated_current/60);
  Serial.print(" As, Kumulierte Kapazität: ");
  Serial.print(accumulated_capacity);
  Serial.print(" %, Verbleibende Kapazität: ");
  Serial.print(remaining_capacity/1000);
  Serial.println(" Ah");*/

  nextion.print("cap.txt=" + cmd + battery_life + " h " + accumulated_current  +   cmd);
  nextionsend();
  
  delay(SAMPLE_INTERVAL * 1000);







//delay(5000);

}

void nextionsend(){
  nextion.write(0xFF);
  nextion.write(0xFF);
  nextion.write(0xFF);
  delay(10);
  }
