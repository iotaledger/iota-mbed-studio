#include "mbed.h"

#include "hydrogen.h"
#include "cJSON.h"
#include "utarray.h"

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
    printf("ret : %d\n", ret);
    hydro_random_buf(buf, sizeof(buf));
    for(size_t i = 0; i < 100; i++){
        printf("0x%x", buf[i]);
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

    // printf("i32 : %d\n", (int)hydro_random_u32());
    printf("\nDone\n\n");

    while (true) {

    }
}

