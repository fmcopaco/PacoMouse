/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
*/


const char menuMsg[][15] PROGMEM = {
  "Conducir",
  "Sel. Loco",
#ifdef USE_LOCONET
  "Dispatch",
#endif
  "Desvios",
  "Plataforma",
  "Lanzadera",
#ifdef USE_AUTOMATION
  "Automatizacion",
#endif
  "Prog. CV",
  "Configurar",
};


const char cfgMsg[][15] PROGMEM = {
  "OPCIONES:",
  "CONTRASTE",
  "",
  "MODO STOP",
  "Velocidad 0",
  "Stop Emerg.",
  "Maniobras",
#ifdef USE_LOCONET
  "TIPO CENTRAL",
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
  "Puerto",
  "Binary",
  "LBServer",
#endif
#ifdef USE_Z21
  "DIR. CORTA",
  "1 a 99",
  "1 a 127",
#endif
  "PLATAFORMA",
  "7686 KB14",
  "7686 KB15",
  "F6915",
#if defined(USE_XPRESSNET) || defined(USE_Z21)
  "Dir. Roco",
#endif
#ifdef USE_SET_TIME
  "RELOJ",
  "Ajustar",
#ifdef USE_LOCONET
  "Pulso Sinc.",
#endif
#endif
  "BLOQUEAR",
  "Sel. Loco",
  "Desvios",
  "Prog. CV",
#ifdef GAME_EXTRA
  "JUEGO",
  "3 en raya",
  "Snake",
  "Flappy Bird",
  "Paku Paku",
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  "Pong",
#endif
#endif
#ifdef USE_PHONE
  "LISTIN TELF.",
  "PacoMouse",
  "Estaciones",
#endif
};


const char prgMsg[][15] PROGMEM = {
  "Via Program.",
  "Via Principal",
  "Direcc. Loco",                         // CV1, CV17, CV18, CV29
  "Vel. min.",                            // CV2
  "Vel. media",                           // CV6
  "Vel. max.",                            // CV5
  "Acelerac.",                            // CV3
  "Frenado",                              // CV4
  "Configurac.",                          // CV29
  "Fabricante",                           // CV8
  "Prog. bits CV",                        // Bit mode
#ifdef USE_LNCV
  "LNCV",
#endif
};


const char otherMsg[][15] PROGMEM = {
  "Selec. Desvio:",                       // SCR_TURN
  "Via destino",                          // SCR_TURNTABLE
  "Selecc. Loco:",                        // SCR_LOCO
  "Loco ",                                // SCR_DISPATCH
  "Buscando...",                          // WIFI
  "Conectando...",                        // WIFI
  "Estacion",                             // PHONE
  "Ocupado...",                           // PHONE
  "Detenga Tren",                         // PHONE
#ifdef USE_AUTOMATION
  "Automatizacion",                       // AUTO
  "Nombre:",                              // AUTO
  "Borrar?",                              // AUTO
#endif
};
