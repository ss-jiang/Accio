#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <string>
#include <thread>
#include <iostream>
#include <signal.h>
#include <sstream>

// server is called with the following parameters
// server <PORT> <FILE-DIR>

void signal_handler(int signum)
{
	std::cout << "signal handler called" << std::endl;
	// Exit zero when SIGQUIT or SIGTERM received
	exit(0);
}


int main(int argc, char* argv[])
{
	if (argc < 3) {
      	std::cerr << "ERROR: server must be called with exactly two parameters: <PORT> <FILE-DIR>" << std::endl;
      	exit(-1);
   	}

   	signal(SIGQUIT, signal_handler);
   	signal(SIGTERM, signal_handler);
	
	//std::string ip_addr = "127.0.0.1";
   	int PORT_NUM = atoi(argv[1]);
   	//std::cout << PORT_NUM << std::endl;
   	std::string FILE_DIR = argv[2];
   	//std::cout << FILE_DIR <<std::endl;
   	struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET_ADDRSTRLEN] = {'\0'};

   	// Check that the port number is in range
   	if (PORT_NUM < 1024 || PORT_NUM > 65535) {
	    std::cerr << "ERROR: Port number out of range [1024 - 65535]" << std::endl;
	    exit(1);
	}


	memset(&hints, 0, sizeof hints);
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

	while (clientSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrSize)) 
	{

		if (clientSockfd == -1) {
			std::cerr << "ERROR: Failed to get accept connection" << std::endl;
			exit(1);
		}

		//for (p = res; p != NULL; p = p->ai_next) {
	  //       struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
	  //       void* addr = &(ipv4->sin_addr);
	  //       const char* ipver = "IPv4";
	  //       unsigned short* port = &(ipv4->sin_port);

	  //       // convert the IP to a string and print it:
	  //       inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
	  //       printf("  %s: %s\n", ipver, ipstr);
	  //       std::cout << "Accept a connection from: " << ipstr << ":" <<
			// ntohs(*port) << std::endl;

		inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
		std::cout << "Accept a connection from: " << ipstr << ":" <<
		ntohs(clientAddr.sin_port) << std::endl;
	    //}

	}

	// // read/write data from/into the connection
	// bool isEnd = false;
	// char buf[20] = {0};
	// std::stringstream ss;

	// while (!isEnd) {
	// memset(buf, '\0', sizeof(buf));

	// if (recv(clientSockfd, buf, 20, 0) == -1) {
	//   perror("recv");
	//   return 5;
	// }

	// ss << buf << std::endl;
	// std::cout << buf << std::endl;

	// if (send(clientSockfd, buf, 20, 0) == -1) {
	//   perror("send");
	//   return 6;
	// }

	// if (ss.str() == "close\n")
	//   break;

	// ss.str("");
	// }

	// close(clientSockfd);

	// return 0;

   	exit(0);
}
