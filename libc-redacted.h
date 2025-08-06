#ifndef LIBC_REDACTED_H
#define LIBC_REDACTED_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Memory functions */
static inline void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    char* restrict d = (char* restrict)dest;
    const char* restrict s = (const char* restrict)src;
    
    #ifdef __x86_64__
    /* Use optimized word/dword copy for aligned data, then handle remainder */
    if (n >= 8 && ((uintptr_t)d & 7) == 0 && ((uintptr_t)s & 7) == 0) {
        size_t words = n / 8;
        __asm__ volatile (
            "rep movsq"
            : "=D" (d), "=S" (s), "=c" (words)
            : "0" (d), "1" (s), "2" (words)
            : "memory"
        );
        n &= 7;
    }
    __asm__ volatile (
        "rep movsb"
        : "=D" (d), "=S" (s), "=c" (n)
        : "0" (d), "1" (s), "2" (n)
        : "memory"
    );
    #else
    /* Optimized word copy for aligned data */
    if (n >= sizeof(size_t) && ((uintptr_t)d & (sizeof(size_t)-1)) == 0 && 
        ((uintptr_t)s & (sizeof(size_t)-1)) == 0) {
        size_t* wd = (size_t*)d;
        const size_t* ws = (const size_t*)s;
        size_t words = n / sizeof(size_t);
        
        while (words--) {
            *wd++ = *ws++;
        }
        
        d = (char*)wd;
        s = (const char*)ws;
        n &= (sizeof(size_t) - 1);
    }
    
    while (n--) {
        *d++ = *s++;
    }
    #endif
    
    return dest;
}

static inline void* memmove(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    
    if (d == s || n == 0) {
        return dest;
    }
    
    /* Check for overlap: ranges [s, s+n) and [d, d+n) overlap if s < d+n && d < s+n */
    if (s < d + n && d < s + n) {
        /* Overlapping ranges - need to choose safe direction */
        if (d < s) {
            /* dest starts before src - copy forward is safe */
            #ifdef __x86_64__
            if (n >= 8 && ((uintptr_t)d & 7) == 0 && ((uintptr_t)s & 7) == 0) {
                size_t words = n / 8;
                __asm__ volatile (
                    "rep movsq"
                    : "=D" (d), "=S" (s), "=c" (words)
                    : "0" (d), "1" (s), "2" (words)
                    : "memory"
                );
                n &= 7;
            }
            __asm__ volatile (
                "rep movsb"
                : "=D" (d), "=S" (s), "=c" (n)
                : "0" (d), "1" (s), "2" (n)
                : "memory"
            );
            #else
            while (n--) {
                *d++ = *s++;
            }
            #endif
        } else {
            /* dest starts after src - copy backward is safe */
            d += n - 1;
            s += n - 1;
            
            #ifdef __x86_64__
            /* Use std (decrement) instruction for backward copy */
            __asm__ volatile (
                "std\n\t"
                "rep movsb\n\t"
                "cld"
                : "=D" (d), "=S" (s), "=c" (n)
                : "0" (d), "1" (s), "2" (n)
                : "memory"
            );
            #else
            while (n--) {
                *d-- = *s--;
            }
            #endif
        }
    } else {
        /* Non-overlapping - copy forward for efficiency */
        #ifdef __x86_64__
        if (n >= 8 && ((uintptr_t)d & 7) == 0 && ((uintptr_t)s & 7) == 0) {
            size_t words = n / 8;
            __asm__ volatile (
                "rep movsq"
                : "=D" (d), "=S" (s), "=c" (words)
                : "0" (d), "1" (s), "2" (words)
                : "memory"
            );
            n &= 7;
        }
        __asm__ volatile (
            "rep movsb"
            : "=D" (d), "=S" (s), "=c" (n)
            : "0" (d), "1" (s), "2" (n)
            : "memory"
        );
        #else
        while (n--) {
            *d++ = *s++;
        }
        #endif
    }
    
    return dest;
}

static inline void* memset(void* s, int c, size_t n) {
    char* p = (char*)s;
    
    #ifdef __x86_64__
    __asm__ volatile (
        "rep stosb"
        : "=D" (p), "=c" (n)
        : "0" (p), "1" (n), "a" ((unsigned char)c)
        : "memory"
    );
    #else
    while (n--) {
        *p++ = (unsigned char)c;
    }
    #endif
    
    return s;
}

static inline int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    
    return 0;
}

/* String functions */
static inline size_t strlen(const char* s) {
    size_t len = 0;
    
    #ifdef __x86_64__
    __asm__ volatile (
        "repne scasb"
        : "=c" (len)
        : "D" (s), "a" (0), "0" (~0UL)
        : "cc"
    );
    len = ~len - 1;
    #else
    while (*s++) {
        len++;
    }
    #endif
    
    return len;
}

static inline char* strcpy(char* dest, const char* src) {
    char* d = dest;
    
    while ((*d++ = *src++)) {
        ;
    }
    
    return dest;
}

static inline char* strncpy(char* dest, const char* src, size_t n) {
    char* d = dest;
    
    while (n && (*d++ = *src++)) {
        n--;
    }
    
    while (n--) {
        *d++ = '\0';
    }
    
    return dest;
}

static inline char* strcat(char* dest, const char* src) {
    char* d = dest;
    
    while (*d) {
        d++;
    }
    
    while ((*d++ = *src++)) {
        ;
    }
    
    return dest;
}

static inline char* strncat(char* dest, const char* src, size_t n) {
    char* d = dest;
    
    while (*d) {
        d++;
    }
    
    while (n && (*d++ = *src++)) {
        n--;
    }
    
    if (n == 0) {
        *d = '\0';
    }
    
    return dest;
}

static inline int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    
    return (unsigned char)*s1 - (unsigned char)*s2;
}

static inline int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    
    return n ? (unsigned char)*s1 - (unsigned char)*s2 : 0;
}

static inline char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) {
            return (char*)s;
        }
        s++;
    }
    
    return (char)c == '\0' ? (char*)s : NULL;
}

static inline char* strrchr(const char* s, int c) {
    const char* last = NULL;
    
    do {
        if (*s == (char)c) {
            last = s;
        }
    } while (*s++);
    
    return (char*)last;
}

/* Character classification */
static inline int isalpha(int c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static inline int isdigit(int c) {
    return (c >= '0' && c <= '9');
}

static inline int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

static inline int isspace(int c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
}

static inline int isupper(int c) {
    return (c >= 'A' && c <= 'Z');
}

static inline int islower(int c) {
    return (c >= 'a' && c <= 'z');
}

static inline int tolower(int c) {
    return isupper(c) ? c + ('a' - 'A') : c;
}

static inline int toupper(int c) {
    return islower(c) ? c - ('a' - 'A') : c;
}

/* Bit manipulation */
static inline int ffs(int i) {
    if (i == 0) {
        return 0;
    }
    
    #ifdef __GNUC__
    return __builtin_ffs(i);
    #else
    int pos = 1;
    
    if ((i & 0xFFFF) == 0) {
        pos += 16;
        i >>= 16;
    }
    if ((i & 0xFF) == 0) {
        pos += 8;
        i >>= 8;
    }
    if ((i & 0xF) == 0) {
        pos += 4;
        i >>= 4;
    }
    if ((i & 0x3) == 0) {
        pos += 2;
        i >>= 2;
    }
    if ((i & 0x1) == 0) {
        pos += 1;
    }
    
    return pos;
    #endif
}

static inline int ffsl(long i) {
    if (i == 0) {
        return 0;
    }
    
    #ifdef __GNUC__
    return __builtin_ffsl(i);
    #else
    return ffs((int)i) ?: (32 + ffs((int)(i >> 32)));
    #endif
}

static inline int ffsll(long long i) {
    if (i == 0) {
        return 0;
    }
    
    #ifdef __GNUC__
    return __builtin_ffsll(i);
    #else
    return ffs((int)i) ?: (32 + ffs((int)(i >> 32)));
    #endif
}

/* Math functions */
static inline int abs(int x) {
    return x < 0 ? -x : x;
}

static inline long labs(long x) {
    return x < 0 ? -x : x;
}

static inline long long llabs(long long x) {
    return x < 0 ? -x : x;
}

static inline double fabs(double x) {
    #ifdef __x86_64__
    __asm__ ("andpd %1, %0" : "+x" (x) : "x" (0x7FFFFFFFFFFFFFFF));
    return x;
    #else
    union { double d; uint64_t i; } u = { x };
    u.i &= 0x7FFFFFFFFFFFFFFFULL;
    return u.d;
    #endif
}

static inline float fabsf(float x) {
    #ifdef __x86_64__
    __asm__ ("andps %1, %0" : "+x" (x) : "x" (0x7FFFFFFF));
    return x;
    #else
    union { float f; uint32_t i; } u = { x };
    u.i &= 0x7FFFFFFFULL;
    return u.f;
    #endif
}

static inline long double fabsl(long double x) {
    return x < 0.0L ? -x : x;
}

static inline double copysign(double x, double y) {
    union { double d; uint64_t i; } ux = { x };
    union { double d; uint64_t i; } uy = { y };
    ux.i = (ux.i & 0x7FFFFFFFFFFFFFFFULL) | (uy.i & 0x8000000000000000ULL);
    return ux.d;
}

static inline float copysignf(float x, float y) {
    union { float f; uint32_t i; } ux = { x };
    union { float f; uint32_t i; } uy = { y };
    ux.i = (ux.i & 0x7FFFFFFFULL) | (uy.i & 0x80000000ULL);
    return ux.f;
}

static inline int signbit(double x) {
    union { double d; uint64_t i; } u = { x };
    return (u.i >> 63) & 1;
}

static inline int isfinite(double x) {
    union { double d; uint64_t i; } u = { x };
    return ((u.i >> 52) & 0x7FF) != 0x7FF;
}

static inline int isinf(double x) {
    union { double d; uint64_t i; } u = { x };
    return ((u.i >> 52) & 0x7FF) == 0x7FF && (u.i & 0xFFFFFFFFFFFFFULL) == 0;
}

static inline int isnan(double x) {
    union { double d; uint64_t i; } u = { x };
    return ((u.i >> 52) & 0x7FF) == 0x7FF && (u.i & 0xFFFFFFFFFFFFFULL) != 0;
}

static inline double fmin(double x, double y) {
    if (isnan(x)) return y;
    if (isnan(y)) return x;
    return x < y ? x : y;
}

static inline double fmax(double x, double y) {
    if (isnan(x)) return y;
    if (isnan(y)) return x;
    return x > y ? x : y;
}

static inline float fminf(float x, float y) {
    if (x != x) return y;  /* x is NaN */
    if (y != y) return x;  /* y is NaN */
    return x < y ? x : y;
}

static inline float fmaxf(float x, float y) {
    if (x != x) return y;  /* x is NaN */
    if (y != y) return x;  /* y is NaN */
    return x > y ? x : y;
}

static inline double trunc(double x) {
    #ifdef __x86_64__
    double result;
    __asm__ volatile (
        "fldl %1\n\t"
        "frndint\n\t"
        "fstpl %0"
        : "=m" (result)
        : "m" (x)
        : "st"
    );
    return result;
    #else
    if (x >= 0.0) {
        return (double)(long long)x;
    } else {
        return (double)(long long)x;
    }
    #endif
}

static inline float truncf(float x) {
    return (float)trunc(x);
}

static inline double floor(double x) {
    #ifdef __x86_64__
    double result;
    __asm__ volatile (
        "fldl %1\n\t"
        "fstcw -2(%%rsp)\n\t"
        "movw -2(%%rsp), %%ax\n\t"
        "orw $0x400, %%ax\n\t"
        "movw %%ax, -4(%%rsp)\n\t"
        "fldcw -4(%%rsp)\n\t"
        "frndint\n\t"
        "fldcw -2(%%rsp)\n\t"
        "fstpl %0"
        : "=m" (result)
        : "m" (x)
        : "ax", "st"
    );
    return result;
    #else
    double t = trunc(x);
    return t > x ? t - 1.0 : t;
    #endif
}

static inline float floorf(float x) {
    return (float)floor(x);
}

static inline double ceil(double x) {
    #ifdef __x86_64__
    double result;
    __asm__ volatile (
        "fldl %1\n\t"
        "fstcw -2(%%rsp)\n\t"
        "movw -2(%%rsp), %%ax\n\t"
        "orw $0x800, %%ax\n\t"
        "movw %%ax, -4(%%rsp)\n\t"
        "fldcw -4(%%rsp)\n\t"
        "frndint\n\t"
        "fldcw -2(%%rsp)\n\t"
        "fstpl %0"
        : "=m" (result)
        : "m" (x)
        : "ax", "st"
    );
    return result;
    #else
    double t = trunc(x);
    return t < x ? t + 1.0 : t;
    #endif
}

static inline float ceilf(float x) {
    return (float)ceil(x);
}

static inline double round(double x) {
    return floor(x + 0.5);
}

static inline float roundf(float x) {
    return floorf(x + 0.5f);
}

static inline long lround(double x) {
    return (long)round(x);
}

static inline long lroundf(float x) {
    return (long)roundf(x);
}

static inline double sqrt(double x) {
    #ifdef __x86_64__
    double result;
    __asm__ volatile (
        "fldl %1\n\t"
        "fsqrt\n\t"
        "fstpl %0"
        : "=m" (result)
        : "m" (x)
        : "st"
    );
    return result;
    #else
    if (x < 0.0) return 0.0/0.0;  /* NaN */
    if (x == 0.0) return x;
    
    /* Newton-Raphson approximation */
    double guess = x;
    double prev;
    do {
        prev = guess;
        guess = (guess + x / guess) * 0.5;
    } while (fabs(guess - prev) > 1e-15);
    
    return guess;
    #endif
}

static inline float sqrtf(float x) {
    #ifdef __x86_64__
    float result;
    __asm__ volatile (
        "flds %1\n\t"
        "fsqrt\n\t"
        "fstps %0"
        : "=m" (result)
        : "m" (x)
        : "st"
    );
    return result;
    #else
    return (float)sqrt(x);
    #endif
}

static inline double fmod(double x, double y) {
    #ifdef __x86_64__
    double result;
    __asm__ volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "1: fprem\n\t"
        "fstsw %%ax\n\t"
        "testb $4, %%ah\n\t"
        "jnz 1b\n\t"
        "fstpl %0\n\t"
        "fstp %%st(0)"
        : "=m" (result)
        : "m" (x), "m" (y)
        : "ax", "st"
    );
    return result;
    #else
    if (y == 0.0) return 0.0/0.0;  /* NaN */
    
    double quotient = x / y;
    double integer_part = trunc(quotient);
    return x - integer_part * y;
    #endif
}

static inline float fmodf(float x, float y) {
    return (float)fmod(x, y);
}

#ifdef __cplusplus
}
#endif

#endif /* LIBC_REDACTED_H */
