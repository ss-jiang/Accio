#include <string>
#include <thread>
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc != 3) {
      	std::cerr << "ERROR: client must be called with exactly three parameters: <HOSTNAME-OR-IP> <PORT> <FILENAME>\n";
      	exit(-1);
   	}

}
