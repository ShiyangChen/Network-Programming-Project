//////////////////////////////
//Shiyang Chen
//chshysy910811@gmail.com
/////////////////////////

#include <iostream>

#include <stdio.h>

#include <sys/socket.h>

#include <sys/types.h>

#include <netinet/in.h>

#include <string.h>

#include <stdlib.h>

#include <netdb.h>

#include <arpa/inet.h>

#include <errno.h>

#include <unistd.h>

#include <cstring>

#include <fstream>

#include "proxy.h"



using namespace std;



int main(int argc, char* argv[])

{

    if(argc == 4)

    {



    struct sockaddr_in info_server;

    bzero((char*)&info_server, sizeof(struct sockaddr_in));

    int fd_socket;



    size_t k=4448576;

    char recvbuffer[k];



    if((fd_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)

    {

        perror( "Client socket creation error\n");

        exit(1);

    }


    //seting a timeout to constructively close the read operation.

    struct timeval timeout;

    timeout.tv_sec = 0;

    timeout.tv_usec = 800000; // 800ms waiting period



    setsockopt (fd_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout));

    string delim("/");

    info_server.sin_family= AF_INET;

    info_server.sin_port = htons(atoi(argv[2]));

    inet_pton(AF_INET, argv[1], &info_server.sin_addr);

    string url(argv[3]);

  //connect to server

  if(connect(fd_socket, (struct sockaddr*)&info_server, sizeof(info_server)) == -1)

  {

    perror("Connection to server error\n");

    exit(1);

  }

  else printf("Connection Established\n");



  //filename data storage

  TOK_LIST slash = seperate(url,delim);

  int jkl = slash.size()-1;

  string file_name = slash.at(jkl);
string delim2(".");
  TOK_LIST file_tok = seperate(file_name,delim2);
  int f2 = file_tok.size();

  string temp(".html");

 string save_string;

  cout<<file_name<<endl;

  if(f2==2)

    save_string =file_name;

   else save_string =file_name+temp;



  string message = "GET "+url+" HTTP/1.0\r\n\r\n";

  //send the GET request to proxy server

  int bytes = send(fd_socket, message.c_str(), message.length(), 0);



  //preparations for reciept of data

  bytes = 0;

  ofstream fp;

  char recvd[65536];//64K is maximum limit of read function we tried squeezing more data into it but it didnt work

  int bytes1 = 0;

 int i=0 ;

  while(1)

 {

	bytes1 = bytes1+bytes;

	bytes = read(fd_socket,recvd,65536);

	//append the recieved bytes into the larger buffer

	for(i =  0;i<=bytes-1;i++)

    {



		recvbuffer[bytes1+i] = recvd[i];

    }



	if(bytes<=0) goto lol1;



 }

  lol1 :string g(recvbuffer,bytes1);



 string delim1("\r\n\r\n"); //setting up delimitor and seperating the body and http header tags
 string g5(g);
    TOK_LIST tl5 = seperate(g5, "\r\n");
    string status(tl5.at(0));
    TOK_LIST tl6 = seperate(status, " ");


  TOK_LIST segments = seperate(g, delim1);

  if(segments.size()==2){ //this is to deal with 404 errors. Here we will just send the header back to client where the Not Found tag is highlighted
  string httpBody = segments.at(1); // meaning we just seperated the body part

  string httpheader = segments.at(0);

  cout<<"--------------------Header Information----------------------"<<endl;

  cout<<httpheader<<endl<<endl;

  //saved the file with the appropriate filename

  cout<<"Complete::::::Saving in the with the following file name::  "<<save_string<<endl;

    fp.open(save_string.c_str());

    fp << httpBody;

    fp.close();

  //file work done close the socket

  close(fd_socket);
  }
  else cout<<tl6.at(1)<<endl<<endl;

  return 0;

  }

  printf("\nUsage: client <proxy_server_ip> <proxy_server_port> <URL to retrieve>\n");

  return -1;

}
