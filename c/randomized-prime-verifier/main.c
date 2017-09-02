#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "long.h"
#include <unistd.h>

typedef LongObject* BigInt;

BigInt GCD(BigInt a, BigInt b);
BigInt exp_mod(BigInt a, BigInt b, BigInt c);
BigInt exp_eq(BigInt a, BigInt b, BigInt c);
BigInt random_long(BigInt max);
bool tiene_raiz_entera(BigInt n, BigInt k, BigInt i, BigInt j);
bool is_power(BigInt n);
bool primality_test(BigInt n, int k);

BigInt ZERO;
BigInt ONE;
BigInt TWO;
BigInt THREE;
BigInt _trash;

void init(){
  ZERO = Long_FromLong(0L);
  ONE = Long_FromLong(1L);
  TWO = Long_FromLong(2L);
  THREE = Long_FromLong(3L);

  srand((unsigned) time(NULL));
}

BigInt random_long(BigInt max){
  size_t msize = max->size;
  BigInt res = _Long_New(msize);
  for (size_t i = 0; i < msize - 1; i++) {
    res->ob_digit[i] = rand() & MASK;
  }
  res->ob_digit[msize-1] = (rand() % max->ob_digit[msize-1]) & MASK;

  long_normalize(res);

  if (long_compare(res, ZERO) == 0){
    return long_add(res, ONE);
  }
  return res;
}


BigInt GCD(BigInt a, BigInt b){
  if (long_compare(a, ZERO) == 0) {
    return b;
  }
  if (long_compare(b, ZERO) == 0) {
    return a;
  }

  BigInt c;
  if (long_compare(a, b) >= 0){
    long_divrem(a, b, &c);
    return GCD(b, c);
  }
  else{
    long_divrem(b, a, &c);
    return GCD(a, c);
  }
}

BigInt exp_mod(BigInt a, BigInt b, BigInt c){
  BigInt m, e, b2, res;

  if (long_compare(b, ONE) == 0){
    long_divrem(a, c, &m);
    return m;
  }
  b2 = long_divrem(b, TWO, &m);
  e = exp_mod(a, b2, c);
  res = long_mul(e, e);

  /* b is odd */
  if (long_compare(m, ONE) == 0){
    e = long_mul(res, a);
    long_divrem(e, c, &m);
    return m;
  }
  /* b is even */
  else{
    long_divrem(res, c, &m);
    return m;
  }
}

BigInt exp_eq(BigInt a, BigInt b, BigInt c){
  BigInt m, e, b2, res;

  if (long_compare(b, ONE) == 0){
    return a;
  }
  b2 = long_divrem(b, TWO, &m);
  e = exp_eq(a, b2, c);

  if (long_compare(c, e) < 0){
    return e;
  }
  res = long_mul(e, e);


  /* b is odd */
  if (long_compare(m, ONE) == 0){
    e = long_mul(res, a);
    return e;
  }
  /* b is even */
  else{
    return res;
  }
}


bool tiene_raiz_entera(BigInt n, BigInt k, BigInt i, BigInt j){
  if( long_compare(i, j) == 0){
    BigInt _exp = exp_eq(i, k, n);
    return (long_compare(_exp, n) == 0);
  }
  if (long_compare(i, j) < 0){
    BigInt ij = long_add(i, j);
    BigInt p = long_divrem(ij, TWO, &_trash);
    BigInt _exp = exp_eq(p, k, n);

    if (long_compare(_exp, n) == 0){
      return true;
    }
    else if (long_compare(_exp, n) < 0){
      return tiene_raiz_entera(n, k, long_add(p, ONE), j);
    }
    else{
      return tiene_raiz_entera(n, k, i, long_sub(p, ONE));
    }
  }
  return false;
}

/* returns true iff n is a power of any other number */
bool is_power(BigInt n){
  if (long_compare(n, THREE) <= 0) return false;
  BigInt k = Long_FromLong(2L);
  BigInt TOP = Long_FromLong(n->size * 9); /* upper bound for log2(n) */

  while (long_compare(k, TOP) < 0){
    if (tiene_raiz_entera(n, k, ONE, n)) return true;
    k = long_add(k, ONE);
  }
  return false;
}

/* returns true iff n is even */
bool is_even(BigInt n){
  return (n->size == 0 || (n->ob_digit[0] & 1) == 0 );
}

bool primality_test(BigInt n, int k){
  if (long_compare(n, TWO) == 0) return true;
  if (is_even(n)) return false;
  if (is_power(n)) return false;

  BigInt* bs = malloc(sizeof(BigInt) * k);
  BigInt n_2 = long_divrem(n, TWO, &_trash);
  int i = k;

  while (i--){
    BigInt ai = random_long(n);
    if (long_compare(GCD(ai, n), ONE) > 0){
      free(bs);
      return false;
    }
    bs[i] = exp_mod(ai, n_2, n);

  }

  int neg = 0;
  BigInt bi;
  while (k--){
    bi = bs[k];

    if (long_compare(long_add(bi, ONE), n) == 0)
      neg++;
    else if (long_compare(long_sub(bi, ONE), ZERO) != 0){
      return false;
    }
  }
  return (neg != 0);
}

void option_1(){
  char buffer[1000] = {0,};
  int k;
  ssize_t read_len;

  printf("\nPlease type in a number to check its primality:\n");

  read_len = read(0, buffer, 1000);
  buffer[read_len-1] = 0;
  BigInt n = Long_FromString(buffer, NULL, 10);

  printf("\nPlease type in a 'k' to bound the error chance to 2^(-k):\n");
  scanf("%d", &k);

  if (primality_test(n, k)){
    printf("\nPRIME NUMBER\n");
  }
  else{
    printf("\nCOMPOUND NUMBER\n");
  }
}

void option_2(){
  char inpt_buffer[1000] = {0,};
  char otpt_buffer[1000] = {0,};
  int k = -1;
  ssize_t read_len;
  FILE *inpt, *otpt;

  printf("\nPlease type the relative route to the file:\n");

  read_len = read(0, inpt_buffer, 1000);
  inpt_buffer[read_len-1] = 0;
  inpt = fopen(inpt_buffer, "r");

  printf("\nPlease type the desired output file name:\n");

  read_len = read(0, otpt_buffer, 1000);
  otpt_buffer[read_len-1] = 0;
  otpt = fopen(otpt_buffer, "w");


  int string_size;
  fseek(inpt, 0, SEEK_END);
  string_size = ftell(inpt);
  rewind(inpt);
  char *buffer = malloc(sizeof(char)*(string_size + 1));
  fread(buffer, sizeof(char), string_size, inpt);
  buffer[string_size] = 0;
  fclose(inpt);

  char* str = buffer;
  char* n_start = buffer;

  printf("\nCalculating...\n");

  while (str < buffer + string_size){
    while (*str == ' ') str++;
    n_start = str;

    while (*str != ',') str++;
    *str++ = 0;

    BigInt n = Long_FromString(n_start, NULL, 10);
    while (*str == ' ') str++;
    n_start = str;

    while (*str != ',' && str < buffer + string_size) str++;
    *str = 0;
    k = atoi(n_start);

    if (primality_test(n, k)){
      fprintf(otpt, "PRIME NUMBER");
    }else{
      fprintf(otpt, "COMPOUND NUMBER");
    }
    while (*str <= '0' && str < buffer + string_size) str++;

    if (str < buffer + string_size)
      fprintf(otpt, ", ");
    n_start = str;
  }

  printf("Results saved on '%s'\n", otpt_buffer);

  fclose(otpt);
}


int main(int argc, char const *argv[]) {
  init();

  char choice = 0;
  while (true) {
    printf("Please choose any of the following options:\n");
    printf("\t1. Type 'n' and 'k' directly on the terminal.\n");
    printf("\t2. Choose an input file with multiple inputs.\n");
    printf("\t3. Quit.\n\n");
    printf("Type the number of the choice you desire to execute.");

    do {
      scanf("%c", &choice);
    } while (!(choice == '1' || choice == '2' || choice == '3' ));

    switch (choice) {
      case '1':
        option_1();
        break;
      case '2':
        option_2();
        break;
      case '3':
        printf("Good Bye!\n");
        return 0;
    }
    printf("\n\n");
  }
  return 0;
}
