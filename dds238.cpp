    
/* Library for reading DDS238-4 W; DDS238 4 ZN/S; DDS238 2 ZN/S; DDS238 1 ZN Modbus Energy meters.
*  Reading via Hardware or Software Serial library & rs232<->rs485 converter
*  2019 ENina (tested on DDS238-4 Wemos d1 mini->ESP8266 with Arduino 1.8.9-beta & 2.3.0 esp8266 core)
*  Based on SDM_Energy_Meter 2016-2019 Reaper7
*  crc calculation by Jaime García (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino/)
*/
//------------------------------------------------------------------------------
#include "dds238.h"
//------------------------------------------------------------------------------
#ifdef USE_HARDWARESERIAL
#if defined ( ESP8266 )
dds238::dds238(HardwareSerial& serial, long baud, int dere_pin, int config, bool swapuart) : sdmSer(serial) {
  this->_baud = baud;
  this->_dere_pin = dere_pin;
  this->_config = config;
  this->_swapuart = swapuart;
}
#elif defined ( ESP32 )
dds238::dds238(HardwareSerial& serial, long baud, int dere_pin, int config, int8_t rx_pin, int8_t tx_pin) : sdmSer(serial) {
  this->_baud = baud;
  this->_dere_pin = dere_pin;
  this->_config = config;
  this->_rx_pin = rx_pin;
  this->_tx_pin = tx_pin;
}
#else
dds238::dds238(HardwareSerial& serial, long baud, int dere_pin, int config) : sdmSer(serial) {
  this->_baud = baud;
  this->_dere_pin = dere_pin;
  this->_config = config;
}
#endif
#else
dds238::dds238(SoftwareSerial& serial, long baud, int dere_pin) : sdmSer(serial) {
  this->_baud = baud;
  this->_dere_pin = dere_pin;
}
#endif

dds238::~dds238() {
}

void dds238::begin(void) {
#ifdef USE_HARDWARESERIAL
#if defined ( ESP8266 )
  sdmSer.begin(_baud, (SerialConfig)_config);
#elif defined ( ESP32 )
  sdmSer.begin(_baud, _config, _rx_pin, _tx_pin);
#else
  sdmSer.begin(_baud, _config);
#endif
#else
  sdmSer.begin(_baud);
#endif
#ifdef USE_HARDWARESERIAL
#ifdef ESP8266
  if (_swapuart)
    sdmSer.swap();
#endif
#endif
  if (_dere_pin != NOT_A_PIN) {
    pinMode(_dere_pin, OUTPUT);                                                 //set output pin mode for DE/RE pin when used (for control MAX485)
    digitalWrite(_dere_pin, LOW);                                               //set init state to receive from dds238 -> DE Disable, /RE Enable (for control MAX485)
  }
}

float dds238::readVal(uint16_t reg,  byte type, uint8_t node) {
  uint16_t temp;
  unsigned long resptime;
  uint8_t sdmarr[FRAMESIZE] = {node, dds238_B_02, 0, 0, dds238_B_05, dds238_B_06, 0, 0, 0};
  float res = NAN;
  uint16_t readErr = dds238_ERR_NO_ERROR;

  sdmarr[2] = highByte(reg);
  sdmarr[3] = lowByte(reg);

  temp = calculateCRC(sdmarr, FRAMESIZE - 3);                                   //calculate out crc only from first 6 bytes

  sdmarr[6] = lowByte(temp);
  sdmarr[7] = highByte(temp);

#ifndef USE_HARDWARESERIAL
  sdmSer.listen();                                                              //enable softserial rx interrupt
#endif

  while (sdmSer.available() > 0)  {                                             //read serial if any old data is available
    sdmSer.read();
  }

  if (_dere_pin != NOT_A_PIN)
    digitalWrite(_dere_pin, HIGH);                                              //transmit to dds238  -> DE Enable, /RE Disable (for control MAX485)

  delay(2);                                                                     //fix for issue (nan reading) by sjfaustino: https://github.com/reaper7/SDM_Energy_Meter/issues/7#issuecomment-272111524

  sdmSer.write(sdmarr, FRAMESIZE - 1);                                          //send 8 bytes

  sdmSer.flush();                                                               //clear out tx buffer

  if (_dere_pin != NOT_A_PIN)
    digitalWrite(_dere_pin, LOW);                                               //receive from dds238 -> DE Disable, /RE Enable (for control MAX485)

  resptime = millis() + MAX_MILLIS_TO_WAIT;

  while (sdmSer.available() < FRAMESIZE) {
    if (resptime < millis()) {
      readErr = dds238_ERR_TIMEOUT;                                                //err debug (4)
      break;
    }
    yield();
  }

  if (readErr == dds238_ERR_NO_ERROR) {                                            //if no timeout...

    if(sdmSer.available() >= FRAMESIZE) {

      for(int n=0; n<FRAMESIZE; n++) {
        sdmarr[n] = sdmSer.read();
      }

      if (sdmarr[0] == node && sdmarr[1] == dds238_B_02 && sdmarr[2] == dds238_REPLY_BYTE_COUNT) {

      if ((calculateCRC(sdmarr, FRAMESIZE - 2)) == ((sdmarr[(FRAMESIZE-1)] << 8) | sdmarr[(FRAMESIZE-2)])) {  //calculate crc and compare with received crc
          

        if(type == 2) {
	       	 int32_t sinput = 0;
            ((uint8_t*)&sinput)[1]= sdmarr[3];
            ((uint8_t*)&sinput)[0]= sdmarr[4];
            res = sinput;
		     } else {
            int32_t sinput = 1;
            ((uint8_t*)&sinput)[3]= sdmarr[3];
            ((uint8_t*)&sinput)[2]= sdmarr[4];
            ((uint8_t*)&sinput)[1]= sdmarr[5];
            ((uint8_t*)&sinput)[0]= sdmarr[6];
            res = sinput;
			}	
          
        } else {
          readErr = dds238_ERR_CRC_ERROR;                                          //err debug (1)
        }

      } else {
        readErr = dds238_ERR_WRONG_BYTES;                                          //err debug (2)
      }

    } else {
      readErr = dds238_ERR_NOT_ENOUGHT_BYTES;                                      //err debug (3)
    }

  }

  if (readErr != dds238_ERR_NO_ERROR) {                                            //if error then copy temp error value to global val and increment global error counter
    readingerrcode = readErr;
    readingerrcount++; 
  } else {
    ++readingsuccesscount;
  }

  while (sdmSer.available() > 0)  {                                             //read redundant serial bytes, if any
    sdmSer.read();
  }

#ifndef USE_HARDWARESERIAL
  sdmSer.end();                                                                 //disable softserial rx interrupt
#endif

  return (res);
}

uint16_t dds238::getErrCode(bool _clear) {
  uint16_t _tmp = readingerrcode;
  if (_clear == true)
    clearErrCode();
  return (_tmp);
}

uint16_t dds238::getErrCount(bool _clear) {
  uint16_t _tmp = readingerrcount;
  if (_clear == true)
    clearErrCount();
  return (_tmp);
}

uint16_t dds238::getSuccCount(bool _clear) {
  uint16_t _tmp = readingsuccesscount;
  if (_clear == true)
    clearSuccCount();
  return (_tmp);
}

void dds238::clearErrCode() {
  readingerrcode = dds238_ERR_NO_ERROR;
}

void dds238::clearErrCount() {
  readingerrcount = 0;
}

void dds238::clearSuccCount() {
  readingsuccesscount = 0;
}

uint16_t dds238::calculateCRC(uint8_t *array, uint8_t num) {
  uint16_t _crc, _flag;
  _crc = 0xFFFF;
  for (uint8_t i = 0; i < num; i++) {
    _crc = _crc ^ array[i];
    for (uint8_t j = 8; j; j--) {
      _flag = _crc & 0x0001;
      _crc >>= 1;
      if (_flag)
        _crc ^= 0xA001;
    }
  }
  return _crc;
}
