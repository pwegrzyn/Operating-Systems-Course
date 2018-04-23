/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#ifndef MBARR_H
#define MBARR_H

#include <stdlib.h>


#define NO_BLOCKS 1000
#define BLOCK_SIZE 100

typedef struct mem_block_tag {
    size_t len;
    char *arr;
} mem_block;

typedef struct mbarr_tag {
    size_t len;
    mem_block **blocks;
} mbarr;

mem_block *create_block(size_t);

void clear_block(mem_block*);

mbarr *create_empty_mbarr(void);

mbarr *create_mbarr(size_t, size_t);

void delete_mbarr(mbarr*);

mem_block *get_block(mbarr*, size_t);

int remove_block(mbarr*, size_t);

int remove_block_shrink(mbarr*, size_t);

mem_block *cut_block(mbarr*, size_t);

int add_block(mbarr*, size_t, mem_block*);

int add_block_extend(mbarr*, size_t, mem_block*);

int calculate_sum(mem_block*);

int find_closest(mbarr*, size_t);

int calculate_sum_static(char*);

int find_closest_static(char [NO_BLOCKS][BLOCK_SIZE], size_t);

void fill_with_chars(char*);

void fill_with_empty(char*);

#endif