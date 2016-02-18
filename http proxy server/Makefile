COMPILER = g++
CFLAGS = -g -c -Wall

all:
	$(COMPILER) $(CFLAGS) HttpClassFile.cpp -o http.o
	$(COMPILER) $(CFLAGS) server.cpp -o server.o
	$(COMPILER) $(CFLAGS) client.cpp -o client.o
	$(COMPILER) -g client.o http.o -o client
	$(COMPILER) -g server.o http.o -o proxy

