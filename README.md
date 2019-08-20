## Library for reading DDS238-4 W; DDS238 4 ZN/S; DDS238 2 ZN/S; DDS238 1 ZN Modbus Energy meters. ##

### SECTIONS: ###
#### 1. [INTRODUCTION](#introduction) ####
#### 2. [SCREENSHOTS](#screenshots) ####
#### 3. [CONFIGURING](#configuring) ####
#### 4. [INITIALIZING](#initializing) ####
#### 5. [READING](#reading) ####
#### 6. [PROBLEMS](#problems) ####
#### 7. [CREDITS](#credits) ####

---

### Introduction: ###
This library allows you reading dds238 module(s) using:
- [x] Hardware Serial (<i><b>recommended option</b>, smallest number of reads errors, especially for esp8266</i>) <b><i>or</i></b>
- [x] Software Serial (<i>[library for ESP8266](https://github.com/plerup/espsoftwareserial)</i>), attached as libraries for esp8266 and avr

you also need rs232<->rs485 converter:
- [x] with automatic flow direction control (<i>look at images below</i>) <b><i>or</i></b>
- [x] with additional pins for flow control, like MAX485</br>
     (<i>in this case MAX485 DE and RE pins must be connected together to one of uC pin</br>
     and this pin must be passed when initializing the library</i>)

_tested on DDS238-4 Wemos d1 mini->ESP8266 with Arduino 1.8.9-beta & 2.3.0 esp8266 core_

---

### Screenshots: ###


<img src="https://github.com/E-NINA/dds238_Energy_Meter/blob/master/img/Picture1.jpg" height="330"></br>
<p align="center">
  <img src="https://github.com/reaper7/SDM_Energy_Meter/blob/master/img/livepage.gif"></br>
  <i>live page example (extended) screenshot</i>
</p>

---

### Configuring: ###
Default configuration is specified in the [dds238.h](https://github.com/E-NINA/dds238_Energy_Meter/blob/master/dd238.h#L18) file, and parameters are set to:</br>
<i>Software Serial, baud 9600, uart config SERIAL_8N1, without DE/RE pin</i>.</br>

User can set the parameters in two ways:
- by editing the [dds238_Config_User.h](https://github.com/E-NINA/dds238_Energy_Meter/blob/master/dd238_Config_User.h) file
- by passing values during initialization (section below)

NOTE for Hardware Serial mode: <i>to force the Hardware Serial mode,</br>
user must edit the corresponding entry in [dds238_Config_User.h](https://github.com/E-NINA/dds238_Energy_Meter/blob/master/dd238_Config_User.h#L17) file.</br>
adding #define USE_HARDWARESERIAL to the main ino file is not enough.</i>

---

### Initializing: ###
If the user configuration is specified in the [dds238_Config_User.h](https://github.com/E-NINA/dds238_Energy_Meter/blob/master/dd238_Config_User.h) file</br>
or if the default configuration from the [dds238.h](https://github.com/E-NINA/dds238_Energy_Meter/blob/master/dd238.h#L18) file is suitable</br>
initialization is limited to passing serial port reference (software or hardware)</br>
and looks as follows:
```cpp
//lib init when Software Serial is used:
#include <SoftwareSerial.h>
#include <dds238.h>

SoftwareSerial swSerdds238(13, 15);

//              _software serial reference
//             |
dds238 dds238(swSerdds238);
```

```cpp
//lib init when Hardware Serial is used:
#include <dds238.h>

//            _hardware serial reference
//           |
dds238 dds238(Serial);
```
If the user wants to temporarily change the configuration during the initialization process</br>
then can pass additional parameters as below:
```cpp
//lib init when Software Serial is used:
#include <SoftwareSerial.h>
#include <dds238.h>

SoftwareSerial swSerdds238(13, 15);

//              __________________software serial reference
//             |      ____________baudrate(optional, default from dds238_Config_User.h)   
//             |     |           _dere pin for max485(optional, default from dds238_Config_User.h)
//             |     |          |
dds238 dds238(swSerdds238, 9600, NOT_A_PIN);
```

```cpp
//lib init when Hardware Serial is used:
#include <dds238.h>

// for ESP8266
//            _____________________________________hardware serial reference
//           |      _______________________________baudrate(optional, default from dds238_Config_User.h)
//           |     |           ____________________dere pin for max485(optional, default from dds238_Config_User.h)
//           |     |          |            ________hardware uart config(optional, default from dds238_Config_User.h)
//           |     |          |           |       _swap hw serial pins from 3/1 to 13/15(optional, default from dds238_Config_User.h)
//           |     |          |           |      |
dds238 dds238(Serial, 9600, NOT_A_PIN, SERIAL_8N1, false);


// for ESP32
//            ____________________________________________hardware serial reference
//           |      ______________________________________baudrate(optional, default from dds238_Config_User.h)
//           |     |           ___________________________dere pin for max485(optional, default from dds238_Config_User.h)
//           |     |          |            _______________hardware uart config(optional, default from dds238_Config_User.h)
//           |     |          |           |       ________rx pin number(optional)
//           |     |          |           |      |       _tx pin number(optional)
//           |     |          |           |      |      | 
dds238 dds238(Serial, 9600, NOT_A_PIN, SERIAL_8N1, rxpin, txpin);


// for AVR
//            _____________________________________hardware serial reference
//           |      _______________________________baudrate(optional, default from dds238_Config_User.h)
//           |     |           ____________________dere pin for max485(optional, default from dds238_Config_User.h)
//           |     |          |            ________hardware uart config(optional, default from dds238_Config_User.h)
//           |     |          |           |
//           |     |          |           |
dds238 dds238(Serial, 9600, NOT_A_PIN, SERIAL_8N1);
```
NOTE for ESP8266: <i>when GPIO15 is used (especially for swapped hardware serial):</br>
some converters (like mine) have built-in pullup resistors on TX/RX lines from rs232 side,</br>
connection this type of converters to ESP8266 pin GPIO15 block booting process.</br>
In this case you can replace the pull-up resistor on converter with higher value (100k),</br>
to ensure low level on GPIO15 by built-in in most ESP8266 modules pulldown resistor.</br></i>

---

### Reading: ###
List of available registers for DDS238-4 W; DDS238 4 ZN/S; DDS238 2 ZN/S; DDS238 1 ZN:</br>
https://github.com/E-NINA/dds238_Energy_Meter/blob/master/dd238.h#L52
```cpp
//reading voltage from dds238 with slave address 0x01 (default)
//                                      __________register name
//                                     |
float voltage = dds238.readVal(dds238_VOLTAGE);

//reading power from 1st dds238 with slave address ID = 0x01
//reading power from 2nd dds238 with slave address ID = 0x02
//useful with several meters on RS485 line
//                                      __________register name
//                                     |      ____dds238 device ID  
//                                     |     |
float power1 = dds238.readVal(dds238_POWER, 0x01);
float power2 = dds238.readVal(dds238_POWER, 0x02);
```
NOTE: <i>if you reading multiple dds238 devices on the same RS485 line,</br>
remember to set the same transmission parameters on each device,</br>
only ID must be different for each dds238 device.</i>

---

### Problems: ###
Sometimes <b>readVal</b> return <b>NaN</b> value (not a number),</br>
this means that the requested value could not be read from the dds238 module for various reasons.</br>

__Please check out open and close issues, maybe the cause of your error is explained or solved there.__

The most common problems are:
- weak or poorly filtered power supply / LDO, causing NaN readings and ESP crashes</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/13#issuecomment-353532711</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/13#issuecomment-353572909</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/8#issuecomment-381402008</br>
- faulty or incorrectly prepared converter</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/16#issue-311042308</br>
- faulty esp module</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/8#issuecomment-381398551</br>
- many users report that between each readings should be placed <i>delay(50);</i></br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/7#issuecomment-272080139</br>
  (I did not observe such problems using the HardwareSerial connection)</br>
- using GPIO15 without checking signal level (note above)</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/17#issue-313606825</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/13#issuecomment-353413146</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/13#issuecomment-353417658</br>
- compilation error for hardware serial mode</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/23</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/24</br>

You can get last error code using function:
```cpp
//get last error code
//                                      __________optional parameter,
//                                     |          true -> read and reset error code
//                                     |          false or no parameter -> read error code
//                                     |          but not reset stored code (for future checking)
//                                     |          will be overwriten when next error occurs
uint16_t lasterror = dds238.getErrCode(true);

//clear error code also available with:
dds238.clearErrCode();
```
Errors list returned by <b>getErrCode</b>:</br>
https://github.com/E-NINA/dds238_Energy_Meter/blob/master/dd238.h#L161</br>

You can also check total number of errors using function:
```cpp
//get total errors counter
//                                       _________optional parameter,
//                                      |         true -> read and reset errors counter
//                                      |         false or no parameter -> read errors counter
//                                      |         but not reset stored counter (for future checking)
uint16_t cnterrors = dds238.getErrCount(true);

//clear errors counter also available with:
dds238.clearErrCount();
```

And finally you can read the counter of correctly made readings:
```cpp
//get total success counter
//                                         _________optional parameter,
//                                        |         true -> read and reset success counter
//                                        |         false or no parameter -> read success counter
//                                        |         but not reset stored counter (for future checking)
uint16_t cntsuccess = dds238.getSuccCount(true);

//clear success counter also available with:
dds238.clearSuccCount();
```

---

### Credits: ###
:+1: SDM_Energy_Meter library by Reaper7 (https://github.com/reaper7/SDM_Energy_Meter)</br>
:+1: ESP SoftwareSerial library by Peter Lerup (https://github.com/plerup/espsoftwareserial)</br>
:+1: crc calculation by Jaime García (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino)</br>

---

