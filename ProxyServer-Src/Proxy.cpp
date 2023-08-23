#include "Proxy.hpp"
#include "http-utils.hpp"
#include "Request.hpp"
#include "Logger.hpp"
#include "ThreadPool.hpp"
#include "Cache.hpp"
#include "Response.hpp"
#include <fstream>
#include "Timer.hpp"

Proxy *Proxy::m_ProxyInstance = nullptr;
std::mutex cacheMutex;
Proxy &Proxy::createInstance(std::string ip, std::string port)
{
    if (m_ProxyInstance != nullptr)
    {
        throw E_SINGLETONE;
    }

    m_ProxyInstance = new Proxy(ip, port);

    return *m_ProxyInstance;
}

Proxy &Proxy::getInstance()
{
    if (m_ProxyInstance == nullptr)
    {
        throw E_SINGLETONE;
    }

    return *m_ProxyInstance;
}

void Proxy::destroyInstance()
{
    if (m_ProxyInstance == nullptr)
    {
         throw E_SINGLETONE;
        return;
    }

    delete m_ProxyInstance;
    m_ProxyInstance = nullptr;
}

Proxy::Proxy(std::string ip, std::string port) : m_address(ip), m_port(port), m_isSet(false)
{
    std::ifstream f;
    f.open("Forbbiden.txt");

    std::string aux;
    while (f >> aux)
    {
        m_blockedWebs.push_back(aux);
    }

    f.close();
}

Proxy::~Proxy()
{
}
void Proxy::setListeningSocket()
{
    if (m_isSet == true)
        return;

    int listenSock;
    int checkSockets;
    struct addrinfo address;
    struct addrinfo *list;

    memset(&address, 0, sizeof(address));
    address.ai_family = AF_UNSPEC;
    address.ai_socktype = SOCK_STREAM;
    address.ai_flags = AI_PASSIVE;

    checkSockets = getaddrinfo(m_address.c_str(), m_port.c_str(), &address, &list);

    if (checkSockets == -1)
    {
        perror("AddrInfo");
        throw E_SERVER_SOCK;

    }

    listenSock = socket(list->ai_family, list->ai_socktype, list->ai_protocol);

    if (m_listenSocket == -1)
    {
        perror("socket");
        throw E_SERVER_SOCK;
    }

    int yes = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    checkSockets = bind(listenSock, list->ai_addr, list->ai_addrlen);

    if (checkSockets == -1)
    {
        perror("bind");
        throw E_SERVER_SOCK;
        close(listenSock);
        
    }

    checkSockets = listen(listenSock, MAX_PENDING_CONNECTIONS);
    if (checkSockets == -1)
        {
            perror("listen");
            throw E_SERVER_SOCK;
        }
    m_listenSocket = listenSock;
    std::cout << "SERVER STARTED LISTENING ON PORT: " << m_port << std::endl;
    m_isSet = true;
    freeaddrinfo(list);
}

int Proxy::connectToRequestedWeb(std::string host, std::string port)
{
    struct addrinfo addr;
    struct addrinfo *list;
    int webSock;
    int check;

    memset(&addr, 0, sizeof(addr));
    addr.ai_family = AF_UNSPEC;
    addr.ai_socktype = SOCK_STREAM;

    check = getaddrinfo(host.c_str(), port.c_str(), &addr, &list);
    if (check != 0)
    {
        fprintf(stderr, "error getting address info");
        return -1;
    }

    webSock = socket(list->ai_family, list->ai_socktype, list->ai_protocol);

    if (webSock == -1)
    {
        fprintf(stderr, "error creating web socket\n");
    
        return -1;
    }

    check = connect(webSock, list->ai_addr, list->ai_addrlen);
    if (check == -1)
    {
        fprintf(stderr, "error connecting to web socket");
        return -1;
    }

    freeaddrinfo(list);
    return webSock;
}
int Proxy::CONNECT_RequestHandle(int clientSock, int serverSock, int idCon)
{
    if (Utils::EstablishHTTPTunnel(clientSock) == -1)
        return -1;

    if (Utils::HTTPTunneling(clientSock, serverSock) == -1)
        return -1;

    return 1;
}

int Proxy::GET_RequestHandle(Request &req, int clientSock, int serverSock, int idCon, std::string host)
{
    int check;
    if (Utils::ForwardHTTP(serverSock, req.getRequest()) < 0)
    {
        std::cerr << "Error forwarding request to server\n";
        return -1;
    }
    std::string rCached;
    cacheMutex.lock();
    rCached = WebCache::getInstance().getCached(req.getStatusLine(), Timer::getCurrentSec());
    cacheMutex.unlock();

    if (rCached != "MISS" && rCached != "REVALIDATE" && rCached.find("EXPIRED") == std::string::npos)
    {
        if (Utils::ForwardHTTP(clientSock, rCached) == -1)
        {
            std::cerr << "Error sending unchunked";
            return -1;
        }

        Logger::logCacheHit(idCon);
        Logger::logResponse(idCon, rCached, req.getHost());
        return 1;
    }
    else if (rCached == "MISS" || rCached.find("EXPIRED") != std::string::npos)
    {
        std::string response;
        check = Utils::RecieveResponse(serverSock, response);

        if (check == -1)
        {
            std::cerr << "Error recieving response from server";
            return -1;
        }
        else if (check == CHUNKED_RESPONSE)
        {

            if (Utils::RecieveChunked(clientSock, serverSock, response) == -1)
                return -1;
            Logger::logResponse(idCon, response, req.getHost());
        }
        else
        {

            if (Utils::RecieveUnChunkedResponse(serverSock, response) == -1)
            {
                std::cerr << "Error recieving unchunked response";
                return -1;
            }

            close(serverSock);
            if (Utils::ForwardHTTP(clientSock, response) == -1)
            {
                std::cerr << "Error sending unchunked";
                return -1;
            }
            close(clientSock);
            Logger::logResponse(idCon, response, req.getHost());
        }

        Response resp(response, Type::GET, Timer::getCurrentSec());
        if (resp.canCache() == "CAN CACHE")
        {
            cacheMutex.lock();
            WebCache::getInstance().loadInCache(req.getStatusLine(), resp);
            cacheMutex.unlock();
            if (resp.getRevalidate())
            {
                Logger::logCacheActions("Cached, but needs revalidate", idCon);
            }
            else
            {
                Logger::logCacheActions("Cached, expires at " + Timer::getCurrentDateTime(), idCon);
            }
        }
        else
        {
            Logger::logCacheActions("Cannot be cached", idCon);
        }
      
    }
      return 1;
}

int POST_RequestHandle(Request &req, int clientSock, int serverSock, int idCon, std::string host)
{
    if (Utils::ForwardHTTP(serverSock, req.getRequest()) < 0)
    {
        std::cerr << "Error forwarding request to server\n";
        return -1;
    }
    int check;
    std::string response;
    check = Utils::RecieveResponse(serverSock, response);

    if (check == -1)
    {
        std::cerr << "Error recieving response from server";
        return -1;
    }
    else if (check == CHUNKED_RESPONSE)
    {

        if (Utils::RecieveChunked(clientSock, serverSock, response) == -1)
            return -1;
        Logger::logResponse(idCon, response, req.getHost());
    }
    else
    {

        if (Utils::RecieveUnChunkedResponse(serverSock, response) == -1)
        {
            std::cerr << "Error recieving unchunked response";
            return -1;
        }

        close(serverSock);
        if (Utils::ForwardHTTP(clientSock, response) == -1)
        {
            std::cerr << "Error sending unchunked";
            return -1;
        }
        Logger::logResponse(idCon, response, req.getHost());
    }

    return 1;
}
void threadHandle(Proxy &proxy, int clientSocket, int conId)
{
    int check;
    int webSock;
    std::string httpReq;

    if (check = Utils::ReadClientRequest(clientSocket, httpReq) == -1)
        return;
    if (check == -1)
    {
        std::cerr<<"Cant read from client";
        close(clientSocket);
        return;
    }

    Request req(httpReq);
    Logger::logRequest(conId, req);
    std::string port;
    if (req.getPort().size() == 0)
        port = "80"; // if no port is specified in the request, the default HTTP port is used
    else
        port = req.getPort();

    std::cout << "\n-----------------------------------------------\n"
              << "Recieved request: \n"
              << req.getStatusLine()
              << "\n-----------------------------------------------\n";
    webSock = proxy.connectToRequestedWeb(req.getHost(), port);
    if (webSock == -1)
    {   perror("error creating connections socket\n");
        close(clientSocket);
    
        return;
    }
    for (auto a : proxy.m_blockedWebs)
    {
        if (a == req.getHost())
        {
            std::string response = "HTTP/1.1 403 Forbidden\n\n";
            response += "<html><head><title>403 Forbidden</title><style>body { font-family: sans-serif; margin: 0; padding: 0; background: url('https://t3.ftcdn.net/jpg/03/02/37/80/360_F_302378042_I4tT3YKlSNhvZWSreNzMPzbcvVrV6QvF.jpg') repeat; } #message { display: flex; align-items: center; justify-content: center; height: 100vh; width: 100vw; } #message h1 { font-size: 2em; margin: 0; padding: 0; } #message p { font-size: 1.5em; margin: 0; padding: 0; }</style></head><body><div id='message'><div><h1>403 Forbidden</h1><p> ATM Blocked this site</p></div></div></body></html>\n";
            Utils::SendString(clientSocket, response);
            close(clientSocket);
            close(webSock);
            return;
        }
    }
  
    Type header = req.getReqType();
    switch (header)
    {
    case Type::GET:
        proxy.GET_RequestHandle(req, clientSocket, webSock, conId, req.getHost());
        break;
    case Type::CONNECT:
        proxy.CONNECT_RequestHandle(clientSocket, webSock, conId);
        break;
    case Type::POST:
        proxy.GET_RequestHandle(req, clientSocket, webSock, conId, req.getHost());
        break;
    default:
        std::cerr << "Unknown request type";
        break;
    }

    close(clientSocket);
    close(webSock);
}
void Proxy::startHandlingConnections()
{
    int clientSocket;

    struct sockaddr addr;
    socklen_t s_size;
    int conId = 1;
    ThreadPool threads(100);
    while (true)
    {
        clientSocket = accept(m_listenSocket, (struct sockaddr *)&addr, &s_size);
        if (clientSocket == -1)
            continue;
     
        conId++;
        threads.add(threadHandle, clientSocket, conId);
    }
    close(clientSocket);
}