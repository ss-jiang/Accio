#include <string>
#include <thread>
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc != 2) {
      	std::cerr << "ERROR: server must be called with exactly two parameters: <PORT> <FILE-DIR>\n";
      	exit(-1);
   	}
  	
  	//std::cerr << "server is not implemented yet" << std::endl;
}
