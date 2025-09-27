/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
*/


////////////////////////////////////////////////////////////
// ***** USER OPTIONS *****
////////////////////////////////////////////////////////////

// Seleccione el idioma de los menus - Select menu language: ENGLISH/SPANISH/CATALAN/PORTUGUESE/GERMAN/FRENCH/ITALIAN/NEDERLANDS/CZECH
#define LANGUAGE                  SPANISH


// Selecione la version hardware de PacoMouse - Select the hardware version of PacoMouse: XNET/LNET/Z21/ECOS
#define HW_VERSION                XNET


// Seleccione el chip controlador del display 128x64 OLED - Select the 128x64 OLED display controller chip:  SSD1306/SH1106/SSD1309
#define OLED_CHIP                 SSD1306


// Direccion I2C del display OLED - I2C address of the OLED display (OLED I2C address - 0x3D: Adafruit I2C OLED, 0x3C: Clone I2C OLED)
#define I2C_ADDRESS_OLED          0x3C


// Seleccione el tipo de teclado - Select the type of keypad: KEYPAD/TOUCHPAD
#define KEYPAD_TYPE               KEYPAD


// Direccion I2C del PCF8574 para el keypad - I2C address of the PCF8574 for keypad (0x20..0x27)
#define I2C_ADDRESS_KEYPAD        0x20


// Descomentar la siguiente linea si se usa el touchpad TTP229 por detras - Uncomment next line if using rear side of touchpad TTP229
#define REAR_TOUCH                1


// Selecion tipo cambio direccion - Select change direction type: BUTTON_ENC/SWITCH_3P
#define CHANGE_DIR                BUTTON_ENC


// Mostrar pasos de velocidad - Show speed steps: LEFT/RIGHT/NONE
#define SHOW_STEPS                RIGHT


// Seleccione tamaño de las funciones a mostrar - Select size of functions: BIG/SMALL
#define FUNC_SIZE                 SMALL


// Selecciona iconos estado desvio - Select turnout status icons: ONLY_TURN/TURN_SIGNAL/BUTTON_IB
#define TURNOUT_ICON              TURN_SIGNAL


// Max. locomotoras guardadas en stack - Max. locomotives saved in stack:
#define LOCOS_IN_STACK            24


/*      Keypad 3x4                      Keypad 4x4

    |-----+-----+-----|         |-----+-----+-----+-----|
    |     |     |     |         |     |     |     |     |
    |  1  |  2  |  3  |         |  1  |  2  |  3  |STOP |
    |     |     |     |         |     |     |     |     |
    |-----+-----+-----|         |-----+-----+-----+-----|
    |     |     |     |         |     |     |     |     |
    |  4  |  5  |  6  |         |  4  |  5  |  6  |LOCO |
    |     |     |     |         |     |     |     |     |
    |-----+-----+-----|         |-----+-----+-----+-----|
    |     |     |     |         |     |     |     |     |
    |  7  |  8  |  9  |         |  7  |  8  |  9  | UP  |
    |     |     |     |         |     |     |     |     |
    |-----+-----+-----|         |-----+-----+-----+-----|
    |     |     |     |         |     |     |     |     |
    |MENU |  0  |ENTER|         |MENU |  0  |ENTER|DOWN |
    |     |     |     |         |     |     |     |     |
    |-----+-----+-----|         |-----+-----+-----+-----|

*/

// Descomentar la siguiente linea si se usa un teclado 4x4 - Uncomment next line if using 3x4 keypad
#define USE_KEYPAD_4x4            1

// Descomentar la siguiente linea para mayor sensibilidad del encoder. No recomendado - Uncomment next line for more encoder sensibility. Not recomended. 
//#define SENSITIVE                 1


// Direccion Xpressnet por defecto de Paco Mouse - Default Xpressnet address for PacoMouse (1..31)
#define DEFAULT_XNET_ADR          8


// Descomentar la siguiente linea para alimentar el touchpad desde pines (solo placa PacoMouse) - Uncomment next line to power touchpad from pins (only PacoMouse board)
//#define POWER_TOUCH                1


////////////////////////////////////////////////////////////
// ***** END OF USER OPTIONS *****
////////////////////////////////////////////////////////////


#if  (HW_VERSION == XNET)  
#define USE_XPRESSNET             1                           // Xpressnet  - Arduino Nano & ESP8266 Wemos D1 mini
#endif
#if  (HW_VERSION == LNET) 
#define USE_LOCONET               1                           // Loconet    - Arduino Nano & ESP8266 Wemos D1 mini
#endif
#if  (HW_VERSION == Z21) 
#define USE_Z21                   1                           // Z21        - ESP8266 Wemos D1 mini
#endif
#if  (HW_VERSION == ECOS) 
#define USE_ECOS                  1                           // ECoS/CS1   - ESP8266 Wemos D1 mini
#endif


#ifdef USE_Z21
#define GAME_EXTRA
#define USE_AUTOMATION
#define USE_SET_TIME
#endif

#ifdef USE_ECOS
#define GAME_EXTRA
#define USE_AUTOMATION
#endif

#ifdef USE_XPRESSNET
#if defined(__AVR__)
#define USE_XNWIRE
//#define GAME_EXTRA
#define USE_AUTOMATION
//#define USE_SET_TIME
#endif
#if defined(ESP8266)
#define USE_XNWIFI
#define GAME_EXTRA
#define USE_AUTOMATION
#define USE_SET_TIME
#endif
#endif

#ifdef USE_LOCONET
#if defined(__AVR__)
#define USE_LNWIRE
#define USE_PHONE
#define USE_LNCV
#define USE_PRG_UHLI
#define USE_DECODE_IMM
#endif
#if defined(ESP8266)
#define USE_LNWIFI
#define USE_PHONE
#define USE_LNCV
#define USE_PRG_UHLI
#define USE_DECODE_IMM
#define GAME_EXTRA
#define USE_AUTOMATION
#define USE_SET_TIME
#endif
#endif


/*
  The SH1106 controller has an internal RAM of 132x64 pixel. The SSD1306 only has 128x64 pixel.
  The SSD1306 has an expanded command set over the SH1106.
  The SH1106 only supports page addressing mode
  The SSD1306 added special horizontal and vertical addressing modes.
  The SSD1306 also has capability for automatic scrolling.
*/
#if  (OLED_CHIP == SSD1306)  
#define USE_SSD1306               1                           // OLED 0.96"
#endif
#if  (OLED_CHIP == SH1106)  
#define USE_SH1106                1                           // OLED 1.3"
#endif
#if  (OLED_CHIP == SSD1309)  
#define USE_SSD1309               1                           // OLED 2.42" & 1.54"
#endif


#if  (KEYPAD_TYPE == KEYPAD)  
#undef USE_TTP229                                             // Keypad (wires or PCF8574)
#endif
#if  (KEYPAD_TYPE == TOUCHPAD)  
#define USE_TTP229                1                           // Touchpad TTP229
#endif

#if defined(USE_LOCONET) && defined(USE_XPRESSNET)
#error Seleccione solo un tipo de interface - Select only one type of interface
#endif

#if defined(ESP8266)
#if defined(USE_LNWIRE) || defined(USE_XNWIRE)
#error Con ESP8266 solo se puede seleccionar protocolo Z21, Loconet, Xpressnet o ECoS - With ESP8266 only the Z21, Loconet, Xpressnet or ECoS protocol can be selected
#endif
#endif

#if defined(__AVR__) && defined(USE_Z21)
#error El protocolo Z21 solo es para ESP8266 - Z21 protocol is only for ESP8266
#endif

#if defined(__AVR__) && defined(USE_ECOS)
#error El protocolo ECOS solo es para ESP8266 - ECOS protocol is only for ESP8266
#endif

#if defined(__AVR__) && defined(USE_LNWIFI)
#error El protocolo Loconet WiFi solo es para ESP8266 - Loconet WiFi protocol is only for ESP8266
#endif

#if defined(__AVR__) && defined(USE_XNWIFI)
#error El protocolo Xpressnet WiFi solo es para ESP8266 - Xpressnet WiFi protocol is only for ESP8266
#endif

#ifdef USE_SSD1306
#if defined(USE_SH1106) || defined(USE_SSD1309)
#error Defina solo un tipo de pantalla - Define only one type of screen
#endif
#define OLED_TYPE_MAIN  Adafruit128x64
#endif

#ifdef USE_SH1106
#if defined(USE_SSD1306) || defined(USE_SSD1309)
#error Defina solo un tipo de pantalla - Define only one type of screen
#endif
#define OLED_TYPE_MAIN  SH1106_128x64
#endif

#ifdef USE_SSD1309
#if defined(USE_SSD1306) || defined(USE_SH1106)
#error Defina solo un tipo de pantalla - Define only one type of screen
#endif
#define OLED_TYPE_MAIN  SSD1309_128x64
#endif

#if defined(USE_SSD1306) || defined(USE_SH1106) || defined(USE_SSD1309)
#else
#error No ha definido ninguna pantalla - You have not defined any screen
#endif

#if defined(USE_LOCONET) || defined(USE_XPRESSNET) || defined(USE_Z21) || defined(USE_ECOS)
#else
#error No ha definido ningun interface - You have not defined any interface
#endif

/*
    Arduino Nano

    D0  RXD   Xpressnet RXD. (NO CAMBIAR) Serial interface
    D1  TXD   Xpressnet TXD. (NO CAMBIAR) Serial interface
    D2  INT0  Encoder (NO CAMBIAR)
    D3        Teclado fila 1
    D4        Teclado fila 2
    D5        Teclado fila 3
    D6        Teclado fila 4
    D7        LN TXD
    D8  ICP   LN RXD (NO CAMBIAR)
    D9        Teclado columna 1
    D10       Teclado columna 2
    D11       Teclado columna 3
    D12       Encoder
    D13 LED
    A0        Teclado columna 4 (opcion USE_KEYPAD_4x4)
    A1        Analogico. Interruptor direccion
    A2        Encoder. Pulsador
    A3        Xpressnet TXRX
    A4  SDA   OLED SSD1306 (NO CAMBIAR)
    A5  SCL   OLED SSD1306 (NO CAMBIAR)
    A6
    A7
*/

////////////////////////////////////////////////////////////
// ***** ARDUINO PINS *****
////////////////////////////////////////////////////////////

#if defined(USE_LNWIRE) || defined(USE_XNWIRE)

const int pinOutA       = 2;                                  // encoder pins. NO CAMBIAR, D2 es un pin de interrupcion
const int pinOutB       = 12;
const int pinSwitch     = A2;

const int pinFILA1      = 3;                                  // Keypad 3x4 or 4x4 pins
const int pinFILA2      = 4;
const int pinFILA3      = 5;
const int pinFILA4      = 6;
const int pinCOLUMNA1   = 9;
const int pinCOLUMNA2   = 10;
const int pinCOLUMNA3   = 11;
const int pinCOLUMNA4   = A0;

const int pinSDA        = A4;                                 // I2C pins: OLED
const int pinSCL        = A5;

const int pinLN_RXD     = 8;                                  // Loconet pins. NO CAMBIAR, pin LN_RXD debe ser pin ICP del Arduino
const int pinLN_TXD     = 7;

const int pinTXRX       = A3;                                 // Xpressnet pins

const int pinCHG_DIR    = A1;                                 // only for change direction using switch 3P

const int pinTOUCH_SCL  = 10;                                 // TTP229 keypad pins
const int pinTOUCH_SDO  = 9;
const int pinTOUCH_VCC  = A0;                                 // only for PacoMouse board
const int pinTOUCH_GND  = 11;

#endif


/*
  Wemos D1 mini Pinout:

   TX  TX               GPIO1       Serial interface
   RX  RX               GPIO3       Serial interface
   D0                   GPIO16
   D1  SCL              GPIO5       I2C OLED/PCF8574
   D2  SDA              GPIO4       I2C OLED/PCF8574
   D3  10k Pull-up      GPIO0       (FLASH)
   D4  10k Pull-up,     GPIO2       BUILTIN_LED
   D5  SCK              GPIO14
   D6  MISO             GPIO12
   D7  MOSI             GPIO13
   D8  SS 10k Pull-down GPIO15      Prog-Enable
   A0                               Sense Input
*/

////////////////////////////////////////////////////////////
// ***** ESP8266 WEMOS D1 PINS *****
////////////////////////////////////////////////////////////

#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)

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

const int pinCHG_DIR    = A0;                                 // only for change direction using switch 3P

const int pinTOUCH_SCL  = D3;                                 // TTP229 keypad pins
const int pinTOUCH_SDO  = D4;

#endif
