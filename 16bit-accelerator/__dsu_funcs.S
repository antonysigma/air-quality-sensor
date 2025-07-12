/*
 * Enable DSU LIBGCC.A functions
 * Makes NO difference to 16x16 Multiply because GCC inlines
 * Makes little difference to 32x32 Multiply
 * Dramatic for 32/32 Divide
 *
 * Internal functions are dependent on LIBGCC version
 * The long, short, signed and unsigned variants from libgcc.a
 * eventually rely on an unsigned primitive e.g. these funcs
 */

#include "dsu_defs.S"
// THESE ARE FOR INTERNAL LIBGCC.A MATH FUNCTIONS

.macro __DSU_FUN name, opcode
.section .text.\name, "ax", @progbits
.global \name
.type \name, @function
\name:
     out DSDX, r26
     out DSDY, r18
     ldi r22, \opcode
     out DSIR, r22
     in  r22, DSAL
     in  r24, DSAH
     ret
.size \name, .-\name
.endm

.macro __DSU_FUN16 name, opcode
.section .text.\name, "ax", @progbits
.global \name
.type \name, @function
\name:
     out DSDX, r24
     out DSDY, r22
     ldi r20, \opcode
     out DSIR, r20
     in  r24, DSAL
     ret
.size \name, .-\name
.endm

.macro __DSU_DIV16 name, opcode
.section .text.\name, "ax", @progbits
.global \name
.type \name, @function
\name:
     out DSAL, r24
     clr r24
     clr r25
     out DSAH, r24
     out DSDY, r22
     ldi r22, \opcode
     out DSIR, r22
     sbis DSCR, DSD1
     rjmp .-4
     in  r22, DSAL
//     in  r24, DSAH
     in  r26, DSDY
//     in  r24, DSSD
     ret
.size \name, .-\name
.endm

.macro __DSU_DIV name, opcode
.section .text.\name, "ax", @progbits
.global \name
.type \name, @function
\name:
     out DSAL, r22
     out DSAH, r24
     out DSDY, r20
     ldi r20, \opcode
     out DSIR, r20
     sbis DSCR, DSD1
     rjmp .-4
     ldi r20, 0b10000101
//     sbic DSCR, DSN
//     out DSIR, r20
     in  r22, DSAL
     in  r24, DSAH
     ret
.size \name, .-\name
.endm

.macro __DSU_MOD name, opcode
.section .text.\name, "ax", @progbits
.global \name
.type \name, @function
\name:
     out DSAL, r22
     out DSAH, r24
     out DSDY, r20
     ldi r20, \opcode
     out DSIR, r20
     sbis DSCR, DSD1
     rjmp .-4
     in  r24, DSDY
     ret
.size \name, .-\name
.endm

.macro __DSU_SHIFT name, opcode
.section .text.\name, "ax", @progbits
.global \name
.type \name, @function
\name:
     out DSAL, r22
     out DSAH, r24
     out DSDY, r20
     ori r20, \opcode
     out DSIR, r20
     in  r22, DSAL
     in  r24, DSAH
     ret
.size \name, .-\name
.endm

.macro __DSU_DIV32 name, opcode
.section .text.\name, "ax", @progbits
.global \name
.type \name, @function
\name:
     out DSAL, r22
     out DSAH, r24
     out DSDY, r18
     ldi r22, \opcode
     out DSIR, r22
     sbis DSCR, DSD1
     rjmp .-4
     in  r18, DSAL
     in  r20, DSAH
     in  r22, DSDY
     ret
.size \name, .-\name
.endm

#if ENABLE_DSU_INTERNAL_FUNCS != 0
/*
 * implement internal functions with these ASM .macros:
 */
__DSU_FUN __umulhisi3, 0b01000100
__DSU_FUN __mulhisi3, 0b01110100
__DSU_DIV16 __udivmodhi4, 0b10110001
__DSU_DIV32 __udivmodsi4, 0b10110001

#endif

/*
 * implement external functions with these ASM .macros:
 */
__DSU_FUN dsu_xmuluu, 0b01000100
__DSU_FUN dsu_xmulsu, 0b01100000
__DSU_FUN dsu_xmulus, 0b01010000
__DSU_FUN dsu_xmulss, 0b01110100
__DSU_FUN16 dsu_muluu, 0b01000100
__DSU_FUN16 dsu_mulsu, 0b01100000
__DSU_FUN16 dsu_mulus, 0b01010000
__DSU_FUN16 dsu_mulss, 0b01110100
__DSU_DIV dsu_div, 0b10110000
__DSU_MOD dsu_mod, 0b10110001
__DSU_SHIFT dsu_shl, 0b11000000
__DSU_SHIFT dsu_shr, 0b11010000
__DSU_SHIFT dsu_shrs, 0b11110000

// operands for __mulsi3: X = r21:r18 and Y = r25:r22
.global dsu_mulsi3
.func dsu_mulsi3
#define X_HI r20
#define X_LO r18
#define Y_HI r24
#define Y_LO r22
dsu_mulsi3:
     out DSDX, X_LO
     out DSDY, Y_HI
     ldi r24, XMULUU
     out DSIR, r24

     out DSDX, X_HI
     out DSDY, Y_LO
     ldi r24, XMADUU
     out DSIR, r24

     ldi r25, SHRA|8
     out DSIR, r25
     out DSIR, r25

     out DSDX, X_LO
     out DSDY, Y_LO
     out DSIR, r24

     in  r22, DSAL
     in  r24, DSAH
     ret
.size dsu_mulsi3, .-dsu_mulsi3
.endfunc

