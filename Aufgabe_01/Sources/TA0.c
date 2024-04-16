#include <msp430.h>
#include "..\base.h"
#include "TA0.h"

/*
 * Man soll sich eine geeignete Datenstruktur überlegen,
 * die eine laufzeiteffiziente Ausführung der ISR ermöglicht.
 */

#define HIGH 0x8000
#define LOW 0x0000
#define ACKFRQ 614.4 // kHz
#define TIMEBASE 250 // ms
#define SCALING ((UInt)(ACKFRQ * TIMEBASE))
#define TICK(t) (((SCALING / 8) / 8) * ((t) / TIMEBASE) - 1)
#define TABSIZE 2
// Zeiten in der Tabelle sind in ms
LOCAL const Int muster[TABSIZE] = {
High | TICK(x),
Low | TICK(y),
0};
LOCAL const Int *ptr;

GLOBAL Void set_blink_muster(UInt arg) {
/*
 * Die Funktion muss so erweitert werden,
 * dass ein Blinkmuster selektiert wird.
 * Diese Lösung hängt stark von der gewählten
 * Datenstruktur ab.
 */
    ptr = &muster[0];







}

#pragma FUNC_ALWAYS_INLINE(TA0_init)
GLOBAL Void TA0_init(Void) {

   CLRBIT(TA0CTL, MC0 | MC1   // stop mode
                  | TAIE      // disable interrupt
                  | TAIFG);   // clear interrupt flag
   CLRBIT(TA0CCTL0, CM1 | CM0 // no capture mode
                  | CAP       // compare mode
                  | CCIE      // disable interrupt
                  | CCIFG);   // clear interrupt flag
   TA0CCR0  = 3125-1;         // set up Compare Register
   TA0EX0   = TAIDEX_4;       // set up expansion register
   TA0CTL   = TASSEL__ACLK    // 614.4 kHz
            | MC__UP          // Up Mode
            | ID__4           // /4
            | TACLR           // clear and start Timer
            | TAIE            // enable interrupt
            | TAIFG;          // set interrupt flag
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt Void TIMER0_A1_ISR(Void) {

   /*
    * Der Inhalt der ISR ist zu implementieren
    */
}
