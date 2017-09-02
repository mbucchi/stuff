#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include "long.h"

// this is used for debugging
#define assert(expr)

typedef unsigned long doubledigit;
typedef signed long sdoubledigit;


LongObject* _Long_New(size_t size){
  LongObject* result;
  result = malloc(offsetof(LongObject, ob_digit) + size * sizeof(digit));
  result->size = size;
  return result;
}

LongObject* _Long_Copy(LongObject* src){
  LongObject *result;
  int i;

  i = src->size;
  result = _Long_New(i);
  if (result != NULL) {
    result->size = src->size;
    while (--i >= 0)
      result->ob_digit[i] = src->ob_digit[i];
  }
  return result;
}




static const unsigned char BitLengthTable[32] = {
  0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

unsigned char _Long_DigitValue[256] = {
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  37, 37, 37, 37, 37, 37,
  37, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
  25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 37, 37, 37, 37, 37,
  37, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
  25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
  37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
};


LongObject * Long_FromLong(unsigned long ival){
  LongObject *v;
  unsigned long t;
  int ndigits = 0;

  t = ival;
  while (t) {
    ++ndigits;
    t >>= SHIFT;
  }
  v = _Long_New(ndigits);
  if (v != NULL) {
    digit *p = v->ob_digit;
    v->size = ndigits;
    t = ival;
    while (t) {
      *p++ = (digit)(t & MASK);
      t >>= SHIFT;
    }
  }
  return v;
}

static int bits_in_digit(digit d){
  int d_bits = 0;
  while (d >= 32) {
    d_bits += 6;
    d >>= 6;
  }
  d_bits += (int)BitLengthTable[d];
  return d_bits;
}

/* Eliminates leading zeros on v */
LongObject* long_normalize(LongObject *v){
  size_t j = v->size;
  size_t i = j;

  while (i > 0 && v->ob_digit[i-1] == 0)
    --i;
  if (i != j)
    v->size = i;
  return v;
}

 /* Shitfs a[0:m] d bits to the left, with 0 <= d < SHIFT. The result is placed on
  * z[0:m] and the d bits that overflow to the left are returned */
 static digit a_lshift(digit *z, digit *a, size_t m, int d)
{
  size_t i;
  digit carry = 0;

  assert(0 <= d && d < PyLong_SHIFT);
  for (i=0; i < m; i++) {
    doubledigit acc = (doubledigit)a[i] << d | carry;
    z[i] = (digit)acc & MASK;
    carry = (digit)(acc >> SHIFT);
  }
  return carry;
}


 /* Shitfs a[0:m] d bits to the right, with 0 <= d < SHIFT. The result is placed on
  * z[0:m] and the d bits that overflow to the right are returned */
static digit a_rshift(digit *z, digit *a, size_t m, int d)
{
  size_t i;
  digit carry = 0;
  digit mask = ((digit)1 << d) - 1U;

  assert(0 <= d && d < SHIFT);
  for (i=m; i-- > 0;) {
    doubledigit acc = (doubledigit)carry << SHIFT | a[i];
    carry = (digit)acc & mask;
    z[i] = (digit)(acc >> d);
  }
  return carry;
}

/* Divides a_in with <size> digits by n, storing the result on a_out and 
 * returning the remainder. Pointers should be poiting the most significative digit
 * in a_in and a_out.  */
digit inplace_divrem(digit *a_out, digit *a_in, int size, digit n){
  doubledigit rem = 0;

  assert(n > 0 && n <= MASK);
  a_in += size;
  a_out += size;
  while (--size >= 0) {
    digit hi;
    rem = (rem << SHIFT) | *--a_in;
    *--a_out = hi = (digit)(rem / n);
    rem -= (doubledigit)hi * n;
  }
  return (digit)rem;
}

/* Divides LongObject <a> by digit <n>, returning the result and placing the reminder 
 * on <prem> */
static LongObject* divrem(LongObject *a, digit n, digit *prem){
    const size_t size = a->size;
    LongObject *z;

    assert(n > 0 && n <= MASK);
    z = _Long_New(size);
    if (z == NULL)
        return NULL;
    *prem = inplace_divrem(z->ob_digit, a->ob_digit, size, n);
    return long_normalize(z);
}


/* Compares two longs */
int long_compare(LongObject *a, LongObject *b){
  int sign;

  if (a->size != b->size) {
    sign = a->size - b->size;
  }
  else {
    int i = a->size;
    while (--i >= 0 && a->ob_digit[i] == b->ob_digit[i]);
    if (i < 0)
      sign = 0;
    else {
      sign = (int)a->ob_digit[i] - (int)b->ob_digit[i];
    }
  }
  return sign < 0 ? -1 : sign > 0 ? 1 : 0;
}


/* Adds two LongObjects, returing the result in a new LongObject */
LongObject* long_add(LongObject *a, LongObject *b){
  size_t size_a = a->size, size_b = b->size;
  LongObject *z;
  size_t i;
  digit carry = 0;

  /* <a> must be the biggest of both LongObjects */
  if (size_a < size_b) {
    { LongObject *temp = a; a = b; b = temp; }
    { size_t size_temp = size_a;
      size_a = size_b;
      size_b = size_temp; }
  }
  z = _Long_New(size_a+1);

  for (i = 0; i < size_b; ++i) {
    carry += a->ob_digit[i] + b->ob_digit[i];
    z->ob_digit[i] = carry & MASK;
    carry >>= SHIFT;
  }
  for (; i < size_a; ++i) {
    carry += a->ob_digit[i];
    z->ob_digit[i] = carry & MASK;
    carry >>= SHIFT;
  }
  z->ob_digit[i] = carry;
  return long_normalize(z);
  return z;
}


LongObject* long_sub(LongObject *a, LongObject *b){
  size_t size_a = a->size, size_b = b->size;
  LongObject *z;
  int i;
  int sign = 1;
  digit borrow = 0;

  if (size_a < size_b) {
    sign = -1;
    { LongObject *temp = a; a = b; b = temp; }
    { size_t size_temp = size_a;
      size_a = size_b;
      size_b = size_temp; }
  }
  else if (size_a == size_b) {
    i = size_a;
    while (--i >= 0 && a->ob_digit[i] == b->ob_digit[i]);

    if (i < 0)
      return Long_FromLong(0);
    if (a->ob_digit[i] < b->ob_digit[i]) {
      sign = -1;
      { LongObject *temp = a; a = b; b = temp; }
    }
    size_a = size_b = i+1;
  }
  z = _Long_New(size_a);
  if (z == NULL)
    return NULL;
  for (i = 0; i < size_b; ++i) {
    borrow = a->ob_digit[i] - b->ob_digit[i] - borrow;
    z->ob_digit[i] = borrow & MASK;
    borrow >>= SHIFT;
    borrow &= 1;
  }
  for (; i < size_a; ++i) {
    borrow = a->ob_digit[i] - borrow;
    z->ob_digit[i] = borrow & MASK;
    borrow >>= SHIFT;
    borrow &= 1;
  }
  assert(borrow == 0);
  if (sign < 0) {
    z->size = -z->size;
  }
  return long_normalize(z);
}

/* Basic multiplication */
LongObject* long_mul(LongObject *a, LongObject *b){
    LongObject *z;
    size_t size_a = a->size;
    size_t size_b = b->size;
    size_t i;

    z = _Long_New(size_a + size_b);
    if (z == NULL)
        return NULL;

    memset(z->ob_digit, 0, z->size * sizeof(digit));

    for (i = 0; i < size_a; ++i) {
      doubledigit carry = 0;
      doubledigit f = a->ob_digit[i];
      digit *pz = z->ob_digit + i;
      digit *pb = b->ob_digit;
      digit *pbend = b->ob_digit + size_b;

      while (pb < pbend) {
        carry += *pz + *pb++ * f;
        *pz++ = (digit)(carry & MASK);
        carry >>= SHIFT;
        assert(carry <= MASK);
      }
      if (carry)
        *pz += (digit)(carry & MASK);
      assert((carry >> SHIFT) == 0);
    }
    return long_normalize(z);
}



LongObject* long_divrem(LongObject *v1, LongObject *w1, LongObject **prem){
  LongObject *v, *w, *a;
  size_t i, k, size_v, size_w;
  signed int d;
  digit wm1, wm2, carry, q, r, vtop, *v0, *vk, *w0, *ak;
  doubledigit vv;
  signed int zhi;
  sdoubledigit z;


  size_v = v1->size;
  size_w = w1->size;
  assert(size_v >= size_w && size_w >= 2);

  if (size_w == 0) {
    fprintf(stderr, "ERROR: division or modulus by 0\n");
    return NULL;
  }
  if (size_v < size_w || (size_v == size_w &&
      v1->ob_digit[size_v-1] < w1->ob_digit[size_w-1])) {
    *prem = _Long_Copy(v1);
    return Long_FromLong(0L);
  }

  if (size_w == 1) {
    digit rem = 0;
    a = divrem(v1, w1->ob_digit[0], &rem);
    *prem = Long_FromLong((long)rem);
    return a;
  }

  v = _Long_New(size_v+1);
  w = _Long_New(size_w);


  d = SHIFT - bits_in_digit(w1->ob_digit[size_w-1]);
  carry = a_lshift(w->ob_digit, w1->ob_digit, size_w, d);
  assert(carry == 0);

  carry = a_lshift(v->ob_digit, v1->ob_digit, size_v, d);
  if (carry != 0 || v->ob_digit[size_v-1] >= w->ob_digit[size_w-1]) {
    v->ob_digit[size_v] = carry;
    size_v++;
  }


  k = size_v - size_w;
  assert(k >= 0);
  a = _Long_New(k);

  v0 = v->ob_digit;
  w0 = w->ob_digit;
  wm1 = w0[size_w-1];
  wm2 = w0[size_w-2];
  for (vk = v0+k, ak = a->ob_digit + k; vk-- > v0;) {

    vtop = vk[size_w];
    assert(vtop <= wm1);
    vv = ((doubledigit)vtop << SHIFT) | vk[size_w-1];
    q = (digit)(vv / wm1);
    r = (digit)(vv - (doubledigit)wm1 * q);

    while ((sdoubledigit)wm2 * q > (((sdoubledigit)r << SHIFT) | vk[size_w-2])) {
      --q;
      r += wm1;
      if (r >= BASE)
          break;
    }
    assert(q <= BASE);

    zhi = 0;
    for (i = 0; i < size_w; ++i) {
      z = (signed int)vk[i] + zhi - (sdoubledigit)q * (sdoubledigit)w0[i];
      vk[i] = (digit)z & MASK;
      zhi = (signed int)(((sdoubledigit)z) >> SHIFT);
    }

    assert((int)vtop + zhi == -1 || (int)vtop + zhi == 0);
    if ((signed int)vtop + zhi < 0) {
        carry = 0;
        for (i = 0; i < size_w; ++i) {
            carry += vk[i] + w0[i];
            vk[i] = carry & MASK;

            carry >>= SHIFT;
        }
        --q;
    }

    assert(q < BASE);
    *--ak = q;
  }

  carry = a_rshift(w0, v0, size_w, d);
  assert(carry==0);

  *prem = long_normalize(w);
  return long_normalize(a);
}

/* Creates a LongObject from a string with given base. If the whole string 
 * cannot be converted, a pointer will be returned to the first char that 
 * was not analyzed in <pend> */
LongObject* Long_FromString(const char *str, char **pend, int base){
  LongObject *z = NULL;
  doubledigit c;

  size_t size_z;
  int digits = 0;
  int i;
  int convwidth;
  doubledigit convmultmax, convmult;
  digit *pz, *pzstop;
  const char *scan, *lastdigit;
  char prev = 0;

  static double log_base_BASE[37] = {0.0e0,};
  static int convwidth_base[37] = {0,};
  static doubledigit convmultmax_base[37] = {0,};

  if (log_base_BASE[base] == 0.0) {
    doubledigit convmax = base;
    int i = 1;

    log_base_BASE[base] = (log((double)base) /
                           log((double)BASE));
    for (;;) {
      doubledigit next = convmax * base;
      if (next > BASE) {
        break;
      }
      convmax = next;
      ++i;
    }
    convmultmax_base[base] = convmax;
    assert(i > 0);
    convwidth_base[base] = i;
  }

  scan = str;
  lastdigit = str;

  while (*scan != 0) {
      ++digits;
      lastdigit = scan;
      prev = *scan;
      ++scan;
  }

  size_z = (size_t)(digits * log_base_BASE[base]) + 1;

  assert(size_z > 0);
  z = _Long_New(size_z);
  if (z == NULL) {
      return NULL;
  }
  z->size = 0;

  convwidth = convwidth_base[base];
  convmultmax = convmultmax_base[base];

  while (str < scan) {
    c = (digit)_Long_DigitValue[(short)*str++];
    for (i = 1; i < convwidth && str != scan; ++str) {
      i++;
      c = (doubledigit)(c * base + (int)_Long_DigitValue[(short)*str]);
      assert(c < BASE);
    }

    convmult = convmultmax;
    if (i != convwidth) {
      convmult = base;
      for ( ; i > 1; --i) {
        convmult *= base;
      }
    }

    pz = z->ob_digit;
    pzstop = pz + z->size;
    for (; pz < pzstop; ++pz) {
      c += (doubledigit)*pz * convmult;
      *pz = (digit)(c & MASK);
      c >>= SHIFT;
    }
    if (c) {
      assert(c < BASE);
      if (z->size < size_z) {
          *pz = (digit)c;
          ++z->size;
      }
      else {
        LongObject *tmp;
        assert(z->size == size_z);
        tmp = _Long_New(size_z + 1);
        memcpy(tmp->ob_digit, z->ob_digit, sizeof(digit) * size_z);
        z = tmp;
        z->ob_digit[size_z] = (digit)c;
        ++size_z;
      }
    }
  }
  long_normalize(z);
  if (pend != NULL) {
      *pend = (char *)str;
  }
  return z;
}

/* Prints a LongObject to stdout in base 10 */
void long_print(LongObject* l){
  LongObject* tmp = _Long_Copy(l);
  LongObject* ZERO = Long_FromLong(0L);

  int d = (33 * DECIMAL_SHIFT) / (10 * SHIFT - 33 * DECIMAL_SHIFT);
  int size = (l->size + l->size/d) * DECIMAL_SHIFT + 1;

  char* str = calloc(size, 1);
  size_t str_len = 0;
  digit rem;

  while (long_compare(tmp, ZERO) > 0){
    rem = inplace_divrem(tmp->ob_digit, tmp->ob_digit, tmp->size, 10);
    long_normalize(tmp);
    str[str_len++] = '0' + rem;
  }
  if (str_len == 0)
    printf("0");
  else
    while (str_len--)
      printf("%c", str[str_len]);
  printf("\n");
  free(str);
  free(tmp);
  free(ZERO);
}
