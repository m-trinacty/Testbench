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
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <thread>
#include <fstream>
#include <queue>
#include <vector>
#include <sys/stat.h>
#include "oDrive.h"
#include "pps.h"
#include "logger.h"
#include "helper.h"
#include "server.h"

#include <unistd.h>


#define STOP 1

std::shared_ptr<Server> server;
std::string logFilePath="/var/log/tevogs/";
std::string logFileName = "/var/log/tevogs/testbench.log";
std::ofstream logFile;
std::queue<std::string> messageQueue;
std::mutex m;

std::string semaphor;
void intHandler(int dummy) {
    logFile<<"Killed"<<std::endl;
    server->closeConnection();
}
void spin(float vel, int time){
    const std::string portName = "/dev/oDrive";
    std::unique_ptr<oDrive> odrive(new oDrive(portName));

    odrive->clearErrors(0);
    odrive->getMinEndstop(0);
    odrive->setHoming(0);

    odrive->getMinEndstop(0);
    odrive->clearErrors(0);
    odrive->setMinEndstop(0,false);

    odrive->getMinEndstop(0);
    odrive->clearErrors(0);
    odrive->setAxisState(0,odrive->AXIS_STATE_CLOSED_LOOP_CONTROL);

    float posCircular=odrive->getPosCircular(0);
    float posEstimate=odrive->getPosEstimate(0);
    float posEstimateCounts = odrive->getPosEstimateCounts(0);
    float posInTurns = odrive->getPosInTurns(0);
    float IqMeasurd = odrive->getIqMeasured(0);



    float velF=0.0;
    velF=vel;
    auto start = std::chrono::steady_clock::now();

    logFile<<Logger::header();

    logFile<<std::endl<<Logger::getTime()<<std::endl;
    logFile<<"HOMING POSITION"<<std::endl<<Logger::record(posCircular,posEstimate,posEstimateCounts,posInTurns,IqMeasurd)<<std::endl<<"HOMING POSITION"<<std::endl;


    logFile<<"Velocity = "<<std::to_string(velF)<<std::endl;
    odrive->setVelocity(0,vel);
    auto timer = std::chrono::milliseconds(10);

    while(1)
    {
        if(semaphor=="END"||semaphor=="STOP")
        {
            logFile<<"Stopping"<<std::endl;
            break;
        }

        if(std::chrono::steady_clock::now() - (start+timer) > std::chrono::milliseconds(100))
        {
            posCircular=odrive->getPosCircular(0);
            posEstimate=odrive->getPosEstimate(0);
            posEstimateCounts = odrive->getPosEstimateCounts(0);
            posInTurns = odrive->getPosInTurns(0);
            IqMeasurd= odrive->getIqMeasured(0);
            std::string log=Logger::record(posCircular,posEstimate,posEstimateCounts,posInTurns,IqMeasurd);
            server->sendMessage(log);
            logFile<<log<<std::endl;
        }
        if(std::chrono::steady_clock::now()- start> std::chrono::seconds(time))
        {
            float stopVel=odrive->getVelocity(0);
            if(stopVel<0.05||stopVel>-0.05){
                m.lock();
                semaphor="STOP";
                m.unlock();
                server->sendMessage("FINISHED");
                break;
            }
        }
    }
    odrive->setAxisState(0,odrive->AXIS_STATE_IDLE);
}
/*
void fofo(int time){
    int run=1;
    int cnt=1;
    while (run) {
        if(semaphor=="START"){
            m.lock();
            semaphor="RUN";
            m.unlock();
        }
        if(semaphor=="END"||semaphor=="STOP")
        {

            run=0;
            break;
        }
        sleep(1);
        server->sendMessage("Run");
        if(cnt==time){
            std::cout<<"Thread:END"<<std::endl;
            m.lock();
            semaphor="STOP";
            m.unlock();
            server->sendMessage("FINISHED");
        }
        cnt++;
    }
}
*/
int main(int argc,char * argv[] ) {
    //Helper::daemonize();
    Helper::setSyslog();
    if(!Helper::fileExist(logFileName)){
        syslog(LOG_INFO,"Creating directory");

        if(mkdir(logFilePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)<0){
            syslog(LOG_ERR, "Can not create folder for log file %s, error %s\n",
                   logFileName.c_str(), strerror(errno));
        }
    }
    logFile=std::ofstream(logFileName);
    logFile<<"Start"<<std::endl;
    try {
        if (!logFile) {
            syslog(LOG_ERR, "Can not open log file: %s, error: %s \n Exiting",
                   logFileName.c_str(), strerror(errno));
            return EXIT_FAILURE;

        }
        signal(SIGSEGV,intHandler);
        while (1) {

            logFile<<"Waiting for connection"<<std::endl;
            server = std::shared_ptr<Server>(new Server("192.168.151.1",1500));
            logFile<<"Server waiting"<<std::endl;
            server->createConnection();

            logFile<<"Connection created"<<std::endl;
            std::string message;
            int runServer=1;
            std::thread th;
            while (runServer)
            {
                runServer=server->handleMessage();
                message=server->getMessage();
                logFile<<message<<std::endl;
                float velocity = -1;
                int time=-1;
                if(message.substr(0,5)=="START"){
                    std::vector<std::string> argss=Helper::splitString(message);
                    velocity= stof(argss.at(1));
                    time = stoi(argss.at(2));
                }

                if((message.substr(0,5)=="START" && time>0 && velocity>0)&& semaphor!="RUN")
                {
                    m.lock();
                    semaphor="START";
                    m.unlock();
                    th = std::thread(spin,velocity,time);
                }
                else if(message.substr(0,5)=="START" && semaphor == "RUN"){
                    server->sendMessage("THREAD ALREADY RUNNING");

                }
                if(message.substr(0,4)=="STOP")
                {
                    m.lock();
                    semaphor="STOP";
                    m.unlock();
                    server->sendMessage("STOPING");
                }
                if(message.substr(0,4)=="QUIT")
                {
                    m.lock();
                    semaphor="STOP";
                    m.unlock();
                    break;
                }
                if(semaphor=="END"||semaphor=="STOP"){
                    if(th.joinable()){
                        th.join();

                    }
                }
            }

            logFile<<"Closing Connection"<<std::endl;
            server->closeConnection();
            //server.reset();
        }
    }  catch (std::exception& e) {
        logFile<< "Exception caught : " << e.what() << std::endl;
        server->closeConnection();
    }



    return 0;
};
