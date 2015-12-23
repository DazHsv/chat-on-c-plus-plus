/* 
	File: boostChatClient.
	By: Hugo Sanchez V
	Started At: 22/12/15
*/

#include <iostream>
#include <queue>
#include <string>
#include <cstdlib>

#include <boost/thread.hpp>		// boost/thread : Multithreading support
#include <boost/bind.hpp>		// boost/bind : Used for create sub-rountines for threads
#include <boost/asio.hpp>		// boost/asio : System socket and network programming library
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>	// boost/algorithm/string : Gives use some new string methods

/* Setting-up some namespaces for human reading */
using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

/* Define some typedef for Boost shared pointers */
typedef boost::shared_ptr<tcp::socket> socket_ptr;
typedef boost::shared_ptr<string> string_ptr;
typedef boost::shared_ptr< queue<string_ptr> > messageQueue_ptr;

/* Setting-up io_service in to initialize the boost::asio networking */
io_service service;						// Boost Asio io service
messageQueue_ptr messageQueue(new queue<string_ptr>);		// Queue for producer-consummer pattern
tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001);	// TCP socket for connecting to server
const int inputSize = 256;					// Maximum size for input buffer
string_prt promptCpy;						// Terminal prompt displayed to chat users


/* Function Prototypes */
bool isOwnMessage(string_ptr);
void displayLoop(socket_ptr);
void inboundLoop(socket_ptr, string_ptr);
void writeLoop(socket_ptr, string_ptr);
string* buildPrompt();

// END of Function Prototypes


/* MAIN FUNCTION */
int main(int agrc, char** argv){
	try{
		boost::thread_group threads;
		socket_ptr sock(new tcp::socket(service));
		
		string_ptr prompt( buildPrompt() );
		promptCpy = prompt;
		
		sock->connect(ep);
		
		cout << "Welcome to the ChatApplication\nType \"exit\" to quit" << endl;
		
		threads.create_thread(boost::bind(displayLoop, sock));
		threads.create_thread(boost::bind(inboundLoop, sock, prompt));
		threads.create_thread(boost::bind(writeLoop, sock, prompt));
		threads.join_all();
	} catch (std::exception& e){
		cerr << e.what() << endl;
	}
	
	puts("Press any key to continue...");
	getc(stdin);
	return EXIT_SUCCESS;
}





