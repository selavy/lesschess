#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(int argc, char **argv) {
    uint64_t x = ((uint64_t)1) << 61;
    uint64_t v = ((uint64_t)1) << 59;
    x |= v;


    uint64_t y = x & -x;
    uint64_t z = x & (-1 * x);
    printf("%" PRIx64 "\n", x);
    printf("%" PRIx64 "\n", y);
    printf("%" PRIx64 "\n", z);
    return 0;
}
