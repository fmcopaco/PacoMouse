/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
*/


const char menuMsg[][15] PROGMEM = {
  "Drive",
  "Sel. Loco",
#ifdef USE_LOCONET
  "Dispatch",
#endif
  "Turnout",
  "Turntable",
  "Shuttle",
#ifdef USE_AUTOMATION
  "Automation",
#endif
  "Prog. CV",
  "Configure",
};


const char cfgMsg[][15] PROGMEM = {
  "OPTIONS:",
  "CONTRAST",
  "",
  "STOP MODE",
  "Speed 0",
  "Emerg. Stop",
  "Shunting",
#ifdef USE_LOCONET
  "CMD. STATION",
  "IB II / DR5000",
  "Uhlenbrock",
  "Digitrax",
#endif
#ifdef USE_XNWIRE
  "DIR. XPRESSNET",
  "",
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  "WIFI",
  "SSID",
  "Password",
  "IP",
#endif
#if defined(USE_LNWIFI)
  "SERVER",
  "Port",
  "Binary",
  "LBServer",
#endif
#ifdef USE_Z21
  "SHORT ADR.",
  "1 to 99",
  "1 to 127",
#endif
  "TURNTABLE",
  "7686 KB14",
  "7686 KB15",
  "F6915",
#if defined(USE_XPRESSNET) || defined(USE_Z21)
  "Roco Addr.",
#endif
#ifdef USE_SET_TIME
  "FAST CLOCK",
  "Set Time",
#ifdef USE_LOCONET
  "Synch. Pulse",
#endif
#endif
  "LOCK",
  "Sel. Loco",
  "Turnout",
  "Prog. CV",
#ifdef GAME_EXTRA
  "GAME",
  "Tic-Tac-Toe",
  "Snake",
  "Flappy Bird",
  "Paku Paku",
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  "Pong",
#endif
#endif
#ifdef USE_PHONE
  "PHONEBOOK",
  "PacoMouse",
  "Stations",
#endif
};


const char prgMsg[][15] PROGMEM = {
  "Prog. Track",
  "Main Track",
  "Loco Address",                         // CV1, CV17, CV18, CV29
  "Speed min.",                           // CV2
  "Speed mid.",                           // CV6
  "Speed max.",                           // CV5
  "Accelerat.",                           // CV3
  "Braking",                              // CV4
  "Configurat.",                          // CV29
  "Manufacturer",                         // CV8
  "Prog. CV bits",                        // Bit mode
#ifdef USE_LNCV
  "LNCV",
#endif
};


const char otherMsg[][15] PROGMEM = {
  "Select Turnout",                       // SCR_TURN
  "Dest. Track",                          // SCR_TURNTABLE
  "Select Loco:",                         // SCR_LOCO
  "Loco ",                                // SCR_DISPATCH
  "Scanning...",                          // WIFI
  "Connecting...",                        // WIFI
  "Station",                              // PHONE
  "Busy...",                              // PHONE
  "Stop Train",                           // PHONE
#ifdef USE_AUTOMATION
  "Automation",                           // AUTO
  "Name:",                                // AUTO
  "Delete?",                              // AUTO
#endif
};
