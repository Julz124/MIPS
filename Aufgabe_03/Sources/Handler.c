#include <msp430.h>
#include "Handler.h"
#include "event.h"
#include "UCA0.h"
#include "UCA1.h"
#include "TA0.h"

#define BASE     10                  // base of the number system can be selected between 2 and 16

// data type of a constant function pointer
typedef Void (* VoidFunc)(Void);

LOCAL Int   pattern_cnt;            // counter for blink pattern from task 1
LOCAL UChar *seg_val;           // identify the external BCD Button that was pressed
LOCAL UChar seg_vals[DIGISIZE];      // BCD counter
LOCAL Char  bcd_uart[DIGISIZE + 3]; // BCD counter array for UART TX (2 additional chars for '\r' and '\n')

LOCAL UChar state;               // function pointer to the current state function
LOCAL UChar idx;                     // index for the BCD counter

LOCAL UInt error;                   // error variable for UART

// ---------------------------------------------------------------------------- Button Handling

static void BCD_Button_Handler(TEvent arg, UChar bcd_button){
    if(Event_tst(arg)) {
        Event_clr(arg);                 // clear the regarding button event
        seg_val = &seg_vals[bcd_button];      // set the button index
        Event_set(EVENT_UPDATE_CNT);    // set event for updating BCD
    }
}

GLOBAL Void Button_Handler(Void) {

    if (Event_tst(EVENT_BTN2)) {
        Event_clr(EVENT_BTN2);
        if (++pattern_cnt GT MUSTER6) {
            pattern_cnt = MUSTER1;
         }
         set_blink_muster(pattern_cnt);
    }

    if (Event_tst(EVENT_BTN1)) {
        Event_clr(EVENT_BTN1);
        TGLBIT(P2OUT, BIT7);
    }

    BCD_Button_Handler(EVENT_BTN3, 0);
    BCD_Button_Handler(EVENT_BTN4, 1);
    BCD_Button_Handler(EVENT_BTN5, 2);
    BCD_Button_Handler(EVENT_BTN6, 3);

}

// ---------------------------------------------------------------------------- Number Handling

GLOBAL Void Number_Handler(Void) {
    if (Event_tst(EVENT_UPDATE_CNT)){
        Event_clr(EVENT_UPDATE_CNT);

        if(!TSTBIT(P2OUT, BIT7)){ //increment
            *seg_val += 1;

            if (*seg_val == BASE) {
                *seg_val = 0;
                seg_val++;
                Event_set(EVENT_UPDATE_CNT);
                return;
            }

        } else { //decrement
            *seg_val -= 1;

            if (*seg_val >= BASE) {
                *seg_val = BASE - 1;
                seg_val++;
                Event_set(EVENT_UPDATE_CNT);
                return;
            }
        }
        if(!Event_tst(EVENT_UPDATE_CNT)) {
            Event_set(EVENT_UPDATE_SEG);
        }
    }
}

// ---------------------------------------------------------------------------- BCD Handling


GLOBAL Void AS1108_Handler(Void) {
    if (state == 0) {
        if (Event_tst(EVENT_UPDATE_SEG)) {
            Event_clr(EVENT_UPDATE_SEG);
            idx = 1;
            state = 1;
            Event_set(EVENT_DONE_SEG);
        }
    } else if (state == 1) {
        if (Event_tst(EVENT_DONE_SEG)) {
            Event_clr(EVENT_DONE_SEG);
            if (idx <= DIGISIZE) {
                UChar ch = seg_vals[(UInt) idx - 1];
                UCA1_emit(idx, ch);
                idx++;
            } else {
                state = 0;
                Event_set(EVENT_TXD);
            }
        }
    }
}

GLOBAL Void get_bcd_cnt(Void) {

    bcd_uart[0] = seg_vals[3] + '0';
    bcd_uart[1] = seg_vals[2] + '0';
    bcd_uart[2] = seg_vals[1] + '0';
    bcd_uart[3] = seg_vals[0] + '0';
    bcd_uart[4] = '\r';
    bcd_uart[5] = '\n';
    bcd_uart[6] = '\0';
}

// ---------------------------------------------------------------------------- UART Handling

GLOBAL Void UART_Handler(Void) {

    if(Event_tst(EVENT_RXD)) {
        Event_clr(EVENT_RXD);
        seg_vals[0] = rx_buf[3] - '0';
        seg_vals[1] = rx_buf[2] - '0';
        seg_vals[2] = rx_buf[1] - '0';
        seg_vals[3] = rx_buf[0] - '0';
        Event_set(EVENT_UPDATE_SEG);
    }

    if(Event_tst(EVENT_TXD)) {
        Event_clr(EVENT_TXD);
        get_bcd_cnt();
        UCA0_printf(bcd_uart);
    }
}

// ---------------------------------------------------------------------------- Error Handling

GLOBAL Void Error_Handler(Void) {

    if(Event_tst(EVENT_ERR)) {
        Event_clr(EVENT_ERR);

        // error handling
        if       (error == BREAK_ERROR) {
            set_blink_muster(MUSTER3);
        } else if(error == FROVPAR_ERROR) {
            set_blink_muster(MUSTER6);
        } else if(error == CHARACTOR_ERROR) {
            set_blink_muster(MUSTER5);
        } else if(error == BUFFER_ERROR) {
            set_blink_muster(MUSTER4);
        } else {
            set_blink_muster(MUSTER1);
        }
    }
}

GLOBAL Void set_error(UChar err) {
    error = err;
    Event_set(EVENT_ERR);
}

// ---------------------------------------------------------------------------- Initialisation

GLOBAL Void Handler_init(Void) {
    pattern_cnt = MUSTER1;  // Should not be changed (because of initialisation)
    state = 0;         // initial state
    idx = 1;                // initial index
    
    seg_vals[0] = 0;         // initial BCD counter
    seg_vals[1] = 0;
    seg_vals[2] = 0;
    seg_vals[3] = 0;
}

