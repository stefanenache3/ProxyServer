#include "Timer.hpp"
#include <time.h>
std::string Timer::getCurrentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return std::string(buf);
}
// copiem in buf prin formatele de mai sus , contentul aflat in tstruct - local time current

double Timer::getCurrentSec()
{
    return time(0);
}
// pur si simplu returnam timpul curent ca numar

