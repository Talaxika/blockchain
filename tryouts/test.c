#include <time.h>
#include <stdio.h>


int main(void) {

    time_t t;
    time(&t);

    char sendbuf[16] = "";
    snprintf(sendbuf, 16, "%d", t);
    printf("%s\n", sendbuf);

    return 0;
}