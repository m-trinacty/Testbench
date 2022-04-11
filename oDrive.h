/*
 * Copyright (C) Aero4TE, s.r.o. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef ODRIVE_H_
#define ODRIVE_H_

#include <string>
#include "port.h"


class oDrive {
private:
    port * m_oDrivePort;
public:

    oDrive();
    enum AxisState{
        AXIS_STATE_UNDEFINED,
        AXIS_STATE_IDLE,
        AXIS_STATE_STARTUP_SEQUENCE,
        AXIS_STATE_FULL_CALIBRATION_SEQUENCE,
        AXIS_STATE_MOTOR_CALIBRATION,
        AXIS_STATE_DUMMY_DO_NOT_USE,
        AXIS_STATE_ENCODER_INDEX_SEARCH,
        AXIS_STATE_ENCODER_OFFSET_CALIBRATION,
        AXIS_STATE_CLOSED_LOOP_CONTROL,
        AXIS_STATE_LOCKIN_SPIN,
        AXIS_STATE_ENCODER_DIR_FIND,
        AXIS_STATE_HOMING,
        AXIS_STATE_ENCODER_HALL_POLARITY_CALIBRATION,
        AXIS_STATE_ENCODER_HALL_PHASE_CALIBRATION
    };
    enum InputMode{
        INPUT_MODE_INACTIVE,
        INPUT_MODE_PASSTHROUGH,
        INPUT_MODE_VEL_RAMP,
        INPUT_MODE_POS_FILTER,
        INPUT_MODE_MIX_CHANNELS,//according to oDrive docs, NOT IMPLEMENTED
        INPUT_MODE_TRAP_TRAJ,
        INPUT_MODE_TORQUE_RAMP,
        INPUT_MODE_MIRROR,
        INPUT_MODE_Tuning
    };
    oDrive(std::string portName);
    int commandConsole();
    virtual ~oDrive();
    int setAxisState(int axis, int state);
    int setInputMode(int axis, int mode);
    int setVelocity(int axis, float vel);
    int setLockinVelocity(int axis, float vel);
    int clearErrors(int axis);
    int getAxisState(int axis);
    float getPosEstimate(int axis);
    float getPosEstimateCounts(int axis);
    float getPosCircular(int axis);
    float getPosCprCounts(int axis);
    int getLockinVelocity(int axis);
    float getVelocity(int axis);
    float getPosInTurns(int axis);
    bool getMinEndstop(int axis);
    float getCurrent(int axis);
    float getIqMeasured(int axis);
    int setPosInTurns(int axis, float pos);
    int setPos(int axis, float pos);    
    int setMinEndstop(int axis, bool enabled);
    void setHoming(int axis);

};


#endif /* ODRIVE_H_ */
