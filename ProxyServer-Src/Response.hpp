#include <iostream>
#include <string>
#include "Request.hpp"
#ifndef RESPONSE
#define RESPONSE
class Response
{
    private:
    std::string m_response;
    Type m_type;
    double m_arriveTime;
    double m_expireTime;
    bool m_revalidate;
    std::string m_eTAG;
    public:
    Response(const std::string& response,Type type,const double arrived);
    double getExpireTime();
    bool getRevalidate();
    std::string getResponse();
    std::string canCache();
};

#endif