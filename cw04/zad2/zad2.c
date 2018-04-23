/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 199309L
#define __USE_UNIX98    

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define MAX_NO_CHILDREN 1000

// Global variables
int wait_time;
int options;
int no_children; 
int no_requests;
int curr_no_requests = 0;
int total_no_requests = 0;
pid_t received_requests[MAX_NO_CHILDREN];
pid_t children[MAX_NO_CHILDREN];
int active_children = 0;

// Error message texts
const char *received_sigint_msg = "\nReceived signal SIGINT.\n";

// Check whether the n-th bit is set
// 1. bit set - show PID of created process
// 2. bit set - show incoming request from child
// 3. bit set - show responds from parent
// 4. bit set - show incoming RTSIG
// 5. bit set - show return value of child
int is_nth_bit_set(int num, int n)
{
    static unsigned char bit_mask[] = {16, 8, 4, 2, 1};
    return num & bit_mask[n-1];
}

// Helper function used to signalize argument errors
void sig_arg_err()
{
    printf("Wrong argument format.\n"
           "Usage: zad2 [no_children] [no_requests] [options]\n"
           "If no options argument is provided, 32 is assumed by default.\n");
    exit(1);
}

// Handler for the the permission granted signal (SIGUSR2)
void handler_permission_granted(int signo)
{
    if(signo == SIGUSR2)
    {
        if(is_nth_bit_set(options, 3))
        {
            printf("Permission granted to child %d.\n", (int)getpid());
        }
        int rt_signal = rand()%31;
        kill(getppid(), SIGRTMIN+rt_signal);
        exit(wait_time);
    }
}

// Handler for SIGINT
void handler_sigint(int signo)
{
    if(signo == SIGINT)
    {
        write(1, received_sigint_msg, strlen(received_sigint_msg));
        system("killall -9 zad2");
    }
}

// Handler for an incoming request from a child
void handler_incoming_request(int signo, siginfo_t *info, void *payload)
{
    if(signo != SIGUSR1) return;
    received_requests[curr_no_requests++] = info->si_pid;
    total_no_requests++;

    if(is_nth_bit_set(options, 2))
    {
        printf("Received new request from child with PID: %d\n", 
               (int)info->si_pid);
    }

    if(curr_no_requests == no_requests || total_no_requests == no_children)
    {
        for(int i = 0; i < curr_no_requests; i++)
        {
            if(is_nth_bit_set(options, 3))
            {
                printf("Graning permission to child with PID: %d\n",
                   received_requests[i]);
            }
            kill(received_requests[i], SIGUSR2);
        }
        curr_no_requests = 0;
    }
}

// Handler for incoming info about a dying child
void handler_dying_child(int signo, siginfo_t *info, void *payload)
{
    if(signo == SIGCHLD)
    {
        if(is_nth_bit_set(options, 5))
        {
            printf("Child %d has died. Return value is %d\n", (int)info->si_pid,
                   (int)info->si_status);
        }
        active_children--;
    }
}

// Handler for incoming real time signal from child
void handler_incoming_rtsignal(int signo, siginfo_t *info, void *payload)
{
    if(is_nth_bit_set(options, 4))
    {
        printf("Received a new real time signal - %d from child %d\n",
               signo, (int)info->si_pid);
    }
}

// Helper function
void clean_up(void)
{
    sleep(1);
    while(active_children > 0)
    {
        sleep(1);
        active_children--;
    }
}

// MAIN function
int main(int argc, char **argv)
{
    if(argc != 3 && argc != 4) sig_arg_err();
    no_children = (int)strtol(argv[1], NULL, 10);
    no_requests = (int)strtol(argv[2], NULL, 10);
    if(argc == 3) options = 31;
    else options = (int)strtol(argv[3], NULL, 10);
    if(options < 0 || options > 31) options = 0;

    if(no_children >= MAX_NO_CHILDREN)
    {
        printf("The maximum number of children is %d.\n", MAX_NO_CHILDREN);
        exit(1);
    }
    if(no_requests > no_children)
    {
        printf("The number of requests cannot be higher than the number"
               " of children.\n");
        exit(1);
    }
    
    if(signal(SIGINT, handler_sigint) == SIG_ERR)
    {
        printf("Error while setting handler: %s\n", strerror(errno));
    }

    struct sigaction new_action1;
    new_action1.sa_flags = SA_SIGINFO;
    new_action1.sa_sigaction = handler_incoming_request;
    sigemptyset(&new_action1.sa_mask);
    if(sigaction(SIGUSR1, &new_action1, NULL) == -1)
    {
        printf("Error while setting handler for incoming request: %s\n", 
                strerror(errno));
    }

    struct sigaction new_action_tab[16];
    for(int i = 0; i < 16; i++)
    {
        new_action_tab[i].sa_flags = SA_SIGINFO;
        new_action_tab[i].sa_sigaction = handler_incoming_rtsignal;
        sigemptyset(&(new_action_tab[i].sa_mask));
        if(sigaction(SIGRTMIN + i, &(new_action_tab[i]), NULL) == -1)
        {
            printf("Error while setting handler for incoming RTSIG: %s\n", 
                strerror(errno));
        }
    }
    struct sigaction new_action_tab2[15];
    for(int i = 14; i >= 0; i--)
    {
        new_action_tab2[i].sa_flags = SA_SIGINFO;
        new_action_tab2[i].sa_sigaction = handler_incoming_rtsignal;
        sigemptyset(&(new_action_tab2[i].sa_mask));
        if(sigaction(SIGRTMAX - i, &(new_action_tab2[i]), NULL) == -1)
        {
            printf("Error while setting handler for incoming RTSIG: %s\n", 
                strerror(errno));
        }
    }

    struct sigaction new_action2;
    new_action2.sa_flags = SA_SIGINFO|SA_RESTART;
    new_action2.sa_sigaction = handler_dying_child;
    sigfillset(&new_action2.sa_mask);
    if(sigaction(SIGCHLD, &new_action2, NULL) == -1)
    {
        printf("Error while setting handler for dying child: %s\n", 
                strerror(errno));
    }

    sigset_t to_block2;
    sigfillset(&to_block2);
    sigdelset(&to_block2, SIGUSR1);
    sigset_t to_block;
    sigfillset(&to_block);
    sigdelset(&to_block, SIGUSR2);

    for(int i = 0; i < no_children; i++)
    {
        children[i] = fork();
        if(children[i] < 0)
        {
            fprintf(stderr, "Error while forking.\n");
            exit(1);
        } 
        else if(children[i] == 0)
        {
            srand(time(NULL) ^ (getpid()<<16));
            wait_time = rand()%10;
            sleep(wait_time);
            struct sigaction new_action;
            new_action.sa_handler = handler_permission_granted;
            sigemptyset(&new_action.sa_mask);
            new_action.sa_flags = 0;
            if(sigaction(SIGUSR2, &new_action, NULL) == -1)
            {
                printf("Error while setting handler in child: %s\n", 
                    strerror(errno));
            }
            kill(getppid(), SIGUSR1);
            sigsuspend(&to_block);
        } 
        else
        {
            if(is_nth_bit_set(options, 1))
            {
                printf("Created new process with PID: %d\n", (int)children[i]);
            }
            active_children++;
            sigsuspend(&to_block2);
        }
    }
    
    clean_up();
    return 0;
}