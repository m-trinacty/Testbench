#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <chrono>

#include <bits/unique_ptr.h>
#include <fstream>
#define EXTENSION ".log"
/*!
 * \brief The logger class
 * \details Logger class provides functions for unified formatting data
 *          observed from ODrive.
 */
class logger
{
private:
    static std::string timestamp();

public:
    static std::string header();
    static std::string get_time();
    static std::string get_timestamp();
    static std::string create_rec(float posCircular,float posEstimate,float posEstimateCounts,float posInTurns,float iqMeasured);
    static std::string get_vel(float vel);
};

#endif // LOGGER_H
