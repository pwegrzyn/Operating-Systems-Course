/* Systemy Operacyjne 2018 
 * Patryk Wegrzyn 
 * Cwiczenie 01 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <getopt.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "mbarr.h"

#ifdef DYNAMIC_RUNTIME_LOAD
#include <dlfcn.h>
#endif

// Defines how many times to we wish to measure the time it takes
// to perform ceratain operations
#ifndef NO_OP_ITERS
#define NO_OP_ITERS 1
#endif

// Name of the file to save the time measurements
#ifndef OUTPUT_FILE_NAME
#define OUTPUT_FILE_NAME "raport2.txt"
#endif

// Length of the array of blocks (numer of blocks)
#define NO_BLOCKS 1000

// Size of each block (number of char elements)
#define BLOCK_SIZE 100

// Save the name of the program
const char *prog_name;

/* Global array (stored in .bss and initialized with zeros) - static memory allocation */
char mbarr_static[NO_BLOCKS][BLOCK_SIZE];

// Help screen
void print_usage(FILE *stream, int e_code)
{
    fprintf(stream, "Usage:  %s [options] no_elements block_size allocation_method\n", prog_name);
    fprintf(stream,
            "   -h  --help              Display the help screen.\n"
            "   -c  --create            Create the array.\n"
            "   -f  --find index        Find the block with the closest sum to index-th block.\n"
            "   -s  --swap no_blo       Swap no_bl first blocks for new ones.\n"
            "   -a  --swap-alter no_bl  Swap no_bl first blocks for new ones (alternately).\n"
            "Note: allocation_method needs to be: dynamic or static.\n");
    exit(e_code);

}

// Used for time measurements
long clctd(struct timeval start, struct timeval end)
{
    return (long)((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
}

// Variadic function used to printf a message to stdout and another file simultanously
void tee(FILE *f, char const *msg, ...)
{     
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
    va_start(ap, msg);
    vfprintf(f, msg, ap);
    va_end(ap);
}

// MAIN
int main(int argc, char** argv)
{

// When the proper define is set during compilation time
// we load the required library dynamic during runtime later
#ifdef DYNAMIC_RUNTIME_LOAD
    void *dll_handle = dlopen("libmbarr_shared.so", RTLD_LAZY);
    if(dll_handle == NULL)
    {
        fprintf(stderr, "Error while dynamically loading a library - aborting.\n");
        exit(1);
    }
    mbarr* (*create_mbarr)(size_t, size_t) = dlsym(dll_handle, "create_mbarr");
    void (*delete_mbarr)(mbarr*) = dlsym(dll_handle, "delete_mbarr");
    void (*clear_block)(mem_block*) = dlsym(dll_handle, "clear_block");
    int (*find_closest)(mbarr*, size_t) = dlsym(dll_handle, "find_closest");
    int (*find_closest_static)(char [NO_BLOCKS][BLOCK_SIZE], size_t) = dlsym(dll_handle, "find_closest_static");
    mem_block* (*create_block)(size_t) = dlsym(dll_handle, "create_block");
    int (*add_block)(mbarr*, size_t, mem_block*) = dlsym(dll_handle, "add_block");
#endif

    // Structs describing the possible options of the program
    int next_opt;
    const char * const short_opts = "hcf:s:a:";
    const struct option long_opts[] = {
        {"help",        0, NULL, 'h'},
        {"create",      0, NULL, 'c'},
        {"find",        1, NULL, 'f'},
        {"swap",        1, NULL, 's'},
        {"swap-alter",  1, NULL, 'a'},
        {NULL,          0, NULL,  0 }
    };

    // Set variables for saving the options of the program
    prog_name = argv[0];

    // If no arguments were provided print help
    if(argc >= 1 && argc <= 4)
    {
        print_usage(stdout, 0);
    }
    
    int create = 0;
    
    int find = 0;
    int find_index = 0;
    
    int swap = 0;
    int swap_no_bl = 0;
    
    int swap_alter = 0;
    int swap_alter_no_bl = 0;

    // Main loop of the parsing process
    do {
        next_opt = getopt_long(argc, argv, short_opts, long_opts, NULL);

        switch (next_opt)
        {
            case 'h':
                print_usage(stdout, 0);
                break;
            
            case '?':
                print_usage(stderr, 1);
                break;

            case -1:
                break;
            
            case 'c':
                create = 1;
                break;

            case 'f':
                find = 1;
                find_index = (int)strtol(optarg, NULL, 10);
                break;

            case 's':
                swap = 1;
                swap_no_bl = (int)strtol(optarg, NULL, 10);
                break;

            case 'a':
                swap_alter = 1;
                swap_alter_no_bl = (int)strtol(optarg, NULL, 10);
                break;

            default:
                abort();
        }
    }
    while (next_opt != -1);
    
    // Some more variables used for saving CL parameters
    const char *alloc_meth_choice = NULL;
    int no_elements, block_size, iter;
    int is_alloc_dynamic;

    // Used for real time measurements
    struct timeval start_r, end_r;
    struct timeval start_s, end_s;
    struct timeval start_u, end_u;
    struct rusage usage;

    // Name of the output file
    const char *output_file_name = OUTPUT_FILE_NAME;

    // If the program is supposed to create a table...
    if(create)
    {
        // If too many arguments were provided, print error
        iter = optind;
        if(iter+2 >= argc) print_usage(stderr, 1);
        
        // Save the additional paramters
        no_elements = (int)strtol(argv[iter], NULL, 10);
        block_size = (int)strtol(argv[iter+1], NULL, 10);
        alloc_meth_choice = argv[iter+2];

        // Check whether or not to allocate the memory dynamically
        if(strcmp(alloc_meth_choice, "static") == 0)
            is_alloc_dynamic = 0;

        if(strcmp(alloc_meth_choice, "dynamic") == 0)
            is_alloc_dynamic = 1;

        if(strcmp(alloc_meth_choice, "static") && strcmp(alloc_meth_choice, "dynamic"))
            print_usage(stderr, 1);

        // Actual functionality of the program starts here
        printf("Starting requested tasks...\n");

        // Open a file to save the time measurements
        FILE *report_file = fopen(output_file_name, "a");
        if (report_file == NULL)
        {
            printf("Error opening file! Exiting...\n");
            exit(1);
        }

#ifdef STATIC_LIB

        fprintf(report_file, "Results from the program with static libraries:\n");

#elif SHARED_LIB

        fprintf(report_file, "Results from the program with shared libraries:\n");

#elif DYNAMIC_RUNTIME_LOAD

        fprintf(report_file, "Results from the program with dynamically loaded libraries:\n");        

#endif

        mbarr *new_array;

        // Seeding the random function
        srand(time(NULL));

        // One can define during compilation the amount of iterations of the
        // atomic operations (eg. -DNO_OP_ITERS=2);
        // if no definne is provided 1 is assumed as default value
        for(int g = 0; g < NO_OP_ITERS; g++) {
        
        // Branch responsible for dynamic allocation
        if(is_alloc_dynamic)
        {
            
            // Dynamically allocate the table and measure all 3 times
            gettimeofday(&start_r, NULL);
            getrusage(RUSAGE_SELF, &usage);
            start_s = usage.ru_stime;
            start_u = usage.ru_utime;

#ifndef DYNAMIC_RUNTIME_LOAD
            new_array = create_mbarr(no_elements, block_size);
#else
            new_array = (*create_mbarr)(no_elements, block_size);
#endif
            
            gettimeofday(&end_r, NULL);
            getrusage(RUSAGE_SELF, &usage);
            end_s = usage.ru_stime;
            end_u = usage.ru_utime;

            // Logging the results of the time measurement
            if(new_array != NULL)
            {
                tee(report_file, "Dynamically allocating an array of size %d " 
                    "with %d - sized blocks took:\n%ld microseconds real time\n"
                    "%ld microseconds system time\n%ld microseconds user time\n", no_elements, block_size,
                    clctd(start_r, end_r), clctd(start_s, end_s), clctd(start_u, end_u));
            }

            // Filling the array with random data
            for(int i = 0; i < no_elements && new_array != NULL; i++)
            {
                for(int j = 0; j < block_size; j++)
                {
                    new_array->blocks[i]->arr[j] = (char)(rand()%100 + 1);
                }
            }         

        }
        // Branch responsible for static allocation, i.e on compile time
        else
        {
            printf("Cannot provide the size for a statically allocated array.\n");
            
            gettimeofday(&start_r, NULL);
            getrusage(RUSAGE_SELF, &usage);
            start_s = usage.ru_stime;
            start_u = usage.ru_utime;
            
            // Nothing to allocate, since it allocates at compile time
            
            gettimeofday(&end_r, NULL);
            getrusage(RUSAGE_SELF, &usage);
            end_s = usage.ru_stime;
            end_u = usage.ru_utime;
            
            if(mbarr_static != NULL)
            {
                tee(report_file, "Statically allocating an array of size %d " 
                    "with %d - sized blocks took:\n%ld microseconds real time\n"
                    "%ld microseconds system time\n%ld microseconds user time\n"
                    "<<The allocation took place at compile time>>\n", NO_BLOCKS, BLOCK_SIZE,
                    clctd(start_r, end_r), clctd(start_s, end_s), clctd(start_u, end_u));
            }

            for(int i = 0; i < NO_BLOCKS; i++)
            {
                for(int j = 0; j < BLOCK_SIZE; j++)
                {
                    mbarr_static[i][j] = (char)(rand()%100 + 1);
                }
            }

        }

        // If the find request has been issued
        if(find)
        {
            gettimeofday(&start_r, NULL);
            getrusage(RUSAGE_SELF, &usage);
            start_s = usage.ru_stime;
            start_u = usage.ru_utime;

            int found = -1;

#ifndef DYNAMIC_RUNTIME_LOAD
            if(is_alloc_dynamic) 
            {
                found = find_closest(new_array, find_index);
            }
            else
            {
                found = find_closest_static(mbarr_static, find_index);
            }
#else
            if(is_alloc_dynamic) 
            {
                found = (*find_closest)(new_array, find_index);
            }
            else
            {
                found = (*find_closest_static)(mbarr_static, find_index);
            }
#endif 
            gettimeofday(&end_r, NULL);
            getrusage(RUSAGE_SELF, &usage);
            end_s = usage.ru_stime;
            end_u = usage.ru_utime;

            if(found == -1)
            {
                printf("No closest block has been found.\n");
                fprintf(report_file, "No closest block has been found.\n");
            }
            else
            {
                tee(report_file, "Found block with closest sum to block nr %d: %d. It took:\n"
                    "%ld microseconds real time\n%ld microseconds system time\n"
                    "%ld microseconds user timer\n", find_index, found,
                    clctd(start_r, end_r), clctd(start_s, end_s), clctd(start_u, end_u));
            }

        }

        // Swapping a continuous array of memory blocks of a given size
        // Only done on the dynamically allocated array, since of the static version
        // there would be no point in measuring the time it takes
        if(swap && is_alloc_dynamic)
        {
            if(swap_no_bl > no_elements) swap_no_bl = no_elements;

            gettimeofday(&start_r, NULL);
            getrusage(RUSAGE_SELF, &usage);
            start_s = usage.ru_stime;
            start_u = usage.ru_utime;
            
            // Looks ugly but has to be done to conform to the requirments of the exercise
#ifndef DYNAMIC_RUNTIME_LOAD
            for(int i = 0; i < swap_no_bl; i++)
            {
                clear_block(new_array->blocks[i]);
            }
            for(int i = 0; i < swap_no_bl; i++)
            {
                add_block(new_array, i, create_block(block_size));
            }
#else
            for(int i = 0; i < swap_no_bl; i++)
            {
                (*clear_block)(new_array->blocks[i]);
            }
            for(int i = 0; i < swap_no_bl; i++)
            {
                (*add_block)(new_array, i, create_block(block_size));
            }
#endif

            gettimeofday(&end_r, NULL);
            getrusage(RUSAGE_SELF, &usage);
            end_s = usage.ru_stime;
            end_u = usage.ru_utime;

            tee(report_file, "Clearing and then allocating %d continuos memory blocks " 
                "with each of size %d took:\n%ld microseconds real time\n"
                "%ld microseconds system time\n%ld microseconds user time\n", swap_no_bl, block_size,
                clctd(start_r, end_r), clctd(start_s, end_s), clctd(start_u, end_u));
        }

        // Similar to the functionality descibed above, only this one
        // alters between the operation of freeing and allocating memory
        if(swap_alter && is_alloc_dynamic)
        {
            if(swap_alter_no_bl > no_elements) swap_alter_no_bl = no_elements;
            
            gettimeofday(&start_r, NULL);
            getrusage(RUSAGE_SELF, &usage);
            start_s = usage.ru_stime;
            start_u = usage.ru_utime;
            
#ifndef DYNAMIC_RUNTIME_LOAD
            for(int i = 0; i < swap_alter_no_bl; i++)
            {
                clear_block(new_array->blocks[i]);
                add_block(new_array, i, create_block(block_size));
            }
#else
            for(int i = 0; i < swap_alter_no_bl; i++)
            {
                (*clear_block)(new_array->blocks[i]);
                (*add_block)(new_array, i, create_block(block_size));
            }
#endif

            gettimeofday(&end_r, NULL);
            getrusage(RUSAGE_SELF, &usage);
            end_s = usage.ru_stime;
            end_u = usage.ru_utime;

            tee(report_file, "Clearing and then allocating %d non-continuos memory blocks " 
                "(alternately) with each of size %d took:\n%ld microseconds real time\n"
                "%ld microseconds system time\n%ld microseconds user time\n", swap_alter_no_bl, block_size,
                clctd(start_r, end_r), clctd(start_s, end_s), clctd(start_u, end_u));
        } 

//if the array has been allocated dynmically, free the memory
#ifndef DYNAMIC_RUNTIME_LOAD
        if(is_alloc_dynamic) delete_mbarr(new_array);
#else
        if(is_alloc_dynamic) (*delete_mbarr)(new_array);
#endif
        }
        
        fclose(report_file);

    }
    
#ifdef DYNAMIC_RUNTIME_LOAD
    dlclose(dll_handle);
#endif

    return 0;
}
