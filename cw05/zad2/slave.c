/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "shared.h"

// Helper function used to signalize argument errors
void sig_arg_err()
{
    printf("Wrong argument format.\n"
           "Usage: slave [fifo_file] [write_counter]\n");
    exit(1);
}

// MAIN function
int main(int argc, char **argv)
{
    if(argc != 3) sig_arg_err();
    const char *fifo_path;
    int write_counter;
    fifo_path = argv[1];
    write_counter = (int)strtol(argv[2], NULL, 10);

    int fd;
    fd = open(fifo_path, O_WRONLY);
    if(fd == -1)
    {
        fprintf(stderr, "SLAVE_%d: Error while opening the fifo file: %s\n", 
                getpid(), strerror(errno));
        exit(1);
    }

    printf("SLAVE_%d: My PID is %d.\n", getpid(), getpid());

    FILE *tmp_file;
    char buffer1[BUFFER_SIZE_1], buffer2[BUFFER_SIZE_2];

    for(int i = 0; i < write_counter; i++)
    {
        int nap_time;
        nap_time = rand()%4 + 2;
        sleep(nap_time);
        
        tmp_file = popen("date", "r");
        if(tmp_file == NULL)
        {
            fprintf(stderr, "Error occured while generating date.\n");
            close(fd);
            exit(1);
        }
        fread(buffer1, 1, BUFFER_SIZE_1, tmp_file);
        pclose(tmp_file);
        sprintf(buffer2, "%d - %s", (int)getpid(), buffer1);
        write(fd, buffer2, BUFFER_SIZE_2);
    }

    close(fd);
    return 0;
}