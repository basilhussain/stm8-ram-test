/*******************************************************************************
 *
 * ram_test_march_c.c - March-C RAM test implementation
 *
 * Copyright (c) 2023 Basil Hussain
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ******************************************************************************/

#include "ram_test.h"

// Uncomment to do the more efficient 'minus' (aka 'C-') version of the March C
// test which skips the 4th 'r0' phase.
// #define MARCH_C_MINUS

#ifdef STM8L
#define IWDG_KR 0x50E0
#define IWDG_PR 0x50E1
#define WWDG_CR 0x50D3
#else
#define IWDG_KR 0x50E0
#define IWDG_PR 0x50E1
#define WWDG_CR 0x50D1
#endif

unsigned char ram_test_march_c_impl(void) __naked {
	__asm
#ifdef WATCHDOG
		; In case IWDG and WWDG are enabled at reset, re-configure them so they
		; will not time-out during testing, but not for an excessively long
		; period. Set the IWDG divider to /64 (for period of 255 ms on S/AF, 431
		; ms on L/AL) and refresh the WWDG counter (for period of 393 ms).
		mov IWDG_KR, #0x55
		mov IWDG_PR, #0x04
		mov IWDG_KR, #0xAA
		mov WWDG_CR, #0x7F
#endif
	
#ifdef __SDCC_MODEL_LARGE
		; Return address on stack is 3 bytes. Save the MSB in A reg and the two
		; LSBs in Y reg.
		pop a
		popw y
#else
		; Save the 16-bit return address from the stack in Y reg, because
		; otherwise it will be overwritten.
		popw y
#endif

		ldw x, #RAM_END

	0001$:
		; Write zeroes, descending order (order not important here).
		clr (x)
		decw x
		jrpl 0001$

		clrw x

	0002$:
		; Read zeroes, write ones, ascending order.
		; Fail if a read value is not zero. The ones are written by simply
		; inverting the existing zero value.
		tnz (x)
		jrne 9999$
		cpl (x)
		incw x
		cpw x, #RAM_END
		jrule 0002$

		clrw x

	0003$:
		; Read ones, write zeroes, ascending order.
		; The write is accomplished by inverting the existing ones value. A
		; separate test is then made to verify the actual result written (versus
		; relying on Z flag from the complement). If it is not zero, we fail.
		cpl (x)
		tnz (x)
		jrne 9999$
		incw x
		cpw x, #RAM_END
		jrule 0003$

#ifndef MARCH_C_MINUS
		ldw x, #RAM_END

	0004$:
		; Read zeroes, descending order (order not important here).
		tnz (x)
		jrne 9999$
		decw x
		jrpl 0004$
#endif

		ldw x, #RAM_END

	0005$:
		; Read zeroes, write ones, descending order.
		; Write is accomplished by inverting the existing zeroes values.
		tnz (x)
		jrne 9999$
		cpl (x)
		decw x
		jrpl 0005$

		ldw x, #RAM_END

	0006$:
		; Read ones, write zeroes, descending order.
		; The write is accomplished by inverting the existing ones value. A
		; separate test is then made to verify the actual result written (versus
		; relying on Z flag from the complement). If it is not zero, we fail.
		cpl (x)
		tnz (x)
		jrne 9999$
		decw x
		jrpl 0006$

		clrw x

	0007$:
		; Read zeroes, ascending order.
		tnz (x)
		jrne 9999$
		incw x
		cpw x, #RAM_END
		jrule 0007$

#ifdef WATCHDOG
		; Refresh the IWDG and WWDG once more.
		mov IWDG_KR, #0xAA
		mov WWDG_CR, #0x7F
#endif

#ifdef __SDCC_MODEL_LARGE
		; Restore the 3-byte return address on to the stack and return.
		pushw y
		push a
		clr a
		retf
#else
		; Put the 16-bit return address back on to the stack and return.
		pushw y
		clr a
		ret
#endif

	9999$:
		; Upon failure, perform a software reset by executing an illegal opcode.
		.db 0x75
	__endasm;
}
