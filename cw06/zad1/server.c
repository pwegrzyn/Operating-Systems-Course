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
#include <time.h>
#include "prot_spec.h"

// Maximum number of simultanous clients
#define MAX_NO_CLIENTS 3

// Current clients being served by the server
pid_t clients_pids[MAX_NO_CLIENTS];
int clients_msgqids[MAX_NO_CLIENTS];

// Represents the state of the server
int server_running = 0;

// Error messages
const char *received_sigint_msg = "\nReceived an interrupt - terminating the"
    " server process...\n";

// Similar to normal errno in functionality
// 1 - wrong expression format in evaluation
// 2 - division by zero
// 3 - unknown error during evaluation
#define REQ_ERRNO_WRONG_FORMAT 1
#define REQ_ERRNO_DIV_BY_ZERO 2
#define REQ_ERRNO_UNKWN_ERR 3
int req_errno = 0;

// ID of the queue create by the server
int servq_id = 0;

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

// Helper function used for clean-up
void perform_cleanup(void)
{
    if(servq_id != 0 && msgctl(servq_id, IPC_RMID, NULL) == -1)
    {
        perror("Error while removing server queue");
    }
}

// Evaluates a given expression
float evaluate_expression(const char *exp)
{
    req_errno = 0;
    
    float num1, num2;
    char operator;

    if(sscanf(exp, "%f%c%f", &num1, &operator, &num2) != 3)
    {
        req_errno = REQ_ERRNO_WRONG_FORMAT;
        return 0;
    }
    
    switch(operator)
    {
        case '+':
            return num1 + num2;
        case '-':
            return num1 - num2;
        case '*':
            return num1 * num2;
        case '/':
            if(num2 == 0)
            {
                req_errno = REQ_ERRNO_DIV_BY_ZERO;
                return 0;
            }
            else
            {
                return num1 / num2;
            }
        default:
            req_errno = REQ_ERRNO_UNKWN_ERR;
            return 0;
    }
}

// Returns the current time and day
void get_timestamp(char *buffer)
{
    time_t raw_time;
    struct tm *time_value;
    
    time(&raw_time);
    time_value = localtime(&raw_time);
    sprintf(buffer, "%s", asctime(time_value));
    if(buffer[strlen(buffer) - 1] == '\n')
    {
        buffer[strlen(buffer) - 1] = '\0';
    }
}

// Reverses a string and saves the output to the buffer
void reverse_string(char *string, char *buffer)
{
    int len = strlen(string);
    for(int i = 0; i < len; i++)
    {
        buffer[i] = string[len - i - 1];
    }
}

// SIGINT handler
void handler_sigint(int signo)
{
    if(signo == SIGINT)
    { 
        write(1, received_sigint_msg, strlen(received_sigint_msg));
        
        // Inform all clients that the server is being taken offlien
        for(int i = 0; i < MAX_NO_CLIENTS; i++)
        {
            if(clients_pids[i] != -1)
            {
                kill(clients_pids[i], SIGUSR1);
            }
        }
        exit(EXIT_FAILURE);
    }
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
void receive_message(int msgqid, long mtype, msg_buff *buffer, int flags)
{
    if(msgrcv(msgqid, buffer, sizeof(msg_data), mtype, flags) == -1)
    {   
        if(errno != ENOMSG)
        {
            perror("Error while receiving message from queue");
        }
    }
}

// Fetchets a new unique identifier for a new client, return -1 if
// the max number of concurrent clients being served is reached 
int get_client_id()
{
    for(int i = 0; i < MAX_NO_CLIENTS; i++)
    {
        if(clients_pids[i] == -1)
        {
            return i;
        }
    }
    return -1;
}

// Returns the unique client id associated to a given pid
int get_clientid_by_pid(pid_t pid)
{
    for(int i = 0; i < MAX_NO_CLIENTS; i++)
    {
        if(clients_pids[i] == pid)
        {
            return i;
        }
    }
    return -1;
}

// Initializes a new client connection
void initialize_new_connection(msg_buff *buffer)
{
    int clientq_key, client_new_id, clientq_id;
    pid_t client_pid;

    clientq_key = (pid_t)strtol((buffer->data).mtext, NULL, 10);
    client_pid = (pid_t)(buffer->data).from;

    client_new_id = get_client_id();
    clientq_id = msgget(clientq_key, 0);
    if(client_new_id == -1)
    {
        send_message(clientq_id, SERV_RESPONSE, "-1");
        return;
    }

    printf("Establishing new connection with client: %d\n",
        (int)(buffer->data).from);
    
    clients_pids[client_new_id] = client_pid;
    clients_msgqids[client_new_id] = clientq_id;

    char response_text[128] = {0};
    sprintf(response_text, "%d", client_new_id);
    send_message(clientq_id, SERV_RESPONSE, response_text);
}

// Closes a connection to a client
void close_connection(msg_buff *buffer)
{
    int client_id;
    client_id = get_clientid_by_pid((buffer->data).from);
    
    if (client_id != -1) {
        printf("Closing connection with client: %d\n", 
                    (int)buffer->data.from);
        clients_pids[client_id] = -1;
        clients_msgqids[client_id] = -1;
    }
}

// Evaluates a given expression and sends a response to the client
void calculate_rsp(msg_buff *buffer)
{
    int client_id, clientq_id;
    float result;
    client_id = get_clientid_by_pid((buffer->data).from);
    clientq_id = clients_msgqids[client_id];
    char response_text[128] = {0};
    result = evaluate_expression((buffer->data).mtext);
    switch(req_errno)
    {
        case 0:
            sprintf(response_text, "%f", result);
            send_message(clientq_id, SERV_RESPONSE, response_text);
            break;
        case REQ_ERRNO_DIV_BY_ZERO:
            sprintf(response_text, "Division by zero!");
            send_message(clientq_id, SERV_RESPONSE, response_text);
            req_errno = 0;
            break;
        case REQ_ERRNO_WRONG_FORMAT:
            sprintf(response_text, "Wrong expression format!");
            send_message(clientq_id, SERV_RESPONSE, response_text);
            req_errno = 0;
            break;
        case REQ_ERRNO_UNKWN_ERR:
            sprintf(response_text, "An unknown error occured!");
            send_message(clientq_id, SERV_RESPONSE, response_text);
            req_errno = 0;
            break;
        default:
            req_errno = 0;
            break;
    }
}

// Reverses a given string and sends a response to the client
void reverse_str_rsp(msg_buff *buffer)
{
    int client_id, clientq_id;
    client_id = get_clientid_by_pid((buffer->data).from);
    clientq_id = clients_msgqids[client_id];
    char response_text[128] = {0};
    reverse_string((buffer->data).mtext, response_text);
    send_message(clientq_id, SERV_RESPONSE, response_text);
    return;
}

// Fetchets the current timestamp and sends a response to the client
void get_timestamp_rsp(msg_buff *buffer)
{
    int client_id, clientq_id;
    client_id = get_clientid_by_pid((buffer->data).from);
    clientq_id = clients_msgqids[client_id];
    char response_text[128] = {0};
    get_timestamp(response_text);
    send_message(clientq_id, SERV_RESPONSE, response_text);
    return;
}

// MAIN function
int main(void)
{
    key_t servq_key;
    const char* home_path;
    home_path = getenv("HOME");

    printf("Server is initializing...\n");
    
    // Generate a queue for this server process
    if((servq_key=ftok(home_path, server_queue_seed)) == -1)
    {
        perror("Cannot create server queue key");
        exit(EXIT_FAILURE);
    }
    servq_id = create_queue(servq_key);

    // Register a cleanup function
    atexit(perform_cleanup);

    if(signal(SIGINT, handler_sigint) == SIG_ERR)
    {
        perror("Error while setting the SIGINT handler");
        exit(EXIT_FAILURE);
    }

    // Initialize used data structures
    for(int i = 0; i < MAX_NO_CLIENTS; i++)
    {
        clients_pids[i] = -1;
        clients_msgqids[i] = -1;
    }

    int received_end_req = 0;
    server_running = 1;

    printf("Server is ready. Listening for clients...\n");
    
    // Main loop of the server
    while(server_running)
    {
        msg_buff buffer;
        if(received_end_req)
        {
            // IF rcvmsg fails with IPC_NOWAIT and errno is ENOMSG
            // the it means that the queue is empty
            receive_message(servq_id, 0, &buffer, IPC_NOWAIT);
            if(errno == ENOMSG)
            {
                //Inform all the clients
                for(int i = 0; i < MAX_NO_CLIENTS; i++)
                {
                    if(clients_pids[i] != -1)
                    {
                        kill(clients_pids[i], SIGUSR1);
                    }
                }
                break;
            }
        }
        else
        {
            receive_message(servq_id, 0, &buffer, 0);
        }
        switch(buffer.mtype)
        {
            case INIT:
                initialize_new_connection(&buffer);
                break;
            case STOP:
                close_connection(&buffer);
                break;
            case REQ_CALC:
                printf("Performing a calculation\n");
                calculate_rsp(&buffer);
                break;
            case REQ_MIRROR:
                printf("Reversing a string\n");
                reverse_str_rsp(&buffer);
                break;
            case REQ_TIME:
                printf("Fetching the current timestamp\n");
                get_timestamp_rsp(&buffer);
                break;
            case REQ_END:
                printf("Received a command to take the server offline\n");
                received_end_req = 1;
            default:
                break;

        }
    }

    printf("The server is going offline\n");
    
    return EXIT_SUCCESS;
}