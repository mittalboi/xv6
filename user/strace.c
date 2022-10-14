#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define stderr 2

int conversion(char *s)
{
    int sum = 0, n;
    for (int i = 0; i < strlen(s); i++)
    {
        n = s[strlen(s) - 1 - i] - '0';

        long long answer = 1, k = 0;
        while (k < i)
        {
            answer *= 10;
            k++;
        }

        sum += n * answer;
        // printf("ola: %d", repeati);
    }
    return sum;
}

int main(int argc, char *argv[])
{
    char *newargs[MAXARG];
    int repeati = 0;

    if (argc <= 2)
    {
        fprintf(stderr, "Error! number of arguments\n");
        exit(1);
    }

    if (argv[1][0] > '0' && argv[1][0] < '9')
    {
        repeati = conversion(argv[1]);

        int check = strace(repeati);
        if (check < 0)
        {
            fprintf(stderr, "Error! trace command\n");
            exit(1);
        }

        int j = 0, k = 2;
        while (k < argc)
        {
            if (k > MAXARG)
            {
                break;
            }
            newargs[j] = argv[k];
            j++;
            k++;
        }
        
        

        exec(argv[2], newargs);
        exit(0);
    }

    fprintf(stderr, "Error! trace command\n");
    exit(1);
}