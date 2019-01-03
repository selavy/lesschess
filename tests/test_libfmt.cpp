#include "fmt/format.h"
#include "fmt/printf.h"

int main(int argc, char**argv) {
    fmt::printf("Hello, %s!", "world\n");
    return 0;
}
