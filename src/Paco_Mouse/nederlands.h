/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
*/


const char menuMsg[][15] PROGMEM = {
  "Bediening",
  "Sel. Loco",
#ifdef USE_LOCONET
  "Dispatch",
#endif
  "Wissel",
  "Draaischijf",
  "Pendeltrein",
#ifdef USE_AUTOMATION
  "Automatisering",
#endif
  "Prog. CV",
  "Configureren",
};


const char cfgMsg[][15] PROGMEM = {
  "OPTIES:",
  "CONTRAST",
  "",
  "STOPMODUS",
  "Snelheid 0",
  "Noodstop",
  "Rangeren",
#ifdef USE_LOCONET
  "CMD. STATION",
  "IB II / DR5000",
  "Uhlenbrock",
  "Digitrax",
#endif
#ifdef USE_XNWIRE
  "ADR. XPRESSNET",
  "",
#endif
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  "WIFI",
  "SSID",
  "Wachtwoord",
  "IP",
#endif
#if defined(USE_LNWIFI)
  "SERVER",
  "Poort",
  "Binary",
  "LBServer",
#endif
#ifdef USE_Z21
  "KORT ADRES",
  "1 tot 99",
  "1 tot 127",
#endif
  "DRAAISCHIJF",
  "7686 KB14",
  "7686 KB15",
  "F6915",
#if defined(USE_XPRESSNET) || defined(USE_Z21)
  "Roco Adres",
#endif
#ifdef USE_SET_TIME
  "KLOCK",
  "Aanpassen",
#ifdef USE_LOCONET
  "Synch. Puls",
#endif
#endif
  "BLOCK",
  "Sel. Loco",
  "Wissel",
  "Prog. CV",
#ifdef GAME_EXTRA
  "SPEL",
  "Tic-Tac-Toe",
  "Snake",
  "Flappy Bird",
  "Paku Paku",
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  "Pong",
#endif
#endif
#ifdef USE_PHONE
  "TELEFOONBOEK",
  "PacoMouse",
  "Treinstations",
#endif
};


const char prgMsg[][15] PROGMEM = {
  "Prog. Spoor",
  "Hoofdspoor",
  "Loco. Adres",                          // CV1, CV17, CV18, CV29
  "Snelheid min.",                        // CV2
  "Snelheid mid.",                        // CV6
  "Snelheid max.",                        // CV5
  "Optrektijd",                           // CV3
  "Remtijd",                              // CV4
  "Configuratie",                         // CV29
  "Fabrikant",                            // CV8
  "Prog. CV-Bits",                        // Bit mode
#ifdef USE_LNCV
  "LNCV",
#endif
};


const char otherMsg[][15] PROGMEM = {
  "Select. Wissel",                       // SCR_TURN
  "Bestem. spoor",                        // SCR_TURNTABLE
  "Select. Loco:",                        // SCR_LOCO
  "Loco ",                                // SCR_DISPATCH
  "Scannen...",                           // WIFI
  "Verbinden...",                         // WIFI
  "Treinstation",                         // PHONE
  "Bezet...",                             // PHONE
  "Stop Trein",                           // PHONE
#ifdef USE_AUTOMATION
  "Automatisering",                       // AUTO
  "Naam:",                                // AUTO
  "Verwijderen?",                         // AUTO
#endif
};
