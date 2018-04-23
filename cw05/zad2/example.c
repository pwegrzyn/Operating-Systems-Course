/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define NO_SLAVES 5

int main(void)
{
    pid_t master;
    master = fork();
    if(master == 0)
    {
        if(execlp("./master", "master", "fifo", NULL) == -1)
        {
            fprintf(stderr, "Error encountered while trying to "
                "execute the master: %s\n", strerror(errno));
            exit(1);
        }
    }
    for(int i = 0; i < NO_SLAVES; i++)
    {
        if(fork() == 0)
        {
            int no_writes;
            char buffer[10];
            srand(time(NULL) ^ (getpid()<<16));
            no_writes = rand()%10 + 1;
            sprintf(buffer, "%d", no_writes);
            if(execlp("./slave", "slave", "fifo", buffer, NULL) == -1)
            {
                fprintf(stderr, "Error encountered while trying to "
                    "execute a slave: %s\n", strerror(errno));
                exit(1);
            }
        }
        sleep(1);
    }

    for(int i = 0; i < NO_SLAVES + 1; i++)
    {
        wait(NULL);
    }
    
    return 0;
}