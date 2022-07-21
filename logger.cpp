#include "logger.h"

#include <fstream>
#include <string>
#include <chrono>
#include <iosfwd>
#include <sstream>
#include <iomanip>

/*!
 * \brief       logger::timestamp
 * \details     This function return string casted from seconds from epoch to "%H:%M:%S" format with milliseconds
 * \return      String with classic time format
 * \retval      "%H:%M:%S" format with milliseconds
 */
std::string logger::timestamp()
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
/*!
 * \brief       logger::header
 * \details     This function defines type of data that are then logged in log file to clearly see
 *              what is logged.
 * \return      String with data categories
 * \retval      "TIME,POS_CIRCULAR,POS_ESTIMATE,POS_ESTIMATE_COUNTS,POS_INTURNS,IQMEASURED;"
 */
std::string logger::header()
{
    std::string logString;
    logString = logString+ "TIME,POS_CIRCULAR,POS_ESTIMATE,POS_ESTIMATE_COUNTS,POS_INTURNS,IQMEASURED;"+"\n";
    return logString;
}
/*!
 * \brief       logger::get_time
 * \details     This function return string casted from seconds from epoch to "%Y-%m-%d %X" format
 * \return      String with year, month, day and daytime
 * \retval      "%Y-%m-%d %X" format
 */
std::string logger::get_time()
{

    auto now=std::chrono::system_clock::now();
    auto nowTime =  std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss<<std::put_time(std::localtime(&nowTime),"%Y-%m-%d %X");
    std::string logString;
    logString = logString + ss.str();
    return logString;
}
/*!
 * \brief       logger::get_timestamp
 * \details     Function returns string with time in format as seconds from epoch.
 * \return      String representing seconds from epoch
 * \retval      Seconds from epoch
 */
std::string logger::get_timestamp()
{

    std::string time;
    //uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::time_t timeStamp = std::time(nullptr);
    time = timeStamp;
    return time;
}
/*!
 * \brief       logger::create_rec
 * \details     Function takes data from ODrive convert it to string and format it as one row which is then logged into log file
 * \param       posCircular         Circular position
 * \param       posEstimate         Estimated position
 * \param       posEstimateCounts   Estimated position counts
 * \param       posInTurns          Position in turns
 * \param       iqMeasured          Current flowing thru motor
 * \return      Function return string as one row which is saved to log file
 * \retval      "timestamp,posCircular,posEstimate,posEstimateCounts,posInTurns,iqMeasured"
 */
std::string logger::create_rec(float posCircular, float posEstimate, float posEstimateCounts, float posInTurns, float iqMeasured)
{
    std::string record = ", "+std::to_string(posCircular)+", "+std::to_string(posEstimate)+", "+std::to_string(posEstimateCounts)+", "+std::to_string(posInTurns)+", "+std::to_string(iqMeasured)+";";

    std::string logString;
    std::string timeStamp = timestamp();
    logString = logString +timeStamp + record;
    return logString;
}
