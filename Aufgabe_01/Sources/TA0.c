#include <msp430.h>
#include "../base.h"
#include "TA0.h"

/*
 * Man soll sich eine geeignete Datenstruktur �berlegen,
 * die eine laufzeiteffiziente Ausf�hrung der ISR erm�glicht.
 */

#define HIGH 0x8000
#define LOW 0x0000
#define ACKFRQ 614.4 // kHz
#define TIMEBASE 50 // ms
#define SCALING ((UInt)(ACKFRQ * TIMEBASE))
#define TICK(t) (((SCALING / 8) / 8) * ((t) / TIMEBASE) - 1)
#define TABSIZE 3
#define TABSIZE_4 4
#define TABSIZE_5 6
#define TABSIZE_6 8

LOCAL const Int muster_1[TABSIZE] = {
    HIGH | TICK(40), LOW | TICK(10), 0};  // Muster 1
LOCAL const Int muster_2[TABSIZE] = {
    HIGH | TICK(15), LOW | TICK(15), 0};  // Muster 2
LOCAL const Int muster_3[TABSIZE] = {
    HIGH | TICK(5), LOW | TICK(5), 0};  // Muster 3
LOCAL const Int muster_4[TABSIZE_4] = {
    LOW | TICK(10), HIGH | TICK(10), LOW | TICK(30), 0}; // Muster 4
LOCAL const Int muster_5[TABSIZE_5] = {
    LOW | TICK(10), HIGH | TICK(10), LOW | TICK(10), HIGH | TICK(10), LOW | TICK(30), 0}; // Muster 5
LOCAL const Int muster_6[TABSIZE_6] = {
    LOW | TICK(10), HIGH | TICK(10), LOW | TICK(10), HIGH | TICK(10), LOW | TICK(10), HIGH | TICK(10), LOW | TICK(30), 0};// Muster 6

LOCAL const Int* muster[TABSIZE_5] = {&muster_1[0], &muster_2[0], &muster_3[0], &muster_4[0], &muster_5[0], &muster_6[0]};

LOCAL const Int *ptr;

GLOBAL Void set_blink_muster(UInt arg) {
/*
 * Die Funktion muss so erweitert werden,
 * dass ein Blinkmuster selektiert wird.
 * Diese L�sung h�ngt stark von der gew�hlten
 * Datenstruktur ab.
 */
    //if (arg > 0 && arg <= TABSIZE) {
    //ptr = &muster[arg];
    //}
}

#pragma FUNC_ALWAYS_INLINE(TA0_init)
GLOBAL Void TA0_init(Void) {
    ptr = muster[2];

   CLRBIT(TA0CTL, MC0 | MC1   // stop mode
                  | TAIE      // disable interrupt
                  | TAIFG);   // clear interrupt flag
   CLRBIT(TA0CCTL0, CM1 | CM0 // no capture mode
                  | CAP       // compare mode
                  | CCIE      // disable interrupt
                  | CCIFG);   // clear interrupt flag
   TA0CCR0  = 0;         // set up Compare Register
   TA0EX0   = TAIDEX_7;       // set up expansion register
   TA0CTL   = TASSEL__ACLK    // 614.4 kHz
            | MC__UP          // Up Mode
            | ID__8           // /4
            | TACLR           // clear and start Timer
            | TAIE            // enable interrupt
            | TAIFG;          // set interrupt flag

   SETBIT(TA0CTL, TAIE | TAIFG);
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt Void TIMER0_A1_ISR(Void) {

   /*
    * Der Inhalt der ISR ist zu implementieren
    */
    UInt cnt = *ptr++; // Lese das nächste Intervall aus dem Muster

    // Setze die LED je nach Intervallzustand
    if (TSTBIT(cnt, HIGH)) {
        SETBIT(P2OUT, BIT7); // LED an
    } else {
        CLRBIT(P2OUT, BIT7); // LED aus
    }

    CLRBIT(TA0CTL, TAIFG); // Lösche Timer-Interrupt-Flag

    // Setze den Wert von TA0CCR0 für das nächste Intervall
    TA0CCR0 = ~HIGH BAND cnt;

    // Überprüfe, ob das Ende des aktuellen Musters erreicht ist
    if (*ptr EQ 0) {
        ptr = muster[2]; // Zurücksetzen auf das erste Intervall im ersten Muster
    }

}
