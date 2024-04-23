#include "../base.h"
#include "event.h"

#ifndef TA1_H_
#define TA1_H_

typedef enum {
    S0,
    S1
}State;

typedef struct {
    const UInt      pin;
    const TEvent    event;
    const Char *    port;
}button_const;

typedef struct {
    Char            cnt;
    State           state;
}button_var;

typedef struct {
    button_const const*   btn_const;
    button_var*     btn_var;
}Button;

EXTERN Void TA1_init(Void);

#endif /* TA1_H_ */

