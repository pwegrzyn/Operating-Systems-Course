/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include "iofuncs.h"

// Name of the file to save the time measurements
#ifndef TIME_MEASURE_FILE
#define TIME_MEASURE_FILE "wyniki.txt"
#endif

// Used to specify the main functionality of the program
enum op_type {GEN, SORT, COPY};

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

// Helper function for printing errors;
void sig_arg_err(void)
{
    printf("Wrong argument format.\n");
    exit(1);
}

// MAIN function
int main(int argc, char **argv)
{
    enum op_type operation;
    
    if(argc == 1) sig_arg_err();

    if(strcmp(argv[1], "generate") == 0)
        operation = GEN;
    else if(strcmp(argv[1], "sort") == 0)
        operation = SORT;
    else if(strcmp(argv[1], "copy") == 0)
        operation = COPY;
    else
        sig_arg_err();

    const char *input_file, *output_file;
    int no_records, record_size;
    int is_sys = -1;

    struct timeval start_s, end_s;
    struct timeval start_u, end_u;
    struct rusage usage;

    const char *report_file_name = TIME_MEASURE_FILE;

    // Open a file to save the time measurements
    FILE *report_file = fopen(report_file_name, "a");
    if (report_file == NULL)
    {
        printf("Error opening the report file! Exiting...\n");
        exit(1);
    }

    switch(operation)
    {
        case GEN:
            if(argc < 5) sig_arg_err();
            output_file = argv[2];
            no_records = (int)strtol(argv[3], NULL, 10);
            record_size = (int)strtol(argv[4], NULL, 10);
            srand(time(NULL));
            tee(report_file, "Genereting and saving %d records with each of "
                "size %d to the file %s...\n", no_records, record_size,
                output_file);
            int gen_res = generate(record_size, no_records, output_file);
            if(gen_res)
            {
                fclose(report_file);
                exit(gen_res);
            }
            else tee(report_file, "Generation completed successfully.\n");
            break;

        case SORT:
            if(argc < 6) sig_arg_err();
            input_file = argv[2];
            no_records = (int)strtol(argv[3], NULL, 10);
            record_size = (int)strtol(argv[4], NULL, 10);
            if(strcmp(argv[5], "sys") == 0)
                is_sys = 1;
            else if(strcmp(argv[5], "lib") == 0)
                is_sys = 0;
            if(is_sys == -1) sig_arg_err();
            int sort_res;

            if(is_sys == 1)
            {
                tee(report_file, "Sorting the file %s which has %d records each of which\n"
                    "contains %d bytes using SYSTEM functions...\n", input_file, no_records,
                    record_size);

                getrusage(RUSAGE_SELF, &usage);
                start_s = usage.ru_stime;
                start_u = usage.ru_utime;

                sort_res = sort_sys(input_file, no_records, record_size);

                getrusage(RUSAGE_SELF, &usage);
                end_s = usage.ru_stime;
                end_u = usage.ru_utime;

                if(sort_res == -1)
                {
                    fclose(report_file);
                    exit(1);
                }
                else tee(report_file, "Sorting completed successfully. It took:\n"
                         "%ld microseconds user time and %ld microseconds system time\n",
                          clctd(start_u, end_u), clctd(start_s, end_s));
            }
            else if(is_sys == 0)
            {
                tee(report_file, "Sorting the file %s which has %d records each of which\n"
                    "contains %d bytes using LIBRARY functions...\n", input_file, no_records,
                    record_size);

                getrusage(RUSAGE_SELF, &usage);
                start_s = usage.ru_stime;
                start_u = usage.ru_utime;

                sort_res = sort_lib(input_file, no_records, record_size);

                getrusage(RUSAGE_SELF, &usage);
                end_s = usage.ru_stime;
                end_u = usage.ru_utime;

                if(sort_res == -1) 
                {
                    fclose(report_file);
                    exit(1);
                }
                else tee(report_file, "Sorting completed successfully. It took:\n"
                         "%ld microseconds user time and %ld microseconds system time\n",
                         clctd(start_u, end_u), clctd(start_s, end_s));
            }
            break;

        case COPY:
            if(argc < 7) sig_arg_err();
            input_file = argv[2];
            output_file = argv[3];
            no_records = (int)strtol(argv[4], NULL, 10);
            int buffer_size = (int)strtol(argv[5], NULL, 10);
            if(strcmp(argv[6], "sys") == 0)
                is_sys = 1;
            else if(strcmp(argv[6], "lib") == 0)
                is_sys = 0;
            if(is_sys == -1) sig_arg_err();
            int copy_res;

            if(is_sys == 1)
            {
                tee(report_file, "Copying %d records from file %s to file %s "
                    "using a buffer with size %d and SYSTEM functions...\n", no_records, input_file,
                    output_file, buffer_size);

                getrusage(RUSAGE_SELF, &usage);
                start_s = usage.ru_stime;
                start_u = usage.ru_utime;

                copy_res = copy_sys(input_file, output_file, buffer_size, no_records);

                getrusage(RUSAGE_SELF, &usage);
                end_s = usage.ru_stime;
                end_u = usage.ru_utime;

                if(copy_res == -1) 
                {
                    fclose(report_file);
                    exit(1);
                }
                else tee(report_file, "Copying process successful. Copied %d bytes between the files. It took:\n"
                         "%ld microseconds user time and %ld microseconds system time\n", copy_res,
                         clctd(start_u, end_u), clctd(start_s, end_s));
            }
            else if(is_sys == 0)
            {
                tee(report_file, "Copying %d records from file %s to file %s "
                    "using a buffer with size %d and C LIBRARY functions...\n", no_records, input_file,
                    output_file, buffer_size);

                getrusage(RUSAGE_SELF, &usage);
                start_s = usage.ru_stime;
                start_u = usage.ru_utime;

                copy_res = copy_lib(input_file, output_file, buffer_size, no_records);

                getrusage(RUSAGE_SELF, &usage);
                end_s = usage.ru_stime;
                end_u = usage.ru_utime;

                if(copy_res == -1) 
                {
                    fclose(report_file);
                    exit(1);
                }
                else tee(report_file, "Copying process successful. Copied %d bytes between the files. It took:\n"
                         "%ld microseconds user time and %ld microseconds system time\n", copy_res,
                         clctd(start_u, end_u), clctd(start_s, end_s));
            }
            break;
    }


    fclose(report_file);
    return 0;
}