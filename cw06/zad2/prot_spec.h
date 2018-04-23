/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#ifndef _PROT_LIMITS_H
#define _PROT_LIMITS_H

// Attributes of the server queue
#define MAX_NO_MESSAGES 6
#define SERVER_QUEUE_NAME "/server_queue"
#define MSGSTR_SIZE 128
#define PERM 0640
#define PRIO 16

// Types of incoming requests
#define REQ_MIRROR 1
#define REQ_CALC 2
#define REQ_TIME 3
#define REQ_END 4
#define STOP 5
#define INIT 6
#define SERV_RESPONSE 7

// Represents the messege itself
typedef struct msg_buff_tag {
    long mtype;
    pid_t from;
    char mtext[MSGSTR_SIZE];
} msg_buff;

#endif