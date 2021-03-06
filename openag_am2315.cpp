/**
 *  \file openag_am2315.cpp
 *  \brief Air temperature and air humidity sensor.
 */
/***************************************************
  This is a library for the AM2315 Humidity & Temp Sensor

  Designed specifically to work with the AM2315 sensor from Adafruit
  ----> https://www.adafruit.com/products/1293

  These displays use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution

  Modified for OpenAg
 ****************************************************/
#include "openag_am2315.h"

Am2315::Am2315(String id, String* parameters) : Peripheral(id, parameters) {}

Am2315::~Am2315() {}

void Am2315::begin() {
  Wire.begin(); // enable i2c port
  _time_of_last_reading = 0;
  _air_temperature_key = "air_temperature";
  _air_humidity_key = "air_humidity";
  _air_temperature_message = _air_temperature_key + ",error";
  _air_humidity_message = _air_humidity_key + ",error";
}

String Am2315::get(String key) {
  if (key == _air_temperature_key) {
    return getAirTemperature();
  }
  else if (key == _air_humidity_key) {
    return getAirHumidity();
  }
  else {
    return String(key + ",error");
  }
}

String Am2315::set(String key, String value) {
  return String(key + ",error");
}

String Am2315::getAirTemperature() {
  if (millis() - _time_of_last_reading > _min_update_interval){ // can only read sensor so often
    readData();
    _time_of_last_reading = millis();
  }
  return _air_temperature_message;
}

String Am2315::getAirHumidity() {
  if (millis() - _time_of_last_reading > _min_update_interval) { // can only read sensor so often
    readData();
    _time_of_last_reading = millis();
  }
  return _air_humidity_message;
}

void Am2315::readData() {
  uint8_t reply[10];
  boolean is_good_reading = true;

  // Wake up sensor
  Wire.beginTransmission(_i2c_address);
  delay(2);
  Wire.endTransmission();

  // Send request to sensor
  Wire.beginTransmission(_i2c_address);
  Wire.write(_read_register);
  Wire.write(0x00);  // start at address 0x0
  Wire.write(4);  // request 4 bytes data
  Wire.endTransmission();

  // Give sensor time to process request
  delay(10);

  // Read sensor
  Wire.requestFrom(_i2c_address, 8);
  for (uint8_t i=0; i<8; i++) {
    reply[i] = Wire.read();
  }

  // Check for failure
  if (reply[0] != _read_register) {
    is_good_reading = false;
  }
  else if (reply[1] != 4) {
    is_good_reading = false;
  }
  else { // good reading
    // Process air humidity
    air_humidity = reply[2];
    air_humidity *= 256;
    air_humidity += reply[3];
    air_humidity /= 10;

    // Process air temperature
    air_temperature = reply[4] & 0x7F;
    air_temperature *= 256;
    air_temperature += reply[5];
    air_temperature /= 10;
    if (reply[4] >> 7) air_temperature = -air_temperature;
  }

  // Update messages
  if (is_good_reading) {
    _air_humidity_message = id + "," + _air_humidity_key + "," + String(air_humidity, 1);
    _air_temperature_message = id + "," + _air_temperature_key + "," + String(air_temperature, 1);

  }
  else { // read failure
    _air_humidity_message = id + "," + _air_humidity_key + ",error";
    _air_temperature_message = id + "," + _air_temperature_key + ",error";
  }
}
