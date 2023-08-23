#include "Cache.hpp"
#include <string>

WebCache* WebCache::instance=nullptr;
  WebCache& WebCache:: createInstance(size_t size)
 {
        if(instance!=nullptr)
        {
            std::cerr<<"Singleton Cache";
            //throw
        }

        instance=new WebCache(size);
        return *instance;
 }
 WebCache& WebCache:: getInstance()
 {
    if(instance==nullptr)
    {
        std::cerr<<"Cache no instance";
        //throw
    }

    return *instance;    
 }
Item::Item(const std::string& key,const Response& resp):m_key(key),m_resp(resp)
{

}

Item::Item(const std::string& key,const Response& resp, const std::unordered_map<std::string,std::string> info):m_key(key),m_resp(resp),m_values(info)
{

}

WebCache::WebCache(size_t size):m_cacheSize(size)
{

}

void WebCache::loadInCache(const std::string key,Response& resp)
{
    if(iMap.count(key)!=0)
    {
        std::unordered_map<std::string,std::string> tmp=(*iMap[key]).m_values;
        itemList.erase(iMap[key]);
        itemList.emplace_back(key,resp,tmp);
        std::list<Item>::iterator it=itemList.end();
        it--;
        iMap[key]=it;
        it->m_values["expireTime"]=std::to_string(resp.getExpireTime());
        if(resp.getRevalidate())
        {
            it->m_values["revalidate"]="true";
        }
        else
        {
            it->m_values["revalidate"]="false";
        }
    }
    else{
        if(itemList.size()>=m_cacheSize){
                iMap.erase(itemList.front().m_key);
                itemList.pop_front();
        }
        itemList.emplace_back(key,resp);
        std::list<Item>::iterator it=itemList.end();
        it--;
        iMap[key]=it;
        it->m_values["expireTime"]=std::to_string(resp.getExpireTime());
        if(resp.getRevalidate())
        {
            it->m_values["revalidate"]="true";
        }
        else{
            it->m_values["revalidate"]="false";
        }   
    }
}


std::string WebCache::getCached(const std::string& key, const double rTime)
{
    
    if(iMap.count(key)==0)
    {
        return "MISS";
    }
    else if(iMap[key]->m_values["revalidate"]=="true"){
        return "REVALIDATE";
    }
    else{
        Response tmpResp=(*iMap[key]).m_resp;
        std::string err="";
        std::unordered_map<std::string,std::string> tmpInfo=(*iMap[key]).m_values;
        if(tmpResp.getExpireTime()<rTime){
                itemList.erase(iMap[key]);
                iMap.erase(key);
                err+="RESPONSE EXPIRED:";
                err+=std::to_string(tmpResp.getExpireTime());
                return err;
        }
        else
        {
            itemList.erase(iMap[key]);
            itemList.emplace_back(key,tmpResp,tmpInfo);
            std::list<Item>::iterator it=itemList.end();
            it--;
            iMap[key]=it;
            return it->m_resp.getResponse();
        }

    }
}
