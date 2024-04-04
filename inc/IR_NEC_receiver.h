#ifndef IR_NEC_RECEIVER_H_
#define IR_NEC_RECEIVER_H_

#include <avr/io.h>
#include <stdint.h>

#define start_cond 0
#define repeat_cond 1
#define ir_cmd_ok 2
#define ir_cmd_error 3

/**
 * @brief start listening for IR command
  */ 
void IR_listen_start(void);

/**
 * @brief stop listening for IR command
  */ 
void IR_listen_stop(void);

/**
 * @brief get device address from IR tranciever
 * @return IR device address
  */ 
uint8_t IR_get_address(void);

/**
 * @brief get command from IR tranciever
 * @return IR command
  */ 
uint8_t IR_get_command(void);


/**
 * @brief доступна команда по ИК
 * @return 1 - доступно
  */ 
uint8_t IR_available(void);


#endif /* IR_NEC_RECEIVER_H_ */