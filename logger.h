#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <chrono>

#include <bits/unique_ptr.h>
#include <fstream>
#define EXTENSION ".log"
class Logger
{
private:
    Logger(){};
    static std::string timestamp();

public:
    static std::string header();
    static std::string getTime();
    static std::string getTimestamp();
    static std::string record(float posCircular,float posEstimate,float posEstimateCounts,float posInTurns,float iqMeasured);
    static std::string velocity(float vel);
};

#endif // LOGGER_H
