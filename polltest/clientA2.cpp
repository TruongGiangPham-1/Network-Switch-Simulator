//------------------------------------------------
// Group members: Truong-Giang Pham, Weiguo Jiang
// Student ID: 1662405
// CMPUT275, Winter 2021
// Assignment 1: Navigation Systems (Teams)
//------------------------------------------------

#include <iostream>			// std::cin, std::cout, std::cerr
#include <sys/types.h>		// include for portability
#include <sys/socket.h>		// socket, connect
#include <arpa/inet.h>		// inet_addr, htonl, htons
#include <unistd.h>			// close
#include <cstring>			// strlen, strcmp
#include <fcntl.h>          // open and oflag
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unordered_set>

#define BUFFER_SIZE 1024
// #define SERVER_PORT 8888
// #define SERVER_IP "127.0.0.1"

using namespace std;

void CharToLL(char* char_arr, long long &latitude_LL, long long &longitude_LL) {
	/*
		Description: Given char array of longitude, follow by latitude, convert it to long long

		Arguments:
			char_arr(char*): pointer to the start of the array, i.e., array name.
			latitude_LL(long long &): reference to the latitude variable, will be
			assigned value after the char array is parsed.
			longitude_LL(long long &): reference to the longitude variable, will be
			assigned value after the char array is parsed.

		Returns: None
	*/
	string lat = "";
	string lon = "";
	// index of space character
	int space = 0;
	for (int i = 0; i < strlen(char_arr); i++) {
		if (char_arr[i] == ' ') {
			space = i;
			break;
		} else {
			lat += char_arr[i];
		}
	}
	// - 1 to exlude \n
	for (int i = space + 1; i < strlen(char_arr); i++) {
		lon += char_arr[i];
	}
	latitude_LL = static_cast<long long>(stod(lat) * 100000);
	longitude_LL = static_cast<long long>(stod(lon) * 100000);
	return;

}

void LongTochar(char* buffer, long long &lat, long long& lon) {
	/*
		Description: create c string that represent a lat/long coord

		Arguments:
			buffer: a buffer of characters used to store the format of
			"latitude longitude"
			lat(long long &): reference to the latitude of type long long
			that we are going to put it into the char array
			lon(long long &): reference to the longitude of type long long
			that we are going to put it into the char array

		Returns: None
	*/
    string buffer_stringform = to_string(lat) + " " + to_string(lon);
    strcpy(buffer, buffer_stringform.c_str());
}

void convertToFinalBuffer(char* finalbuffer, char* startBuffer, char* endBuffer) {
	/*
		Description: given two arrays storing start and endpoints' cooridinates,
		we convert it into the format "R startpoint endpoint".

		Arguments:
			finalbuffer(char*): pointer to the start of the char array, the final
			format will be stored here
			startBuffer(char*): char array containing the startpoint coordinates
			endBuffer(char*): char array containing the endpoint coordinates

		Returns: None
	*/
    string totalbuffer = "R ";
    string start(startBuffer);
    string end(endBuffer);
    totalbuffer  += start + " " + end;
    memset(finalbuffer, 0, BUFFER_SIZE);
    strcpy(finalbuffer, totalbuffer.c_str());
}

int main(int argc, char const *argv[]) {
	if (argc != 3) {
        cout << "This program takes two command line arguments" << endl;
        return 0;
    }
    int SERVER_PORT = atoi(argv[1]);
    const char* SERVER_IP = argv[2];
	// client now has to read from inpipe
	const char* inpipe = "inpipe";
	const char* outpipe = "outpipe";
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
	int ifd = open(inpipe, O_RDONLY);
	if (ifd == -1) {
		cerr << "Error: cannot open the named pipe." << endl;
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
	char end[] = "E\n";
	char readBuffer[BUFFER_SIZE] = {0};
	// will contains "R startpoint endpoint"
    char finalbuffer[BUFFER_SIZE] = {};
	char NO[] = "N \n";
	char quit[] = "Q\n";
	char ACK[] = "A";
	// avoid sending duplicate points to plotter when packet drop happens
    unordered_set<string> dupwaypointSet;
	// Declare structure variables that store local and peer socket addresses
	// sockaddr_in is the address sturcture used for IPv4
	// sockaddr is the protocol independent address structure
	struct sockaddr_in my_addr, peer_addr;
	// zero out the structor variable because it has an unused part
	memset(&my_addr, '\0', sizeof my_addr);
	// Declare socket descriptor
	int socket_desc;
	char outbound[BUFFER_SIZE] = {};
	char inbound[BUFFER_SIZE] = {};
	/*
		socket() input arguments are:
		socket domain (AF_INET):	IPv4 Internet protocols
		socket type (SOCK_STREAM):	sequenced, reliable, two-way, connection-based
									byte streams
		socket protocol (0): 		OS selects a protocol that supports the requested
							 		socket type (in this case: IPPROTO_TCP)
		socket() returns a socket descriptor
	*/
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) {
		std::cerr << "Listening socket creation failed!\n";
		return 1;
	}
	// Prepare sockaddr_in structure variable
	peer_addr.sin_family = AF_INET;							// address family (2 bytes)
	peer_addr.sin_port = htons(SERVER_PORT);				// port in network byte order (2 bytes)
															// htons takes care of host-order to short network-order conversion.

	// peer_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);		// internet address (4 bytes). INADDR_LOOPBACK is localhost address
	peer_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

															// inet_addr converts an IPv4 in dotted decimal notation to a 32-bit integer value.
															// htons takes care of host-order to long network-order conversion.
	if (connect(socket_desc, (struct sockaddr *) &peer_addr, sizeof peer_addr) == -1) {
		std::cerr << "Cannot connect to the host!\n";
		close(socket_desc);
		return 1;
	}
	// will be used later as a flag when resetting the state of
    // the server due to either message corruption or timeout, it avoids reading
    // again from the inpipe after breaking the inner while loop
    bool flag = true;
	std::cout << "Connection established with " << inet_ntoa(peer_addr.sin_addr) << ":" << ntohs(peer_addr.sin_port) << "\n";
    while (true) {
    	// each iteration of the while loop,
    	// 1. take in coordinate
    	// 2, send coordinate to server
    	// 3. receive waypoints from server
    	// 4. end 2nd while loop when receive 'E'
    	char startBuffer[BUFFER_SIZE] = {};
    	char endBuffer[BUFFER_SIZE] = {};
    	if (flag) {
    		for (int i = 0; i < 2; i++) {
    			int bytesread = read(ifd, readBuffer, 22);
    			if (bytesread == -1)
    				cerr << "Error: read operation failed!" << endl;
    			// do the clean up if plotter is closed, i.e., sends Q
    			if (readBuffer[0] == 'Q') {
    				send(socket_desc, quit, strlen(quit) + 1, 0);
    				close(socket_desc);
    				close(ifd);
    				close(ofd);
    				unlink(inpipe);
    				unlink(outpipe);
    				return 0;
    			} else {
    				// process startpoint
 					if (i == 0) {
 						long long lat, lon;
 						CharToLL(readBuffer, lat, lon);
 						LongTochar(startBuffer, lat, lon);
 						// process endpoint
 					} else {
 						long long latE, lonE;
 						CharToLL(readBuffer, latE, lonE);
 						LongTochar(endBuffer, latE, lonE);
 					}
    			}
    		}
            // make final buffer to final form to send to server
    		convertToFinalBuffer(finalbuffer, startBuffer, endBuffer);
    		send(socket_desc, finalbuffer, strlen(finalbuffer) + 1, 0);
            memset(readBuffer, 0, BUFFER_SIZE);
   		}
    	// set the timer and socket options
    	struct timeval timer = {.tv_sec = 1, .tv_usec = 10000};
	    if (setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, (void *) &timer, sizeof(timer)) == -1) {
			std::cerr << "Cannot set socket options!\n";
			close(socket_desc);
			return 1;
		}
    	// timeout and invalid message handling
    	while (true) {
    		int rec_waypointnum = recv(socket_desc, inbound, BUFFER_SIZE, 0);
    		if (rec_waypointnum == -1) {
    			// resend request
    			send(socket_desc, finalbuffer, strlen(finalbuffer) + 1, 0);
                memset(inbound, 0, BUFFER_SIZE);
    		} else if (inbound[0] != 'N') {
    			send(socket_desc, finalbuffer, strlen(finalbuffer) + 1, 0);
                memset(inbound, 0, BUFFER_SIZE);
    		} else {
    			break;
    		}
    	}
    	// no path
    	if (inbound[2] == '0' || inbound == "N 1") {
    		write(ofd, end, strlen(end));
            memset(inbound, 0, BUFFER_SIZE);
    		continue;
    	}
    	while (true) {
    		int rec_size = recv(socket_desc, inbound, BUFFER_SIZE, 0);
    		// reset state if no waypoints are sent back
    		if (rec_size == -1) {
    			send(socket_desc, finalbuffer, strlen(finalbuffer) + 1, 0);
    			flag = false;
    			break;
    		}
    		if (inbound[0] == 'E') {
    			write(ofd, inbound, strlen(inbound));
                flag = true;
                memset(inbound, 0, BUFFER_SIZE);
                // can safety clear the set here since server now successfully sent waypoint without delay
                dupwaypointSet.clear();
    			break;
    		}
            // at this point only write if waypoint string is not in set
            string waypointForSet(inbound);
            if (dupwaypointSet.find(waypointForSet) == dupwaypointSet.end()) {
                char client_waypoint_char[BUFFER_SIZE] = {};
                // i starts at 2 since we receive 'W waypoint'
                for (int i = 2; i < strlen(inbound) + 2; i++) {
                    client_waypoint_char[i - 2] = inbound[i];
                }
                client_waypoint_char[strlen(inbound) - 2] = '\0';
                write(ofd, client_waypoint_char, strlen(client_waypoint_char));
                // send acknowledgement
                send(socket_desc, ACK, strlen(ACK) + 1, 0);
                // we can safely start the next loop
                flag = true;
                memset(inbound, 0, BUFFER_SIZE);
                // insert to set so that next time we encounter
                // this cooridinate, we won't send it to plotter
                dupwaypointSet.insert(waypointForSet);
            } else {
                // dont writing
                // but still send ack so sever can send more W
                send(socket_desc, ACK, strlen(ACK) + 1, 0);
                flag = true;
            }
    	}
    }
    close(socket_desc);
    close(ifd);
    close(ofd);
    unlink(inpipe);
    unlink(outpipe);
	return 0;
}
