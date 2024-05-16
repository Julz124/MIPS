#include <msp430.h>
#include "Handler.h"
#include "event.h"
#include "UCA1.h"
#include "TA0.h"

#define DIGISIZE 4
#define BASE     10

typedef Void (* VoidFunc)(Void);
LOCAL Void State0(Void);
LOCAL Void State1(Void);
LOCAL VoidFunc state;

LOCAL int pattern_cnt;  // Aktuelles Pattern

LOCAL char curr_seg_val[DIGISIZE];   //Aktueller Zählwert der 7-Seg Anzeige
LOCAL char* seg_val_idx;
LOCAL VoidFunc state;   // Funktionspointer für 7-Seg Anzeige (Set and Reset)
LOCAL UInt idx;                     // index for the BCD counter

// ----------------------------------------------------------------------------

static void EX_Button_Handler(TEvent arg, UChar ex_button){
    if(Event_tst(arg)) {
        Event_clr(arg);
        *seg_val_idx = curr_seg_val[ex_button];
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

    // Eventhandler External Buttons
    EX_Button_Handler(EX_EVENT_BTN0, 0);
    EX_Button_Handler(EX_EVENT_BTN1, 1);
    EX_Button_Handler(EX_EVENT_BTN2, 2);
    EX_Button_Handler(EX_EVENT_BTN3, 3);

}

// ----------------------------------------------------------------------------


GLOBAL Void Number_Handler(Void) {
    if (Event_tst(EVENT_UPDATE_CNT)){
        Event_clr(EVENT_UPDATE_CNT);

        if(!TSTBIT(P2OUT, BIT7)){ //increment
            *seg_val_idx += 1;

            if (*seg_val_idx == BASE) {
                *seg_val_idx = 0;
                seg_val_idx++ ;
                Event_set(EVENT_UPDATE_CNT);
                return;
            }

        } else { //decrement
            *seg_val_idx -= 1;

            if (*seg_val_idx >= BASE) {
                *seg_val_idx = 0;
                seg_val_idx-- ;
                Event_set(EVENT_UPDATE_CNT);
                return;
            }
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
        if (idx LE DIGISIZE) {
            UChar ch = curr_seg_val[idx - 1];
            //ch += '0';
            UCA1_emit(idx, ch);
            idx++;
        } else {
            state = State0;
        }
    }
}

GLOBAL Void AS1108_Handler(Void) {
    (*state)();
}

// ----------------------------------------------------------------------------

GLOBAL Void Handler_init(Void) {
    pattern_cnt = MUSTER1;
    state = State0;
    idx = 1;

    curr_seg_val[0] = 4;
    curr_seg_val[1] = 3;
    curr_seg_val[2] = 2;
    curr_seg_val[3] = 1;
}

