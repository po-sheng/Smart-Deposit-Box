	.syntax unified
	.cpu cortex-m4
	.thumb
.data
	.global delay_us
	.global max7219_init
	.global max7219_send
	.type delay_us, %function
	.type max7219_init, %function
	.type max7219_send, %function

.text
	.equ DECODE_MODE,	0x09
	.equ DISPLAY_TEST,	0x0F
	.equ SCAN_LIMIT, 	0x0B
	.equ INTENSITY,		0x0A
	.equ SHUTDOWN,		0x0C

	.equ DATA, 			0x20
	.equ LOAD,			0x40
	.equ CLOCK,			0x80

	.equ GPIOA_BASE, 0x48000000
	.equ GPIO_BSRR_OFFSET, 0x18
	.equ GPIO_BRR_OFFSET, 0x28

delay_us:
adds r0, #1
subs r0, #2
bne delay_us
bx lr

max7219_send:
	push {r4, r5, r6, r7, r8, r9, r10, r11}
	lsl r0, r0, #8
	add r0, r0, r1
	ldr r1, =#GPIOA_BASE
	ldr r2, =#LOAD
	ldr r3, =#DATA
	ldr r4, =#CLOCK
	ldr r5, =#GPIO_BSRR_OFFSET
	ldr r6, =#GPIO_BRR_OFFSET
	mov r7, #16

max7219_loop:
	mov r8, #1
	sub r9, r7, #1
	lsl r8, r8, r9
	str r4, [r1, r6]
	tst r0, r8
	beq bit_not_set
	str r3, [r1, r5]
	b if_done

bit_not_set:
	str r3, [r1, r6]

if_done:
	str r4, [r1, r5]
	subs r7, r7, #1
	bgt max7219_loop
	str r2, [r1, r6]
	str r2, [r1, r5]
	pop {r4, r5, r6, r7, r8, r9, r10, r11}
	bx lr

max7219_init:
	push {r4, r5, r6, r7, r8, r9, r10, r11, lr}
	ldr r0, =#DECODE_MODE
	ldr r1, =#0x0
	bl max7219_send
	ldr r0, =#DISPLAY_TEST
	ldr r1, =#0x0
	bl max7219_send
	ldr r0, =#SCAN_LIMIT
	ldr r1, =#0x7
	bl max7219_send
	ldr r0, =#INTENSITY
	ldr r1, =#0xA
	bl max7219_send
	ldr r0, =#SHUTDOWN
	ldr r1, =#0x1
	bl max7219_send
	pop {r4, r5, r6, r7, r8, r9, r10, r11, pc}
	bx lr

