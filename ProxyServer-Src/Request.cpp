#include "Request.hpp"

Type EnumConverter(std::string type)
{
    if(type=="POST")
    return Type::POST;
    else if(type=="CONNECT")
    return Type::CONNECT;
    else if(type=="GET")
    return Type::GET;

    return Type::INVALID;
}
Request::Request(std::string catched)
{
    parse(catched);

}

void Request::parse(std::string str)
{   
    int i=0;
    std::string requestType;
    std::string hostLine;
    m_req=str;
    m_statusLine=str.substr(0,str.find("\r\n"));
    requestType=str.substr(0,str.find(" "));
    m_reqType=EnumConverter(requestType);
    int pos=str.find("Host: ");
    hostLine=str.substr(pos+6);
    hostLine=hostLine.substr(0,hostLine.find("\r\n"));

    pos=hostLine.find(":");
    if(pos!=std::string::npos)
    {
        m_host=hostLine.substr(0,pos);
        m_port=hostLine.substr(pos+1);
    }
    else 
    {
        m_port="";
        m_host=hostLine;
    }
    

}

std::string Request:: getHost()
{
    return m_host;
}

std::string Request:: getPort()
{
    return m_port;
}

std::string Request:: getRequest()
{
    return m_req;
}

Type Request:: getReqType()
{
    return m_reqType;
}


std::string Request::getStatusLine()
{
    return m_statusLine;
}

std::string Request::getHeader()
{
    std::string delim="\r\n\r\n";
    int poz=m_req.find(delim);

    return m_req.substr(0,poz);
}