#include <stdint.h>
#include "ram_test.h"

#define PC_ODR (*(volatile uint8_t *)(0x500A))
#define PC_ODR_ODR5 5
#define PC_DDR (*(volatile uint8_t *)(0x500C))
#define PC_DDR_DDR5 5
#define PC_CR1 (*(volatile uint8_t *)(0x500D))
#define PC_CR1_C15 5

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
	// Configure GPIO PC5 as an output.
	PC_DDR = (1 << PC_DDR_DDR5);
	PC_CR1 = (1 << PC_CR1_C15);
	
	// Turn GPIO PC5 on to signal main() has been successfully entered, e.g. by
	// illuminating a connected LED, or similar.
	PC_ODR |= (1 << PC_ODR_ODR5);
	
	while(1);
}

void dummy_isr(void) __interrupt(29) {
	// Not relevant to RAM tests; purpose is to pad interrupt vector table to
	// maximum possible size and determine largest likely return address from
	// __sdcc_external_startup().
}
