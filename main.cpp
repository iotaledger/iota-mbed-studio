#include "mbed.h"

#include "hydrogen.h"
#include "cJSON.h"
#include "utarray.h"
#include "blake2.h"

// main() runs in its own thread in the OS
int main()
{
    printf("IOTA example\n");

#ifdef MBED_MAJOR_VERSION
    printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif

    // test hydrogen
    int ret = hydro_init();
    char buf[100] = {};
    printf("hydro_init : %d\n", ret);
    hydro_random_buf(buf, sizeof(buf));
    for(size_t i = 0; i < sizeof(buf); i++){
        printf("%X", buf[i]);
    }
    printf("\n");

    // test cjson 
    cJSON *monitor = cJSON_CreateObject();
    if (monitor == NULL){
        printf("cjson create object failed\n");
    }else{
        cJSON_free(monitor);
        printf("create and free cjson object done\n");
    }

    //test utarray
    UT_array *nums;
    int i, *p;
    utarray_new(nums,&ut_int_icd);
    for(i=0; i < 10; i++) utarray_push_back(nums,&i);

    for(p=(int*)utarray_front(nums);
        p!=NULL;
        p=(int*)utarray_next(nums,p)) {
        printf("%d",*p);
    }
    printf("\n");
    utarray_free(nums);

    // test blake2b
    char blake2b_sum[32] = {};
    ret = blake2b(blake2b_sum, sizeof(blake2b_sum), buf, sizeof(buf), NULL, 0);
    printf("blake2 : %d\n", ret);
    for(size_t i = 0; i < sizeof(blake2b_sum); i++){
        printf("%02X ", buf[i]);
    }
    printf("\n");

    // printf("i32 : %d\n", (int)hydro_random_u32());
    printf("\nDone\n");

    while (true) {

    }
}

