/*    Paco Mouse -- F. Cañada 2022-2024 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.

      Defined only needed methods for PacoMouse to mimic Keypad.h & Keypad_I2C.h
*/

#include "keypadTTP229.h"

Keypad_TTP229::Keypad_TTP229(char *userKeymap, byte pinSCL, byte pinSDO) {
  _pinSCL = pinSCL;
  _pinSDO = pinSDO;
  pinMode (_pinSDO, INPUT);
  pinMode (_pinSCL, OUTPUT);
  digitalWrite (_pinSCL, HIGH);
  keymap = userKeymap;
  rawKey = 0;
  oldRawKey = 0;
  startTime = millis();
  setDebounceTime(10);
}


void Keypad_TTP229::begin() {   // defined only for compatibilty with Keypad_I2C
}


void Keypad_TTP229::setDebounceTime(unsigned int debounce) {
  debounceTime = (debounce < 1) ? 1 : debounce;
}


KeyState Keypad_TTP229::getState() {
  if (rawKey > 0)
    return PRESSED;
  else
    return RELEASED;
}


char Keypad_TTP229::getKey() {
  byte state;
  state = NO_KEY;
  if ( (millis() - startTime) > debounceTime ) {
    startTime = millis();
    rawKey = readTTP229();
    if (rawKey > 0) {
      if (rawKey != oldRawKey)
        state = keymap[rawKey - 1];
    }
    oldRawKey = rawKey;
  }
  return state;
}


byte Keypad_TTP229::readTTP229() {
  byte n, data;

  data = 0;
  for (n = 1; n <= 16; n++) {
    digitalWrite(_pinSCL, LOW);
    if (!digitalRead(_pinSDO))
      data = n;
    digitalWrite(_pinSCL, HIGH);
  }
  return data;
}
