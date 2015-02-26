

#if ARDUINO < 100
#include <WProgram.h>
#include <pins_arduino.h>  // fix for broken pre 1.0 version - TODO TEST
#else
#include <Arduino.h>
#endif

#ifndef dht22_h
#define dht22_h
/*!
 * \brief An interrupt-driven library for the DHT22 sensor
 *
 * This class provides an interrupt-driven interface to the DHT22 sensor.
 * The user must attach an ISR to the falling edge of the DHT22 data pin
 * and invoke #onFallingEdge() in the ISR.
 *
 * Reading can be started with #startRead(), which returns immediately,
 * or #blockingRead(), which waits for the reading to complete.
 *
 * If #state() returns #Done, the data has been read and can be accessed
 * with #getTemp() and #getHumidity(). Be aware of the measuring units!!
 * If #state() returns #Invalid, #lastResult() can give some information on
 * the cause.
 *
 * Usage example:
 * \code
 * DHT22 dht(2);
 *
 * void dhtISR() {
 *    dht.onFallingEdge();
 * }
 *
 * void setup() {
 *     Serial.begin(9600);
 *     // On Arduino Uno, interrupt 0 is attached to pin 2
 *     attachInterrupt(0, &dhtISR, FALLING);
 *     // let the sensor warm up for 2 seconds
 *     delay(2000);
 * }
 *
 * void loop() {
 *   uint32_t start = micros();
 *   if (dht.blockingRead() == DHT22::Ok) {
 *       uint32_t stop = micros();
 *       Serial.print("Temperature: ");
 *       Serial.print(dht.getTemp(),1);
 *       Serial.println("C");
 *       Serial.print("Humidity: ");
 *       Serial.print(dht.getHumidity(),1);
 *       Serial.println("%");
 *       Serial.print("Read Time: ");
 *       Serial.println(stop - start);
 *   } else {
 *     switch (dht.lastResult())
 *     {
 *       case DHT22::Ok:
 *         Serial.println("Ok");
 *         break;
 *       case DHT22::None:
 *         Serial.println("None");
 *         break;
 *       case DHT22::ChecksumMismatch:
 *         Serial.println("CRC error");
 *         break;
 *       case DHT22::WakeUpError:
 *         Serial.println("WakeUp error");
 *         break;
 *       case DHT22::DataError:
 *         Serial.println("Data Error");
 *         break;
 *       default: 
 *         Serial.print("Unknown error"); 
 *         break;
 *     }
 * }
 *
 *     // Minimum interval between reads.
 *     delay(2000);
 * }
 * \endcode
 */

class DHT22
{
public:
    /*!
     * \brief Possible result values
     */
    enum Result {
        None, //!< Data has never been read
        Ok, //!< Yay!!
        ChecksumMismatch, //!< The data reading was currupted
        WakeUpError, //!< There was a protocol timing error during the sensor wake-up
        DataError //!< There was a protocol timing error during data transmission
    };
    /*!
     * \brief The status of the library state machine
     */
    enum Status {
        WakingUp, //!< The sensor is waking up
        Reading, //!< The sensor is transmitting data
        Done, //!< Data was successfully read
        Invalid //!< Data reading failed or was never attempted
    };

    /*!
     * \brief Construct a DHT22 library instance
     * \param pin The data pin of the sensor
     */
    DHT22(int pin);
    /*!
     * \brief Signals the library that the data pin has gone low
     */
    void onFallingEdge();
    /*!
     * \brief Wakes up the sensor and starts waiting for data
     * \return Whether the reading was started
     */
    bool startRead();
    /*!
     * \brief Starts reading, if necessary, and waits for result
     * \return The reading result
     */
    Result blockingRead();
    /*!
     * \brief The last temperature reading
     * \return The temperature in tents of Celsius degree
     */
    float getTemp();
    /*!
     * \brief The last humidity reading
     * \return The relative humidity in per mil
     */
    float getHumidity();
    /*!
     * \brief The sensor status
     */
    Status state();
    /*!
     * \brief The result of the last reading
     */
    Result lastResult();
private:
    void delayus(uint16_t us);
    int _pin;
    volatile Status  _state;
    volatile Result _result;
    volatile uint8_t _data[5];
    volatile uint8_t _bit;
    volatile uint8_t _byte;
    volatile unsigned long _lastEdge;
    volatile float _humidity;
    volatile float _temp;
};
#endif
