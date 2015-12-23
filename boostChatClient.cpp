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


/* FUNCTION DEFINITIONS */

//  Takes a string of the clients name and assigns it to the value of the prompt pointer 
string* buildPrompt(){
	const int inputSize = 256;
	char inputBuf[inputSize] = {0};
	char nameBuf[inputSize] = {0};
	string* prompt = new string(": ");
	
	return prompt;
}

// Creates a loop which only inserts into the thread when a message is available on the socket connected to the server
void inboundLoop(socket_ptr sock, string_ptr prompt){
	int bytesRead = 0;
	char readBuf[1024] = {0};
	
	for(;;){
		if(sock->available()){
			bytesRead = sock->read_some(buffer(readBuf, inputSize));
			string_ptr msg(new string(readBuf, bytesRead));
			
			messageQueue->push(msg);
		}
		
		boost::this_thread::sleep( boost::posix_time::millisec(1000) );
	}
}

// Write to Queue function
void writeLoop(socket_ptr sock, string_ptr prompt){
	char inputBuf[inputSize] = {0};
	string inputMsg;
	
	for(;;){
		cin.getline(inputBuf, inputSize);
		inputMsg = *prompt + (string)inputBuf + '\n';
		if(!inputMsg.empty()){
			sock->write_some(buffer(inputMsg, inputSize));
		}
		
		// The string for quitting the application
		// On the serer-side there is also check for "quit" to terminate the TCP socket
		if( inputMsg.find("exit") != string::npos ){
			exit(1);
		}
		
		inputMsg.clear();
		memset(inputBuf, 0, inputSize);
	}
}


// Display function
void displayLoop(socket_ptr sock){
	for(;;){
		if( !messageQueue->empty() ){
			if( !isOwnMessage(messageQueue->front()) ){
				cout << "\n" + *(messageQueue->front());
			}
			
			messageQueue->pop();
		}
		
		boost::this_thread::sleep( boost::posix:time::millisec(1000) );
	}
}

bool isOwnMessage(string_ptr message){
	if( message->find(*promptCpy) != string::npos )
		return true;
	else
		return false;
}


