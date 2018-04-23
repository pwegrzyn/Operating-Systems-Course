/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#define _XOPEN_SOURCE

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Signal flags
int sleeping = 0;

// Error message texts
const char *received_sigint_msg = "\nOdebrano sygnał SIGINT.\n";
const char *received_sigtstp_msg = "\nOczekuję na CTRL+Z - kontynuacja albo "
                                   "CTR+C - zakonczenie programu.\n";

// SIGINT handler
void handler_sigint(int signo)
{
    if(signo == SIGINT)
    {
        write(1, received_sigint_msg, strlen(received_sigint_msg));
        exit(1);
    }
}

// SIGTSTP handler
void handler_sigtstp(int signo)
{
    if(signo != SIGTSTP) return;

    if(!sleeping)
    {
        write(1, received_sigtstp_msg, strlen(received_sigtstp_msg));
        sleeping = 1;
    }
    else
    {
        sleeping = 0;
    }
    return;
}

// MAIN function
int main(void)
{
    time_t raw_time;
    struct tm *time_value;

    if(signal(SIGINT, handler_sigint) == SIG_ERR)
    {
        fprintf(stderr, "Error while setting handler using signal():\n%s", 
                strerror(errno));
        exit(1);
    }

    struct sigaction new_action;
    new_action.sa_handler = handler_sigtstp;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    if(sigaction(SIGTSTP, &new_action, NULL))
    {
        fprintf(stderr, "Error while setting hanlder using sigaction():\n%s",
                strerror(errno));
        exit(1);
    }

    while(1)
    {
        if(sleeping) pause();
        time(&raw_time);
        time_value = localtime(&raw_time);
        printf("\n%s", asctime(time_value));
        sleep(1);
    }

    return 0;
}