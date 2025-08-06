# libc-redacted

A truly portable subset of the C standard library that contains only functions implementable with inline assembly on x86. No syscalls, no linking dependencies - just pure, portable C code.

## Philosophy

libc-redacted follows a strict rule: if it can't be implemented with a few lines of inline x86 assembly, it's not included. This means:

**✅ Included:**
- Memory operations (`memcpy`, `memset`, `memmove`, `memcmp`)
- Basic string functions (`strlen`, `strcmp`, `strcpy`, `strcat`)
- Character classification (`isalpha`, `isdigit`, `tolower`, `toupper`)
- Basic arithmetic utilities
- Bit manipulation functions

**❌ Excluded:**
- I/O functions (`printf`, `scanf`, `fopen`, etc.) - require syscalls
- Mathematical functions (`sin`, `cos`, `sqrt`, etc.) - too complex for inline ASM
- Memory allocation (`malloc`, `free`) - require heap management
- Threading primitives - require OS support
- Network functions - require syscalls

## Design Goals

1. **Zero Dependencies**: No linking to external libraries
2. **No Syscalls**: Everything implemented in userspace
3. **Portable**: Works on any x86 system with a C compiler
4. **Minimal**: Only the most essential functions
5. **Performance**: Optimized inline assembly implementations

## Usage

libc-redacted is **header-only**! Just include the single header:

```c
#include "libc-redacted.h"
#include <stdio.h>  // Only for printf in examples

int main() {
    char buf[100];
    const char* src = "Hello, World!";
    
    // String operations
    strcpy(buf, src);
    size_t len = strlen(buf);
    int cmp = strcmp(buf, src);
    
    // Math operations
    double x = sqrt(16.0);      // Uses x87 fsqrt instruction
    int n = abs(-42);           // Simple conditional
    double y = floor(3.7);      // x87 rounding with mode control
    
    printf("sqrt(16) = %f, abs(-42) = %d, floor(3.7) = %f\n", x, n, y);
    
    return 0;
}
```

## Building

No build required! Just compile directly:

```bash
gcc -O2 -o myprogram myprogram.c
```

Or test the included example:
```bash
make test
```

## License

MIT License - See LICENSE file for details.
