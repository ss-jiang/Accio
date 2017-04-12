#include <string>
#include <thread>
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc != 4) {
      	std::cerr << "ERROR: client must be called with exactly three parameters: <HOSTNAME-OR-IP> <PORT> <FILENAME>" <<std::endl;
      	exit(-1);
   	}

}
