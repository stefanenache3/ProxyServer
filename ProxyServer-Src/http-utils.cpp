#include "http-utils.hpp"
#include "Proxy.hpp"
#include <ctime>
#include <vector>
#include <iomanip>
#include <sstream>
#define RECV_BUFF_LENGTH 65536
int Utils::ReadClientRequest(int fd, std::string &request)
{
    int bytesRecieved = 0;
    char temp[2001];
    std::string reqEnd = "\r\n\r\n";
    std::string aux;

    while (true)
    {
        memset(temp, '\0', sizeof(temp));
        bytesRecieved = recv(fd, temp, 2000, (int)NULL);
        if (bytesRecieved == -1)
            return -1;

        aux = temp;
        if (aux.find(reqEnd) != std::string::npos)
        {
            request += aux;
            return true;
        }
        else
            request += aux;
    }
}

int Utils::RecieveUnChunkedResponse(int sock, std::string &resp)
{
    int allLen = 0;
    int hasGot=resp.length();
    int numbytes = 0;
    if (resp.find("HTTP/1.1 200 OK") == std::string::npos)
    {
      allLen = 0;
    }
    else
    {
      int len = Utils::ResponseParser::getResponseLength(resp);
      if (len < 0)
      {
        return false;
      }
      int headlen = Utils::ResponseParser::getHeaderLength(resp);
      if (headlen < 0)
      {
        return false;
      }
      allLen = headlen + len;
    }
    int i = 0;
    while (hasGot < allLen)
    {
      std::vector<char> temp;
      temp.resize(65536);
      if ((numbytes = recv(sock, &temp[0], 65535, 0)) == -1)
      {
        std::perror("recv");
        return false;
      }
      hasGot += numbytes;
      temp.resize(numbytes);
      std::string tempStr(temp.begin(), temp.end());
      if (hasGot >= allLen)
      {
        resp += tempStr;
        break;
      }
      else
      {
        resp += tempStr;
      }
      i++;
    }
    return true;
}
int Utils::RecieveChunked(int client, int server, std::string &resp)
{
    if (ForwardHTTP(client, resp) == -1)
        return -1;

     std::string endMark = "0\r\n\r\n";
    while (true)
    {
      char temp[65536];
      int numbytes = 0;
      if ((numbytes = recv(server, temp, 65536, 0)) == -1)
      {
        std::perror("chuncked recv server");
        return false;
      }
      if (send(client, temp, numbytes, 0) == -1)
      {
        std::perror("chuncked send to client");
        return false;
      }
      std::string tempStr(temp);
      if (tempStr.find(endMark) != std::string::npos || numbytes == 0)
      { 
        return true;
      }
    }
    return true;
}

int Utils::ResponseParser::getResponseLength(std::string resp)
{
    int poz;
    std::string aux;
    std::string pattern;
    pattern = "Content-Length: ";
    poz = resp.find(pattern.c_str());
    if (poz == std::string::npos)
    {
        std::cerr << "Cant find the length";
        return -1;
    }

    resp = resp.substr(poz + pattern.length());
    aux = resp.substr(0, aux.find("\n"));

    try
    {
        poz = std::stoi(aux);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Bad Content-Length argument for stoi" << '\n';
    }

    return poz;
}

int Utils::ResponseParser::getHeaderLength(std::string &resp)
{
    int poz;
    poz = resp.find("\r\n\r\n");

    if (poz == std::string::npos)
    {
        std::cerr << "Invalid response";
        return -1;
    }

    return poz;
}

int Utils::ForwardHTTP(int sock, const std::string &request)
{
    int len = request.length();
    int bytesSent = 0;
    int ret = 0;
    while (bytesSent < len)
    {
        ret = send(sock, request.c_str() + bytesSent, len - bytesSent, 0);
        if (ret < 0)
        {
            perror("send");
            return ret;
        }
        bytesSent += ret;
    }

    return true;
}
int Utils::SendString(int sock, std::string str)
{
    int len = str.length();
    int bytesSent = 0;
    int ret;

    while (bytesSent < len)
    {
        ret = send(sock, str.c_str() + bytesSent, len - bytesSent, 0);
        if (ret == -1)
        {
            perror("send");
            return -1;
        }
        bytesSent += ret;
    }

    return 1;
}
int Utils::ResponseParser::ChunckedResponse(std::string resp)
{
    if (resp.find("Transfer-Encoding:") != std::string::npos)
    {
        if (resp.find("chunked") != std::string::npos)
        {
            return true;
        }
    }

    return false;
}
int Utils::RecieveString(int sock, std::string &str)
{
    int totalLength = 0, headerLength = 0, contentLength = 0;
    int bytesRecv = 0;
    int aux;
    str = "";
    char buff[RECV_BUFF_LENGTH + 1];

    bytesRecv = recv(sock, buff, RECV_BUFF_LENGTH, 0);
    str += buff;
    if (bytesRecv < 0)
    {
        std::perror("Response recv");
        return -1;
    }

    return 1;
}
int Utils::RecieveResponse(int sock, std::string &resp)
{
    int totalLength = 0, headerLength = 0, contentLength = 0;
    int bytesRecv = 0;
    int aux;
    resp = "";
    char buff[RECV_BUFF_LENGTH + 1];

    bytesRecv = recv(sock, buff, RECV_BUFF_LENGTH, 0);
    if (bytesRecv < 0)
    {
        std::perror("Response recv");
        return -1;
    }
    resp += buff;
    if (Utils::ResponseParser::ChunckedResponse(resp))
        return CHUNKED_RESPONSE;

    return bytesRecv;
}

int Utils::EstablishHTTPTunnel(int sock)
{
    if (Utils::SendString(sock, "HTTP/1.0 200  CONNECTION ESTABLISHED\r\n\r\n") == -1)
    {
        std::cerr << "Error creating HTTP tunnel";
        return -1;
    }

    return 1;
}
int Utils::HTTPTunneling(int clientSock, int serverSock)
{

    int maxSock;
    if (clientSock > serverSock)
        maxSock = clientSock;
    else
        maxSock = serverSock;
    fd_set sockSet_org;
    FD_ZERO(&sockSet_org);
    FD_SET(clientSock, &sockSet_org);
    FD_SET(serverSock, &sockSet_org);

    int ret;
    char buff[RECV_BUFF_LENGTH + 1];
    while (true)
    {
        fd_set sockSet_tmp = sockSet_org;
        if (select(maxSock + 1, &sockSet_tmp, NULL, NULL, NULL) == -1)
        {
            std::cerr << "Connect // Select\n";
            return -1;
        }

        for (int i = 0; i <= maxSock; i++)
        {
            if (FD_ISSET(i, &sockSet_tmp))
            {
                if (i == clientSock)
                {

                    // ret = Utils::RecieveResponse(clientSock, buff);
                    memset(buff, 0, RECV_BUFF_LENGTH);
                    ret = recv(clientSock, buff, RECV_BUFF_LENGTH, 0);
                    if (ret == -1)
                    {
                        std::cerr << "Browser send tunneling error";
                        return -1;
                    }
                    else if (ret == 0)
                        return 1;

                    ret = send(serverSock, buff, ret, 0);
                    if (ret == -1)
                    {
                        std::cerr << "Forwarding tunneling error";
                        return -1;
                    }
                }
                else if (i == serverSock)
                {
                    memset(buff, 0, RECV_BUFF_LENGTH);
                    ret = recv(serverSock, buff, RECV_BUFF_LENGTH, 0);
                    // ret = Utils::RecieveString(serverSock, buff);
                    if (ret == -1)
                    {
                        std::cerr << "Connect server send ";
                        return -1;
                    }
                    if (ret == 0)
                        return 1;

                    ret = send(clientSock, buff, ret, 0);
                    //  ret = Utils::ForwardHTTP(clientSock, buff);
                    if (ret == -1)
                    {
                        std::cerr << "Forwarding back tunneling error";
                        return -1;
                    }
                }
            }
        }
    }

    return 1;
}

std::string Utils::ResponseParser::getFieldValue(std::string target, std::string header)
{
    size_t pos = target.find(header);
    size_t end;
    size_t len = header.length();
    std::string value;
    if (pos != std::string::npos)
    {
        end = target.find_first_of("\n", pos);
        value = target.substr(pos + len, end - pos - len);
        return value;
    }
    else
    {
        throw 100;
    }
}

double Utils::ResponseParser::getCacheExpireControl(std::string &resp)
{
    if (resp.find("max-age") != std::string::npos)
    {
        std::string aux = Utils::ResponseParser::getFieldValue(resp, "max-age=");
        return std::stod(aux);
    }
    else
    {
        return INT64_MAX;
    }
}
double Utils::ResponseParser::getDateExpireTime(std::string &resp)
{
    std::string expireHeader = Utils::ResponseParser::getFieldValue(resp, "Expires: ");
    std::string dateHeader = Utils::ResponseParser::getFieldValue(resp, "Date: ");

    std::tm date, expire;
    std::istringstream date_stream(dateHeader);
    std::istringstream expire_stream(expireHeader);

    date_stream >> std::get_time(&date, "%a, %d %b %Y %T $Z");
    expire_stream >> std::get_time(&expire, "%a, %d %b %Y %T %Z");

    std::time_t date_t = std::mktime(&date);
    std::time_t expire_t = std::mktime(&expire);

    double age = std::difftime(expire_t, date_t);

    return age;
}
double Utils::ResponseParser::getExpireTime(std::string &resp)
{
    double exp;
    try
    {
        if (resp.find("Cache-control: ") != std::string::npos)
        {
            exp = Utils::ResponseParser::getCacheExpireControl(resp);
        }
        else if (resp.find("Expires: ") != std::string::npos)
        {
            exp = Utils::ResponseParser::getDateExpireTime(resp);
        }
        else
            exp = INT64_MAX;
    }
    catch (const std::exception &e)
    {
        exp = INT64_MAX;
    }

    return exp;
}

std::string Utils::ResponseParser::getETAG(std::string &resp)
{

    try
    {
        std::string TAG = Utils::ResponseParser::getFieldValue(resp, "ETAG: ");
        return TAG;
    }
    catch (int c)
    {
        return "";
    }
}

bool Utils::ResponseParser::needsValidation(std::string &resp)
{
    bool ret = false;
    std::string field;
    if (resp.find("Cache-control") != std::string::npos)
    {
        if (resp.find("must-revalidate") != std::string::npos)
            ret = true;
    }
    if (resp.find("ETag") != std::string::npos)
    {
        ret = true;
    }
    if (resp.find("ETag") != std::string::npos)
    {
        ret = true;
    }
    if (resp.find("Last-Modified") != std::string::npos)
    {
        ret = true;
    }
    ret = true;

    return ret;
}

std::string Utils::ResponseParser::canCache(const std::string &response)
{
    if (response.find("Cache-Control") != std::string::npos)
    {
        if (response.find("no-store") != std::string::npos)
        {
            return "no-store";
        }
        else if (response.find("private") != std::string::npos)
        {
            return "private";
        }
    }
    return "Can cache";
}