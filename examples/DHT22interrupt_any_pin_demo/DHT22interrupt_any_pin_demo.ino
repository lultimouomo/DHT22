#include "DHT22.h"

//--------------------------------------------------------------------------------------------------
//Sensor DHT22 Vars
#define DHT_PIN 9
DHT22 Dht(DHT_PIN);


//--------------------------------------------------------------------------------------------------
//Pin change Vars
byte pinIntConfig[] = { DHT_PIN, 10, 11 };    // Define arduino pins to look for changes (D8 to D13 only)
struct  PortStatusStruct { byte value, valuePrevious, pinChangedFlag; byte portMaskBit; };
PortStatusStruct PortStatus[sizeof(pinIntConfig)]; // Each PortStatus index will have the monitored value of the defined pin in pinIntConfig

int userCommand;
void setup() {
  Serial.begin(9600);

  pinInit();
  
  dhtInit();
  
  // let the sensor warm up for 2 seconds
  delay(2000);
  userCommand = 0;
  printMenu();
}
void loop() {
  if(userCommand == 1) {
   for (byte p=0; p < sizeof(pinIntConfig); p++) {
      byte curStat = pinGetPortValue(p);
      Serial.print("PIN");
      Serial.print(pinIntConfig[p]);
      Serial.print(": ");
      if (curStat == 0xFF)
        Serial.println("nochange");  // Send Pin Status
      else
        Serial.println(curStat);  // Send Pin Status
   }
   userCommand = 0;
  }

  if(userCommand == 2) {
   dhtReadSensor();
   userCommand = 0;
  }
  
  if(Serial.available())
  {
    userCommand=Serial.parseInt();
    Serial.print("Got: ");
    Serial.println(userCommand);
  }
}

void printMenu() {
    Serial.println("DHT22 interrupt on any pin demo");
    Serial.println(" 1 - PIN status.");
    Serial.println(" 2 - DHT22 data.");
    Serial.println("Press a key...");
}

// #########################################################################
// ## Sensor DHT22
// #########################################################################

void dhtInit() {
}

void dhtReadSensor() {
    // READ DATA
    uint32_t start = micros();
    if (Dht.blockingRead() == DHT22::Ok) {
        uint32_t stop = micros();
        Serial.print("Temperature: ");
        Serial.print(Dht.getTemp(),1);
        Serial.println("C");
        Serial.print("Humidity: ");
        Serial.print(Dht.getHumidity(),1);
        Serial.println("%");
        Serial.print("Read Time:");
        Serial.println(stop - start);
    } else {
      switch (Dht.lastResult())
      {
        case DHT22::Ok:
          Serial.println("Ok");
          break;
        case DHT22::None:
          Serial.println("None");
          break;
        case DHT22::ChecksumMismatch:
          Serial.println("CRC error");
          break;
        case DHT22::WakeUpError:
          Serial.println("WakeUp error");
          break;
        case DHT22::DataError:
          Serial.println("Data Error");
          break;
        default: 
          Serial.print("Unknown error"); 
          break;
      }
  }
}

// #########################################################################
// ## Pin change detection
// #########################################################################
/* Arduino pin     | Port | Change Int.  | Pin Change Mask    | Change Int Flag | Int Vector
 * D8-D13          =  PB  | PCICR:PCIE0  | PCMSK0:PCINT0-5    | PCIFR:PCIF0     | PCINT0
 * A0-A5 (D14-D19) =  PC  | PCICR:PCIE1  | PCMSK1:PCINT8-13   | PCIFR:PCIF1     | PCINT1
 * D0-D7           =  PD  | PCICR:PCIE2  | PCMSK2:PCINT16-23  | PCIFR:PCIF2     | PCINT2
 */ 
// Init ports as input+pullup and set interrupts from defined pins in pinIntConfig
void pinInit() {
   Serial.print("PIN: Watch ");
   for (byte p=0; p < sizeof(pinIntConfig); p++) {
      // enable interrupt for pin p
      pinMode(pinIntConfig[p],INPUT_PULLUP);
      PCIFR  |= bit (digitalPinToPCICRbit(pinIntConfig[p]));                   // Clear Pin Change Interrupt Flag Register
      PCICR  |= bit (digitalPinToPCICRbit(pinIntConfig[p]));                   // Set Pin Change Interrupt Control Register Enable
      *digitalPinToPCMSK(pinIntConfig[p]) |= bit (digitalPinToPCMSKbit(pinIntConfig[p]));  // Set Pin Change Enable Mask

      //set initial values
      PortStatus[p].portMaskBit=bit(digitalPinToPCMSKbit(pinIntConfig[p]));    // Save pin mask for use in int later
      PortStatus[p].value = (PortStatus[p].portMaskBit & PINB) >> (PortStatus[p].portMaskBit - 1); // Status of PORTB pins only for the enabled mask pins in PCMSK0
      PortStatus[p].valuePrevious = PortStatus[p].value;
      PortStatus[p].pinChangedFlag = true;
      Serial.print(pinIntConfig[p]);
      Serial.print(" ");
   }
   Serial.println();
}

//Get current value for port
byte pinGetPortValue(byte portId)
{
    byte curStat = 0xFF;
    if(PortStatus[portId].pinChangedFlag) {
      if (PortStatus[portId].value != PortStatus[portId].valuePrevious) {
        curStat=PortStatus[portId].valuePrevious;
        PortStatus[portId].pinChangedFlag = true;
      }
      else {
        curStat=PortStatus[portId].value;
        PortStatus[portId].pinChangedFlag = false;
      }
      PortStatus[portId].valuePrevious=PortStatus[portId].value;
    }
    return curStat;
}

// Interrupt on Pin Change PCINT0..PCINT7 (D8 to D13)
ISR(PCINT0_vect)
{
  if ((PortStatus[0].portMaskBit & PINB) >> (PortStatus[0].portMaskBit - 1) == 0) { // Detect if PortStatus[0] of DHT sensor got low.
     Dht.onFallingEdge();
  }

  // Pin status monitoring
  for (byte p=0; p < sizeof(pinIntConfig); p++) {
    PortStatus[p].value = (PortStatus[p].portMaskBit & PINB) >> (PortStatus[p].portMaskBit - 1); // PortStatus only for the pin in the mask
    if (!PortStatus[p].pinChangedFlag)
       PortStatus[p].valuePrevious = PortStatus[p].value;
    PortStatus[p].pinChangedFlag = true;
  }
}
