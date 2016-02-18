/////////////////////////////////
//Made by Shiyang Chen
//email: chshysy910811@gmail.com
//Homework-2 ECEN 602
//File: server.c
/////////////////////////////////


// Standard header files
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include "server.h"
#include <pthread.h>
#include <sys/stat.h>

// Global variable declaration

int ACK_count = 50;
//If 50 ACKs are lost then go to timeout
//As per discussion on Piazza
int Total_users;
//Total number of users
struct sockaddr_in addr_server;
//Multiple thread handling variable
pthread_mutex_t  MUTEX;
//MUTEX for multiple clients
struct info_client **CLIENTS;
//client structure pointer for data storage on multiple clients


// This method handle the clients request by sending back the data give a specific
// file name by the client. It uses the thread created by the main funcion.

void * Multi_client_RRQ(void * add_client)
{
    struct packet ack;
    char Buffer[516];
    char Buffer_recv[516];
    int count = 0;
    //file variables
    char file[100];
	int  file_size;
	int block_size;
    int isLast;

    //data transfer socket fd and port
    int sockfd;
    int port;

    //check if we got ACK from client
    int recv_data;

    //we use unsigned short as it has maximum length 2 bytes only. Int has 4 bytes on 64bit system
	unsigned short block_number;
	struct sockaddr addr_current_client;

    //thread address struct
	struct sockaddr_in threads;

    //variable initialisation
	block_number = 1;
    isLast = 0;

    //file pointer created
	FILE *fp;

	//file buffer
	struct stat file_buffer;

    struct timeval time_out;
	time_out.tv_sec = 5;
    time_out.tv_usec = 0;

	struct info_client *current_client = (struct info_client*)add_client;

	// get the file, client_addr



	addr_current_client = current_client->addr_client;

    //initialization of data sending socket
    memset(file, 0, sizeof(file));
    strcpy(file, current_client->file);
    file[strlen(current_client->file)] = '\0';
	memset(&threads, 0,sizeof(struct sockaddr_in));
	memcpy(&threads,&addr_server , sizeof(struct sockaddr_in));

    //UDP type socket created
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	// Create random port number, greater than 1024. The probablity that two
    // threads got the same number is very low.
	while(1)
    {
		port = (rand()%(9000))+20000;//m port declaration. Anything above 1024 will do upto (2^16-1)
		threads.sin_port = htons(port); //given thread takes port
		int check = bind(sockfd, (struct sockaddr *)&threads, sizeof threads);
		if ( check == 0)
        {
                printf("server data channel bound, Port Number: %d\n",port);
                break;//Breakout if port has been assigned
        }
	}

	//Used setsocketopt for Timeout for port.
    int check = setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out,sizeof(time_out));
    if ( check < 0) perror("setsockopt");

    fp = fopen(file, "r");

    stat(file, &file_buffer);
    file_size = file_buffer.st_size;


    // Check whether the file exists.
    if (!fp)
    {
        printf("No Such File Exists.\n");
        memset(Buffer,0,516);
        Buffer[0] = 0;
        Buffer[1] = ERROR;
        Buffer[2] = 0;
		Buffer[3] = 1; //When this recieves RRQ for a file non existent in the system, It sends error message
		bcopy("No Such File", Buffer+4, 12);
		Buffer[16] = 0;
		sendto(sockfd, Buffer, 16, 0, &addr_current_client,sizeof(addr_current_client));


    }
    // Check whether the file exceeds the maximum size.
    else if (file_size > FILE_MAX)
    {
        printf("File is larger than 32 MB.\n");
        memset(Buffer,0,516);
        Buffer[0] = 0;
        Buffer[1] = ERROR;
        Buffer[2] = 0;
        Buffer[3] = 0;// File is larger than 32 Mb.
        bcopy("File size greater than 32 MB", Buffer+4, 28);
        Buffer[32] = 0;
        sendto(sockfd, Buffer, 32, 0, &addr_current_client,sizeof(addr_current_client));


    }
    // For other case, send the file block by block. Each block contains 512B.
    else
    {
        while (!isLast) {
            //ever block is set up like this
            //Header declarations
            memset(Buffer, 0, sizeof(Buffer));
            Buffer[0] = 0;
            Buffer[1] = DATA;
            Buffer[2] = (block_number >> 8) & 0xff;
            Buffer[3] = (block_number) & 0xff;

            block_size = fread(Buffer+4, 1, DATA_SIZE, fp);
	    int kl12 = ftell(fp);

            file_size -= block_size;
            printf("File Size Remaining : %d File Pointer Position: %d\n",file_size,kl12);

            // If the file size reduces to 0, or the block size less than 512B,
            // then it is the last block.
            if ((block_size != 512)||(file_size == 0) )
            {
                isLast = 1;

            }

            printf("Sending block#%u to client#%d ...\n", block_number, current_client->user);

            // Send the file back to client.
            if (sendto(sockfd, Buffer, block_size+4, 0, &addr_current_client, sizeof(addr_current_client)) < 0)
            {
                perror("sendto");
                exit(1);
            }


            // If there is error in the transmission and file is not received,
            // send again.
			
          last: memset(Buffer_recv, 0, 516);
            if ((recv_data = recvfrom(sockfd, Buffer_recv, 516, 0, NULL, NULL)) < 0)
            {
		
		fseek(fp,-(block_size),SEEK_CUR);  //re read current data segment;
                file_size =file_size+block_size; //file size remaining remains unchanged
                count = count+1;                 //count incremented
                printf("count,%d\n",count);

                if(count >= ACK_count)           //if count is more than ACK count then terminate this particular thread
                {
                    printf("No ACK from Client for %d iterations: Connection Terminated...\n",count);
                    isLast = 1;                  //goes out of the loop for closing
                }
                printf("Re-Sending block#%u to client#%d ...\n", block_number, current_client->user); //else keep resending the data packet till you get ACK
                continue;
            }
            else{

                count = 0;
            }
            memset(&ack, 0, sizeof(ack));
            memcpy(&ack, Buffer_recv, sizeof(ack));

            // Check the ACK sent back from the client, and then keep counting
            // the block number.
            if (ntohs(ack.op_code) == ACK && ntohs(ack.block_number) == block_number)
            {
                printf("Received ACK for block#%u from client#%d ...\n", block_number, current_client->user);

                block_number = block_number+1;

            }
            else
            {
                continue;
            }

        // Send the last block and reset the block number.
            if (isLast)
            {
                if ((block_size == 512) && (file_size == 0)) //special case for file sizes divisible by 512. Last data is of 512 bytes and we have to send an empty packet after this
                {
                        memset(Buffer,0,516);
                        Buffer[0] = 0;
		                Buffer[1] = DATA;
                		Buffer[2] = (block_number >> 8) & 0xff;
                		Buffer[3] = block_number & 0xff;
                        block_size = 0; //since no data has to be sent
                        printf("Sending Last block of data to client# %d ...\n", current_client->user);
                        if (sendto(sockfd, Buffer, block_size+4, 0, &addr_current_client, sizeof(addr_current_client)) < 0)
                        {
                			printf("sendto() error.\n");
                			exit(2);
                        }
                        goto last;

                }
                fclose(fp);

            }
            else
            {
                continue;
            }

        }

    }
    count = 0;
    // Close the socket and thread.
	close(sockfd);
	//lock the thread
	pthread_mutex_lock(&MUTEX);
	//update user list
    Total_users = Total_users-1;
    //unlock the thread
   	pthread_mutex_unlock(&MUTEX);
   	//make the client block available for any other client
	((struct info_client*)add_client)->state = AVAILABLE;
	//close the thread
	pthread_exit(0);
	//return 0 means success here
	return 0;
}



// Main function. Standard command line form:
// ./server <server IP address> <server port number>
// It creates socket connection and allocate multiple threads.

int main(int argc, char *argv[])
{
	if(argc == 3)//checking for input arguments
	{

        Total_users = 0;

        int sockfd;
        // Use double pointers to store mulitple clients, with each client including
        // his state (AVAILABLE or UNAVAILABLE), user number, client address and requested file.

        CLIENTS = (struct info_client **)malloc(CLT_MAX * sizeof(struct info_client*));

        int n = 0;


        struct sockaddr addr_client;
        socklen_t length;
        length = sizeof(struct sockaddr);

        ssize_t recv_data;
        char Buffer[516];
        pthread_t  Thread_ID;
        pthread_mutex_init(&MUTEX, NULL);

    // Create socket connection and set up the attributes. SOCK_DGRAM for UDP
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        memset(&addr_server,0,sizeof(addr_server));

        addr_server.sin_addr.s_addr = inet_addr(argv[1]);
        addr_server.sin_family = AF_INET;
        addr_server.sin_port = htons(atoi(argv[2]));

        int check = bind(sockfd, (struct sockaddr *)&addr_server, sizeof (addr_server));
        if ( check< 0)
        {
                close(sockfd);
                perror("bind");
                exit (1);
        }

        // To start with, allocate memory for all the possible clients and set the
        // state to available
        for (n = 0; n < CLT_MAX; n++)
        {
                CLIENTS[n] = (struct info_client*)malloc(sizeof(struct info_client));
                CLIENTS[n]->state = AVAILABLE;
        }


	while(1) {

		memset(Buffer,0, 516);
		recv_data = recvfrom(sockfd, Buffer, 516, 0,&addr_client, &length);
		if (recv_data == -1) continue;               // Ignore failed request

		if (Buffer[1] != RRQ)
        {
			printf("\n Request Denied, Only RRQ to be done");
			continue;          // For other functions, ignore them and loop back
		}

        // Find a free space for a new client
		for (n = 0; n < Total_users; n++)
        {
            if (CLIENTS[n]->state == AVAILABLE)
            {
                        	break;
            }
        }

        // Lock the thread to keep counting number of users correctly.
		pthread_mutex_lock(&MUTEX);
        Total_users = Total_users+1;
  	 	pthread_mutex_unlock(&MUTEX);

        //add information of the new client to the database
		memset(CLIENTS[n],0,sizeof(struct info_client));
        CLIENTS[n]->user = n;
        CLIENTS[n]->state = UNAVAILABLE;
        bcopy(&addr_client, &(CLIENTS[n]->addr_client), length);
		CLIENTS[n]->length = length;
		bcopy(Buffer+2, CLIENTS[n]->file, strlen(Buffer+2));

        // Create the child thread and pass the client's information.
		pthread_create(&Thread_ID, NULL, Multi_client_RRQ, (void *) CLIENTS[n]);
	}
	}
	printf("\nServer Usage: %s <server-ip> <server-port(greater than 1024)>\n", argv[0]);
    return 1;
}
