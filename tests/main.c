// Runs one test per invocation. Every test starts from a page this program maps for itself and
// the offset 0x0100000000000000, which sets bit 56 and so changes the page address's top byte
// from 00 to 01 and nothing else.
//
//   valid     the same steps with offset 0, which must work everywhere
//   add       ADDPT the offset and print the result without using it
//   read      read through that result
//   load      load with the offset inside the load instruction
//   write     write through that result
//   store     store with the offset inside the store instruction
//   library   the load test, in a library that has only an arm64e slice
#include <dlfcn.h>
#include <inttypes.h>
#include <libproc.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// From checks.S.
extern uintptr_t add(uintptr_t base, uintptr_t offset);
extern unsigned load(uintptr_t base, uintptr_t offset);
extern void store(uintptr_t base, uintptr_t offset, unsigned value);

static const char *slice(void)
{
    switch (_dyld_get_image_header(0)->cpusubtype & ~CPU_SUBTYPE_MASK) {
    case CPU_SUBTYPE_ARM64_ALL: return "arm64";
    case CPU_SUBTYPE_ARM64E: return "arm64e";
    case CPU_SUBTYPE_ARM64E_X1: return "arm64e.x1";
    default: return "other";
    }
}

int main(int argc, char **argv)
{
    const char *test = argc > 1 ? argv[1] : "valid";
    setbuf(stdout, NULL);

    struct proc_bsdinfo info = {0};
    proc_pidinfo(getpid(), PROC_PIDTBSDINFO, 0, &info, sizeof(info));
    printf("slice %s, process flags 0x%08x\n", slice(), info.pbi_flags);

    size_t size = (size_t)getpagesize();
    unsigned char *page = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (page == MAP_FAILED)
        return 1;
    page[0] = 42;

    uintptr_t base = (uintptr_t)page;
    uintptr_t offset = strcmp(test, "valid") ? UINT64_C(0x0100000000000000) : 0;
    uintptr_t result = add(base, offset);
    printf("page   0x%016" PRIxPTR "\n", base);
    printf("offset 0x%016" PRIxPTR "\n", offset);
    printf("ADDPT  0x%016" PRIxPTR "\n", result);

    unsigned (*library_load)(uintptr_t, uintptr_t) = NULL;
    if (!strcmp(test, "valid") || !strcmp(test, "library")) {
        void *library = dlopen("@executable_path/library.dylib", RTLD_NOW);
        library_load = library ? dlsym(library, "library_load") : NULL;
        if (!library_load) {
            fprintf(stderr, "couldn't load library.dylib\n");
            return 1;
        }
    }

    if (!strcmp(test, "valid")) {
        printf("read %u, load %u, library %u\n",
            load(result, 0), load(base, offset), library_load(base, offset));
    } else if (!strcmp(test, "add")) {
        puts("not used");
    } else if (!strcmp(test, "read")) {
        printf("read %u\n", load(result, 0));
    } else if (!strcmp(test, "load")) {
        printf("load %u\n", load(base, offset));
    } else if (!strcmp(test, "write")) {
        store(result, 0, 7);
        puts("wrote");
    } else if (!strcmp(test, "store")) {
        store(base, offset, 7);
        puts("stored");
    } else if (!strcmp(test, "library")) {
        printf("library %u\n", library_load(base, offset));
    } else {
        fprintf(stderr, "unknown test %s\n", test);
        return 1;
    }
    puts("exited normally");
    return 0;
}
