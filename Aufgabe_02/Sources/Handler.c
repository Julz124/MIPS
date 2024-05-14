#include <msp430.h>
#include "Handler.h"
#include "event.h"
#include "UCA1.h"

#define DIGISIZE 4
#define BASE     8 // Basis des Zahlensystems kann zwischen 2 und 16 gewählt werden

LOCAL Uchar butn_idx;
LOCAL int pattern_cnt;


// ----------------------------------------------------------------------------

satic void GPIO_BTN_Handle (TEvent arg, UChar gpio_btn){
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

    GPIO_BTN_Handle(EVENT_BTN3, 0);
    GPIO_BTN_Handle(EVENT_BTN4, 1);
    GPIO_BTN_Handle(EVENT_BTN5, 2);
    GPIO_BTN_Handle(EVENT_BTN6, 3);

}

// ----------------------------------------------------------------------------


GLOBAL Void Number_Handler(Void) {
    if (Event_tst(EVENT_UPDATE_CNT)){
        Event_clr(EVENT_UPDATE_CNT);
            if(TSTBIT());

    }
}

// ----------------------------------------------------------------------------



GLOBAL Void AS1108_Handler(Void) {

}

// ----------------------------------------------------------------------------

GLOBAL Void Handler_init(Void) {

}

