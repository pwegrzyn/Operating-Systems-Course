/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#ifndef _COMMON_H
#define _COMMON_H

#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 199309L

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<time.h>
#include<sys/time.h>
#include<unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include<semaphore.h>
#include<sys/mman.h>

// Names of the particular semaphores
#define QUEUE_SEM_NAME "/queue_sem"
#define CHAIR_SEM_NAME "/chair_sem"
#define BARBER_SEM_NAME "/barber_sem"
#define CLIENT_SEM_NAME "/client_sem"
#define SHARED_SEM_NAME "/shared_sem"
#define SHARED_MEM_NAME "/shared_mem"

// Permissions for the IPC
#define PERMISSIONS 0640

// Maximum size of the queue in the salon
#define MAX_NO_CLIENTS 256
#define MAX_NO_CHILDREN 1024

// Represents the state of the barber
typedef enum barber_state_tag
{
    SLEEPING,
    WORKING
} barber_state;

// Represents the shared info about the current state of the salon
typedef struct barbers_salon_tag
{
    pid_t queue[MAX_NO_CLIENTS];
    int current_clients;
    int no_chairs;
    pid_t at_chair;
    int head;
    int tail;
    barber_state b_state;
} barbers_salon;

#endif