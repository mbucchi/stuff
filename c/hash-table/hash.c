#include "hash.h"
#include <stdlib.h>

#define TABLE_MIN_SIZE 32
#define PERTURB_SHIFT 5

/** Represents an entry on the hash table */
typedef struct {
    mpz_t key;
    uint8_t value;
} Entry;

/** Represents the hash table */
struct hash_table {
    /** Number of entries already assigned on the table */
    uint32_t used;

    /** Table size - 1 (always of the form 2^i - 1)                */
    /** Useful for clipping a hash to a useful value in this table */
    uint32_t mask;

    /** Entry array. Has a length of <mask> + 1 */
    Entry *table;

    /** This is the key associated to empty entries */
    mpz_t dummy;
};

/** Allocates memory and initializes a new hash table */
HashTable* new_hash_table(){
    HashTable* ht = malloc(sizeof(HashTable));
    ht->used = 0;
    mpz_init(ht->dummy);
    ht->table = calloc(sizeof(Entry), TABLE_MIN_SIZE);
    ht->mask = TABLE_MIN_SIZE - 1;
    return ht;
}

/** Destroys and frees the memory associated to a hash table */
void destroy_hash_table(HashTable* ht){
    /** Keys are freed */
    for (int i = 0; i < ht->mask + 1; ++i) {
        mpz_clear(ht->table[i].key);
    }
    /** dummy key is freed */
    mpz_clear(ht->dummy);
    /** entry array is freed */
    free(ht->table);
    /** table struct is freed */
    free(ht);
}

/** Doubles the entry array size, rehashing all existing entries */
void resize_ht(HashTable* ht){
    /** size *= 2 */
    uint32_t size = ht->mask + 1;
    /** setting new mask */
    ht->mask = size * 2 - 1;
    /** Reseting so that the insert function can be used normally */
    ht->used = 0;
    /** temporal pointer to old table */
    Entry* table = ht->table;
    /** allocating new empty table */
    ht->table = calloc(sizeof(Entry), size * 2);

    /** Rehashing all entries of the previous table */
    for (int i = 0; i < size; ++i) {
        if (mpz_cmp(table[i].key, ht->dummy))
            hash_table_insert(ht, table[i].key, table[i].value);
        mpz_clear(table[i].key);
    }
    /** Old table is freed */
    free(table);
}

/** Returns a reference to the entry associated to the given key.
 ** ALWAYS returns a valid reference */
Entry* get_entry(HashTable* ht, mpz_t key){
    uint32_t h, perturb, j, i;

    /** Open adressing is used to find the desired bucket on the table */
    h = perturb = (uint32_t) mpz_get_ui(key);
    j = i = h & ht->mask;
    /** While the wanted bucket or an empty bucket is not found, the iteration will continue */
    while (mpz_cmp(ht->table[i].key, ht->dummy) != 0 && mpz_cmp(ht->table[i].key, key) != 0){
        j = (5 * j) + 1 + perturb;
        perturb >>= PERTURB_SHIFT;
        i = j & ht->mask;
    }

    /** Returning the entry reference */
    return &ht->table[i];
}

/** Inserts the key value pair in the table. ALWAYS succeeds */
void hash_table_insert(HashTable* ht, mpz_t key, uint8_t val){

    /** resize if necessary */
    if (ht->used > ht->mask * 2 / 3)
        resize_ht(ht);

    /** Looking for the associated entry */
    Entry* e = get_entry(ht, key);
    /** If the entry is empty, a new spot will be used and the used counter is increased */
    if (mpz_cmp(e->key, ht->dummy) == 0)
        ht->used++;

    /** Saving the key value pair in the given entry */
    mpz_set(e->key, key);
    e->value = val;
}

/** Searchs for the value associated to the given key. If the key is found, the value is 
 * assigned to the given <val> pointer and then returns true. If no key is found, this 
 * function returns false and the given pointer is left as is. */
bool hash_table_get(HashTable* ht, mpz_t key, uint8_t* val){
    /** The entry is searched. */
    Entry* e = get_entry(ht, key);

    /** Key has not been previously assigned in this hash table */
    if (mpz_cmp(e->key, key) != 0)
        return false;

    /** Found value is set on the given pointer */
    *val = e->value;

    return true;
}
