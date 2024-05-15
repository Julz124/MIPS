#include <msp430.h>
#include "..\base.h"
#include "TA1.h"
#include "event.h"

#define BTN_MAX     6
#define COUNT_MAX   5

LOCAL Void debounce_BTN(const Button* curr_button);

// Button 1
LOCAL const button_const BTN1_CONST = { BIT1, EVENT_BTN1, (const Char *) &P1IN };
LOCAL button_var BTN1_VAR;
LOCAL const Button BTN_1 = { .btn_const = &BTN1_CONST, .btn_var = &BTN1_VAR };

// Button 2
LOCAL const button_const BTN2_CONST = { BIT0, EVENT_BTN2, (const Char *) &P1IN };
LOCAL button_var BTN2_VAR;
LOCAL const Button BTN_2 = {.btn_const = &BTN2_CONST, .btn_var = &BTN2_VAR };

// EX Btn 0
LOCAL const button_const EX_BTN0_CONST = { BIT0, EX_EVENT_BTN0, (const Char *) &P3IN };
LOCAL button_var EX_BTN0_VAR;
LOCAL const Button EX_BTN_0 = {.btn_const = &EX_BTN0_CONST, .btn_var = &EX_BTN0_VAR };

// EX Btn 1
LOCAL const button_const EX_BTN1_CONST = { BIT1, EX_EVENT_BTN1, (const Char *) &P3IN };
LOCAL button_var EX_BTN1_VAR;
LOCAL const Button EX_BTN_1 = {.btn_const = &EX_BTN1_CONST, .btn_var = &EX_BTN1_VAR };

// EX Btn 2
LOCAL const button_const EX_BTN2_CONST = { BIT2, EX_EVENT_BTN2, (const Char *) &P3IN };
LOCAL button_var EX_BTN2_VAR;
LOCAL const Button EX_BTN_2 = {.btn_const = &EX_BTN2_CONST, .btn_var = &EX_BTN2_VAR };

// EX Btn 3
LOCAL const button_const EX_BTN3_CONST = { BIT3, EX_EVENT_BTN3, (const Char *) &P3IN };
LOCAL button_var EX_BTN3_VAR;
LOCAL const Button EX_BTN_3 = {.btn_const = &EX_BTN3_CONST, .btn_var = &EX_BTN3_VAR };

// Button Array
LOCAL const Button* const BUTTONS[] = { &BTN_1, &BTN_2, &EX_BTN_0, &EX_BTN_1, &EX_BTN_2, &EX_BTN_3};
LOCAL UChar BTN_INDEX;

#pragma FUNC_ALWAYS_INLINE(TA1_init)
GLOBAL Void TA1_init(Void) {

    BTN1_VAR.cnt = 0;
    BTN2_VAR.cnt = 0;
    EX_BTN_0.cnt = 0;
    EX_BTN_1.cnt = 0;
    EX_BTN_2.cnt = 0;
    EX_BTN_3.cnt = 0;

    BTN1_VAR.state = S0;
    BTN2_VAR.state = S0;
    EX_BTN_0.state = S0;
    EX_BTN_1.state = S0;
    EX_BTN_2.state = S0;
    EX_BTN_3.state = S0;

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

    debounce_BTN(BUTTONS[BTN_INDEX]);

    BTN_INDEX++;
    if(BTN_INDEX >= BTN_MAX)
    {
        BTN_INDEX = 0;
    }

    CLRBIT(TA1CTL, TAIFG);           // clear interrupt flag
    __low_power_mode_off_on_exit();  // restore Active Mode on return
}

#pragma FUNC_ALWAYS_INLINE(debounce_BTN)
LOCAL Void debounce_BTN(const Button* curr_button) {

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
