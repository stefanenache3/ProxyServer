#include <cstdio>
#include <cstdlib>
#include <string>

using namespace std;

class Timer
{
private:
    //Obs: daca timpul de expirare este mai mare decat aceasta valoare , consideram ca nu expira nicioadata
   
public:
    static std::string getCurrentDateTime(); // returneaza timpul local curent ca STRING
    static double getCurrentSec(); // ret timpul local curent ca numar
    static std::string getlocalTimeStr(long timeNum); //convertor number --> string

};