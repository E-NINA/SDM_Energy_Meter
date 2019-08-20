    
/* Library for reading DDS238-4 W; DDS238 4 ZN/S; DDS238 2 ZN/S; DDS238 1 ZN Modbus Energy meters.
*  Reading via Hardware or Software Serial library & rs232<->rs485 converter
*  2019 ENina (tested on DDS238-4 Wemos d1 mini->ESP8266 with Arduino 1.8.9-beta & 2.3.0 esp8266 core)
*  Based on SDM_Energy_Meter 2016-2019 Reaper7
*  crc calculation by Jaime García (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino/)
*/

/*
*  USER CONFIG:
*/

//------------------------------------------------------------------------------

/*
*  define or undefine USE_HARDWARESERIAL (uncomment only one or none)
*/
//#undef USE_HARDWARESERIAL
//#define USE_HARDWARESERIAL

//------------------------------------------------------------------------------

/*
*  define user baudrate
*/
//#define dds238_UART_BAUD                       9600                                

//------------------------------------------------------------------------------

/*
*  define user DERE_PIN for control MAX485 DE/RE lines (connect DE & /RE together to this pin)
*/
//#define DERE_PIN                            NOT_A_PIN                           

//------------------------------------------------------------------------------

#ifdef USE_HARDWARESERIAL

  /*
  *  define user dds238_UART_CONFIG
  */
  //#define dds238_UART_CONFIG                   SERIAL_8N1                          

  //----------------------------------------------------------------------------

  /*
  *  define user SWAPHWSERIAL, if true(1) then swap uart pins from 3/1 to 13/15 (only ESP8266)
  */
  //#define SWAPHWSERIAL                      0                                   

#endif

//------------------------------------------------------------------------------

/*
*  define user MAX_MILLIS_TO_WAIT to wait for response from dds238
*/
//#define MAX_MILLIS_TO_WAIT                  500                                 

//------------------------------------------------------------------------------
