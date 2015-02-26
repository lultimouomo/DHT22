#include "DHT22.h"

namespace {
namespace Trigger {
    const uint16_t low = 2000;
    const uint16_t high = 40;
}
namespace WakeUp {
    const uint8_t min = 125;
    const uint8_t max = 190;
}
namespace Bit {
    const uint8_t min = 60;
    const uint8_t threshold = 100;
    const uint8_t max = 145;
}
}

DHT22::DHT22(int pin):
    _pin(pin),
    _state(Invalid),
    _result(None),
    _humidity(0),
    _temp(0)
{
}

void DHT22::delayus(uint16_t us)
{
    uint16_t usStart = (uint16_t)micros();
    while (((uint16_t)micros() - usStart) < us) {   } // delay us
}

bool DHT22::startRead() {
    if (_state == Invalid || _state == Done) {
        for (uint8_t i=0; i< sizeof(_data); i++) {
            _data[i] = 0;
        }
        _bit = 7;
        _byte = 0;

        // Trigger the sensor
        _state = Invalid;
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
        DHT22::delayus(Trigger::low);
        pinMode(_pin, INPUT_PULLUP);
        DHT22::delayus(Trigger::high);
        _lastEdge = micros();
        _state = WakingUp;
        return true;
    }
    return false;
}

DHT22::Result DHT22::blockingRead() {
    startRead();
    while((_state != Done && _state != Invalid)){}
    return lastResult();
}

void DHT22::onFallingEdge() {
    unsigned long now = micros();
    unsigned int elapsed = now - _lastEdge;
    _lastEdge = now;
    switch(_state) {
    case WakingUp:
        if(elapsed > WakeUp::min && elapsed < WakeUp::max) {
            _state = Reading;
        } else {
            _result = WakeUpError;
            _state = Invalid;
        }
        break;
    case Reading:
        if(elapsed > Bit::min && elapsed < Bit::max) {
            if(elapsed > Bit::threshold)  _data[_byte] |= (1 << _bit);
            if (_bit == 0) {
                _bit = 7;
                if(++_byte == sizeof(_data)) {
                    uint8_t sum = _data[0] + _data[1] + _data[2] + _data[3];
                    if (_data[4] != sum) {
                        _result = ChecksumMismatch;
                        _state = Invalid;
                    } else {
                        _humidity = word(_data[0], _data[1]) * 0.1;
                        _temp = (_data[2] & 0x80 ?
                                    -word(_data[2] & 0x7F, _data[3]) :
                                    word(_data[2], _data[3])
                                ) * 0.1;
                        _result = Ok;
                        _state = Done;
                    }
                    break;
                }
            }
            else _bit--;
        } else {
            _result = DataError;
            _state = Invalid;
        }
        break;
    default:
        break;
    }
}

DHT22::Status DHT22::state() {
    return _state;
}

DHT22::Result DHT22::lastResult() {
    return _result;
}

float DHT22::getTemp() {
    return _temp;
}

float DHT22::getHumidity() {
    return _humidity;
}
