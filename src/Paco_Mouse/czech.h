/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
*/


const char menuMsg[][15] PROGMEM = {
  "Jizda",
  "Vyb. Loko",
#ifdef USE_LOCONET
  "Dispatch",
#endif
  "Vyhybka",
  "Tocna",
  "Pendl",
#ifdef USE_AUTOMATION
  "Automatika",
#endif
  "Prog. CV",
  "Nastaveni",
};


const char cfgMsg[][15] PROGMEM = {
  "VOLBY:",
  "KONTRAST",
  "",
  "REZIM STOP",
  "Rychlost 0",
  "Nouz. Zast.",
  "Posun",
#ifdef USE_LOCONET
  "CENTRALA",
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
  "Heslo",
  "IP",
#endif
#if defined(USE_LNWIFI)
  "SERVER",
  "Port",
  "Binary",
  "LBServer",
#endif
#ifdef USE_Z21
  "KRATKA ADR.",
  "1 az 99",
  "1 az 127",
#endif
  "TOCNA",
  "7686 KB14",
  "7686 KB15",
  "F6915",
#if defined(USE_XPRESSNET) || defined(USE_Z21)
  "Roco Adr.",
#endif
#ifdef USE_SET_TIME
  "RYCHLE HOD.",
  "Nast. Cas",
#ifdef USE_LOCONET
  "Synch. Pulz",
#endif
#endif
  "ZAMEK",
  "Vol. Loko",
  "Vyhybka",
  "Prog. CV",
#ifdef GAME_EXTRA
  "HRY",
  "Piskvorky",
  "Had",
  "Flappy Bird",
  "Paku Paku",
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  "Pong",
#endif
#endif
#ifdef USE_PHONE
  "ADRESAR",
  "PacoMouse",
  "Stanice",
#endif
};


const char prgMsg[][15] PROGMEM = {
  "Prog. Kolej",
  "Hlavni Kolej",
  "Adresa Loko.",                         // CV1, CV17, CV18, CV29
  "Min. rychlost",                        // CV2
  "Stred. rychl.",                        // CV6
  "Max. rychlost",                        // CV5
  "Zrychleni",                            // CV3
  "Brzdeni",                              // CV4
  "Konfigurace",                          // CV29
  "Vyrobce",                              // CV8
  "Prog. bity CV",                        // Bit mode
#ifdef USE_LNCV
  "LNCV",
#endif
};


const char otherMsg[][15] PROGMEM = {
  "Zvol Vyhybku",                         // SCR_TURN
  "Cilova Kolej",                         // SCR_TURNTABLE
  "Zvol Loko:",                           // SCR_LOCO
  "Loko ",                                // SCR_DISPATCH
  "Hledam...",                            // WIFI
  "Pripojuji...",                         // WIFI
  "Stanice",                              // PHONE
  "Obsazeno...",                          // PHONE
  "Zastav vlak",                          // PHONE
#ifdef USE_AUTOMATION
  "Automatika",                           // AUTO
  "Jmeno:",                               // AUTO
  "Smazat?",                              // AUTO
#endif
};
