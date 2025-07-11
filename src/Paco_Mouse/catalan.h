/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
*/


const char menuMsg[][15] PROGMEM = {
  "Conduir",
  "Selec. Loco",
#ifdef USE_LOCONET
  "Dispatch",
#endif
  "Agulles",
  "Plataforma",
  "Llancadora",
#ifdef USE_AUTOMATION
  "Automatitzar",
#endif
  "Prog. CV",
  "Configurar",
};


const char cfgMsg[][15] PROGMEM = {
  "OPCIONS:",
  "CONTRAST",
  "",
  "MODE STOP",
  "Velocitat 0",
  "Stop Emerg.",
  "Maniobres",
#ifdef USE_LOCONET
  "TIPUS CENTRAL",
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
  "SERVIDOR",
  "Port",
  "Binary",
  "LBServer",
#endif
#ifdef USE_Z21
  "DIR. CURTA",
  "1 a 99",
  "1 a 127",
#endif
  "PLATAFORMA",
  "7686 KB14",
  "7686 KB15",
  "F6915",
#if defined(USE_XPRESSNET) || defined(USE_Z21)
  "Adr. Roco",
#endif
#ifdef USE_SET_TIME
  "RELLOTGE",
  "Ajustar",
#ifdef USE_LOCONET
  "Pols Sinc.",
#endif
#endif
  "BLOQUEJAR",
  "Sel. Loco",
  "Agulles",
  "Prog. CV",
#ifdef GAME_EXTRA
  "JOC",
  "Tic-Tac-Toe",
  "Snake",
  "Flappy Bird",
  "Paku Paku",
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  "Pong",
#endif
#endif
#ifdef USE_PHONE
  "GUIA TELEF.",
  "PacoMouse",
  "Estacions",
#endif
};


const char prgMsg[][15] PROGMEM = {
  "Via Program.",
  "Via Principal",
  "Direcc. Loco",                         // CV1, CV17, CV18, CV29
  "Vel. min.",                            // CV2
  "Vel. mitja",                           // CV6
  "Vel. max.",                            // CV5
  "Accelerac.",                           // CV3
  "Frenada",                              // CV4
  "Configurac.",                          // CV29
  "Fabricant",                            // CV8
  "Prog. bits CV",                        // Bit mode
#ifdef USE_LNCV
  "LNCV",
#endif
};


const char otherMsg[][15] PROGMEM = {
  "Selec. Agulla",                        // SCR_TURN
  "Via destinacio",                       // SCR_TURNTABLE
  "Selec. Loco:",                         // SCR_LOCO
  "Loco ",                                // SCR_DISPATCH
  "Escanejant...",                        // WIFI
  "Conectant...",                         // WIFI
  "Estacio",                              // PHONE
  "Ocupat...",                            // PHONE
  "Atureu Tren",                          // PHONE
#ifdef USE_AUTOMATION
  "Automatitzar",                         // AUTO
  "Nom:",                                 // AUTO
  "Esborrar?",                            // AUTO
#endif
};
