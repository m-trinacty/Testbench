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
    port * m_oDrive_port;
public:
    enum axis_state{
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
    enum input_mode{
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
    oDrive(std::string port_name);
    int command_console();
    virtual ~oDrive();
    int set_axis_state(int axis, int state);
    int set_input_mode(int axis, int mode);
    int set_vel(int axis, float vel);
    float get_vel(int axis);
    int set_lockin_vel(int axis, float vel);
    int get_locking_vel(int axis);
    int clear_errors(int axis);
    int get_axis_state(int axis);
    float get_pos_est(int axis);
    float get_pos_est_cnt(int axis);
    float get_pos_cir(int axis);
    float get_pos_cpr_cnt(int axis);
    float get_pos_turns(int axis);
    bool get_min_endstop(int axis);
    float get_curr(int axis);
    float get_curr_Iq(int axis);
    int set_pos_turns(int axis, float pos);
    int set_pos(int axis, float pos);
    int set_min_endstop(int axis, bool enabled);
    void homing(int axis);

};


#endif /* ODRIVE_H_ */
