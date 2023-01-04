#include "ram_test.h"

unsigned char __sdcc_external_startup(void) {
	// These are actually macros that jump directly to the test routine. When
	// that returns, this means it effectively does so directly from
	// __sdcc_external_startup() itself. Anything below the test is never
	// executed; the 'return' statement is just to avoid a compiler warning.
	ram_test_march_c();
	// ram_test_checkerboard();
	return 0;
}

void main(void) {
	while(1);
}

void dummy_isr(void) __interrupt(29) {
	// Not relevant to RAM tests; purpose is to pad interrupt vector table to
	// maximum possible size and determine largest likely return address from
	// __sdcc_external_startup().
}
