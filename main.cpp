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

void intHandler(int dummy) {
    logFile<<"Killed"<<std::endl;
    server->closeConnection();
}
void spin(){
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
    std::string msg = messageQueue.front();
    int spacePos=msg.find(" ");
    velF=stof(msg.substr(spacePos,2));
    m.lock();
    messageQueue.pop();
    m.unlock();

    auto start = std::chrono::steady_clock::now();

    logFile<<Logger::header();

    logFile<<std::endl<<Logger::getTime()<<std::endl;
    logFile<<"HOMING POSITION"<<std::endl<<Logger::record(posCircular,posEstimate,posEstimateCounts,posInTurns,IqMeasurd)<<std::endl<<"HOMING POSITION"<<std::endl;


    logFile<<"Velocity = "<<std::to_string(velF)<<std::endl;

    auto timer = std::chrono::milliseconds(10);

    while(1)
    {
        if(!messageQueue.empty()&& messageQueue.front()=="STOP"){

            logFile<<"Stopping"<<std::endl;
            break;
            m.lock();
            messageQueue.pop();
            m.unlock();
        }
        if(std::chrono::steady_clock::now() - (start+timer) > std::chrono::milliseconds(1))
        {
            posCircular=odrive->getPosCircular(0);
            posEstimate=odrive->getPosEstimate(0);
            posEstimateCounts = odrive->getPosEstimateCounts(0);
            posInTurns = odrive->getPosInTurns(0);
            IqMeasurd= odrive->getIqMeasured(0);

            logFile<<Logger::record(posCircular,posEstimate,posEstimateCounts,posInTurns,IqMeasurd)<<std::endl;
        }
        if(std::chrono::steady_clock::now()- start> std::chrono::seconds(15))
        {
            float stopVel=odrive->getVelocity(0);
            if(stopVel<0.05||stopVel>-0.05)
                break;
        }
    }
    odrive->setAxisState(0,odrive->AXIS_STATE_IDLE);
}

void fofo(){
    int run=1;
    while (run) {
        if(!messageQueue.empty()&& messageQueue.front().substr(0,5)=="START"){

            std::cout<<"Thread:"<<messageQueue.front()<<std::endl;
            logFile<<"Thread:"<<messageQueue.front()<<std::endl;
            m.lock();
            messageQueue.pop();
            m.unlock();
        }
        if(!messageQueue.empty()&& messageQueue.front()=="STOP"){

            std::cout<<"Thread: Stopping"<<std::endl;

            logFile<<"Thread:"<<"STOP"<<std::endl;
            run=0;
            m.lock();
            messageQueue.pop();
            m.unlock();
        }

        logFile<<"Thread:"<<"RUN"<<std::endl;
        if(server != NULL)
        server->sendMessage("Run");
        sleep(2);
    }
}

int main(int argc,char * argv[] ) {
    //Helper::daemonize();
    Helper::setSyslog();
    mkdir(logFilePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    logFile=std::ofstream(logFileName);
    logFile<<"Start"<<std::endl;
    try {
        if (!logFile) {
            syslog(LOG_ERR, "Can not open log file: %s, error: %s \n Exiting",
                   logFileName.c_str(), strerror(errno));

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
                //std::cout<<"SUBSTRING:"<<message.substr(0,5)<<" "<<message.substr(6,2) << " "<<message.substr(9,3) <<std::endl;
                float velocity = -1;
                int time=-1;
                if(message.substr(0,5)=="START"){
                    std::vector<std::string> argss=Helper::splitString(message);
                    velocity= stof(argss.at(1));
                    time = stoi(argss.at(2));
                }

                std::cout<<"SUBSTRING:"<<" "<<velocity << " "<<time<<std::endl;
                if((message.substr(0,5)=="START" && time>0 && velocity>0 )&& !th.joinable())
                {
                    m.lock();
                    messageQueue.push(message);
                    m.unlock();
                    server->sendMessage("STARTING");
                    th = std::thread(fofo);
                }
                if(message.substr(0,4)=="STOP" && th.joinable())
                {

                    m.lock();
                    messageQueue.push(message);
                    m.unlock();
                    server->sendMessage("STOPING");
                    th.join();
                }
                if(message.substr(0,4)=="QUIT")
                {
                    if(th.joinable()){
                        m.lock();
                        messageQueue.push("STOP");
                        m.unlock();
                        th.join();
                    }

                    break;
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
