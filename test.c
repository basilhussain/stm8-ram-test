#include "ram_test.h"

unsigned char __sdcc_external_startup(void) {
	// return ram_test_checkerboard();
	return ram_test_march_c();
}

void main(void) {
	while(1);
}

void dummy_isr(void) __interrupt(29) {

}
