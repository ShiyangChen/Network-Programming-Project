//////////////////////////////
//Shiyang Chen
//chshysy910811@gmail.com
/////////////////////////

#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include "proxy.h"

using namespace std;
typedef map<string, HTTP_pointer*> CACHE_LIST;

string not_modified_response("304");
string not_exist("404");
CACHE_LIST cache;

//function explicit declaration
string download_WEB(string, int*,string,int);

void update_cache(string, HTTP_pointer*,int);

void cache_display();
HTTP_pointer* found_in_cache(string,int);
HTTP_pointer* not_found_in_cache(string,int);

int main(int argc, char*argv[])
{
  if(argc == 3)
  {
    int max_clients = 5;//you can select the number of MAX clients. But please also change MAX in header file
  	//initialization of various buffers variables etc that we will use in this program
  	int i, fd_client, fdmax;
    struct sockaddr_in SERVER;
    struct sockaddr_in CLIENT;
  	fd_set readfds;
    FD_ZERO(&readfds);
    fd_set master;
  	FD_ZERO(&master);

    short port;
  	char copybuffer[1024];
  	char proxy_buffer[1024];

  	int fd_server = socket(AF_INET, SOCK_STREAM, 0);
  	if(fd_server < 0)
  	{
    		perror("Server socket creation error\n");
    		exit(1);
  	}

  	int yes = 1;
  	if(setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
  	{
    		perror("setsockopt error");
    		exit(-1);
  	}

    //assigning of the server's file descriptor
  	bzero((char*) &SERVER, sizeof(struct sockaddr_in));
  	SERVER.sin_family= AF_INET;
    port = htons(atoi(argv[2]));
  	SERVER.sin_port = port;
  	inet_pton(AF_INET, argv[1], &SERVER.sin_addr);


    //bind the server to a port
  	if(bind(fd_server, (struct sockaddr*) &SERVER, sizeof(SERVER))<0)
  	{
    		perror("Server bind error\n");
    		exit(-2);
  	}
    //listen on max_clients number of ports
  	if(listen(fd_server, max_clients) < 0)
  	{
    		perror("Listen error");
    		exit(-33);
  	}

    cout<<"Proxy Server is Waiting for New Connections"<<endl;

  	FD_SET(fd_server, &master);
  	fdmax = fd_server;
  	u_int leg;

  	while(1)
  	{
    		printf("\n");
    		readfds = master;
    		bzero(copybuffer,sizeof(copybuffer));
    		bzero(proxy_buffer,sizeof(proxy_buffer));

    		if(select(fdmax+1, &readfds, NULL, NULL, NULL) == -1)
    		{
      			perror("Select error\n");

      			exit(1);
    		}
            //ignore the first two descriptors as they belong to server and web-server
    		for(i=3; i<=fdmax; i++)
    		{
      			if(FD_ISSET(i, &readfds))
      			{
        			if(i == fd_server)
				{
	  				leg = sizeof(CLIENT);
	  				fd_client = accept(fd_server, (struct sockaddr*) &CLIENT, &leg);
	  				if(fd_client >= 0)
	  				{

	    					FD_SET(fd_client, &master);
            					if(fd_client > fdmax)
              					fdmax = fd_client;
	  				}
	  				else
	  				{
	    					perror("Could not accept connection\n");
	  				}
                }
                else
                {
                    int b = recv(i, proxy_buffer, sizeof(proxy_buffer), 0);
                    if(b <= 0)
                    {
	    				FD_CLR(i, &master);
	    				continue;
                    }

                    string str(proxy_buffer, b);
	  	    //parse the request
          	    strcpy(copybuffer, proxy_buffer);
                    char* token = (char*)malloc(sizeof(char*));
                    token = strtok(copybuffer, " ");
                    char* tokens[3];
                    int NN = 0;
                    while(token != NULL)
                    {
    					tokens[NN] = token;
    					token = strtok(NULL, " ");
    					NN++;
                    }

                    //now get the url

          			string url(tokens[1]);
          			string http_body_extract;
          			cout<<"Requested Page: "<<url<<endl;
                    //if we find the page we are looking for in cache we use found_in_cache
          			if(cache.count(url) != 0)
          			{       int now = 1;
            				HTTP_pointer* ptr_found = found_in_cache(url,now);
                            		http_body_extract = ptr_found->get_http_body();//returns the http response body

            				send(i, http_body_extract.c_str(), http_body_extract.length(),0);

            				cache_display();
          			}
          			else
          			{
                        //else we use the function not_found_in_cache
                                int now = 0;
                       			HTTP_pointer* ptr_not_found = not_found_in_cache(url,now);
                                        
 					
                        		http_body_extract = ptr_not_found->get_http_body(); //returns the http body
					cout<<http_body_extract.length()<<endl<<endl<<endl<<endl<<endl;
					
                        		send(i, http_body_extract.c_str(), http_body_extract.length(),0);

                        		cache_display();
          			}
			}
      		}
    	}
  }

  close(fd_server);
  return 0;
  }
  else
  {
    perror("Usage:./server server-IP server-Port");
    exit(1);
  }
  return -1;
}



 // method to add new entries to the cache using LRU status 0->add 1->update/change/reinsert/delete when stale

void update_cache(string url, HTTP_pointer* entity,int status)
{
  if(status == 0)
  {
  	//first check if cache has empty places
  	double MTD=INT_MIN;//minimum access time
  	time_t currentTime = time_current();
  	if(cache.size() < MAX)
  	{
    		//just insert it and return
    		cout<<"Current size of cache: "<<cache.size()<<endl;
    		entity = set_Time(entity);
    		cache.insert(make_pair(url, entity));
    		return;
  	}

  //now if cache is full, find the entry with the oldest access time
  //
  	int i=0;

  	CACHE_LIST::iterator Least_recent_key;
  	CACHE_LIST::iterator N = cache.begin();
  	time_t last;
	for(; N != cache.end(); N++)
  	{
  		HTTP_pointer* en = N->second;
    		last = en->getHeader()->get_last_accessed();
    		double timediff = difftime(currentTime, last);
    		if(timediff > MTD)
    		{
      			MTD = timediff;//find the minima
      			Least_recent_key = N;
    		}
    		i++;
  	}

  //now we have the least recently used key in index leastRecent
  	cout<<"The least recently used(LRU) page is "<<Least_recent_key->first<<", hence this entry will be deleted in the cache"<<endl;
  	cache.erase(Least_recent_key);
  	cache.insert(make_pair(url, entity));
   }
 else
 {

 //method to replace existing entries in cache

  	string key;
	CACHE_LIST::iterator N = cache.begin();

	for(; N != cache.end(); N++)
	{
		key = N->first;
		if(key == url)
		{
			cout<<"Entry found for page "<<key<<", this will be replaced with the new page now"<<endl;
			cache.erase(key);
			entity = set_Time(entity);
			cache.insert(make_pair(url, entity));
			break;
		}
	}
 }
}




/*
 * method to get a page from the web server. Now if ifModified=0 that means we are doing a normal GET request else we will do a Conditional Get request
 */
string download_WEB(string url, int* length_page,string sinceTime, int ifModified)
{
  int b1, b2;
  string servername, location, message;

  struct addrinfo *httpinfo;
  struct addrinfo hints;
  if(ifModified == 1)cout<<"The page "<<url<<" has been found in cache but it has expired"<<endl;

  b1 = url.find("/");
  if(url.at(b1) == '/' && url.at(b1+1) == '/')
  {
    b2 = url.substr(b1+2, url.length()-1).find("/");
    servername = url.substr(b1+2, b2);
    location = url.substr(b2+b1+2, url.length()-1);
  }
  else
  {
    servername = url.substr(0, b1);
    location = url.substr(b1, url.length()-1);
  }

  int httpsd;
  if((httpsd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("HTTP socket creation error\n");
    exit(1);
  }

  bzero(&hints,sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  if(getaddrinfo(servername.c_str(), "http", &hints, &httpinfo) != 0)
  {
    perror("getaddrinfo error\n");

    exit(1);
  }

  if(connect(httpsd, httpinfo->ai_addr, httpinfo->ai_addrlen) == -1)
  {
    perror("Connection to http server error\n");

    exit(1);
  }


  if(ifModified == 0)message = "GET "+url+" HTTP/1.0\r\n\r\n";
  if(ifModified ==1)message = "GET "+url+" HTTP/1.0\r\n"+"If-Modified-Since: "+sinceTime+"\r\n\r\n";
  cout<<endl<<endl;
  cout<<"Sending Get Request: "<<endl<<message;

  int bytes = 0;
  if((bytes = send(httpsd, message.c_str(), message.length(), 0)) <=0)
  {
    perror("Send error\n");

    exit(1);
  }

  bytes = 0;
  long int k = 4448576;
  char BUFFER[65536];
  char proxy_buffer[k];
  int bytes1 = 0;
  while(1)
  {
	bytes1 = bytes1+bytes;
	bytes = read(httpsd,BUFFER,65536);
	if(bytes<=0) break;
	int i;

	for(i =  0;i<=bytes-1;i++)
		proxy_buffer[bytes1+i] = BUFFER[i];

   }
   if(ifModified == 0)
   {
    
    *length_page = bytes1;
    string str(proxy_buffer, bytes1);
    string g5(str);
    TOK_LIST tl5 = seperate(g5, "\r\n");
    string status(tl5.at(0));
    TOK_LIST tl6 = seperate(status, " ");
    TOK_LIST tl13 = seperate(g5, "\r\n\r\n");
   string http_complete_response(tl13.at(0));
    if(tl6.at(1) == not_exist)
    return http_complete_response;
    else return str;
    
   }
   else
   {
  *length_page = bytes1;
  string g(proxy_buffer, bytes1);
  string g1(g);
  string g2(g);
  //just check if modified or not
  string nothing;  //return a blank if not and send the file available in server
  TOK_LIST tl = seperate(g1, "\r\n");
  string status(tl.at(0));
  TOK_LIST tl3 = seperate(g2, "\r\n\r\n");
  string http_complete_response(tl3.at(0));
  cout<<"------------------Conditional Get Header ----------------------"<<endl<<endl;
  cout<<http_complete_response<<endl<<endl;
  cout<<"From the above we see that the response from the web server is: "<<status<<endl<<endl;

  TOK_LIST tl2 = seperate(status, " ");
  if(tl2.at(1) == not_exist) return http_complete_response;
  if(tl2.at(1) == not_modified_response)
    return nothing;//this will direct the proxy to send back the stored page in cache as no modification was done
  else
    return g;
}
}


//Incase you find the requested webpage in the cache of the proxy server
HTTP_pointer* found_in_cache(string url,int do_now)
{
  int current, expire_int;
  CACHE_LIST::iterator N;
  HTTP_pointer* ptr;
  N = cache.find(url);
  ptr = N->second;

  //get the expiration time
  string expires = ptr->getHeader()->get_expires();

  //if Expires field is not there, use the Last-modified or the Date field, in that order
  if(expires.empty())
  {
    expires = ptr->getHeader()->get_last_modified();
    if(expires.empty())
      expires = ptr->getHeader()->get_date();
  }


  time_t expire_time = convert_time_format(expires); //convert to time_t format
  if(expire_time <= 0) //handle spl cases
  {

    cout<<"Format for Expires is not suitable: "<<expires;
    expire_int = -1;
  }
  else
  expire_int = int(expire_time);

  current = int(time_current());
  if(current > expire_int)
  {
    //this means that the page has expired in the cache so we will check for a 304 statement

    cout<<"Requested webpage has expired in proxy cache"<<endl;
    int* length_page;
    //we download the expired page with information on ifModified
    string expired_page = download_WEB(url, length_page, expires,do_now);
  
    //this being a conditional get, we need to make sure we need this to be downloaded again
    if(!expired_page.empty())//if we get no page ---------> no download necessary
    {
      HTTP_pointer* new_ptr = parseResponse(expired_page, *length_page);
      update_cache(url, new_ptr,1);
      cout<<"The page----->"<<url<<" has been refreshed in cache"<<endl;
      return new_ptr; //return the page from the server
    }
    else
    {

      cout<<"The page----->"<<url<<"is fresh no need to re-download------->Sending from Cache"<<endl;
      ptr = set_Time(ptr); //send from cache itself and set its last access time
      return ptr;
    }
  }
  else
  {
    //if page hasnt expired in the cache
    cout<<"The page "<<url<<" has been found in cache and there is no need to refresh the page"<<endl;
    ptr = set_Time(ptr);
    return ptr;
  }
}


//for files not found in the cache
HTTP_pointer* not_found_in_cache(string url,int do_now)
{
  int* length_page = new int;
  string dummy(" ");
  string page = download_WEB(url, length_page,dummy,do_now);//download the file with no ifModified options
  string g5(page);
    TOK_LIST tl5 = seperate(g5, "\r\n");
    string status(tl5.at(0));
    TOK_LIST tl6 = seperate(status, " ");
  
  HTTP_pointer* new_one = parseResponse(page, *length_page);
  cout<<status<<endl<<endl<<endl;
  if(tl6.at(1) != not_exist)
  {
   update_cache(url, new_one,0);                         //add a new element to cache
   cout<<"The new page--------> "<<url<<" has been stored in cache"<<endl;
  }
  return new_one;
}

void cache_display()
{
  HTTP_header* http_hdr;
  HTTP_pointer* ptr;
  CACHE_LIST::iterator N;//use a standard iterator variable to move through the cache

  string LR_key, last;
  time_t last_accessed;
  cout<<endl;
  cout<<"_____________________Files Cache____________________________"<<endl;
  N= cache.begin();
  while(N != cache.end())
  {
    LR_key = N->first;//first element in cache
    ptr = N->second;//pointer to the next

    http_hdr = ptr->getHeader();
    last_accessed = http_hdr->get_last_accessed();
    last = UTC_convert(last_accessed);

    cout<<"page: "<<LR_key<<", was last accessed at: "<<last<<endl;
    N++;
  }
}
