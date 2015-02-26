#include <DHT22.h>

#define DHT_PIN 9     // On Arduino Uno, interrupt 0 is attached to pin 2  
DHT22 dht(DHT_PIN);

void dhtISR() {
   dht.onFallingEdge();
}

void setup() {
     Serial.begin(9600);
     attachInterrupt(DHT_PIN, &dhtISR, FALLING);
     // let the sensor warm up for 2 seconds
     delay(2000);
}

void loop() {
   uint32_t start = micros();
   if (dht.blockingRead() == DHT22::Ok) {
       uint32_t stop = micros();
       Serial.print("Temperature: ");
       Serial.print(dht.getTemp(),1);
       Serial.println("C");
       Serial.print("Humidity: ");
       Serial.print(dht.getHumidity(),1);
       Serial.println("%");
       Serial.print("Read Time: ");
       Serial.println(stop - start);
   } else {
     switch (dht.lastResult())
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
     // Minimum interval between reads.
     delay(2000);
}