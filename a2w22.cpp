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
    int switchID;
    int lowIP;
    int highIP;
    int pswj;
    int pswk;
} SWITCH;

typedef struct {
    int numSwitch;
    // need 2d array of char maybe
    
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


// POPULATE MASTER SWITCH STRUCT
void populateMaster(MASTERSWITCH * master, char token[][MAXWORD]) {
    char tempStr[MAXWORD];
    memset(tempStr, 0, MAXWORD);
    strcpy(tempStr, token[1]);
    string nSwitchStr = string(tempStr);
    master -> numSwitch = stoi(nSwitchStr);
}

void doMasterPolling() {

}
// UPDATE fds incase more switch wants to connect
void updateFDs() {
    
}

string getfifoName(int x, int y) {
    string name = "fifo-" + to_string(x) + "-" + to_string(y);
    return name;
}

int openfifoRead(string name) {
    string fifoname = "./" + name;
    int fd = open(fifoname.c_str(), O_RDONLY | O_NONBLOCK); // this will block under other side is established
    if (fd < 0) {
        cerr << "cannot open named pipe for read" << endl;
        unlink(name.c_str());
        exit(EXIT_FAILURE);
    }

    return fd;   
}
int openfifoWrite(string name) {
    string fifoname = "./" + name;
//https://stackoverflow.com/questions/580013/how-do-i-perform-a-non-blocking-fopen-on-a-named-pipe-mkfifo
    int fd = open(fifoname.c_str(), O_RDWR | O_NONBLOCK); // O_WRONLY dont work, cant open
    if (fd < 0) {
        cerr << "cannot open named pipe for write" << endl;
        unlink(name.c_str());
        exit(EXIT_FAILURE);
    }
    return fd;
}

// MASTER LOOP
void do_master(MASTERSWITCH * masterswitch, int fds[MAX_SWITCH + 1][MAX_SWITCH + 1]) {
    char readbuff[MAXWORD];
    char writebuff[MAXWORD];
    int nswitch_ = masterswitch -> numSwitch;
    pollfd pollfds[nswitch_ + 1];
    // SETUP KEYBOARD POLL
    pollfds[0].fd = STDIN_FILENO;
    pollfds[0].events = POLLIN;
    pollfds[0].revents = 0;
    // SETUP FILE POLL
    for (int i = 0; i < nswitch_; i++) {
        string outpipeName = getfifoName(0, i + 1);
        string inpipeName = getfifoName(i + 1, 0);
        fds[i + 1][0] = openfifoRead(inpipeName);
        fds[0][i + 1] = openfifoWrite(outpipeName);
        pollfds[i + 1].fd = fds[i + 1][0];
        pollfds[i + 1].events = POLLIN;
    } 

    // 1. poll, if pollfd.fd = -1, poll() will ignore; revent = 0;
    printf("established file descriptors, waiting for HELLO\n");
    while (true) {
        //updateFDs();
        //doMasterPolling();
        int pollret = poll(pollfds, nswitch_ + 1, 0); // 
        if (pollret < 0) {
            cerr << "polling returned -1" << endl;
            exit(EXIT_FAILURE);
        }
        if (pollfds[0].revents and POLLIN) {
            memset(readbuff, 0, MAXWORD);
            int bytesread = read(pollfds[0].fd, readbuff, MAXWORD); // theres a \n character
            readbuff[strlen(readbuff) - 1] = '\0';  // clear \n character
            printf("received: %s\n", readbuff);
        }
        if (pollfds[1].revents and POLLIN) { // poll psw1
            int bytesread = read(pollfds[1].fd, readbuff, MAXWORD);
            if (strcmp(readbuff, "HELLO") == 0) {
                //send ACK
                memset(writebuff, 0, MAXWORD);
                strcpy(writebuff, "ACK");
                write(fds[0][1], writebuff, MAXWORD);
            }
        }
    

    }
}
// pSwitch loop
void do_switch(SWITCH * pSwitch, int fds[MAX_SWITCH + 1][MAX_SWITCH + 1]) {
    // establish connection with master/pswj/pswk
    string fifo_i_master = getfifoName(pSwitch -> switchID, 0);
    string fifo_master_i = getfifoName(0, pSwitch->switchID);  // READ
    string fifo_i_j;
    string fifo_j_i;  // READ
    string fifo_i_k;
    string fifo_k_i;  // READ
    if (pSwitch->pswj != -1) {
        fifo_i_j = getfifoName(pSwitch->switchID, pSwitch->pswj);
        fifo_j_i = getfifoName(pSwitch->pswj, pSwitch->switchID);
        fds[pSwitch->pswj][pSwitch->switchID] = openfifoRead(fifo_j_i);
        fds[pSwitch->switchID][pSwitch->pswj] = openfifoWrite(fifo_i_j);
    }
    if (pSwitch->pswk != -1) {
        fifo_i_k = getfifoName(pSwitch->switchID, pSwitch->pswk);
        fifo_k_i = getfifoName(pSwitch->pswk, pSwitch->switchID);
        fds[pSwitch->pswk][pSwitch->switchID] = openfifoRead(fifo_k_i);
        fds[pSwitch->switchID][pSwitch->pswk] = openfifoWrite(fifo_i_k);
    }
    fds[0][pSwitch->switchID] = openfifoRead(fifo_master_i);
    fds[pSwitch->switchID][0] = openfifoWrite(fifo_i_master);

    pollfd pollfds[SWITCHPORTS_N]; // = 5
    //pollfds[0] = fifo-0-i; pollfds[1]... ..pollfds[4] = keyboard
    pollfds[0].fd = fds[0][pSwitch->switchID];
    pollfds[0].events = POLLIN; 
    pollfds[1].fd = -1;
    pollfds[2].fd = -1;
    pollfds[3].fd = -1;
    pollfds[4].fd = -1;
    char readbuff[MAXWORD];
    char writebuff[MAXWORD];
    memset(writebuff, 0, MAXWORD);
    strcpy(writebuff, "HELLO");
    write(fds[pSwitch->switchID][0], writebuff, MAXWORD);
    while (true) {
        // todo; send HELLO and receive HELLO_ACK
        int pollret = poll(pollfds, SWITCHPORTS_N, 0);
        if (pollret < 0) {
            cerr << "pollret returned -1" << endl;
            exit(EXIT_FAILURE);
        }
        // send HELLO
        if (pollfds[0].revents and POLLIN) {
            // received something
            memset(readbuff, 0, MAXWORD);
            int bytesread = read(pollfds[0].fd, readbuff, MAXWORD);
            readbuff[strlen(readbuff) - 1] = '\0';
            if (strcmp(readbuff, "ACK") == 0)
                printf("received from master: %s\n", readbuff);
        }

    } 

}

int main(int argc, char *argv[]) {
    char tokens[10][MAXWORD];
    int fds[MAX_SWITCH + 1][MAX_SWITCH + 1]; //fds[i][j] means fd for fifo-i-j

    SWITCH pSwitch;
    MASTERSWITCH master;
    
    // parse the input 
    // open fifo

    if (argc == 3 and strcmp(argv[1], "master") == 0) {
        // master switch
        for (int i = 1; i < 3; i++) {
            strcpy(tokens[i - 1], argv[i]);
            // tokens[0] = "master"
            // tokens[1] = "nSwitch"
            //do_master();
        }
        populateMaster(&master, tokens);
        do_master(&master, fds);
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