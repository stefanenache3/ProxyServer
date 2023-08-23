#include <iostream>
#include <unordered_map>
#include <list>
#include <map>
#include "Response.hpp"
#ifndef CACHE
#define CACHE


class Item
{
    public:
    std::string m_key;
    Response m_resp;
    std::unordered_map<std::string,std::string> m_values;

    public:
    Item(const std::string& key,const Response& resp);
    Item(const std::string& key,const Response& resp, const std::unordered_map<std::string,std::string> info);
};

class WebCache
{
    private:

    std::map<std::string,std::list<Item>::iterator> iMap;
    std::list<Item> itemList;
    size_t m_cacheSize;

    private:
    WebCache(size_t size);
    WebCache(const WebCache&){;}

    static WebCache* instance;
    public:
    static WebCache& createInstance(size_t size);
    static WebCache& getInstance();
    void loadInCache(const std::string key,Response& resp);
    std::string getCached(const std::string& key, const double rTime);
};

#endif