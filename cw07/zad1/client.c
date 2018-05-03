/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#include "common.h"

// Global variables
int queue_semid, chair_semid, barber_semid, client_semid, shared_semid, shared_memid;
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

// Generates a key from a ftok seed
key_t get_key(int seed)
{
    key_t result;
    if((result=ftok(getenv("HOME"), seed)) == -1)
    {
        perror("Error while generating a key");
        exit(EXIT_FAILURE);
    }
    return result;
}

// Gets a given semaphore id
int get_sem(key_t key)
{
    int result;
    if((result=semget(key, 0, 0)) == -1)
    {
        perror("Error while getting a semaphore");
        exit(EXIT_FAILURE);
    }
    return result;
}

// Clears all the previously created resources (ipcs)
void perform_cleanup(void)
{
    if(shared != NULL && shmdt(shared) == -1)
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
void sem_give(int semid, int value)
{
    struct sembuf buff;
    buff.sem_num = 0;
    buff.sem_flg = 0;
    buff.sem_op = value;
    if(semop(semid, &buff, 1) == -1)
    {
        perror("Error while posting semaphore");
        exit(EXIT_FAILURE);
    }
}

// Semaphore controls
void sem_take(int semid, int value)
{
    struct sembuf buff;
    buff.sem_num = 0;
    buff.sem_flg = 0;
    buff.sem_op = -value;
    if(semop(semid, &buff, 1) == -1)
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
            sem_take(shared_semid, 1);
            if(shared->b_state == SLEEPING && shared->current_clients == 0)
            {
                sprintf(buffer, "I am waking up the barber\n");
                print_tag(buffer);
                shared->b_state = WORKING;
                shared->at_chair = getpid();
                sem_give(queue_semid, 1);
                sem_give(shared_semid, 1);
                sprintf(buffer, "I am sitting on the chair\n");
                print_tag(buffer);
                sem_give(chair_semid, 1);
                sem_take(barber_semid, 1);
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
                sem_give(shared_semid, 1);
            }
            else if(shared->current_clients < shared->no_chairs)
            {
                sprintf(buffer, "Im taking the place number %d in the queue\n",
                    shared->current_clients);
                print_tag(buffer);
                shared->queue[shared->tail] = getpid();
                shared->tail = shared->tail + 1 % MAX_NO_CLIENTS;
                shared->current_clients++;
                sem_give(shared_semid, 1);
                int my_turn = 0;
                while(!my_turn)
                {
                    sem_take(shared_semid, 1);
                    if(shared->at_chair == getpid())
                    {
                        my_turn = 1;
                    }
                    else
                    {
                        sem_give(shared_semid, 1);
                        sem_take(client_semid, 1);
                    }
                }
                sem_give(shared_semid, 1);
                sprintf(buffer, "I have been invited by the barber, "
                    "Im sitting on the chair now\n");
                print_tag(buffer);
                sem_give(chair_semid, 1);
                sem_take(barber_semid, 1);
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

    key_t queue_semk, chair_semk, barber_semk, client_semk, shared_semk, shared_memk;
    queue_semk = get_key(QUEUE_SEMK_SEED);
    chair_semk = get_key(CHAIR_SEMK_SEED);
    barber_semk = get_key(BARBER_SEMK_SEED);
    client_semk = get_key(CLIENT_SEMK_SEED);
    shared_semk = get_key(SHARED_SEMK_SEED);
    shared_memk = get_key(SHARED_MEMK_SEED);

    atexit(perform_cleanup);

    queue_semid = get_sem(queue_semk);
    chair_semid = get_sem(chair_semk);
    barber_semid = get_sem(barber_semk);
    client_semid = get_sem(client_semk);
    shared_semid = get_sem(shared_semk);

    if((shared_memid=shmget(shared_memk, 0, 0)) == -1)
    {
        perror("Error while getting the shared memory space");
        exit(EXIT_FAILURE);
    }
    
    if((shared=shmat(shared_memid, NULL, 0)) == (void*)-1)
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