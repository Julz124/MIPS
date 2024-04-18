#include <msp430.h>
#include "..\base.h"
#include "TA1.h"
#include "event.h"

typedef struct {
    volatile int counter;   // Counter im RAM
    volatile enum {
        BTN_IDLE,
        BTN_PRESSED
    } state;
} Ram;

// Definition der Button-Konfigurationsstruktur
typedef struct {
    const char *port_id;   // Port ID im Flash-Speicher
    int bit_position;       // Bitposition im Flash-Speicher
    int n;                  // N-Wert im Flash-Speicher
    int event;              // Event im Flash-Speicher
    Ram * ram; // State im RAM
} ButtonConfig;

Ram ram_button1 = {
    .counter = 0,
    .state = BTN_IDLE
};

Ram ram_button2 = {
    .counter = 0,
    .state = BTN_IDLE
};

// Definition der Button-Konfigurationen im Flash-Speicher
const ButtonConfig btn1_config = {
    .port_id = "BTN1",
    .bit_position = BIT0,
    .n = 1,
    .event = 1,
    .ram = &ram_button1,
};

const ButtonConfig btn2_config = {
    .port_id = "BTN2",
    .bit_position = BIT1,
    .n = 2,
    .event = 2,
    .ram = &ram_button2,
};

// Pointer auf die Button-Konfigurationen im Flash-Speicher
const ButtonConfig *const btn1_ptr = &btn1_config;
const ButtonConfig *const btn2_ptr = &btn2_config;

#pragma FUNC_ALWAYS_INLINE(TA1_init)
GLOBAL Void TA1_init(Void) {

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

    // Prüfe, welcher Taster den Interrupt ausgelöst hat (BTN1 oder BTN2)
    if (TSTBIT(BTN, btn1_ptr->bit_position) == 1) {
        // Button 1 hat den Interrupt ausgelöst
        if (btn1_ptr->ram->state == BTN_IDLE) {
            // Der Button war im Ruhezustand (idle), jetzt ist er gedrückt
            btn1_ptr->ram->state = BTN_PRESSED;
            if (btn1_ptr->bit_position EQ 0) {
                if (btn1_ptr->ram->counter EQ 0) {
                    btn1_ptr->ram->state = BTN_IDLE;
                } else {
                    btn1_ptr->ram->counter -= 1;
                }
            } else {
                if (btn1_ptr->ram->counter LT N-1) {
                    btn1_ptr->ram->counter += 1;
                } else {
                    if (btn1_ptr->ram->state EQ BTN_IDLE) {
                        btn1_ptr->ram->state = BTN_PRESSED;
                        Event();
                    }
                }
            } // Starte den Entprell-Counter
        }
        P1IFG &= ~btn1_ptr->bit_position; // Lösche das Interrupt-Flag für BTN1
    }


    if (P1IFG & btn2_ptr->bit_position) {
        // Button 2 hat den Interrupt ausgelöst
        if (btn2_ptr->ram->state == BTN_IDLE) {
            // Der Button war im Ruhezustand (idle), jetzt ist er gedrückt
            btn2_ptr->ram->state = BTN_PRESSED;
            btn2_ptr->ram->counter = 0; // Starte den Entprell-Counter
        }
        P1IFG &= ~btn2_ptr->bit_position; // Lösche das Interrupt-Flag für BTN2
    }
}
