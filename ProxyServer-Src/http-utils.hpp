#include <thread>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <iostream>
#ifndef HTTP_H
#define HTTP_H 1

#define CHUNKED_RESPONSE 5

namespace Utils
{
    int ReadClientRequest(int fd, std::string &request);
    int ForwardHTTP(int sock, const std::string &request);
    int SendString(int sock, std::string str);
    int RecieveString(int sock, std::string &str);
    int RecieveResponse(int sock, std::string &resp);
    /***GET***/
    int RecieveUnChunkedResponse(int sock, std::string &resp);
    int RecieveChunked(int client, int server, std::string &resp);

    /***CONNECT***/
    int EstablishHTTPTunnel(int sock);
    int HTTPTunneling(int clientSock, int serverSock);
    namespace ResponseParser
    {
        int getResponseLength(std::string resp);
        int getHeaderLength(std::string &resp);
        int ChunckedResponse(std::string resp);
        bool needsValidation(std::string& resp);

        /*--------------------------------------*/
        double getExpireTime(std::string& resp);
        double getDateExpireTime(std::string& resp);
        double getCacheExpireControl(std::string& resp);
        std::string getETAG(std::string &resp);
        std::string canCache(const std::string & response);
        /*--------------------------------------*/

        std::string getFieldValue(std::string target,std::string header);
    }
}
#endif
