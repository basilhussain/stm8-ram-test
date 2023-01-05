# Overview

This is a library for the STM8 microcontroller and [SDCC](http://sdcc.sourceforge.net/) compiler providing routines for performing tests that verify proper operation of the internal RAM of the microcontroller, such as may be desired or required when aiming to create fault-tolerant, high-reliability, or safety-critical software.

The RAM testing routines are designed to be called at device startup via SDCC's `__sdcc_external_startup()` function, which is executed by SDCC's initialisation code before `main()` and before global/static variables are initialised. This is necessary because the RAM test, by nature, will overwrite all contents of the entire RAM.

# Requirements

Requires SDCC version 4.2.10 or greater. The `__sdcc_external_startup()` function is not implemented for the STM8 platform in SDCC versions older than this.

# Test Algorithms

Two different test algorithms are provided:

* **Checkerboard**: Fills RAM with alternating bit pattern 01010101 (0x55), then reads back and verifies the values read match the pattern. Repeats for the pattern 10101010 (0xAA).
* **March C**: The entire RAM is alternately checked and filled byte-by-byte with background patterns (value 00000000, 0x00) and inverse background patterns (11111111, 0xFF) in seven loops. The loops are performed in either ascending or descending address order, as dictated by the stage of the algorithm. The read/write loops, and their order, are: write 0 (desc.); read 0, write 1 (asc.); read 1, write 0 (asc.); read 0 (desc.); read 0, write 1 (desc.); read 1, write 0 (desc.); read 0 (asc.) In this manner, set bits are 'marched' across the RAM area.

# Building

Run `make` in the code's root folder. Some arguments may be required; see below. The output `.lib` file is placed in the `lib` folder.

**IMPORTANT**: The code must be compiled using options suited to the particular STM8 device you intend to run it on, in particular with regard to the size of the device's RAM, and also memory model (medium or large).

By default, without specifying any options to `make`, the library compiles for an STM8S003 (or similar) having 1 KB of RAM and using the medium memory model.

To compile for other devices, give one or more of the following options as arguments to `make`:

* `RAM_END=0xXXXX`: Specifies the end address, in hex format, of the device's RAM. See your target device's memory map in the relevant datasheet.
* `MODEL=large`: Compiles with SDCC's `--model-large` argument. If *your* code is compiled with `--model-large`, then you need to use this option. You will probably be using this option if your device has more than 32 KB of flash.

For example, to build for an STM8S208 with 6 KB of RAM and large memory model: `make MODEL=large RAM_END=0x17FF`.

Or, for an STM8S105 with 2 KB of RAM: `make RAM_END=0x07FF`.

# Usage

The test functions should be executed from within `__sdcc_external_startup()` by way of one of the following macros:

* `ram_test_checkerboard()`
* `ram_test_march_c()`

The macro call should be the first and only statement within `__sdcc_external_startup()`. **Do not perform any other actions inside `__sdcc_external_startup()` other than calling a RAM test macro.**

The macros jump directly to the test routine, without using a function call. This means that when the test function returns, it effectively does so as if directly from `__sdcc_external_startup()` itself. It avoids creating a new stack frame, and thus an extra return address on the stack that would need preserving during the test. So, be aware that the effect is that anything following the macro call is never executed. However, you may need to add a dummy `return` statement to avoid a compiler warning.

It is also recommended that you place your definition of `__sdcc_external_startup()` at the top of the file containing `main()` so that it resides at an address in flash as near to 0x8000 as possible.

Example usage:

```c
#include "ram_test.h"

unsigned char __sdcc_external_startup(void) {
	ram_test_march_c();
	return 0;
}

void main(void) {
	while(1) {
		// ...etc...
	}
}
```

When compiling your code, link the library's `.lib` file using the SDCC `-l` option.

## Implementation Details

Should a RAM test function encounter a failure, it will reset the microcontroller by executing an invalid opcode, causing an illegal opcode reset. You can determine after the fact whether this has occurred by checking whether the `ILLOPF` bit in the `RST_SR` register is set to 1. If the failure is permanent or persistent, the microcontroller will effectively remain in a reset loop and not execute the rest of the firmware as normal.

In case either of the STM8's watchdogs (IWDG & WWDG) are enabled from reset by the option bytes, in order to avoid a watchdog time-out (and resultant reset), the RAM test functions re-configure the watchdogs before beginning testing. For the IWDG, the time-out period is increased to maximum (1 second); for the WWDG, it is disabled. Therefore, please be aware that when `main()` is entered, the watchdog registers will *not* have all their default values.

The test functions are able to return, even though ostensibly their return address on the stack (in RAM) is wiped out during the course of the test, because they preserve the return address in CPU registers. After testing is finished, the saved return address is pushed back onto the stack.

Each RAM test function returns a fixed value of zero. The return value is effectively passed through and becomes the return value from `__sdcc_external_startup()`, to indicate global/static variable initialisation should continue and not be skipped (see SDCC manual for details).

# Disclaimer

The author of this library makes no representations or warranties of correctness or fitness for purpose for use in safety-critical, high-reliability, or certified/compliant applications. The user assumes all liability when used in these applications.

In other words: if you intend to use this code to satisfy something like IEC 60730-1 Class B requirements, it is *your* responsibility to understand it and to ensure it works properly.

# Licence

This library is licenced under the MIT Licence. Please see file LICENSE.txt for full licence text.
