#include <msp430.h>
#include "Handler.h"
#include "event.h"
#include "UCA1.h"
#include "TA1.c"

#define DIGISIZE 4
#define BASE     10 // Basis des Zahlensystems kann zwischen 2 und 16 gewählt werden

LOCAL Uchar butn_idx;   // Welcher GPIO BTN gedrückt?
LOCAL int pattern_cnt;  // Aktuelles Pattern
LOCAL int curr_seg_val[DIGISIZE];   //Aktueller Zählwert der 7-Seg Anzeige

LOCAL VoidFunc state;   // Funktionspointer für 7-Seg Anzeige (Set and Reset)
LOCAL UInt idx; // Indexcounter 7-Seg Anzeige

// ----------------------------------------------------------------------------

satic void GPIO_BTN_Handler (TEvent arg){
    if (Event_tst(arg)){
        Event_clr(arg);
            btn_idx = gpio_btn;
            Event_set(EVENT_UPDATE_CNT);
    }
}

GLOBAL Void Button_Handler(Void) {

    //Eventhandler Button 1
    if (Event_tst(EVENT_BTN2)){
        Event_clr(EVENT_BTN2);
        if (++pattern_cnt GT MUSTER6) {
            pattern_cnt = MUSTER1;
        }
        set_blink_muster(pattern_cnt);
    }

    //Eventhandler Button 2
    if (Event_tst(EVENT_BTN1)){
        Event_clr(EVENT_BTN1);
        TGLBIT(P2OUT, BIT7);
    }

    GPIO_BTN_Handler(EX_EVENT_BTN0);
    GPIO_BTN_Handler(EX_EVENT_BTN1);
    GPIO_BTN_Handler(EX_EVENT_BTN2);
    GPIO_BTN_Handler(EX_EVENT_BTN3);

}

// ----------------------------------------------------------------------------


GLOBAL Void Number_Handler(Void) {
    if (Event_tst(EVENT_UPDATE_CNT)){
        Event_clr(EVENT_UPDATE_CNT);

        if(!TSTBIT(P2OUT, BIT7)){
            curr_seg_val[0] += EX_BTN_0.btn_var;



            curr_seg_val[butn_idx] += 1;
            curr_seg_val[butn_idx] == BASE;
            curr_seg_val[butn_idx] = 0;
            curr_seg_val[butn_idx+1] += 1

        } else { //decrement


        }

        Event_set(EVENT_UPDATE_SEG);

    }
}

// ----------------------------------------------------------------------------

static void State0(void) {
    if (Event_tst(EVENT_UPDATE_SEG)) {
        Event_clr(EVENT_UPDATE_SEG);
        idx = 1;
        state = State1;
        Event_set(EVENT_DONE_SEG);
    }
}

LOCAL Void State1(Void) {
    if (Event_tst(EVENT_DONE_SEG)) {
        Event_clr(EVENT_DONE_SEG);
        if (idx LE DIGISIZE)
        {
            UChar ch = bcd_cnt[idx - 1];
            //ch += '0';                  // convert to ASCII? Will not display decimal point anymore (is set with D7 = 0 regarding datasheet)
            UCA1_emit(idx, ch);
            idx++;
        }
        else
        {
            state = State0;
        }
    }
}

GLOBAL Void AS1108_Handler(Void) {
    (*state)()
}

// ----------------------------------------------------------------------------

GLOBAL Void Handler_init(Void) {

}

