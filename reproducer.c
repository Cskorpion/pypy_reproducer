#include <link.h>
#include <stdlib.h>
#include <stdio.h>

static void* unw_local_addr_space = NULL;
static int (*unw_get_proc_name_by_ip)(void *, u_int64_t, char *, size_t, u_int64_t *, void *) = NULL;

int main(void) {
    // This code should reproduce the segfault occuring when using vmprof to profile code on pypy mentioned in https://github.com/pypy/pypy/issues/5186
    // The segfault occurs when trying to resolve the name of a function, that is not exposed in the symboltable of its correspoonding shared object
    // And (so far) only for libffi 

    // Load libunwind, unw_get_proc_name_by_ip and unw_local_addr_space 
    // A self built, recent version of libunwind is required
    void * handle_libunwind = NULL;
    if ((handle_libunwind = dlopen("libunwind.so", RTLD_NOW)) == NULL) {
        printf("couldn't open libunwind \n");
        return 1;
    }
    unw_get_proc_name_by_ip = dlsym(handle_libunwind, "_ULx86_64_get_proc_name_by_ip");
    unw_local_addr_space = dlsym(handle_libunwind, "_ULx86_64_local_addr_space");
    unw_local_addr_space = *((void**) unw_local_addr_space); 


    // Load libffi
    // Running this code with an e.g. apt or yum installed version of libffi will NOT cause a segfault
    // This segfaults only when using PyPy's libffi.so that was built on a buildbot
    // Set the path in dlopen to /path/to/your/nightly-pypy/lib/libffi.so.6
    void * handle_libffi = NULL;
    if ((handle_libffi = dlopen("/path/to/your/nightly-pypy/lib/libffi.so.6", RTLD_NOW)) == NULL) {
        printf("couldn't open libffi \n");
        return 1;
    }
    
    // prepare a buffer for the function name
    char * buffer = malloc(256 * sizeof(char));
    u_int64_t offset = 0;
    // function in libffi, not exposed
    void * func_ptr = (void*) dlsym(handle_libffi, "ffi_prep_cif") + 0x3780;

    // libunwind tries to access the gnu hash table of libffi.so,
    // but something goes wrong when accessing the gnu hash to find the number of buckets
    // segfault at libunwind/src/elfxx.c:178
    if (unw_get_proc_name_by_ip(unw_local_addr_space, (u_int64_t) func_ptr, buffer, 256, &offset, NULL) == 0) {
        printf("found name %s \n", buffer);
    } 

    free(buffer);
} 
