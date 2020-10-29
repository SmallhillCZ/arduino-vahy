
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// WIFI
const char *wifiName = "MW40V_F05B";
const char *wifiPass = "b836a011f4ace30c";

// WEIGHT
const int pSCK = D4;
const int pDT = D3;

#define channel_A_128 1
#define channel_B_32 2 
#define channel_A_64 3

const byte weightMeasuringType = channel_A_128;

// TEMPERATURE
const int pOneWire = D5;  
OneWire oneWire(pOneWire);
DallasTemperature sensors(&oneWire);
int deviceCount = 0;

// HTTP
HTTPClient http;
String api = "http://spindl.smallhill.cz/input-bees.php";

void initialize()
{
  Serial.begin(9600);

  Serial.println("Version 0.0.1");
  Serial.println("Initializing...");
  
  sensors.begin();

  pinMode(pSCK, OUTPUT);
  pinMode(pDT, INPUT);  

  byte index;
  for (index = 0; index < weightMeasuringType; index++)
  {
    digitalWrite (pSCK, HIGH);
    digitalWrite (pSCK, LOW);
  }
  
  WiFi.begin(wifiName, wifiPass);
}

long getWeight()
{
  byte index;
  long measuredValue = 0L;
  
  // načtení 24-bit dat z modulu
  while(digitalRead (pDT)){    
    yield();
  }
  
  for (index = 0; index < 24; index++)
  {
    digitalWrite (pSCK, HIGH);
    measuredValue = (measuredValue << 1) | digitalRead (pDT);
    digitalWrite (pSCK, LOW);
  }  
  
  // konverze z 24-bit dvojdoplňkového čísla
  // na 32-bit znaménkové číslo
  if (measuredValue >= 0x800000 )
  {
    measuredValue = measuredValue | 0xFF000000L;
  }
  
  // přepočet výsledku na mikrovolty podle zvoleného
  // kanálu a zesílení
  switch(weightMeasuringType) {
    case 1: measuredValue = measuredValue/ 128/2; break;
    case 2: measuredValue = measuredValue/ 32/2; break;
    case 3: measuredValue = measuredValue/ 64/2; break;
  }
  return measuredValue;
}

int getTemperatureCount()
{
  return sensors.getDeviceCount();
}

float getTemperature(int tempIndex)
{
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(tempIndex);
}

void setup()
{

  initialize();
  
  String data = "";

  Serial.print("Waiting for wifi");
    
  while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
  }

  Serial.println("Connected.");

  Serial.print("Initializing temperature sensors... ");
  deviceCount = sensors.getDeviceCount();
  Serial.println(String(deviceCount) + " sensors");

  Serial.print("Weight: ");
  long weight = getWeight();
  Serial.print(weight);
  Serial.println(" uV");
  data += ("weight=" + String(weight));
  
  float temperature_outside = getTemperature(0);
  Serial.println("Temperature outside: " + String(temperature_outside));
  data += ("&temperature_outside=" + String(temperature_outside));  

//  float temperature_inside= getTemperature(1);
//  Serial.println("Temperature inside: " + temperature_inside);
//  data += ("&temperature_inside=" + String(temperature_inside));  

  Serial.println("Sending data...");
  String url = api + "?" + data;
  Serial.println("GET " + url);
  http.begin(url);
  int httpCode = http.GET();
  if(httpCode == 200)
  {
    Serial.println("Data sent.");
  }
  else{
    Serial.println("HTTP error: " + String(httpCode));
  }

  // pauza 10 min
  Serial.println("Wait 10 minutes");
  ESP.deepSleep(10 * 60 * 1e6); 
}

void loop()
{
  
}
