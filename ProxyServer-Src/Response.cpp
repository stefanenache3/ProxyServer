#include "Response.hpp"
#include "http-utils.hpp"
Response::Response(const std::string& response,Type type,const double arrived):m_response(response),m_type(type),m_arriveTime(arrived)
{
    m_expireTime=Utils::ResponseParser::getExpireTime(m_response);
    m_eTAG=Utils::ResponseParser::getETAG(m_response);
    m_revalidate=Utils::ResponseParser::needsValidation(m_response);

}
double Response::getExpireTime()
{
    return m_expireTime;
}

bool Response::getRevalidate()
{
    return m_revalidate;
}

std::string Response::getResponse()
{
    return m_response;
}

std::string Response:: canCache()
{
    if(m_type!=Type::GET)
    {
        return "NOT A GET REQUEST";
    }
    else if(m_response.find("HTTP/1.1 200 OK")==std::string::npos){
        return "RESPONSE ERROR, NOT OK";
    }
    else if(Utils::ResponseParser::canCache(m_response)!="Can cache")
    {
        return "NO CACHE";
    }
    else 
    return "CAN CACHE";
}