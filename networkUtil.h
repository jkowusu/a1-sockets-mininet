#include <arpa/inet.h>	// ntohs()
#include <netdb.h>		// gethostbyname(), struct hostent
#include <netinet/in.h>	// struct sockaddr_in
#include <stdio.h>		// printf()
#include <string.h>		// memcpy()
#include <iostream>
#include <chrono>
#include <string>
#include <sys/socket.h>	// socket(), bind(), listen(), accept()
#include <netinet/in.h>
#include <unistd.h>		// close()
#include <cassert>
#include <iomanip>  // Include for std::setprecision
using namespace std;


const int FIN_MESSGE_SIZE = 1;
const int ACK_MESSGE_SIZE = 1;
const int BUFFER_SIZE = 1000;
const char FIN_VALUE = '5';
const char ACK_VALUE = '6';


/**
 * Make a client sockaddr given a remote hostname and port.
 * Parameters:
 *		addr: The sockaddr to modify (this is a C-style function).
 *		hostname: The hostname of the remote host to connect to.
 *		port: The port to use to connect to the remote hostname.
 * Returns:
 *		0 on success, -1 on failure.
 * Example:
 *		struct sockaddr_in client;
 *		int err = make_client_sockaddr(&client, "141.88.27.42", 8888);
 */

int makeClientSockaddr(struct sockaddr_in *addr, const char *hostname, int port) {
	// Step (1): specify socket family.
	// This is an internet socket.
	addr->sin_family = AF_INET;

	// Step (2): specify socket address (hostname).
	// The socket will be a client, so call this unix helper function
	// to convert a hostname string to a useable `hostent` struct.
	struct hostent *host = gethostbyname(hostname);
	if (host == nullptr) {
		printf("Error: Unknown host %s\n", hostname);
		return -1;
	}
	memcpy(&(addr->sin_addr), host->h_addr, host->h_length);

	// Step (3): Set the port value.
	// Use htons to convert from local byte order to network byte order.
	addr->sin_port = htons(port);

	return 0;
}

/**
 * Make a server sockaddr given a port.
 * Parameters:
 *		addr: The sockaddr to modify (this is a C-style function).
 *		port: The port on which to listen for incoming connections.
 * Returns:
 *		0 on success, -1 on failure.
 * Example:
 *		struct sockaddr_in server;
 *		int err = make_server_sockaddr(&server, 8888);
 */
int makeServerSockaddr(struct sockaddr_in *addr, int port){
    // Step (1): specify socket family.
	// This is an internet socket.
	addr->sin_family = AF_INET;

	// Step (2): specify socket address (hostname).
	// The socket will be a server, so it will only be listening.
	// Let the OS map it to the correct address.
	addr->sin_addr.s_addr = INADDR_ANY;

	// Step (3): Set the port value.
	// If port is 0, the OS will choose the port for us.
	// local byte order to network byte order.
	addr->sin_port = htons(port);

    return 0;

}

void sendMessage(int socket, char* clientArgs[]){
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);

    // Get the start time using std::chrono
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Calculate the end time based on the specified duration in seconds
    auto duration = std::chrono::seconds(stoi(clientArgs[7]));

    long long dataSentSize = 0;

    // Make sure time elapsed now and the start time is less than specified duration
     do {
        // Send 1000-byte chunks of data
        ssize_t bytes_sent = send(socket, buffer, BUFFER_SIZE, 0);
        if (bytes_sent == -1) {
            perror("Error sending data");
            break;
        }

        // Track the total amount of data sent
        dataSentSize += bytes_sent;

        } while (std::chrono::high_resolution_clock::now() - startTime < duration);

        //After the client finishes sending its data, it should send a FIN message 
        char fin[] = {FIN_VALUE};
        ssize_t bytes_sent = send(socket, fin, FIN_MESSGE_SIZE, 0);
        if (bytes_sent == -1) {
            perror("Error sending fin message");
        }

        shutdown(socket, SHUT_WR);

        //and wait for an acknowledgement before exiting the program.
        char recBuffer[1];
        int bytesReceived = 0;

    
        bytesReceived = recv(socket, recBuffer, 1, MSG_WAITALL);

    
        if (bytesReceived == -1) {
            perror("Error receiving acknowledgment");
        } else if (bytesReceived == 0) {
            std::cout << "Connection closed by server before acknowledgment\n";
        } else {
            assert(recBuffer[0] == ACK_VALUE);
        }

            // Get the end time after receiving the acknowledgment
        auto endTime = std::chrono::high_resolution_clock::now();

        // Calculate the actual time elapsed from start to finish (in seconds)
        std::chrono::duration<double> elapsedTime = endTime - startTime;

        // Calculate the rate in Megabits per second (Mbps)
        // Data sent is in bytes, multiply by 8 to get bits, then divide by 1,000,000 to convert to megabits
        double rate = (dataSentSize * 8.0) / (elapsedTime.count() * 1000000.0);  // Mbps

        // Output the total data sent in kilobytes (KB) and the rate in Mbps
        std::cout << "Sent=" << (dataSentSize / 1000) 
              << " KB, Rate=" << std::fixed << std::setprecision(3) << rate << " Mbps\n";

    return ;
}

void recvMessages(int socket) {
    char buffer[BUFFER_SIZE];
    long long totalBytesReceived = 0;

    // Get the start time when the server starts receiving data
    auto startTime = std::chrono::high_resolution_clock::now();

    while (true) {
        // Receive data from the client
        ssize_t bytesReceived = recv(socket, buffer, BUFFER_SIZE, MSG_WAITALL);

        if (bytesReceived == -1) {
            perror("Error receiving message");
            break;
        } else if (bytesReceived == 0) {
            // Client closed the connection
            cout << "Connection closed by client\n";
            break;
        }

        // Accumulate the total bytes received
        totalBytesReceived += bytesReceived;

        // Check if the 'FIN' message was received (assuming '5' is the FIN message)
        if (bytesReceived == FIN_MESSGE_SIZE && buffer[0] == FIN_VALUE) {
            break;
        }
    }

    // Get the end time after receiving the FIN message
    auto endTime = std::chrono::high_resolution_clock::now();

    // Send an acknowledgment to the client
    char ack[] = {ACK_VALUE};  // Send ACK message
    ssize_t bytes_sent = send(socket, ack, ACK_MESSGE_SIZE, 0);
    if (bytes_sent == -1) {
        perror("Error sending ack message");
    }

    // Calculate the elapsed time in seconds
    std::chrono::duration<double> elapsedTime = endTime - startTime;


    // Calculate the rate in Megabits per second (Mbps)
    // (totalBytesReceived * 8) converts bytes to bits, then divide by 1,000,000 to get megabits
    double rateMbps = (totalBytesReceived * 8.0) / (elapsedTime.count() * 1000000.0);

    // Print the output in the required format
    std::cout << "Received=" << totalBytesReceived / 1000 
              << " KB, Rate=" << std::fixed << std::setprecision(3) 
              << rateMbps << " Mbps\n";

    return;
}