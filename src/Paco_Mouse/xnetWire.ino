/*    Paco Mouse -- F. Cañada 2022-2025 --  https://usuaris.tinet.cat/fmco/

      This software and associated files are a DIY project that is not intended for commercial use.
      This software uses libraries with different licenses, follow all their different terms included.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.

      Sources are only provided for building and uploading to the device.
      You are not allowed to modify the source code or fork/publish this project.
      Commercial use is forbidden.
*/

#ifdef USE_XNWIRE

void beginXpressNet (byte xnetAddr) {
  pinMode (pinTXRX, OUTPUT);
  digitalWrite (pinTXRX, LOW) ;                             // recibir datos
  miCallByte = paridadCallByte (xnetAddr);
  enviaMensaje = false;
  leerDatoXN = false;
  rxIndice = DATA1;
  cli();                                                    // deshabilitar interrupciones para acceder a registros
  UBRR0H = 0;                                               // UBRR = (FXTAL / (16 *  baud)) - 1 si U2X = 0
  UBRR0L = 0x0F;                                            // Set 62500 baud
  UCSR0A = 0;                                               // U2X = 0
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0) | (1 << UCSZ02); // Enable reception (RXEN) transmission (TXEN0) Receive Interrupt (RXCIE = 1)
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);                   // UCSZ = b111 = 9 bits
  sei();                                                    // habilitar interrupciones
  getVersion();                                             // pide la version del Xpressnet
  getStatus();                                              // pide estado de la central
}


byte paridadCallByte (byte direccionXN) {
  bool paridad = false;
  byte bits;
  direccionXN &= 0x1F;                                      // Borra bit 7 de la direccion
  bits = direccionXN;
  while (bits) {                                            // mientras haya bits cambia paridad
    paridad = !paridad;
    bits &= (bits - 1);
  }
  if (paridad)                                              // coloca paridad en bit 7
    direccionXN |= 0x80;
  return (direccionXN);                                     // P00AAAAA
}


void enviaUSART (byte data) {
  while (!(UCSR0A & (1 << UDRE0)));                         // esperar a que se pueda enviar
  UDR0 = data;
}


ISR(USART_RX_vect)  {                                       // Interrupcion recepcion datos por Serial
  if (UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0))) {  // error en la recepcion?
    rxData = UDR0;
    return;
  }
  if (UCSR0B & (1 << RXB80)) {                              // leer primero 9 bit. Activado: Call Byte
    rxData = UDR0;
    leerDatoXN = false;
    rxIndice = HEADER;
    if (rxData == (miCallByte ^ 0xC0)) {                    // Call Byte: P10AAAAA. Normal Inquiry
      if (enviaMensaje) {                                   // Hay mensaje para enviar?
        delayMicroseconds(48);                              // esperamos el tiempo de tres bit antes de enviar
        digitalWrite (pinTXRX, HIGH);                       // enviamos mensaje
        delayMicroseconds(8);                               // esperamos el tiempo de medio bit antes de enviar
        for (rxIndice = HEADER; rxIndice < txBytes; rxIndice++)
          enviaUSART (txBuffer[rxIndice]);
        WAIT_FOR_XMIT_COMPLETE
        digitalWrite (pinTXRX, LOW);
        enviaMensaje = false;
      }
    }
    // Call Byte:   P11AAAAA Message,    P0100000 Feedback BC o P1100000 Broadcast
    if ((rxData == (miCallByte | 0x60)) || (rxData == 0xA0) || (rxData == 0x60)) {
      leerDatoXN = true;
      rxXOR = 0;
    }
    if (rxData == miCallByte) {                             // Call Byte: P00AAAAA. Request ACK
      delayMicroseconds(32);                                // esperamos el tiempo de dos bit antes de enviar
      digitalWrite (pinTXRX, HIGH);                         // respuesta inmediata si ha habido error de transmision
      delayMicroseconds(8);                                 // esperamos el tiempo de medio bit antes de enviar
      enviaUSART (0x20);
      enviaUSART (0x20);
      WAIT_FOR_XMIT_COMPLETE
      digitalWrite (pinTXRX, LOW);
    }
  }
  else {                                                    // 9 bit desactivado: Datos
    rxData = UDR0;
    if (leerDatoXN) {                                       // leer paquete Xpressnet
      rxBufferXN[rxIndice++] = rxData;
      rxXOR ^= rxData;
      if (((rxBufferXN[HEADER] & 0x0F) + 2) == rxIndice) {  // si se han recibido todos los datos indicados en el paquete
        leerDatoXN = false;
        if (rxXOR == 0) {                                   // si el paquete es correcto
          rxBytes = rxIndice;
          procesaXN();                                      // nuevo paquete recibido, procesarlo
        }
      }
    }
  }
}


void headerXN (byte header) {
  while (enviaMensaje);                                    // espera a que se envie el ultimo mensaje
  //enviaMensaje = false;                                     // ahora podemos modificar el buffer
  txBytes = HEADER;                                         // coloca header en el buffer
  txXOR = header;
  txBuffer[txBytes++] = header;
}


void dataXN (byte dato) {
  txBuffer[txBytes++] = dato;                               // coloca dato en el buffer
  txXOR ^= dato;
}


void sendXN () {
  txBuffer[txBytes++] = txXOR;                              // coloca XOR byte en el buffer
  enviaMensaje = true;
}


void xpressnetProcess () {                                  // procesa Xpressnet
  if (getInfoLoco && (csStatus == csNormalOps))
    infoLocomotora(myLoco.address);
  if (millis() - infoTimer > 1000UL) {                      // Cada segundo
    infoTimer = millis();
    if (getResultsSM)                                       // Resultados de CV pendientes
      getResults();                                         // pide resultados
    else {
      if (bitRead(mySteps, 3))                              // Loco controlada por otro mando
        getInfoLoco = true;                                 // pide info locomotora
      if (askMultimaus) {                                   // pide info Multimaus
        askMultimaus = false;
        versionMultimaus();
      }
      if ((scrOLED == SCR_TURNOUT) && (xnetCS > 0x10))      // Z21 info desvios
        infoDesvio (myTurnout);
    }
  }
  if (progFinished) {                                       // fin de lectura/programacion CV
    progFinished = false;
    endProg();
  }
}


#endif
