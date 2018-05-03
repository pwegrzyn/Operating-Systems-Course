/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#include "common.h"

// Global variables
sem_t *queue_semid, *chair_semid, *barber_semid, *client_semid, *shared_semid;
int shared_memid;
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

// Gets a given semaphore id (creates a new one if neccessary)
sem_t* get_sem(const char *name, int init)
{
    sem_t *result;
    if((result=sem_open(name, O_CREAT|O_RDWR, PERMISSIONS, init)) == SEM_FAILED)
    {
        perror("Error while getting a semaphore");
        exit(EXIT_FAILURE);
    }
    return result;
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
        sem_take(shared_semid);
        if(shared->current_clients == 0 && shared->b_state == WORKING && shared->at_chair == -1)
        {
            shared->b_state = SLEEPING;
            print_tag("I am going to sleep\n");
            sem_give(shared_semid);
            sem_take(queue_semid);
            print_tag("I am waking up\n");
        }
        else if(shared->b_state == WORKING && shared->at_chair != -1)
        {
            sem_take(chair_semid);
            sprintf(buffer, "The client %d has sat down. I begin "
                "shaving him\n", shared->at_chair);
            print_tag(buffer);
            sprintf(buffer, "I have finished shaving the client %d\n",
                shared->at_chair);
            print_tag(buffer);
            shared->at_chair = -1;
            sem_give(barber_semid);
            sem_give(shared_semid);
        }
        else if(shared->b_state == WORKING)
        {
            shared->at_chair = shared->queue[shared->head];
            shared->head = shared->head + 1 % MAX_NO_CLIENTS;
            sprintf(buffer, "I am inviting the client %d\n",
                shared->at_chair);
            print_tag(buffer);
            for(int i = 0; i < shared->current_clients; i++)
            {
                sem_give(client_semid);
            }
            shared->current_clients--;
            sem_give(shared_semid);
            sem_take(chair_semid);
            sprintf(buffer, "The client %d has sat down. I begin "
                "shaving him\n", shared->at_chair);
            print_tag(buffer);
            sprintf(buffer, "I have finished shaving the client %d\n",
                shared->at_chair);
            print_tag(buffer);
            shared->at_chair = -1;
            sem_give(barber_semid);
            sem_give(shared_semid);
        }
    }
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

    if(queue_semid != NULL && sem_unlink(QUEUE_SEM_NAME) == -1)
    {
        perror("Error while removing the queue semaphore");
    }
    if(chair_semid != NULL && sem_unlink(CHAIR_SEM_NAME) == -1)
    {
        perror("Error while removing the chair semaphore");
    }
    if(barber_semid != NULL && sem_unlink(BARBER_SEM_NAME) == -1)
    {
        perror("Error while removing the barber semaphore");
    }
    if(client_semid != NULL && sem_unlink(CLIENT_SEM_NAME) == -1)
    {
        perror("Error while removing the client semaphore set");
    }
    if(shared_semid != NULL && sem_unlink(SHARED_SEM_NAME) == -1)
    {
        perror("Error while removing the shared resources semaphore");
    }

    if(shared != NULL && munmap(shared, sizeof(barbers_salon)) == -1)
    {
        perror("Error while dettaching the shared memory space");
    }
    if(shared_memid != 0 && shm_unlink(SHARED_MEM_NAME) == -1)
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

    atexit(perform_cleanup);

    queue_semid = get_sem(QUEUE_SEM_NAME, 0);
    chair_semid = get_sem(CHAIR_SEM_NAME, 0);
    barber_semid = get_sem(BARBER_SEM_NAME, 0);
    client_semid = get_sem(CLIENT_SEM_NAME, 0);
    shared_semid = get_sem(SHARED_SEM_NAME, 1);

    if((shared_memid=shm_open(SHARED_MEM_NAME, O_CREAT|O_RDWR, PERMISSIONS)) == -1)
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

    if(signal(SIGTERM, handler_sigterm) == SIG_ERR)
    {
        perror("Error while setting the SIGTERM handler");
        exit(EXIT_FAILURE);
    }

    initialize_barbers_salon(no_seats);

    handle_clients();

    exit(EXIT_SUCCESS);
}