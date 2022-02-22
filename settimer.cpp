#include <iostream>
#include <unistd.h> 
#include <stdarg.h> 
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <poll.h>
#include <sys/time.h>
#include <signal.h>

using namespace std;
#define MAXLINE   132
#define MAXWORD    32
#define MSG_KINDS 5
#define MAX_SWITCH 7
#define SWITCHPORTS_N 5
#define DELAY 2000  // 5s


int canRead = true;
void timerHandler() {
    printf("timer done\n");
    canRead = true;  // set it true so we can read it
}

void calltimer(int delay) {
    struct itimerval timer;
    // we dont want to have a polling alarm
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    //
    timer.it_value.tv_sec = delay / 1000;
    timer.it_value.tv_usec = (delay % 1000) * 1000;
    // total timer time = tv_sec + tv_usec
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
    }
}
void parseFileLine(char* readbuff, int switchID) {
    if (readbuff[0] == '#') {
        printf("# so skipp\n");
        return;
    }
    if (readbuff[0] == '\0') {
        printf("emptyline, skipping\n");
        return;
    }
    vector<string>tokens;
    char readcopied[MAXLINE];
    char * token = NULL, *theRest = NULL;
    memset(readcopied, 0, MAXLINE);
    strcpy(readcopied, readbuff);
    char switchID_char = '0' + switchID;
    //assert(switchID_char == '1');
    //printf("swithcid char [%c]\n", switchID_char);
    theRest = readcopied;
    for (int i = 0; i < 3; i++) {
        token = strtok_r(theRest, " ", &theRest);
        string tok(token);
        tokens.push_back(tok);
    }
    //printf("tokens[0][0] = [%c]\n", tokens[0][0]);
    if (tokens[0][3] == switchID_char and tokens[1] != "delay") {  // process delay last
        int lowip = stoi(tokens[1]);
        int highip = stoi(tokens[2]);
        printf("lowip [%d], highIP [%d]\n", lowip, highip);
    }
    if (tokens[0][3] == switchID_char and tokens[1] == "delay") {
        // handles delay pack
        int delay = stoi(tokens[2]);
        //printf("delay: %d\n", delay);
        // set alarm
        calltimer(delay);
        canRead = false;
        printf("delaying for %d ms\n", delay);
    }

}
int main() {
    struct itimerval timer;
    if (signal(SIGALRM, (void (*)(int))timerHandler) == SIG_ERR) {
        perror("Unable to catch SIGALARM");
        exit(1);
    }


    const char* m = "ex3.dat";
    string str = "./" + string(m);

    FILE* fp;
    char readbuff[MAXLINE];
    fp = fopen(str.c_str(), "r");
    if (fp == NULL) {
        perror("failed to open");
        exit(1);
    }
    memset(readbuff, 0, MAXLINE);
    while (true) {
        if (!canRead) {  // if canRead = false we skip
            continue;  
        }
        if (fgets(readbuff, MAXLINE, (FILE*)fp) == NULL) {
            break;
        }
        readbuff[strlen(readbuff) - 1] = '\0';
        //printf("line:[%s]\n", readbuff);
        parseFileLine(readbuff, 1);
        memset(readbuff, 0, MAXLINE);
    }
    printf("printing eof: [%s]\n", readbuff);
    return 0;
}