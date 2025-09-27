/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.

       v0.1     08oct22   Start writting code
       v0.2     12oct22   First test with DR5000
       v0.3     15oct22   Removed LocoNetThrottleClass use
       v0.4     16oct22   Loco operations working
       v0.5     17oct22   Added fast clock catch & loco stack
       v0.6     21oct22   Added Dispatching
       v0.7     23oct22   Added Config options & turnouts
       v0.8     28oct22   Added secondary OLED (Accesibility).Modified turnout input, minor updates in Loconet
       v0.9     30oct22   Added turntable
       v0.10    06nov22   Added Program/Read CV & Language text files.
       v0.11    27nov22   Added shunting mode & Xpressnet version
       v0.12    04dec22   Added Z21 version with Wemos D1 mini (ESP8266)
       v0.13    11dec22   Added Lock loco selection, turnouts & prog options (Club Mode)
       v0.14    27dec22   Up to 28 functions
       v0.15    06jan23   Z21 max short address & Games
       v0.16    10jan23   Added shuttle train
       v0.17    26oct23   Added Fast Clock for Z21 FW 1.43 & big clock display. Added SSD1309 OLED support. Removed secondary OLED (Use 2.4" OLED for accesibility).
       v0.18    27oct23   Added PhoneBook for Loconet
       v0.19    01nov23   Added LNCV programming. Corrected minor bugs.
       v0.20b   05nov23   Withrottle version. Doesn't works as expected. Removed
       v0.21    03feb24   Option IB2 programming packets. Screen update IB function change. Correct F13..F20 for MM. Changed ISR.      Not implemented: Virtual PBX for Loconet.Transfer of library in Multimaus. Detect Loconet command station.
       v0.22    12mar24   Improvements on menu displacement and encoder reading
       v0.23    23mar24   Change arrows behaviour in turnout menu. Added game Paku Paku. Corrected bug that affects menu when updating speed. Push readed loco address in stack
       v0.24    28apr24   Added show speed step. Option to show F0-F8 instead of F0-F4
       v0.25    09may24   Change options to show Big or Small functions and turnout icon. Correct bug in Loconet driving 14 steps. Added OPC_IMM_PACKET function decoding. Correct bug in xpressnet shuttle contacts reading.
       v0.26    20jul24   Added support for ECoS and Märklin CS1. Added support for touchpad with TTP229BSF. Added Pong game. Added Loco button double press goes to turnouts. Corrected minor bugs.
       v0.27    20dec24   Added EEPROM disk. Added Automation. Added CV bits programming. Corrected minor bugs.
       v0.28    26jan25   Using modified and retailed version of Loconet.h library to reduce 0.5K the memory size to support Uhlenbrock programming (30488+678 to 29962+710). Added WiFi version with Loconet over TCP/IP (LBserver & Binary).
       v0.29    18feb25   Added support for Xpressnet WiFi. Added set fast clock option.
       v0.30    30jun25   Corrected bugs in saving automation, Loconet & Z21. Improved support of SSD1309 OLED and Daisy II WLAN.Added support for direction switch 3 positions: FWD(ON)-(OFF)-(ON)REV. Added Czech language.
*/

// Paco Mouse program version
#define VER_H "0"
#define VER_L "30"


//#define DEBUG                                               // Descomentar para mensajes de depuracion

#define ENGLISH       1                                     // Idiomas
#define SPANISH       2
#define CATALAN       3
#define PORTUGUESE    4
#define GERMAN        5
#define FRENCH        6
#define ITALIAN       7
#define NEDERLANDS    8
#define CZECH         9

#define NONE          0                                     // Posicion pasos
#define LEFT          1
#define RIGHT         2

#define BIG           0                                     // Tamaño funciones
#define SMALL         1

#define ONLY_TURN     0                                     // Estado desvios
#define TURN_SIGNAL   1
#define BUTTON_IB     2

#define XNET          0                                     // Version hardware de PacoMouse
#define LNET          1
#define Z21           2
#define ECOS          3

#define SSD1306       0                                     // Tipo de controlador de pantalla OLED
#define SH1106        1
#define SSD1309       2

#define KEYPAD        0                                     // Tipo de teclado, normal o tactil
#define TOUCHPAD      1

#define BUTTON_ENC    0                                     // Cambio direccion con boton encoder o conmutador 3 posiciones (ON-OFF-ON)
#define SWITCH_3P     1

// Libraries

#include "config.h"                                         // Paco Mouse config file
#ifdef USE_Z21
#include <ESP8266WiFi.h>                                    // Arduino ESP8266 core                         v3.1.2
#include <WiFiUdp.h>
#include <Wire.h>
#ifndef USE_TTP229
#include <Keypad_I2C.h>                                     // https://github.com/joeyoung/arduino_keypads  v3.0.0
#endif
#endif
#ifdef USE_ECOS
#include <ESP8266WiFi.h>                                    // Arduino ESP8266 core                         v3.0.2
#include <Wire.h>
#ifndef USE_TTP229
#include <Keypad_I2C.h>                                     // https://github.com/joeyoung/arduino_keypads  v3.0.0
#endif
#endif
#ifdef USE_LOCONET
#include "lnet.h"
#ifdef USE_LNWIFI
#include <ESP8266WiFi.h>                                    // Arduino ESP8266 core                         v3.1.2
#include <Wire.h>
#ifndef USE_TTP229
#include <Keypad_I2C.h>                                     // https://github.com/joeyoung/arduino_keypads  v3.0.0
#endif
#endif
#ifdef USE_LNWIRE
#include <avr/io.h>
#include <avr/interrupt.h>
#endif
#endif
#ifdef USE_XPRESSNET
#ifdef USE_XNWIRE
#include <avr/interrupt.h>
#endif
#ifdef USE_XNWIFI
#include <ESP8266WiFi.h>                                    // Arduino ESP8266 core                         v3.1.2
#include <Wire.h>
#ifndef USE_TTP229
#include <Keypad_I2C.h>                                     // https://github.com/joeyoung/arduino_keypads  v3.0.0
#endif
#endif
#endif
#ifdef USE_TTP229
#include "keypadTTP229.h"
#else
#include <Keypad.h>                                         // https://github.com/Chris--A/Keypad           v3.1.1
#endif
#include <SSD1306Ascii.h>                                   // https://github.com/greiman/SSD1306Ascii      v1.3.5
#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
#include <SSD1306AsciiAvrI2c.h>
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
#include "SSD1306AsciiWire.h"
#endif
#include <EEPROM.h>


#include "Sym_16x16.h"                                      // Paco Mouse fonts
#include "Sym_Proportional.h"
#include "Num_15x31.h"
#include "Num_32x48.h"
#if  (LANGUAGE == ENGLISH)                                  // Paco Mouse language
#include "english.h"
#endif
#if  (LANGUAGE == SPANISH)
#include "spanish.h"
#endif
#if  (LANGUAGE == CATALAN)
#include "catalan.h"
#endif
#if  (LANGUAGE == PORTUGUESE)
#include "portuguese.h"
#endif
#if  (LANGUAGE == GERMAN)
#include "german.h"
#endif
#if  (LANGUAGE == FRENCH)
#include "french.h"
#endif
#if  (LANGUAGE == ITALIAN)
#include "italian.h"
#endif
#if  (LANGUAGE == NEDERLANDS)
#include "nederlands.h"
#endif
#if  (LANGUAGE == CZECH)
#include "czech.h"
#endif

#ifdef DEBUG
char output[80];

#define DEBUG_MSG(...)  snprintf(output,80, __VA_ARGS__ );   \
  Serial.println(output);
#else
#define DEBUG_MSG(...)
#endif


////////////////////////////////////////////////////////////
// ***** TECLADO *****
////////////////////////////////////////////////////////////

enum teclas {K_NONE, K_NUM0, K_NUM1, K_NUM2, K_NUM3, K_NUM4, K_NUM5, K_NUM6, K_NUM7, K_NUM8, K_NUM9, K_MENU, K_ENTER, K_A, K_B, K_C, K_D};

#define FILAS 4                                             // Filas y Columnas de la matriz del teclado

#ifdef  USE_TTP229

#ifdef REAR_TOUCH
char keys[16] = {K_A, K_NUM3, K_NUM2, K_NUM1, K_B, K_NUM6, K_NUM5, K_NUM4, K_C, K_NUM9, K_NUM8, K_NUM7, K_D, K_ENTER, K_NUM0, K_MENU};
#else
char keys[16] = {K_NUM1, K_NUM2, K_NUM3, K_A, K_NUM4, K_NUM5, K_NUM6, K_B, K_NUM7, K_NUM8, K_NUM9, K_C, K_MENU, K_NUM0, K_ENTER, K_D};
#endif

Keypad_TTP229 keypad (keys, pinTOUCH_SCL, pinTOUCH_SDO);

#else

#ifdef USE_KEYPAD_4x4
#define COLUMNAS 4

char keys[FILAS][COLUMNAS] = {                              // Disposicion de las teclas
  {K_NUM1, K_NUM2, K_NUM3, K_A},
  {K_NUM4, K_NUM5, K_NUM6, K_B},
  {K_NUM7, K_NUM8, K_NUM9, K_C},
  {K_MENU, K_NUM0, K_ENTER, K_D}
};

byte filaPins[FILAS]   = {pinFILA1, pinFILA2, pinFILA3, pinFILA4};                // pins de las filas del teclado
byte colPins[COLUMNAS] = {pinCOLUMNA1, pinCOLUMNA2, pinCOLUMNA3, pinCOLUMNA4};    // pins de las columnas del teclado
#else
#define COLUMNAS 3

char keys[FILAS][COLUMNAS] = {                              // Disposicion de las teclas
  {K_NUM1, K_NUM2, K_NUM3},
  {K_NUM4, K_NUM5, K_NUM6},
  {K_NUM7, K_NUM8, K_NUM9},
  {K_MENU, K_NUM0, K_ENTER}
};

byte filaPins[FILAS]   = {pinFILA1, pinFILA2, pinFILA3, pinFILA4};                // pins de las filas del teclado
byte colPins[COLUMNAS] = {pinCOLUMNA1, pinCOLUMNA2, pinCOLUMNA3};                 // pins de las columnas del teclado
#endif

#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
Keypad keypad = Keypad( makeKeymap(keys), filaPins, colPins, FILAS, COLUMNAS);
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
Keypad_I2C keypad ( makeKeymap(keys), filaPins, colPins, FILAS, COLUMNAS, I2C_ADDRESS_KEYPAD);
#endif
#endif

byte keyValue;                                              // valor tecla pulsada
bool keyOn;

byte dirValue;                                              // valor interruptor direccion
bool dirChange;

////////////////////////////////////////////////////////////
// ***** ENCODER *****
////////////////////////////////////////////////////////////

byte outA, outB, copyOutA, copyOutB;                        // encoder input used by ISR
volatile byte encoderValue;                                 // encoder shared values (ISR & program)
volatile byte encoderMax;
volatile bool encoderChange;

byte statusSwitch;
bool switchOn;
const unsigned long timeoutButtons = 50;                    // temporizador antirebote
unsigned long timeButtons;


////////////////////////////////////////////////////////////
// ***** OLED *****
////////////////////////////////////////////////////////////

#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
SSD1306AsciiAvrI2c oled;                                    // Pantalla principal
AvrI2c i2c;
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
SSD1306AsciiWire oled;                                      // Pantalla principal
#endif

enum Screen { SCR_LOGO, SCR_ENDLOGO, SCR_MENU, SCR_SPEED, SCR_LOCO, SCR_DISPATCH, SCR_TURNOUT, SCR_ERROR, SCR_TURNTABLE,
              SCR_CV, SCR_CFG, SCR_WAIT, SCR_SELCV, SCR_DIRCV, SCR_SSID, SCR_PASSWORD, SCR_IP_ADDR, SCR_PORT, SCR_GAME,
              SCR_SHUTTLE, SCR_PHONE, SCR_STATION, SCR_PHONEBOOK, SCR_PHONE_NUM, SCR_LNCV, SCR_DEVICE,
              SCR_AUTO_SELECT, SCR_AUTO_NAME, SCR_AUTO_EDIT, SCR_AUTO_DELETE, SCR_TIME,
            };

#ifdef USE_LOCONET
enum Option { OPT_SPEED, OPT_LOCO, OPT_DISPATCH, OPT_TURNOUT, OPT_TURNTABLE, OPT_SHUTTLE,
#ifdef USE_AUTOMATION
              OPT_AUTOMATION,
#endif
              OPT_CV, OPT_CFG, MENU_ITEMS
            };
#endif
#ifdef USE_XPRESSNET
enum Option { OPT_SPEED, OPT_LOCO, OPT_TURNOUT, OPT_TURNTABLE, OPT_SHUTTLE,
#ifdef USE_AUTOMATION
              OPT_AUTOMATION,
#endif
              OPT_CV, OPT_CFG, MENU_ITEMS
            };
#endif
#ifdef USE_Z21
enum Option { OPT_SPEED, OPT_LOCO, OPT_TURNOUT, OPT_TURNTABLE, OPT_SHUTTLE,
#ifdef USE_AUTOMATION
              OPT_AUTOMATION,
#endif
              OPT_CV, OPT_CFG, MENU_ITEMS
            };
#endif
#ifdef USE_ECOS
enum Option { OPT_SPEED, OPT_LOCO, OPT_TURNOUT, OPT_TURNTABLE, OPT_SHUTTLE,
#ifdef USE_AUTOMATION
              OPT_AUTOMATION,
#endif
              OPT_CV, OPT_CFG, MENU_ITEMS
            };
#endif

enum Err {ERR_OFF, ERR_STOP, ERR_SERV};
enum PosDigit {UNITS, TENS, CENTS, THOUSANDS};

bool updateOLED, updateAllOLED, updateExtraOLED;
byte scrOLED, errOLED, optOLED, cfgOLED, prgOLED, fileOLED;
byte scrSpeed, scrFunc;
byte scrHour, scrMin, scrRate, scrPosTime;
byte clockHour, clockMin, clockRate;
unsigned long clockTimer, clockInterval;                    // Internal fast clock calculation
unsigned int scrLoco, scrPort;
unsigned long timeOLED, timeoutOLED;
#define ONE_DAY 86400000UL

union numInput {                                            // Entrada numerica 4 digitos
  byte digit[4];
  unsigned long packed;
} myInput;

byte diezMiles;

const byte posPuntos [8][2] = {{80, 4}, {64, 6}, {48, 6}, {32, 4}, {32, 2}, {48, 0}, {64, 0}, {80, 2}};
byte estadoPuntos;

enum Turntable {TT_7686_KB14, TT_7686_KB15, TT_F6915};

#ifdef USE_LOCONET
enum cmdStation {CMD_DR, CMD_ULI, CMD_DIG};

enum cfgOpt { CFG_TIT, CFG_CONTR, CFG_CONTR_VAL, CFG_STOP_MODE, CFG_STOP_VEL0, CFG_STOP_EMERG, CFG_SHUNTING, CFG_CMD_STA, CFG_CMD_DR, CFG_CMD_ULI, CFG_CMD_DIG,
#if defined(USE_LNWIFI)
              CFG_WIFI, CFG_WIFI_SSID, CFG_WIFI_PWD, CFG_WIFI_IP,
              CFG_SERVER, CFG_WIFI_PORT, CFG_BINARY, CFG_LBSERVER,
#endif
              CFG_TT, CFG_TT_KB14, CFG_TT_KB15, CFG_TT_TCTRL,
#ifdef USE_SET_TIME
              CFG_TIME, CFG_SET_TIME, CFG_SYNC,
#endif
              CFG_LOCK, CFG_LOCK_LOCO, CFG_LOCK_TURNOUT, CFG_LOCK_PROG,
#ifdef GAME_EXTRA
              CFG_GAME, CFG_GAME_TIC, CFG_GAME_SNAKE, CFG_GAME_FLAPPY, CFG_GAME_PAKU, CFG_GAME_PONG,
#endif
#ifdef  USE_PHONE
              CFG_PHONE, CFG_PHONE_ME, CFG_PHONE_STAT,
#endif
              MAX_CFG_ITEMS
            };
#endif
#ifdef USE_XPRESSNET
enum cfgOpt { CFG_TIT, CFG_CONTR, CFG_CONTR_VAL, CFG_STOP_MODE, CFG_STOP_VEL0, CFG_STOP_EMERG, CFG_SHUNTING,
#if defined(USE_XNWIRE)
              CFG_XBUS, CFG_XBUS_DIR,
#endif
#if defined(USE_XNWIFI)
              CFG_WIFI, CFG_WIFI_SSID, CFG_WIFI_PWD, CFG_WIFI_IP,
#endif
              CFG_TT, CFG_TT_KB14, CFG_TT_KB15, CFG_TT_TCTRL, CFG_TT_ROCO,
#ifdef USE_SET_TIME
              CFG_TIME, CFG_SET_TIME,
#endif
              CFG_LOCK, CFG_LOCK_LOCO, CFG_LOCK_TURNOUT, CFG_LOCK_PROG,
#ifdef GAME_EXTRA
              CFG_GAME, CFG_GAME_TIC, CFG_GAME_SNAKE, CFG_GAME_FLAPPY, CFG_GAME_PAKU,
#if defined(USE_XNWIFI)
              CFG_GAME_PONG,
#endif
#endif
              MAX_CFG_ITEMS
            };
#endif
#ifdef USE_Z21
enum cfgOpt { CFG_TIT, CFG_CONTR, CFG_CONTR_VAL, CFG_STOP_MODE, CFG_STOP_VEL0, CFG_STOP_EMERG, CFG_SHUNTING, CFG_WIFI, CFG_WIFI_SSID, CFG_WIFI_PWD, CFG_WIFI_IP,
              CFG_SHORT, CFG_SHORT_99, CFG_SHORT_127, CFG_TT, CFG_TT_KB14, CFG_TT_KB15, CFG_TT_TCTRL, CFG_TT_ROCO,
#ifdef USE_SET_TIME
              CFG_TIME, CFG_SET_TIME,
#endif
              CFG_LOCK, CFG_LOCK_LOCO, CFG_LOCK_TURNOUT, CFG_LOCK_PROG,
#ifdef GAME_EXTRA
              CFG_GAME, CFG_GAME_TIC, CFG_GAME_SNAKE,  CFG_GAME_FLAPPY, CFG_GAME_PAKU, CFG_GAME_PONG,
#endif
              MAX_CFG_ITEMS
            };
#endif
#ifdef USE_ECOS
enum cfgOpt { CFG_TIT, CFG_CONTR, CFG_CONTR_VAL, CFG_STOP_MODE, CFG_STOP_VEL0, CFG_STOP_EMERG, CFG_SHUNTING, CFG_WIFI, CFG_WIFI_SSID, CFG_WIFI_PWD, CFG_WIFI_IP,
              CFG_TT, CFG_TT_KB14, CFG_TT_KB15, CFG_TT_TCTRL,
              CFG_LOCK, CFG_LOCK_LOCO, CFG_LOCK_TURNOUT, CFG_LOCK_PROG,
#ifdef GAME_EXTRA
              CFG_GAME, CFG_GAME_TIC, CFG_GAME_SNAKE,  CFG_GAME_FLAPPY, CFG_GAME_PAKU, CFG_GAME_PONG,
#endif
              MAX_CFG_ITEMS
            };
#endif


enum cfgType {TCFG_OPT, TCFG_NUM, TCFG_RAD, TCFG_CHK, TCFG_TIT, TCFG_TXT};

struct cfgItem {
  byte type;
  unsigned int value;
};


struct cfgItem cfgMenu[MAX_CFG_ITEMS] = {
  {TCFG_TIT,  0},                                         // OPTIONS
  {TCFG_OPT,  0},                                         // CONTRAST
  {TCFG_NUM,  0},
  {TCFG_OPT,  0},                                         // STOP MODE
  {TCFG_RAD,  0},
  {TCFG_RAD,  0},
  {TCFG_CHK,  0},
#ifdef USE_LOCONET
  {TCFG_OPT,  0},                                         // CMD. STATION
  {TCFG_RAD,  0},
  {TCFG_RAD,  0},
  {TCFG_RAD,  0},
#endif
#ifdef USE_XNWIRE
  {TCFG_OPT,  0},                                         // DIR. XPRESSNET
  {TCFG_NUM,  0},
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  {TCFG_OPT,  0},                                         // WIFI
  {TCFG_TXT,  0},
  {TCFG_TXT,  0},
  {TCFG_TXT,  0},
#endif
#if defined(USE_LNWIFI)
  {TCFG_OPT,  0},                                         // SERVER
  {TCFG_TXT,  0},
  {TCFG_RAD,  0},
  {TCFG_RAD,  0},
#endif
#if defined(USE_Z21)
  {TCFG_OPT,  0},                                         // SHORT ADR.
  {TCFG_RAD,  0},
  {TCFG_RAD,  0},
#endif
  {TCFG_OPT,  0},                                         // TURNTABLE
  {TCFG_RAD,  0},
  {TCFG_RAD,  0},
  {TCFG_RAD,  0},
#if defined(USE_XPRESSNET) || defined(USE_Z21)
  {TCFG_CHK,  0},
#endif
#ifdef USE_SET_TIME
  {TCFG_OPT,  0},                                         // SET TIME
  {TCFG_TXT,  0},
#ifdef USE_LOCONET
  {TCFG_CHK,  0},
#endif
#endif
  {TCFG_OPT,  0},                                         // LOCK
  {TCFG_CHK,  0},
  {TCFG_CHK,  0},
  {TCFG_CHK,  0},
#ifdef GAME_EXTRA
  {TCFG_OPT,  0},                                         // GAME
  {TCFG_TXT,  0},
  {TCFG_TXT,  0},
  {TCFG_TXT,  0},
  {TCFG_TXT,  0},
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  {TCFG_TXT,  0},
#endif
#endif
#ifdef USE_PHONE
  {TCFG_OPT,  0},                                         // PHONEBOOK
  {TCFG_TXT,  0},
  {TCFG_TXT,  0},
#endif
};


bool changeCfgOption;

bool showBigClock;
bool modeTurntable;
byte destTrack, typeTurntable;
unsigned int baseAdrTurntable, baseOffset;

#ifdef USE_SSD1309

// Initialization commands for a 128x64 SSD1309 oled display.
static const uint8_t MEM_TYPE SSD1309_128x64init[] = {
  // Init sequence for SSD1309 128x64 OLED module based on luma.oled initialization
  SSD1306_DISPLAYOFF,
  SSD1306_SETDISPLAYCLOCKDIV, 0xF0,  // Set Display Clock Divisor v = 0xF0, default is 0x80
  SSD1306_SETMULTIPLEX, 0x3F,        // ratio 64
  SSD1306_SETDISPLAYOFFSET, 0x00,    // no offset
  SSD1306_SETSTARTLINE,              // line #0
  //    SSD1306_CHARGEPUMP, 0x14,          // internal vcc
  SSD1306_MEMORYMODE, 0x00,          // horizontal mode
  SSD1306_SEGREMAP | 0x01,           // column 127 mapped to SEG0
  SSD1306_COMSCANDEC,                // column scan direction reversed
  SSD1306_SETCOMPINS, 0x12,          // alt COM pins, disable remap
  SSD1306_SETPRECHARGE, 0xF1,        // Set Pre-charge Period
  SSD1306_SETVCOMDETECT, 0x40,       // vcomh regulator level
  SSD1306_DISPLAYALLON_RESUME,       // Resume display from GRAM content
  SSD1306_NORMALDISPLAY,             // Set Normal Display
  SSD1306_SETCONTRAST, 0x32,         // contrast level (0xCF) 207 by default. Using 0x32 to reduce consumption
  0x21, 0x00, 0x7F,                  // Set Column Address
  0x22, 0x00, 0x07,                  // Set Page Address
  SSD1306_DISPLAYON,
};
/*
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
*/
/*
  // Initialization commands for a 128x64 SSD1309 oled display.
  static const uint8_t MEM_TYPE SSD1309_128x64init[] = {
  // Init sequence for SSD1309 128x64 OLED module based on u8g2 library initialization
  SSD1306_DISPLAYOFF,
  SSD1306_SETDISPLAYCLOCKDIV, 0xA0,  // the suggested ratio 0xA0
  //    SSD1306_SETMULTIPLEX, 0x3F,        // ratio 64
  SSD1306_SETSTARTLINE,              // line #0
  //    SSD1306_CHARGEPUMP, 0x14,          // internal vcc
  SSD1306_MEMORYMODE, 0x02,          // page mode
  SSD1306_SEGREMAP | 0x1,            // column 127 mapped to SEG0
  SSD1306_COMSCANDEC,                // column scan direction reversed
  SSD1306_SETCOMPINS, 0x12,          // alt COM pins, disable remap
  SSD1306_SETCONTRAST, 0x6F,         // contrast level 111
  SSD1306_SETPRECHARGE, 0xD3,        // pre-charge period (1, 15)! 0xF1
  //    SSD1306_SETDISPLAYOFFSET, 0x0,     // no offset
  SSD1306_SETVCOMDETECT, 0x20,       // vcomh regulator level
  SSD1306_DEACTIVATE_SCROLL,         // !!
  SSD1306_DISPLAYALLON_RESUME,
  SSD1306_NORMALDISPLAY,
  //    SSD1306_DISPLAYON
  };

  // Initialization commands for a 128x64 SSD1309 oled display.
  static const uint8_t MEM_TYPE SSD1309_128x64init[] = {
  // Init sequence for SSD1309 128x64 OLED module based on Aliexpress initialization
  SSD1306_DISPLAYOFF,
  SSD1306_SETLOWCOLUMN,
  SSD1306_SETHIGHCOLUMN,
  SSD1306_SETSTARTLINE,
  SSD1306_SETCONTRAST, 0x32,
  SSD1306_SEGREMAP | 0x1,            // column 127 mapped to SEG0
  SSD1306_NORMALDISPLAY.
  SSD1306_SETMULTIPLEX, 0x3F,
  SSD1306_COMSCANDEC,
  SSD1306_SETDISPLAYOFFSET, 0x00,
  SSD1306_SETDISPLAYCLOCKDIV, 0xA0,
  SSD1306_SETPRECHARGE, 0xF1,
  SSD1306_SETCOMPINS, 0x12,
  0x91, 0x3F, 0x3F, 0x3F, 0x3F,
  SSD1306_DISPLAYON,
  };
*/

// Initialize a 128x64 oled display. //
static const DevType MEM_TYPE SSD1309_128x64 = {
  SSD1309_128x64init,
  sizeof(SSD1309_128x64init),
  128,
  64,
  0
};

#endif


////////////////////////////////////////////////////////////
// ***** LOCONET *****
////////////////////////////////////////////////////////////

#ifdef USE_LOCONET

#ifdef USE_LNWIRE
#define VER_STR F(" v" VER_H "." VER_L "-LN")

typedef enum
{
  LN_CD_BACKOFF = 0,
  LN_PRIO_BACKOFF,
  LN_NETWORK_BUSY,
  LN_DONE,
  LN_COLLISION,
  LN_UNKNOWN_ERROR,
  LN_RETRY_ERROR
} LN_STATUS ;

lnMsg uartPacket;                                           // Paquete recibido por Loconet
lnMsg *RecvPacket ;                                         // Paquete recibido
lnMsg SendPacket;                                           // Paquete a enviar por Loconet

#endif

#ifdef USE_LNWIFI
#define VER_STR F(" v" VER_H "." VER_L "-LNW")

WiFiClient LnWiFi;

lnMsg SendPacket;                                           // Paquete a enviar por Loconet WiFi
lnMsg RecvPacket;                                           // Paquete recibido por Loconet WiFi

bool isLBServer;
char rcvStr[10];
byte rcvStrPos;
enum rcvPhase {WAIT_TOKEN, RECV_TOKEN, RECV_PARAM, SENT_TOKEN, SENT_PARAM};
byte rcvStrPhase;
bool sentOK;
unsigned long timeout;

#endif

union {                                                     // Paquete a enviar por Loconet para Uhlenbrock
  lnMsg   lnMsgCV;
  uint8_t data[32];
} SendPacketUhli;

#define UHLI_PRG_START  0x41                                // Intellibox II program task
#define UHLI_PRG_END    0x40

enum statusSlot {STAT_FREE, STAT_COMMON, STAT_IDLE, STAT_IN_USE};

struct slot {
  byte num;                                                 // slot number: 1..120 (0x01..0x78)
  byte state;                                               // state: in_use/idle/common/free
  byte trk;                                                 // track status
} mySlot;

const byte stepsLN[] = {28, 28, 14, 128, 28, 28, 14, 128};

#define PING_INTERVAL 55000UL                               // Prevent PURGE of slot (55s)
unsigned long pingTimer;
#define SYNC_TIME     70000UL                               // clock slot is typically "pinged" or read SYNC'd every 70 to 100 seconds
unsigned long syncTimer;
bool doDispatchGet, doDispatchPut;                          // dispatching
unsigned int myID;
unsigned int virtualLoco;
bool accStateReq;                                           // peticion de estado de desvio
byte typeCmdStation;                                        // tipo de central para funciones
bool lnetProg;                                              // programando CV o LNCV
byte ulhiProg;                                              // Programming track off (UHLI)

#ifdef USE_PHONE

#define PHONE_BASE 700                                      // Accesory base address for phone numbers

enum phone {PHONE_IDLE, PHONE_CHECK, PHONE_BUSY, PHONE_CALLING, WAIT_ANSWER, PHONE_RING, WAIT_USER, CALL_REJECTED, CALL_ACCEPTED, WAIT_TRAIN, GET_TRAIN};
enum stations {PHONE_ME, PHONE_A, PHONE_B, PHONE_C, PHONE_D, MAX_STATION};

unsigned int phoneBook[MAX_STATION];
byte phonePhase;
unsigned int callingStation;
unsigned int originateStation;
unsigned int myStation;
unsigned int lastStationPickUp;
bool doPhoneStatus;
//byte phonePBX;

#endif

#ifdef USE_LNCV
#define LNCV_REQID_CFGREAD      0x1F
#define LNCV_REQID_CFGWRITE     0x20
#define LNCV_REQID_CFGREQUEST   0x21
#define LNCV_FLAG_PRON          0x80
#define LNCV_FLAG_PROFF         0x40

enum lncv {LNCV_ART, LNCV_MOD, LNCV_ADR, LNCV_VAL};
unsigned int artNum;
unsigned int modNum;
unsigned int numLNCV;
unsigned int valLNCV;

byte optLNCV;

#endif

#endif


////////////////////////////////////////////////////////////
// ***** XPRESSNET *****
////////////////////////////////////////////////////////////

#ifdef USE_XPRESSNET
#ifdef USE_XNWIRE
#define VER_STR F("v" VER_H "." VER_L "-XPN")

#define WAIT_FOR_XMIT_COMPLETE {while (!(UCSR0A & (1<<TXC0))); UCSR0A = (1<<TXC0); UCSR0A = 0;}

enum xAnswer {HEADER, DATA1, DATA2, DATA3, DATA4, DATA5};

unsigned long infoTimer;
#endif
#ifdef USE_XNWIFI
#define VER_STR F("v" VER_H "." VER_L "-XPW")

#define XnetPort 5550

WiFiClient XnWiFi;

enum xPacket {FRAME1, FRAME2, HEADER, DATA1, DATA2, DATA3, DATA4, DATA5};

#define PING_INTERVAL 10000UL                               // Prevent closing the connection
unsigned long pingTimer;
unsigned long timeout;

#endif

#define csNormalOps                   0x00                  // estado de la central
#define csEmergencyStop               0x01
#define csEmergencyOff                0x02
#define csStartMode                   0x04
#define csShortCircuit                0x04                  // Z21
#define csServiceMode                 0x08
#define csReserved                    0x10
#define csProgrammingModeActive       0x20                  // Z21
#define csPowerUp                     0x40
#define csErrorRAM                    0x80

volatile byte rxBufferXN[20];                               // Comunicacion Xpressnet
volatile byte txBuffer[14];
volatile byte txBytes;
volatile byte rxBytes;
volatile byte miCallByte;
volatile bool enviaMensaje;                                 // Envia nuevo mensaje Xpressnet
volatile bool leerDatoXN;
byte rxXOR, rxIndice, txXOR, rxData;
byte csStatus, xnetVersion, xnetCS;
byte miModulo, miAccPos;

int miDireccionXpressnet;
bool getInfoLoco, getResultsSM;
bool askMultimaus;                                          // Multimaus
byte highVerMM, lowVerMM;

#endif


////////////////////////////////////////////////////////////
// ***** Z21 *****
////////////////////////////////////////////////////////////

#ifdef USE_Z21
#define VER_STR F("v" VER_H "." VER_L "-Z21")

WiFiUDP Udp;
#define z21Port 21105                                       // local port to listen on Z21

byte packetBuffer[1500];                                    // buffer to hold incoming packet, max. 1472 bytes
byte OutData[80];
byte OutPos = 0;
byte OutXOR;
byte csStatus = 0;

// Z21 white message header
#define LAN_GET_SERIAL_NUMBER         0x10
#define LAN_GET_CODE                  0x18
#define LAN_GET_HWINFO                0x1A
#define LAN_LOGOFF                    0x30
#define LAN_X_Header                  0x40
#define LAN_SET_BROADCASTFLAGS        0x50
#define LAN_GET_BROADCASTFLAGS        0x51
#define LAN_GET_LOCOMODE              0x60
#define LAN_SET_LOCOMODE              0x61                  // Not implemented
#define LAN_GET_TURNOUTMODE           0x70
#define LAN_SET_TURNOUTMODE           0x71                  // Not implemented
#define LAN_RMBUS_DATACHANGED         0x80
#define LAN_RMBUS_GETDATA             0x81
#define LAN_RMBUS_PROGRAMMODULE       0x82                  // Not implemented
#define LAN_SYSTEMSTATE_DATACHANGED   0x84
#define LAN_SYSTEMSTATE_GETDATA       0x85
#define LAN_RAILCOM_DATACHANGED       0x88
#define LAN_RAILCOM_GETDATA           0x89
#define LAN_LOCONET_Z21_RX            0xA0
#define LAN_LOCONET_Z21_TX            0xA1
#define LAN_LOCONET_FROM_LAN          0xA2
#define LAN_LOCONET_DISPATCH_ADDR     0xA3                  // unused
#define LAN_LOCONET_DETECTOR          0xA4
#define LAN_FAST_CLOCK_CONTROL        0xCC
#define LAN_FAST_CLOCK_DATA           0xCD
#define LAN_FAST_CLOCK_SETTINGS_GET   0xCE                  // unused
#define LAN_FAST_CLOCK_SETTINGS_SET   0xCF                  // unused

#define csNormalOps                   0x00                  // Normal Operation Resumed
#define csEmergencyStop               0x01                  // Emergency Stop
#define csTrackVoltageOff             0x02                  // Emergency off
#define csShortCircuit                0x04                  // ShortCircuit
#define csProgrammingModeActive       0x20                  // Service Mode

enum xAnswer {DATA_LENL, DATA_LENH, DATA_HEADERL, DATA_HEADERH, XHEADER, DB0, DB1, DB2, DB3, DB4, DB5, DB6, DB7, DB8};

byte shortAddress;
byte pingTimer;
#define PING_INTERVAL 5                                     // Prevent automatic LOGOFF (5s)
bool waitResultCV;
#endif


////////////////////////////////////////////////////////////
// ***** ECOS / CS1 *****
////////////////////////////////////////////////////////////

#ifdef USE_ECOS
#define VER_STR F("v" VER_H "." VER_L "-ECS")

#define ECoSPort 15471

WiFiClient ECoS;

#define ID_ECOS             1                               // Base objects ID
#define ID_PRGMANAGER       5
#define ID_POMMANAGER       7
#define ID_LOKMANAGER       10
#define ID_SWMANAGER        11
#define ID_SNIFFERMANAGER   25
#define ID_S88MANAGER       26
#define ID_BOOSTERMANAGER   27
#define ID_S88FEEDBACK      100
#define ID_INTBOOSTER       65000

char cmd[64];                                               // send buffer

#define NAME_LNG 16                                         // loco names length

struct {
  unsigned int id;                                          // stack loco data
  unsigned int locoAddress;
  char locoName[NAME_LNG + 1];
  byte  protocol;
} rosterList[LOCOS_IN_STACK];


#define BUF_LNG 1024
char inputBuffer[BUF_LNG];
unsigned int inputLength;

unsigned long timeout;
byte csStatus;
bool requestedCV;
int myLocoID;
byte mySpeedStep;
int myTurnoutID;

#define TEXTLEN         32             // Length of symbols in input

char  putBackChr;
int   posFile;

// Tokens
enum {
  T_NULL, T_START, T_ENDB, T_REPLY, T_EVENT, T_END, T_COMMA, T_INTLIT, T_SEMI, T_LPARENT, T_RPARENT,
  T_LBRACKET, T_RBRACKET, T_STRLIT, T_IDENT, T_PRINT, T_INT,
  T_GET, T_SET, T_QOBJ, T_ADDR, T_NAME, T_PROT, T_STATUS, T_STATUS2, T_STATE,
  T_REQ, T_VIEW, T_CONTROL, T_RELEASE, T_FORCE,
  T_GO, T_STOP, T_SHUTDWN, T_OK, T_MSG, T_SPEED, T_STEPS, T_DIR, T_FUNC,
  T_LOST, T_OTHER, T_CV, T_CVOK, T_ERROR, T_SWITCH,
  T_CS1, T_ECOS, T_ECOS2
};

#ifdef DEBUG
// List of printable tokens
const char *tokstr[] = { "null", "<", ">", "REPLY", "EVENT", "END", ",", "LIT", ";", "(", ")",
                         "[", "]", "STRING", "IDENT", "PRINT", "INT",
                         "get", "set", "queryObjects", "addr", "name", "protocol", "status", "status2", "state",
                         "request", "view", "control", "release", "force",
                         "GO", "STOP", "SHUTDOWN", "OK", "msg", "speed", "speedstep", "dir", "func",
                         "CONTROL_LOST", "other", "cv", "ok", "error", "switch",
                         "CentralStation", "ECoS", "ECoS2"
                       };
#endif

// Token structure
struct token {
  char token;
  int intvalue;
};

struct token T;
int   tokenType;
char  Text[TEXTLEN + 1];

int errCode;
int idManager;
int idCommand;
int idS88;
int stateS88;
int numLoks;
int lastNumValue;
char lastTxtChar;
byte typeCmdStation;

byte msgDecodePhase;
enum {MSG_WAIT, MSG_START, MSG_REPLY, MSG_EVENT, MSG_END, MSG_REPLYBODY, MSG_EVENTBODY};

#endif



////////////////////////////////////////////////////////////
// ***** MOUSE *****
////////////////////////////////////////////////////////////

enum Settings {
  EE_ADRH, EE_ADRL, EE_IDH, EE_IDL, EE_CONTRAST, EE_STOP_MODE, EE_CMD_STA, EE_XBUS, EE_TT_TYPE, EE_SHUNTING, EE_ROCO, EE_LOCK, EE_SHORT,
  EE_MOD_A, EE_CONT_A, EE_MOD_B, EE_CONT_B, EE_TIME, EE_PHONE_ME, EE_PHONE_A, EE_PHONE_B, EE_PHONE_C, EE_PHONE_D,
  EE_WIFI,                                     //  datos WiFi. (Tiene que ser el ultimo)
}; // EEPROM settings

#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)

struct {
  char ssid[33];                                            // SSID
  char password[65];                                        // Password
  IPAddress CS_IP;                                          // IP
  unsigned int port;                                        // Port
  bool type;                                                // Server type
  int  ok;
} wifiSetting;

int networks;                                               // WiFi networks found
byte scrSSID;
char scrPwd[65];                                            // new password
byte scrPwdLng;
IPAddress scrIP;
byte scrPosIP;
//byte shortAddress;
unsigned long infoTimer;

#define LowBattADC  2980                                    // LOW BATT 2.97V
ADC_MODE(ADC_VCC);                                          // For Vcc voltage read
int battery;
bool lowBATT;

#endif

// DESVIADO: ROJO  - > links  thrown
// RECTO:    VERDE + < rechts closed
enum posDesvio {NO_MOVIDO, DESVIADO, RECTO, INVALIDO};

union locomotora {                                          // direccion locomotora
  byte adr[2];
  unsigned int address;
} myLoco;

union funciones {
  byte xFunc[4];                                            // array para funciones, F0F4, F5F12, F13F20 y F21F28
  unsigned long Bits;                                       // long para acceder a los bits
} myFunc;

#if  (FUNC_SIZE == BIG)
enum BlkFunc {FNC_0_4, FNC_5_8, FNC_9_12, FNC_13_16, FNC_17_20, FNC_21_24, FNC_25_28};
#endif
#if  (FUNC_SIZE == SMALL)
enum BlkFunc {FNC_0_9, FNC_10_19, FNC_20_28};
#endif

byte mySteps, myDir, mySpeed, myPosTurnout, myModule;       // locomotora y desvio actual
byte lastPosTurnout;
unsigned int myTurnout, myCV;
byte stopMode;
bool shuntingMode;
bool accEnviado;                                            // se ha enviado orden activacion de desvio

unsigned int locoStack[LOCOS_IN_STACK];                     // stack para ultimas locos accedidas

enum nameCV {PRG_PTRK, PRG_MTRK, PRG_DIR, PRG_CV2, PRG_CV6, PRG_CV5, PRG_CV3, PRG_CV4, PRG_CV29, PRG_FAB, PRG_BIT,
#ifdef USE_LNCV
             PRG_LNCV,
#endif
             PRG_ITEMS
            };
enum other {TXT_SEL_TURN, TXT_DEST_TRK, TXT_SEL_LOCO, TXT_LOCO, TXT_SEARCH, TXT_CONNECT, TXT_STATION, TXT_PHONE_BUSY, TXT_STOP_TRAIN,
#ifdef USE_AUTOMATION
            TXT_AUTOMATION, TXT_AUTO_NAME, TXT_AUTO_DELETE,
#endif
           };
enum prgCV {PRG_IDLE, PRG_CV, PRG_RD_CV29, PRG_RD_CV1, PRG_RD_CV17, PRG_RD_CV18, PRG_WR_CV1, PRG_WR_CV17, PRG_WR_CV18, PRG_WR_CV29};

unsigned int CVaddress, CVdata, CVmax;                      // programacion CV
bool modeProg;
bool modeBit;
bool enterCVdata;
bool progFinished;
byte progStepCV, lastCV;
byte cv29, cv17, cv18;
unsigned int decoAddress;

enum lock {LOCK_SEL_LOCO, LOCK_TURNOUT, LOCK_PROG};
byte  lockOptions;

struct {
  byte time;                                                // Waiting time at stations
  unsigned long timer;
  byte moduleA;                                             // feedback contact station A
  byte inputA;
#ifdef USE_ECOS
  unsigned int statusA;
#else
  byte statusA;
#endif
  byte moduleB;                                             // feedback contact station B
  byte inputB;
#ifdef USE_ECOS
  unsigned int statusB;
#else
  byte statusB;
#endif
  byte speed;                                               // running speed
} Shuttle;

enum States {STATION_A, BRAKE_A, A_TO_B, TRAVELLING, B_TO_A, BRAKE_B, STATION_B, DEACTIVATED};
bool activeShuttle;                                         // Shuttle train active (Lanzadera)
bool shuttleConfig;                                         // Configuring shuttle
byte suttleCurrentState, optShuttle;
#ifdef USE_LOCONET
enum optionsShuttle {SET_SHUTT, CONTACT_A, CONTACT_B, TIME_SHUTT};
#endif
#if defined(USE_Z21) || defined(USE_XPRESSNET)
enum optionsShuttle {SET_SHUTT, MODULE_A, CONTACT_A, MODULE_B, CONTACT_B, TIME_SHUTT};
#endif
#ifdef USE_ECOS
enum optionsShuttle {SET_SHUTT, MODULE_A, CONTACT_A, MODULE_B, CONTACT_B, TIME_SHUTT};
#endif


////////////////////////////////////////////////////////////
// ***** AUTOMATION *****
////////////////////////////////////////////////////////////

#define SECTOR_SIZE         16                        // EEPROM Disk
#if defined(ESP8266)
#define FAT_SIZE            111
#define FAT_INI             0x00A0
#define EEPROM_SIZE         2048
#else
#define FAT_SIZE            58
#define FAT_INI             0x0020
#define EEPROM_SIZE         1024
#endif
#define DISK_INI            FAT_INI + FAT_SIZE
#define DISK_SIZE           FAT_SIZE * SECTOR_SIZE


#if ((DISK_INI + DISK_SIZE) > EEPROM_SIZE)
#error Disco demasiado grande - Disk too large
#endif

#define START_FILE          0x00
#define CONT_FILE           0x80
#define FREE_SECTOR         0xFF
#define END_SECTOR          0x00

#define EDIT_BUF_SECT       10                        // Edit buffer
#define EDIT_BUF_SIZE       EDIT_BUF_SECT * SECTOR_SIZE
#define AUTO_NAME_LNG       13

#define MAX_AUTO_SEQ        6                         // Max. autosequences started
#define AUTO_INTERVAL       100UL                     // Timer interval (100ms)     
#define AUTO_ACC_ON         2                         // accessory activation time accessory (200ms)
#define AUTO_ACC_OFF        4                         // time to wait after accessory deactivated for CDU (400ms)
#define AUTO_LOCO_CHG       3                         // time to wait after loco change to receive status (300ms)

#define OPC_AUTO_MASK       0xF0
#define OPC_TIME_MASK       0x0FFF
#define OPC_AUTO_NAME       0x00                      //  Sequence name     '0000NNNN 0ccccccc' ['0ccccccc 0ccccccc' ... '0ccccccc 0ccccccc']   0x0000..0x7FFF    Name
#define OPC_AUTO_LOCO_SEL   0x80                      //  Loco selection    '10LLLLLL LLLLLLLL'                                                 0x8001..0x8FFF    Loco        (1..4095)
#define OPC_AUTO_LOCO_SEL4  0x90                      //                                                                                        0x9000..0x9FFF                (4096..8191)
#define OPC_AUTO_LOCO_SEL8  0xA0                      //                                                                                        0xA000..0xA70F                (8192..9999)
#define OPC_AUTO_LOCO_OPT   0xB0                      //  Loco Speed        '10110000 0sssssss'                                                 0xB000..0xB07F    Speed       (0%..100%)
//                                                        Loco Function     '10110000 10Sfffff'                                                 0xB080..0xB09C    Function    (0..28)
//                                                        Loco Direction    '10110000 110000dd'                                                 0xB0C0..0xB0C3    Direction   (REV, FWD, CHG, ESTOP)
#define OPC_AUTO_DELAY      0xC0                      //  Delay (x100ms)    '1100tttt tttttttt'                                                 0xC000..0xCFFF    Delay       (0.1s..400s)       
#define OPC_AUTO_ACC        0xD0                      //  Accessory         '1101Paaa aaaaaaaa'                                                 0xD000..0xDFFF    Accessory   (1..2048)
#define OPC_AUTO_FBK        0xE0                      //  Feedback          '1110Saaa aaaaaaaa'                                                 0xE000..0xEFFF    Feedback    (1..2048 / 1.1..128.8 / 1.1..20.8 / 1.1..32.16)
#define OPC_AUTO_JUMP       0xF0                      //  Run sequence      '11110000 0sssssss'                                                 0xF000..0xF07E    Sequence    (0..126: FAT)
#define OPC_AUTO_LOOP       0x7F                      //  Run sequence      '11110000 01111111'                                                 0xF07F..0xF07F    Loop        
#define OPC_AUTO_END        0xFF                      //  End sequence      '11111111 11111111'                                                 0xFFFF            End

const unsigned int defaultOpcode[] {
  0xD000, 0xD800,                                           // OPC_AUTO_ACC
  0xE000, 0xE800,                                           // OPC_AUTO_FBK
#ifdef USE_ECOS
  0x83E8,                                                   // OPC_AUTO_LOCO_SEL. ID=1000
#else
  0x8003,                                                   // OPC_AUTO_LOCO_SEL
#endif
  0xB000,                                                   // OPC_AUTO_LOCO_OPT  speed
  0xB080, 0xB0A0,                                           //                    function
  0xB0C0, 0xB0C1, 0xB0C2, 0xB0C3,                           //                    direction
  0xC00A,                                                   // OPC_AUTO_DELAY
  0xF000,                                                   // OPC_AUTO_JUMP      sequence
  0xF07F,                                                   // OPC_AUTO_LOOP
};

struct autoSeq {
  byte myPosFAT;
  byte currPosFAT;
  byte currStep;
  byte opcode;
  byte param;
  unsigned int value;
};

struct autoSeq automation[MAX_AUTO_SEQ];

byte currAutomation;
unsigned long timerAutomation;


byte bufferEdit[EDIT_BUF_SIZE];
byte editPos;
char scrName[AUTO_NAME_LNG + 1];
byte scrNameLng;
byte fileCount;
bool isNewFile;
bool editingOpcode;
bool editingSequence;
bool editingJump;
byte editingPosition;
byte editingLoco;
byte oldOpcode;
byte oldParam;
bool changedEEPROM;

enum fifo {FIFO_CHECK, FIFO_OFF, FIFO_WAIT};

unsigned int accessoryFIFO[32];                                     // FIFO 32 elements. Hardcoded in functions.
unsigned int lastInFIFO;
unsigned int firstOutFIFO;
unsigned int cntFIFO;
unsigned int lastAccessory;
byte accessoryTimer;
byte stateFIFO;


////////////////////////////////////////////////////////////
// ***** PROGRAMA PRINCIPAL *****
////////////////////////////////////////////////////////////

void setup() {
#ifdef DEBUG
  delay(500);
  Serial.begin(115200);                                     // Debug messages, serial at 115200b
  DEBUG_MSG("\nPaco Mouse %s", VER_STR);
#endif
  initVariables();                                          // Estado inicial
#ifdef USE_LNWIRE
  beginLnWire();
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  Wire.begin();
#endif
  beginHID();                                               // initialize HID (Human Interface Device) OLED, keypad and encoder
#ifdef USE_LNWIFI
  beginLnWiFi();                                            // Initialize the Loconet wifi
#endif
#ifdef USE_XNWIRE
  oled.setFont(Arial_bold_14);
  printOtherTxt(TXT_CONNECT);
  beginXpressNet(miDireccionXpressnet);                     // Initialize the Xpressnet interface
#endif
#ifdef USE_XNWIFI
  beginXnWiFi();                                            // Initialize the Xpressnet wifi
#endif
#ifdef USE_Z21
  beginZ21();                                               // Initialize the Z21 wifi
#endif
#ifdef USE_ECOS
  beginECoS();                                              // Initialize the ECoS wifi
#endif
  infoLocomotora(myLoco.address);                           // busca informacion de la locomotora
}


void loop() {
#ifdef USE_LOCONET
  lnetProcess();                                            // Loconet comunications
#ifdef USE_PHONE
  doPhoneLine();
#endif
#endif
#ifdef USE_XPRESSNET
  xpressnetProcess();                                       // Xpressnet comunications
#endif
#ifdef USE_Z21
  z21Process();                                             // Z21 comunications
#endif
#ifdef USE_ECOS
  ECoSProcess();                                            // ECoS comunications
#endif
  hidProcess();                                             // HID: Human Interface Device
  if (activeShuttle)
    mainShuttle();                                          // Shuttle train
#ifdef USE_AUTOMATION
  processAutomation();
#endif
}


////////////////////////////////////////////////////////////
// ***** SOPORTE *****
////////////////////////////////////////////////////////////

void initVariables() {
  byte pos;
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  EEPROM.begin(EEPROM_SIZE);
#endif
  timeOLED = millis();
  keypad.setDebounceTime(20);
  encoderValue = 0;
  encoderMax = 2;
#if (CHANGE_DIR == SWITCH_3P)
  dirValue = readDirSwitch();
#endif
  scrOLED = SCR_LOGO;
  optOLED = OPT_SPEED;
  prgOLED = PRG_PTRK;
  cfgOLED = CFG_CONTR;
  estadoPuntos = 0;
#if  (FUNC_SIZE == BIG)
  scrFunc = FNC_0_4;
#endif
#if  (FUNC_SIZE == SMALL)
  scrFunc = FNC_0_9;
#endif
  scrSpeed = 0;
  myTurnout = 1;
  myPosTurnout = NO_MOVIDO;
  lastPosTurnout = DESVIADO;
  accEnviado = false;
  CVdata = 0;                                               // CV programming
  CVaddress = 0;
  modeProg = false;
  progFinished = false;
  progStepCV = PRG_IDLE;
  clockRate = 0;                                            // fast clock internal
  clockHour = 0;
  clockMin = 0;
  clockInterval = ONE_DAY;
  showBigClock = false;
  for (pos = 0; pos < LOCOS_IN_STACK; pos++)                // borra loco stack
    locoStack[pos] = 0;
#ifdef USE_ECOS
  pos = readEEPROM (EE_ADRH);                               // Ultima locomotora controlada
  myLocoID = readEEPROM (EE_ADRL) + (pos << 8);
  pushLoco(myLocoID);
#else
  myLoco.adr[1] = readEEPROM (EE_ADRH);                     // Ultima locomotora controlada
  myLoco.adr[0] = readEEPROM (EE_ADRL);
  checkLocoAddress();
  pushLoco(myLoco.address);
#endif
  pos = readEEPROM(EE_CONTRAST);
#ifdef USE_SSD1309
  cfgMenu[CFG_CONTR_VAL].value = (pos > 100) ? 100 : pos;
#else
  cfgMenu[CFG_CONTR_VAL].value = pos;                       // Configuracion menu
#endif
  stopMode = readEEPROM(EE_STOP_MODE);
  cfgMenu[CFG_STOP_VEL0].value = (stopMode == 0) ? 1 : 0;
  cfgMenu[CFG_STOP_EMERG].value = (stopMode != 0) ? 1 : 0;
  shuntingMode = (readEEPROM(EE_SHUNTING) > 0) ? true : false;
  cfgMenu[CFG_SHUNTING].value = (shuntingMode == true) ? 1 : 0;
  changeCfgOption = false;
  modeTurntable = false;                                    // Turntable options
  destTrack = 1;
#if defined(USE_XPRESSNET) || defined(USE_Z21)
  baseOffset = (readEEPROM(EE_ROCO) > 0) ? 4 : 0;
  cfgMenu[CFG_TT_ROCO].value = (baseOffset == 0) ? 0 : 1;
#endif
  typeTurntable = readEEPROM(EE_TT_TYPE);
  switch (typeTurntable) {
    case TT_7686_KB14:
      cfgMenu[CFG_TT_KB14].value = 1;
      cfgMenu[CFG_TT_KB15].value = 0;
      cfgMenu[CFG_TT_TCTRL].value = 0;
      baseAdrTurntable = 209;
      break;
    case TT_F6915:
      cfgMenu[CFG_TT_KB14].value = 0;
      cfgMenu[CFG_TT_KB15].value = 0;
      cfgMenu[CFG_TT_TCTRL].value = 1;
      baseAdrTurntable = 200;
      break;
    default:
      cfgMenu[CFG_TT_KB14].value = 0;
      cfgMenu[CFG_TT_KB15].value = 1;
      cfgMenu[CFG_TT_TCTRL].value = 0;
      baseAdrTurntable = 225;
      break;
  }
#ifdef USE_SET_TIME
#ifdef USE_LOCONET
  cfgMenu[CFG_SYNC].value = 0;
#endif
#endif
  lockOptions = readEEPROM(EE_LOCK);
  cfgMenu[CFG_LOCK_LOCO].value = (bitRead(lockOptions, LOCK_SEL_LOCO)) ? 1 : 0;
  cfgMenu[CFG_LOCK_TURNOUT].value = (bitRead(lockOptions, LOCK_TURNOUT)) ? 1 : 0;
  cfgMenu[CFG_LOCK_PROG].value = (bitRead(lockOptions, LOCK_PROG)) ? 1 : 0;
  activeShuttle = false;                                    // shuttle
  shuttleConfig = false;
  suttleCurrentState = DEACTIVATED;
  Shuttle.moduleA = readEEPROM (EE_MOD_A);
  Shuttle.inputA = readEEPROM (EE_CONT_A);
  Shuttle.moduleB = readEEPROM (EE_MOD_B);
  Shuttle.inputB = readEEPROM (EE_CONT_B);
  Shuttle.time = readEEPROM (EE_TIME);
#ifdef USE_AUTOMATION
  initAutomation();
  fileCount = getFileCount();
  fileOLED = 0;
#endif
#ifdef USE_LOCONET
  typeCmdStation = readEEPROM(EE_CMD_STA);
  cfgMenu[CFG_CMD_DR].value = (typeCmdStation == CMD_DR) ? 1 : 0;
  cfgMenu[CFG_CMD_ULI].value = (typeCmdStation == CMD_ULI) ? 1 : 0;
  cfgMenu[CFG_CMD_DIG].value = (typeCmdStation > CMD_ULI) ? 1 : 0;
  mySlot.num = 0;
  mySlot.trk = 0x01;                                        // Power on
  doDispatchGet = false;
  doDispatchPut = false;
  accStateReq = false;
  lnetProg = false;
  ulhiProg = UHLI_PRG_END;
#ifdef USE_PHONE
  for (pos = 0; pos < MAX_STATION; pos++)                   // lee listin telefonico
    phoneBook[pos] = readEEPROM(EE_PHONE_ME + pos);
  myStation = phoneBook[PHONE_ME] + PHONE_BASE;
  lastStationPickUp = 0;
  //phonePBX = 0;                                             // all RED
  doPhoneStatus = false;
  phonePhase = PHONE_IDLE;
#endif
#if defined(USE_LNWIFI)
  EEPROM.get (EE_WIFI, wifiSetting);
  if (wifiSetting.ok != 0x464D) {
    snprintf (wifiSetting.ssid, 32, "");
    snprintf (wifiSetting.password, 64, "12345678");
    wifiSetting.CS_IP = IPAddress(192, 168, 0, 111);
    wifiSetting.port = 1234;
    wifiSetting.type = true;
    wifiSetting.ok = 0x464D;
    EEPROM.put(EE_WIFI, wifiSetting);
    EEPROM.commit();
    //DEBUG_MSG("Set default WiFi");
  }
  cfgMenu[CFG_WIFI_PORT].value = wifiSetting.port;
  cfgMenu[CFG_BINARY].value = (wifiSetting.type == false) ? 1 : 0;
  cfgMenu[CFG_LBSERVER].value = (wifiSetting.type == true) ? 1 : 0;
  isLBServer = wifiSetting.type;
#endif
#endif
#ifdef USE_XPRESSNET
#if defined(USE_XNWIRE)
  miDireccionXpressnet = readEEPROM(EE_XBUS);
  if ((miDireccionXpressnet > 31) || (miDireccionXpressnet == 0)) {   // Si no es valido inicializa direccion bus en EEPROM
    miDireccionXpressnet = DEFAULT_XNET_ADR;
    updateEEPROM (EE_XBUS, miDireccionXpressnet);
  }
  cfgMenu[CFG_XBUS_DIR].value = miDireccionXpressnet;
#endif
#if defined(USE_XNWIFI)
  EEPROM.get (EE_WIFI, wifiSetting);
  if (wifiSetting.ok != 0x464D) {
    snprintf (wifiSetting.ssid, 32, "");
    snprintf (wifiSetting.password, 64, "12345678");
    wifiSetting.CS_IP = IPAddress(192, 168, 0, 200);
    wifiSetting.ok = 0x464D;
    EEPROM.put(EE_WIFI, wifiSetting);
    EEPROM.commit();
    //DEBUG_MSG("Set default WiFi");
  }
#endif
  mySteps = bit(2);                                         // default 128 steps
  csStatus = csNormalOps;                                   // Power on
  infoTimer = millis();
  getResultsSM = false;
  askMultimaus = false;
  highVerMM = 0;
  lowVerMM = 0;
#endif
#ifdef USE_Z21
  lowBATT = false;
  battery = ESP.getVcc ();                                  // Read VCC voltage
  infoTimer = millis();
  pingTimer = 0;
  waitResultCV = false;
  shortAddress = readEEPROM(EE_SHORT);
  if ((shortAddress != 99) && (shortAddress != 127))
    shortAddress = 99;
  cfgMenu[CFG_SHORT_99].value = (shortAddress == 99) ? 1 : 0;
  cfgMenu[CFG_SHORT_127].value = (shortAddress == 127) ? 1 : 0;
  EEPROM.get (EE_WIFI, wifiSetting);
  if (wifiSetting.ok != 0x464D) {
    snprintf (wifiSetting.ssid, 32, "");
    snprintf (wifiSetting.password, 64, "12345678");
    wifiSetting.CS_IP = IPAddress(192, 168, 0, 111);
    wifiSetting.ok = 0x464D;
    EEPROM.put(EE_WIFI, wifiSetting);
    EEPROM.commit();
    //DEBUG_MSG("Set default WiFi");
  }
#endif
#ifdef USE_ECOS
  lowBATT = false;
  battery = ESP.getVcc ();                                  // Read VCC voltage
  infoTimer = millis();
  timeout = millis();
  csStatus = 0;                                             // Stop
  typeCmdStation = T_ECOS;
  requestedCV = false;
  msgDecodePhase = MSG_WAIT;
  EEPROM.get (EE_WIFI, wifiSetting);
  if (wifiSetting.ok != 0x464D) {
    snprintf (wifiSetting.ssid, 32, "");
    snprintf (wifiSetting.password, 64, "12345678");
    wifiSetting.CS_IP = IPAddress(192, 168, 0, 100);
    wifiSetting.ok = 0x464D;
    EEPROM.put(EE_WIFI, wifiSetting);
    EEPROM.commit();
    //DEBUG_MSG("Set default WiFi");
  }
  clearRosterList();
  //myLocoID = 0;
#endif
}


void hidProcess() {
  if (updateOLED)                                           // Actualizar pantalla
    showOLED();
  if (millis() - timeOLED > timeoutOLED)                    // Fin tiempo espera para actualizar OLED
    updateOLED = true;
  if (encoderChange)                                        // se ha movido el encoder
    controlEncoder();
  if (switchOn)                                             // se ha pulsado el boton del encoder
    controlSwitch();
  if (keyOn)                                                // se ha pulsado una tecla
    controlKeypad();
#if (CHANGE_DIR == SWITCH_3P)
  if (dirChange)                                            // se ha movido el interruptor de direccion
    controlDirSwitch();
#endif
  if (millis() - timeButtons > timeoutButtons)              // lectura de boton
    readButtons();
  readKeypad();                                             // lectura de teclado
}


////////////////////////////////////////////////////////////
// ***** SOPORTE LOCOMOTORAS *****
////////////////////////////////////////////////////////////

void checkLocoAddress() {
  myLoco.address &= 0x3FFF;
  if ((myLoco.address > 9999) || (myLoco.address == 0))     // Comprueba que este entre 1 y 9999
    myLoco.address = 3;
  //DEBUG_MSG("Loco: %d", myLoco.address);
#ifdef USE_XPRESSNET
  if (myLoco.address > 99)                                  // Comprueba si es direccion larga
    myLoco.address |= 0xC000;
#endif
}



void updateSpeedHID() {
  byte spd, steps;
  switch (scrOLED) {
    case SCR_SPEED:
      updateOLED = true;
#ifdef USE_PHONE
    case SCR_PHONE:                                         // Control loco en llamada telefonica
#endif
    case SCR_SHUTTLE:                                       // Control loco en menu lanzadera
    case SCR_TURNTABLE:                                     // Control loco en menu plataforma giratoria
    case SCR_TURNOUT:                                       // Control loco en menu desvios
#ifdef USE_LOCONET
      if (mySpeed > 1) {
        steps = getMaxStepLnet();
        if (steps == 128) {
          encoderMax = 63;                                    // Max 100% speed (64 pasos, compatible con 14, 28 y 128 pasos)
          encoderValue = (mySpeed > 1) ? (mySpeed >> 1) : 0;  // 0..127 -> 0..63
        }
        else {
          if (steps == 28) {                                  // 0..31
            encoderMax = 31;
            spd = (((mySpeed - 2) << 1) / 9);
            encoderValue = spd + 4;
          }
          else {
            encoderMax = 15;
            spd = (mySpeed - 2) / 9;
            encoderValue = spd + 2;
          }
        }
      }
      else
        encoderValue = 0;
      //DEBUG_MSG("HID Enc:%d Spd:%d", encoderValue, mySpeed);
#endif
#if defined(USE_Z21) || defined(USE_XPRESSNET)
      if (bitRead(mySteps, 2)) {                            // 0..127 -> 0..63
        encoderMax = 63;
        encoderValue = (mySpeed > 1) ? (mySpeed >> 1) : 0;
      }
      else {
        if (bitRead(mySteps, 1)) {                          // 0..31
          encoderMax = 31;
          spd = (mySpeed & 0x0F) << 1;
          if (bitRead(mySpeed, 4))
            bitSet(spd, 0);;
          encoderValue = (spd > 3) ? spd : 0;
        }
        else {                                              // 0..15
          encoderMax = 15;
          spd = mySpeed & 0x0F;
          encoderValue = (spd > 1) ? spd : 0;
        }
      }
#endif
#ifdef  USE_ECOS
      encoderMax = 63;                                      // 0..127 -> 0..63
      encoderValue = (mySpeed > 1) ? (mySpeed >> 1) : 0;
#endif
      break;
  }
}


void updateMySpeed() {
#ifdef USE_LOCONET
  byte spd, steps;

  steps = getMaxStepLnet();
  if (steps == 128) {
    if ((encoderValue == 0) && shuntingMode)                  // Modo maniobras
      encoderValue = 1;
    mySpeed = (encoderValue << 1);                          // 0..63 -> 0..127
    if (encoderValue > 61)
      mySpeed++;
  }
  else {
    if (steps == 28) {
      encoderValue = shuntingSpeed (encoderValue, 4);
      if (encoderValue > 3) {
        spd = ((encoderValue - 3) * 9) + 1;                 // 0..31 -> 0..127
        mySpeed =  (spd >> 1) + 1;
      }
      else {
        switch (encoderValue) {
          case 0:
          case 3:
            mySpeed = 0;
            encoderValue = 0;
            break;
          case 1:
          case 2:
            mySpeed = 5;
            encoderValue = 4;
            break;
        }
      }
    }
    else {
      encoderValue = shuntingSpeed (encoderValue, 2);
      if (encoderValue == 1)
        return;
      mySpeed = (encoderValue > 1) ? ((encoderValue - 2) * 9) + 2 : 0;    // 0..15 -> 0..127
    }
  }
  //DEBUG_MSG("LN  Enc:%d Spd:%d", encoderValue, mySpeed);
#endif
#if defined(USE_Z21) || defined(USE_XPRESSNET)
  if (bitRead(mySteps, 2)) {                                // 0..63 -> 0..127
    if ((encoderValue == 0) && shuntingMode)                // comprueba Modo maniobras
      encoderValue = 1;
    mySpeed = encoderValue << 1;
  }
  else {
    if (bitRead(mySteps, 1)) {                              // 0..31 -> 0..31     '---43210' -> '---04321'
      encoderValue = shuntingSpeed (encoderValue, 4);
      if (encoderValue > 3) {
        mySpeed =  (encoderValue >> 1) & 0x0F;
        bitWrite(mySpeed, 4, bitRead(encoderValue, 0));
      }
      else {
        switch (encoderValue) {
          case 0:
          case 3:
            mySpeed = 0;
            encoderValue = 0;
            break;
          case 1:
          case 2:
            mySpeed = 2;
            encoderValue = 4;
            break;
        }
      }
    }
    else {
      encoderValue = shuntingSpeed (encoderValue, 2);
      mySpeed = (encoderValue > 1) ? encoderValue : 0;      // 0..15 -> 0..15
    }
  }
#endif
#ifdef USE_ECOS
  if ((encoderValue == 0) && shuntingMode)                  // Modo maniobras
    encoderValue = 1;
  mySpeed = (encoderValue << 1);                          // 0..63 -> 0..127
  if (encoderValue > 61)
    mySpeed++;
#endif
  locoOperationSpeed();
}


byte shuntingSpeed (byte encoder, byte stepMin) {           // comprueba Modo maniobras
  if (encoder < stepMin) {
    if (shuntingMode)
      return stepMin;
  }
  return encoder;
}


void pushLoco(unsigned int loco) {                          // mete locomotora en el stack
  byte pos;
  unsigned int adr;
  for (pos = 0; pos < LOCOS_IN_STACK; pos++) {              // busca loco en stack
    if (locoStack[pos] == loco)
      locoStack[pos] = 0;                                   // evita que se repita
  }
  pos = 0;
  do {
    adr = locoStack[pos];                                   // push the loco in stack
    locoStack[pos] = loco;
    loco = adr;
    pos++;
  } while ((adr > 0) && (pos < LOCOS_IN_STACK));
  /*
    #ifdef DEBUG
    Serial.print(F("STACK: "));
    for (pos = 0; pos < LOCOS_IN_STACK; pos++) {
      Serial.print(locoStack[pos]);
      Serial.print(' ');
    }
    Serial.println();
    #endif
  */
}


void scrLocoGet() {
#ifdef USE_ECOS
  byte pos, n;
  if (scrLoco > 0) {
    pos = LOCOS_IN_STACK;
    for (n = 0; n < LOCOS_IN_STACK; n++)                    // search ID in roster list
      if (rosterList[n].id == scrLoco)
        pos = n;
    if (pos != LOCOS_IN_STACK) {
      releaseLoco();                                        // release old loco ID
      myLoco.address = rosterList[pos].locoAddress;
      myLocoID = scrLoco;
      pushLoco(myLocoID);
      infoLocomotora(myLocoID);
      updateEEPROM (EE_ADRH, highByte(myLocoID));                  // guarda nueva direccion en EEPROM
      updateEEPROM (EE_ADRL, lowByte(myLocoID));
      optOLED = OPT_SPEED;
      enterMenuOption();
    }
  }
#else
  if (scrLoco != 0) {
    activeShuttle = false;
    myLoco.address = scrLoco;                               // seleccionada nueva loco
    checkLocoAddress();
    pushLoco(myLoco.address);
    updateEEPROM (EE_ADRH, myLoco.adr[1]);                  // guarda nueva direccion en EEPROM
    updateEEPROM (EE_ADRL, myLoco.adr[0]);
    optOLED = OPT_SPEED;
    enterMenuOption();
#ifdef USE_LOCONET
    doDispatchGet = false;
    doDispatchPut = false;
    liberaSlot();                                           // pasa slot actual a COMMON
    /*
      if (typeCmdStation == CMD_DR)                           // For Intellibox II
      sendLocoIB();
    */
    infoLocomotora(myLoco.address);                         // busca info nueva loco
#endif
#if defined(USE_Z21) || defined(USE_XPRESSNET)
    infoLocomotora(myLoco.address);                         // busca info nueva loco
#endif
  }
#endif
}


#ifdef USE_LOCONET
void selectDispatch() {
  switch (encoderValue) {
    case 0:                                             // Dispatch Get
      showWait();
      dispatchGet();
      break;
    case 1:
      dispatchPut();                                    // Dispatch put
      if (mySlot.num == 0) {
        optOLED = OPT_LOCO;
        enterMenuOption();
      }
      break;
#ifdef USE_PHONE
    case 2:
      encoderMax = 3;                                   // Select station to call
      encoderValue = 0;
      scrOLED = SCR_STATION;
      updateOLED = true;
      break;
#endif
  }
}
#endif



////////////////////////////////////////////////////////////
// ***** SOPORTE MENU *****
////////////////////////////////////////////////////////////

bool notLocked () {                                         // check if not locked
  if (lockOptions & ((1 << LOCK_SEL_LOCO) | (1 << LOCK_TURNOUT) | (1 << LOCK_PROG)))
    return false;
  else
    return true;
}


bool notLockedOption (byte opt) {                           // check if option not locked
  if (lockOptions & (1 << opt))
    return false;
  else
    return true;
}


void enterMenuOption() {
  switch (optOLED) {                                        // entra en la opcion del menu actual
    case OPT_SPEED:
      scrOLED = SCR_SPEED;
      updateExtraOLED = true;
      controlLocoOption();
      break;
    case OPT_LOCO:
      if (notLockedOption (LOCK_SEL_LOCO)) {
        encoderMax = LOCOS_IN_STACK - 1;
        encoderValue = 0;
        scrOLED = SCR_LOCO;
        clearInputDigit();
        scrLoco = 0;
        updateAllOLED = true;
        updateOLED = true;
      }
      break;
#ifdef USE_LOCONET
    case OPT_DISPATCH:
      if (notLockedOption (LOCK_SEL_LOCO)) {
#ifdef USE_PHONE
        encoderMax = (phoneBook[PHONE_ME] > 0) ? 2 : 1;    // Select GET/PUT or GET/PUT/Station if can call
#else
        encoderMax = 1;
#endif
        encoderValue = 0;
        scrOLED = SCR_DISPATCH;
        updateOLED = true;
      }
      break;
#endif
    case OPT_TURNOUT:
      if (notLockedOption (LOCK_TURNOUT)) {
        myPosTurnout = NO_MOVIDO;
        scrOLED = SCR_TURNOUT;
        updateExtraOLED = true;
        clearInputDigit();
        if (myTurnout > 0) {
          infoDesvio(myTurnout);                            // busca la posicion del desvio
        }
        controlLocoOption();
        updateOLED = true;
      }
      break;
    case OPT_CFG:
      encoderMax = MAX_CFG_ITEMS - 1;
      encoderValue = 1;
      changeCfgOption = false;
      scrOLED = SCR_CFG;
      cfgOLED = CFG_CONTR;
      updateOLED = true;
      break;
    case OPT_TURNTABLE:
      if (notLockedOption (LOCK_TURNOUT)) {
        scrOLED = SCR_TURNTABLE;
        updateOLED = true;
        clearInputDigit();
        controlLocoOption();
      }
      break;
    case OPT_CV:
      if (notLockedOption (LOCK_PROG)) {
        prgOLED = PRG_PTRK;
        showPrgMenu();
      }
      break;
    case OPT_SHUTTLE:
      scrOLED = SCR_SHUTTLE;
      updateOLED = true;
      controlLocoOption();
      break;
#ifdef USE_AUTOMATION
    case OPT_AUTOMATION:
      if (notLocked()) {
        encoderMax = (fileCount > 0) ? fileCount - 1 : 0;
        encoderValue = fileOLED;
        editingOpcode = false;
        editingSequence = false;
        editingJump = false;
#ifdef USE_ECOS
        editingLoco = false;
#endif
        scrOLED = SCR_AUTO_SELECT;
        updateAllOLED = true;
        updateOLED = true;
      }
      break;
#endif
  }
}



void enterPrgOption() {
  modeProg = false;
  modeBit = false;
#ifdef USE_LOCONET
  lnetProg = true;
#endif
  switch (prgOLED) {                                        // entra en la opcion del menu de programacion actual
    case PRG_BIT:
      modeBit = true;
    case PRG_PTRK:                                          // Programar en via programacion
      //modeProg = false;
      enterCVdata = false;
      clearInputDigit();
      scrOLED = SCR_CV;
      updateAllOLED = true;
      updateOLED = true;
      break;
    case PRG_MTRK:                                          // Programar en via principal
      modeProg = true;
      enterCVdata = false;
      clearInputDigit();
      scrOLED = SCR_CV;
      updateAllOLED = true;
      updateOLED = true;
      break;
    case PRG_DIR:                                           // Programar direccion locomotora
      showWait();
      clearInputDigit();
      //modeProg = false;
      CVaddress = 29;
      readCV(29, PRG_RD_CV29);
      break;
    case PRG_CV2:                                           // Programar CV basicos
      readPrgCV(2);
      break;
    case PRG_CV3:
      readPrgCV(3);
      break;
    case PRG_CV4:
      readPrgCV(4);
      break;
    case PRG_CV5:
      readPrgCV(5);
      break;
    case PRG_CV6:
      readPrgCV(6);
      break;
    case PRG_FAB:
      readPrgCV(8);
      break;
    case PRG_CV29:
      readPrgCV(29);
      break;
#ifdef USE_LNCV
    case PRG_LNCV:
      artNum = 0;
      modNum = 1;
      numLNCV = 0;
      valLNCV = 1;
      optLNCV = LNCV_ART;
      clearInputDigit();
      scrOLED = SCR_LNCV;
      updateOLED = true;
      break;
#endif
  }
}


void controlLocoOption() {
  updateSpeedHID();
  updateAllOLED = true;
}



#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
void enterCfgWiFi (byte scr) {
  byte n;
  switch (scr) {
    case CFG_WIFI_SSID:
      oled.clear();                                       // buscar SSID WiFi
      oled.setFont(Arial_bold_14);
      oled.print(F("SSID WiFi"));

      networks = 0;
      while (networks == 0) {

        oled.setCursor(0, 4);
        printOtherTxt(TXT_SEARCH);
        WiFi.scanDelete();
        networks = WiFi.scanNetworks();
        DEBUG_MSG("Networks: %d", networks);
        if (networks > 0) {
          scrOLED = SCR_SSID;
          encoderMax = networks - 1;
          encoderValue = 0;
          scrSSID = 0;
        }
        else {
          oled.setCursor(0, 4);
          oled.clearToEOL();
          if (networks == 0)
            oled.print(F("No SSID"));
          else
            oled.print(F("????"));
          delay(2000);
          networks = 0;
          continue;
        }

      }
      updateOLED = true;
      break;
    case CFG_WIFI_PWD:
      scrOLED = SCR_PASSWORD;
      encoderMax = 95;
      encoderValue = 0;
      strcpy(scrPwd, wifiSetting.password);
      scrPwdLng = strlen(scrPwd);
      updateAllOLED = true;
      updateOLED = true;
      DEBUG_MSG("CFG: %d %s", scrPwdLng, scrPwd);
      break;
    case CFG_WIFI_IP:
      scrOLED = SCR_IP_ADDR;
      scrIP[0] = wifiSetting.CS_IP[0];
      scrIP[1] = wifiSetting.CS_IP[1];
      scrIP[2] = wifiSetting.CS_IP[2];
      scrIP[3] = wifiSetting.CS_IP[3];
      scrPosIP = 0;
      encoderMax = 255;
      encoderValue = scrIP[0];
      updateAllOLED = true;
      updateOLED = true;
      break;
#if defined(USE_LNWIFI)
    case CFG_WIFI_PORT:
      scrOLED = SCR_PORT;
      encoderMax = 9;
      encoderValue = 0;
      clearInputDigit();
      scrPort = wifiSetting.port;
      updateAllOLED = true;
      updateOLED = true;
      break;
#endif
  }
}
#endif



void enterCfgOption (byte numOpt) {                         // entra nuevo valor configuracion
  encoderValue = 0;
  switch (numOpt) {
    case CFG_CONTR:
      cfgOLED = CFG_CONTR_VAL;                              // line
#ifdef USE_SSD1309
      encoderMax = 100;                                     // max value
#else
      encoderMax = 255;                                     // max value
#endif
      encoderValue = cfgMenu[CFG_CONTR_VAL].value;          // curr. value
      break;
    case CFG_STOP_MODE:
      cfgOLED = CFG_STOP_VEL0;
      encoderMax = 2;
      break;
#ifdef USE_LOCONET
    case CFG_CMD_STA:
      cfgOLED = CFG_CMD_DR;
      encoderMax = 2;
      break;
#endif
#ifdef USE_XNWIRE
    case CFG_XBUS:
      cfgOLED = CFG_XBUS_DIR;
      encoderMax = 31;
      encoderValue = cfgMenu[CFG_XBUS_DIR].value;
      break;
#endif
    case CFG_TT:
      cfgOLED = CFG_TT_KB14;
      encoderMax = 3;
      break;
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
    case CFG_WIFI:
      cfgOLED = CFG_WIFI_SSID;
      encoderMax = 2;
      break;
#endif
#if defined(USE_LNWIFI)
    case CFG_SERVER:
      cfgOLED = CFG_WIFI_PORT;
      encoderMax = 2;
      break;
#endif
#ifdef USE_Z21
    case CFG_SHORT:
      cfgOLED = CFG_SHORT_99;
      encoderMax = 1;
      break;
#endif
#ifdef USE_SET_TIME
    case CFG_TIME:
      cfgOLED = CFG_SET_TIME;
#ifdef USE_LOCONET
      encoderMax = 1;
#else
      encoderMax = 0;
#endif
      break;
#endif
    case CFG_LOCK:
      cfgOLED = CFG_LOCK_LOCO;
      encoderMax = 2;
      break;
#ifdef GAME_EXTRA
    case CFG_GAME:
      cfgOLED = CFG_GAME_TIC;
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
      encoderMax = 4;
#else
      encoderMax = 3;
#endif
      break;
#endif
#ifdef USE_PHONE
    case CFG_PHONE:
      cfgOLED = CFG_PHONE_ME;
      encoderMax = 1;
      break;
#endif
  }
}



////////////////////////////////////////////////////////////
// ***** SOPORTE TECLADO Y ENCODER *****
////////////////////////////////////////////////////////////

void clearInputDigit() {
  diezMiles = 0;
  myInput.packed = 0;
}

void inputDigit() {
  myInput.packed = (myInput.packed << 8) + ((keyValue - K_NUM0) & 0x0F);
}


unsigned int calcInputValue() {
  return ((myInput.digit[THOUSANDS] * 1000) + (myInput.digit[CENTS] * 100) + (myInput.digit[TENS] * 10) + myInput.digit[UNITS]);
}



#ifdef USE_PHONE
void enterPhoneNumber() {
  cfgMenu[CFG_PHONE].value = encoderValue;
  encoderValue = phoneBook[encoderValue + 1];  // get current phone number
  encoderMax = 99;
  scrOLED = SCR_PHONE_NUM;
  updateOLED = true;
}
#endif


#ifdef USE_LNCV
void input5Digit() {
  diezMiles = myInput.digit[THOUSANDS];
  inputDigit();
}


unsigned int calc5InputValue() {
  unsigned int total;
  total = calcInputValue();                     // calc 4 last numbers
  if (diezMiles > 6)
    total = 0;
  else {
    if ((diezMiles == 6) && (total > 5535))
      total = 0;
    else
      total = (diezMiles * 10000) + total;
  }
  if (total == 0) {
    clearInputDigit();
  }
  return (total);
}
#endif


unsigned int maxInputValue(unsigned int maxValue) {
  unsigned int value;
  value = calcInputValue();
  if (value > maxValue) {
    value = 0;
    clearInputDigit();
  }
  return value;
}


////////////////////////////////////////////////////////////
// ***** CV PROGRAMMING *****
////////////////////////////////////////////////////////////

void endProg() {                                            // Fin de programcion/lectura CV
  if (CVdata > 255) {
    if (progStepCV == PRG_RD_CV29)                          // Si buscaba direccion, muestra CV1 en lugar de CV29
      CVaddress = 1;
    showDataCV();
  }
  else {
    switch (progStepCV) {
      case PRG_CV:
        showDataCV();
        break;
      case PRG_RD_CV29:
        cv29 = (byte) CVdata;
        if (bitRead(cv29, 5)) {
          CVaddress = 17;                                   // Long address
          readCV(CVaddress, PRG_RD_CV17);
        }
        else {
          CVaddress = 1;                                    // Short address
          readCV(CVaddress, PRG_RD_CV1);
        }
        break;
      case PRG_RD_CV1:
        decoAddress = CVdata;
        showDirCV();
        break;
      case PRG_RD_CV17:
        cv17 = (byte) CVdata;
        CVaddress = 18;
        readCV(CVaddress, PRG_RD_CV18);
        break;
      case PRG_RD_CV18:
        cv18 = (byte) CVdata;
        decoAddress = ((cv17 & 0x3F) << 8) | cv18;
        showDirCV();
        break;
      case PRG_WR_CV17:                                     // Long address
        CVaddress = 18;
        writeCV(CVaddress, lowByte(decoAddress), PRG_WR_CV18);
        break;
      case PRG_WR_CV18:
        bitSet(cv29, 5);
        CVaddress = 29;
        writeCV(CVaddress, cv29, PRG_WR_CV29);
        break;
      case PRG_WR_CV1:                                      // short address
        bitClear(cv29, 5);
        CVaddress = 29;
        writeCV(CVaddress, cv29, PRG_WR_CV29);
        break;
      case PRG_WR_CV29:
        showDirCV();
        break;
    }
  }
}


void readPrgCV(unsigned int num) {                          // Lee una CV generica
  CVaddress = num;
  //modeProg = false;
  showWait();
  readCV(num, PRG_CV);
}


void showDataCV() {                                         // muestra valor de la CV
  progStepCV = PRG_IDLE;
  scrOLED = SCR_CV;
  enterCVdata = true;
  updateAllOLED = true;
  updateOLED = true;
#if defined(USE_LOCONET) && defined (USE_PRG_UHLI)
  progUhli(UHLI_PRG_END);
#endif
}


void showDirCV() {                                          // muestra direccion de la locomotora segun sus CV
  progStepCV = PRG_IDLE;
  scrOLED = SCR_DIRCV;
  enterCVdata = true;
  updateAllOLED = true;
  updateOLED = true;
#if defined(USE_LOCONET) && defined (USE_PRG_UHLI)
  progUhli(UHLI_PRG_END);
#endif
#ifndef USE_ECOS
  pushLoco(decoAddress);                                    // mete esta loco en el stack
#endif
}


////////////////////////////////////////////////////////////
// ***** EEPROM *****
////////////////////////////////////////////////////////////


byte readEEPROM(int adr) {
  byte data;
  data = EEPROM.read(adr);
  return data;
}

void updateEEPROM (int adr, byte data) {
#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
  EEPROM.update(adr, data);
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  if (EEPROM.read(adr) != data) {
    EEPROM.write (adr, data);
    EEPROM.commit();
  }
#endif
}

#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
void saveSSID (byte select) {                               // guardar nuevo SSID
  snprintf (wifiSetting.ssid, 32, WiFi.SSID(select).c_str());
  EEPROM.put(EE_WIFI, wifiSetting);
  EEPROM.commit();
  //DEBUG_MSG("New SSID: %s", wifiSetting.ssid);
}

void savePassword() {
  snprintf (wifiSetting.password, 64, "%s", scrPwd);
  EEPROM.put(EE_WIFI, wifiSetting);
  EEPROM.commit();
  //DEBUG_MSG("New password: %s", wifiSetting.password);
}

void saveIP() {
  wifiSetting.CS_IP[0] = scrIP[0];
  wifiSetting.CS_IP[1] = scrIP[1];
  wifiSetting.CS_IP[2] = scrIP[2];
  wifiSetting.CS_IP[3] = scrIP[3];
  EEPROM.put(EE_WIFI, wifiSetting);
  EEPROM.commit();
  //DEBUG_MSG("New IP: %d.%d.%d.%d", scrIP[0], scrIP[1], scrIP[2], scrIP[3]);
}
#if defined(USE_LNWIFI)
void saveServer() {
  EEPROM.put(EE_WIFI, wifiSetting);
  EEPROM.commit();
  DEBUG_MSG("New Server port: %d Type: %s", wifiSetting.port, wifiSetting.type ? "LBServer" : "Binary");
}
#endif
#endif


void saveCfgOption() {
  if (changeCfgOption) {                                    // guarda nueva configuracion
    switch (cfgOLED) {
      case CFG_CONTR_VAL:
        cfgMenu[CFG_CONTR_VAL].value = encoderValue;
        updateEEPROM(EE_CONTRAST, cfgMenu[CFG_CONTR_VAL].value);
        break;
      case CFG_STOP_VEL0:
      case CFG_STOP_EMERG:
        stopMode = cfgOLED - CFG_STOP_VEL0;
        updateEEPROM(EE_STOP_MODE, stopMode);
        cfgMenu[CFG_STOP_VEL0].value = (stopMode == 0) ? 1 : 0;
        cfgMenu[CFG_STOP_EMERG].value = (stopMode != 0) ? 1 : 0;
        break;
      case CFG_SHUNTING:
        shuntingMode = !shuntingMode;
        updateEEPROM(EE_SHUNTING, (byte) shuntingMode);
        cfgMenu[CFG_SHUNTING].value = (shuntingMode == true) ? 1 : 0;
        break;
#ifdef USE_LOCONET
      case CFG_CMD_DR:
      case CFG_CMD_ULI:
      case CFG_CMD_DIG:
        typeCmdStation = cfgOLED - CFG_CMD_DR;
        updateEEPROM(EE_CMD_STA, typeCmdStation);
        cfgMenu[CFG_CMD_DR].value = (typeCmdStation == CMD_DR) ? 1 : 0;
        cfgMenu[CFG_CMD_ULI].value = (typeCmdStation == CMD_ULI) ? 1 : 0;
        cfgMenu[CFG_CMD_DIG].value = (typeCmdStation > CMD_ULI) ? 1 : 0;
        break;
#endif
#ifdef USE_XNWIRE
      case CFG_XBUS_DIR:
        if (encoderValue > 0) {
          cfgMenu[CFG_XBUS_DIR].value = encoderValue;
          updateEEPROM(EE_XBUS, cfgMenu[CFG_XBUS_DIR].value);
        }
        else {
          cfgMenu[CFG_XBUS_DIR].value = miDireccionXpressnet;
        }
        break;
#endif
#if defined(USE_XPRESSNET) || defined(USE_Z21)
      case CFG_TT_ROCO:
        if (baseOffset > 0)
          baseOffset = 0;
        else
          baseOffset = 4;
        updateEEPROM(EE_ROCO, (byte) baseOffset);
        cfgMenu[CFG_TT_ROCO].value = (baseOffset == 0) ? 0 : 1;
        break;
#endif
      case CFG_TT_KB14:
      case CFG_TT_KB15:
      case CFG_TT_TCTRL:
        typeTurntable = cfgOLED - CFG_TT_KB14;
        updateEEPROM(EE_TT_TYPE, typeTurntable);
        cfgMenu[CFG_TT_KB14].value = (typeTurntable == 0) ? 1 : 0;
        cfgMenu[CFG_TT_KB15].value = (typeTurntable == 1) ? 1 : 0;
        cfgMenu[CFG_TT_TCTRL].value = (typeTurntable > 1) ? 1 : 0;
        switch (typeTurntable) {
          case TT_7686_KB14:
            baseAdrTurntable = 209;
            break;
          case TT_F6915:
            baseAdrTurntable = 200;
            break;
          default:
            baseAdrTurntable = 225;
            break;
        }
        break;
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
      case CFG_WIFI_SSID:
        enterCfgWiFi(CFG_WIFI_SSID);
        return;
        break;
      case CFG_WIFI_PWD:
        enterCfgWiFi(CFG_WIFI_PWD);
        return;
        break;
      case CFG_WIFI_IP:
        enterCfgWiFi(CFG_WIFI_IP);
        return;
        break;
#endif
#if defined(USE_LNWIFI)
      //CFG_SERVER, CFG_WIFI_PORT, CFG_LBSERVER, CFG_BINARY,
      case CFG_WIFI_PORT:
        enterCfgWiFi(CFG_WIFI_PORT);
        return;
        break;
      case CFG_BINARY:
      case CFG_LBSERVER:
        wifiSetting.type = cfgOLED - CFG_BINARY;
        saveServer();
        isLBServer = wifiSetting.type;
        cfgMenu[CFG_BINARY].value = (wifiSetting.type == false) ? 1 : 0;
        cfgMenu[CFG_LBSERVER].value = (wifiSetting.type == true) ? 1 : 0;
        break;
#endif
#ifdef USE_Z21
      case CFG_SHORT_99:
      case CFG_SHORT_127:
        if (cfgOLED == CFG_SHORT_99)
          shortAddress = 99;
        else
          shortAddress = 127;
        updateEEPROM(EE_SHORT, shortAddress);
        cfgMenu[CFG_SHORT_99].value = (shortAddress == 99) ? 1 : 0;
        cfgMenu[CFG_SHORT_127].value = (shortAddress == 127) ? 1 : 0;
        break;
#endif
#ifdef  USE_SET_TIME
      case CFG_SET_TIME:
        scrHour = clockHour;
        scrMin = clockMin;
        scrRate = clockRate;
        scrPosTime = 0;
        encoderValue = clockHour;
        encoderMax = 23;
        scrOLED = SCR_TIME;
        updateAllOLED = true;
        updateOLED = true;
        return;
        break;
#ifdef USE_LOCONET
      case CFG_SYNC:
        if (cfgMenu[CFG_SYNC].value > 0)
          cfgMenu[CFG_SYNC].value = 0;
        else {
          cfgMenu[CFG_SYNC].value = 1;
          sendSYNC();
        }
        break;
#endif
#endif
      case CFG_LOCK_LOCO:
      case CFG_LOCK_TURNOUT:
      case CFG_LOCK_PROG:
        if (bitRead(lockOptions, cfgOLED - CFG_LOCK_LOCO))
          bitClear(lockOptions, cfgOLED - CFG_LOCK_LOCO);
        else
          bitSet(lockOptions, cfgOLED - CFG_LOCK_LOCO);
        updateEEPROM(EE_LOCK, lockOptions);
        cfgMenu[CFG_LOCK_LOCO].value = (bitRead(lockOptions, LOCK_SEL_LOCO)) ? 1 : 0;
        cfgMenu[CFG_LOCK_TURNOUT].value = (bitRead(lockOptions, LOCK_TURNOUT)) ? 1 : 0;
        cfgMenu[CFG_LOCK_PROG].value = (bitRead(lockOptions, LOCK_PROG)) ? 1 : 0;
        break;
#ifdef GAME_EXTRA
      case CFG_GAME_TIC:
        initGameTic();
        scrOLED = SCR_GAME;
        updateOLED = true;
        break;
      case CFG_GAME_SNAKE:
        initGameSnake();
        scrOLED = SCR_GAME;
        updateOLED = true;
        break;
      case CFG_GAME_FLAPPY:
        initGameFlappy();
        scrOLED = SCR_GAME;
        updateOLED = true;
        break;
      case CFG_GAME_PAKU:
        initGamePaku();
        scrOLED = SCR_GAME;
        updateOLED = true;
        break;
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
      case CFG_GAME_PONG:
        initGamePong();
        scrOLED = SCR_GAME;
        updateOLED = true;
        break;
#endif
#endif
#ifdef USE_PHONE
      case CFG_PHONE_ME:
        cfgMenu[CFG_PHONE].value = MAX_STATION;           // set my station number
        encoderValue = phoneBook[PHONE_ME];               // get current phone number
        encoderMax = 99;                                  //
        scrOLED = SCR_PHONE_NUM;
        updateOLED = true;
        return;
        break;
      case CFG_PHONE_STAT:
        encoderMax = 3;                                   // Select station to call
        encoderValue = 0;                                 //
        scrOLED = SCR_PHONEBOOK;
        updateOLED = true;
        return;
        break;
#endif
    }
    changeCfgOption = false;
    encoderMax = MAX_CFG_ITEMS - 1;
    encoderValue = cfgOLED;
    updateOLED = true;
  }
  else {
    if (cfgMenu[cfgOLED].type == TCFG_OPT) {
      if (notLocked() || (cfgOLED == CFG_LOCK)) {
        changeCfgOption = true;
        enterCfgOption(cfgOLED);
        updateOLED = true;
      }
    }
  }
}


#ifdef USE_PHONE
void savePhoneNumber() {
  byte n;
  n = cfgMenu[CFG_PHONE].value;
  n = (n == MAX_STATION) ? 0 : n + 1;                     // save in phonebook & EEPROM
  phoneBook[n] = encoderValue;
  updateEEPROM(EE_PHONE_ME + n, phoneBook[n]);
  myStation = phoneBook[PHONE_ME] + PHONE_BASE;
  showMenu();                                             // return to menu
  updateOLED = true;
}
#endif



////////////////////////////////////////////////////////////
// ***** SHUTTLE *****
////////////////////////////////////////////////////////////


void checkAtStation (byte state) {
  if (millis() - Shuttle.timer > (Shuttle.time << 9)) {
    myDir ^= 0x80;                                          // if enough time to stop passed, change direction
#ifdef USE_LOCONET
    changeDirectionF0F4();
#endif
#ifdef USE_XPRESSNET
    locoOperationSpeed();
#endif
#ifdef USE_Z21
    locoOperationSpeed();
#endif
#ifdef USE_ECOS
    changeDirection();
#endif
    suttleCurrentState = state;
    Shuttle.timer = millis();
    if (scrOLED == SCR_SHUTTLE)
      updateOLED = true;
  }
}


void checkDeparture (byte state) {
  if (millis() - Shuttle.timer > (Shuttle.time << 9)) {
    mySpeed = Shuttle.speed;                              // if enough time at station, start travel
    locoOperationSpeed();
    suttleCurrentState = state;
    if (scrOLED == SCR_SHUTTLE)
      updateOLED = true;
  }
}

#ifdef  USE_ECOS
void checkArrivingStation (byte input, unsigned int stat, byte state) {
#else
void checkArrivingStation (byte input, byte stat, byte state) {
#endif
  bool activated;
  activated = false;
#ifdef USE_LOCONET
  if (stat > 0)
    activated = true;
#endif
#if defined(USE_Z21) || defined(USE_XPRESSNET) || defined(USE_ECOS)
  if (input > 0)
    if (stat & (1 << (input - 1)))
      activated = true;
#endif
  if (activated) {
    Shuttle.speed = mySpeed;                            // if arriving station, brake
    mySpeed = 0;
    locoOperationSpeed();
    suttleCurrentState = state;
    Shuttle.timer = millis();
    if (scrOLED == SCR_SHUTTLE)
      updateOLED = true;
  }
}


void mainShuttle() {
  switch (suttleCurrentState) {
    case TRAVELLING:                                    // check both stations
      checkArrivingStation (Shuttle.inputA, Shuttle.statusA, BRAKE_A);
    case A_TO_B:
      checkArrivingStation (Shuttle.inputB, Shuttle.statusB, BRAKE_B);
      break;
    case B_TO_A:
      checkArrivingStation (Shuttle.inputA, Shuttle.statusA, BRAKE_A);
      break;
    case BRAKE_A:
      checkAtStation (STATION_A);
      break;
    case BRAKE_B:
      checkAtStation (STATION_B);
      break;
    case STATION_A:
      checkDeparture (A_TO_B);
      break;
    case STATION_B:
      checkDeparture (B_TO_A);
      break;
  }
}

////////////////////////////////////////////////////////////
// ***** PHONEBOOK *****
////////////////////////////////////////////////////////////

#ifdef  USE_PHONE
void doPhoneLine() {
  switch (phonePhase) {
    case PHONE_IDLE:                              // No comunication. Phone hunged, answer call through OPC_SW_REP notifySwitchReport()
    case PHONE_CHECK:                             // wait destinaton phone status through OPC_SW_REP notifySwitchReport() / OPC_LONG_ACK
    case WAIT_ANSWER:                             // wait answer through OPC_SW_REQ notifySwitchRequest()
    case PHONE_BUSY:                              // destination busy. Phone picked up.
    case WAIT_USER:                               // wait key to accept or reject call
    case WAIT_TRAIN:                              // wait hung my phone to get train through OPC_SW_REQ notifySwitchRequest()
      break;
    case PHONE_CALLING:
      lnetRequestSwitch(myStation , 1, RECTO - DESVIADO);         // pick up my phone. GREEN
      lnetRequestSwitch(myStation , 0, RECTO - DESVIADO);
      lnetRequestSwitch(callingStation, 1, RECTO - DESVIADO);     // call station. GREEN
      lnetRequestSwitch(callingStation, 0, RECTO - DESVIADO);
      phonePhase = WAIT_ANSWER;
      break;
    case CALL_ACCEPTED:
      dispatchPut();
      lnetRequestSwitch(callingStation, 1, DESVIADO - DESVIADO);  // transfer & hung. RED
      lnetRequestSwitch(callingStation, 0, DESVIADO - DESVIADO);
      phoneCallEnd();
      optOLED = OPT_LOCO;
      enterMenuOption();
      break;
    case CALL_REJECTED:
      lnetRequestSwitch(myStation , 1, DESVIADO - DESVIADO);      // hung my phone. RED
      lnetRequestSwitch(myStation , 0, DESVIADO - DESVIADO);
      phoneCallEnd();
      updateOLED = true;                          // "Stop Train"
      break;
    case PHONE_RING:
      phonePhase = WAIT_USER;
      scrOLED = SCR_PHONE;
      updateSpeedHID();                           // Control the speed
      updateOLED = true;
      updateAllOLED = true;                       // draw Big phone & hang
      break;
    case GET_TRAIN:                               // ask for my train
      dispatchGet();
      phoneCallEnd();
      break;
  }
}


void phoneCallEnd() {
  phonePhase = PHONE_IDLE;                    // End of call
  lastStationPickUp = 0;
}


void callToStation() {
  byte posPBX;
  posPBX = encoderValue + 1;
  if (mySlot.num > 0) {
    callingStation = phoneBook[posPBX] + PHONE_BASE;
    if (callingStation == PHONE_BASE)
      phonePhase = PHONE_BUSY;
    else {
      phonePhase = PHONE_CHECK;
      doPhoneStatus = true;
      infoDesvio(callingStation);               // OPC_SW_STATE (0xBC) -> OPC_LONG_ACK
      //phonePhase = (phonePBX & bit(posPBX)) ? PHONE_BUSY : PHONE_CALLING;   // green/red
#ifdef DEBUG
      for (posPBX = 0; posPBX < MAX_STATION; posPBX++)
        infoDesvio(phoneBook[posPBX] + PHONE_BASE);
      infoDesvio(myStation);
      infoDesvio(callingStation);
      infoDesvio(originateStation);
#endif
    }
    scrOLED = SCR_PHONE;
    updateOLED = true;
    updateAllOLED = true;                               // draw Big phone & hanged phone
  }
}

#endif
