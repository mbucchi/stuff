#ifndef LONG_H_T3
#define LONG_H_T3

#include <stdlib.h>

typedef unsigned int digit;
#define SHIFT	30
#define DECIMAL_SHIFT	9
#define DECIMAL_BASE

#define BASE	((digit)1 << SHIFT)
#define MASK	((digit)(BASE - 1))

typedef struct _longobject {
  int size;
	digit ob_digit[1];
} LongObject;

LongObject* long_divrem(LongObject *v1, LongObject *w1, LongObject **prem);
void long_print(LongObject* l);
LongObject* _Long_New(size_t size);
LongObject* _Long_Copy(LongObject* src);
LongObject* Long_FromLong(unsigned long ival);
LongObject* Long_FromString(const char *str, char **pend, int base);
LongObject* long_mul(LongObject *a, LongObject *b);
LongObject* long_sub(LongObject *a, LongObject *b);
LongObject* long_add(LongObject *a, LongObject *b);
LongObject* long_normalize(LongObject *v);
int long_compare(LongObject *a, LongObject *b);

#endif
