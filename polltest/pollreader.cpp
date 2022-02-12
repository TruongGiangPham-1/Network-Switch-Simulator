#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <string.h>
#include <poll.h>

using namespace std;


#define N 1
// POLL READER


struct packet {
    char msg[50];
    int type;
};


#define MSG_SIZE 1024


int main() {
   cout << "Reader started..." << endl << flush;

   const char * pipe_pathname = "./pipe";
   
   int fd = open(pipe_pathname, O_RDONLY); // oflag for read-only access
   if (fd == -1) {
      cout << "Pipe open failed" << endl << flush;
      exit(1);
   }
   cout << "Named pipe opened for reading..." << endl << flush;

   char line[MSG_SIZE] = {0};

   struct packet got;


   int bytes_read;
   /* reads up to MSG_SIZE bytes from fd into the buffer */
   //bytes_read = read(fd, line, MSG_SIZE);
   bytes_read = read(fd, &got, sizeof(struct packet));

   cout << bytes_read << " bytes are read from the pipe: " << line <<  endl;
   printf("packet id %d and packet msg %s\n", got.type, got.msg);
   //bytes_read = read(fd, line, MSG_SIZE);
   //cout << bytes_read << " bytes are read from the pipe: " << line <<  endl;
   cout << "Finished reading bytes from the pipe..." << endl << flush;

   /* writes up to MSG_SIZE bytes from the buffer to fd */
   // write(fd, first, MSG_SIZE);   	
   // write(fd, second, MSG_SIZE);

   // close pipe from the write end
   close(fd);
   
   return 0;
}
