/*    Paco Mouse -- F. Cañada 2022-2024 -- https://usuaris.tinet.cat/fmco/

      Test de hardware para placa PacoMouse con Arduino Nano

*/

#define KEYPAD        0                                     // Tipo de teclado, normal o tactil
#define TOUCHPAD      1


////////////////////////////////////////////////////////////
// ***** USER OPTIONS *****
////////////////////////////////////////////////////////////

#define SERIAL_SPEED 115200

// Seleccione el tipo de teclado - Select the type of keypad: KEYPAD/TOUCHPAD
#define KEYPAD_TYPE               KEYPAD

// I2C address of the PCF8574 for keypad (0x20..0x27)
#define I2C_ADDRESS_KEYPAD        0x20                        // PCF8574 keypad

// Descomentar la siguiente linea si se usa el touchpad TTP229 por detras - Uncomment next line if using rear side of touchpad TTP229
//#define REAR_TOUCH                1

////////////////////////////////////////////////////////////
// ***** END OF USER OPTIONS *****
////////////////////////////////////////////////////////////


#if defined(__AVR__)
#include "SSD1306Ascii.h"                                   // https://github.com/greiman/SSD1306Ascii      v1.3.0
#include <SSD1306AsciiAvrI2c.h>
#endif
#if defined(ESP8266)
#include <ESP8266WiFi.h>                                    // Arduino ESP8266 core                         v3.0.2
#include <Wire.h>
#include "SSD1306AsciiWire.h"
#if  (KEYPAD_TYPE == KEYPAD)
#include <Keypad_I2C.h>                                     // https://github.com/joeyoung/arduino_keypads  v3.0.0
#endif
#endif
#if  (KEYPAD_TYPE == KEYPAD)
#include <Keypad.h>                                         // https://github.com/Chris--A/Keypad           v3.1.1
#endif
#if  (KEYPAD_TYPE == TOUCHPAD)
#include "keypadTTP229.h"
#endif
#include <EEPROM.h>

#if defined(__AVR__)
const int pinOutA       = 2;                                  // encoder pins. NO CAMBIAR, D2 es un pin de interrupcion
const int pinOutB       = 12;
const int pinSwitch     = A2;

const int pinFILA1      = 3;                                  // Keypad 3x4 or 4x4 pins. Teclado membrana
const int pinFILA2      = 4;
const int pinFILA3      = 5;
const int pinFILA4      = 6;
const int pinCOLUMNA1   = 9;
const int pinCOLUMNA2   = 10;
const int pinCOLUMNA3   = 11;
const int pinCOLUMNA4   = A0;
/*
  const int pinFILA1      = 9;                                  // Keypad 3x4 or 4x4 pins. Teclado mecanico
  const int pinFILA2      = 10;
  const int pinFILA3      = 11;
  const int pinFILA4      = A0;
  const int pinCOLUMNA1   = 3;
  const int pinCOLUMNA2   = 4;
  const int pinCOLUMNA3   = 5;
  const int pinCOLUMNA4   = 6;
*/
const int pinSDA        = A4;                                 // I2C pins: OLED
const int pinSCL        = A5;

const int pinLN_RXD     = 8;                                  // Loconet pins. NO CAMBIAR, pin LN_RXD debe ser pin ICP del Arduino
const int pinLN_TXD     = 7;

const int pinTXRX       = A3;                                 // Xpressnet pins

const int pinTOUCH_SCL  = 10;                                 // TTP229 keypad pins
const int pinTOUCH_SDO  = 9;
const int pinTOUCH_VCC  = A0;                                 // only for PacoMouse board
const int pinTOUCH_GND  = 11;

const int pins[] = {pinOutA, pinOutB, pinSwitch, pinFILA1, pinFILA2, pinFILA3, pinFILA4, pinCOLUMNA1, pinCOLUMNA2, pinCOLUMNA3, pinCOLUMNA4, pinSDA, pinSCL, pinLN_RXD, pinLN_TXD, pinTXRX};

SSD1306AsciiAvrI2c oled;                                    // Pantalla principal
AvrI2c i2c;
#endif
#if defined(ESP8266)
const int pinOutA       = D5;                                 // encoder pins.  Este es un pin de interrupcion, NO USAR GPIO16 (D0)
const int pinOutB       = D6;
const int pinSwitch     = D7;

const int pinFILA1      = 7;                                  // Keypad 3x4 or 4x4 (pins PCF8574)
const int pinFILA2      = 6;
const int pinFILA3      = 5;
const int pinFILA4      = 4;
const int pinCOLUMNA1   = 3;
const int pinCOLUMNA2   = 2;
const int pinCOLUMNA3   = 1;
const int pinCOLUMNA4   = 0;

const int pinSDA        = D2;                                 // I2C pins: OLED. NO CAMBIAR
const int pinSCL        = D1;

const int pinTOUCH_SCL  = D3;                                 // TTP229 keypad pins
const int pinTOUCH_SDO  = D4;

const int pins[] = {pinOutA, pinOutB, pinSwitch, pinSDA, pinSCL};

SSD1306AsciiWire oled;                                      // Pantalla principal

#endif

byte i2cAdr;
bool isOLED;
bool isSSD1306;
bool isSH1106;
bool isSSD1309;
byte adrPCF8574;
bool isKeypad;
bool poweredTouch;

// Initialization commands for a 128x64 SSD1309 oled display.
static const uint8_t MEM_TYPE SSD1309_128x64init[] = {
  // Init sequence for SSD1309 128x64 OLED module based on Arduboy2 library initialization & uploader.py lcdBootProgram = b"\xD5\xF0\x8D\x14\xA1\xC8\x81\xCF\xD9\xF1\xAF\x20\x00"
  //                                                                                                 patched for SSD1309= b"\xD5\xF0\xE3\xE3\xA1\xC8\x81\cnt\xD9\xF1\xAF\x20\x00"
  //    SSD1306_DISPLAYOFF,
  SSD1306_SETDISPLAYCLOCKDIV, 0xF0,   // Set Display Clock Divisor v = 0xF0, default is 0x80
  //    SSD1306_SETMULTIPLEX, 0x3F,        // ratio 64
  //    SSD1306_SETDISPLAYOFFSET, 0x00,    // no offset
  //    SSD1306_SETSTARTLINE,              // line #0
  0xE3, 0xE3,                         //Charge Pump command not supported, use two NOPs instead to keep same size and easy patchability
  SSD1306_SEGREMAP | 0x1,             // column 127 mapped to SEG0
  SSD1306_COMSCANDEC,                 // column scan direction reversed
  //    SSD1306_SETCOMPINS, 0x12,          // alt COM pins, disable remap
  SSD1306_SETCONTRAST, 0x32,          // contrast level (0xCF) 207 by default. Using 0x32 to reduce consumption
  SSD1306_SETPRECHARGE, 0xF1,
  //    SSD1306_SETVCOMDETECT, 0x40,       // vcomh regulator level
  //    SSD1306_DISPLAYALLON_RESUME,
  //    SSD1306_NORMALDISPLAY,
  SSD1306_DISPLAYON,
  SSD1306_MEMORYMODE, 0x00,
  //    0x21, 0x00, 0x7F,
  //    0x22, 0x00, 0x07
};

// Initialize a 128x64 oled display. //
static const DevType MEM_TYPE SSD1309_128x64 = {
  SSD1309_128x64init,
  sizeof(SSD1309_128x64init),
  128,
  64,
  0
};


enum teclas {K_NONE, K_NUM0, K_NUM1, K_NUM2, K_NUM3, K_NUM4, K_NUM5, K_NUM6, K_NUM7, K_NUM8, K_NUM9, K_MENU, K_ENTER, K_A, K_B, K_C, K_D};

#define FILAS 4                                             // Filas y Columnas de la matriz del teclado

#if  (KEYPAD_TYPE == TOUCHPAD)

#ifdef REAR_TOUCH
char keys[16] = {K_A, K_NUM3, K_NUM2, K_NUM1, K_B, K_NUM6, K_NUM5, K_NUM4, K_C, K_NUM9, K_NUM8, K_NUM7, K_D, K_ENTER, K_NUM0, K_MENU};
#else
char keys[16] = {K_NUM1, K_NUM2, K_NUM3, K_A, K_NUM4, K_NUM5, K_NUM6, K_B, K_NUM7, K_NUM8, K_NUM9, K_C, K_MENU, K_NUM0, K_ENTER, K_D};
#endif

Keypad_TTP229 keypad (keys, pinTOUCH_SCL, pinTOUCH_SDO);

#else

#define COLUMNAS 4

char keys[FILAS][COLUMNAS] = {                              // Disposicion de las teclas
  {K_NUM1, K_NUM2, K_NUM3, K_A},
  {K_NUM4, K_NUM5, K_NUM6, K_B},
  {K_NUM7, K_NUM8, K_NUM9, K_C},
  {K_MENU, K_NUM0, K_ENTER, K_D}
};

byte filaPins[FILAS]   = {pinFILA1, pinFILA2, pinFILA3, pinFILA4};                // pins de las filas del teclado
byte colPins[COLUMNAS] = {pinCOLUMNA1, pinCOLUMNA2, pinCOLUMNA3, pinCOLUMNA4};    // pins de las columnas del teclado

#if defined(__AVR__)
Keypad keypad = Keypad( makeKeymap(keys), filaPins, colPins, FILAS, COLUMNAS);
#endif
#if defined(ESP8266)
Keypad_I2C keypad ( makeKeymap(keys), filaPins, colPins, FILAS, COLUMNAS, I2C_ADDRESS_KEYPAD);
#endif
#endif

byte keyValue;                                              // valor tecla pulsada
bool keyOn;
byte numTeclas;
byte testedTeclas;
byte pressedTeclas[16];
char keyText[][6] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "MENU", "ENTER", "STOP", "LOCO", "-^-", "-v-"};
byte textPos[]    = {13,   0,   1,   2,   4,   5,   6,   8,   9,   10,   12,      14,     3,      7,      11,    15};


byte outA, outB, copyOutA, copyOutB;                        // encoder input used by ISR
volatile byte encoderValue;                                 // encoder shared values (ISR & program)
volatile byte encoderMax;
volatile bool encoderChange;

unsigned long timeout;
bool encoderOK;

byte statusSwitch;
bool switchOn;
const unsigned long timeoutButtons = 50;                    // temporizador antirebote
unsigned long timeButtons;

bool useLN, useXPN, useZ21, useECOS;
bool errorLN;

enum Settings {
  EE_ADRH, EE_ADRL, EE_IDH, EE_IDL, EE_CONTRAST, EE_STOP_MODE, EE_CMD_STA, EE_XBUS, EE_TT_TYPE, EE_SHUNTING, EE_ROCO, EE_LOCK, EE_SHORT,
  EE_MOD_A, EE_CONT_A, EE_MOD_B, EE_CONT_B, EE_TIME, EE_PHONE_ME, EE_PHONE_A, EE_PHONE_B, EE_PHONE_C, EE_PHONE_D,
  EE_WIFI,                                     //  datos WiFi. (Tiene que ser el ultimo)
}; // EEPROM settings

#if defined(ESP8266)
struct {
  char ssid[33];                                            // SSID
  char password[65];                                        // Password
  IPAddress CS_IP;                                          // IP
  unsigned int port;                                        // Port
  bool type;                                                // Server type
  int  ok;
} wifiSetting;

#define FAT_SIZE            111
#define FAT_INI             0x00A0
#define FREE_SECTOR         0xFF

#else

#define FAT_SIZE            58
#define FAT_INI             0x0020
#define FREE_SECTOR         0xFF

#endif



// ---------------------------------------------------------------------------------------------------


char getResponse(const char *valid) {
  byte n;
  char c = '\0';
  do {
    if (Serial.available()) {
      c = Serial.read();
      for (n = 0; valid[n] != '\0' && valid[n] != c; n++);
      c = valid[n];
    }
  } while (c == '\0');
  Serial.println(c);
  return (toupper(c));
}

byte i2cTest(byte address) {
  byte error;
#if defined(__AVR__)
  i2c.begin(true);
  error = i2c.start(address << 1) ? 0 : 1;
  i2c.stop();
#endif
#if defined(ESP8266)
  Wire.beginTransmission(address);
  error = Wire.endTransmission();
#endif
  return error;
}

bool buscaOLED() {
  printSeparator();
  Serial.println(F("Buscando OLED..."));
  i2cAdr = 0x3C;
  if (i2cTest(i2cAdr) == 0) {
    return true;
  }
  i2cAdr = 0x3D;
  if (i2cTest(i2cAdr) == 0) {
    return true;
  }
  return false;
}


void buscaI2C() {
  byte devices, adr, error;
  devices = 0;
  for (adr = 1; adr < 127; adr++) {
    error = i2cTest(adr);
    if (error == 0) {
      printHEX(adr);
      Serial.print(' ');
      devices++;
    }
  }
  if (devices == 0)
    Serial.println(F("No se han encontrado dispositivos I2C"));
  Serial.println();
}


bool buscaKeypad() {
  byte adr, error;
  printSeparator();
  Serial.println(F("Buscando PCF8574..."));
  adrPCF8574 = I2C_ADDRESS_KEYPAD;
  if (i2cTest(adrPCF8574) == 0) {
    return true;
  }
  else {
    for (adr = 0x20; adr < 0x28; adr++) {     // busca PCF8574 existente
      adrPCF8574 = adr;
      error = i2cTest(adr);
      if (error == 0)
        return (false);
    }
  }
  adrPCF8574 = 0;
  return false;
}


byte testKeypad() {
  byte n, x, k;
  for (n = 0; n < 16; n++)
    pressedTeclas[n] = 0;
  x = 0;
  while (x != numTeclas) {
#if defined(ESP8266)
    yield();
#endif
    k = keypad.getKey();
    if (k != NO_KEY) {
      x++;
      switch (k) {
        case K_NUM0:
        case K_NUM1:
        case K_NUM2:
        case K_NUM3:
        case K_NUM4:
        case K_NUM5:
        case K_NUM6:
        case K_NUM7:
        case K_NUM8:
        case K_NUM9:
          Serial.println(keyText[k - 1]);
          pressedTeclas[k - 1] = 1;
          break;
        case K_MENU:
          Serial.println(keyText[10]);
          pressedTeclas[10] = 1;
          break;
        case K_ENTER:
          Serial.println(keyText[11]);
          pressedTeclas[11] = 1;
          break;
        case K_A:
          Serial.println(keyText[12]);
          pressedTeclas[12] = 1;
          break;
        case K_B:
          Serial.println(keyText[13]);
          pressedTeclas[13] = 1;
          break;
        case K_C:
          Serial.println(keyText[14]);
          pressedTeclas[14] = 1;
          break;
        case K_D:
          Serial.println(keyText[15]);
          pressedTeclas[15] = 1;
          break;
      }
      if (isOLED)
        showKeyboard();
    }
  }
  x = 0;
  for (n = 0; n < 16; n++)
    if (pressedTeclas[n] != 0)
      x++;
  return x;
}


#if defined(__AVR__)
void encoderISR ()
#endif
#if defined(ESP8266)
//ICACHE_RAM_ATTR void encoderISR ()                          // ESP core version 2.x.x       // interrupción encoder
IRAM_ATTR void encoderISR ()                                // ESP core version 3.x.x
#endif
{ // interrupción encoder
  outA = digitalRead (pinOutA);
  outB = digitalRead (pinOutB);
  if (outA != copyOutA)  {                                 // evitamos rebotes
    copyOutA = outA;
    if (copyOutB == 0x80) {
      copyOutB = outB;
    }
    else {
      if ( outB != copyOutB) {
        copyOutB = 0x80;
        if (outA == outB)                                   // comprueba sentido de giro
          encoderValue = (encoderValue < encoderMax) ? ++encoderValue : encoderMax ;  // CW, hasta maximo
        else
          encoderValue = (encoderValue > 0) ? --encoderValue : 0;                     // CCW, hasta 0
        encoderChange = true;
      }
    }
  }
}

#if defined(__AVR__)
/*
  // Definir ISR para cada puerto
  ISR (PCINT0_vect) {
  encoderISR();                                             // gestionar para PCINT para D8 a D13
  }
*/
ISR (PCINT2_vect) {
  encoderISR();                                             // gestionar PCINT para D0 a D7
}
/*
  ISR (PCINT1_vect)
  {
                                                            // gestionar PCINT para A0 a A5
  }
*/


void pciSetup(byte pin)
{
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));   // activar pin en PCMSK
  PCIFR  |= bit (digitalPinToPCICRbit(pin));                    // limpiar flag de la interrupcion en PCIFR
  PCICR  |= bit (digitalPinToPCICRbit(pin));                    // activar interrupcion para el grupo en PCICR
}
#endif

void testEncoder() {
  byte n;
#if defined(ESP8266)
  yield();
#endif
  if (encoderChange) {
    encoderChange = false;
    if (isOLED) {
      oled.setCursor(0, 4);
      for (n = 0; n < encoderValue; n++)
        oled.ssd1306WriteRam(0xFF);
      for (n = encoderValue; n < 128; n++)
        oled.ssd1306WriteRam(0x00);
    }
    Serial.println(encoderValue);
  }
}


void testSwitch() {
  byte inputButton;
#if defined(ESP8266)
  yield();
#endif
  if (millis() - timeButtons > timeoutButtons)  {            // lectura de boton
    timeButtons = millis();                                   // lee cada cierto tiempo
    inputButton = digitalRead (pinSwitch);                    // comprueba cambio en boton del encoder
    if (statusSwitch != inputButton) {
      statusSwitch = inputButton;
      if (isOLED) {
        oled.setCursor(60, 0);
        if (statusSwitch == LOW) {
          Serial.println(F("Pulsado"));
          oled.print('0');
        }
        else {
          oled.print(F("  "));
        }
      }
      else  {
        if (statusSwitch == LOW)
          Serial.println(F("Pulsado"));
      }
    }
  }
}


void updateEEPROM (int adr, byte data) {
#if defined(__AVR__)
  EEPROM.update(adr, data);
#endif
#if defined(ESP8266)
  if (EEPROM.read(adr) != data) {
    EEPROM.write (adr, data);
    //EEPROM.commit();
  }
#endif
}


void formatDisk() {
  unsigned int adr, n;
  for (n = 0; n < FAT_SIZE; n++)
    updateEEPROM(FAT_INI + n, FREE_SECTOR);
}

// ---------------------------------------------------------------------------------------------------

void instrucciones() {
  Serial.println(F("\nSiga los siguientes pasos para comprobar el hardware de la placa PacoMouse"));
  Serial.println(F("Se comprobaran el OLED, el teclado y el encoder\n"));
#if defined(__AVR__)
  Serial.println(F("ATENCION: No conecte el Arduino al bus Loconet o al Xpressnet"));
  Serial.println(F("ATENCION: En caso de usar la version Xpressnet, no instale el MAX485\n"));
#endif
  Serial.println(F("Si es necesario repita este test añadiendo un elemento cada vez"));
  Serial.println(F("Responda a las preguntas desde el Monitor Serie del Arduino IDE\n"));
  Serial.println(F(">> Escriba Y cuando este listo\n"));
}


void printSeparator() {
  Serial.println(F("=========================================================================="));
}


void printHEX (byte adr) {
  Serial.print(F("0x"));
  if (adr < 16)
    Serial.print('0');
  Serial.print(adr, HEX);
}


void showTestOLED(bool ask) {
  byte i;
  oled.setFont(Arial_bold_14);
  oled.clear();
  for (i = 0; i < 127; i++)
    oled.ssd1306WriteRam(0x01);
  for (i = 0; i < 8; i++) {
    oled.setCursor(0, i);
    oled.ssd1306WriteRam(0xFF);
    oled.setCol(127);
    oled.ssd1306WriteRam(0xFF);
  }
  oled.setCursor(1, 7);
  for (i = 1; i < 127; i++)
    oled.ssd1306WriteRam(0x80);
  oled.setCursor (34, 2);
  oled.set2X();
  oled.print(F("Paco"));
  oled.set1X();
  oled.setCursor (42, 5);
  oled.print(F("Mouse"));
  if (ask)
    Serial.println(F("Ve correctamente  el recuadro y el texto en pantalla sin desplazamientos? (Y/N)"));
}

void printComment(bool prn) {
  if (prn)
    Serial.print(F("//"));
}

void printTypeOLED () {
  if (isSSD1306 || isSSD1309 || isSH1106) {
    Serial.print(F("#define OLED_CHIP                 "));
    if (isSSD1306)
      Serial.println(F("SSD1306      // OLED 0.96\""));
    if (isSH1106)
      Serial.println(F("SH1106       // OLED 1.3\""));
    if (isSSD1309) {
      Serial.println(F("SSD1309      // OLED 1.54\" & 2.42\""));
      Serial.println(F("\nATENCION: Su pantalla consume mucha corriente, tengalo en cuenta!!\n"));
    }
  }
  else {
    Serial.println(F("ERROR: Compruebe las conexiones de su pantalla. Si son correctas su OLED no esta soportada"));
  }
}


void printResumen() {
  if (isOLED)
    showTestOLED(false);
  printSeparator();
  Serial.println(F("\nFIN DEL TEST"));
  Serial.print(F("\n\nEn el archivo config.h de PacoMouse use: \n\n"));
  if (!encoderOK)
    Serial.println(F("\n\nERROR: Compruebe conexiones del encoder!!. Si cambian los valores al reves intercambie los cables S1 y S2\n"));
  switch (numTeclas) {
    case 0:
#if (KEYPAD_TYPE == TOUCHPAD)
      Serial.println(F("\nERROR: Compruebe las conexiones del TTP229 y la linea '#define REAR_TOUCH' de este programa!!"));
#else
      Serial.println(F("ERROR: Compruebe las conexiones de su teclado!!"));
#endif
      break;
    case 12:
      printComment(true);
    case 16:
      Serial.println(F("#define USE_KEYPAD_4x4            1"));
      break;
  }
#if (KEYPAD_TYPE == TOUCHPAD)
#if defined(__AVR__)
  if (!poweredTouch)
    printComment(true);
  Serial.println(F("#define POWER_TOUCH            1"));
#endif
#endif

  Serial.println();
  if (isOLED) {
    Serial.print(F("#define I2C_ADDRESS_OLED          "));
    printHEX(i2cAdr);
    Serial.println('\n');
    printTypeOLED();
  }
  else {
    Serial.println(F("ERROR: Compruebe las conexiones de su pantalla OLED"));
  }
  Serial.println();
  Serial.print(F("#define HW_VERSION                "));
  if (useXPN)
    Serial.println(F("XNET"));
  if (useLN)
    Serial.println(F("LNET"));
  if (useZ21)
    Serial.println(F("Z21"));
  if (useECOS)
    Serial.println(F("ECOS"));
  if (errorLN)
    Serial.println(F("\nERROR: Compruebe las conexiones del interface Loconet de PacoMouse!!. No responde adecuadamente!!"));

  Serial.println();
  Serial.print(F("#define KEYPAD_TYPE               "));
#if (KEYPAD_TYPE == TOUCHPAD)
  Serial.println(F("TOUCHPAD\n"));
#ifndef REAR_TOUCH
  printComment(true);
#endif
  Serial.println(F("#define REAR_TOUCH                1"));
#else
  Serial.println(F("KEYPAD"));
#endif


#if defined(ESP8266)
#if (KEYPAD_TYPE == KEYPAD)
  if (isKeypad) {
    Serial.print(F("\n#define I2C_ADDRESS_KEYPAD         "));
    printHEX(adrPCF8574);
    Serial.println();
  }
  else {
    Serial.println(F("\nERROR: Compruebe las conexiones del PCF8574 y la linea '#define I2C_ADDRESS_KEYPAD' de este programa!!"));
  }
#endif
#endif

  if (useECOS) {
    Serial.println(F("\nDefina para el stack un numero mayor que las locomotoras que tiene definidas en la central. p.ej 80"));
    Serial.println(F("\n#define LOCOS_IN_STACK            80"));
  }
  else {
    Serial.println(F("\nDefina para el stack un numero comodo para seleccionar de la lista. p.ej 24"));
    Serial.println(F("\n#define LOCOS_IN_STACK            24"));
  }
}


void showKeyboard() {
  byte n, i;
  oled.clear();
  for (n = 0; n < 16; n++) {
    if (pressedTeclas[n] != 0) {
      i = textPos[n];
      oled.setCursor((i & 0x03) << 5, (i >> 1) & 0xFE);
      if (n > 9) {
        oled.print(keyText[n][0]);
        oled.print(keyText[n][1]);
        oled.print(keyText[n][2]);
      }
      else
        oled.print(keyText[n]);
    }
  }
}



// ---------------------------------------------------------------------------------------------------

void setup() {
  byte n, i;

  n = sizeof(pins) / sizeof(pins[0]);
  for (i = 0; i < n; i++)
    pinMode(pins[i], INPUT);
  Serial.begin(SERIAL_SPEED);
  Serial.print(F("\n\nPacoMouse Hardware Test "));
#if defined(__AVR__)
  Serial.println(F("con Arduino Nano"));
#endif
#if defined(ESP8266)
  Serial.println(F("con Wemos D1 Mini"));
#endif
  instrucciones();
  getResponse("Yy");
  pinMode (pinSDA, INPUT_PULLUP);                           // OLED
  pinMode (pinSCL, INPUT_PULLUP);
#if defined(__AVR__)
  oled.setI2cClock(400000UL);
#endif
#if defined(ESP8266)
  Wire.begin();
  Wire.setClock(400000UL);
#endif
  isOLED = buscaOLED();
  if (isOLED) {
    Serial.print(F("OLED encontrado con la direccion I2C: 0x"));
    Serial.println(i2cAdr, HEX);
  }
  else {
    Serial.println(F("ERROR: OLED no encontrado!!\n\nBuscando otros dispositivos en el bus I2C: "));
    buscaI2C();
  }
  Serial.println();
  if (isOLED) {
    isSSD1306 = false;
    isSH1106 = false;
    isSSD1309 = false;
    oled.begin(&Adafruit128x64, i2cAdr);
    showTestOLED(true);
    if (getResponse("YyNn") == 'Y') {
      Serial.println(F("Usa una pantalla grande de 1.54\" o 2.42\" con el SSD1309? (Y/N)"));
      if (getResponse("YyNn") == 'N') {
        isSSD1306 = true;
      }
      else {
        oled.begin(&SSD1309_128x64, i2cAdr);
        showTestOLED(true);
        if (getResponse("YyNn") == 'Y') {
          isSSD1309 = true;
        }
      }
    }
    else {
      oled.begin(&SH1106_128x64, i2cAdr);
      showTestOLED(true);
      if (getResponse("YyNn") == 'Y') {
        isSH1106 = true;
      }
    }
    //printTypeOLED();
  }

  poweredTouch = false;
#if (KEYPAD_TYPE == TOUCHPAD)
  numTeclas = 16;
#if defined(__AVR__)
  Serial.println(F("Alimenta el TTP229 desde los pines del teclado de la placa PacoMouse? (Y/N)"));
  if (getResponse("YyNn") == 'Y') {
    poweredTouch = true;
    pinMode(pinTOUCH_GND, OUTPUT);
    digitalWrite(pinTOUCH_GND, LOW);
    pinMode(pinTOUCH_VCC, OUTPUT);
    digitalWrite(pinTOUCH_VCC, HIGH);
  }
#endif

#else

#if defined(__AVR__)
  isKeypad = true;
#endif
#if defined(ESP8266)
  isKeypad = buscaKeypad();
  if (isKeypad) {
    Serial.print(F("Keypad encontrado en la direccion I2C: "));
    printHEX(adrPCF8574);
    Serial.println();
    keypad.begin();
  }
  else {
    if (adrPCF8574 > 0) {
      Serial.print(F("ERROR: Keypad no encontrado en la direccion I2C: "));
      printHEX(I2C_ADDRESS_KEYPAD);
      Serial.print(F("\n\nLocalizado posible Keypad en la direccion I2C: "));
      printHEX(adrPCF8574);
      Serial.println(F("\n\nModifique esta linea en este programa y vuelva a subirlo al Arduino:"));
      Serial.print(F("#define I2C_ADDRESS_KEYPAD       "));
      printHEX(adrPCF8574);
      Serial.println();
    }
    else {
      Serial.println(F("ERROR: Keypad no encontrado!!\n\nBuscando otros dispositivos en el bus I2C:"));
      buscaI2C();
    }
  }

#endif
  printSeparator();
  keypad.setDebounceTime(20);
  Serial.println(F("Usa un teclado 4x4? (Y/N)"));
  numTeclas = (getResponse("YyNn") == 'Y') ? 16 : 12;

#endif
  Serial.println(F("Desea comprobar el teclado ahora? (Y/N)"));
  if (getResponse("YyNn") == 'Y') {
    Serial.print(F("Pulse una a una las "));
    Serial.print(numTeclas);
    Serial.println(F(" teclas y compruebe que corresponde con su posicion"));
    testedTeclas = testKeypad();
    Serial.print(F("\nHa pulsado "));
    Serial.print(testedTeclas);
    Serial.println(F(" teclas diferentes.\n\nCorresponden todas las teclas con su posicion? (Y/N)"));
    if (getResponse("YyNn") == 'N')
      numTeclas = 0;
  }

  printSeparator();
  copyOutA = digitalRead (pinOutA);                         // init encoder ISR
  copyOutB = digitalRead (pinOutB);
#if defined(ESP8266)
  attachInterrupt (digitalPinToInterrupt (pinOutA), encoderISR, CHANGE);
#endif
#if defined(__AVR__)
  pciSetup(pinOutA);
#endif
  Serial.println(F("Mueva el encoder durante 10s y compruebe que varian los valores"));
  Serial.println(F("Pulse Y cuando este listo."));
  getResponse("Yy");
  encoderValue = 64;
  encoderMax = 127;
  encoderOK = true;
  timeout = millis();
  if (isOLED)
    oled.clear();
  while (millis() - timeout < 10000UL)
    testEncoder();
  Serial.println(F("Los valores han variado segun el movimiento del encoder? (Y/N)"));
  if (getResponse("YyNn") == 'N')
    encoderOK = false;

  printSeparator();
  Serial.println(F("Pulse el boton del encoder repetidamente durante 5s y compruebe su funcionamiento"));
  Serial.println(F("Pulse Y cuando este listo."));
  getResponse("Yy");
  timeout = millis();
  while (millis() - timeout < 5000UL)
    testSwitch();
  Serial.println(F("Se ha detectado la pulsacion del boton del encoder? (Y/N)"));
  if (getResponse("YyNn") == 'N')
    encoderOK = false;

  printSeparator();
  errorLN = false;
  useLN = false;
  useXPN = false;
  useZ21 = false;
  useECOS = false;
#if defined(ESP8266)
  Serial.println(F("Usa la version ECoS de PacoMouse? (Y/N)"));
  if (getResponse("YyNn") == 'Y')
    useECOS = true;
  else {
    Serial.println(F("Usa la version Z21 de PacoMouse? (Y/N)"));
    if (getResponse("YyNn") == 'Y')
      useZ21 = true;
    else
      useLN = true;
  }
#endif
#if defined(__AVR__)
  Serial.println(F("Usa la version Loconet de PacoMouse? (Y/N)"));
  if (getResponse("YyNn") == 'Y') {
    useLN = true;
    /*
      pinMode(pinLN_RXD, INPUT_PULLUP);
      pinMode(pinLN_TXD, OUTPUT);
      digitalWrite(pinLN_TXD, LOW);
      delay(1);
      if (digitalRead(pinLN_RXD) == LOW) {
      errorLN = true;
      Serial.println(F("ERROR: Estado MARK no detectado. Error en recepcion, compruebe LM311"));
      }
      digitalWrite(pinLN_TXD, HIGH);
      delay(1);
      if (digitalRead(pinLN_RXD) == HIGH) {
      Serial.println(F("ERROR: Estado SPACE no detectado. Error en transmision, compruebe transistor y LM311."));
      errorLN = true;
      }
      digitalWrite(pinLN_TXD, LOW);
    */
  }
  else {
    useXPN = true;
  }
#endif

  Serial.println(F("Desea restaturar la EEPROM a los valores por defecto? (Y/N)"));
  if (getResponse("YyNn") == 'Y') {
    Serial.println(F("Esta seguro? (Y/N)"));
    if (getResponse("YyNn") == 'Y') {
#if defined(ESP8266)
      EEPROM.begin(1024);
#endif
      updateEEPROM(EE_ADRH, 0);                             // Current Loc
      updateEEPROM(EE_ADRL, 3);
      updateEEPROM(EE_CONTRAST, 20);                        // Contraste
      updateEEPROM(EE_STOP_MODE, 0);                        // Modo Stop
      updateEEPROM(EE_SHUNTING, 0);                         // Maniobras
      updateEEPROM(EE_CMD_STA, 2);                          // Central Loconet
      updateEEPROM(EE_XBUS, 8);                             // Direccion Xbus
      updateEEPROM(EE_ROCO, 0);                             // Dir. Roco
      updateEEPROM(EE_TT_TYPE, 1);                          // Plataforma
      updateEEPROM(EE_SHORT, 99);                           // Z21 Short Address
      updateEEPROM(EE_LOCK, 0);                             // Bloquear
      updateEEPROM(EE_PHONE_ME, 0);                         // PhoneBook
      updateEEPROM(EE_PHONE_A, 0);
      updateEEPROM(EE_PHONE_B, 0);
      updateEEPROM(EE_PHONE_C, 0);
      updateEEPROM(EE_PHONE_D, 0);
      if (useLN) {                                          // Shuttle
        updateEEPROM(EE_MOD_A, 0);
        updateEEPROM(EE_CONT_A, 1);
        updateEEPROM(EE_MOD_B, 0);
        updateEEPROM(EE_CONT_B, 2);
      }
      else {
        updateEEPROM(EE_MOD_A, 65);
        updateEEPROM(EE_CONT_A, 1);
        updateEEPROM(EE_MOD_B, 65);
        updateEEPROM(EE_CONT_B, 2);
      }
      updateEEPROM(EE_TIME, 10);

      formatDisk();                                         // Automation

#if defined(ESP8266)                                        // WiFi
      snprintf (wifiSetting.ssid, 32, "");
      snprintf (wifiSetting.password, 64, "12345678");
      wifiSetting.CS_IP = IPAddress(192, 168, 0, 111);
      wifiSetting.port = 1234;
      wifiSetting.type = true;
      wifiSetting.ok = 0x464D;
      EEPROM.put(EE_WIFI, wifiSetting);
      EEPROM.commit();
#endif
      Serial.println(F("EEPROM restaurada a valores por defecto"));
    }
  }

  printResumen();
}





void loop() {

}
