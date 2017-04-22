#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h> 
#include <vector>

#include <string>
#include <thread>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <fstream>

// server is called with the following parameters
// server <PORT> <FILE-DIR>

bool SIG_HANDLER_CALLED = 0;

void signal_handler(int signum)
{
	SIG_HANDLER_CALLED = 1;
	std::cout << "Signal handler called" << std::endl;
	// Exit zero when SIGQUIT or SIGTERM received
	//exit(0);
}

void handle_thread(struct sockaddr_in clientAddr, int clientSockfd, int connection_number, std::string file_dir)
{
	char ipstr[INET_ADDRSTRLEN] = {'\0'};

	inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
	std::cout << "Accept a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;

	connection_number++;
	std::string save_name = file_dir + "/" + std::to_string(connection_number) + ".file";

	std::ofstream new_file;
	new_file.open(save_name, std::ios::out);
	char receive_buf[1500];
	int file_size = 0;
	int rc = 0;

    /* get the file name from the client */
    while( (rc = recv(clientSockfd, receive_buf, sizeof(receive_buf), 0)) > 0)
    {
	    if (rc == -1) {
			fprintf(stderr, "recv failed: %s\n", strerror(errno));
			exit(1);
	    }
	    new_file.write(receive_buf, rc);
	    file_size += rc;
	}
	std::cout << "Received file of " << file_size << " bytes\n";
	file_size = 0;
	new_file.close();
}


int main(int argc, char* argv[])
{
	if (argc < 3) {
      	std::cerr << "ERROR: server must be called with exactly two parameters: <PORT> <FILE-DIR>" << std::endl;
      	exit(-1);
   	}

   	// Handle signals
   	struct sigaction act;
   	act.sa_handler = &signal_handler;
   	sigaction(SIGQUIT, &act, NULL);
   	sigaction(SIGTERM, &act, NULL);
   	// FOR TESTING PURPOSES: Ctrl-C
   	sigaction(SIGINT, &act, NULL);

	
   	int port_num = atoi(argv[1]);
   	std::string file_dir = argv[2];
   	struct addrinfo hints, *res;
    int status;

   	// Check that the port number is in range
   	if (port_num < 1024 || port_num > 65535) {
	    std::cerr << "ERROR: Port number out of range [1024 - 65535]" << std::endl;
	    exit(1);
	}


	memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE; 
    hints.ai_family = AF_INET; // AF_INET specifies IPv4
    hints.ai_socktype = SOCK_STREAM;

    // e.g. "www.example.com" or IP; e.g. "http" or port number
	if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
        std::cerr << "ERROR: getaddrinfo: " << gai_strerror(status) << std::endl;
        exit(1);
    }

	// create a socket using TCP IP
	int sockfd = socket(res->ai_family, res->ai_socktype, 0);
	if (sockfd == -1) {
		std::cerr << "ERROR: Failed to create socket" << std::endl;
		exit(-1);
	}

	// allow others to reuse the address
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	// bind the socket
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		std::cerr << "ERROR: Failed to bind to socket" << std::endl;
		exit(-1);
	}

	// set socket to listen status
	if (listen(sockfd, 1) == -1) {
		std::cerr << "ERROR: Failed to listen to socket" << std::endl;
		exit(-1);
	}

	// accept a new connection
	struct sockaddr_in clientAddr;
	socklen_t clientAddrSize = sizeof(clientAddr);
	int clientSockfd;
	int connection_number = 0;

	// multithreading headers
	// use vector to keep track of threads
	std::vector<std::thread> t;

	while ((clientSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrSize)) && !SIG_HANDLER_CALLED) 
	{	

		if (clientSockfd == -1) {
			std::cerr << "ERROR: Failed to get accept connection" << std::endl;
			exit(1);
		}

		// Create a thread for each new accepted fd
		t.push_back(std::thread(handle_thread, clientAddr, clientSockfd, connection_number, file_dir));
		connection_number++;

	}
	if (SIG_HANDLER_CALLED) 
	{
		for (unsigned i = 0; i < t.size(); i++)
		{
			t[i].join();
		}
	}

   	exit(0);
}
