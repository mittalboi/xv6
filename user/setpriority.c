#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

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

    if (argc >= 4 && argc <= 2)
    {
        printf("Invalid number of arguments.\n");
        exit(0);
    }
    int new_priority = conversion(argv[1]);
    int pid = conversion(argv[2]);
    if (new_priority < 0)
    {
        printf("Invalid priority [0,100]\n");
        exit(1);
    }
    if (new_priority > 100)
    {
        printf("Invalid priority [0,100]\n");
        exit(1);
    }
    int old_priority = setpriority(new_priority, pid);
    if (old_priority < 0)
    {
        printf("Process not found\n");
        exit(1);
    }
    else
    {
        printf("Process PID: %d updated from priority %d to %d.\n", pid, old_priority, new_priority);
        exit(0);
    }
}