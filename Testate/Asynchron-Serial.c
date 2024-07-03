// Konfigurieren beider GPIO-Ports für RxD und TxD (Folie 116)
CLRBIT(P2SEL0, BIT1 + BIT0);        //set up Port2: Pin0 => TXD
SETBIT(P2SEL1, BIT1 + BIT0);        // Pin1 <= RXD
CLRBIT(P2REN, BIT1 + BIT0);         // without pull up

// Reset vorgang des UCA-Moduls [UCSWRST (software reset enable) auf Steuerregister UCA0CTLW0 setzen]
SETBIT(UCA0CTLW0, UCSWRST);         // UCA0 software reset

// -------------------- Entweder

// Bei UCSWRST=1 => beladen von restlichen Steuerregistern des UCA0-Moduls
UCA0CTLW1 = 0x0002;                 // deglitch time approximately 100ns
UCA0CTLW0_H = 0xC0;                 // even parity, LSB first, 8-bit data, One stop bit, UART mode, Asynchronous mode
SETBIT(UCA0CTLW0, UCBRKIE);         // recieve break character interrupt enable
SETBIT(UCA0CTLW0, UCSSEL__ACLK);    // select ACLK wih 4915,2 kHz
UCA0BRW = 64;                       // set clock prescaler for 4800 baud
UCA0MCTLW_L = 0;                    // first modulation stage
UCA0MCTLW_H = 0x00;                 // second modulation stage
SETBIT(UCA0MCTLW, UCOS16);          // times 16 oversampling

// Rücksetzen von UCSWRST in Steuerregister UCA0CTLW0 
CLRBIT(UCA0CTLW0, UCSWRST);         // release the UCA0 for operation

// Set Interrupt-Enable-Flag UCRXIE im Statusregister UCA0IE
SETBIT(UCA0IE, UCRXIE);             // enable recieve interrupt

// -------------------- ODER    (Folie 117)
UCA0CTLW1 = 0x0002;                 // deglitch time approximately 100ns
UCA0BRW = 4;                        // set clock prescaler for 9600 baud
// (Folie 121)
UCA0MCTLW = 0x00 << 0               // second modulation stage              (Bit 15-8)
          | 0x00                    // first modulation stage              pattern of BITCLK16 when UCOS16 = 1 (Bit 7-4) reserved bits (Bit 3-1)
          | UCOS16;                 // enable 16 times oversampling         (0 => Disabled, 1 => Enabled) (Bit 0)

UCA0CTLW0 = UCPEN                   // enable parity                        (0 => disabled, 1 => enabled) (Bit 15)
          | UCPAR                   // even parity                          (parity disabled => odd parity) (Bit 14)
          | 0                       // LSB first                            (0 => LSB, 1 => MSB) (Bit 13)
          | 0                       // 8-bit data                           (0 => 8-bit, 1 => 7-bit) (Bit 12)
          | 0                       // one stop bit                         (0 => One Stop Bit, 1 => Two Stop Bit) (Bit 11)
          | UCMODE_0                // UART mode                            (_0 => UART, 
                                    //                                       _1 => Idle-line multiprocessor, 
                                    //                                       _2 => Address-bit multiprocessor, 
                                    //                                       _3 => UART with automatic baud rate detection) (Bit 10-9)
          | 0                       // Asynchronous mode                    (0 => Asynchronous, 1 => Synchronous) (Bit 8)
          | UCSSEL__ACLK            // select clock source: ACLK 614.4 kHz  (_0 | __UCLK => UCLK,
                                    //                                       _1 | __ACLK => ACLK, 
                                    //                                       _2 | __SMCLK => SMCLK, 
                                    //                                       _3 | __SMCLK => SMCLK) (Bit 7-6)
          | UCRXEIE                 // error interrupt enable
          | UCBRKIE                 // break interrupt enable
          | 0;                      // release UCA0 for operation

UCA0IE = 0                          // Transmit Complete Interrupt Disable
       | 0                          // Start Bit Interrupt Disable
       | 0                          // Transmit Interrupt Disable
       | UCRXIE;                    // Recieve Interrupt Enable 


// ---------------------------------------- Error Types

/*
-------------------- Framing Error
Startbit erkannt, Stoppbit nicht vorhanden/zu viele oder zu wenig (low Stop bit)
UCFE bit ist gesetzt

-------------------- Parity Error
Anz. Nullen/Einsen im empfangenen Byte stimmen nicht mit der parität überein
UCPE bit ist gesetzt

-------------------- Recieve Overrun Error
empfangenes Byte wurde nicht rechtzeitig abgeholt und durch ein neues überschrieben
oder byte wurde in UCAxRXBUF geladen, bevor es richtig gelesen wurde
UCOE bit ist gesetzt

-------------------- Break Condition
Wenn alle Datenbits, das Stoppbit und das Paritätsbit auf 0 sind
UCBRK bit oder UCRXIFG interrupt flag ist gesetzt. letzteres ist der UCBRKIE bit gesetzt

*/
