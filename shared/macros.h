#ifdef C99_COMPLIANT
/* Simpley ignore the u8 prefix.
 * u8 only is valid in gnu99, but it adds clarity that we are dealing with utf8.
 * The U'' should assure ucs4, but it is gnu99.
 * If we are not using gnu99, we can only hope L'' is ucs4...
 * -fwide-exec-charset=UCS-4 does this
 */
#define u8
#define U L
#endif

#define likely(x)       __builtin_expect((x), 1)
#define unlikely(x)     __builtin_expect((x), 0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

