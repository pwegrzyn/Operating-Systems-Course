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

// Seeds for the ftok function for particular ipcs
#define QUEUE_SEMK_SEED 64
#define CHAIR_SEMK_SEED 65
#define BARBER_SEMK_SEED 66
#define CLIENT_SEMK_SEED 67
#define SHARED_SEMK_SEED 68
#define SHARED_MEMK_SEED 69

// Permissions for the IPC
#define PERMISSIONS 0640

// Maximum size of the queue in the salon
#define MAX_NO_CLIENTS 256
#define MAX_NO_CHILDREN 1024

// Used for semtcl()
union semun 
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buff;
};

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