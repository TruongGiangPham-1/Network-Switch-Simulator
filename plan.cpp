// planinng the system
struct Packet {
    int senderID; // if psw1 send it, senderID == 1
    char msg[50];
    int packetType;
    
};


// a2w22 master 2
// a2w22 psw1 ex2.dat null psw2 100-110
// a2w22 psw2 ex2.dat psw1 null 200-210


// this is for PSW1 ------------------------------
struct pollfd pfds[4];

pfds[0].fd = STDIN_FILENO 
pfds[1].fd = fifo[0][1]; // fifo from master to psw1
pfds[2].fd = fifo[2][1]; // fifo from psw2 to psw1
pfds[3].fd = NULL        
struct psw1 {
        
};

while (true) {
    poll();
    if (pfds[0] and POLLIN) // poll keyboard
    
    // poll the rest
    for (int i = 1; i < 4; i++) {
        if (pfds[i].fd and POLLIN) {
            // do something
        } else {
            continue;
        }
    }
}
// ---------------------------------------------------
// this is for psw2

struct pollfd pfds[4];

pfds[0].fd = STDIN_FILENO 
pfds[1].fd = fifo[0][2]; // fifo from master to psw1
pfds[2].fd = fifo[1][2]; // fifo from psw1 to psw2
pfds[3].fd = NULL        
struct psw2 {
        
};


HELLO {
   // 1. the switch number
   // the numbers of neighbouring switches
   // the range of IP address
   int switchNum;
   int Nneighbor;
   int lowIP;
   int highIP;
} HELLOPACK;

struct HELLO_ACK_PACK {
    // no carry package;
    int nothing;
} HELLO_ACK_PACK;


struct {
    // does stuff
} ASK_PACK;


struct {

} ADD_PACK;

struct {

} RELAY;

