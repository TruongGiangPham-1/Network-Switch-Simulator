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

int main(int argc, char *argv[]) {
    char tokens[10][MAXWORD];
    // parse the input 
        
    // open fifo

    if (argc == 3 and strcmp(argv[1], "master") == 0) {
        // master switch
        for (int i = 1; i < 3; i++) {
            strcpy(tokens[i], argv[i]);
            // tokens[0] = "master"
            // tokens[1] = "nSwitch"
        }
    } else if (argc == 6) {
        // pswi switch, TODO: error check argument
        for (int i = 1; i < 6; i++) {
            strcpy(tokens[i], argv[i]);
            // tokens[1] = "pswi"
            // tokens[2] = "datafile"
            // tokens[3] = "null//pswj"
            // tokens[4] = "null/pswk"
            // tokens[5] = "IPlow-IPhigh"
        }
    }


    return 0;
}