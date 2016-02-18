//////////////////////////////
//Shiyang Chen
//chshysy910811@gmail.com
/////////////////////////
#include<string.h>

#include<iostream>

#include<sstream>

#include<map>

#include<stdio.h>
#include<stdlib.h>

#include<vector>

#include<time.h>

#include"proxy.h"





using namespace std;



//storing our target tags



string lastMod("Last-Modified");
string expires("Expires");

string date("Date");

char* format = "%a, %d %b %Y %H:%M:%S %Z";



//class of storing the particulars of http headers

HTTP_header::HTTP_header()

{

}



void HTTP_header::set_expires(string exp)

{

  expires = exp;

}



void HTTP_header::set_date(string s)

{

  date = s;

}



void HTTP_header::set_last_modified(string s)

{

  last_modified = s;

}



void HTTP_header::set_last_accessed(time_t t)

{

  last_accessed = t;

}









string HTTP_header::get_last_modified()

{

  return last_modified;

}

string HTTP_header::get_expires()

{

  return expires;

}




string HTTP_header::get_date()

{

  return date;

}



time_t HTTP_header::get_last_accessed()

{

  return last_accessed;

}







//class of HTTP_pointer for storing the header and body as well as retrieving them



HTTP_pointer::HTTP_pointer()

{

}



void HTTP_pointer::set_http_header(HTTP_header* hdr)

{

  header = hdr;

}





HTTP_header* HTTP_pointer::getHeader()

{

  return header;

}


void HTTP_pointer::set_http_body(string s)

{

  body = s;

}



string HTTP_pointer::get_http_body()

{

  return body;

}









//parsing function

HTTP_pointer* parseResponse(string response, int responseLen)

{

  HTTP_pointer* pointer = new HTTP_pointer;

  HTTP_header* hdr = new HTTP_header;

  TOK_LIST tags;
  //we have 3 kinds of delimitors. We just need to find the \r\n\r\n delimitor and then use the \r\n to extricate the delim1 and : for the final extrication

  string main_delim("\r\n\r\n");
  string delim1("\r\n");
  string delim2(": ");

  string response_server = response;

  TOK_LIST segments = seperate(response, main_delim);



  if(segments.size() == 1)//this is to deal exclusively with 404 errors where we will be sending client the header

  {
    string resp = response;
    pointer->set_http_header(hdr);
    pointer->set_http_body(resp);
    cout<<resp<<endl<<endl<<endl;
   return pointer;

  }



  //now take the header and break it up

  string httpHdr = segments.at(0);
  cout<<httpHdr<<endl;



  TOK_LIST html_tags = seperate(httpHdr, delim1);
  int num_tags = html_tags.size();

  if(num_tags < 1)

  {

    cout<<"No tags/fields found in html header"<<endl;

    exit(10);

  }



  //interate through the html tags/fields extracted and store the expires time, modified time and date
  //if expires is there it will set that
  //if expires is absent and we have last modified field we will use that
  //if both are absent we have no idea if the file is new or not so we will download the file anyway



  for(int i=1; i<num_tags; i++)

  {



    tags = seperate(html_tags.at(i), delim2);

    if(tags.at(0) == expires)
       hdr->set_expires(tags.at(1));

    else if(tags.at(0) == lastMod)

       hdr->set_last_modified(tags.at(1));

    else if(tags.at(0) == date)

      hdr->set_date(tags.at(1));



  }



  //set the last accessed time as the current time

  time_t current = time_current();

  string httpBody = segments.at(1);




  //now set the body

  hdr->set_last_accessed(current);

  pointer->set_http_body(response_server);

  pointer->set_http_header(hdr);





  return pointer;

}


//Implementation of file seperation into header and body

TOK_LIST seperate(string data, string delimitor)

{

  TOK_LIST tokens;
  int size = data.length();

  int begin = 0;

  int size_delimitor =  delimitor.length();

  size_t seeker;

  int found;

  string token;int differ = 0;

  while(size>begin)

  {

    seeker =data.find(delimitor, begin);

    if(seeker == string::npos)

    {

      tokens.push_back(data.substr(begin));//same implementation is used by string tokenizer in JAVA

      break;

    }



    found = int(seeker);
    differ = found-begin;//find all the characters in between and sperate them


    token = data.substr(begin, differ);

    tokens.push_back(token);//use push back



    begin =  size_delimitor + found;

  }



  return tokens;

}




/*

 * converts a string to a time_t object which holds the local time

 */

time_t convert_time_format(string t)

{
  struct tm time1;
  time_t time_object;

  char* value = strptime(t.c_str(), format, &time1);

  if(t.empty()||value == NULL)

  {

    cout<<"The time string is null or format is not recognized"<<endl;

    return -1;

  }

  time_object = timegm(&time1);

  return time_object;

}





/*

 * display a time_t object in UTC

 */

string UTC_convert(time_t t)

{


  struct tm* time = gmtime(&t);

  char time_string[30];



  strftime(time_string, 30, format, time);

  string UTC(time_string, 30);

  return UTC;

}



//function to get the current time

time_t time_current()

{

  struct tm* time1;

  time_t current,utc;
  current = time(NULL);

  time1 = gmtime(&current);

  utc = timegm(time1);

  return utc;

}





//Set the last accessed time for the LR cache to sort

HTTP_pointer* set_Time(HTTP_pointer* ptr)

{

  HTTP_header* http_hdr = ptr->getHeader();

  http_hdr->set_last_accessed(time_current());

  ptr->set_http_header(http_hdr);

  return ptr;

}
