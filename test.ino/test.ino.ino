
#include <OneWire.h>
#include <DallasTemperature.h>

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

}

void loop()
{

  Serial.print("Weight: ");
  long weight = getWeight();
  Serial.print(weight);
  Serial.println(" uV");  
  
  float temperature_outside = getTemperature(0);
  Serial.println("Temperature: " + String(temperature_outside));

  delay(500);
}
