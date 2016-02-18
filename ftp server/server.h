/////////////////////////////////
//Made by Shiyang Chen
//email: chshysy910811@gmail.com
//Homework-2 ECEN 602
//File: server.c
/////////////////////////////////
#include <sys/socket.h>
#include <netdb.h>
//We defined the opcodes as follows. Of course WRQ has not been implemented as it was not required by assignment
#define RRQ                        1
#define WRQ                        2
#define DATA                       3
#define ACK                        4
#define ERROR                      5

//We also defined the maximum data size that can be sent on a packet
#define DATA_SIZE                  512
//Maximum clients accommodated in a single session
#define CLT_MAX     100
//Basically 32 MB is the maximum size of file that can be transfered. Block numbers can only be 2 byte long and only a certain number of packets can be sent.
#define FILE_MAX    33553919   // We cannot send files larger than 32 MB as tftp transfer for files above 32 mb fails for old devices

//Checking if the client has finished a session(0) or a new client position is open(0) or is still present(1)
#define AVAILABLE 0
#define UNAVAILABLE 1

//header for Acknowlegement packet
struct packet {
	unsigned short op_code;
	unsigned short block_number;
};

//datastructure for storing client data
struct info_client {
	int state; //specifies if particular client  thread is available or not
    int length; // length of structure
    int user;   // user number
	char file[256]; // requested filename
	struct sockaddr addr_client;//client address info

};


