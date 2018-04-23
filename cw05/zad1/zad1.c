/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

// Maximum number of arguments a single task can have
#define MAX_ARGV_SIZE 5

// Maximum number of piped tasks in one line
#define MAX_PIPE_SIZE 5

#define READ  0
#define WRITE 1

// Global context for strtok_r
char *end_str;
char *end_token;

// Used for printing purpouses, no logical value
char execution_chain[1024];

// Describes a single task in a pipe chain
typedef struct task_data_tag {
    size_t argc;
    char* argv[MAX_ARGV_SIZE+1];
} task_data_t;

// Represents a whole sequence (singe line) of piped tasks
typedef struct piped_task_tag {
    size_t piped_len;
    task_data_t* piped_tasks[MAX_PIPE_SIZE];
} piped_task_t;

// Helper function used to signalize argument errors
void sig_arg_err()
{
    printf("Wrong argument format.\n"
           "Usage: zad1 [file]\n");
    exit(1);
}

// Helper function for the parser
int consists_only_of_whitespaces(const char *string)
{
    int i = 0;
    while(string[i] != '\0')
    {
        switch(string[i])
        {
            case 32:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
                i++;
                break;
            default:
                return 0;
        }
    }
    return 1;
}

// Provides the info about the next task which is supposed to be started
task_data_t* read_next_task(char *input_str)
{
    char *line, *token;
    line = NULL;
    task_data_t *new_task = (task_data_t*)malloc(sizeof(task_data_t));
    if(new_task == NULL)
    {
        fprintf(stderr,"Error while allocationg memory in read_next_task().\n");
    }
    new_task->argc = 0;

    line = input_str;
    token = strtok_r(line, " \n", &end_token);
    if(token == NULL)
    {
        free(line);
        return new_task;
    }
    else
    {
        new_task->argv[(new_task->argc)++] = token;
    }

    while((token=strtok_r(NULL, " \n", &end_token)) != NULL 
            && new_task->argc < MAX_ARGV_SIZE)
    {
        new_task->argv[(new_task->argc)++] = token;
    }

    int last_arg_len = strlen(new_task->argv[(new_task->argc)-1]);
    if(new_task->argv[(new_task->argc)-1][last_arg_len-1] == '\n')
        new_task->argv[(new_task->argc)-1][last_arg_len-1] = '\0';
    new_task->argv[new_task->argc] = NULL;
    return new_task;
}

// Provides a info struct about a whole pipe chain of tasks (one line)
piped_task_t* read_next_pipe(FILE *input_file)
{
    char *line, *token;
    line = NULL;
    size_t len;
    len = 0;
    ssize_t read;
    piped_task_t *new_pipe = (piped_task_t*)malloc(sizeof(task_data_t));
    if(new_pipe == NULL)
    {
        fprintf(stderr, "Error while allocating memory in read_next_pipe().\n");
    }
    new_pipe->piped_len = 0;
    task_data_t *new_task;

    read = getline(&line, &len, input_file);
    if(read == -1)
    {
        free(new_pipe);
        free(line);
        return NULL;
    }

    // Save the full line for pretty-printing purpouses...
    strcpy(execution_chain, line);

    token = strtok_r(line, "|", &end_str);
    if (!consists_only_of_whitespaces(token)) {
        new_task = read_next_task(token);
        new_pipe->piped_tasks[(new_pipe->piped_len)++] = new_task;
    }

    while((token = strtok_r(NULL, "|", &end_str)) != NULL 
            && new_pipe->piped_len < MAX_PIPE_SIZE)
    {
        new_task = read_next_task(token);
        new_pipe->piped_tasks[(new_pipe->piped_len)++] = new_task;
    }

    return new_pipe;
}

// Create a new process and sets it's std_in and std_out accordingly
int create_new_process (int input, int output, task_data_t *task)
{
    pid_t process_id;
    process_id = fork();
    if(process_id == 0)
    {
        if(input != READ)
        {
            dup2(input, READ);
            close(input);
        }

        if(output != WRITE)
        {
            dup2(output, WRITE);
            close(output);
        }

        return execvp(task->argv[0], task->argv);
    }
    else
    {
        return process_id;
    }
}

// Interprets all the tasks found in the input file
int interpret_all(const char *input_file_name)
{
    FILE *input_file;
    input_file = fopen(input_file_name, "r");
    if(input_file == NULL)
    {
        fprintf(stderr, "Error while opening the input file %s: %s\n",
                input_file_name, strerror(errno));
        return -1;
    }

    piped_task_t *new_pipe;
    int fail_occured = 0;
    
    while((new_pipe=read_next_pipe(input_file)) != NULL && !fail_occured)
    {
        if(new_pipe->piped_len == 0)
        {
            free(new_pipe);
            continue;
        }

        printf("Initializing the following exection chain: %s", 
               execution_chain);

        int in, fd[2];
        int children[MAX_PIPE_SIZE];

        in = READ;
        
        for(int i = 0; i < new_pipe->piped_len - 1; i++)
        {
            pipe(fd);
            children[i] = create_new_process(in,fd[1],new_pipe->piped_tasks[i]);
            close(fd[1]);
            in = fd[0];
        }
        if(in != READ) dup2(in, READ);

        // Handle the last child; special case
        pid_t last_child_pid;
        last_child_pid = fork();
        if(last_child_pid == 0)
        {
            task_data_t *task = new_pipe->piped_tasks[new_pipe->piped_len-1];
            execvp(task->argv[0], task->argv);
        }
        children[new_pipe->piped_len-1] = last_child_pid;
        
        // Wait for the processess to finish and print their exit codes
        for(int i = 0; i < new_pipe->piped_len; i++)
        {
            pid_t child;
            int task_result;
            child = wait(&task_result);
            char *name;
            for(int j = 0; j < new_pipe->piped_len; j++)
            {
                if(children[j] == child)
                {
                    name = new_pipe->piped_tasks[j]->argv[0];
                }
            }
            if(WIFEXITED(task_result) && WEXITSTATUS(task_result) != 0)
            {
                fprintf(stdout, "%s has failed with exit status %d\n",
                    name, WEXITSTATUS(task_result));
                fail_occured = 1;
            }
            else
            {
                printf("%s has finished successfully.\n", name);
            }
        }
        printf("\n");

        // Perform clean-up
        for(int i = 0; i < new_pipe->piped_len; i++)
        {
            free(new_pipe->piped_tasks[i]);
        }
        free(new_pipe);
    }

    if(fclose(input_file) == EOF)
    {
        fprintf(stderr, "Error while closing the input file %s: %s\n",
                input_file_name, strerror(errno));
        return -1;
    }
    return 0;
}

// MAIN function
int main(int argc, char **argv)
{
    if(argc != 2) sig_arg_err();
    const char *input_file_name;
    input_file_name = argv[1];

    interpret_all(input_file_name);

    return 0;
}