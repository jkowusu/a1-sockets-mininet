#include <iostream>
#include <chrono>
#include <string>
#include <sys/socket.h>	// socket(), bind(), listen(), accept()
#include <netinet/in.h>
#include <unistd.h>		// close()
#include "networkUtil.h"
#include <cassert>
#include <iomanip>  // Include for std::setprecision


using namespace std;

int CLIENT_MODE_ARGS = 8;

int SERVER_MODE_ARGS = 4;


void validatePortNumber(int portNumber){
    if ( portNumber < 1024 || portNumber > 65535){
        cout << "Error: port number must be in the range of [1024, 65535]\n";
        exit(1);
    }
}
/*
 * Error Checking for Client Mode
 * Checks Number of Arguments
 * Ensures port number is within the valid range [1024, 65535].
 * Makes sure time arg is greater than 0
 */
void validateClientMode(int argc , char* argv[]){
    //Error Check
        if (argc != CLIENT_MODE_ARGS){
            cout << "Error: missing or extra arguments\n";
            exit(1);
        }
        validatePortNumber(stoi(argv[5]));

        if (stoi(argv[7]) <= 0){
            cout << "Error: time argument must be greater than 0\n";
            exit(1);

        }
}

/*
 * Error Checking for Server Mode
 * Checks Number of Arguments
 * Ensures port number is within the valid range [1024, 65535].
 */
void validateServerMode(int argc , char* argv[]){
    
        if (argc != SERVER_MODE_ARGS){
            cout << "Error: missing or extra arguments\n";
            exit(1);
        }
        validatePortNumber(stoi(argv[3]));

       
}
void runClientMode(char* clientArgs[]){

// Create a socket.
int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if (sockfd == -1){
    cout << "Error opening stream socket\n";
}

// Connect to a server.
struct sockaddr_in addr;

//Passing in IP Address (hostname) and port
makeClientSockaddr(&addr,clientArgs[3],stoi(clientArgs[5]));

if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1){
    perror("Could not connect socket");
	exit(1);
}

// Send the information 
sendMessage(sockfd, clientArgs);


// Close the connection.
int close_retval = close(sockfd);
assert(close_retval == 0);
}

void runServerMode(char* serverArgs[]){

    //Make a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1){
    cout << "Error opening stream socket\n";
}

// Set SO_REUSEADDR option so ports can be resused
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        cout << "Error setting SO_REUSEADDR\n";
        return;
    }

    //Bind socket to port
    struct sockaddr_in addr;
    makeServerSockaddr(&addr, stoi(serverArgs[3]));

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        cout << "Error binding socket\n";
        return;
    }

    // Listen for connections
    if (listen(sockfd, 10) == -1) {
        cout << "Error listening on socket\n";
        return;
    }

    //Accept Connection
    socklen_t addrSize = sizeof(addr);
    int connectedSocket = accept(sockfd, (struct sockaddr *) &addr, &addrSize);
    if (connectedSocket == -1){
    cout << "Error opening connected socket\n";
}

    // Recv Messages
    recvMessages(connectedSocket);

    // Close the connection.
    int close_retval = close(sockfd);
    assert(close_retval == 0);

}

int main( int argc , char* argv[]) {

    //Client Mode
    if (string(argv[1]) == "-c"){
        validateClientMode(argc,argv);
        runClientMode(argv);
    }

    //Server mode
    if (string(argv[1]) == "-s"){
        validateServerMode(argc,argv);
        runServerMode(argv);
    }

    return 0;
}
