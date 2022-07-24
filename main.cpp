/*
 * Copyright (C) Aero4TE, s.r.o. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */



#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <mutex>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <thread>
#include <fstream>
#include <queue>
#include <vector>
#include <sys/stat.h>
#include "oDrive.h"
#include "logger.h"
#include "helper.h"
#include "server.h"

#include <unistd.h>


#define STOP 1

std::shared_ptr<server> server_port;                                                        /*!< Global variable for Server class */
std::string logFilePath="/var/log/tevogs/";                                                 /*!< Global variable for path where log file is placed */
std::string logFileName = "/var/log/tevogs/testbench.log";                                  /*!< Global variable for absolute path to log file */
std::ofstream logFile;                                                                      /*!< Global variable for stream which is writing to log file*/
std::mutex m;                                                                               /*!< Global variable for mutex for prevent data ovewrite by threads */

std::string semaphor;                                                                       /*!< Global variable for string representin semaphor to run or stop testbench*/

/*!
 * \brief intHandler
 * \details This function is called when application is manually stopped.
 */
void intHandler(int dummy) {
    logFile<<"Killed"<<std::endl;
    server_port->dconn_server();
}
/*!
 * \brief spin
 * \details Spin function does data logging from ODrive, starting and stopping testbench and desired velocity for desired time.
 *          At first, function finds base position by homing. Then homing position is logged. Then function sets AXIS_STATE_CLOSED_LOOP_CONTROL
 *          and velocity passed as parameter. After that testbench starts spinning. Every 100 milliseconds are position data logged. After
 *          time passes, Velocity is set to 0 and position data are logging until actual velocity is close to 0. Then function sets AXIS_STATE_IDLE, so no force is affecting motor.
 * \param vel   Velocity in rotation per second on motor
 * \param time  Time motor forces testbench to spin
 *
 */
void spin(float vel, int time){
    const std::string portName = "/dev/oDrive";
    std::unique_ptr<oDrive> odrive(new oDrive(portName));

    odrive->clear_errors(0);                                                                /** Homing testbench*/
    odrive->get_min_endstop(0);
    odrive->homing(0);

    odrive->get_min_endstop(0);
    odrive->clear_errors(0);
    odrive->set_min_endstop(0,false);

    odrive->get_min_endstop(0);
    odrive->clear_errors(0);
    odrive->set_axis_state(0,odrive->AXIS_STATE_CLOSED_LOOP_CONTROL);                       /** Set AXIS_STATE_CLOSED_LOOP_CONTROL to prepare for spinning*/

    float posCircular=odrive->get_pos_cir(0);                                               /** Observing starting position and current data*/
    float posEstimate=odrive->get_pos_est(0);
    float posEstimateCounts = odrive->get_pos_est_cnt(0);
    float posInTurns = odrive->get_pos_turns(0);
    float IqMeasurd = odrive->get_curr_Iq(0);



    float velF=0.0;
    velF=vel;
    auto start = std::chrono::steady_clock::now();

    logFile<<logger::header();

    logFile<<std::endl<<logger::get_time()<<std::endl;                                      /** Write to log file homing position data and then velocity*/
    logFile<<"HOMING POSITION"<<std::endl<<logger::create_rec(posCircular,posEstimate,posEstimateCounts,posInTurns,IqMeasurd)<<std::endl<<"HOMING POSITION"<<std::endl;


    logFile<<"Velocity = "<<std::to_string(velF)<<std::endl;
    odrive->set_vel(0,vel);
    auto timer = std::chrono::milliseconds(10);

    while(1)
    {
        if(semaphor=="END"||semaphor=="STOP")                                               /** If semaphorn is changed to STOP or END sign, exit this loop*/
        {
            logFile<<"Stopping"<<std::endl;
            break;
        }

        if(std::chrono::steady_clock::now() - (start+timer) > std::chrono::milliseconds(100))/** Every 100 milliseconds capture data from ODrive*/
        {
            posCircular=odrive->get_pos_cir(0);
            posEstimate=odrive->get_pos_est(0);
            posEstimateCounts = odrive->get_pos_est_cnt(0);
            posInTurns = odrive->get_pos_turns(0);
            IqMeasurd= odrive->get_curr_Iq(0);
            std::string log=logger::create_rec(posCircular,posEstimate,posEstimateCounts,posInTurns,IqMeasurd);
            server_port->send_msg(log);
            logFile<<log<<std::endl;
        }
        if(std::chrono::steady_clock::now()- start> std::chrono::seconds(time))             /** If time parameter in seconds is reached, set velocity to zero and then stop
                                                                                                after reaching actual zero velocity logging data*/
        {
            float stopVel=odrive->get_vel(0);
            if(stopVel<0.05||stopVel>-0.05){
                m.lock();
                semaphor="STOP";
                m.unlock();
                server_port->send_msg("FINISHED");
                break;
            }
        }
    }
    odrive->set_axis_state(0,odrive->AXIS_STATE_IDLE);                                      /** Set axis state to idle so no force affect testbench*/
}
/*!
 * \brief   main
 * \details Main function tries to create log file for logging position data from ODrive if it does not already exist
 *          Then it waits until connection is created and then in infinete loop are read messages from client which can start and stop spinning testbench
 * \return  Main function returns status code
 * \retval  EXIT_SUCCESS in case in succesfull executing main function
 * \retval  EXIT_FAILURE in case of fail in main function
 */
int main() {
    helper::cfg_syslog();                                                                   /** Configure syslog*/
    if(!helper::check_file(logFileName)){                                                   /** Try to create a file for logging at "/var/log/tevogs/testbench.log" */
        syslog(LOG_INFO,"Creating directory");

        if(mkdir(logFilePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)<0){
            syslog(LOG_ERR, "Can not create folder for log file %s, error %s\n",
                   logFileName.c_str(), strerror(errno));
        }
    }
    logFile=std::ofstream(logFileName);
    logFile<<"Start"<<std::endl;
    try {
        if (!logFile) {                                                                     /** Check if the file for logging at "/var/log/tevogs/testbench.log" can be opened */
            syslog(LOG_ERR, "Can not open log file: %s, error: %s \n Exiting",
                   logFileName.c_str(), strerror(errno));
            return EXIT_FAILURE;

        }
        signal(SIGSEGV,intHandler);
        while (1) {                                                                         /** Infinite loop which should never be exited */

            logFile<<"Waiting for connection"<<std::endl;
            server_port = std::shared_ptr<server>(new server("192.168.151.1",1500));        /** Create listening socket server at defined adress and port*/
            logFile<<"Server waiting"<<std::endl;
            server_port->conn_server();                                                     /** Connect to server*/

            logFile<<"Connection created"<<std::endl;
            std::string message;
            int runServer=1;
            std::thread th;                                                                 /** Create second thread for spinning function to run at*/
            while (runServer)                                                               /** Endless loop until connection breaks/client disconnect*/
            {
                runServer=server_port->handler_msg();                                       /** In the beging of every loop check for incoming message*/
                message=server_port->get_msg();
                logFile<<message<<std::endl;
                float velocity = -1;
                int time=-1;
                if(message.substr(0,5)=="START"){                                           /** START (velocity) (time in sec) is only allowed messag with more words.
                                                                                               Here is the message parsed to concrete variables*/
                    std::vector<std::string> argss=helper::split_str(message);
                    velocity= stof(argss.at(1));
                    time = stoi(argss.at(2));
                }

                if((message.substr(0,5)=="START" && time>0 && velocity>0)&& semaphor!="RUN")/** In case of Start message and testbench is not running, start a thread and start spining*/
                {
                    m.lock();
                    semaphor="START";
                    m.unlock();
                    th = std::thread(spin,velocity,time);
                }
                else if(message.substr(0,5)=="START" && semaphor == "RUN"){                 /** If testbench is already running, do nothing*/
                    server_port->send_msg("THREAD ALREADY RUNNING");

                }
                if(message.substr(0,4)=="STOP")                                             /** In case of STOP message, stop testbench, do not disconnect*/
                {
                    m.lock();
                    semaphor="STOP";
                    m.unlock();
                    server_port->send_msg("STOPING");
                }
                if(message.substr(0,4)=="QUIT")                                             /** In case of QUIT message stop testbench and disconnect*/
                {
                    m.lock();
                    semaphor="STOP";
                    m.unlock();
                    break;
                }
                if(semaphor=="END"||semaphor=="STOP"){                                      /** In case of discconecting, join the thread so it can be reused*/
                    if(th.joinable()){
                        th.join();

                    }
                }
            }

            logFile<<"Closing Connection"<<std::endl;
            server_port->dconn_server();
        }
    }  catch (std::exception& e) {
        logFile<< "Exception caught : " << e.what() << std::endl;
        server_port->dconn_server();
    }



    return EXIT_SUCCESS;
};
