/////////////////////////////////
//Made by Shiyang Chen
//email: chshysy910811@gmail.com
//Homework-1 ECEN 602
//File: server.c
/////////////////////////////////

#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

// Standard header files needed for Socket Programming

#include "DataStructComm.h"

// Here we also created a Header file specifically for the data communication to take place.
// It contains Data structures Message which is the standard SBCP Message structure model.
// It has a 9 bit version number, a 7 bit type number, length of packet and a attribute data structure with maximum capacity 4*512 bytes.
// It also consists of an Attributes structure which basically consists of the Attribute type,length and a payload of maximum capacity 512 bytes.

int Number_of_users = 0;

// this the number of users at each instant of the server operation
int LIMIT = 0;

//this is the maximum number of clients allowed in one chat session
int check;

//variable for checking various read and write operations
char newuser[16];

//variable for storing new Client's username
struct infoClient *info;

//structure from DataStructComm which basically stores client information at every step


int checkUsernameExists(char name[]){

//here we created a function to check if a username is already in use.
// if not in use status = 0 else is status = 1;
    int status = 0;
	int i = 0;
    for(i = 0; i < Number_of_users ; i++){
        if(!strcmp(info[i].user,name)){
            status = -1;
            break;
        }
    }
    return status;
//returning status of username
}

//This function is used to send ACK or Acknowledgement of connection establishment to client
void ACK_client(int fd_client){
    int i = 0;
    char buffer[200];

    struct Attributes Message_client_ATB;// Attributes of SBCP Message

    struct Message Message_client;// SBCP message to be sent
    Message_client.version = 3;//version type
    Message_client.type = 7;//ACK type message
    Message_client_ATB.type = 3;//Attribute type is message

    //Doing the following is necessary or it will show random characters
    char digit;
    digit = (char)(((int)'0')+ Number_of_users);//number of users appended to the beginning of the ACK message payload
    buffer[0] = digit;
    buffer[1] = ' ';
    buffer[2] = '\0';

	//Concatenate users
    for(i =0; i<Number_of_users-1; i++)
    {

        strcat(buffer,info[i].user);
        if(i != Number_of_users-2)
        strcat(buffer, ",");
    }

    //now we identify the message length and copy buffer data into the attribute payload
    Message_client_ATB.length = strlen(buffer)+1;
    strcpy(Message_client_ATB.payload, buffer);

    //after this stage the attribute payload is encapsulated into the Message
    Message_client.attributes[0] = Message_client_ATB;

    //data is written i.e sent to the client with given field descriptor
    write(fd_client,(void *) &Message_client,sizeof(Message_client));


}

//We defined a function called NAK_client which based on status acquired from Status_client sends
//the appropriate reason for NAK or refusing the client access to the chat room
void NAK_client(int fd_client,int status){

        char REASON[32];

        //REASON field is sent with Attribute type 1
    	struct Message NAK;         //NAK message to be sent
    	NAK.version = 3;            //version number
        NAK.type = 5;               //NAK message type

        struct Attributes NAK_ATB;  //Attribute of NAK
    	NAK_ATB.type = 1;           //Attribute type

    	//Status == -1 : User name in use hence NAK
    	//Status == 2  : Client Limit Exceeded
    	if(status == -1)
        {
        	strcpy(REASON,"NAK: User Name in use");
    	}
        if(status == 2)
        {
		strcpy(REASON,"NAK: Chat Limit");
        }

        //Length calculation and REASON is taken as payload
    	NAK_ATB.length = strlen(REASON);
    	strcpy(NAK_ATB.payload, REASON);
        NAK.attributes[0] = NAK_ATB;

        //Send NAK to client and close the file descriptor
    	write(fd_client,(void *) &NAK,sizeof(NAK));
    	close(fd_client);

}



//Function: Status_Client(int client file-descriptor)
//checks the current status of old client and makes relevant enquiries
//for new clients like whether or not they have the same username as other clients
//or if space is available for Chat

int Status_Client(int fd_client){

//JOIN is the new SBCP message type and JOIN_ATB is an attribute variable
    struct Message JOIN;
    struct Attributes JOIN_ATB;
//Various variables declared for use.
// status_client_number is the number of existing clients and checks with client Limit
// buffer is payload variable of 16 bits or 2 bytes
//status is used to check for same username
//status1 is the value returned based on decision
    char buffer[16];
    int status = 0;
    int length=0;
    int status_client_number = 0;
    int status1 = 0;

    //JOIN message from client is read
    read(fd_client,(struct Message *) &JOIN,sizeof(JOIN));

    JOIN_ATB = JOIN.attributes[0];
    //attributes decapsulated and will be used for comparison

    //User Name extraction
    length=strlen(JOIN_ATB.payload);
    strcpy(buffer, JOIN_ATB.payload);
    buffer[length]='\0';

    //User name checking
    //Returns 0 for no match -1 for match
    status = checkUsernameExists(buffer);

    //Client number decided
    status_client_number = Number_of_users;

    if(status == -1 && status_client_number< LIMIT)
    {
        printf("Client already exists. \n");
        NAK_client(fd_client, -1); // -1 for client already exists
    }
    else
    {
        if(status ==0 && status_client_number < LIMIT)
        {
    //Here status1 = 0 implies that client is a new person and ACK will be sent to him
            status1 = 0;

            info[Number_of_users].fd = fd_client;                  // infoClient database updated
            info[Number_of_users].client_number = Number_of_users; // User number assigned
            strcpy(info[Number_of_users].user, buffer);            // User Name stored

            Number_of_users = Number_of_users + 1;                 //Total user number updated
            ACK_client(fd_client);                                 //ACK sent containing info for new user like people
                                                                   //already online and total number of online people
        }
        if(status_client_number >= LIMIT)
        {
            printf("Client Limit Exceeded. \n");                   //Total number of clients = to LIMIT initially set
            status1 = 1;
            NAK_client(fd_client,2);                               //NAK sent to user

        }

    }
    return status1;                                                 //status returned
}


// Main function has 3 command line inputs of the form
// ./server <server IP number>,<Server port number>, <LIMIT of clients admitted to chat session>
// Main Socket creation and multiplexing is handled by select in this case
int main(int argc, char const *argv[])
{
	int status_return = 0;
    if(argc==4)
    {
	    struct Message CLT;                  // Message CLT stores message from Client
	    struct Attributes CLT_ATB;           // Attributes CLT_ATB stores the respective attributes

        fd_set fd_Master;
	    fd_set Iterator;

        struct hostent* Hints;
        struct sockaddr_in *sockaddr_client;
	    struct sockaddr_in sockaddr_server;

        char new_name[16];
	    int fd_temp,i,j,k,l, Max_fd,fd_server, fd_client, status_client;  // various variablees to be used
        j = 0; l = 0; i = 0; k = 0; status_client = 0;                    // to read the current or new client's  status
        short port=0;

	    unsigned int im_length; // we need unsigned type here.

	    LIMIT=atoi(argv[3]);//specify maximum number of clients that can join

	    sockaddr_client=(struct sockaddr_in*)malloc(LIMIT*sizeof(struct sockaddr_in));//address storage for clients done here.
                                                                                      //We allocate memory for that as well

	    info=(struct infoClient*)malloc(LIMIT*sizeof(struct infoClient));//memory allocation for infoClient structure to store data


	    fd_server = socket(AF_INET,SOCK_STREAM,0);                        // socket creation connection oriented
	    if(fd_server == -1)
	    {
            printf("Server Message:Socket Cannot be created. \n");
            status_return = -1;
	    }
	    else
        {
            printf("Server Message:Socket Created. \n");
	    }


	    bzero(&sockaddr_server,sizeof(sockaddr_server));                   //server address initialized



	    Hints = gethostbyname(argv[1]);                                     //DNS lookup.
        port = atoi(argv[2]);                                               //get port address

	    sockaddr_server.sin_family = AF_INET;
	    sockaddr_server.sin_port = htons(port);                             //Conversion done to overcome big endian-little endian issues

	    memcpy(&sockaddr_server.sin_addr.s_addr, Hints->h_addr, Hints->h_length);  //Bing process is used after this

        if((bind(fd_server, (struct sockaddr*)&sockaddr_server, sizeof(sockaddr_server)))==0)
	    {
            printf("Server Message: Socket successfully bound. \n");
	    }
	    else
	    {
            printf("Server Message: Socket bind failed. \n");
            status_return = -1;
	    }

	    if((listen(fd_server, LIMIT))==0)                                   ////Listening for connections
	    {
            printf("Waiting for connections. \n");
	    }
	    else
	    {
            printf("Server Listen failed.\n");
            status_return = -1;
	    }

	    FD_SET(fd_server, &fd_Master);                                      //add the server address to the master file descriptor set
	    Max_fd = fd_server;                                                 //initially the maximum file descriptor is set as the server file descriptor

	    while(1){

		Iterator = fd_Master;//Iterator is the local fd-set and copies the value of master set

        //Here we use select for multiplexing IO operations
		select(Max_fd+1, &Iterator, NULL, NULL, NULL);

		for(i=0 ; i<=Max_fd ; i++)
        {
		    if(FD_ISSET(i, &Iterator))
		    {
		        if(i == fd_server) // incase we have server's file descriptor
		        {

		            im_length = sizeof(sockaddr_client[Number_of_users]);

        //          Server will accept a connection from the server

		            fd_client = accept(fd_server,(struct sockaddr *)&sockaddr_client[Number_of_users],&im_length);
		            if(fd_client < 0)
		            {
		                printf("Server acccept failed . \n");
		                status_return = -1;
		            }
		            else
		            {
		                fd_temp = Max_fd;
		                FD_SET(fd_client, &fd_Master);
		                if(fd_client > Max_fd)
		                {
		                    Max_fd = fd_client;
		                }
		                status_client = Status_Client(fd_client);//Status_Client decides whether or not the client can be allowed to chat

		                if(status_client == 0 && Number_of_users<=LIMIT)
                        {
			    printf("ACK:New Connection Established!!!\n");
//For the status = 0, the user is new and hence Message OL and attribute OL_ATB used with appropriate versions no.s etc
//This will also mean client is online hence type = 8 assigned
                            struct Message OL;
                            OL.version=3;
                            OL.type=8;
//For this case username will be sent
                            struct Attributes OL_ATB;
                            OL_ATB.type=2;//username type


//Initial Message creation
                            strcpy(OL_ATB.payload,info[Number_of_users-1].user);
                            OL_ATB.length=strlen(OL_ATB.payload);
                            OL.attributes[0]=OL_ATB;

                            int length_1=0;
                            char buffer1[30];
//Username extraction
                            length_1=strlen(info[Number_of_users-1].user);
                            strcpy(buffer1,info[Number_of_users-1].user);
                            buffer1[length_1]='\0';
                            printf("Name of the new Client : %s \n",buffer1);
//The following for loop has been used for broadcasting all informations of clients online and offline along with initial message
                            for(j = 0; j <= Max_fd; j++)
                            {

                                if (FD_ISSET(j, &fd_Master))
                                {

                                    if (j != fd_server && j != fd_client)
                                    {
                                        write(j,(void *) &OL,sizeof(OL));

                                    }
                                }
                            }
		                }
                        else
                        {
//In case if the client has been denied chat permission simply clear File descriptor from the master set
		                    Max_fd = fd_temp;
		                    FD_CLR(fd_client, &fd_Master);
		                }

		            }
		        }
                else
                {//In case we have a client file descriptor we have twoi possibilities: One: He is online or two he is offline
		//Reading data from client and incase we get nothing.. means client is offline.
		            if ((check=read(i,(struct Message *) &CLT,sizeof(CLT))) <= 0)
                    {

//This means that we have to forward this fact to all that the client is no longer available
                        struct Message FWD;
                        FWD.version=3;
                        FWD.type=6;  //offline type;
                        struct Attributes FWD_ATB;
                        FWD_ATB.type=2;

                        char user_vairable[16];
                        int len1;
                        bzero(user_vairable,sizeof(strlen));

                        for(l=0;l<Number_of_users;l++)
                        {
                            if(info[l].fd==i)
                            {
                                len1=strlen(info[l].user);
                                strcpy(user_vairable,info[l].user);
                                user_vairable[len1]='\0';
                                break;
                            }
                        }

                        //Payload: username is forwarded
                        strcpy(FWD_ATB.payload,user_vairable);
                        FWD_ATB.length=strlen(FWD_ATB.payload);
                        FWD.attributes[0]=FWD_ATB;

		                // got error or connection closed by client
		                if (check == 0)
                        {

		                	printf("%s is OFFLINE\n", user_vairable);
		                }
		                else
                        {
		                	perror("read");
		                }

		                close(i); //close the respective file descriptor
		                FD_CLR(i, &fd_Master); //the file descriptor has to be removed from master set
				//bzero(&info[i],sizeof(info[i]));
                        //assign his details to the next person in the linked list
                        for(k=i;k<=Number_of_users-1;k++)
                        {
                            strcpy(info[k].user,info[k+1].user);
                            info[k].fd=info[k+1].fd;
                            info[k].client_number=info[k+1].client_number;
                        }
                        //last element in list is initialized
                        //here we assumed iterative leaving of clients
                        //that is there are no simultaneous exits
                        //here the old username is available for use now

                        bzero(&info[Number_of_users-1],sizeof(info[Number_of_users-1]));

                        //Update the number of users available
                        Number_of_users=Number_of_users-1;

                        //This for loop simply broadcasts the details of the recently exited client
                        for(j = 0; j <= Max_fd; j++)
                        {

                            if (FD_ISSET(j, &fd_Master))
                            {

                                if (j != fd_server && j != i)
                                {
                                    write(j,(void *) &FWD,sizeof(FWD));
                                }
                            }
                        }
		            }

                    else
                    {
//If the client is available and check>0 that means server will then forward his message to all others other than server and that client
                        CLT.type=3;
                        CLT_ATB = CLT.attributes[0];
                        CLT.attributes[1].type=2;
                        printf("Payload from Client:FWD:%s\n",CLT_ATB.payload);


                        int Length_2;
                        //Update the user information data structure
                        for(l=0;l<Number_of_users;l++)
                        {
                            if(info[l].fd==i)
                            {
                                Length_2=strlen(info[l].user);
                                strcpy(new_name,info[l].user);
                                new_name[Length_2]='\0';
                                break;
                            }
                        }

                        strcpy(CLT.attributes[1].payload,new_name);
                        CLT.attributes[1].length=strlen(CLT.attributes[1].payload);

                        //Now whenever server gets a new messages from client it simply broadcasts to all
                        //except for itself and the client who sent the information.

                        for(j = 0; j <= Max_fd; j++)
                        {

                            if (FD_ISSET(j, &fd_Master))
                            {

                                if (j != fd_server && j != i)
                                {
                                    write(j,(void *) &CLT,sizeof(CLT));

                                }
                            }
                        }
			     }
		        }
		    }
		}
	    }

	    close(fd_server);
	}
	printf("Usage Details: <Server IP> <Server Port> <Max No of Clients>");
    return status_return;
}

