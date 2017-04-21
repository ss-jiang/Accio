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

int main(int argc, char* argv[])
{
	if (argc != 4) {
      	std::cerr << "ERROR: client must be called with exactly three parameters: <HOSTNAME-OR-IP> <PORT> <FILENAME>" <<std::endl;
      	exit(-1);
   	}

	std::string IP_ADDR = argv[1];
	std::cout << IP_ADDR << std::endl;
	int PORT_NUM = atoi(argv[2]);
	//std::cout << PORT_NUM << std::endl;
	std::string FILE_DIR = argv[3];
	//std::cout << FILE_DIR << std::endl;

	// Check that the port number is in range
   	if (PORT_NUM < 1024 || PORT_NUM > 65535) {
	    std::cerr << "ERROR: Port number out of range [1024 - 65535]" << std::endl;
	    exit(1);
	}

   	// create a socket using TCP IP
  	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  	struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET_ADDRSTRLEN] = {'\0'};

	memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_PASSIVE; 
    hints.ai_family = AF_INET; // AF_INET specifies IPv4
    hints.ai_socktype = SOCK_STREAM;
    
    // e.g. "www.example.com" or IP; e.g. "http" or port number
	if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
        std::cerr << "ERROR: getaddrinfo: " << gai_strerror(status) << std::endl;
        exit(1);
    }

	// connect to the server
	if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		std::cerr << "ERROR: Failed to connect to socket" << std::endl;
		exit(1);
	}

	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
		std::cerr << "ERROR: Failed to get socket name" << std::endl;
		exit(1);
	}

	// //for (p = res; p != NULL; p = p->ai_next) {
 //    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
 //    void* addr = &(ipv4->sin_addr);
 //    const char* ipver = "IPv4";
 //    unsigned short* port = &(ipv4->sin_port);

 //    // convert the IP to a string and print it:
 //    inet_ntop(res->ai_family, addr, ipstr, sizeof ipstr);
 //    printf("  %s: %s\n", ipver, ipstr);
 //    std::cout << "Set up a connection from: " << ipstr << ":" <<
	// ntohs(*port) << std::endl;
 //    //}

  	inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  	std::cout << "Set up a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;



	// send/receive data to/from connection
	bool isEnd = false;
	std::string input;
	char buf[20] = {0};
	std::stringstream ss;

	while (!isEnd) {
		memset(buf, '\0', sizeof(buf));

		std::cout << "send: ";
		std::cin >> input;
		if (send(sockfd, input.c_str(), input.size(), 0) == -1) {
			perror("send");
			return 4;
		}


		if (recv(sockfd, buf, 20, 0) == -1) {
			perror("recv");
			return 5;
		}

		ss << buf << std::endl;
		std::cout << "echo: ";
		std::cout << buf << std::endl;

		if (ss.str() == "close\n")
			break;

		ss.str("");
	}

	close(sockfd);

	return 0;

}
