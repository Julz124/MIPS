#include <msp430.h>
#include "Handler.h"
#include "event.h"
#include "UCA0.h"
#include "UCA1.h"
#include "TA0.h"

#define BASE     10                  // base of the number system can be selected between 2 and 16

// data type of a constant function pointer
typedef Void (* VoidFunc)(Void);

LOCAL UChar seg_val;           // identify the external BCD Button that was pressed
LOCAL UChar seg_vals[DIGISIZE];      // BCD counter
LOCAL Char  bcd_uart[DIGISIZE + 3]; // BCD counter array for UART TX (2 additional chars for '\r' and '\n')

LOCAL UChar state;               // function pointer to the current state function
LOCAL UChar idx;                     // index for the BCD counter

LOCAL UInt error;                   // error variable for UART

// ---------------------------------------------------------------------------- Button Handling

GLOBAL Void Button_Handler(Void) {

    TEvent local_btn_event = get_events(0x003F);

    if (TSTBIT(local_btn_event, EVENT_BTN1)) {
        TGLBIT(P2OUT, BIT7);
        CLRBIT(local_btn_event, EVENT_BTN1);
    }

    if (TSTBIT(local_btn_event, EVENT_BTN3 + EVENT_BTN4 + EVENT_BTN5 + EVENT_BTN6)) {
            seg_val = (local_btn_event >> 2) & 0xF;
            CLRBIT(local_btn_event, EVENT_BTN3 + EVENT_BTN4 + EVENT_BTN5 + EVENT_BTN6);
            Event_set(EVENT_UPDATE_CNT);
        }
}

// ---------------------------------------------------------------------------- Number Handling

GLOBAL Void Number_Handler(Void) {

    TEvent local_event = get_events(0x0800);

    if (TSTBIT(local_event, EVENT_UPDATE_CNT)){
        CLRBIT(local_event, EVENT_UPDATE_CNT);

        if(!TSTBIT(P2OUT, BIT7)){ //increment
            seg_vals[0] += seg_val & 0x1;
            seg_vals[1] += (seg_val >> 1) & 0x1;
            seg_vals[2] += (seg_val >> 2) & 0x1;
            seg_vals[3] += (seg_val >> 3) & 0x1;
            seg_val = 0x00;

            if (seg_vals[0] == BASE) {
                seg_vals[0] = 0;
                seg_vals[1] += 1;
            }
            if (seg_vals[1] == BASE) {
                seg_vals[1] = 0;
                seg_vals[2] += 1;
            }
            if (seg_vals[2] == BASE) {
                seg_vals[2] = 0;
                seg_vals[3] += 1;
            }
            if (seg_vals[3] == BASE) {
                seg_vals[3] = 0;
            }

        } else { //decrement
            seg_vals[0] -= seg_val & 0x1;
            seg_vals[1] -= (seg_val >> 1) & 0x1;
            seg_vals[2] -= (seg_val >> 2) & 0x1;
            seg_vals[3] -= (seg_val >> 3) & 0x1;
            seg_val = 0x00;

            if (seg_vals[0] >= BASE) {
                seg_vals[0] = 9;
                seg_vals[1] -= 1;
            }
            if (seg_vals[1] >= BASE) {
                seg_vals[1] = 9;
                seg_vals[2] -= 1;
            }
            if (seg_vals[2] >= BASE) {
                seg_vals[2] = 9;
                seg_vals[3] -= 1;
            }
            if (seg_vals[3] >= BASE) {
                seg_vals[3] = 9;
            }
        }
        if(!Event_tst(EVENT_UPDATE_CNT)) {
            Event_set(EVENT_UPDATE_SEG);
        }
    }
}


// ---------------------------------------------------------------------------- BCD Handling

GLOBAL Void AS1108_Handler(Void) {

    TEvent local_event = get_events(0x0600);

    if (state == 0) {
        if (TSTBIT(local_event, EVENT_UPDATE_SEG)) {
            CLRBIT(local_event, EVENT_UPDATE_SEG);
            idx = 1;
            state = 1;
            Event_set(EVENT_DONE_SEG);
        }
    } else if (state == 1) {
        if (TSTBIT(local_event, EVENT_DONE_SEG)) {
            CLRBIT(local_event, EVENT_DONE_SEG);
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

    TEvent local_event = get_events(0x00C0);

    if(TSTBIT(local_event, EVENT_RXD)) {
        CLRBIT(local_event, EVENT_RXD);
        seg_vals[0] = rx_buf[3] - '0';
        seg_vals[1] = rx_buf[2] - '0';
        seg_vals[2] = rx_buf[1] - '0';
        seg_vals[3] = rx_buf[0] - '0';
        Event_set(EVENT_UPDATE_SEG);
    }

    if(TSTBIT(local_event, EVENT_TXD)) {
        CLRBIT(local_event, EVENT_TXD);
        get_bcd_cnt();
        UCA0_printf(bcd_uart);
    }
}

// ---------------------------------------------------------------------------- Error Handling

GLOBAL Void Error_Handler(Void) {

    TEvent local_event = get_events(0x8000);

    if(TSTBIT(local_event, EVENT_ERR)) {
        CLRBIT(local_event, EVENT_ERR);

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
    state = 0;
    idx = 1;
    
    seg_vals[0] = 0;
    seg_vals[1] = 0;
    seg_vals[2] = 0;
    seg_vals[3] = 0;
}

