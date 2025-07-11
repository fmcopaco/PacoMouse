/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
*/


const char menuMsg[][15] PROGMEM = {
  "Comandare",
  "Sel. Loco",
#ifdef USE_LOCONET
  "Dispatch",
#endif
  "Scambi",
  "Piattaforma",
  "Navetta",
#ifdef USE_AUTOMATION
  "Automazione",
#endif
  "Prog. CV",
  "Configurare",
};


const char cfgMsg[][15] PROGMEM = {
  "OPZIONI:",
  "CONTRASTO",
  "",
  "MOD. ARRESTO",
  "Velocita 0",
  "Stop Emerg.",
  "Manovra",
#ifdef USE_LOCONET
  "CENTRALINA",
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
  "Porta",
  "Binary",
  "LBServer",
#endif
#ifdef USE_Z21
  "IND. BREVE",
  "1 a 99",
  "1 a 127",
#endif
  "PIATTAFORMA",
  "7686 KB14",
  "7686 KB15",
  "F6915",
#if defined(USE_XPRESSNET) || defined(USE_Z21)
  "Ind. Roco",
#endif
#ifdef USE_SET_TIME
  "OROLOGIO",
  "Imposta",
#ifdef USE_LOCONET
  "Impulso Sinc.",
#endif
#endif
  "BLOCCARE",
  "Sel. Loco",
  "Scambi",
  "Prog. CV",
#ifdef GAME_EXTRA
  "GIOCO",
  "Tris",
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
  "Stazione",
#endif
};


const char prgMsg[][15] PROGMEM = {
  "Binario Prog.",
  "Binario Princ.",
  "Indiriz. Loco",                        // CV1, CV17, CV18, CV29
  "Vel. min.",                            // CV2
  "Vel. media",                           // CV6
  "Vel. mass.",                           // CV5
  "Acceleraz.",                           // CV3
  "Frenata",                              // CV4
  "Configuraz.",                          // CV29
  "Produttore",                           // CV8
  "Prog. bit CV",                         // Bit mode
#ifdef USE_LNCV
  "LNCV",
#endif
};


const char otherMsg[][15] PROGMEM = {
  "Selez. Scambi:",                       // SCR_TURN
  "Binario dest.",                        // SCR_TURNTABLE
  "Selez. Loco:",                         // SCR_LOCO
  "Loco ",                                // SCR_DISPATCH
  "Ricerca...",                           // WIFI
  "Collegamento..",                       // WIFI
  "Stazione",                             // PHONE
  "Occupata...",                          // PHONE
  "Fermare Treno",                        // PHONE
#ifdef USE_AUTOMATION
  "Automazione",                          // AUTO
  "Nome:",                                // AUTO
  "Eliminare?",                           // AUTO
#endif
};
