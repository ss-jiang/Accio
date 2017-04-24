#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <stdlib.h>
#include <fcntl.h>

#include <string>
#include <thread>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <fstream>

#define MAX_PATH_LENGTH        4096

int main(int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "ERROR: client must be called with exactly three parameters: <HOSTNAME-OR-IP> <PORT> <FILENAME>" <<std::endl;
    exit(-1);
  }

  std::string ip_addr = argv[1];
  int port_num = atoi(argv[2]);
  std::string file_name = argv[3];
  
  // for timeout
  fd_set myset;
  int valopt;
  struct timeval tv;
  socklen_t lon;
  long arg; 

  // Check that the port number is in range
  if (port_num < 1024 || port_num > 65535) {
    std::cerr << "ERROR: Port number out of range [1024 - 65535]" << std::endl;
    exit(1);
  }

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // set non-blocking to check timeout
  arg = fcntl(sockfd, F_GETFL, NULL); 
  arg |= O_NONBLOCK; 
  fcntl(sockfd, F_SETFL, arg); 

  // stuff to get address information and connect
  struct addrinfo hints, *res;
  int status;
  char ipstr[INET_ADDRSTRLEN] = {'\0'};

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE; 
  hints.ai_family = AF_INET; // AF_INET specifies IPv4
  hints.ai_socktype = SOCK_STREAM;

  // e.g. "www.example.com" or IP; e.g. "http" or port number
  if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) {
    std::cerr << "ERROR: getaddrinfo: " << gai_strerror(status) << std::endl;
    exit(1);
  }

  // TIMEOUT LOGIC FROM http://developerweb.net/viewtopic.php?id=3196
  int cv = connect(sockfd, res->ai_addr, res->ai_addrlen);
  if (cv < 0) { 
    if (errno == EINPROGRESS) { 
      while(1) 
      { 
        tv.tv_sec = 10; 
        tv.tv_usec = 0; 
        FD_ZERO(&myset); 
        FD_SET(sockfd, &myset); 

        cv = select(sockfd+1, NULL, &myset, NULL, &tv); 
        if (cv < 0 && errno != EINTR) { 
          std::cerr << "ERROR: Error connecting\n";
          exit(1);
        } 
        else if (cv > 0) { 
          // Socket selected for write 
          lon = sizeof(int); 
          if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
            std::cerr << "ERROR: Error in getsockopt()\n";
            exit(1); 
          } 
          if (valopt) { 
            std::cerr << "ERROR: Error in delayed connection()\n"; 
            exit(1); 
          } 
          break; 
        } 
        else 
        { 
          std::cerr << "ERROR: Timeout in select() - Cancelling!\n"; 
          exit(1); 
        } 
      } 
    } 
    else 
    { 
      std::cerr << "ERROR: Error connecting\n";
      exit(1); 
    } 
  } 
  // Set to blocking mode again... 
  arg = fcntl(sockfd, F_GETFL, NULL); 
  arg &= (~O_NONBLOCK); 
  fcntl(sockfd, F_SETFL, arg); 

  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    std::cerr << "ERROR: Failed to get socket name" << std::endl;
    exit(1);
  }

  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Set up a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;

  // std::cout << "Sleeping\n";
  // sleep(11);
  // std::cout << "End sleep\n";

  // FROM: 
  // http://tldp.org/LDP/LGNET/91/misc/tranter/server.c.txt
  /* open the file to be sent */
  // int fd = open(file_name.c_str(), std::ios::in | std::ios::binary);
  // if (fd == -1) {
    // fprintf(stderr, "unable to open '%s': %s\n", &file_name[0], strerror(errno));
    // exit(1);
  // }

  // /* get the size of the file to be sent */
  // fstat(fd, &stat_buf);

  // sleep(10);

  // // timeout variables
  // fd_set writefds;
  // FD_ZERO(&writefds);
  // FD_SET(sockfd, &writefds);
  // int rv = select(sockfd+1, NULL, &writefds, NULL, &tv);

  // if (rv == -1) 
  // {
  //   std::cerr << "ERROR: Select() failure" << std::endl;
  //     exit(1);
  // } 
  // else if (rv == 0) 
  // {
  //   printf("Timeout occurred!  No data after 10 seconds.\n");
  // } 
  // else 
  // {
  //   /* copy file using sendfile */
  //   int rc = sendfile(sockfd, fd, NULL, stat_buf.st_size);
  //   std::cout << "Sending file: " << file_name << std::endl;
  //   std::cout << "Bytes: " << rc << std::endl;

  //   if (rc == -1) {
  //     fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
  //     exit(1);
  //   }

  //   if (rc != stat_buf.st_size) {
  //     fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n", rc, (int)stat_buf.st_size);
  //     exit(1);
  //   }
  // }

  std::ifstream open_file (file_name.c_str(), std::ios::in | std::ios::binary );
  char in_buffer[25000];
  int wc = 0;

  while(!open_file.eof())
  {
    open_file.read(in_buffer, 25000);
    wc += open_file.gcount();
    if(send(sockfd, in_buffer, open_file.gcount(), 0) == -1)
    {
      std::cerr << "ERROR: Could not send file\n";
      exit(1); 
    }
    // test delay
    // std::cout << "timeout" << std::endl;
    // sleep(2);
  }
  std::cout << "Sent file: " << file_name << std::endl;
  std::cout << "Bytes: " << wc << std::endl;

  open_file.close();
  close(sockfd);

  return 0;
}
