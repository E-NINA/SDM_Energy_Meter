/* Library for reading DDS238-4 W; DDS238 4 ZN/S; DDS238 2 ZN/S; DDS238 1 ZN Modbus Energy meters.
*  Reading via Hardware or Software Serial library & rs232<->rs485 converter
*  2019 ENina (tested on DDS238-4 Wemos d1 mini->ESP8266 with Arduino 1.8.9-beta & 2.3.0 esp8266 core)
*  Based on SDM_Energy_Meter 2016-2019 Reaper7
*  crc calculation by Jaime García (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino/)
*/
//------------------------------------------------------------------------------
#ifndef dds238_h
#define dds238_h
//------------------------------------------------------------------------------
#include <Arduino.h>
#include <dds238_Config_User.h>
#ifdef USE_HARDWARESERIAL
  #include <HardwareSerial.h>
#else
  #include <SoftwareSerial.h>
#endif
//------------------------------------------------------------------------------
//DEFAULT CONFIG (DO NOT CHANGE ANYTHING!!! for changes use dds238_Config_User.h):
//------------------------------------------------------------------------------
#ifndef dds238_UART_BAUD
  #define dds238_UART_BAUD                     9600                                //default baudrate on dds238-4 W
#endif

#ifndef DERE_PIN
  #define DERE_PIN                          NOT_A_PIN                           //default digital pin for control MAX485 DE/RE lines (connect DE & /RE together to this pin)
#endif

#ifdef USE_HARDWARESERIAL

  #ifndef dds238_UART_CONFIG
    #define dds238_UART_CONFIG                 SERIAL_8N1                          //default hardware uart config
  #endif

  #ifdef ESP8266
    #ifndef SWAPHWSERIAL
      #define SWAPHWSERIAL                    0                                 //(only esp8266) when hwserial used, then swap uart pins from 3/1 to 13/15 (default not swap)
    #endif
  #endif

#endif

#ifndef MAX_MILLIS_TO_WAIT
  #define MAX_MILLIS_TO_WAIT                500                                 //default max time to wait for response from dds238
#endif
//------------------------------------------------------------------------------
#define FRAMESIZE                           9                                   //size of out/in array
#define dds238_REPLY_BYTE_COUNT                0x04                                //number of bytes with data

#define dds238_B_01                            0x01                                //BYTE 1 -> slave address (default value 1 read from node 1)
#define dds238_B_02                            0x03                                //BYTE 2 -> function code (default value 0x03 read from 4X input registers)
                                                                                //BYTES 3 & 4 (BELOW)

//dds238 x registers
#define dds238_VOLTAGE                     0x000C                              //V
#define dds238_CURRENT                     0x000D                              //A
#define dds238_POWER                       0x000E                              //W
#define dds238_REACTIVE_APPARENT_POWER     0x000F                              //VAR
#define dds238_POWER_FACTOR                0x0010                              //
#define dds238_FREQUENCY                   0x0011                              //Hz
#define dds238_IMPORT_ACTIVE_ENERGY        0x000A                              //Wh   (0x02)
#define dds238_EXPORT_ACTIVE_ENERGY        0x0008                              //Wh   (0x02)
#define dds238_TOTAL_ACTIVE_ENERGY         0x0000                              //Wh   (0x02)



#define dds238_B_05                            0x00                                //BYTE 5
#define dds238_B_06                            0x02                                //BYTE 6
//------------------------------------------------------------------------------
#define dds238_ERR_NO_ERROR                    0                                   //no error
#define dds238_ERR_CRC_ERROR                   1                                   //crc error
#define dds238_ERR_WRONG_BYTES                 2                                   //bytes b0,b1 or b2 wrong
#define dds238_ERR_NOT_ENOUGHT_BYTES           3                                   //not enough bytes from dds238
#define dds238_ERR_TIMEOUT                     4                                   //timeout
//------------------------------------------------------------------------------
class dds238 {
  public:
#ifdef USE_HARDWARESERIAL
  #if defined ( ESP8266 )
    dds238(HardwareSerial& serial, long baud = dds238_UART_BAUD, int dere_pin = DERE_PIN, int config = dds238_UART_CONFIG, bool swapuart = SWAPHWSERIAL);
  #elif defined ( ESP32 )
    dds238(HardwareSerial& serial, long baud = dds238_UART_BAUD, int dere_pin = DERE_PIN, int config = dds238_UART_CONFIG, int8_t rx_pin=-1, int8_t tx_pin=-1);
  #else
    dds238(HardwareSerial& serial, long baud = dds238_UART_BAUD, int dere_pin = DERE_PIN, int config = dds238_UART_CONFIG);
  #endif
#else
    dds238(SoftwareSerial& serial, long baud = dds238_UART_BAUD, int dere_pin = DERE_PIN);
#endif
    virtual ~dds238();

    void begin(void);
    float readVal(uint16_t reg, byte type = 0, uint8_t node = dds238_B_01);     //read value from register = reg and from deviceId = node
    uint16_t getErrCode(bool _clear = false);                                   //return last errorcode (optional clear this value, default flase)
    uint16_t getErrCount(bool _clear = false);                                  //return total errors count (optional clear this value, default flase)
    uint16_t getSuccCount(bool _clear = false);                                 //return total success count (optional clear this value, default false)
    void clearErrCode();                                                        //clear last errorcode
    void clearErrCount();                                                       //clear total errors count
    void clearSuccCount();                                                      //clear total success count

  private:
#ifdef USE_HARDWARESERIAL
    HardwareSerial& sdmSer;
#else
    SoftwareSerial& sdmSer;
#endif

#ifdef USE_HARDWARESERIAL
    int _config = dds238_UART_CONFIG;
  #if defined ( ESP8266 )
    bool _swapuart = SWAPHWSERIAL;
  #elif defined ( ESP32 )
    int8_t _rx_pin=-1;
    int8_t _tx_pin=-1;
  #endif
#endif
    long _baud = dds238_UART_BAUD;
    int _dere_pin = DERE_PIN;
    uint16_t readingerrcode = dds238_ERR_NO_ERROR;                              //4 = timeout; 3 = not enough bytes; 2 = number of bytes OK but bytes b0,b1 or b2 wrong, 1 = crc error
    uint16_t readingerrcount = 0;                                               //total errors counter
    uint32_t readingsuccesscount = 0;                                           //total success counter
    uint16_t calculateCRC(uint8_t *array, uint8_t num);
};
#endif //dds238_h
