#include <msp430.h>
#include "../base.h"
#include "TA0.h"

/*
 * Man soll sich eine geeignete Datenstruktur �berlegen,
 * die eine laufzeiteffiziente Ausf�hrung der ISR erm�glicht.
 */

#define HIGH 0x8000
#define LOW  0x0000

#define ACKFRQ   614.4  // kHz
#define TIMEBASE 10  // ms
#define SCALING  ((UInt)(ACKFRQ * TIMEBASE))
#define TICK(t)  (((SCALING / 8) / 8) * ((t) / TIMEBASE) - 1)
#define TABSIZE 3
#define TABSIZE_4 4
#define TABSIZE_5 6
#define TABSIZE_6 8

// Zeiten in der Tabelle sind in ms
LOCAL const Int muster_1[TABSIZE] = {
  HIGH | TICK(2000), // HIGH
  LOW | TICK(500), // LOW
  0
}; //Muster 1

LOCAL const Int muster_2[TABSIZE] = {
    HIGH | TICK(1500),
    LOW | TICK(1500),
    0
};  // Muster 2

LOCAL const Int muster_3[TABSIZE] = {
    HIGH | TICK(5),
    LOW | TICK(5),
    0
};  // Muster 3

LOCAL const Int muster_4[TABSIZE_4] = {
    LOW | TICK(10),
    HIGH | TICK(10),
    LOW | TICK(30),
    0
}; // Muster 4

LOCAL const Int muster_5[TABSIZE_5] = {
    LOW | TICK(10),
    HIGH | TICK(10),
    LOW | TICK(10),
    HIGH | TICK(10),
    LOW | TICK(30),
    0
}; // Muster 5

LOCAL const Int muster_6[TABSIZE_6] = {
    LOW | TICK(10),
    HIGH | TICK(10),
    LOW | TICK(10),
    HIGH | TICK(10),
    LOW | TICK(10),
    HIGH | TICK(10),
    LOW | TICK(30),
    0
}; // Muster 6

LOCAL const Int *muster[6];

LOCAL const Int *ptr;
//LOCAL const Int * start_ptr;

GLOBAL Void set_blink_muster(UInt arg) {
    ptr  = muster[arg];
    //start_ptr = muster[arg];
}

#pragma FUNC_ALWAYS_INLINE(TA0_init)
GLOBAL Void TA0_init(Void) {
    muster[0] = muster_1;
    muster[1] = muster_2;
    muster[2] = muster_3;
    muster[3] = muster_4;
    muster[4] = muster_5;
    muster[5] = muster_6;
   CLRBIT(TA0CTL, MC0 | MC1   // stop mode
                  | TAIE      // disable interrupt
                  | TAIFG);   // clear interrupt flag
   CLRBIT(TA0CCTL0, CM1 | CM0 // no capture mode
                  | CAP       // compare mode
                  | CCIE      // disable interrupt
                  | CCIFG);   // clear interrupt flag
   TA0CCR0  = 0;              // set up Compare Register
   TA0EX0   = TAIDEX_7;       // set up expansion register
   TA0CTL   = TASSEL__ACLK    // 614.4 kHz
            | MC__UP          // Up Mode
            | ID__8           // /8
            | TACLR;          // clear and start Timer
   SETBIT(TA0CTL, TAIE        // enable interrupt
                | TAIFG);     // set interrupt flag

}

#pragma vector = TIMER0_A1_VECTOR
__interrupt Void TIMER0_A1(Void) {
   UInt cnt = *ptr++;

   if (TSTBIT(cnt, HIGH)) {
      SETBIT(P2OUT, BIT7);
   } else {
      CLRBIT(P2OUT, BIT7);
   }

   CLRBIT(TA0CTL, TAIFG);    // clear interrupt flag
   TA0CCR0 = ~HIGH BAND cnt;

   if (*ptr EQ 0) {
      ptr = muster[0]; //start_ptr;
   }
}
