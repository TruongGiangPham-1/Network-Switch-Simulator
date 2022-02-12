#include <iostream>
#include <cstring>		// strlen, strcmp
#include <unistd.h>		// read, write, close
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
	const char * inpipe = "pipe1";
    const char * outpipe = "pipe2";

	// create a backing fifo file
	if (mkfifo(inpipe, 0666) == -1) {
        cerr << "Error: fifo creation failed." << endl;
        return 1;
    }
    // create a backing fifo file
    if (mkfifo(outpipe, 0666) == -1) {
        cerr << "Error: fifo creation failed." << endl;
        return 1;
    }

    /*	
    	open as write-only; a file descriptor that refers to 
    	the file description table is returned 
    */
    int ofd = open(outpipe, O_WRONLY);
    if (ofd == -1) {
        cerr << "Error: cannot open the named pipe." << endl;
        
        // reclaim the backing files
        unlink(inpipe);
        unlink(outpipe);
        return 1;
    }

    /*  
        open as read-only; a file descriptor that refers to 
        the file description table is returned 
    */
    int ifd = open(inpipe, O_RDONLY);
    if (ifd == -1) {
        cerr << "Error: cannot open the named pipe." << endl;
        
        // reclaim the backing files
        unlink(inpipe);
        unlink(outpipe);
        return 1;
    }

    char writerbuf[MAX_SIZE] = {0};
    char readerbuf[MAX_SIZE] = {0};

	cout << "Writer started..." << endl;
    cout << "Enter the first line of your message" << endl;
    struct packet sendPack;
    struct packet readPack;
    while (true) {
    	cin.getline(writerbuf, MAX_SIZE);
        if (strcmp(writerbuf, "hello") == 0) {
            memset(&sendPack, 0, sizeof(sendPack)); // clea
            strcpy(sendPack.msg, "hello");
            sendPack.packetType = 1;
        } else {
            memset(&sendPack, 0, sizeof(sendPack)); // clea
            strcpy(sendPack.msg, "something else");
            sendPack.packetType = 2;
        }

        // getline stores a null character into the next available location in the buffer array
    	if (write(ofd, &sendPack,sizeof(struct packet)) == -1)
    		cerr << "Error: write operation failed!" << endl;
    	if (strcmp("Quit", writerbuf) == 0) // if user entered Quit we quit
    		break;
        // read from reader
        //memset them
        memset(&readPack, 0, sizeof(readPack));
        int bytesread = read(ifd, &readPack, sizeof(struct packet));
        if (bytesread == -1)
            cerr << "Error: read operation failed!" << endl;
        if (strcmp("RECEIVED",readPack.msg) != 0) {
            cerr << "Error: message corrupted!" << endl;
            break;
        }
        memset(&sendPack, 0, sizeof(sendPack));
        cout << "Enter the next line" << endl;
    }

    // close the file descriptors
    close(ifd);
    close(ofd);

    // reclaim the backing files
    unlink(inpipe);
    unlink(outpipe);

	return 0;
}