/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#ifndef _PROT_LIMITS_H
#define _PROT_LIMITS_H

#define MSGSTR_SIZE 1024
#define PERM 0640

// Types of incoming requests
#define REQ_MIRROR 1
#define REQ_CALC 2
#define REQ_TIME 3
#define REQ_END 4
#define STOP 5
#define INIT 6
#define SERV_RESPONSE 7

// Used as seeds for generating keys
char server_queue_seed = 16;
char client_queue_seed = 32;

// Represents the data in a message
typedef struct msg_data_tag {
    pid_t from;
    char mtext[MSGSTR_SIZE];
} msg_data;

// Represents the messege itself
typedef struct msg_buff_tag {
    long mtype;
    msg_data data;
} msg_buff;

#endif