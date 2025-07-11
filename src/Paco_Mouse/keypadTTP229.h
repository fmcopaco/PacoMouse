/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/

#ifndef KEYPAD_T229_H
#define KEYPAD_T229_H

/******************************************************************************\
  | This library designed to communicate with the TTP229 chip from an Arduino  |
  | Defined only needed methods for PacoMouse to mimic Keypad.h & Keypad_I2C.h |
  |                                                                            |
  | Works with a TTP229-BSF (16-Channel Digital Touch Capacitive Switch Sensor)|
  | using the 2-wires serial interface protocol - only 2 arduino pins.         |
  | General:                                                                   |
  | * TTP229 VCC to pin VCC                                                    |
  | * TTP229 GND to pin GND                                                    |
  | * TTP229 SCL to any pin you choose...                                      |
  | * TTP229 SDO to any pin you choose...                                      |
  |                                                                            |
  | 16 Buttons Mode (else 8 Buttons Mode):   -----> Using this mode            |
  | * TTP229 TP2 to GND via 1 Megohm resistor! ---> Solder wire in TP2         |
  |                                                                            |
  | Multi Keys Mode (else Single key Mode):                                    |
  | * TTP229 TP3 to GND via 1 Megohm resistor!                                 |
  | * TTP229 TP4 to GND via 1 Megohm resistor!                                 |
  |                                                                            |
  | 64Hz Sampling Rate (else 8Hz Sampling Rate):                               |
  | * TTP229 TP5 to GND via 1 Megohm resistor!                                 |
  |                                                                            |
  | 2ms Wake Up Rate (else 4ms Wake Up Rate):                                  |
  | * TTP229 TP6 to GND via 1 Megohm resistor!                                 |
  |                                                                            |
  | 60sec Key On Timeout (else No Key On Timeout):                             |
  | * TTP229 TP7 to GND via 1 Megohm resistor!                                 |
  |                                                                            |
  | Important:                                                                 |
  | * Must reconnect the TTP229 power so the mode changes will take effect     |
  | * The 1 Megohm resistors already exist on some TTP229 modules              |
  \******************************************************************************/

#include <Arduino.h>

typedef enum { IDLE, PRESSED, HOLD, RELEASED } KeyState;

const char NO_KEY = '\0';

//#define makeKeymap(x) ((char*)x)

class Keypad_TTP229 {
  public:
    // constructor
    Keypad_TTP229(char *userKeymap, byte pinSCL, byte pinSDO);
    
    // methods
    char getKey();
    KeyState getState();
    void begin();
    void setDebounceTime(unsigned int debounce);

  private:
    byte _pinSCL;
    byte _pinSDO;
    char *keymap;
    unsigned int debounceTime;
    unsigned long startTime;
    byte rawKey;
    byte oldRawKey;

    byte readTTP229();
};


#endif
