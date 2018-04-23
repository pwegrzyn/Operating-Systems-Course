/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#define _XOPEN_SOURCE

#include <sys/types.h>
#include <mqueue.h>
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
mqd_t clients_msgqfds[MAX_NO_CLIENTS];

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
  
// Descriptor of the queue create by the server
int servq_fd = 0;

// Creates (if necessary) a new queue
mqd_t create_queue(const char *name)
{
    int queue_fd;
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_NO_MESSAGES;
    attr.mq_msgsize = sizeof(msg_buff);
    if((queue_fd = mq_open(name, O_CREAT|O_RDWR, PERM, &attr)) == -1)
    {
        perror("Error encountered while creating queue");
        exit(EXIT_FAILURE);
    }
    return queue_fd;
}

// Helper function used for clean-up
void perform_cleanup(void)
{
    if(servq_fd != 0)
    {
        if(mq_close(servq_fd) == -1)
        {
            perror("Error while closing server queue");
        }
        if(mq_unlink(SERVER_QUEUE_NAME) == -1)
        {
            perror("Error while removing server queue");
        }
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
        // and close all clients queues so they can remove them
        for(int i = 0; i < MAX_NO_CLIENTS; i++)
        {
            if(clients_msgqfds[i] != -1)
            {
                mq_close(clients_msgqfds[i]);
            }
            
            if(clients_pids[i] != -1)
            {
                kill(clients_pids[i], SIGUSR1);
            }
        }
        exit(EXIT_FAILURE);
    }
}

// Sends a message to a queue
void send_message(mqd_t msgqfd, long type, const char *msg_text)
{
    msg_buff msg;
    msg.mtype = type;
    if(msg_text == NULL)
    {
        strcpy(msg.mtext, "");
    }
    else
    {
        strcpy(msg.mtext, msg_text);
    }
    msg.from = (int)getpid();

    if(mq_send(msgqfd, (const char*)&msg, sizeof(msg_buff), PRIO) == -1)
    {
        perror("Error while sending message to queue");
    }
    
    return;
}

// Receives a message from a queue
void receive_message(mqd_t msgqfd, long mtype, msg_buff *buffer)
{
    if(mq_receive(msgqfd, (char*)buffer, sizeof(msg_buff), NULL) == -1)
    {   
        perror("Error while receiving message from queue");
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
    
    char clientq_name[128];
    int client_new_id;
    mqd_t clientq_fd;
    pid_t client_pid;

    strcpy(clientq_name, buffer->mtext);
    client_pid = (pid_t)buffer->from;

    client_new_id = get_client_id();
    clientq_fd = mq_open(clientq_name, O_WRONLY);
    if(client_new_id == -1)
    {
        send_message(clientq_fd, SERV_RESPONSE, "-1");
        return;
    }

    printf("Establishing new connection with client: %d\n",
        (int)(buffer->from));
    
    clients_pids[client_new_id] = client_pid;
    clients_msgqfds[client_new_id] = clientq_fd;

    char response_text[128] = {0};
    sprintf(response_text, "%d", client_new_id);
    send_message(clientq_fd, SERV_RESPONSE, response_text);
}

// Closes a connection to a client
void close_connection(msg_buff *buffer)
{
    int client_id;
    client_id = get_clientid_by_pid(buffer->from);
    
    if (client_id != -1) {
        printf("Closing connection with client: %d\n", (int)(buffer->from));

        mq_close(clients_msgqfds[client_id]);
    
        clients_pids[client_id] = -1;
        clients_msgqfds[client_id] = -1;
    }
}

// Evaluates a given expression and sends a response to the client
void calculate_rsp(msg_buff *buffer)
{
    int client_id;
    mqd_t clientq_fd;
    float result;
    client_id = get_clientid_by_pid(buffer->from);
    clientq_fd = clients_msgqfds[client_id];
    char response_text[128] = {0};
    result = evaluate_expression(buffer->mtext);
    switch(req_errno)
    {
        case 0:
            sprintf(response_text, "%f", result);
            send_message(clientq_fd, SERV_RESPONSE, response_text);
            break;
        case REQ_ERRNO_DIV_BY_ZERO:
            sprintf(response_text, "Division by zero!");
            send_message(clientq_fd, SERV_RESPONSE, response_text);
            req_errno = 0;
            break;
        case REQ_ERRNO_WRONG_FORMAT:
            sprintf(response_text, "Wrong expression format!");
            send_message(clientq_fd, SERV_RESPONSE, response_text);
            req_errno = 0;
            break;
        case REQ_ERRNO_UNKWN_ERR:
            sprintf(response_text, "An unknown error occured!");
            send_message(clientq_fd, SERV_RESPONSE, response_text);
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
    int client_id;
    mqd_t clientq_fd;
    client_id = get_clientid_by_pid(buffer->from);
    clientq_fd = clients_msgqfds[client_id];
    char response_text[128] = {0};
    reverse_string(buffer->mtext, response_text);
    send_message(clientq_fd, SERV_RESPONSE, response_text);
    return;
}

// Fetchets the current timestamp and sends a response to the client
void get_timestamp_rsp(msg_buff *buffer)
{
    int client_id;
    mqd_t clientq_fd;
    client_id = get_clientid_by_pid(buffer->from);
    clientq_fd = clients_msgqfds[client_id];
    char response_text[128] = {0};
    get_timestamp(response_text);
    send_message(clientq_fd, SERV_RESPONSE, response_text);
    return;
}

// MAIN function
int main(void)
{
    printf("Server is initializing...\n");
    
    // Generate a queue for this server process
    servq_fd = create_queue(SERVER_QUEUE_NAME);

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
        clients_msgqfds[i] = -1;
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
            // Check whether or not the queue is empty
            struct mq_attr attr;
            mq_getattr(servq_fd, &attr);
            if(attr.mq_curmsgs == 0)
            {
                //Inform all the clients
                for(int i = 0; i < MAX_NO_CLIENTS; i++)
                {
                    if(clients_msgqfds[i] != -1)
                    {
                        mq_close(clients_msgqfds[i]);
                    }
                    if(clients_pids[i] != -1)
                    {
                        kill(clients_pids[i], SIGUSR1);
                    }
                }
                // Wait for all clients to close the server queue
                sleep(2);
                break;
            }
        }
        else
        {
            receive_message(servq_fd, 0, &buffer);
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
    
    exit(EXIT_SUCCESS);
}