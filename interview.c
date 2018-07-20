#include <stdio.h>
#include <stdlib.h>
size_t getline(char **lineptr, size_t *n, FILE *stream);

int main() {
    size_t maxLineLen = 1024;
    char *line = (char*)malloc(maxLineLen);
    while(getline(&line, &maxLineLen, stdin) != -1) {
        printf("%s", line);
    }
}


size_t getline(char **lineptr, size_t *n, FILE *stream)
{
    
}


/*
1)Calculate the integers and square of input
2)Calculate values from N p q and replace it with desired output
3)From input of 2014-02-07,3,monkey put them into
different categories according to the date,total items in the same date
and is it a unique or not

input
2014-02-07,3,monkey
2014-02-07,2,bear
2014-02-08,3,bear
output
2014-02-07,5,2
2014-02-08,3,1


*/