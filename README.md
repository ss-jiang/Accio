## Report

# Stan Jiang, 104473226

# High Level Design Description
* Server
 * Creates a socket, binds it, and sets it to listen.
 * When the server accepts a connection using accpet(), it will create a thread allowing that connection to run and receive a file. The threads are detached from the main thread. 
 * The handle_thread() function uses select to determine if the connections times out before receiving data. 
 * The server reads in sent data using recv() on the socket. This runs in a loop until recv() returns a 0 or -1, signifying end of file or an error.

* Client
 * Creates a socket and set the file descriptor into non-blocking mode.
 * Call connect() to connect to the address; use select() to timeout when attempting to connect for more than 10 seconds. Set the file descriptor back to blocking mode. 
 * Once connected, client can write to the server. Using an ifstream, client reads in the file specified into a buffer of 1500 bytes. This is the typical TCP packet size. Continuously read and send the buffer data to the server using read() and send().


# The problems you ran into and how you solved the problems
I had trouble working on the timeout of the server and client. Just by reading the man pages of select() and poll(), it was hard to understand how to properly implement the timeout. I fixed this by reading tutorials and code examples that employed the select() function. These are referenced below. 


# List of any additional libraries used


# Acknowledgement of any online tutorials or code example (except class website) you have been using.
* Timeout logic from: http://developerweb.net/viewtopic.php?id=3196
* Client send example from: http://tldp.org/LDP/LGNET/91/misc/tranter/server.c.txt