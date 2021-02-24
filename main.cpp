#include "mbed.h"

// main() runs in its own thread in the OS
int main()
{
    printf("WiFi example\n");

#ifdef MBED_MAJOR_VERSION
    printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif

    printf("\nDone\n");

    while (true) {

    }
}

