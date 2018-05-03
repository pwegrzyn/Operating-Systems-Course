/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#include "common.h"

// Global variables
sem_t *queue_semid, *chair_semid, *barber_semid, *client_semid, *shared_semid;
int shared_memid;
barbers_salon *shared;
int created_children;
int children_cleared;

// Used for printing the signal tag
void print_tag(const char *msg)
{
    struct timespec time;
    long microseconds;
    clock_gettime(CLOCK_MONOTONIC, &time);
    microseconds = (time.tv_sec * 1000000000 + time.tv_nsec) / 1000;
    printf("[CLIENT | TIME: %ld | PID: %d]: %s", microseconds, getpid(), msg);
}

// Helper function used to signalize argument errors
void sig_arg_err()
{
    printf("Wrong argument format\n"
           "Usage: client [number of clients] [number of shavings per client]\n");
    exit(EXIT_FAILURE);
}

// Gets a given semaphore id (creates a new one if neccessary)
sem_t* get_sem(const char *name)
{
    sem_t *result;
    if((result=sem_open(name, O_RDWR)) == SEM_FAILED)
    {
        perror("Error while getting a semaphore");
        exit(EXIT_FAILURE);
    }
    return result;
}

// Clears all the previously created resources (ipcs)
void perform_cleanup(void)
{
    if(queue_semid != NULL && sem_close(queue_semid) == -1)
    {
        perror("Error while removing the queue semaphore");
    }
    if(chair_semid != NULL && sem_close(chair_semid) == -1)
    {
        perror("Error while removing the chair semaphore");
    }
    if(barber_semid != NULL && sem_close(barber_semid) == -1)
    {
        perror("Error while removing the barber semaphore");
    }
    if(client_semid != NULL && sem_close(client_semid) == -1)
    {
        perror("Error while removing the client semaphore set");
    }
    if(shared_semid != NULL && sem_close(shared_semid) == -1)
    {
        perror("Error while removing the shared resources semaphore");
    }
    
    if(shared != NULL && munmap(shared, sizeof(barbers_salon)) == -1)
    {
        perror("Error while dettaching the shared memory space");
    }
    
    if (!children_cleared) {
        for(int i = 0; i < created_children; i++)
        {
            wait(NULL);
        }
        children_cleared = 1;
    }
}

// Semaphore controls
void sem_give(sem_t *sem)
{
    if(sem_post(sem) == -1)
    {
        perror("Error while posting semaphore");
        exit(EXIT_FAILURE);
    }
}

// Semaphore controls
void sem_take(sem_t *sem)
{
    if(sem_wait(sem) == -1)
    {
        perror("Error while waiting for semaphore");
        exit(EXIT_FAILURE);
    }
}

// Performs the main functionality of the client process
void spawn_clients(int no_clients, int no_shavings)
{
    int child_pid;
    for(int i = 0; i < no_clients; i++)
    {
        child_pid = fork();
        if(child_pid == 0)
        {
            break;
        }
        printf("[CLIENT HOST]: Client %d has been spawned\n", child_pid);
        created_children++;
    }
    if(child_pid < 0)
    {
        perror("Error while forking a client");
        exit(EXIT_SUCCESS);
    }

    if(child_pid == 0)
    {
        int left_shavings = no_shavings;
        char buffer[128] = {0};
        while(left_shavings > 0)
        {
            sem_take(shared_semid);
            if(shared->b_state == SLEEPING && shared->current_clients == 0)
            {
                sprintf(buffer, "I am waking up the barber\n");
                print_tag(buffer);
                shared->b_state = WORKING;
                shared->at_chair = getpid();
                sem_give(queue_semid);
                sem_give(shared_semid);
                sprintf(buffer, "I am sitting on the chair\n");
                print_tag(buffer);
                sem_give(chair_semid);
                sem_take(barber_semid);
                left_shavings--;
                sprintf(buffer, "The barber has finished my shaving - Im "
                    "leaving the salon\n");
                print_tag(buffer);
            }
            else if(shared->current_clients >= shared->no_chairs)
            {
                sprintf(buffer, "There's no place left in the waiting room"
                    " - Im leaving the salon\n");
                print_tag(buffer);
                sem_give(shared_semid);
            }
            else if(shared->current_clients < shared->no_chairs)
            {
                sprintf(buffer, "Im taking the place number %d in the queue\n",
                    shared->current_clients);
                print_tag(buffer);
                shared->queue[shared->tail] = getpid();
                shared->tail = shared->tail + 1 % MAX_NO_CLIENTS;
                shared->current_clients++;
                sem_give(shared_semid);
                int my_turn = 0;
                while(!my_turn)
                {
                    sem_take(shared_semid);
                    if(shared->at_chair == getpid())
                    {
                        my_turn = 1;
                    }
                    else
                    {
                        sem_give(shared_semid);
                        sem_take(client_semid);
                    }
                }
                sem_give(shared_semid);
                sprintf(buffer, "I have been invited by the barber, "
                    "Im sitting on the chair now\n");
                print_tag(buffer);
                sem_give(chair_semid);
                sem_take(barber_semid);
                left_shavings--;
                sprintf(buffer, "The barber has finished my shaving - Im "
                    "leaving the salon\n");
                print_tag(buffer);
            }
        }
        exit(EXIT_SUCCESS);
    }
}

// MAIN function
int main(int argc, char **argv)
{
    if (argc != 3) sig_arg_err();
    int no_clients, no_shavings;
    no_clients = (int)strtol(argv[1], NULL, 10);
    no_shavings = (int)strtol(argv[2], NULL, 10);

    atexit(perform_cleanup);

    queue_semid = get_sem(QUEUE_SEM_NAME);
    chair_semid = get_sem(CHAIR_SEM_NAME);
    barber_semid = get_sem(BARBER_SEM_NAME);
    client_semid = get_sem(CLIENT_SEM_NAME);
    shared_semid = get_sem(SHARED_SEM_NAME);

    if((shared_memid=shm_open(SHARED_MEM_NAME, O_RDWR, PERMISSIONS)) == -1)
    {
        perror("Error while getting the shared memory space");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(shared_memid, sizeof(barbers_salon)) == -1)
    {
        perror("Error while truncating the shared memory space");
        exit(EXIT_FAILURE);
    }
    
    if((shared=mmap(NULL, sizeof(barbers_salon), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memid, 0)) == (void*)-1)
    {
        perror("Error while attaching the shared memory space");
        exit(EXIT_FAILURE);
    }

    spawn_clients(no_clients, no_shavings);

    if (!children_cleared) {
        for(int i = 0; i < created_children; i++)
        {
            wait(NULL);
        }
        children_cleared = 1;
    }
    
    exit(EXIT_SUCCESS);
}