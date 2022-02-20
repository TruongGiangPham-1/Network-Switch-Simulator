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
typedef struct {
    int one;
    int two;
    int three;
    int four;
    int five;
} SWITCH;
void printKind(KIND kind) {
    printf("recieved %s\n", KINDNAME[kind]);
}

int main() {
//    printKind(HELLO);
//    printKind(HELLO_ACK);
//    printKind(ASK);
//    printKind(ADD);
//    printKind(RELAY);
//
    //const char* m = "datafile";
    //string str = "./" + string(m);
    //cout << str << endl;
    SWITCH s1 = {
        1,
        3,
        4
    };
    printf("%d, %d, %d, %d, %d\n", s1.one, s1.two, s1.three, s1.four, s1.five);
    
    return 0;
}