/* 
 * Обработка ИК сигналов от пультов ДУ по протоколу NEC
 * 
 * (C) 2015-2024 Vitaliy Zinoviev
 * 
 * History
 * =======
 * 02.02.2015 Оптимизация кода
 * 28.03.2024 integrated to avr-gcc project
*/

#include <avr/io.h>

#define __zero_reg__ r1

#define start_cond 0
#define repeat_cond 1
#define ir_cmd_ok 2
#define ir_cmd_error 3


.extern tick_counter
.extern ir_address
.extern ir_inv_address
.extern ir_command
.extern ir_inv_command
.extern bit_counter
.extern ir_flags


			.text
;------------------------------------------------------------------------------
;
;    Внешнее прерывание от ИК-приемника
;
;------------------------------------------------------------------------------
.global INT1_vect
INT1_vect:
			push    r30
			in		r30,_SFR_IO_ADDR(SREG)
			push	r30
			push	r31
			push	r29
			;------------------------------
			
			; запуск таймера
			ldi		r30,160		; 96 мкс - период таймера T0
			out		_SFR_IO_ADDR(TCNT0),r30
			ldi		r30,(0<<CS02)|(1<<CS01)|(0<<CS00) ; Prescaler = 8
			out		_SFR_IO_ADDR(TCCR0B),r30

			lds		r31,tick_counter

			; проверка на стартовый испульс
			; если случилось от 130 до 150 тиков (tick >= 130 && tick < 150)
			LDI		r30,130
			CP		r31,r30
			BRLO	40f ; если меньше 130, переходим на следующее условие
			LDI		r30,150
			CP		r31,r30
			BRSH	40f ; если больше, либо равно 150, переходим на следующее условие
			; тело условия
			lds		r30,ir_flags
			sbr		r30,(1<<start_cond)		; start_cond = 1	фиксируем стартовое условие
			cbr		r30,(1<<repeat_cond)	; repeat_cond = 0	обнуляем флаг повтора
			sts		ir_flags,r30
			
			sts		ir_address,__zero_reg__
			sts		ir_inv_address,__zero_reg__
			sts		ir_command,__zero_reg__
			sts		ir_inv_command,__zero_reg__
			; конец тела условия
			rjmp	99f

40:		; next_cond_1
			; проверка на импульс повтора
			; если случилось от 110 до 130 тиков (tick >= 110 && tick < 130)
			LDI		r30,110
			CP		r31,r30
			BRLO	45f ; если меньше 110, переходим на следующее условие
			LDI		r30,130
			CP		r31,r30
			BRSH	45f ; если больше, либо равно 130, переходим на следующее условие
			; тело условия
			lds		r30,ir_flags
			cbr		r30,(1<<start_cond)		; start_cond = 0	обнуляем флаг стартового условия
			sbr		r30,(1<<repeat_cond)	; repeat_cond = 1	фиксируем флаг повтора
			sts		ir_flags,r30
			out		_SFR_IO_ADDR(TCNT0),__zero_reg__			; TCNT0=0x00   ??????????????
			; конец тела условия
			rjmp	99f
45:		; next_cond_2
			lds		r30,ir_flags
			SBRS	r30,start_cond	; проверяем, было ли стартовое условие
			RJMP	99f
			; проверка на получение "1"
			; если случилось от 19 до 29 тиков (tick >= 19 && tick < 29 && start_cond)
			LDI		r30,19
			CP		r31,r30
			BRLO	65f ; если меньше 19, переходим на следующее условие
			LDI		r30,29
			CP		r31,r30
			BRSH	65f ; если больше, либо равно 29, переходим на следующее условие
			; тело условия
			lds		r29,bit_counter
			inc		r29		; ++b_cnt	приняли "1" и увеличили счетчик битов
			sts		bit_counter,r29
			; === условие (b_cnt < 9) ===
			LDI		r30,9
			CP		r29,r30
			BRSH	50f
			; первый байт - прямой адрес
			; addr_1 = (addr_1 << 1) + 1
			lds		r30,ir_address
			lsl		r30
			inc		r30
			sts		ir_address,r30
50:		; next_subcond1_cond_2
			; === условие (b_cnt >= 9 && b_cnt < 17) ===
			LDI		r30,9
			CP		r29,r30
			BRLO	55f
			LDI		r30,17
			CP		r29,r30
			BRSH	55f
			; второй байт - инверсный адрес
			; addr_0 = (addr_0 << 1) + 1
			lds		r30,ir_inv_address
			lsl		r30
			inc		r30
			sts		ir_inv_address,r30
55:		; next_subcond2_cond_2
			; === условие (b_cnt >= 17 && b_cnt < 25) ===
			LDI		r30,17
			CP		r29,r30
			BRLO	60f
			LDI		r30,25
			CP		r29,r30
			BRSH	60f
			; третий байт - прямая команда
			; cmd_1 = (cmd_1 << 1) + 1
			lds		r30,ir_command
			LSL		r30
			INC		r30
			sts		ir_command,r30
60:		; next_subcond3_cond_2
			; === условие (b_cnt >= 25) ===
			LDI		r30,25
			CP		r29,r30
			BRLO	65f
			; четвертый байт - инверсная команда
			; cmd_0 = (cmd_0 << 1) + 1
			lds		r30,ir_inv_command
			lsl		r30
			inc		r30
			sts		ir_inv_command,r30
			; конец тела условия
			rjmp	85f
65:		;next_cond_3
			; если случилось от 7 до 17 тика (tick >= 7 && tick < 17 && start_cond)
			; проверка на получение "0"
			LDI		r30,7
			CP		r31,r30
			BRLO	85f ; если меньше 7, переходим на следующее условие
			LDI		r30,17
			CP		r31,r30
			BRSH	85f ; если больше, либо равно 17, переходим на следующее условие
			; тело условия
			lds		r29,bit_counter
			inc		r29		; ++b_cnt	приняли "0" и увеличили счетчик битов
			sts		bit_counter,r29
			; First byte - address
			; === условие (b_cnt < 9) ===
			LDI		r30,9
			CP		r29,r30
			BRSH	70f
			; первый байт - прямой адрес
			; addr_1 = (addr_1 << 1)
			lds		r30,ir_address
			lsl		r30
			sts		ir_address,r30
70:			; Second byte - inverse address
			; === условие (b_cnt >= 9 && b_cnt < 17) ===
			LDI		r30,9
			CP		r29,r30
			BRLO	75f
			LDI		r30,17
			CP		r29,r30
			BRSH	75f
			; второй байт - инверсный адрес
			; addr_0 = (addr_0 << 1) 
			lds		r30,ir_inv_address
			lsl		r30
			sts		ir_inv_address,r30
75:			; Third byte - command
			; === условие (b_cnt >= 17 && b_cnt < 25) ===
			LDI		r30,17
			CP		r29,r30
			BRLO	80f
			LDI		r30,25
			CP		r29,r30
			BRSH	80f
			; третий байт - прямая команда
			; cmd_1 = (cmd_1 << 1)
			lds		r30,ir_command
			lsl		r30
			sts		ir_command,r30
80:			; Fourth byte - inverse command
			; === условие (b_cnt >= 25) ===
			LDI		r30,25
			CP		r29,r30
			BRLO	85f
			; четвертый байт - инверсная команда
			; cmd_0 = (cmd_0 << 1)
			lds		r30,ir_inv_command
			lsl		r30
			sts		ir_inv_command,r30
			; конец тела условия
85:			; All bits received
			LDI		r30,32
			CP		r29,r30 ; если приняли уже 4 байта
			BRNE	99f
			; проверка адреса
			lds		r30,ir_address
			com		r30
			lds		r31,ir_inv_address
			cp		r30,r31
			brne	90f
			; проверка команды
			lds		r30,ir_command
			com		r30
			lds		r31,ir_inv_command
			cp		r30,r31
			brne	90f
			; проверка прошла успешно
			lds		r30,ir_flags
			sbr		r30,(1<<ir_cmd_ok)
			rjmp	95f
90:	; Error: IR code sequention is failed
			; проверка не выполняется
			lds		r30,ir_flags
			sbr		r30,(1<<ir_cmd_error)
95:			; Finish: IR code sequention is ended
			cbr		r30,(1<<repeat_cond)	; repeat_cond = 0	обнуляем флаг повтора
			cbr		r30,(1<<start_cond)		; start_cond = 0	обнуляем флаг стартового условия
			sts		ir_flags,r30

			sts		bit_counter,__zero_reg__
			out		_SFR_IO_ADDR(TCNT0),__zero_reg__			; TCNT0=0x00     ?????????????????
			out		_SFR_IO_ADDR(TCCR0B),__zero_reg__			; TCCR0B=0x00 выключение таймера
99:
			; обнулили тики tick = 0
			sts		tick_counter,__zero_reg__
			;------------------------------
			pop		r29
			pop		r31
			pop		r30
			out		_SFR_IO_ADDR(SREG),r30
			pop     r30
			reti



;------------------------------------------------------------------------------
; Переполнение таймера T0
; подсчет длительности интервалов ИК сигналов
;------------------------------------------------------------------------------
.global TIMER0_OVF_vect
TIMER0_OVF_vect:
			push    r30
			in		r30,_SFR_IO_ADDR(SREG)
			push	r30
			;------------------------------
			; переинициализировали таймер - отсчитывает 96мкс до переполнения
			ldi		r30,160
			out		_SFR_IO_ADDR(TCNT0),r30

			; tick + 1
			lds		r30,tick_counter
			inc		r30
			sts		tick_counter, r30

			cpi		r30,255
			brlo	99f
			; сбросили все к исходному состоянию
			clr		r30
			out		_SFR_IO_ADDR(TCCR0B),r30			; TCCR0B=0x00
			out		_SFR_IO_ADDR(TCNT0),r30			; TCNT0=0x00      ???????????????????
			sts		tick_counter,r30

			lds		r30,ir_flags
			cbr		r30,(1<<repeat_cond)  ; repeat_cond = 0	обнуляем флаг повтора
			cbr		r30,(1<<start_cond)   ; start_cond = 0	обнуляем флаг стартового условия
			sts		ir_flags,r30
99:
			;------------------------------
			pop		r30
			out		_SFR_IO_ADDR(SREG),r30
			pop     r30
			reti

.end
