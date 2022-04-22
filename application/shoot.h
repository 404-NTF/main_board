/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       shoot.c/h
  * @brief      ������ܡ�
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. ���
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */

#ifndef SHOOT_H
#define SHOOT_H
#include "struct_typedef.h"

#include "CAN_receive.h"
#include "gimbal_task.h"
#include "remote_control.h"
#include "user_lib.h"



//������俪��ͨ������
#define SHOOT_RC_MODE_CHANNEL       1
//��̨ģʽʹ�õĿ���ͨ��

#define SHOOT_CONTROL_TIME          GIMBAL_CONTROL_TIME

//���Ħ���ּ��⿪��
#define SHOOT_KEYBOARD              KEY_PRESSED_OFFSET_F
//���ֿ���
#define BULLET_BOX_CONTROL          KEY_PRESSED_OFFSET_R

//��곤���ж�
#define PRESS_LONG_TIME             400
//ң����������ش��µ�һ��ʱ��� ���������ӵ� �����嵥
#define RC_S_LONG_TIME              2000
//Ħ�����ٶ�
#define FRIC_SPEED_HIGH             8000
#define FRIC_SPEED_LOW              3000
#define FRIC_SPEED_OFF              0
//Ħ����ֹͣʱ��
#define FRIC_STOP_TIME              8000
//�����������ֵ��Χ
#define HALF_ECD_RANGE              4096
#define ECD_RANGE                   8191
//���rmp �仯�� ��ת�ٶȵı���
#define MOTOR_RPM_TO_SPEED          0.00290888208665721596153948461415f
#define MOTOR_ECD_TO_ANGLE          0.000021305288720633905968306772076277f
#define FULL_COUNT                  18
//�����ٶ�
#define TRIGGER_SPEED               10.0f
#define CONTINUE_TRIGGER_SPEED      15.0f
#define ONCE_TRIGGER_SPEED          5.0f
//���β���ʱ��
#define ONCE_TRIGGER_TIME           100

#define KEY_OFF_JUGUE_TIME          500
#define SWITCH_TRIGGER_ON           0
#define SWITCH_TRIGGER_OFF          1

//����ʱ�� �Լ���תʱ��
#define BLOCK_TRIGGER_SPEED         1.0f
#define BLOCK_TIME                  700
#define REVERSE_TIME                500
#define REVERSE_SPEED_LIMIT         13.0f

#define PI_FOUR                     0.78539816339744830961566084581988f
#define PI_TEN                      0.314f

//�����ֵ��PID
#define TRIGGER_ANGLE_PID_KP        800.0f
#define TRIGGER_ANGLE_PID_KI        0.5f
#define TRIGGER_ANGLE_PID_KD        0.0f

#define TRIGGER_BULLET_PID_MAX_OUT  10000.0f
#define TRIGGER_BULLET_PID_MAX_IOUT 9000.0f

#define TRIGGER_READY_PID_MAX_OUT   10000.0f
#define TRIGGER_READY_PID_MAX_IOUT  7000.0f


#define SHOOT_HEAT_REMAIN_VALUE     80

typedef enum
{
    SHOOT_STOP = 0,
    SHOOT_READY,
    SHOOT_BULLET,
    SHOOT_BULLET_ONCE,
    SHOOT_CONTINUE_BULLET,
} shoot_mode_e;

typedef enum
{
    FRIC_MODE_HIGH = 0,
    FRIC_MODE_LOW,
    FRIC_MODE_OFF,
    FRIC_MODE_STOP,
} fric_mode_e;

typedef enum
{
    BOX_OFF_DONE = 0,
    BOX_OFF,
    BOX_ON_DONE,
    BOX_ON,
} bullet_box_e;

typedef struct
{
    shoot_mode_e shoot_mode;
    const RC_ctrl_t *shoot_rc;
    const motor_measure_t *shoot_motor_measure;
    pid_type_def trigger_motor_pid;
    bullet_box_e bullet_box_mode;
    fric_mode_e fric_mode;
    fp32 trigger_speed_set;
    fp32 speed;
    fp32 speed_set;
    fp32 angle;
    fp32 set_angle;
    int16_t given_current;
    int8_t ecd_count;

    bool_t press_l;
    bool_t press_r;
    bool_t last_press_l;
    bool_t last_press_r;
    uint16_t rc_s_time;
    uint16_t press_r_time;

    uint16_t fric_off_time;
    uint16_t bullet_time;
    uint16_t block_time;
    uint16_t reverse_time;

    uint16_t heat_limit;
    uint16_t heat;
} shoot_control_t;

//�����������̨ʹ��ͬһ��can��id��Ҳ�����������̨������ִ��
extern void shoot_init(void);
extern int16_t shoot_control_loop(void);

#endif
