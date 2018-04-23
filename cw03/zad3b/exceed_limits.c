/* Systemy Operacyjne 2018 Patryk Wegrzyn */

#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 200000000

int main(void)
{
    char *huge_buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
    if(huge_buffer == NULL)
    {
        printf("Malloc has faild, which means the resouce limit has been " 
               "exceeded.\n");
        exit(1);
    }
    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        huge_buffer[i] = 'a';
    }
    free(huge_buffer);

    unsigned long counter = 0;
    while(1)
    {
        counter++;
        counter--;
    }
    
    return 0;
}