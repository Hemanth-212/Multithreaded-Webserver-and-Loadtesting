#include "http_server.hh"
#include <vector>
#include <sys/stat.h>
#include <fstream>
#include <sstream>

vector<string> split(const string &s, char delim) {
  vector<string> elems;

  stringstream ss(s);
  string item;

  while (getline(ss, item, delim)) 
  {
    if (!item.empty())
      elems.push_back(item);
  }

  return elems;
}

HTTP_Request::HTTP_Request(string request) {
    vector<string> lines = split(request, '\n');
    vector<string> initial_line = split(lines[0], ' ');
    //vector<string> host_line = split(lines[2],' ');
    //vector<string> connection_line = split(lines[5],' ');

    this->method = initial_line[0];
    this->url = initial_line[1];
    this->HTTP_version = "1.0"; // Irrespective of the request only 1.0 version will be used
    //this->Host = host_line[1];
    //this->connection = connection_line[1];

    if (this->method != "GET") 
    {
        cerr << "Method '" << this->method << "' not supported" << endl;
        exit(1);
    }
}

HTTP_Response *handle_request(string req) {
  HTTP_Request *request = new HTTP_Request(req);

  HTTP_Response *response = new HTTP_Response();

  string url = string("html_files") + request->url;

  response->HTTP_version = "HTTP/1.0";

  struct stat sb;
  if (stat(url.c_str(), &sb) == 0) // requested path exists
  {
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";
    response->connection = "Closed";

    string body;

    if (S_ISDIR(sb.st_mode)) {

        url = url + "/index.html";
    }
    ifstream htmlfile(url);
    string responseBody,str;
    while(getline(htmlfile,str))
    {
        responseBody = responseBody + str + "\n";
    }
    responseBody.pop_back();
    response->body = responseBody;
    response->content_length = to_string(responseBody.size());

  }

  else 
  {

    response->status_code = "404";
    response->status_text = "NOT FOUND";
    response->content_type = "text/html";
    response->connection = "Closed";
    string NFbody;
    string nfURL = "./404body.html";
    ifstream nfHTML(nfURL);
    string str;
    while(getline(nfHTML,str))
    {
      NFbody = NFbody + str + "\n";
    }
    NFbody.pop_back();
    response->body = NFbody;
    response->content_length = to_string(NFbody.size());
  }

  delete request;

  return response;
}

string HTTP_Response::get_string() {
  string res;
  res = res + this->HTTP_version + " ";
  res = res + this->status_code + " ";
  res = res + this->status_text + "\r\n";
  res = res + "Content-Length: " +this->content_length + "\r\n";
  res = res + "Connection: " + this->connection + "\r\n";
  res = res + "Content-Type: " + this-> content_type + "\r\n\r\n";
  res = res + this->body;

 return res;
}