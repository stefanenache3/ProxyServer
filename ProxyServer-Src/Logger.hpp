#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <mutex>
#include <netdb.h>
#include <arpa/inet.h>

#ifndef logger_h
#define logger_h 1

class Request;
class Logger
{

    static void writeLogFile(std::string line);
public:
    static void logRequest(int pairID, Request &req);
    static void logResponse(int pairID, std::string &resp, std::string host);

    static void logCacheHit(int pairID);
    static void logCacheActions(std::string str,int pairId);
};

#endif