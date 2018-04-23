/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

// Maximum real time signal queue time
#define MAX_NO_SIGNALS 999

int received_by_child = 0;
int sent_by_parent = 0;
int received_by_parent = 0;

// Helper function used to signalize argument errors
void sig_arg_err()
{
    printf("Wrong argument format.\n"
           "Usage: zad3 [no_signals] [type]\n");
    exit(1);
}

// Handler for SIGUSR1 for the child
void handler_sigusr1_child(int signo, siginfo_t *info, void *payload)
{
    union sigval value;
    if(signo == SIGUSR1 || signo == SIGRTMIN)
    {
        received_by_child++;
        sigqueue(getppid(), SIGUSR1, value);
    }
}

// Handler for SIGUSR2 for the child
void handler_sigusr2_child(int signo, siginfo_t *info, void *payload)
{
    if(signo == SIGUSR2 || signo == SIGRTMAX)
    {
        printf("Received by child: %d.\n", received_by_child);
        exit(1);
    }
}

// Handler for SIGUSR1 for parent
void handler_sigusr1_parent(int signo, siginfo_t *info, void *payload)
{
    if(signo == SIGUSR1)
    {
        received_by_parent++;
    }
}

// Handler for the alarm signal
void handler_alarm(int signo)
{
    signal(SIGALRM, handler_alarm);
    return;
}

// Handler for SIGINT
void handler_sigint(int signo)
{
    if(signo == SIGINT)
    {
        printf("\nReceived SIGINT.\n");
        system("killall -9 zad3");
    }
}

// MAIN function
int main(int argc, char **argv)
{
    if(argc != 3) sig_arg_err();
    int no_signals, type;
    no_signals = (int)strtol(argv[1], NULL, 10);
    if(no_signals > MAX_NO_SIGNALS)
    {
        printf("Maxium number of signals supported is: %d.\n", MAX_NO_SIGNALS);
        exit(1);
    }
    type = (int)strtol(argv[2], NULL, 10);

    int child_pid;
    child_pid = fork();
    if(child_pid < 0)
    {
        fprintf(stderr, "Error while forking.\n");
        exit(1);
    }
    else if(child_pid == 0)
    {        
        struct sigaction new_action;
        new_action.sa_flags = SA_SIGINFO;
        new_action.sa_sigaction = handler_sigusr1_child;
        sigfillset(&new_action.sa_mask);
        sigdelset(&new_action.sa_mask, SIGUSR2);
        if(sigaction(SIGUSR1, &new_action, NULL) == -1)
        {
            printf("Error while setting handler for child - SIGUSR1: %s\n", 
                strerror(errno));
        }
        if(type == 3){
        struct sigaction new_action5;
        new_action5.sa_flags = SA_SIGINFO;
        new_action5.sa_sigaction = handler_sigusr1_child;
        sigfillset(&new_action5.sa_mask);
        sigdelset(&new_action5.sa_mask, SIGUSR2);
        sigdelset(&new_action5.sa_mask, SIGRTMAX);
        sigaction(SIGRTMIN, &new_action5, NULL);
        }

        struct sigaction new_action2;
        new_action2.sa_flags = SA_SIGINFO;
        new_action2.sa_sigaction = handler_sigusr2_child;
        sigfillset(&new_action2.sa_mask);
        if(sigaction(SIGUSR2, &new_action2, NULL) == -1)
        {
            printf("Error while setting handler for child - SIGUSR2: %s\n", 
                strerror(errno));
        }
        if(type == 3){
        struct sigaction new_action6;
        new_action6.sa_flags = SA_SIGINFO;
        new_action6.sa_sigaction = handler_sigusr2_child;
        sigfillset(&new_action6.sa_mask);
        sigaction(SIGRTMAX, &new_action6, NULL);
        }
        
        sigset_t to_block;
        sigfillset(&to_block);
        if(type == 3)
        {
            sigdelset(&to_block, SIGRTMIN);
            sigdelset(&to_block, SIGRTMAX);
        }
        else
        {
            sigdelset(&to_block, SIGUSR1);
            sigdelset(&to_block, SIGUSR2);
        }
        
        while(1)
        {
            sigsuspend(&to_block);
        }
    }
    else
    {
        if(signal(SIGINT, handler_sigint) == SIG_ERR)
        {
            printf("Error while setting handler for parent - sigint.\n");
        }
        
        struct sigaction new_action3;
        new_action3.sa_flags = SA_SIGINFO;
        new_action3.sa_sigaction = handler_sigusr1_parent;
        sigfillset(&new_action3.sa_mask);
        if(sigaction(SIGUSR1, &new_action3, NULL) == -1)
        {
            printf("Error while setting handler for parent - SIGUSR1: %s\n", 
                strerror(errno));
        }

        if(signal(SIGALRM, handler_alarm) == SIG_ERR)
        {
            printf("Error while setting handler for parent - alarm.\n");
        }

        sigset_t to_block;
        sigfillset(&to_block);
        sigdelset(&to_block, SIGUSR1);
        sigdelset(&to_block, SIGALRM);

        sleep(1);

        //union sigval value;
        
        for(int i = 0; i < no_signals; i++)
        {
            sent_by_parent++;
            //sigqueue(child_pid, SIGUSR1, value);
            if(type == 3)
            {
                kill(child_pid, SIGRTMIN);
            } 
            else
            {
                kill(child_pid, SIGUSR1);
                if(type == 2)
                {
                    sigsuspend(&to_block);
                    alarm(3);
                }
            }
        }
        printf("Sent by parent: %d.\n", sent_by_parent);
        if(type == 3)
        {
            kill(child_pid, SIGRTMAX);
        }
        else
        {
            kill(child_pid, SIGUSR2);
        }
        sleep(1);
        printf("Received by parent: %d.\n", received_by_parent);
    }
    return 0;
}