//////////////////////////////
//Shiyang Chen
//chshysy910811@gmail.com
/////////////////////////

#include<string>

#include<vector>

#include<time.h>



using namespace std;



#define MAX 10



typedef vector<string> TOK_LIST;




//HTTP header class for storing important time information

class HTTP_header

{

  string date;

  string expires;

  string last_modified;

  time_t last_accessed;



  public:

  HTTP_header();



  void set_expires(string);
  string get_expires();

  void set_last_modified(string);
  string get_last_modified();

  void set_date(string);
  string get_date();

  void set_last_accessed(time_t);

  time_t get_last_accessed();



};
//HTTP pointer class which we use only to store data and the header portion

class HTTP_pointer

{

  HTTP_header* header;

  string body;



  public:

  HTTP_pointer();

  void set_http_header(HTTP_header*);
  HTTP_header* getHeader();

  void set_http_body(string);

  string get_http_body();



};



//Various methods that will be called upon during the program for proxy server

TOK_LIST seperate(string, string);

HTTP_pointer* parseResponse(string, int);
HTTP_pointer* set_Time(HTTP_pointer*);

string UTC_convert(time_t);

time_t convert_time_format(string);

time_t time_current();

