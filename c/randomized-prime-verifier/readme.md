# Randomized Primality Verifier 
This program test the primality of very big numbers using a randomized algorithm which is very efficient and with a bounded error probability. This error can be minimized as much as one would like.

------------

In order to compile this program, you just need to link `main.c` and `long.c` files. With gcc this would look like

> ```gcc -o prime_test main.c long.c```

Once you've done that, you just need to execute from terminal in order to run the program
 (`./prime_test`) if you used the previous command to compile.

Two different choices will be presented:
1.  Writing the number to test and the chosen error bound directly on terminal
2.  Writing a relative route to a file with multiple numbers and error bounds

On the first case, you shall write the following numbers:
> **_n_** - The number to be tested \
> **_k_** - Error bound (given by _2^(-k)_)

If you chose the second option, you will be prompted for a file with the following structure

> *n_1*, *k_1*, *n_2*, *k_2*, ... , *n_m*, *k_m*

Where *n_i* and *k_i* are the parameters for the *i'th* test. The results will be printed on a file of your choice.

## Arbitrary precision arithmetic

For this program you may use numbers as big as you like. For this, a LongInt implementation based on Python is made on `long.h` and `long.c`.
