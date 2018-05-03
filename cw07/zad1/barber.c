/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#include "common.h"

// Global variables
int queue_semid, chair_semid, barber_semid, client_semid, shared_semid, shared_memid;
int barber_interrupted = 0;
barbers_salon *shared;

// Error messages
const char *received_sigterm_msg = "\n[BARBER] Received an interrupt - terminating the"
    " barber process...\n";

// Used for printing the signal tag
void print_tag(const char *message)
{
    struct timespec time;
    long microseconds;
    clock_gettime(CLOCK_MONOTONIC, &time);
    microseconds = (time.tv_sec * 1000000000 + time.tv_nsec) / 1000;
    printf("[BARBER | TIME: %ld | PID: %d]: %s", microseconds, getpid(), message);
}

// Helper function used to signalize argument errors
void sig_arg_err()
{
    fprintf(stderr, "Wrong argument format\n"
           "Usage: barber [number of seats]\n");
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

// Gets a given semaphore id (creates a new one if neccessary)
int get_sem(key_t key, int how_many)
{
    int result;
    if((result=semget(key, how_many, IPC_CREAT | PERMISSIONS)) == -1)
    {
        perror("Error while getting a semaphore");
        exit(EXIT_FAILURE);
    }
    return result;
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

// Semaphore controls
void sem_init(int semid, int value)
{
    union semun arg;
    arg.val = value;
    if(semctl(semid, 0, SETVAL, arg) == -1)
    {
        perror("Error while setting the initial value of the semaphore");
        exit(EXIT_FAILURE);
    }
}

// Initializes the shared memory area with proper values
void initialize_barbers_salon(int no_seats)
{
    shared->at_chair = -1;
    shared->b_state = WORKING;
    shared->current_clients = 0;
    shared->head = 0;
    shared->tail = 0;
    shared->no_chairs = no_seats;
    for(int i = 0; i < MAX_NO_CLIENTS; i++)
    {
        shared->queue[i] = -1;
    }
}

// Handles all the functionality of the barber process
void handle_clients()
{
    char buffer[128] = {0};
    while(!barber_interrupted)
    {
        sem_take(shared_semid, 1);
        if(shared->current_clients == 0 && shared->b_state == WORKING && shared->at_chair == -1)
        {
            shared->b_state = SLEEPING;
            print_tag("I am going to sleep\n");
            sem_give(shared_semid, 1);
            sem_take(queue_semid, 1);
            print_tag("I am waking up\n");
        }
        else if(shared->b_state == WORKING && shared->at_chair != -1)
        {
            sem_take(chair_semid, 1);
            sprintf(buffer, "The client %d has sat down. I begin "
                "shaving him\n", shared->at_chair);
            print_tag(buffer);
            sprintf(buffer, "I have finished shaving the client %d\n",
                shared->at_chair);
            print_tag(buffer);
            shared->at_chair = -1;
            sem_give(barber_semid, 1);
            sem_give(shared_semid, 1);
        }
        else if(shared->b_state == WORKING)
        {
            shared->at_chair = shared->queue[shared->head];
            shared->head = shared->head + 1 % MAX_NO_CLIENTS;
            sprintf(buffer, "I am inviting the client %d\n",
                shared->at_chair);
            print_tag(buffer);
            sem_give(client_semid, shared->current_clients);
            shared->current_clients--;
            sem_give(shared_semid, 1);
            sem_take(chair_semid, 1);
            sprintf(buffer, "The client %d has sat down. I begin "
                "shaving him\n", shared->at_chair);
            print_tag(buffer);
            sprintf(buffer, "I have finished shaving the client %d\n",
                shared->at_chair);
            print_tag(buffer);
            shared->at_chair = -1;
            sem_give(barber_semid, 1);
            sem_give(shared_semid, 1);
        }
    }
}

// Clears all the previously created resources (ipcs)
void perform_cleanup(void)
{
    if(queue_semid != 0 && semctl(queue_semid, 0, IPC_RMID) == -1)
    {
        perror("Error while removing the queue semaphore");
    }
    if(chair_semid != 0 && semctl(chair_semid, 0, IPC_RMID) == -1)
    {
        perror("Error while removing the chair semaphore");
    }
    if(barber_semid != 0 && semctl(barber_semid, 0, IPC_RMID) == -1)
    {
        perror("Error while removing the barber semaphore");
    }
    if(client_semid != 0 && semctl(client_semid, 0, IPC_RMID) == -1)
    {
        perror("Error while removing the client semaphore set");
    }
    if(shared_semid != 0 && semctl(shared_semid, 0, IPC_RMID) == -1)
    {
        perror("Error while removing the shared resources semaphore");
    }

    if(shared != NULL && shmdt(shared) == -1)
    {
        perror("Error while dettaching the shared memory space");
    }
    if(shared_memid != 0 && shmctl(shared_memid, IPC_RMID, NULL) == -1)
    {
        perror("Error while removing the shared memory space");
    }
}

// SIGTERM handler
void handler_sigterm(int signo)
{
    if(signo == SIGTERM)
    { 
        write(1, received_sigterm_msg, strlen(received_sigterm_msg));
        barber_interrupted = 1;
    }
}

// MAIN function
int main(int argc, char **argv)
{
    if (argc != 2) sig_arg_err();
    int no_seats = (int)strtol(argv[1], NULL, 10);
    if (no_seats > MAX_NO_CLIENTS)
    {
        fprintf(stderr, "The maximum size of the queue is %d\n", MAX_NO_CLIENTS);
        exit(EXIT_FAILURE);
    }

    key_t queue_semk, chair_semk, barber_semk, client_semk, shared_semk, shared_memk;
    queue_semk = get_key(QUEUE_SEMK_SEED);
    chair_semk = get_key(CHAIR_SEMK_SEED);
    barber_semk = get_key(BARBER_SEMK_SEED);
    client_semk = get_key(CLIENT_SEMK_SEED);
    shared_semk = get_key(SHARED_SEMK_SEED);
    shared_memk = get_key(SHARED_MEMK_SEED);

    atexit(perform_cleanup);

    queue_semid = get_sem(queue_semk, 1);
    chair_semid = get_sem(chair_semk, 1);
    barber_semid = get_sem(barber_semk, 1);
    client_semid = get_sem(client_semk, 1);
    shared_semid = get_sem(shared_semk, 1);

    sem_init(queue_semid, 0);
    sem_init(chair_semid, 0);
    sem_init(barber_semid, 0);
    sem_init(client_semid, 0);
    sem_init(shared_semid, 1);

    if((shared_memid=shmget(shared_memk, sizeof(barbers_salon), IPC_CREAT | PERMISSIONS)) == -1)
    {
        perror("Error while getting the shared memory space");
        exit(EXIT_FAILURE);
    }
    
    if((shared=shmat(shared_memid, NULL, 0)) == (void*)-1)
    {
        perror("Error while attaching the shared memory space");
        exit(EXIT_FAILURE);
    }

    if(signal(SIGTERM, handler_sigterm) == SIG_ERR)
    {
        perror("Error while setting the SIGTERM handler");
        exit(EXIT_FAILURE);
    }

    initialize_barbers_salon(no_seats);

    handle_clients();

    exit(EXIT_SUCCESS);
}