#include "Logger.hpp"
#include "Timer.hpp"
#include "http-utils.hpp"
#include "Request.hpp"
#include <fstream>
std::mutex logMutex;
void Logger::writeLogFile(std::string line)
{
    logMutex.lock();
    //std::lock_guard<std::mutex> lock(logMutex);
    std::string fName = "proxy.log";
    std::ofstream file(fName, ios::app);
    file << line;
    file.close();
    logMutex.unlock();
}
void Logger::logRequest(int pairID, Request &req)
{
    std::string reqLine;
    reqLine = "";

    reqLine += std::to_string(pairID);
    reqLine += "| ";
    reqLine += "Request: ";
    reqLine += req.getStatusLine();
    reqLine += " from ";

    hostent *host = gethostbyname(req.getHost().c_str());
    if (host == NULL)
    {
        std::cerr << "Invalid netdb entry";
        exit(-1);
    }

    in_addr *ip = (in_addr *)host->h_addr;
    std::string ipAddr = inet_ntoa(*ip);
    reqLine += " ";
    reqLine += ipAddr;
    reqLine += " [";
    reqLine += Timer::getCurrentDateTime();
    reqLine += "]\n";
    writeLogFile(reqLine);
}
void Logger::logCacheHit(int pairId)
{
    std::string line="In Cache";
    line+=" ";
    line+=std::to_string(pairId);
    line+="\n";
    writeLogFile(line);
}
void Logger::logResponse(int pairID, std::string &resp, std::string host)
{

   
    std::string repLine;
    repLine = "";

    repLine += std::to_string(pairID);
    repLine += "| ";
    repLine += "Response ";

    std::string status;
    status = resp.substr(0, resp.find_first_of("\r\n"));
    repLine += status;
    repLine += " from " + host + " ";
    repLine += "[" + Timer::getCurrentDateTime() + "]\n";
    writeLogFile(repLine);
}


 void Logger::logCacheActions(std::string str,int pairId)
 {
    std::string line;
    line=str;
    line+=" ";
    line+=std::to_string(pairId);
    line+="\n";
    writeLogFile(line);

 }