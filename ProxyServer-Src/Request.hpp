#include <stdio.h>
#include <string>

#ifndef REQ
#define REQ
enum class Type
{ GET,POST,CONNECT,INVALID};
class Request
{   
    private:
    /*header items*/          
    std::string m_host;
    std::string m_port;
    std::string m_statusLine;
        /**/
    Type m_reqType;
    std::string m_req;  

    private:
    void parse(std::string str);
    public:
    Request(std::string catched);
    std::string getHost();
    std::string getPort();
    std::string getRequest();
    std::string getStatusLine();
    std::string getHeader();
    Type getReqType();


};

#endif