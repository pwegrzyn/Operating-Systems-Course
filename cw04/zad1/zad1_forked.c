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
int received_sigint = 0;
int child_working = 1;

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
        received_sigint = 1;
    }
}

// SIGTSTP handler
void handler_sigtstp(int signo)
{
    if(signo != SIGTSTP) return;

    if(child_working)
    {
        write(1, received_sigtstp_msg, strlen(received_sigtstp_msg));
        child_working = 0;
    }
    else
    {
        child_working = 1;
    }
    return;
}

// MAIN function
int main(void)
{
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

    int child_pid;
    child_pid = fork();
    if(child_pid == 0)
    {
        char* const av[]={NULL};
        if(execvp("./print_date.sh", av) == -1)
        {
            fprintf(stderr, "Error encountered while trying to "
                    "execute the task: %s\n", strerror(errno));
            exit(1);
        }
    }

    while(child_pid > 0)
    {
        pause();
        if(received_sigint)
        {
            if(child_working) kill(child_pid, SIGTERM);
            exit(1);
        }
        else
        {
            if(child_working)
            {
                child_pid = fork();
                if(child_pid == 0)
                {
                    char* const av[]={NULL};
                    if(execvp("./print_date.sh", av) == -1)
                    {
                        fprintf(stderr, "Error encountered while trying to "
                                "execute the task: %s\n", strerror(errno));
                        exit(1);
                    }
                }
                else if(child_pid > 0)
                {
                    continue;
                }
                else
                {
                    fprintf(stderr, "Error encountered while trying to "
                            "fork: %s\n", strerror(errno));
                    exit(1);
                }
            }
            else
            {
                kill(child_pid, SIGTERM);
            }
        }
    }

    return 0;
}