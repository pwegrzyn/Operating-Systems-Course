/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "iofuncs.h"

// Generate random bytes arrays in a file
int generate(size_t record_size, int no_records, const char *output_file)
{
    FILE *file = fopen(output_file, "w");
    if (file == NULL)
    {
        fprintf(stderr,"Error opening file! Exiting...\n");
        return 3;
    }

    unsigned char *record = (unsigned char*)malloc(record_size);
    if(record == NULL)
    {
        fprintf(stderr, "Error while allocationg memory.\n");
        fclose(file);
        return 2;
    }

    for(int i = 0; i < no_records; i++)
    {
        for(int j = 0; j < record_size - 1; j++)
        {
            record[j] = (unsigned char)((rand() % 127) + 1);
        }
        record[record_size - 1] = '\0';
        fwrite(record, sizeof(unsigned char), record_size, file);
    }

    free(record);
    if(fclose(file) != 0)
    {
        fprintf(stderr, "Error while closing file: %s\n", 
                strerror(errno));
        return 3;
    }
    return 0;
}

// Copy no_records records from in_file to out_file 
// using a buffer_size buffer and system functions;
// assumes records are separated with a NULL byte
int copy_sys(const char *in_file, const char *out_file, 
             size_t buffer_size, int no_records)
{
    if(in_file == NULL || out_file == NULL ||
       buffer_size <= 0 || no_records <= 0) return -1;

    unsigned char *buffer = (unsigned char*)malloc(buffer_size);
    if(buffer == NULL)
    {
        fprintf(stderr, "Error while allocating memory.\n");
        return -1;
    }
    int in_fd, out_fd;
    in_fd = open(in_file, O_RDONLY);
    if(in_fd == -1)
    {
        fprintf(stderr, "Error while opening file: %s\n", strerror(errno));
        free(buffer);
        return -1;
    }
    out_fd = open(out_file, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
    if(out_fd == -1)
    {
        fprintf(stderr, "Error while opening file: %s\n", strerror(errno));
        free(buffer);
        close(in_fd);
        return -1;
    }

    // Measure the size of a record in the file
    char c;
    size_t record_size = 0;
    while(read(in_fd, &c, 1) == 1)
    {
        record_size++;
        if(c == '\0') break;
    }
    lseek(in_fd, 0, SEEK_SET);

    size_t total_copy_size = record_size * no_records;
    size_t bytes_to_copy = total_copy_size;
    size_t how_many_read, how_many_read_cp, how_many_written;

    while(bytes_to_copy > 0)
    {
        how_many_read = read(in_fd, buffer, buffer_size);
        how_many_read_cp = how_many_read;
        if(how_many_read == -1)
        {
            fprintf(stderr, "Error while reading from file: %s\n", strerror(errno));
            free(buffer);
            close(in_fd);
            close(out_fd);
            return (total_copy_size - bytes_to_copy);
        }
        while(how_many_read > 0) 
        {
            how_many_written = write(out_fd, buffer, how_many_read);
            if(how_many_written == -1)
            {
                fprintf(stderr, "Error while writing to file: %s\n", strerror(errno));
                free(buffer);
                close(in_fd);
                close(out_fd);
                return (total_copy_size - bytes_to_copy);
            }
            how_many_read -= how_many_written;
        }
        bytes_to_copy -= how_many_read_cp;
        if(how_many_read_cp == 0) break;
    }

    free(buffer);
    if(close(in_fd) == -1)
    {
        fprintf(stderr, "Error while closing file: %s\n", strerror(errno));
    }
    if(close(out_fd) == -1)
    {
        fprintf(stderr, "Error while closing file: %s\n", strerror(errno));
    }
    return total_copy_size - bytes_to_copy;
}

// Same as above only works with C library functions
int copy_lib(const char *in_file, const char *out_file,
             size_t buffer_size, int no_records)
{
    if(in_file == NULL || out_file == NULL ||
       buffer_size <= 0 || no_records <= 0) return -1;

    unsigned char *buffer = (unsigned char*)malloc(buffer_size);
    if(buffer == NULL)
    {
        fprintf(stderr, "Error while allocating memory.\n");
        return -1;
    }

    FILE *in_ptr, *out_ptr;
    in_ptr = fopen(in_file, "r");
    if(in_ptr == NULL)
    {
        fprintf(stderr, "Error while opening file: %s\n", strerror(errno));
        free(buffer);
        return -1;
    }
    out_ptr = fopen(out_file, "w");
    if(out_ptr == NULL)
    {
        fprintf(stderr, "Error while opening file: %s\n", strerror(errno));
        free(buffer);
        fclose(in_ptr);
        return -1;
    }

    char c;
    size_t record_size = 0;
    while(fread(&c, 1, 1, in_ptr) == 1)
    {
        record_size++;
        if(c == '\0') break;
    }
    if(fseek(in_ptr, 0, 0))
    {
        fprintf(stderr, "Error while operating on the file pointer: %s\n", strerror(errno));
        free(buffer);
        fclose(in_ptr);
        fclose(out_ptr);
        return -1;
    }

    size_t total_copy_size = record_size * no_records;
    size_t bytes_to_copy = total_copy_size;
    size_t how_many_read, how_many_read_cp, how_many_written;

    while(bytes_to_copy > 0)
    {
        how_many_read = fread(buffer, 1, buffer_size, in_ptr);
        how_many_read_cp = how_many_read;
        if(ferror(in_ptr))
        {
            fprintf(stderr, "Error while reading from file\n");
            free(buffer);
            fclose(in_ptr);
            fclose(out_ptr);
            return (total_copy_size - bytes_to_copy);
        }
        while(how_many_read > 0) 
        {
            how_many_written = fwrite(buffer, 1, how_many_read, out_ptr);
            if(ferror(out_ptr))
            {
                fprintf(stderr, "Error while writing to file\n");
                free(buffer);
                fclose(in_ptr);
                fclose(out_ptr);
                return (total_copy_size - bytes_to_copy);
            }
            how_many_read -= how_many_written;
        }
        bytes_to_copy -= how_many_read_cp;
        if(how_many_read_cp == 0) break;
    }

    free(buffer);
    if(fclose(in_ptr) == EOF)
    {
        fprintf(stderr, "Error while closing file: %s\n", strerror(errno));
    }
    if(fclose(out_ptr) == EOF)
    {
        fprintf(stderr, "Error while closing file: %s\n", strerror(errno));
    }
    return total_copy_size - bytes_to_copy;            
}

// Sort no_records records in the file each of which contains record_size bytes
// using simple insertion sort and comparing by the ASCII code of the first element
// in the record; uses system functions and assumes records are separated with
// a NULL byte
int sort_sys(const char *input_file, int no_records, size_t record_size)
{
    if(input_file == NULL || no_records == 0 || record_size == 0)
        return -1;

    unsigned char *buffer1 = (unsigned char*)malloc(record_size);
    unsigned char *buffer2 = (unsigned char*)malloc(record_size);
    if(buffer1 == NULL || buffer2 == NULL)
    {
        fprintf(stderr, "Error while allocating memory.\n");
        return -1;
    }

    int sorted_fd;
    sorted_fd = open(input_file, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if(sorted_fd == -1)
    {
        fprintf(stderr, "Error while opening file %s: %s\n", input_file, strerror(errno));
        free(buffer1);
        free(buffer2);
        return -1;
    }

    int read_bytes;
    unsigned char key, curr;
    int j;

    for(int i = 1; i < no_records; i++)
    {
        lseek(sorted_fd, i * record_size, SEEK_SET);
        read_bytes = read(sorted_fd, buffer1, record_size);
        if(read_bytes == -1)
        {
            fprintf(stderr, "Error while reading from file %s: %s\n", 
                    input_file, strerror(errno));
            free(buffer1);
            free(buffer2);
            close(sorted_fd);
            return -1;
        }
        key = buffer1[0];
        j = i - 1;
        lseek(sorted_fd, j * record_size, SEEK_SET);
        read_bytes = read(sorted_fd, buffer2, record_size);
        if(read_bytes == -1)
        {
            fprintf(stderr, "Error while reading from file %s: %s\n", 
                    input_file, strerror(errno));
            free(buffer1);
            free(buffer2);
            close(sorted_fd);
            return -1;
        }
        curr = buffer2[0];
        while(j >= 0 && curr > key)
        {
            lseek(sorted_fd, (j + 1) * record_size, SEEK_SET);
            write(sorted_fd, buffer2, record_size);
            j--;
            if(j >= 0)
            {
                lseek(sorted_fd, j * record_size, SEEK_SET);
                read_bytes = read(sorted_fd, buffer2, record_size);
                if(read_bytes == -1)
                {
                    fprintf(stderr, "Error while reading from file %s: %s\n", 
                        input_file, strerror(errno));
                    free(buffer1);
                    free(buffer2);
                    close(sorted_fd);
                    return -1;
                }
                curr = buffer2[0];
            }
        }
        lseek(sorted_fd, (j + 1) * record_size, SEEK_SET);
        write(sorted_fd, buffer1, record_size);
    }    

    free(buffer1);
    free(buffer2);
    if(close(sorted_fd) == -1)
    {
        fprintf(stderr, "Error while closing file %s: %s\n", input_file, strerror(errno));
    }
    return 0;       
}

// Same as sort_sys(), only this one uses IO functions from the C Standard Library
int sort_lib(const char *input_file, int no_records, size_t record_size)
{
    if(input_file == NULL || no_records == 0 || record_size == 0)
        return -1;

    unsigned char *buffer1 = (unsigned char*)malloc(record_size);
    unsigned char *buffer2 = (unsigned char*)malloc(record_size);
    if(buffer1 == NULL || buffer2 == NULL)
    {
        fprintf(stderr, "Error while allocating memory.\n");
        return -1;
    }

    FILE *sorted_ptr;
    sorted_ptr = fopen(input_file, "r+");
    if(sorted_ptr == NULL)
    {
        fprintf(stderr, "Error while opening file %s: %s\n", input_file, strerror(errno));
        free(buffer1);
        free(buffer2);
        return -1;
    }

    unsigned char key, curr;
    int j;

    for(int i = 1; i < no_records; i++)
    {
        fseek(sorted_ptr, i * record_size, 0);
        fread(buffer1, 1, record_size, sorted_ptr);
        if(ferror(sorted_ptr))
        {
            fprintf(stderr, "Error while reading from file %s\n", 
                    input_file);
            free(buffer1);
            free(buffer2);
            fclose(sorted_ptr);
            return -1;
        }
        key = buffer1[0];
        j = i - 1;
        fseek(sorted_ptr, j * record_size, 0);
        fread(buffer2, 1, record_size, sorted_ptr);
        if(ferror(sorted_ptr))
        {
            fprintf(stderr, "Error while reading from file %s\n", 
                    input_file);
            free(buffer1);
            free(buffer2);
            fclose(sorted_ptr);
            return -1;
        }
        curr = buffer2[0];
        while(j >= 0 && curr > key)
        {
            fseek(sorted_ptr, (j + 1) * record_size, 0);
            fwrite(buffer2, 1, record_size, sorted_ptr);
            j--;
            if(j >= 0)
            {
                fseek(sorted_ptr, j * record_size, 0);
                fread(buffer2, 1, record_size, sorted_ptr);
                if(ferror(sorted_ptr))
                {
                    fprintf(stderr, "Error while reading from file %s\n", 
                        input_file);
                    free(buffer1);
                    free(buffer2);
                    fclose(sorted_ptr);
                    return -1;
                }
                curr = buffer2[0];
            }
        }
        fseek(sorted_ptr, (j + 1) * record_size, 0);
        fwrite(buffer1, 1, record_size, sorted_ptr);
    }    

    free(buffer1);
    free(buffer2);
    fclose(sorted_ptr);
    return 0;     
}