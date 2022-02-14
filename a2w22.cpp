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


using namespace std;
#define MAXLINE   132
#define MAXWORD    32
#define MSG_KINDS 5
typedef enum { HELLO, HELLO_ACK, ASK, ADD, RELAY } KIND;	  // Packet kinds
char KINDNAME[][MAXWORD]= { "HELLO", "HELLO_ACK", "ASK", "ADD", "RELAY" };

typedef struct {
    int switchID;
    int lowIP;
    int highIP;
    int pswj;
    int pswk;
} SWITCH;

typedef struct {
    int numSwitch;
    
} MASTERSWITCH;

typedef struct {
    int switchNUM;
    int Nneighbor;
    int lowIP;
    int highIP;
} HELLO_PACK;

typedef struct {
    int nothing;
} HELLO_ACK_PACK;

typedef struct {
    int nothing;
} ASK_PACK;

typedef struct {
    int nothing;
} ADD_PACK;

typedef struct {
    int nothing;
} RELAY_PACK;



typedef union {HELLO_PACK pHello; HELLO_ACK_PACK pHelloAck; ASK_PACK pAsk; ADD_PACK pAdd; RELAY_PACK pRelay;} MSG;

typedef struct { KIND kind; MSG msg; } FRAME;


// ------------------------------
// The WARNING and FATAL functions are due to the authors of
// the AWK Programming Language.

void FATAL (const char *fmt, ... )
{
    va_list  ap;
    fflush (stdout);
    va_start (ap, fmt);  vfprintf (stderr, fmt, ap);  va_end(ap);
    fflush (NULL);
    exit(1);
}

void WARNING (const char *fmt, ... )
{
    va_list  ap;
    fflush (stdout);
    va_start (ap, fmt);  vfprintf (stderr, fmt, ap);  va_end(ap);
}
// ------------------------------


// ------------------------------
MSG composeHELLOmsg (const char *a, const char *b, const char *c)
{
    MSG  msg;


    memset( (char *) &msg, 0, sizeof(msg) );


    return msg;
}    
// ------------------------------    
MSG composeACKmsg (int a, int b, int c)
{
    MSG  msg;

    return msg;
}    
// ------------------------------
MSG  composeADDmsg (float a, float b, float c)
{
    MSG  msg;

   
    return msg;
}
// ----------------------------
void sendFrame (int fd, KIND kind, MSG *msg)
{
    FRAME  frame;

    assert (fd >= 0);
    memset( (char *) &frame, 0, sizeof(frame) );
    frame.kind= kind;
    frame.msg=  *msg;
    write (fd, (char *) &frame, sizeof(frame));
}

FRAME rcvFrame (int fd)
{
    int    len; 
    FRAME  frame;

    assert (fd >= 0);
    memset( (char *) &frame, 0, sizeof(frame) );
    len= read (fd, (char *) &frame, sizeof(frame));
    if (len != sizeof(frame))
        WARNING ("Received frame has length= %d (expected= %d)\n",
		  len, sizeof(frame));
    return frame;		  
}
  
      
// ------------------------------
void printFrame (const char *prefix, FRAME *frame)
{
    MSG  msg= frame->msg;
    
}
// ------------------------------



// SERVER LOOP
void do_server (int fifoCS, int fifoSC)
{
    return;
}

// CLIENT LOOP
void do_client (int fifoCS, int fifoSC)
{
    return;
}

void printToken(char tokens[][MAXWORD], int len) {
    for (int i = 0; i < len; i++) {
        printf("argument %d: %s\n", i, tokens[i]);
    }
}

void printSwitch(SWITCH* pSwitch) {
    printf("switchID: %d\n", pSwitch -> switchID);
    printf("lowIP: %d\n", pSwitch -> lowIP);
    printf("highIP: %d\n", pSwitch -> highIP);
    printf("pswj: %d\n", pSwitch -> pswj);
    printf("pswk: %d\n", pSwitch -> pswk);
}

// EXIT IF INCORRECT ARGUMENT
int checkArgument() {
    return 5;
}
// PARSE TOKEN AND POPULATE SWITCH
void populateSwitch(SWITCH * pSwitch, char tokens[][MAXWORD]) {
    //https://stackoverflow.com/questions/5029840/convert-char-to-int-in-c-and-c
    pSwitch -> switchID = tokens[0][3] - '0'; // eg: '1' - '0' = int 
    if (strcmp(tokens[2], "null") == 0) {
        pSwitch -> pswj = -1; // -1 indicates null
    } else  {
        pSwitch -> pswj = tokens[2][3] - '0';
    }
    if (strcmp(tokens[3], "null") == 0) {
        pSwitch -> pswk = -1; // -1 indicates null
    } else {
        pSwitch -> pswk = tokens[3][3] - '0';
    }

    // parse lowIP-HighIP
    int IPstrLen = strlen(tokens[4]);
    string lowIPstr = "";
    string highIPstr = "";
    int flag = 0;
    for (int i = 0; i < IPstrLen + 1; i++) {
        if (tokens[4][i] == '-' and flag == 0) {
            flag = 1;
            continue; 
        }
        if (flag == 1) {
            highIPstr += tokens[4][i];
        } else {
            lowIPstr += tokens[4][i];
        }
    }
    int lowIPint = stoi(lowIPstr);
    int highIPint = stoi(highIPstr);
    pSwitch -> lowIP = lowIPint;
    pSwitch -> highIP = highIPint;

}

int main(int argc, char *argv[]) {
    char tokens[10][MAXWORD];
    
    SWITCH pSwitch;
    
    // parse the input 
        
    // open fifo

    if (argc == 3 and strcmp(argv[1], "master") == 0) {
        // master switch
        for (int i = 1; i < 3; i++) {
            strcpy(tokens[i - 1], argv[i]);
            // tokens[0] = "master"
            // tokens[1] = "nSwitch"
        }
    } else if (argc == 6) {  // SWTICH PERSPECTIVE
        // pswi switch, TODO: error check argument
        for (int i = 1; i < 6; i++) {
            memset(tokens[i - 1], 0, MAXWORD); 
            strcpy(tokens[i - 1], argv[i]);
            // tokens[0] = "pswi"
            // tokens[1] = "datafile"
            // tokens[2] = "null//pswj"
            // tokens[3] = "null/pswk"
            // tokens[4] = "IPlow-IPhigh"
        }
        populateSwitch(&pSwitch, tokens);
        printSwitch(&pSwitch);  
    } else {
        printf("invalid arguments\n");
        return 0;
    }
    //printToken(tokens, argc - 1);  

    return 0;
}