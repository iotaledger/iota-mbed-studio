/*
	a custom randombytes must implement:

	void ED25519_FN(ed25519_randombytes_unsafe) (void *p, size_t len);

	ed25519_randombytes_unsafe is used by the batch verification function
	to create random scalars
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void ED25519_FN(ed25519_randombytes_unsafe) (void *p, size_t len){
	// TODO use (T)RNG or mbed PSA
	uint8_t num = 0;
	srand((unsigned int)time(NULL));
	 for (size_t l = 0; l < len; l++) {
    num = (uint8_t)rand();
		memcpy(p+l, &num, sizeof(uint8_t));
  }
}
