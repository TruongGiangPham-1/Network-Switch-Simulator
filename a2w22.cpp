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
#include <utility> // for PAIR


using namespace std;
#define MAXLINE   132
#define MAXWORD    32
#define MSG_KINDS 5
#define MAX_SWITCH 7
#define SWITCHPORTS_N 5
#define MAXIP 1000

typedef pair<int, int> PII; // PII = pair int int
typedef enum { HELLO, HELLO_ACK, ASK, ADD, RELAY } KIND;	  // Packet kinds
char KINDNAME[][MAXWORD]= { "HELLO", "HELLO_ACK", "ASK", "ADD", "RELAY" };
typedef enum {FORWARD, DROP} tableACTION;    // forward table action
char ACTIONNAME[][MAXWORD] = {"FORWARD", "DROP"};
typedef struct {  // each switch has vector of fTABLEROW
    int scrIP_lo;
    int scrIP_hi;
    int destIP_lo;
    int destIP_hi;
    tableACTION ACTIONTYPE;
    int actionVAL;
    int pktCount;
} fTABLEROW;

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
    //SWITCH switchArray[MAX_SWITCH]; // array of switches for 'info' command
    int helloCount;
    int askCount;
    int ackCount;
    int addCount;
    
} MASTERSWITCH;

typedef struct {
    int switchNUM;
    int Nneighbor;
    int lowIP;
    int highIP;
    int pswj;
    int pswk;
} HELLO_PACK;

typedef struct {
    int nothing;
} HELLO_ACK_PACK;

typedef struct {
    int switchID;  // just in case
    int scrIP;
    int destIP;  // ask for this destIP

} ASK_PACK;

typedef struct {
    int nothing;
    int destIP_lo;
    int destIP_hi;
    tableACTION ACTIONTYPE;
    int actionVAL;
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
MSG composeHELLOmsg (int switchID, int nNeighbor, int lowIP, int highIP, int pswj, int pswk)
{
    MSG  msg;
    memset( (char *) &msg, 0, sizeof(msg) );
    //char helloStr[] = "HELLO";
    msg.pHello.switchNUM = switchID;
    msg.pHello.Nneighbor = nNeighbor;
    msg.pHello.lowIP = lowIP;
    msg.pHello.highIP = highIP;
    msg.pHello.pswj = pswj;
    msg.pHello.pswk = pswk;
    return msg;
}    
// ------------------------------    
MSG composeACKmsg ()
{
    MSG  msg;
    memset( (char *) &msg, 0, sizeof(msg) );
    msg.pHelloAck.nothing = 0; // dummy value
    return msg;
}    
// ------------------------------
MSG  composeADDmsg (int dest_lo, int dest_hi, tableACTION action, int actionVAL)
{
    MSG  msg;
    msg.pAdd.ACTIONTYPE = action;
    msg.pAdd.actionVAL = actionVAL;
    msg.pAdd.destIP_lo = dest_lo;
    msg.pAdd.destIP_hi = dest_hi;
   
    return msg;
}
MSG composeASKmsg(int scrIP, int destIP, int switchID) {
    MSG msg;
    msg.pAsk.scrIP = scrIP;
    msg.pAsk.destIP = destIP;
    msg.pAsk.switchID = switchID;
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

FRAME rcvFrame (int fd, struct pollfd* pollfds, int index)
{
    int    len; 
    FRAME  frame;

    assert (fd >= 0);
    memset( (char *) &frame, 0, sizeof(frame) );
    len= read (fd, (char *) &frame, sizeof(frame));
    if (len != sizeof(frame))
        WARNING ("Received frame has length= %d (expected= %d)\n",
		  len, sizeof(frame));
    if (len == 0) {
        pollfds[index].fd = -1; // means that othe end have closed pipe 
    }
    return frame;		  
}
  
      
// ------------------------------
void printFrame (const char *prefix, FRAME *frame)
{
    // prefix = "received"
    MSG  msg= frame->msg;
    printf("%s [%s] \n", prefix, KINDNAME[frame->kind]);
    switch (frame->kind)
    {
    case HELLO/* constant-expression */:
        /* code */
        printf("switchID: [%d], nNeighbor: [%d], lowIP: [%d], highIP: [%d]\n", msg.pHello.switchNUM, msg.pHello.Nneighbor, 
               msg.pHello.lowIP, msg.pHello.highIP);
        break;
    case HELLO_ACK:
        printf("ACKED\n");
        break;
    case ASK:
    {
        printf("[ASK]: header= (srcIP= %d, destIP=%d)\n", msg.pAsk.scrIP, msg.pAsk.destIP);
    }
    case ADD:
    {
        printf("[ADD]: (srcIP-0-1000, destIP=%d-%d, action=%s:%d, pktCount= 0\n", 
        msg.pAdd.destIP_lo, msg.pAdd.destIP_hi, ACTIONNAME[msg.pAdd.ACTIONTYPE], msg.pAdd.actionVAL);
    }
    default:
        break;
    }
    
}
// ------------------------------

PII getLowIP_HighIP(const char * ips) {
    // ips = "lowip=highip"
    int IPstrLen = strlen(ips);
    string lowIPstr = "";
    string highIPstr = "";
    int flag = 0;
    for (int i = 0; i < IPstrLen + 1; i++) {
        if (ips[i] == '-' and flag == 0) {
            flag = 1;
            continue; 
        }
        if (flag == 1) {
            highIPstr += ips[i];
        } else {
            lowIPstr += ips[i];
        }
    }
    int lowIPint = stoi(lowIPstr);
    int highIPint = stoi(highIPstr);
    PII p = make_pair(lowIPint, highIPint);
    return p;
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
    PII ipPair = getLowIP_HighIP(tokens[4]);
    pSwitch -> lowIP = ipPair.first;
    pSwitch -> highIP = ipPair.second;

}


// POPULATE MASTER SWITCH STRUCT
void populateMaster(MASTERSWITCH * master, char token[][MAXWORD]) {
    char tempStr[MAXWORD];
    memset(tempStr, 0, MAXWORD);
    strcpy(tempStr, token[1]);
    string nSwitchStr = string(tempStr);
    master -> numSwitch = stoi(nSwitchStr);
    master->ackCount = 0;
    master->addCount = 0;
    master->askCount = 0;
    master->helloCount = 0;
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
// KEYBOARD ----------------------------------------------------------------
void printInfoMaster(vector<SWITCH>&sArray) {
    // mode == master or switch
    assert(sArray.size() > 0);
    printf("Switch information:\n");
    for (int i = 0; i < sArray.size(); i++) {
        string port3 = to_string(sArray[i].lowIP) + "-" + to_string(sArray[i].highIP);
        printf("[psw %d] port1= %d, port2= %d, port3= %s\n", 
            sArray[i].switchID, sArray[i].pswj, sArray[i].pswk, port3.c_str());
    }
    return;
}
void printInfoSwitch(vector<fTABLEROW>&forwardTable) {
    assert(forwardTable.size() > 0);
    for (int i = 0; i < forwardTable.size(); i++) {
       // I realized i can just do %d-%d instead of above
        printf("[%d] (scrIP= %d-%d, destIP= %d-%d, action=%s:%d, pktCount= %d\n",
        i, forwardTable[i].scrIP_lo, forwardTable[i].scrIP_hi, forwardTable[i].destIP_lo, 
        forwardTable[i].destIP_hi, ACTIONNAME[forwardTable[i].ACTIONTYPE], forwardTable[i].actionVAL, 
        forwardTable[i].pktCount);
   } 
}
void parseKeyboardSwitch(const char* keyboardInput, vector<fTABLEROW>&ftable) {
    if (strcmp(keyboardInput, "info") == 0) {
        assert(ftable.size() > 0);
        printInfoSwitch(ftable);
    }
}
void parseKeyboardMaster(const char * keyboardInput, vector<SWITCH>&sArray) {
    // print stuff/
    if (strcmp(keyboardInput, "info") == 0) {
        assert(sArray.size() >= 1); // assert at least one switch exist
        printInfoMaster(sArray);
    } 
}
// --------------------------------------------------------------------------------
int getACK(pollfd * pollfds) {  // poll master until i get acknowledge
    FRAME frame;
    while (true) {
        int pollret = poll(pollfds, SWITCHPORTS_N, 1);
        if (pollret < 0) {
            cerr << "pollret returned -1" << endl;
            exit(EXIT_FAILURE);
        }
        if (pollfds[0].revents and POLLIN) { // poll form master
            frame = rcvFrame(pollfds[0].fd, pollfds, 0);
            if (pollfds[0].fd == -1) return 0; //master closed
            if (frame.kind == HELLO_ACK) {
                printFrame("recived ACK ", &frame);
                return 1;
            }   
        }
    }
    return 0;
}

void parseAndSendToSwitch(int fd, FRAME * frame, vector<SWITCH>& sArray, MASTERSWITCH * master, SWITCH * sw) {
    // parse Frame and send to fd // 
    MSG msg;
    switch (frame->kind)
    {
    case HELLO:
        {
            master->helloCount += 1;
            master->ackCount += 1;
            msg = composeACKmsg();
            sendFrame(fd, HELLO_ACK, &msg);
            SWITCH incomingSwitch = {
                /*.highIP =*/ (frame->msg).pHello.highIP,
                /*.lowIP = */(frame->msg).pHello.lowIP,
                /*.pswj = */ (frame->msg).pHello.pswj,
                /*.pswk =*/ (frame->msg).pHello.pswk,
                /*.switchID = */ (frame->msg).pHello.switchNUM
            };
            sArray.push_back(incomingSwitch);
            break;
        }
    case ASK:  // compose ADD_PACK
        {
            int dIP_toASk = ((frame->msg).pAsk).destIP;
            bool found = false;
            int switchIndex = -1;
            for (int i = 0; i < sArray.size(); i++) {
                if (sArray[i].lowIP <= dIP_toASk and dIP_toASk <= sArray[i].highIP) {
                    found = true;
                    break;
                }
            }
            if ( switchIndex == -1) {  // no found == make DROP rule
                msg = composeADDmsg(dIP_toASk, dIP_toASk, DROP, 0); // eg (0-1000, 300-300, DROP, 0)
                sendFrame(fd, ADD, &msg);
            } else {
                msg = composeADDmsg(sArray[switchIndex].lowIP,sArray[switchIndex].highIP, FORWARD, switchIndex); 
                sendFrame(fd, ADD, &msg);
            }
            master->ackCount +=1;
            master->addCount +=1;
            break;
        }
    case HELLO_ACK:
        {
            printFrame("recieved: ", frame);
        }
    default:
        printf("default\n");
        break;
    }
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
    int switchIndex = 0; // whenever switch connect, we update switch array
    // 1. poll, if pollfd.fd = -1, poll() will ignore; revent = 0;
    MSG msg;
    FRAME frame;
    vector<SWITCH> sArray;
    printf("established file descriptors, waiting for HELLO\n");
    while (true) {
        //updateFDs();
        //doMasterPolling();
        int pollret = poll(pollfds, nswitch_ + 1, 1); // 
        //cout << "pollred " << pollret << endl;
        if (pollret < 0) {
            cerr << "polling returned -1" << endl;
            exit(EXIT_FAILURE);
        }
        if (pollfds[0].revents and POLLIN) {  // keyboard poll
            memset(readbuff, 0, MAXWORD);
            int bytesread = read(pollfds[0].fd, readbuff, MAXWORD); // theres a \n character
            readbuff[strlen(readbuff) - 1] = '\0';  // clear \n character
            parseKeyboardMaster(readbuff, sArray);
        }
        for (int i = 1; i < nswitch_ + 1; i++) {
            if (pollfds[i].revents and POLLIN) {
                // something to read
                frame = rcvFrame(pollfds[i].fd, pollfds, i);
                if (pollfds[i].fd == -1) continue; // other end closed pipe so rcvFrame() changed fd to -1
                printFrame("recieved ", &frame); 
                parseAndSendToSwitch(fds[0][i], &frame, sArray, masterswitch, nullptr);
                pollfds[i].revents = 0;
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
    //pollfds[0] = fifo-0-i/master to pswi; pollfds[1]=port1l pollfds[2]=port2, pollfds[4] = keyboard

    pollfds[0].fd = fds[0][pSwitch->switchID];
    pollfds[0].events = POLLIN; 
    pollfds[1].fd = -1;
    pollfds[2].fd = -1;
    pollfds[3].fd = -1;
    pollfds[4].fd = -1;
    char readbuff[MAXWORD];
    char writebuff[MAXWORD];
    //memset(writebuff, 0, MAXWORD);
    //strcpy(writebuff, "HELLO");
    //write(fds[pSwitch->switchID][0], writebuff, MAXWORD);
    FRAME frame;
    MSG msg;
    msg = composeHELLOmsg(pSwitch->switchID, 0, pSwitch->lowIP, pSwitch->highIP, pSwitch->pswj, pSwitch->pswk);
    sendFrame(fds[pSwitch->switchID][0], HELLO, &msg);
    int ackowledge = getACK(pollfds);
    assert(ackowledge == 1); // assert its ackolowdged

    msg = composeASKmsg(200, 300, pSwitch->switchID);
    sendFrame(fds[pSwitch->switchID][0], ASK, &msg);
    while (true) {
        // todo; send HELLO and receive HELLO_ACK
        int pollret = poll(pollfds, SWITCHPORTS_N, 1);
        if (pollret < 0) {
            cerr << "pollret returned -1" << endl;
            exit(EXIT_FAILURE);
        }
        // poll keyboard
        // send HELLO
        for (int i = 0; i < SWITCHPORTS_N - 1; i++) {  // check everything exept keyboard[0 - 3]
            if (pollfds[i].revents and POLLIN) {
                frame = rcvFrame(pollfds[i].fd, pollfds, i);
                if (pollfds[i].fd == -1) continue; //closed
                printFrame("recieved ", &frame);  
            }
        }

    } 
}
// ----------------------------------------------------------------------------------
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
        //printSwitch(&pSwitch);  
        do_switch(&pSwitch, fds);
    } else {
        printf("invalid arguments\n");
        return 0;
    }
    //printToken(tokens, argc - 1);  
    
    return 0;
}