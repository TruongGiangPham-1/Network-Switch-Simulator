#include <iostream>
#include <cstring>		// strlen

#include <unistd.h>		// read/write lib functions
#include <fcntl.h>		// open and oflags

#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

#define MAX_SIZE 1024

struct packet {
    char msg[50];
    int packetType;
};
int main() {
	const char * inpipe = "pipe2";
    const char * outpipe = "pipe1";

    /*	
    	open as write-only; a file descriptor that refers to 
    	the open file description is returned 
    */
    int ifd = open(inpipe, O_RDONLY);
    if (ifd == -1) {
        cerr << "Error: cannot open the named pipe." << endl;
        
        // reclaim the backing file
        unlink(inpipe);
        return 1;
    }

    int ofd = open(outpipe, O_WRONLY);
    if (ofd == -1) {
        cerr << "Error: cannot open the named pipe." << endl;
        
        // reclaim the backing file
        unlink(outpipe);
        return 1;
    }


    char buffer[MAX_SIZE];
    char ack[] = "RECEIVED";
    struct packet reaceivedPack;
    struct packet ACK;
    cout << "Reader started..." << endl;
    while (true) {
        memset(&reaceivedPack, 0, sizeof(reaceivedPack));
        int bytesread = read(ifd, &reaceivedPack, sizeof(struct packet));
        if (bytesread == -1)
            cerr << "Error: read operation failed!" << endl;
        if (strcmp("Quit", reaceivedPack.msg) == 0)
            break;
        printf("reaceived: %s packettype: %d\n", reaceivedPack.msg, reaceivedPack.packetType);
        //cout << "received" << reaceivedPack.msg << endl;
        
        memset(&ACK, 0, sizeof(ACK));
        strcpy(ACK.msg, "RECEIVED");
        write(ofd, &ACK, sizeof(struct packet));
        // sleep(10);
    }

    // close both file descriptors
    close(ifd);
    close(ofd);

	return 0;
}