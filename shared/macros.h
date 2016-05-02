#define likely(x)       __builtin_expect((x), 1)
#define unlikely(x)     __builtin_expect((x), 0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

