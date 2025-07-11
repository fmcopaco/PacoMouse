/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
*/


const char menuMsg[][15] PROGMEM = {
  "Fahren",
  "Sel. Lok",
#ifdef USE_LOCONET
  "Dispatch",
#endif
  "Weichen",
  "Drehscheibe",
  "Pendelzug",
#ifdef USE_AUTOMATION
  "Automatisier",
#endif
  "Prog. CV",
  "Konfigur.",
};


const char cfgMsg[][15] PROGMEM = {
  "OPTIONEN:",
  "KONTRAST",
  "",
  "STOP MODUS",
  "Geschw. 0",
  "Not-Halt",
  "Rangieren",
#ifdef USE_LOCONET
  "ZENTRALE",
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
  "Passwort",
  "IP",
#endif
#if defined(USE_LNWIFI)
  "SERVER",
  "Port",
  "Binary",
  "LBServer",
#endif
#ifdef USE_Z21
  "KURZE ADR.",
  "1 bis 99",
  "1 bis 127",
#endif
  "DREHSCHEIBE",
  "7686 KB14",
  "7686 KB15",
  "F6915",
#if defined(USE_XPRESSNET) || defined(USE_Z21)
  "Roco-Adr.",
#endif
#ifdef USE_SET_TIME
  "MODELZEIT",
  "Uhrzeit",
#ifdef USE_LOCONET
  "Synch. Puls",
#endif
#endif
  "SPERREN",
  "Sel. Lok",
  "Weichen",
  "Prog. CV",
#ifdef GAME_EXTRA
  "SPIEL",
  "Tic-Tac-Toe",
  "Snake",
  "Flappy Bird",
  "Paku Paku",
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  "Pong",
#endif
#endif
#ifdef USE_PHONE
  "TELEFONBUCH",
  "PacoMouse",
  "Bahnhofe",
#endif
};


const char prgMsg[][15] PROGMEM = {
  "Prog. Gleis",
  "Hauptgleis",
  "Lokadresse",                           // CV1, CV17, CV18, CV29
  "Min. Gesch.",                          // CV2
  "Mitt. Gesch",                          // CV6
  "Max. Gesch.",                          // CV5
  "Beschleunig",                          // CV3
  "Bremsen",                              // CV4
  "Konfigurat.",                          // CV29
  "Hersteller",                           // CV8
  "Prog. CV-Bits",                        // Bit mode
#ifdef USE_LNCV
  "LNCV",
#endif
};


const char otherMsg[][15] PROGMEM = {
  "Sel. Weichen",                         // SCR_TURN
  "Zielgleis",                            // SCR_TURNTABLE
  "Sel. Lokomot:",                        // SCR_LOCO
  "Lok ",                                 // SCR_DISPATCH
  "Scannen...",                           // WIFI
  "Verbinden...",                         // WIFI
  "Bahnhof",                              // PHONE
  "Besetzter...",                         // PHONE
  "Stoppen Zug",                          // PHONE
#ifdef USE_AUTOMATION
  "Automatisier",                         // AUTO
  "Name:",                                // AUTO
  "Loschen?",                             // AUTO
#endif
};
