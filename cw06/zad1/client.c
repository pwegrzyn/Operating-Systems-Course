/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#define _XOPEN_SOURCE

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include "prot_spec.h"

// Error message texts
const char *received_sigint_msg = "\nReceived an interrupt - terminating the"
    " client process...\n";
const char *received_sigusr1_msg = "\nThe server is being"
    " taken offline - closing connection...\n";

// Name of the file represeting a instance of this program (process)
char proc_iden[512];

// ID of the queue created by the client
int clientq_id = 0;

// Represents the state of the connection with the server
int connection_established = 0;

// ID of the server
int servq_id;

// Creates (if necessary) a new queue
int create_queue(key_t key)
{
    int queue_id;
    if((queue_id = msgget(key, PERM | IPC_CREAT)) == -1)
    {
        perror("Error encountered while creating queue");
        exit(EXIT_FAILURE);
    }
    return queue_id;
}

// Sends a message to a queue
void send_message(int msgqid, long type, const char *msg_text)
{
    msg_buff msg;
    msg.mtype = type;
    if(msg_text == NULL)
    {
        strcpy(msg.data.mtext, "");
    }
    else
    {
        strcpy(msg.data.mtext, msg_text);
    }
    msg.data.from = (int)getpid();

    if(msgsnd(msgqid, (void*)&msg, sizeof(msg_data), 0) == -1)
    {
        perror("Error while sending message to queue");
    }
    
    return;
}

// Receives a message from a queue
void receive_message(int msgqid, long mtype, msg_buff *buffer)
{
    if(msgrcv(msgqid, buffer, sizeof(msg_data), mtype, 0) == -1)
    {   
        perror("Error while receiving message from queue");
    }
}

// Helper function used for clean-up
void perform_cleanup(void)
{
    remove(proc_iden);
    if(clientq_id != 0 && msgctl(clientq_id, IPC_RMID, NULL) == -1)
    {
        perror("Error while removing client queue");
    }
}

// Helper function used to compare strings igonring case
int strcasecmp(char const *fst, char const *snd)
{
    int difference;
    for(;; fst++, snd++)
    {
        difference = tolower(*fst) - tolower(*snd);
        if(difference != 0 || !*fst)
        {
            return difference;
        }
    }
}

// SIGINT handler
void handler_sigint(int signo)
{
    if(signo == SIGINT)
    {
        write(1, received_sigint_msg, strlen(received_sigint_msg));
        send_message(servq_id, STOP, NULL);
        exit(EXIT_FAILURE);
    }
}

// SIGUSR1 handler (for server being offline)
void handler_sigusr1(int signo)
{
    if(signo == SIGUSR1)
    {
        write(1, received_sigusr1_msg, strlen(received_sigusr1_msg));
        exit(EXIT_FAILURE);
    }
}

// MAIN function
int main(void)
{
    key_t clientq_key, servq_key;
    int proc_iden_fd;
    char msg[128], user_input[MSGSTR_SIZE];
    const char *home_path;
    home_path = getenv("HOME");
    sprintf(proc_iden, "%s/client_%d", home_path, (int)getpid());
    
    // Generate a tmp file representing this process (for ftok)
    if((proc_iden_fd=open(proc_iden, O_CREAT, PERM)) == -1)
    {
        perror("Error while creating temporary process identifying file");
        exit(EXIT_FAILURE);
    }

    // Register a clean-up function
    atexit(perform_cleanup);

    // Generate a queue for this client process
    if((clientq_key=ftok(proc_iden, client_queue_seed)) == -1)
    {
        perror("Cannot create client queue key");
        exit(EXIT_FAILURE);
    }
    clientq_id = create_queue(clientq_key);

    // Open the server's queue
    if((servq_key=ftok(home_path, server_queue_seed)) == -1)
    {
        perror("Cannot create server queue key");
        exit(EXIT_FAILURE);
    }
    if((servq_id=msgget(servq_key, 0)) == -1)
    {
        perror("Cannot open server queue");
        exit(EXIT_FAILURE);
    }

    // Send the INIT message with the key of the created client queue
    sprintf(msg, "%d", clientq_key);
    send_message(servq_id, INIT, msg);

    if(signal(SIGINT, handler_sigint) == SIG_ERR)
    {
        perror("Error while setting the SIGINT handler");
        exit(EXIT_FAILURE);
    }
    if(signal(SIGUSR1, handler_sigusr1) == SIG_ERR)
    {
        perror("Error while setting the SIGUSR1 handler");
        exit(EXIT_FAILURE);
    }

    // ID of the client coming from the server
    int client_id;
    msg_buff buffer1;
    
    // Receive the initial reponse from the server
    // If it retures -1 the the server has reached its limit of concurrent users
    // In that case try again in a while
    receive_message(clientq_id, SERV_RESPONSE, &buffer1);
    while((client_id = (int)strtol(buffer1.data.mtext, NULL, 10)) == -1)
    {
        printf("The server is busy right now...\n");
        printf("Trying to establish a connection again in 5 seconds\n");
        sleep(5);
        send_message(servq_id, INIT, msg);
        receive_message(clientq_id, SERV_RESPONSE, &buffer1);
    }
    printf("ID received from the server: %d\n", client_id);
    connection_established = 1;

    while(connection_established)
    {
        char *token;
        printf(">> ");
        fgets(user_input, MSGSTR_SIZE, stdin);
        if(user_input[strlen(user_input) - 1] == '\n')
        {
            user_input[strlen(user_input) - 1] = '\0';
        }

        token = strtok(user_input, " ");
        if(token == NULL)
        {
            continue;
        }
        
        long mode;
        if(strcasecmp(token, "MIRROR") == 0)
            mode = REQ_MIRROR;
        else if(strcasecmp(token, "CALC") == 0)
            mode = REQ_CALC;
        else if(strcasecmp(token, "TIME") == 0)
            mode = REQ_TIME;
        else if(strcasecmp(token, "END") == 0)
            mode = REQ_END;
        else if(strcasecmp(token, "HELP") == 0)
            mode = -1;
        else
        {
            printf("Invalid command - type HELP for more info\n");
            continue;
        }

        switch(mode)
        {
            case REQ_MIRROR:
                token = strtok(NULL, " ");
                if(token == NULL)
                {
                    printf("Lacking argument\n");
                    continue;
                }
                printf("Sending a request to reverse the string %s...\n",token);
                break;

            case REQ_CALC:
                token = strtok(NULL, " ");
                if(token == NULL)
                {
                    printf("Lacking expression to calculate\n");
                    continue;
                }
                printf("Sending a request to calculate the expression %s...\n",
                    token);
                break;

            case REQ_TIME:
                token = strtok(NULL, " ");
                if(token == NULL)
                {
                    printf("Sending a request to get the current timestamp...\n");
                }
                else
                {
                    printf("Wrong format\n");
                    continue;
                }
                break;

            case REQ_END:
                token = strtok(NULL, " ");
                if(token == NULL)
                {
                    printf("Informing the server of the termination of the "
                           "connection...\n");
                    connection_established = 0;
                }
                else
                {
                    printf("Wrong format\n");
                    continue;
                }
                break;

            case -1:
                printf("CALC   - calculate a given expression\n"
                       "MIRROR - reverse a string\n"
                       "TIME   - get the current timestamp\n"
                       "END    - terminate the connection with the server\n"
                       "HELP   - print this info\n");
                continue;
                break;
        }

        // Actutally sending the request to the server
        send_message(servq_id, mode, token);

        // Do not wait for a response if server is to be closed
        if(mode == REQ_END)
        {
            break;
        }

        // Receive a response from the server
        msg_buff buffer2;
        receive_message(clientq_id, SERV_RESPONSE, &buffer2);
        
        printf("Response from the server: %s\n", buffer2.data.mtext);
    }

    exit(EXIT_SUCCESS);
}