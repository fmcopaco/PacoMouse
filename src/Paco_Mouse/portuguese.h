/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
*/


const char menuMsg[][15] PROGMEM = {
  "Dirigir",
  "Sel. Loco",
#ifdef USE_LOCONET
  "Dispatch",
#endif
  "Agulha",
  "Mesa Girat.",
  "Transporte",
#ifdef USE_AUTOMATION
  "Automatizar",
#endif
  "Prog. CV",
  "Configurar",
};


const char cfgMsg[][15] PROGMEM = {
  "SELECIONAR:",
  "CONTRASTE",
  "",
  "MODO STOP",
  "Velocidade 0",
  "Stop Emerg.",
  "Manobras",
#ifdef USE_LOCONET
  "TIPO CENTRO",
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
  "Porta",
  "Binary",
  "LBServer",
#endif
#ifdef USE_Z21
  "END. CURTO",
  "1 a 99",
  "1 a 127",
#endif
  "MESA GIRAT.",
  "7686 KB14",
  "7686 KB15",
  "F6915",
#if defined(USE_XPRESSNET) || defined(USE_Z21)
  "End. Roco",
#endif
#ifdef USE_SET_TIME
  "RELOGIO",
  "Definir",
#ifdef USE_LOCONET
  "Pulso Sinc.",
#endif
#endif
  "TRAVAR",
  "Sel. Loco",
  "Agulha",
  "Prog. CV",
#ifdef GAME_EXTRA
  "JOGO",
  "da Velha",
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
  "Estacoes",
#endif
};


const char prgMsg[][15] PROGMEM = {
  "Ramal Prog.",
  "Ramal Princ.",
  "Ender. Loco",                          // CV1, CV17, CV18, CV29
  "Vel. min.",                            // CV2
  "Vel. media",                           // CV6
  "Vel. max.",                            // CV5
  "Acelerac.",                            // CV3
  "Frenagem",                             // CV4
  "Configurac.",                          // CV29
  "Fabricante",                           // CV8
  "Prog. bits CV",                        // Bit mode
#ifdef USE_LNCV
  "LNCV",
#endif
};


const char otherMsg[][15] PROGMEM = {
  "Selec. Agulha:",                       // SCR_TURN
  "Ramal destino",                        // SCR_TURNTABLE
  "Sel. Locomot:",                        // SCR_LOCO
  "Loco ",                                // SCR_DISPATCH
  "Procurando...",                        // WIFI
  "Conectando...",                        // WIFI
  "Estacao",                              // PHONE
  "Ocupada...",                           // PHONE
  "Pare o Trem",                          // PHONE
#ifdef USE_AUTOMATION
  "Automatizar",                          // AUTO
  "Nome:",                                // AUTO
  "Apagar?",                              // AUTO
#endif
};
