#include <stdio.h>
#include <stdlib.h>

#include "flash.h"
//#include "block.h"



int main(void) {
	fprintf(stderr, "Flash based data logger.\n");

	flash_test();

	return EXIT_SUCCESS;
}
