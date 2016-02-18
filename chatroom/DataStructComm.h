/////////////////////////////////
//Made by Shiyang Chen
//email: chshysy910811@gmail.com
//Homework-1 ECEN 602
//File: DataStructComm.h
/////////////////////////////////


//This is a common header file for all data communication between the Client and Server in SBCP
//Note: I had to define attributes above Message data structure as it was giving a compilation error.
// I still dont know why this was necessary

//So basically the Message and Attributes have the defined field types as shown below

#ifndef DataStructComm_H
#define DataStructComm_H

//Attributes field actually contains the payload of maximum size 512 bytes.
//We were thinking of dynamically allocating memory here to save space but
//we got many segmentaion faults prevented us from doing so and thus we decided
//to go ahead with maximum length payload field for all communications
//we will use '\0' to delimit out payload
struct Attributes
{

    int type;
    int length;
    char payload[512];

};


// Message Data structure encapsulation showcased.
//We did not use union as we were getting garbage value whenever we did some chat operation
struct Message
{
    int version:9;
    int type:7;
    int length;
    struct Attributes attributes[4]; //we only used two attributes that is username and payload but more can be assigned here
};

//Data structure for client information keeps all the data of the clients stored here
struct infoClient
{

   int fd;
   int client_number;
   char user[16];

};





#endif
