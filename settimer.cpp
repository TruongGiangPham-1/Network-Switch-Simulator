#include <sys/time.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>

using namespace std;
#define DELAY 2000  // 5s
int timerflag = false;
void timerHandler() {
    printf("int titi timer\n");
    timerflag = true;
}

int main() {
    struct itimerval timer;
    if (signal(SIGALRM, (void (*)(int))timerHandler) == SIG_ERR) {
        perror("Unable to catch SIGALARM");
        exit(1);
    }
    // we dont want to have a polling alarm
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    //
    timer.it_value.tv_sec = DELAY / 1000;
    timer.it_value.tv_usec = (DELAY % 1000) * 1000;
    // total timer time = tv_sec + tv_usec
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
    }

    while (true) {
        printf("timerflag = %d\n", timerflag);
    }
    return 0;
}