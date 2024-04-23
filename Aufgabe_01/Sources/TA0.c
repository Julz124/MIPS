#include <msp430.h>
#include "../base.h"
#include "TA0.h"

#define SCALING (2400 - 1)
#define HIGH 0x80
#define LOW  0x00
#define MASK 0x7F

LOCAL const UChar blink_pattern1[] = {
    HIGH | 8, LOW | 2, 0
};
LOCAL const UChar blink_pattern2[] = {
    HIGH|3, LOW | 3, 0
};
LOCAL const UChar blink_pattern3[] = {
    HIGH|1, LOW | 1,  0
};
LOCAL const UChar blink_pattern4[] = {
    HIGH| 2, LOW | 8,   0
};
LOCAL const UChar blink_pattern5[] = {
  HIGH |2, LOW| 2,  HIGH |2,  LOW| 8, 0
};
LOCAL const UChar blink_pattern6[] = {
   HIGH |2, LOW | 2, HIGH |2, LOW| 2, HIGH|2 , LOW| 8, 0
};

LOCAL const UChar * const blink_ptr_arr[] = {
    &blink_pattern1[0],
    &blink_pattern2[0],
    &blink_pattern3[0],
    &blink_pattern4[0],
    &blink_pattern5[0],
    &blink_pattern6[0]
};

LOCAL const UChar* cur_pattern_ptr;  // Zeiger auf das aktuelle Muster
LOCAL UChar cnt_led;
LOCAL UChar req_pattern_index;

GLOBAL Void set_blink_muster(UInt arg) {
    req_pattern_index = arg;
}

#pragma FUNC_ALWAYS_INLINE(TA0_init)
GLOBAL Void TA0_init(Void) {
   cnt_led              = 0;
   req_pattern_index    = 0;

   CLRBIT(TA0CTL, MC0 | MC1   // stop mode
                  | TAIE      // disable interrupt
                  | TAIFG);   // clear interrupt flag
   CLRBIT(TA0CCTL0, CM1 | CM0 // no capture mode
                  | CAP       // compare mode
                  | CCIE      // disable interrupt
                  | CCIFG);   // clear interrupt flag

   TA0CCR0  = SCALING;        // set up Compare Register with SCALING Factor
   TA0EX0   = TAIDEX_7;       // set up expansion register. IDEX in TAxEX0: {/1, /2, /3, /4, /5, /6, /7, /8}
   TA0CTL   = TASSEL__ACLK    // 614.4 kHz
            | MC__UP          // Up Mode
            | ID__8           // /8     ID in TAxCTL: {/1, /2, /4, /8}
            | TACLR;          // clear and start Timer

   SETBIT(TA0CTL, TAIE        // enable interrupt
                | TAIFG);     // set interrupt flag

   cur_pattern_ptr = blink_ptr_arr[0];
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt Void TIMER0_A1_ISR(Void) {
    cnt_led++;
    if (cnt_led == ((*cur_pattern_ptr) && MASK)) {
        if(((*cur_pattern_ptr) && HIGH) == HIGH)
            CLRBIT(P1OUT, BIT2);
        else
            SETBIT(P1OUT, BIT2);
        cur_pattern_ptr++;

        if (*cur_pattern_ptr == 0) {
            cur_pattern_ptr = blink_ptr_arr[req_pattern_index];
        }
        cnt_led = 0;
    }
    CLRBIT(TA0CTL, TAIFG);
}
