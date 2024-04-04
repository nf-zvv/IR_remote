#include "IR_NEC_receiver.h"

volatile uint8_t tick_counter;
volatile uint8_t ir_address;
volatile uint8_t ir_inv_address;
volatile uint8_t ir_command;
volatile uint8_t ir_inv_command;
volatile uint8_t bit_counter;
volatile uint8_t ir_flags;

void IR_listen_start(void) {
    // INT1 init
    EIFR = (1<<INTF1); // Сброс флага прерывания
    EICRA = (1<<ISC11)|(0<<ISC10); // INT1 IRQ: Falling Edge
    EIMSK = (1<<INT1); // INT1 IRQ Enable
    // Timer 0 init
    TIFR0 = (1<<TOV0);
    TIMSK0 = (1<<TOIE0);
}

void IR_listen_stop(void) {
    // Disable INT1 interrupt
    EICRA = 0;
    EIMSK = 0;
    // Disable Timer 0 interrupt
    TCCR0B = 0;
    TIMSK0 = 0;
    TCNT0 = 0;
    // Clearing
    ir_flags = 0;
    bit_counter = 0;
    tick_counter = 0;
}

uint8_t IR_get_address(void) {
    return ir_address;
}

uint8_t IR_get_command(void) {
    return ir_command;
}

uint8_t IR_available(void) {
    if (ir_flags & (1 << ir_cmd_ok)) {
        ir_flags &= ~(1 << ir_cmd_ok); //сброс флага
        return 1;
    }
    else
        return 0;
}
