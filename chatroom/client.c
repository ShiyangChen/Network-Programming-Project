/////////////////////////////////
//Made by Shiyang Chen
//email: chshysy910811@gmail.com
//Homework-1 ECEN 602
//File: client.c
/////////////////////////////////
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include<sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//Here above we used standard socket programming header files which are usually used

#include "DataStructComm.h"
//Here we defined a common mode of data communication using predefined data structures

//The following code is the basic client code for asking permission to join the chat via SBCP protocol
//And we have the JoinChatRoom function for that. Also for standard data transmission between server and
//client we have readMessage_Server and sendMessage_server which both take the file descriptor as input
//and perform operations of decapsulation and encapsulation of the data to be sent.

int check;
//For read/write messages or socket connection messages

// Function for reading the messages from the Server
int readMessage_Server(int fd)
{

    char buffer[512];char new_user[16];
    int type_message,message_ATB_type = 0;
    int status,i,length,len = 0;
    i=0;length = 0;
//  Server message and its attributes
    struct Attributes Message_server_ATB;
    struct Message Message_server;

    //information read from client
    if((check=read(fd,(struct Message *) &Message_server,sizeof(Message_server)))<=0)
    {
		 printf("cannot read from server");
		 exit(0);

    }
    // payload extraction
    Message_server_ATB = Message_server.attributes[0];
    message_ATB_type = Message_server_ATB.type;
    type_message=Message_server.type;
    length = strlen(Message_server_ATB.payload);
    strcpy(buffer,Message_server_ATB.payload);
    buffer[length] = '\0';
    length = strlen(buffer);


	if(type_message==3)
    {
		//if type is 4 in server message, then it is a normal message sent by other clients
	    if(message_ATB_type == 4)
        {
            if(Message_server.attributes[1].type==2 && Message_server.attributes[1].payload!=NULL)
            {
                length=strlen(Message_server.attributes[1].payload);
                strcpy(new_user,Message_server.attributes[1].payload);
                new_user[length]='\0';
            }

            if(new_user != " ")
            printf("%s : %s",new_user, buffer);
	    }
    }

    //if we get NAK then we are sent a reason for it
    if(type_message==5)
    {
	    //if Type 1 is in server message, server is sending a failure message
	    if(message_ATB_type == 1){

		printf("Reason for failure  %s ", buffer);
		status = 1;

	    }
    }
	//In case the person goes offline by exiting unceremoniously
   if(type_message==6)
    {
		if(message_ATB_type== 2)
		{
			printf(" %s is offline ",buffer);
		}
    }

    if(type_message==7)
    {
	    //ACK message recieved has type 7 and attribute 3 will mean that it has client number too!!
        if(message_ATB_type == 3)
        {
	    printf("ACK: Joined Chat\n");
            printf("Connected Clients= %c \n", (char)buffer[0]);
            printf("Online : ");
            for(i=2 ; i<length; i++)
            {
		        printf("%c",(char)buffer[i]);
            }


		status = 0;
	    }
    }
    //in case other clients are to notifying you that they are online
    if(type_message==8)
    {
		if(message_ATB_type==2)
		{
			printf("%s is online",buffer);
		}
    }
    return status;
}

//Join the Chat room
void ChatRoom_join(int fd,const char *arg[]){


    int joinStatus = 0;
    int length=0;
    struct Message JOIN;
    JOIN.version = 3;
    JOIN.type = 2;//type 2 for JOIN
    length=strlen(arg[1]);
    arg[length]='\0';

    //attribute values assigned
    struct Attributes attributes;
    attributes.length = strlen(arg[1]) + 1;
    attributes.type = 2;//type 2 for user
    strcpy(attributes.payload,arg[1]);//stores the user entered by client
    JOIN.attributes[0] = attributes;//stores the client attribute in Message structure

    //Send the joining message to the server to join
    write(fd,(void *) &JOIN,sizeof(JOIN));
    //put some time gap for collision avoidance
    sleep(1);

    joinStatus = readMessage_Server(fd);
	//close the connection if server rejects message
    if(joinStatus == 1)
    {
            close(fd);
    }
}

//function for sending messages from client to the server
void sendMessage_server(int fd){


    struct Attributes attributes;
    struct Message message;

    int check1 = 0;
    char buffer[512];

    check1 = read(STDIN_FILENO, buffer, sizeof(buffer));
    if(check1<=0)//read message from standard input
    {
		printf("Server Data Unreadable");
    }
    if(check1 > 0)
    {
        buffer[check1] = '\0';
    }

    //For SEND messages we need to follow this technique
    attributes.type = 4;
    strcpy(attributes.payload,buffer);
    message.attributes[0] = attributes;
    message.length=sizeof(message);
    write(fd,(void *) &message,sizeof(message));//send the message to the server for broadcasting purposes

}



int main(int argc, char const *argv[])
{
    /* code */
    if(argc != 4)
    {
        printf("\n Usage Details: <user name> <server address> <server port_number> \n");
        return 0;
    }
    if(argc == 4)
    {
        fd_set Main_fd; FD_ZERO(&Main_fd);
    	fd_set Iterator1; FD_ZERO(&Iterator1);


    	struct sockaddr_in server_socket;
    	struct hostent* Header_ptr;
        short port_number = 0;

    	int fd;
        //socket creation is being done below
    	fd = socket(AF_INET,SOCK_STREAM,0);

    	if(fd!=-1)
    	{
    	    printf("Socket successfully created..\n");

    	}

	    else
	    {
	        printf("socket creation failed...\n");
	        exit(0);
	    }

	    bzero(&server_socket,sizeof(server_socket));

	    server_socket.sin_family=AF_INET;

	    //Here we used gethostbyname as we found out that getaddrinfo basically utilized this function only
	    //for DNS look ups etc

	    Header_ptr = gethostbyname(argv[2]);

	    //Port number assigned
	    port_number = atoi(argv[3]);

	    //Server port number for communication by client and respective addresses are added to the .sin_addr.s_addr
	    server_socket.sin_port=htons(port_number);
	    memcpy(&server_socket.sin_addr.s_addr, Header_ptr->h_addr, Header_ptr->h_length);

        //client calls connect
        check = connect(fd,(struct sockaddr *)&server_socket,sizeof(server_socket));
	    if(check!=0)
	    {
	        printf("Server Disconnected\n");
	        exit(0);
	    }
	    else
	    {
	        printf("Server Connection Established\n");
	        ChatRoom_join(fd, argv);              //Client attempts to join the chat witht he information he has provided
	        FD_SET(fd, &Main_fd);                 //Set the file descriptor of the socket in Main file set
	        FD_SET(STDIN_FILENO, &Main_fd);       //While the client types the data is stored in the main file sets location
	        while(1)
            {
	            Iterator1 = Main_fd;

		    printf("\n");

	            if(select(fd+1, &Iterator1, NULL, NULL, NULL) == -1)exit(0);    //we use select to handle multiplexing operations

	            if(FD_ISSET(fd, &Iterator1))readMessage_Server(fd);             // this interprets and transfers the server message to the client

	            if(FD_ISSET(STDIN_FILENO, &Iterator1))sendMessage_server(fd);   //this reads client input and sends the data to the server after encapsulation

            }
        }
    }

    return 0;
}
