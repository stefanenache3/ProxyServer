#include "Proxy.hpp"
#include "Cache.hpp"
#include <fstream>
void print_art();
int main()
{
    std::string serverIP = "0.0.0.0";
    std::string port = "12345";
    print_art();
    try
    {
        Proxy &httpProxy = Proxy::createInstance(serverIP, port);
        WebCache& cache=WebCache::createInstance(100);
        httpProxy.setListeningSocket();
        httpProxy.startHandlingConnections();
    }
    catch (int err)
    {   
        if(err==E_SINGLETONE)
            std::cerr<<"Singletion error";
        if(err==E_FORBBIDEN)
            std::cerr<<"Can't read blocked hosts";

    }
    return 0;
}

void print_art()
{
    std::ifstream f;
    std::string line;

    f.open("art.txt");

 
    if (!f) {
        std::cerr << "Unable to open file";
        return;
    }

    while (std::getline(f, line)) {
        std::cout << line << std::endl;
    }
    f.close();
}
