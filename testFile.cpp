
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


using namespace std;
#define MAXLINE   132
#define MAXWORD    32
#define MSG_KINDS 5
#define MAX_SWITCH 7
#define SWITCHPORTS_N 5

typedef enum { HELLO, HELLO_ACK, ASK, ADD, RELAY } KIND;	  // Packet kinds

char KINDNAME[][MAXWORD]= { "HELLO", "HELLO_ACK", "ASK", "ADD", "RELAY" };

void printKind(KIND kind) {
    printf("recieved %s\n", KINDNAME[kind]);
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
        // we want this line
        //printf("%s,%s,%s\n", tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str());
        //1. check the rule table 
        // 2. if dont exist, we send ASK. 
        //if (tokens[0])
        int lowip = stoi(tokens[1]);
        int highip = stoi(tokens[2]);
        printf("lowip [%d], highIP [%d]\n", lowip, highip);
    }

}
int main() {
//    printKind(HELLO);
//    printKind(HELLO_ACK);
//    printKind(ASK);
//    printKind(ADD);
//    printKind(RELAY);

    const char* m = "ex3.dat";
    string str = "./" + string(m);
    cout << str << endl;
    FILE* fp;
    char readbuff[MAXLINE];
    fp = fopen(str.c_str(), "r");
    if (fp == NULL) {
        perror("failed to open");
        exit(1);
    }
    memset(readbuff, 0, MAXLINE);
    while (fgets(readbuff, MAXLINE, (FILE*)fp) != NULL) {
        //if (readbuff[strlen(readbuff) - 1] == '\n') {
        //    readbuff[strlen(readbuff) - 1] = '\0';
        //}
        //printf("line:[%s]\n", readbuff);
        parseFileLine(readbuff, 1);
        memset(readbuff, 0, MAXLINE);
    }
    printf("printing eof: [%s]\n", readbuff);
    return 0;
}