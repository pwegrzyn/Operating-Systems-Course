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

// Describes a new task found in the input file
typedef struct task_data_tag {
    size_t argc;
    char* argv[MAX_ARGV_SIZE+1];
} task_data_t;

// Helper function used to signalize argument errors
void sig_arg_err()
{
    printf("Wrong argument format.\n"
           "Usage: program [file] [max cpu time in seconds] [max virtual " 
           "memory in MBs]\n");
    exit(1);
}

// Used for time measurements
long clctd(struct timeval start, struct timeval end)
{
    return (long)((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
}

// Returns a new struct which describes a new task found in the input file;
// The users has to free the returned object after its no longer needed
task_data_t* read_next_task(FILE *input_file)
{
    char *line, *token;
    line = NULL;
    size_t len;
    len = 0;
    ssize_t read;
    task_data_t *new_task = (task_data_t*)malloc(sizeof(task_data_t));
    new_task->argc = 0;

    read = getline(&line, &len, input_file);
    if(read == -1)
    {
        free(new_task);
        free(line);
        return NULL;
    }

    token = strtok(line, " \n");
    if(token == NULL)
    {
        free(line);
        return new_task;
    }
    else
    {
        new_task->argv[(new_task->argc)++] = token;
    }

    while((token=strtok(NULL, " \n")) != NULL && new_task->argc < MAX_ARGV_SIZE)
    {
        new_task->argv[(new_task->argc)++] = token;
    }

    int last_arg_len = strlen(new_task->argv[(new_task->argc)-1]);
    if(new_task->argv[(new_task->argc)-1][last_arg_len-1] == '\n')
        new_task->argv[(new_task->argc)-1][last_arg_len-1] = '\0';
    new_task->argv[new_task->argc] = NULL;
    return new_task;
}

// Interprets all the tasks found in the input file
int interpret_all(const char *input_file_name, unsigned long max_cpu_time, 
                  unsigned long max_vm_size)
{
    FILE *input_file;
    input_file = fopen(input_file_name, "r");
    if(input_file == NULL)
    {
        fprintf(stderr, "Error while opening the input file %s: %s\n",
                input_file_name, strerror(errno));
        return -1;
    }

    int task_result;
    task_result = 0;
    task_data_t *new_task;
    int child_pid;
    const char *task_name;
    struct rlimit vm_limit;
    struct rlimit cpu_limit;

    vm_limit.rlim_max = 1000000 * max_vm_size;
    vm_limit.rlim_cur = vm_limit.rlim_max / 2;
    cpu_limit.rlim_max = max_cpu_time;
    cpu_limit.rlim_cur = cpu_limit.rlim_max / 2;

    struct timeval start_s, end_s;
    struct timeval start_u, end_u;
    struct rusage usage;
    
    while((new_task=read_next_task(input_file)) != NULL)
    {
        if(new_task->argc == 0)
        {
            free(new_task);
            continue;
        }
        task_name = new_task->argv[0];
        
        getrusage(RUSAGE_CHILDREN, &usage);
        start_s = usage.ru_stime;
        start_u = usage.ru_utime;
        
        child_pid = fork();
        if(child_pid == 0)
        {
            printf("Starting the task: %s...\n", task_name);
            if(setrlimit(RLIMIT_AS, &vm_limit) == -1)
            {
                fprintf(stderr, "Error while setting the resource limit: %s\n",
                        strerror(errno));
            }
            if(setrlimit(RLIMIT_CPU, &cpu_limit) == -1)
            {
                fprintf(stderr, "Error while setting the resource limit: %s\n",
                        strerror(errno));
            }
            if(execvp(task_name, new_task->argv) == -1)
            {
                fprintf(stderr, "Error encountered while trying to execute the " 
                        "task %s: %s\n", task_name, strerror(errno));
                fclose(input_file);
                exit(1);
            }
        }
        else if(child_pid > 0)
        {
            waitpid(child_pid, &task_result, 0);

            getrusage(RUSAGE_CHILDREN, &usage);
            end_s = usage.ru_stime;
            end_u = usage.ru_utime;

            if(WIFEXITED(task_result) && WEXITSTATUS(task_result) != 0)
            {
                fprintf(stdout, "%s has failed with exit status %d\n\n",
                        task_name, WEXITSTATUS(task_result));
                fclose(input_file);
                free(new_task);
                return -1;
            }
            else if(WIFSIGNALED(task_result))
            {
                printf("%s was killed by a signal. The signal was: %d\n\n", 
                       task_name, WTERMSIG(task_result));
                fclose(input_file);
                free(new_task);
                return -1;
            }
            else
            {
                printf("%s has finished successfully. Execution time was:\n" 
                       "USER: %ld microseconds,\n"
                       "SYS: %ld microseconds.\n\n", task_name,
                       clctd(start_u, end_u), clctd(start_s, end_s));
                free(new_task);
            }
        }
        else
        {
            fprintf(stderr, "Error encountered while trying to fork: %s\n",
                    strerror(errno));
            fclose(input_file);
            free(new_task);
            return -1;
        }
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
    if(argc != 4) sig_arg_err();
    const char *input_file_name;
    unsigned long max_cpu_time, max_vm_size;
    input_file_name = argv[1];
    max_cpu_time = (unsigned long)strtol(argv[2], NULL, 10);
    max_vm_size = (unsigned long)strtol(argv[3], NULL, 10);
    
    interpret_all(input_file_name, max_cpu_time, max_vm_size);

    return 0;
}