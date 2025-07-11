/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/
*/


const char menuMsg[][15] PROGMEM = {
  "Conduire",
  "Sel. Loco",
#ifdef USE_LOCONET
  "Dispatch",
#endif
  "Aiguilles",
  "Plaque Tourn.",
  "Navette",
#ifdef USE_AUTOMATION
  "Automatisation",
#endif
  "Prog. CV",
  "Configurer",
};


const char cfgMsg[][15] PROGMEM = {
  "OPTIONS:",
  "CONTRASTE",
  "",
  "STOP MODE",
  "Vitesse 0",
  "Stop Emerg.",
  "Manoeuvre",
#ifdef USE_LOCONET
  "CENTRALE",
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
  "SERVEUR",
  "Port",
  "Binary",
  "LBServer",
#endif
#ifdef USE_Z21
  "ADR. COURTE",
  "1 a 99",
  "1 a 127",
#endif
  "PLAQUE TOURN.",
  "7686 KB14",
  "7686 KB15",
  "F6915",
#if defined(USE_XPRESSNET) || defined(USE_Z21)
  "Adr. Roco",
#endif
#ifdef USE_SET_TIME
  "HORLOGE",
  "Reglage",
#ifdef USE_LOCONET
  "Impuls. Synch.",
#endif
#endif
  "VERROUILLER",
  "Sel. Loco",
  "Aiguilles",
  "Prog. CV",
#ifdef GAME_EXTRA
  "JEU",
  "Tic-Tac-Toe",
  "Snake",
  "Flappy Bird",
  "Paku Paku",
#if defined(USE_Z21) || defined(USE_ECOS) || defined(USE_LNWIFI) || defined(USE_XNWIFI)
  "Pong",
#endif
#endif
#ifdef USE_PHONE
  "ANNUAIRE",
  "PacoMouse",
  "Gares",
#endif
};


const char prgMsg[][15] PROGMEM = {
  "Voie Prog.",
  "Voie Princ.",
  "Adresse Loco",                         // CV1, CV17, CV18, CV29
  "Vites. min.",                          // CV2
  "Vites. moy.",                          // CV6
  "Vites. max.",                          // CV5
  "Accelerat.",                           // CV3
  "Freinage",                             // CV4
  "Configurat.",                          // CV29
  "Fabricant",                            // CV8
  "Prog. bits CV",                        // Bit mode
#ifdef USE_LNCV
  "LNCV",
#endif
};


const char otherMsg[][15] PROGMEM = {
  "Sel. Aiguille",                        // SCR_TURN
  "Voie Dest.",                           // SCR_TURNTABLE
  "Selec. Loco:",                         // SCR_LOCO
  "Loco ",                                // SCR_DISPATCH
  "Recherche...",                         // WIFI
  "De liaison...",                        // WIFI
  "Gare",                                 // PHONE
  "Occupe...",                            // PHONE
  "Arretez Train",                        // PHONE
#ifdef USE_AUTOMATION
  "Automatisation",                       // AUTO
  "Nom:",                                 // AUTO
  "Supprimer?",                           // AUTO
#endif
};
