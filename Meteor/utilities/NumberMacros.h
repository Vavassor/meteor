#ifndef NUMBER_MACROS_H
#define NUMBER_MACROS_H

#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef IS_ODD
#define IS_ODD(n)	((n) & 1)
#endif

#ifndef IS_EVEN
#define IS_EVEN(n)  !IS_ODD(n)
#endif

#ifndef IS_BETWEEN
#define IS_BETWEEN(n, min, max)	((n) >= (min) && (n) <= (max))
#endif

#endif
