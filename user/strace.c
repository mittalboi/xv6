
#include "kernel/param.h"
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
    if (argc < 3)
        fprintf(2, "Invalid number of arguments!!");

    int result = conversion(argv[1]);
    if(result < 0)
        fprintf(2, "Invalid number of arguments!!");

    if (trace(result) < 0)
    {
        fprintf(2, "%s: Invalid strace!!\n", argv[0]);
        exit(1);
    }
    exec(argv[2], argv + 2);
    exit(0);
}