README for SBCP Multi-Client Single Server Protocol For Chatting Applications Implementation:

Authors: Shiyang Chen

The Simple Broadcast Chat Protocol is a protocol that allows clients to join and leave a global chat session and send and receive messages.
This consists of the following files:
1. server.c
2. client.c
3. DataStructComm.h
4. Makefile1

Steps:

1. Now in order to run this program one just needs to use make -f Makefile1 in the command line of Terminal in Linux system. The program will auto compile. It consists of the necessary compilation commands and thus need not be entered again.

2. Server is executed first by typing ./server <server_ip_address> <server_port> <maximum clients>.

3. Client is executed by typing ./client <User Name> <server_ip_address> <server_port>.

4. DataStructComm.h is a common header file we created which would use a common datastructure for communication.

5. The server basically establishes  a session where it adds clients with unique usernames, upon recieving the JOIN message and while adding it sends them an ACK message to complete joining.

6.The username that one uses is unique for a given session. If a client tries to login with a username already in use he will get a NAK and be dissallowed.

7. The number of people together in a session is specified by the 3rd server argument <maximum number of clients>. Any more than this will recieve a NAK message upon an attempt to join.

8. If a client exits unceremoniously, his status is broadcast to everybody else by the server and his username is made available to the client who wishes to use that particular username.

9. Clients use SEND to send their messages to the server which then uses FWD to broadcast their messages across the other clients in the session. 

10. Apart from this the client can also recieve a variety of messages from the server.