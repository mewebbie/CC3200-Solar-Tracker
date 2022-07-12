#ifndef __CC3200R1M1RGC__ 
#include <SPI.h> 
#endif 
#include <WiFi.h> 
#include <Wire.h> 
#include "Adafruit_TMP006.h" 
#include <stdlib.h> 
char thingSpeakAddress[] = "api.thingspeak.com"; 
String writeAPIKey = "S5CEPJUL7KFTHSES"; 
const int updateThingSpeakInterval = 16 * 1000; 
char buffer[25]; 
char ssid[] = "R2C";
char password[] = "r2cudt@2021";
WiFiClient client;
unsigned long lastConnectionTime = 0; 
boolean lastConnected = false; 
const unsigned long postingInterval = 10*1000; 
int failedCounter = 0; 

void setup() {
 Serial.begin(115200);
 
 pinMode(2,INPUT);    // LDR Sensor-1 placed along EAST 
 pinMode(6,INPUT);    // LDR Sensor-2 placed along WEST
 pinMode(23,INPUT);   // LDR Sensor-3 attached with Solar Panels

 pinMode(9, OUTPUT);  // Yellow LED
 pinMode(10, OUTPUT); // Green LED
 pinMode(29, OUTPUT); // Red LED
 pinMode(3, OUTPUT);  // Buzzer 
 
Serial.print("Attempting to connect to Network named: ");
Serial.println(ssid);
WiFi.begin(ssid, password);
while ( WiFi.status() != WL_CONNECTED) {
Serial.print(".");
delay(300); 
} 

Serial.println("\nYou're connected to the network");
Serial.println("Waiting for an ip address"); 
while (WiFi.localIP() == INADDR_NONE) { 
Serial.print("."); 
delay(300); 
} 
Serial.println("\nIP Address obtained"); 
printWifiStatus(); 
}
void loop() { 
while (client.available()) { 
char c = client.read(); 
Serial.print(c); 
} 
if (!client.connected() && lastConnected) { 
Serial.println(); 
Serial.println("disconnecting."); 
client.stop(); 
} 

int maxIntensity = 4095; // A THRESHOLD to determine the need of altering the DIRECTION of solar PANEL.

if (!client.connected() && (millis() - lastConnectionTime > postingInterval)) { 
//float objt = tmp006.readObjTempC();
 float LDR_EAST  = analogRead(2);
 float LDR_WEST  = analogRead(6);
 float LDR_PANEL = analogRead(23);
 int dirInd = 0; // This is the value to sent to thingspeak INDICATING required change in DIRECTION.
 
 // Checking if current position of panel is ideal based on the threshold value "maxIntensity"
 if(LDR_PANEL<0.75*maxIntensity) {
    Serial.print("Current Direction LDR value: ");
    Serial.print(LDR_PANEL);
    Serial.println("");
    
    // BUZZER set HIGH to indicate that change of DIRECTION is required.
    digitalWrite(3, HIGH);
    
    // Checking in which direction panels must be moved based on LDR SENSOR values placed along EAST & WEST
    if(LDR_EAST > LDR_WEST) {
      Serial.print("LDR Value EAST "); Serial.print(LDR_EAST);
      Serial.print("LDR Value WEST "); Serial.print(LDR_WEST);
      Serial.println("--ALERT--\nMOVE THE PANELS TOWARDS EAST");
      dirInd = -100;          // -100 indicating TOWARDS EAST
      digitalWrite(10, HIGH); // GREEN LED indicating TOWARDS EAST

      digitalWrite(9, LOW);
      digitalWrite(29, LOW);
    }
    else {
      Serial.print("LDR Value EAST "); Serial.print(LDR_EAST);
      Serial.print("LDR Value WEST "); Serial.print(LDR_WEST);
      Serial.println("--ALERT--\nMOVE THE PANELS TOWARDS WEST");
      dirInd = 100;           // +100 indicating TOWARDS WEST
      digitalWrite(29, HIGH); // RED LED indicating TOWARDS WEST

      digitalWrite(10, LOW);
      digitalWrite(9, LOW);
    }
 }

 else {
  // Buzzer is turned off as the current direction of panels are ideal enough.
  digitalWrite(3, LOW);
  
  digitalWrite(9, HIGH); // YELLOW LED indicates halt state i.e panels are well placed for optimum sunlight
  Serial.println("PANELS are placed along OPTIMUM DIRECTION");
  dirInd = 0;
 }

String sLDR_PANEL = dtostrf(LDR_PANEL,3,3,buffer);
String sLDR_EAST  = dtostrf(LDR_EAST,3,3,buffer);
String sLDR_WEST  = dtostrf(LDR_WEST,3,3,buffer);
String sdirInd    = dtostrf(dirInd,3,3,buffer); 
 

updateThingSpeak("field1=" + sdirInd+"&field2=" + sLDR_PANEL+"&field3=" + sLDR_EAST + "&field4=" + sLDR_WEST); 
} 
lastConnected = client.connected();
 
} 
void updateThingSpeak(String tsData) 
{ 
if (client.connect(thingSpeakAddress, 80)) 
{
client.print("POST /update HTTP/1.1\n"); 
client.print("Host: api.thingspeak.com\n"); 
client.print("Connection: close\n"); 
client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n"); 
client.print("Content-Type: application/x-www-form-urlencoded\n"); 
client.print("Content-Length: "); 
client.print(tsData.length()); 
Serial.println(">>TSDATALength=" + tsData.length()); 
client.print("\n\n");
client.print(tsData);
Serial.println(">>TSDATA=" + tsData);
lastConnectionTime = millis();
if (client.connected()) 
{ 
Serial.println("Connecting to ThingSpeak...");
Serial.println();
failedCounter = 0; 
}
else
{ 
failedCounter++;
Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");
Serial.println();
} 
} 
else 
{ 
failedCounter++; 
Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");
Serial.println(); 
lastConnectionTime = millis(); 
} 
}
int getLength(int someValue) { 
int digits = 1; 
int dividend = someValue / 10; 
while (dividend > 0) { 
dividend = dividend / 10; 
digits++; 
} 
return digits; 
} 
void printWifiStatus() {
Serial.print("SSID: ");
Serial.println(WiFi.SSID());
IPAddress ip = WiFi.localIP(); 
Serial.print("IP Address: "); 
Serial.println(ip); 
long rssi = WiFi.RSSI(); 
Serial.print("signal strength (RSSI):"); 
Serial.print(rssi); 
Serial.println(" dBm"); 
}
