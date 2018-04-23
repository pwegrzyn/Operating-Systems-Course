/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#define _XOPEN_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "shared.h"

// Helper function used to signalize argument errors
void sig_arg_err()
{
    printf("Wrong argument format.\n"
           "Usage: master [fifo_file]\n");
    exit(1);
}

// MAIN function
int main(int argc, char **argv)
{
    if(argc != 2) sig_arg_err();
    const char *fifo_path;
    fifo_path = argv[1];

    if(access(fifo_path, F_OK) == -1 && mkfifo(fifo_path, 0777) == -1)
    {
        printf("Error while creating fifo file: %s\n", strerror(errno));
        exit(1);
    }
    printf("MASTER_%d: Named pipe is ready, initializing listening...\n",
           getpid());

    int fd;
    fd = open(fifo_path, O_RDONLY);
    if(fd == -1)
    {
        fprintf(stderr, "MASTER_%d: Error while opening the fifo file: %s\n", 
                getpid(), strerror(errno));
        exit(1);
    }

    char buffer[BUFFER_SIZE_2];
    while(1)
    {
        if(read(fd, buffer, BUFFER_SIZE_2) == 0)
        {
            break;
        }
        printf("MASTER_%d: Received: %s", getpid(), buffer);
    }

    close(fd);
    return 0;
}