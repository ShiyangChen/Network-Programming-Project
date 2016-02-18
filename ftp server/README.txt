README for TFTP Server Implementation:

Authors: Shiyang Chen
The TFTP is a protocol that allows clients to read and get the remote files.
This consists of the following files:
1. server.c
2. server.h
3. Makefile

Steps:

1. Now in order to run this program one just needs to use make -f makefile in the command line of Terminal in Linux system. The program will auto compile. It consists of the necessary compilation commands and thus need not be entered again.

2. Server is executed first by typing ./server <server_ip_address> <server_port> 

3. The client can fetch the file by:
typing tftp -m octet <server_ip_address> <server_port> -c get <filename> <newfilename>
or:
typing python grading_hw2.py <server_ip_address> <server_port>

4. server.h is a common header file we created which would use a common data structure for communication.
5. We have used threads for this program so you would need -pthread in the makefile to compile it.
6. Client closing tieout has been taken to be 50 lost acknowledgements and can be changed if necessary.
7.We used setsockopt for timeout implementation. The user can choose different timeout values in seconds or miliseconds format as per their wish. For testing purpose we kept it at 5 sec. It can be reduced as per user wish.
