
/*
# ------------------------------
# a2w22.cpp -- A2 program for 379
#     This file includes the following:
#     - Functions WARNING and FATAL that can be used to report warnings and
#       errors after system calls
#     -Functions timerHander() and USR1handler() to handler SIGARM and SIGUSR1 respectively.
#     -Functions that compose HELLO/ACK/ADD/ASK/RELAY packages
#     -sendFrame() and rcvframe() to communicate using fifos
#     -a function printFrame that prints msgs from fifo
#     - and much more
#    
#  Compile with:  g++ a2w22.cpp -o a2w22         (no check for warnings)
#		  g++ -g a2w22.cpp -o a2w22   (for debugging with gdb)
#
#  Usage:  starter  stringArg  intArg	 (e.g., starter abcd 100)
#
#  Author: Truong-Giang Pham (for CMPUT 379, U. of Alberta)
# ------------------------------
*/
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
#include <sys/time.h>  // for setitimer
#include <signal.h>  // for signal()


using namespace std;
#define MAXLINE   132
#define MAXWORD    32
#define MSG_KINDS 5
#define MAX_SWITCH 7
#define SWITCHPORTS_N 5
#define MAXIP 1000
int canRead = true; // flag for delay, if !canRead, we dont read from file

typedef pair<int, int> PII; // PII = pair int int
typedef enum { HELLO, HELLO_ACK, ASK, ADD, RELAY } KIND;	  // Packet kinds
char KINDNAME[][MAXWORD]= { "HELLO", "HELLO_ACK", "ASK", "ADD", "RELAY" };
typedef enum {FORWARD, DROP} tableACTION;    // forward table action
char ACTIONNAME[][MAXWORD] = {"FORWARD", "DROP"};
// SOME FUNCTION DECLARATION
void printInfoMaster();
void printInfoSwitch();
//

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
    int nACKreceived;
    int nADDreceived;
    int nRELAYIN;
    int nHELLOtransm;
    int nASKtrans;
    int nRelayout;
    int admit; // admit in forwardtable
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
    int destID;
} HELLO_ACK_PACK;

typedef struct {
    int switchID;  // just in case
    int scrIP;
    int destIP;  // ask for this destIP

} ASK_PACK;

typedef struct {
    int destSwitchID; // switch id of switch that can enforce rule
    int destIP_lo;
    int destIP_hi;
    tableACTION ACTIONTYPE;
    int actionVAL;
    int currSwitchID; // id to switch that send ask
    int asked_srcIP;  // for relay, they needed this for output
    int asked_destIP; // for relay
} ADD_PACK;

typedef struct {
    int switchID;  // source switch id
    int destSwitchID; // assert(destSwithID == switchID of switch relayed to)
    int srcIP;  // not needed, but for error check
    int destIP;  // notneeded, but for error check
} RELAY_PACK;



typedef union {HELLO_PACK pHello; HELLO_ACK_PACK pHelloAck; ASK_PACK pAsk; ADD_PACK pAdd; RELAY_PACK pRelay;} MSG;

typedef struct { KIND kind; MSG msg; } FRAME;

vector<fTABLEROW> forwardTable;
vector<SWITCH> sArray;
MASTERSWITCH globalMaster;
SWITCH globalSwitch;
int isMaster = false; // indicate if this program is ran as master or switch
int isSwitch = false; 
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
// ALARM AND SIGNAL STUFF FOR DELAY AND SIGUSR1 implementation
void timerHandler() {
    printf("\n");
    printf("** Delay period ended\n");
    printf("\n");
    canRead = true; // set it to true so we can read it
}
void callTimer(int delay) {
    struct itimerval timer;
    // we dont want polling timer so dont trigger it periodically
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    //
    timer.it_value.tv_sec = delay / 1000; // get sec component;
    timer.it_value.tv_usec = (delay % 1000) * 1000; // microsec component
    // total timer time = tv_sec + tv_usec
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
    }
}
void USR1handler() {
    if (isMaster) {
        printf("SIGUSR1 detected.. printing Master info\n");
        printInfoMaster();
    } else if (isSwitch) {
        printf("SIGUSR1 detected.. printing Switch info\n");
        printInfoSwitch();
    }
}
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
MSG composeACKmsg (int destid)
{
    MSG  msg;
    memset( (char *) &msg, 0, sizeof(msg) );
    msg.pHelloAck.destID = destid; // dummy value
    return msg;
}    
MSG  composeADDmsg (int dest_lo, int dest_hi, tableACTION action, int actionVAL, int destSwitchID, int switchID, int asked_srcIP, int asked_destIP)
{
    MSG  msg;
    //printf("in composeADD\n");
    //printf("line 176\n");
    msg.pAdd.ACTIONTYPE = action;
    msg.pAdd.actionVAL = actionVAL;
    msg.pAdd.destIP_lo = dest_lo;
    msg.pAdd.destIP_hi = dest_hi;
    msg.pAdd.destSwitchID = destSwitchID;
    msg.pAdd.currSwitchID = switchID;
    msg.pAdd.asked_srcIP = asked_srcIP;
    msg.pAdd.asked_destIP = asked_destIP;
    //printf("line 183\n");
    return msg;
}
MSG composeASKmsg(int scrIP, int destIP, int switchID) {
    MSG msg;
    msg.pAsk.scrIP = scrIP;
    msg.pAsk.destIP = destIP;
    msg.pAsk.switchID = switchID;
    return msg; 

}
MSG composeRELAYmsg(FRAME * frame, int switchID) {
    MSG msg;
    assert(frame->kind == ADD);
    assert((frame->msg).pAdd.ACTIONTYPE == FORWARD);
    assert((frame->msg).pAdd.actionVAL != 0);
    msg.pRelay.destSwitchID = (frame->msg).pAdd.destSwitchID; // 
    msg.pRelay.switchID = switchID;
    msg.pRelay.srcIP = (frame->msg).pAdd.asked_srcIP;
    msg.pRelay.destIP = (frame->msg).pAdd.asked_destIP;
    return msg;

}
// ----------------------------
/*
sendFrame: c379 lab3 functions that send Frame to fifo
*/
void sendFrame (int fd, KIND kind, MSG *msg)
{
    FRAME  frame;

    assert (fd >= 0);
    memset( (char *) &frame, 0, sizeof(frame) );
    frame.kind= kind;
    frame.msg=  *msg;
    write (fd, (char *) &frame, sizeof(frame));
}
/* 
rcvFrame: c379 lab3 functions, but I modifed it for my need
          it now changes a pollfd fd to -1 if the otherside closes fd
*/
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
    if (len == 0) {  // ** MY EDIT
        pollfds[index].fd = -1; // means that othe end have closed pipe 
    }
    return frame;		  
}
  
      
// ------------------------------
/* 
printFrame: prints FRAME that matches sample output
*/
void printFrame (const char *prefix, FRAME *frame)
{
    // prefix = "received"
    MSG  msg= frame->msg;
    printf("%s [%s] ", prefix, KINDNAME[frame->kind]);
    switch (frame->kind)
    {
    case HELLO/* constant-expression */:
        /* code */
        {
            //printf("switchID: [%d], nNeighbor: [%d], lowIP: [%d], highIP: [%d]\n", msg.pHello.switchNUM, msg.pHello.Nneighbor, 
                //msg.pHello.lowIP, msg.pHello.highIP);
            string port1Str;
            string port2Str;
            if (msg.pHello.pswj == -1) {
                port1Str = "null";
            } else {
                port1Str = "psw" + to_string(msg.pHello.pswj);
            }
            if (msg.pHello.pswk == -1) {
                port2Str = "null";
            } else {
                port2Str = "psw" + to_string(msg.pHello.pswk);
            }
            printf("(scr= psw%d, dest= master) [HELLO]:\n", msg.pHello.switchNUM);
            printf("     (port0= master, port1=%s, port2=%s, port3=%d-%d\n", port1Str.c_str(), port2Str.c_str(),
            msg.pHello.lowIP, msg.pHello.highIP);
            break;
        }
    case HELLO_ACK:
        printf("(src= master, dest=psw%d) [HELLO_ACK]\n", msg.pHelloAck.destID);
        break;
    case ASK:
    {
        //printf("[ASK]: header= (srcIP= %d, destIP=%d)\n", msg.pAsk.scrIP, msg.pAsk.destIP);
        printf("(scr=psw%d, dest= master) [ASK]: header= (srcIP= %d, destIP= %d)\n"
        ,msg.pAsk.switchID, msg.pAsk.scrIP, msg.pAsk.destIP);
        break;

    }
    case ADD:
    {
        //printf("[ADD]: (srcIP: 0-1000, destIP=%d-%d, action=%s:%d, pktCount= 0\n", 
        //msg.pAdd.destIP_lo, msg.pAdd.destIP_hi, ACTIONNAME[msg.pAdd.ACTIONTYPE], msg.pAdd.actionVAL);
        //printf("PRINTING ADD\n");
        printf("(src= master, dest= psw%d) [ADD]:\n", msg.pAdd.currSwitchID);
        printf(" scrIP=0-1000, destIP= %d-%d, action= %s:%d, pktCount=0\n", 
        msg.pAdd.destIP_lo, msg.pAdd.destIP_hi, ACTIONNAME[msg.pAdd.ACTIONTYPE], msg.pAdd.actionVAL);
        break;
    }
    case RELAY:
    {
        printf("src= psw%d, dest= psw%d)  [RELAY]: header= (srcIP= %d, destIP= %d)\n", 
        msg.pRelay.switchID, msg.pRelay.destSwitchID, msg.pRelay.srcIP, msg.pRelay.destIP);
    }
    default:
        break;
    }
    
}


// ------------------------------
/* 
getLowIP_HighIP: parse low/high ip from program argument(for switch)
Argument: ips - "lowip-higip" form argument when running as switch
Returns: 
    -a pair<int, int> = (lowip, highip)
*/
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

/* 
PopulateSwitch: PARSE command arguments AND POPULATE SWITCH(for switch)
Argument:
    -pSwitch: structure that represent a Switch
    -tokens: tokenized commanline arguments
*/
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
    pSwitch->nACKreceived = 0;
    pSwitch->nADDreceived = 0;
    pSwitch->nASKtrans = 0;
    pSwitch->nHELLOtransm = 0;
    pSwitch->nRELAYIN = 0;
    pSwitch->nRelayout = 0;
    pSwitch->admit = 0;

}


/* populateMaster: POPULATE MASTER SWITCH STRUCT (for master)
Argument: 
    -master: struct that represent master
    -token: tokenized commandline argument 
*/
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

/* 
getfifoName: returns fifo name;
*/
string getfifoName(int x, int y) {
    string name = "fifo-" + to_string(x) + "-" + to_string(y);
    return name;
}

/* 
openfifoRead: open fifo to read, none block
argument:
    -name: fifoname
return:
    -fd
*/
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

/* 
openfifoWrite: open fifo to write, none block
argument:
    -name: fifoname
return:
    -fd
*/

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
/* 
print master info
*/
void printInfoMaster() {
    // mode == master or switch
    assert(sArray.size() > 0);
    //printf("sArrap[0].highIP == %d\n", sArray[0].highIP); == -1 for somereason
    printf("Switch information:\n");
    for (int i = 0; i < sArray.size(); i++) {
        //string port3 = to_string(sArray[i].lowIP) + "-" + to_string(sArray[i].highIP);
        printf("[psw %d] port1= %d, port2= %d, port3= %d-%d\n", 
            sArray[i].switchID, sArray[i].pswj, sArray[i].pswk, sArray[i].lowIP, sArray[i].highIP);
    }
    printf("\n");
    printf("Packet Stats:\n");
    printf("       Received: HELLO:%d, ASK:%d\n", globalMaster.helloCount, globalMaster.askCount);
    printf("       Transmitted: HELLO_ACK: %d, ADD:%d\n", globalMaster.ackCount, globalMaster.addCount);
    return;
}

/* 
print switch info
*/
void printInfoSwitch() {
    assert(forwardTable.size() > 0);
    printf("Forwarding table: \n");
    for (int i = 0; i < forwardTable.size(); i++) {
       // I realized i can just do %d-%d instead of above
        printf("[%d] (scrIP= %d-%d, destIP= %d-%d, action=%s:%d, pktCount= %d)\n",
        i, forwardTable[i].scrIP_lo, forwardTable[i].scrIP_hi, forwardTable[i].destIP_lo, 
        forwardTable[i].destIP_hi, ACTIONNAME[forwardTable[i].ACTIONTYPE], forwardTable[i].actionVAL, 
        forwardTable[i].pktCount);
    }
    printf("\n");
    printf("Packet Stats:\n");
    printf("Received: ADMIT: %d, HELLO_ACK: %d, ADD: %d, RELAYIN: %d\n", 
    sArray[0].admit, sArray[0].nACKreceived, sArray[0].nADDreceived, sArray[0].nRELAYIN);
    printf("Transmitted: HELLO: %d, ASK: %d, RELAYOUT: %d\n", 
    sArray[0].nHELLOtransm, sArray[0].nASKtrans, sArray[0].nRelayout); 
}

/* 
Parse typed msg from keyboard (for switch)
argument:
    -keyboardInout: either "info" or "exit"
*/
void parseKeyboardSwitch(const char* keyboardInput) {
    if (strcmp(keyboardInput, "info") == 0) {
        assert(forwardTable.size() > 0);
        //printf("reached parseKEyboarSwitch\n");
        printInfoSwitch();
    } else if (strcmp(keyboardInput, "exit") == 0) {
        assert(forwardTable.size() > 0);
        printInfoSwitch();
        exit(0);
    }
}
/* 
Parse typed msg from keyboard (for master)
argument:
    -keyboardInout: either "info" or "exit"
*/
void parseKeyboardMaster(const char * keyboardInput) {
    // print stuff/
    if (strcmp(keyboardInput, "info") == 0) {
        assert(sArray.size() >= 1); // assert at least one switch exist
        printInfoMaster();
    } else if (strcmp(keyboardInput, "exit") == 0) {
        assert(sArray.size() >= 1);
        printInfoMaster();
        exit(0);
    } 
}
// --------------------------------------------------------------------------------
/*
poll fifo-0-i until i receives ack from master (for switch)
argument:
    - pollfds: pollfd array
return:
    -1 if ack received
*/
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
// ----------------------------------------------------------------------------------
/* 
parse and respond to incoming packages from switches to master(for master)
Argument: 
    -fd: fd of fifo-i-0
    -frame: package received
    -master: master struct
    -sw: NULL pointer, unused
*/
void parseAndSendToSwitch(int fd, FRAME * frame, MASTERSWITCH * master, SWITCH * sw) {
    // parse Frame and send to fd // 
    MSG msg;
    switch (frame->kind)
    {
    case HELLO:
        {
            master->helloCount += 1;
            master->ackCount += 1;
            msg = composeACKmsg((frame->msg).pHello.switchNUM);
            sendFrame(fd, HELLO_ACK, &msg);
            FRAME fack;
            fack.kind = HELLO_ACK;
            fack.msg = msg;
            printFrame("Transmitted  ", &fack);
            printf("\n");
            SWITCH incomingSwitch = {
                /*.switchID = */ (frame->msg).pHello.switchNUM,
                /*.lowIP = */(frame->msg).pHello.lowIP,
                /*.highIP =*/ (frame->msg).pHello.highIP,
                /*.pswj = */ (frame->msg).pHello.pswj,
                /*.pswk =*/ (frame->msg).pHello.pswk,
            };
            //printf("[adding switch] switchID[%d], highIP[%d]", (frame->msg).pHello.switchNUM, (frame->msg).pHello.highIP);
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
                    switchIndex = i;
                    break;
                }
            }
            printf("before compose ADD \n");
            if ( switchIndex == -1) {  // no found == make DROP rule
                //assert(sw->switchID > 0);  // segfault cuz sw is null
                msg = composeADDmsg(dIP_toASk, dIP_toASk, DROP, 0, 0, (frame->msg).pAsk.switchID, 0, 0); // eg (0-1000, 300-300, DROP, 0)
                sendFrame(fd, ADD, &msg);
                //printf("line 517\n");
                FRAME fadd;
                fadd.kind = ADD;
                fadd.msg = msg;
                printFrame("Transmitted  ", &fadd);
                printf("\n");
                //printf("line 522\n");
            } else {
                //printf("line 527\n");
                int currSwitchID = (frame->msg).pAsk.switchID;
                int actionval = 1;  // initially set to port 1
                if (currSwitchID < sArray[switchIndex].switchID) {
                    actionval = 2; // destSwitchID > currID so relay to port 2
                }
                msg = composeADDmsg(sArray[switchIndex].lowIP,sArray[switchIndex].highIP, FORWARD, actionval, 
                sArray[switchIndex].switchID, (frame->msg).pAsk.switchID, (frame->msg).pAsk.scrIP, (frame->msg).pAsk.destIP); 
                sendFrame(fd, ADD, &msg);
                FRAME fadd;
                fadd.kind = ADD;
                fadd.msg = msg;
                printFrame("Transmitted  ", &fadd);
                printf("\n");
            }
            master->askCount +=1;
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

/* 
parse and respond to incoming packages from switch's perspective
arguments: 
    -currSwitchID: the 'i' in pswi
    -frame: package received
    -fds: 2d array of fds
    -pSwitch: struc that represent pswi
*/
void parseSwitchMSG(int currSwitchID, FRAME * frame,int fds[MAX_SWITCH + 1][MAX_SWITCH + 1], SWITCH * pSwitch) {
    MSG msg;
    msg = frame->msg;
    MSG sendmsg;
    switch (frame->kind)
    {
    case ADD:
        {
            // add to forwarding table
            printf("\n");
            (pSwitch->nADDreceived) += 1;
            fTABLEROW rule = {
                /* scrIP_lo*/ 0,
                /* scrIP_hi */1000,
                /* destIP_lo*/ msg.pAdd.destIP_lo,
                /* destIP_hi*/ msg.pAdd.destIP_hi,
                /* actiontype*/msg.pAdd.ACTIONTYPE,
                /* actionVal*/ msg.pAdd.actionVAL,
                /* pktCount */ 0
            };
            forwardTable.push_back(rule);
            // apply header to new rule
            if (rule.ACTIONTYPE == DROP) {
                forwardTable[forwardTable.size() - 1].pktCount += 1;
            } else if (rule.ACTIONTYPE == FORWARD) {
                // relay to port 2, use  fds[i][i + 1] 
                // relay to port 1, use fds[i][i - 1] 
                // composeRelayMSg 
                if (rule.actionVAL == 2) {
                    assert(fds[currSwitchID][currSwitchID + 1] > 0);
                    sendmsg = composeRELAYmsg(frame, currSwitchID);
                    sendFrame(fds[currSwitchID][currSwitchID + 1], RELAY, &sendmsg);
                    //FRAME relayF;
                    //relayF.msg = sendmsg;
                    //relayF.kind = RELAY;
                    //printFrame("Transmitted ", &relayF);
                } else if (rule.actionVAL == 1) {
                    assert(fds[currSwitchID][currSwitchID - 1] > 0);
                    sendmsg = composeRELAYmsg(frame, currSwitchID);
                    sendFrame(fds[currSwitchID][currSwitchID - 1], RELAY, &sendmsg);
                    //FRAME relayF;
                    //relayF.msg = sendmsg;
                    //relayF.kind = RELAY;
                    //printFrame("Transmitted ", &relayF);
                }
                forwardTable[forwardTable.size() - 1].pktCount += 1;
                pSwitch->nRelayout += 1;
            }
            break;
        }
    case RELAY:
    {
        if (msg.pRelay.destSwitchID == currSwitchID) {
            // this is the switch that we can add rule to.
            //for (int i = 0; i < forwardTable.size(); i++) {
            //    if (msg.pRelay.destIP >= forwardTable[i].destIP_lo and msg.pRelay.destIP <= forwardTable[i].destIP_hi) {
            //        forwardTable[i].pktCount += 1;
            //        return;
            //    }
            //} 
        } // else we relay to whichever port of this switch TODO
        (pSwitch->nRELAYIN) += 1;
        break;
    } 
    default:
        break;
    }
}
// --------------------------------------------------------------------------------------
/* 
parse incoming header from port 3 of a switch
Argument: 
    -readbuff: a line from the file
    -swichID: the 'i' of pswi
    -fds: 2d array of fds
    -pswtich: struct that represent pswi
return:
    -1 if we had to send ASK for this header
    -2 if we applied rule to this header
    -0 otherwise
*/
int parseFileLine(char* readbuff, int switchID, int fds[8][8], SWITCH*pswitch) {
    if (readbuff[0] == '#') {
        //printf("# so skipp\n");
        return 0;
    }
    if (readbuff[0] == '\0') {
        //printf("emptyline, skipping\n");
        return 0;
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
        pswitch->admit += 1;
        // we want this line
        //printf("%s,%s,%s\n", tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str());
        //1. check the rule table 
        // 2. if dont exist, we send ASK. 
        //if (tokens[0])
        assert(forwardTable.size() > 0);
        assert(forwardTable[0].actionVAL == 3);
        //printf("forwardTable[0].ACtionTYPE=%d\n", forwardTable[0].ACTIONTYPE);
        assert(forwardTable[0].ACTIONTYPE == FORWARD);
        int srcIP = stoi(tokens[1]);
        int destIP = stoi(tokens[2]);
        for (int i = 0; i < forwardTable.size(); i++) {
            // assumes that lowip is within range
            if (forwardTable[i].destIP_lo <= destIP and destIP <= forwardTable[i].destIP_hi) {
                //printf("ine 669\n");
                forwardTable[i].pktCount += 1;
                if (forwardTable[i].ACTIONTYPE == FORWARD and forwardTable[i].actionVAL != 3) {
                    // case: if we have to relay this packet to diff switch
                    pswitch->nRelayout += 1;
                    // send relay?
                    MSG addmsg;
                    MSG relaymsg;
                    addmsg = composeADDmsg(srcIP, destIP, FORWARD, forwardTable[i].actionVAL, forwardTable[i].actionVAL, switchID, srcIP, destIP);
                    FRAME f1;
                    f1.msg = addmsg;
                    f1.kind = ADD;
                    relaymsg = composeRELAYmsg(&f1, switchID);
                    if (forwardTable[i].actionVAL == 1) { // relay to port 1
                        sendFrame(fds[switchID][switchID - 1], RELAY, &relaymsg);
                    } else if (forwardTable[i].actionVAL == 2) {  // relay to port 2
                        sendFrame(fds[switchID][switchID + 1], RELAY, &relaymsg);
                    }
                    // print statement
                }
                return 2;
            } 
        }
        // send ask
        //printf("ASK send\n");
        MSG msg;
        //printf("token1 [%s], token2[%s]\n", tokens[1].c_str(), tokens[2].c_str());
        msg = composeASKmsg(srcIP, destIP, switchID);
        sendFrame(fds[switchID][0], ASK, &msg);
        FRAME printASK;
        printASK.kind = ASK;
        printASK.msg = msg;
        printFrame("Transmitted  ", &printASK);
        return 1; 
        
    }
    if (tokens[0][3] == switchID_char and tokens[1] == "delay") {
        // handle delay packet
        int delay = stoi(tokens[2]);
        callTimer(delay);
        canRead = false;
        printf("\n");
        printf("**Entering a delay period of %d msec\n", delay);
        printf("\n");
    }
    return 0;
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
    //vector<SWITCH> sArray;
    printf("established file descriptors, waiting for HELLO\n");
    while (true) {
        //updateFDs();
        //doMasterPolling();
        int pollret = poll(pollfds, nswitch_ + 1, 1); // 
        //cout << "pollred " << pollret << endl;
        if (pollret < 0) {
            if (errno == EINTR) {  // casued by SIGALARM to interupt poll
                //cerr << "EINTR error while poll" << endl;
                continue;
            }
            cerr << "polling returned -1" << endl;
            exit(EXIT_FAILURE);
        }
        if (pollfds[0].revents and POLLIN) {  // keyboard poll
            memset(readbuff, 0, MAXWORD);
            int bytesread = read(pollfds[0].fd, readbuff, MAXWORD); // theres a \n character
            readbuff[strlen(readbuff) - 1] = '\0';  // clear \n character
            parseKeyboardMaster(readbuff);
        }
        for (int i = 1; i < nswitch_ + 1; i++) {
            if (pollfds[i].revents and POLLIN) {
                // something to read
                frame = rcvFrame(pollfds[i].fd, pollfds, i);
                if (pollfds[i].fd == -1) continue; // other end closed pipe so rcvFrame() changed fd to -1
                printFrame("recieved ", &frame); 
                parseAndSendToSwitch(fds[0][i], &frame, masterswitch, nullptr);
                pollfds[i].revents = 0;
            }
        }
    }
}

// SWITCH loop
void do_switch(SWITCH * pSwitch, int fds[MAX_SWITCH + 1][MAX_SWITCH + 1], const char* datafile) {
    // establish connection with master/pswj/pswk
    pollfd pollfds[SWITCHPORTS_N]; // = 5
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
        assert(fds[pSwitch->switchID][pSwitch->pswj] > 0);
        assert(fds[pSwitch->pswj][pSwitch->switchID] > 0);
        pollfds[1].fd = fds[pSwitch->pswj][pSwitch->switchID]; pollfds[1].events = POLLIN;
    } else {  // case: pswj is null
        //fds[pSwitch->pswj][pSwitch->switchID] = -1;  // this shares memory with nASKswitch BECUASE its fds[-1][i]
        pollfds[1].fd = -1;
    }
    if (pSwitch->pswk != -1) {
        fifo_i_k = getfifoName(pSwitch->switchID, pSwitch->pswk);
        fifo_k_i = getfifoName(pSwitch->pswk, pSwitch->switchID);
        fds[pSwitch->pswk][pSwitch->switchID] = openfifoRead(fifo_k_i);
        fds[pSwitch->switchID][pSwitch->pswk] = openfifoWrite(fifo_i_k);
        assert(fds[pSwitch->switchID][pSwitch->pswk] > 0);
        assert(fds[pSwitch->pswk][pSwitch->switchID] > 0);
        pollfds[2].fd = fds[pSwitch->pswk][pSwitch->switchID]; pollfds[2].events = POLLIN;
        
    } else {  // case: pswk is null
        //fds[pSwitch->pswk][pSwitch->switchID] = -1;
        pollfds[2].fd = -1;
    }
    fds[0][pSwitch->switchID] = openfifoRead(fifo_master_i);
    fds[pSwitch->switchID][0] = openfifoWrite(fifo_i_master);
    
    //pollfds[0] = master to pswi
    //pollfds[1]=port1,  pollfds[2]=port2, pollfds[4] = keyboard
    pollfds[0].fd = fds[0][pSwitch->switchID]; pollfds[0].events = POLLIN; 
    pollfds[3].fd = -1;
    pollfds[4].fd = STDIN_FILENO; pollfds[4].events = POLLIN; pollfds[4].revents = 0;
    char readbuff[MAXLINE];  // read from file
    char keyboardbuff[MAXLINE];  // read from stdin

    //vector<fTABLEROW> forwardTable;
    fTABLEROW initialRule = {
    /* scrIP_lo*/ 0,
    /* scrIP_hi */ MAXIP,
    /* destIP_lo*/ pSwitch->lowIP,
    /* destIP_hi*/ pSwitch->highIP,
    /* actiontype*/FORWARD,
    /* actionVal*/ 3,
    /* pktCount */ 0
    };
    forwardTable.push_back(initialRule);
    FRAME frame;
    MSG msg;
    // send HELLO
    msg = composeHELLOmsg(pSwitch->switchID, 0, pSwitch->lowIP, pSwitch->highIP, pSwitch->pswj, pSwitch->pswk);
    sendFrame(fds[pSwitch->switchID][0], HELLO, &msg);
    // print 
    FRAME printHel;
    printHel.kind = HELLO;
    printHel.msg = msg;
    printFrame("Transmitted ", &printHel);
    // get ACK from master first
    int ackowledge = getACK(pollfds);
    assert(ackowledge == 1); // assert its ackolowdged
    printf("\n");
    (pSwitch->nHELLOtransm) += 1;
    (pSwitch->nACKreceived) += 1;
    assert(pSwitch->nACKreceived > 0);
    assert(pSwitch->nHELLOtransm > 0);
    // open DATAFILE
    FILE *fp;
    char* result;
    string datafileSTR = "./" + string(datafile);
    fp = fopen(datafileSTR.c_str(), "r");
    if (fp == NULL) {
        perror("fopen FAILED: ");
    }

    bool EOFreached = false;  // EOF of file
    bool ADDreceived = true;  // flag so that we only read more line from file if ADD is received.
    while (true) {
        memset(readbuff, 0, MAXLINE);
        if (ADDreceived and canRead) { // only read more line if ADD received
            
            if(fgets(readbuff, MAXLINE, (FILE*) fp) != NULL) {
                if (readbuff[strlen(readbuff) - 1] == '\n') {
                    readbuff[strlen(readbuff) - 1] = '\0';  // clear \n char 
                }
                int ret = parseFileLine(readbuff, pSwitch->switchID, fds, pSwitch);
                if (ret == 1) {  // means that we sent ASK
                    ADDreceived = false;
                    pSwitch->nASKtrans += 1;
                } 
            } else {
                printf("EOF REACHED\n");
                EOFreached = true;
                ADDreceived = false; // so we dont read from file anymore
            }
        }
        // ** POLLING FROM MASTER, NEIGHBOR, AND KEYBOARD
        int pollret = poll(pollfds, SWITCHPORTS_N, 1);
        if (pollret < 0) {
            if (errno == EINTR) {  // casued by SIGALARM/SIGUSR1 to interupt poll
                //cerr << "EINTR error while poll" << endl;
                continue;
            }
            cerr << "pollret returned -1" << endl;
            exit(EXIT_FAILURE);
        }
        // **POLL keyboard
        if (pollfds[4].revents and POLLIN) {
            memset(keyboardbuff, 0, MAXLINE);
            int bytesread = read(pollfds[4].fd, keyboardbuff, MAXLINE);
            keyboardbuff[strlen(keyboardbuff) - 1] = '\0'; // clear \n
            assert(bytesread > 0);
            parseKeyboardSwitch(keyboardbuff);
        } 
        // POLL everything else 
        for (int i = 0; i < SWITCHPORTS_N - 2; i++) {  // check everything exept keyboard[0 - 3] and port 3
            if (pollfds[i].revents and POLLIN) {
                frame = rcvFrame(pollfds[i].fd, pollfds, i);
                // rcvFrame make pollfds[i].fd = -1 if that fd is closed by other side
                if (pollfds[i].fd == -1) continue; 
                printFrame("received ", &frame);  
                if (frame.kind == ADD) ADDreceived = true;
                parseSwitchMSG(pSwitch->switchID, &frame, fds, pSwitch);
            }
        }
    } 
}
// ----------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    printf("PID=%d\n", getpid());  //pid of process incase you needed
    char tokens[10][MAXWORD];
    int fds[MAX_SWITCH + 1][MAX_SWITCH + 1]; //fds[i][j] means fd for fifo-i-j
    
    SWITCH pSwitch;
    MASTERSWITCH master;
    
    // install signal handler
    if (signal(SIGALRM, (void (*)(int))timerHandler) == SIG_ERR) {
        perror("Unable to catch SIGALARM");
        exit(1);
    }
    if (signal(SIGUSR1, (void (*)(int))USR1handler) == SIG_ERR) {
        perror("Unable to catch SIGUSR1");
        exit(1);
    }

    if (argc == 3 and strcmp(argv[1], "master") == 0) {
        // master switch
        isMaster = true;
        for (int i = 1; i < 3; i++) {
            strcpy(tokens[i - 1], argv[i]);
            // tokens[0] = "master"
            // tokens[1] = "nSwitch"
        }
        populateMaster(&globalMaster, tokens);
        do_master(&globalMaster, fds);
    } else if (argc == 6) {  // SWTICH PERSPECTIVE
        isSwitch = true;
        for (int i = 1; i < 6; i++) {
            memset(tokens[i - 1], 0, MAXWORD); 
            strcpy(tokens[i - 1], argv[i]);
            // tokens[0] = "pswi"
            // tokens[1] = "datafile"
            // tokens[2] = "null//pswj"
            // tokens[3] = "null/pswk"
            // tokens[4] = "IPlow-IPhigh"
        }
        // if we are switch switch array stores only struct for pswi
        sArray.push_back(pSwitch); 
        populateSwitch(&sArray[0], tokens);
        do_switch(&sArray[0], fds, tokens[1]);
    } else {
        printf("invalid arguments\n");
        return 0;
    }
    
    return 0;
}