#include "mbarr.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

/* Used in finding the closest block */
#define MAX_INT_VAL 2147483647

/* Length of the array of blocks (numer of blocks) */
#define NO_BLOCKS 1000

/* Size of each block (number of char elements) */
#define BLOCK_SIZE 100

/* Functions used in dynamic memory allocation */

mem_block *create_block(size_t size)
{   
    mem_block *new_block = (mem_block*)calloc(1, sizeof(mem_block));
    if (new_block == NULL)
    {
        fprintf(stderr, "error while allocating memory block: %s\n", strerror(errno));
        return NULL;
    }

    new_block->len = size;
    new_block->arr = (char*)calloc(size, sizeof(char));
    if(new_block->arr == NULL && size != 0)
    {
        fprintf(stderr, "error while allocating internal memory of the block: %s\n", 
                strerror(errno));
        return NULL;
    }

    return new_block;
}

void clear_block(mem_block *old_block)
{
    if(old_block == NULL) return;
    if(old_block->len != 0 && old_block->arr != NULL)
        free(old_block->arr);
    free(old_block);
    return;
}

mbarr *create_empty_mbarr()
{
    mbarr *new_array = (mbarr*)calloc(1, sizeof(mbarr));
    if (new_array == NULL)
    {
        fprintf(stderr, "error while allocating memory block array: %s\n", 
                strerror(errno));
        return NULL;
    }

    new_array->len = 0;
    return new_array;
}

mbarr *create_mbarr(size_t mbarr_size, size_t block_size)
{
    mbarr *new_mbarr = create_empty_mbarr();
    if(new_mbarr == NULL)
    {
        fprintf(stderr, "error while creating an empty array: %s\n", strerror(errno));
        return NULL;
    }

    new_mbarr->blocks = (mem_block**)calloc(mbarr_size, sizeof(mem_block*));
    if(new_mbarr->blocks == NULL && mbarr_size != 0)
    {
        fprintf(stderr, "error while allocating internal memory of the array: %s\n", 
                strerror(errno));
        free(new_mbarr);
        return NULL;
    }
    
    new_mbarr->len = mbarr_size;
    for(int i = 0; i < mbarr_size; i++)
    {
        new_mbarr->blocks[i] = create_block(block_size);
        if(new_mbarr->blocks[i] == NULL)
        {
            fprintf(stderr, "error while allocating internal memory of an element " 
                    "of the array at index %d: %s\n", i, strerror(errno));
            for(int j = 0; j < i; j++)
            {
                clear_block(new_mbarr->blocks[i]);
            }
            free(new_mbarr->blocks);
            free(new_mbarr);
            return NULL;
        }
    }

    return new_mbarr;
}

void delete_mbarr(mbarr *old_array)
{
    if(old_array == NULL) return;

    for(int i = 0; i < old_array->len; i++)
    {
        clear_block(old_array->blocks[i]);
    }

    free(old_array->blocks);
    free(old_array);
    return;
}

mem_block *get_block(mbarr *array, size_t index)
{
    if(array != NULL && index < array->len)
    {
        return array->blocks[index];
    }
    return NULL;
}

int remove_block(mbarr *array, size_t index)
{
    if(array == NULL || array->len < index) return -1;

    mem_block *found = get_block(array, index);
    if(found != NULL)
    {
        clear_block(found);
        for(int i = index; i < (array->len) - 1; i++)
        {
            array->blocks[i] = array->blocks[i+1];
        }
        return --(array->len);
    }
    return -1;
}

mem_block *cut_block(mbarr *array, size_t index)
{
    if(array == NULL || array->len < index) return NULL;

    mem_block *found = get_block(array, index);
    if(found != NULL)
    {
        for(int i = index; i < (array->len) - 1; i++)
        {
            array->blocks[i] = array->blocks[i+1];
        }
        --(array->len);
        return found;
    }
    return NULL;
}

// risky, it is advised to use normal remove_block instead of this
int remove_block_shrink(mbarr *array, size_t index)
{
    int new_size = remove_block(array, index);
    if(new_size > 0)
    {
        mem_block **old_blocks = array->blocks;
        array->blocks = (mem_block**)realloc(array->blocks, new_size * sizeof(mem_block*));
        if(array->blocks == NULL)
        {
            array->blocks = old_blocks;
            return -1;
        }
        return new_size;
    } 
    return -1;
}

int add_block(mbarr *array, size_t index, mem_block *new_block)
{
    if(array == NULL || array->len < index)
        return -1;

    array->blocks[index] = new_block;
    return array->len;
}

// again, I am not sure whether this is the right implementation,
// if not just use normal add_block instead
int add_block_extend(mbarr *array, size_t index, mem_block *new_block)
{
    if(array == NULL || array->len < index || new_block == NULL)
        return -1;

    mem_block **old_blocks = array->blocks;
    array->blocks = (mem_block**)realloc(array->blocks, (++(array->len)) * sizeof(mem_block*));
    if(array->blocks == NULL)
    {
        array->blocks = old_blocks;
        return -1;
    }
    for(int i = index; i < (array->len) - 1; i++)
    {
        array->blocks[i+1] = array->blocks[i];
    }
    ++(array->len);
    array->blocks[index] = new_block;
    return array->len;
}

int find_closest(mbarr* array, size_t index)
{
    if(array == NULL || index > array->len) return -1;
    int foundSum = calculate_sum(get_block(array, index));
    if(foundSum == -1) return -1;

    int closestSum = MAX_INT_VAL;
    int closestIndex = -1;
    int currentSum = -1;

    for(int i = 0; i < array->len; i++)
    {
        currentSum = calculate_sum(get_block(array, i));
        if(currentSum == -1) continue;
        if(abs(currentSum - foundSum) < closestSum && i != index)
        {
            closestSum = abs(currentSum - foundSum);
            closestIndex = i;
        }
    }
    return closestIndex;
}

int calculate_sum(mem_block *block)
{
    if(block != NULL && block->len > 0 && block->arr != NULL)
    {
        int sum = 0;
        for(int i = 0; i < block->len; i++)
        {
            sum += block->arr[i];
        }
        return sum;
    }
    return -1;
}

/* Static memory managment */

int calculate_sum_static(char* block)
{
    if(block != NULL)
    {
        int sum = 0;
        for(int i = 0; i < BLOCK_SIZE; i++)
        {
            sum += block[i];
        }
        return sum;
    }
    return -1;
}

int find_closest_static(char arr[NO_BLOCKS][BLOCK_SIZE], size_t index)
{
    int foundSum = calculate_sum_static(arr[index]);
    if(foundSum == -1) return -1;

    int closestSum = MAX_INT_VAL;
    int closestIndex = -1;
    int currentSum = -1;

    for(int i = 0; i < NO_BLOCKS; i++)
    {
        currentSum = calculate_sum_static(arr[i]);
        if(currentSum == -1) continue;
        if(abs(currentSum - foundSum) < closestSum && i != index)
        {
            closestSum = abs(currentSum - foundSum);
            closestIndex = i;
        }
    }
    return closestIndex;
}

void fill_with_chars(char* block)
{
    if(block != NULL)
    {
        for(int i =0; i < BLOCK_SIZE; i++)
        {
            block[i] = (char)(rand()%100 + 1);
        }
    }
}

void fill_with_empty(char* block)
{
    if(block != NULL)
    {
        for(int i = 0; i < BLOCK_SIZE; i++)
            block[i] = '\0';
    }
}