#include "logger.h"

#include <fstream>
#include <string>
#include <chrono>
#include <iosfwd>
#include <sstream>
#include <iomanip>


std::string Logger::timeStamp()
{
    std::string logString;
    std::stringstream ss;
    auto now=std::chrono::system_clock::now();
    auto nowTime =  std::chrono::system_clock::to_time_t(now);
    const auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count() % 1000;
    ss << std::put_time(std::localtime(&nowTime) , "%H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms ;

    logString = logString+ ss.str();
    return logString;
}

std::string Logger::header()
{
    std::string logString;
    logString = logString+ "TIME,POS_CIRCULAR,POS_ESTIMATE,POS_ESTIMATE_COUNTS,POS_INTURNS,IQMEASURED;"+"\n";
    return logString;
}

std::string Logger::getTime()
{

    auto now=std::chrono::system_clock::now();
    auto nowTime =  std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss<<std::put_time(std::localtime(&nowTime),"%Y-%m-%d %X");
    std::string logString;
    logString = logString + ss.str();
    return logString;
}

std::string Logger::getTimestamp()
{

    std::string time;
    uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    time = ms;
    return time;
}

std::string Logger::record(float posCircular, float posEstimate, float posEstimateCounts, float posInTurns, float iqMeasured)
{
    std::string record = ", "+std::to_string(posCircular)+", "+std::to_string(posEstimate)+", "+std::to_string(posEstimateCounts)+", "+std::to_string(posInTurns)+", "+std::to_string(iqMeasured)+";";

    std::string logString;
    std::string timestamp = timeStamp();
    logString = logString +timestamp + record;
    return logString;
}
