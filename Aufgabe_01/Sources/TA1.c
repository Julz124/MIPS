#include <msp430.h>
#include "..\base.h"
#include "TA1.h"
#include "event.h"

#define BTN_MAX     2
#define COUNT_MAX   5

LOCAL Void debounce_Button(const Button* curr_button);

LOCAL button_var BTN1_VAR;
LOCAL const button_const BTN1_CONST = { BIT1, EVENT_BTN1, (const Char *) &P1IN };
LOCAL const Button BTN_1 = { .btn_const = &BTN1_CONST, .btn_var = &BTN1_VAR };

LOCAL button_var BTN2_VAR;
LOCAL const button_const BTN2_CONST = { BIT0, EVENT_BTN2, (const Char *) &P1IN };
LOCAL const Button BTN_2 = {.btn_const = &BTN2_CONST, .btn_var = &BTN2_VAR };

LOCAL const Button* const BUTTONS[] = { &BTN_1, &BTN_2};
LOCAL UChar BTN_INDEX;

#pragma FUNC_ALWAYS_INLINE(TA1_init)
GLOBAL Void TA1_init(Void) {

    BTN1_VAR.cnt = 0;
    BTN2_VAR.cnt = 0;

    BTN1_VAR.state = S0;
    BTN2_VAR.state = S0;

    BTN_INDEX = 0;

   CLRBIT(TA1CTL,   MC0 | MC1  // stop mode
                  | TAIE       // disable interrupt
                  | TAIFG);    // clear interrupt flag
   CLRBIT(TA1CCTL0, CM1 | CM0  // no capture mode
                  | CAP        // compare mode
                  | CCIE       // disable interrupt
                  | CCIFG);    // clear interrupt flag
   TA1CCR0  = 96-1;            // set up Compare Register
   TA1EX0   = TAIDEX_7;        // set up expansion register
   TA1CTL   = TASSEL__ACLK     // 614.4 kHz
            | MC__UP           // Up Mode
            | ID__8            // /8
            | TACLR            // clear and start Timer
            | TAIE;            // enable interrupt
}



#pragma vector = TIMER1_A1_VECTOR
__interrupt Void TIMER1_A1_ISR(Void) {

    debounce_Button(BUTTONS[BTN_INDEX]);

    BTN_INDEX++;
    if(BTN_INDEX >= BTN_MAX)
    {
        BTN_INDEX = 0;
    }

    CLRBIT(TA1CTL, TAIFG);
    __low_power_mode_off_on_exit();
}

#pragma FUNC_ALWAYS_INLINE(debounce_Button)
LOCAL Void debounce_Button(const Button* curr_button) {

    if(TSTBIT(*curr_button->btn_const->port, curr_button->btn_const->pin)) {
        if(curr_button->btn_var->state == S0) {
            if(curr_button->btn_var->cnt < COUNT_MAX) {
                curr_button->btn_var->cnt++;
            } else {
                curr_button->btn_var->state = S1;
                Event_set(curr_button->btn_const->event);
            }
        } else if(curr_button->btn_var->state == S1) {
            if(curr_button->btn_var->cnt < COUNT_MAX) {
                curr_button->btn_var->cnt++;
            }
        }
    } else  {
        if(curr_button->btn_var->state == S0) {
            if(curr_button->btn_var->cnt > 0) {
                curr_button->btn_var->cnt--;
            }
        } else if(curr_button->btn_var->state == S1) {
            if(curr_button->btn_var->cnt > 0) {
                curr_button->btn_var->cnt--;
            } else {
                curr_button->btn_var->state = S0;
            }
        }
    }
}
