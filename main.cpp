#include "mbed.h"

#include "libhydrogen/hydrogen.h"

// main() runs in its own thread in the OS
int main()
{
    printf("IOTA example\n");

#ifdef MBED_MAJOR_VERSION
    printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif
    int ret = hydro_init();
    char buf[100] = {};
    printf("ret : %d\n", ret);
    hydro_random_buf(buf, sizeof(buf));
    for(size_t i = 0; i < 100; i++){
        printf("0x%x", buf[i]);
    }
    // printf("i32 : %d\n", (int)hydro_random_u32());
    printf("\nDone\n\n");

    while (true) {

    }
}

