#ifndef HASH_H
#define HASH_H

#include <gmp.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct hash_table HashTable;

void hash_table_insert(HashTable* ht, mpz_t key, uint8_t val);
bool hash_table_get(HashTable* ht, mpz_t key, uint8_t* val);
void destroy_hash_table(HashTable* ht);
HashTable* new_hash_table();


#endif 
