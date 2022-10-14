/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       shoot.c/h
  * @brief      射击功能.
  * @note
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. 完成
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */

#include "shoot.h"
#include "main.h"

#include "cmsis_os.h"

#include "bsp_usart.h"
#include "arm_math.h"
#include "user_lib.h"
#include "referee.h"

#include "CAN_receive.h"
#include "gimbal_behaviour.h"
#include "detect_task.h"
#include "pid.h"

#define shoot_laser_on()    laser_on()      //激光开启宏定义
#define shoot_laser_off()   laser_off()     //激光关闭宏定义
#define bullet_box_on()     pwm_on()        //弹仓开启宏定义
#define bullet_box_off()    pwm_off()       //弹仓关闭宏定义


/**
  * @brief          射击状态机设置，遥控器上拨一次开启，再上拨关闭，下拨1次发射1颗，一直处在下，则持续发射，用于3min准备时间清理子弹
  * @param[in]      void
  * @retval         void
  */
static void shoot_set_mode(void);
/**
  * @brief          射击数据更新
  * @param[in]      void
  * @retval         void
  */
static void shoot_feedback_update(void);

/**
  * @brief          堵转倒转处理
  * @param[in]      void
  * @retval         void
  */
static void trigger_motor_turn_back(void);

/**
  * @brief          射击控制，控制拨弹电机角度，完成一次发射
  * @param[in]      void
  * @retval         void
  */
static void shoot_bullet_control(void);



shoot_control_t shoot_control;          //射击数据


/**
  * @brief          射击初始化，初始化PID，遥控器指针，电机指针
  * @param[in]      void
  * @retval         返回空
  */
void shoot_init(void)
{

    static const fp32 Trigger_speed_pid[3] = {TRIGGER_ANGLE_PID_KP, TRIGGER_ANGLE_PID_KI, TRIGGER_ANGLE_PID_KD};
    shoot_control.shoot_mode = SHOOT_STOP;
    //遥控器指针
    shoot_control.shoot_rc = get_remote_control_point();
    //电机指针
    shoot_control.shoot_motor_measure = get_trigger_motor_measure_point();
    //初始化PID
    PID_init(&shoot_control.trigger_motor_pid, PID_POSITION, Trigger_speed_pid, TRIGGER_READY_PID_MAX_OUT, TRIGGER_READY_PID_MAX_IOUT);
    //更新数据
    shoot_feedback_update();
    shoot_control.fric_mode = FRIC_MODE_STOP;
    shoot_control.bullet_box_mode = BOX_OFF_DONE;
    shoot_control.laser_mode = LASER_MODE_OFF;
    shoot_control.fric_off_time = 0;
    shoot_control.ecd_count = 0;
    shoot_control.angle = shoot_control.shoot_motor_measure->ecd * MOTOR_ECD_TO_ANGLE;
    shoot_control.given_current = 0;
    shoot_control.set_angle = shoot_control.angle;
    shoot_control.speed = 0.0f;
    shoot_control.speed_set = 0.0f;
}

/**
  * @brief          射击循环
  * @param[in]      void
  * @retval         返回can控制值
  */
int16_t shoot_control_loop(void)
{

    shoot_set_mode();        //设置状态机
    shoot_feedback_update(); //更新数据

    if (shoot_control.bullet_box_mode == BOX_OFF)
    {
        bullet_box_off();
        shoot_control.bullet_box_mode = BOX_OFF_DONE;
    }
    else if (shoot_control.bullet_box_mode == BOX_ON)
    {
        bullet_box_on();
        shoot_control.bullet_box_mode = BOX_ON_DONE;
    }

    if (shoot_control.shoot_mode == SHOOT_STOP)
    {
        //设置拨弹轮的速度
        shoot_control.speed_set = 0.0f;
    }
    else if(shoot_control.shoot_mode == SHOOT_READY)
    {
        shoot_control.speed_set = 0.0f;
        shoot_control.trigger_motor_pid.max_out = TRIGGER_BULLET_PID_MAX_OUT;
        shoot_control.trigger_motor_pid.max_iout = TRIGGER_BULLET_PID_MAX_IOUT;
    }
    else if (shoot_control.shoot_mode == SHOOT_BULLET_ONCE)
    {
        shoot_control.trigger_speed_set = ONCE_TRIGGER_SPEED;
        shoot_bullet_control();
    }
    else if (shoot_control.shoot_mode == SHOOT_BULLET)
    {
        shoot_control.trigger_speed_set = TRIGGER_SPEED;
        trigger_motor_turn_back();
    }
    else if (shoot_control.shoot_mode == SHOOT_CONTINUE_BULLET)
    {
        //设置拨弹轮的拨动速度,并开启堵转反转处理
        shoot_control.trigger_speed_set = CONTINUE_TRIGGER_SPEED;
        trigger_motor_turn_back();
    }

    if(shoot_control.shoot_mode == SHOOT_STOP)
    {
        if (shoot_control.laser_mode == LASER_MODE_ON)
        {
            shoot_laser_off(); //激光关闭
            shoot_control.laser_mode = LASER_MODE_OFF;
        }
        shoot_control.given_current = 0;
        if (shoot_control.fric_mode < FRIC_MODE_OFF)
        {
            shoot_control.fric_mode = FRIC_MODE_OFF;
        }
    }
    else
    {
        if (shoot_control.laser_mode == LASER_MODE_OFF)
        {
            shoot_laser_on(); //激光开启
            shoot_control.laser_mode = LASER_MODE_ON;
        }
        //计算拨弹轮电机PID
        PID_calc(&shoot_control.trigger_motor_pid, shoot_control.speed, shoot_control.speed_set);
        shoot_control.given_current = (int16_t)(shoot_control.trigger_motor_pid.out);
        if(shoot_control.shoot_mode < SHOOT_BULLET)
        {
            shoot_control.given_current = 0;
        }

        shoot_control.fric_mode = FRIC_MODE_ON;
    }

    switch (shoot_control.fric_mode)
    {
        case FRIC_MODE_ON:
            CAN_cmd_fric(FRIC_SPEED_ON, -FRIC_SPEED_ON);
            break;
        case FRIC_MODE_OFF:
            CAN_cmd_fric(-FRIC_SPEED_BACK, FRIC_SPEED_BACK);
            if (shoot_control.fric_off_time == FRIC_STOP_TIME)
            {
                shoot_control.fric_mode = FRIC_MODE_STOP;
            }
            break;
        case FRIC_MODE_STOP:
        default:
            CAN_cmd_fric(FRIC_SPEED_OFF, FRIC_SPEED_OFF);
            break;
    }

    if (shoot_control.fric_mode == FRIC_MODE_OFF)
    {
        shoot_control.fric_off_time++;
    }
    else
    {
        shoot_control.fric_off_time = 0;
    }
    return shoot_control.given_current;
}

/**
  * @brief          射击状态机设置，遥控器上拨一次开启，再上拨关闭，下拨1次发射1颗，一直处在下，则持续发射，用于3min准备时间清理子弹
  * @param[in]      void
  * @retval         void
  */
static void shoot_set_mode(void)
{
    static int8_t last_s = RC_SW_UP;
    static uint16_t last_v = 0;
    bool_t chassis_stop = gimbal_cmd_to_shoot_stop();

    //弹仓开关判断
    if (!chassis_stop && shoot_control.shoot_mode == SHOOT_STOP && ((switch_is_down(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_down(last_s)) || ((shoot_control.shoot_rc->key.v & BULLET_BOX_CONTROL) && !(last_v & BULLET_BOX_CONTROL))))
    {
        if (shoot_control.bullet_box_mode == BOX_ON_DONE)
        {
            shoot_control.bullet_box_mode = BOX_OFF;
        }
        else if (shoot_control.bullet_box_mode == BOX_OFF_DONE)
        {
            shoot_control.bullet_box_mode = BOX_ON;
        }
    }

    //上拨判断， 一次开启，再次关闭
    if ((switch_is_up(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_up(last_s) && shoot_control.shoot_mode == SHOOT_STOP))
    {
        shoot_control.shoot_mode = SHOOT_READY;
    }
    else if ((switch_is_up(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_up(last_s) && shoot_control.shoot_mode != SHOOT_STOP))
    {
        shoot_control.shoot_mode = SHOOT_STOP;
    }

    //处于中档， 可以使用键盘开关摩擦轮
    if (switch_is_mid(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && (shoot_control.shoot_rc->key.v & SHOOT_KEYBOARD) && !(last_v & SHOOT_KEYBOARD))
    {
        if (shoot_control.shoot_mode == SHOOT_STOP)
        {
            shoot_control.shoot_mode = SHOOT_READY;
        }
        else
        {
            shoot_control.shoot_mode = SHOOT_STOP;
        }
    }

    if(shoot_control.shoot_mode == SHOOT_READY)
    {
        //下拨一次或者鼠标按下一次，进入射击状态
        if ((switch_is_down(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_down(last_s)) || (shoot_control.press_r && shoot_control.last_press_r == 0))
        {
            shoot_control.shoot_mode = SHOOT_BULLET;
        }
        else if (shoot_control.press_l && shoot_control.last_press_l == 0)
        {
            shoot_control.shoot_mode = SHOOT_BULLET_ONCE;
        }
    }
    else if (shoot_control.shoot_mode > SHOOT_READY)
    {
        //拨中间退出射击状态
        if (switch_is_mid(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && switch_is_down(last_s))
        {
            shoot_control.shoot_mode = SHOOT_READY;
        }
    }

    if (shoot_control.shoot_mode > SHOOT_READY)
    {
        //鼠标长按一直进入射击状态 保持连发
        if ((shoot_control.press_r_time == PRESS_LONG_TIME) || (shoot_control.rc_s_time == RC_S_LONG_TIME))
        {
            shoot_control.shoot_mode = SHOOT_CONTINUE_BULLET;
        }
        else if(!switch_is_down(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !shoot_control.press_l && !shoot_control.press_r)
        {
            shoot_control.shoot_mode = SHOOT_READY;
        }
    }

    get_shoot_heat0_limit_and_heat0(&shoot_control.heat_limit, &shoot_control.heat);
    if(!toe_is_error(REFEREE_TOE) && (shoot_control.heat + SHOOT_HEAT_REMAIN_VALUE > shoot_control.heat_limit))
    {
        if(shoot_control.shoot_mode > SHOOT_READY)
        {
            shoot_control.shoot_mode = SHOOT_READY;
        }
    }
    //如果云台状态是 无力状态，就关闭射击
    if (chassis_stop)
    {
        shoot_control.shoot_mode = SHOOT_STOP;
    }

    last_s = shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL];
    last_v = shoot_control.shoot_rc->key.v;
}
/**
  * @brief          射击数据更新
  * @param[in]      void
  * @retval         void
  */
static void shoot_feedback_update(void)
{

    static fp32 speed_fliter_1 = 0.0f;
    static fp32 speed_fliter_2 = 0.0f;
    static fp32 speed_fliter_3 = 0.0f;

    //拨弹轮电机速度滤波一下
    static const fp32 fliter_num[3] = {1.725709860247969f, -0.75594777109163436f, 0.030237910843665373f};

    //二阶低通滤波
    speed_fliter_1 = speed_fliter_2;
    speed_fliter_2 = speed_fliter_3;
    speed_fliter_3 = speed_fliter_2 * fliter_num[0] + speed_fliter_1 * fliter_num[1] + (shoot_control.shoot_motor_measure->speed_rpm * MOTOR_RPM_TO_SPEED) * fliter_num[2];
    shoot_control.speed = speed_fliter_3;

    //电机圈数重置， 因为输出轴旋转一圈， 电机轴旋转 36圈，将电机轴数据处理成输出轴数据，用于控制输出轴角度
    if (shoot_control.shoot_motor_measure->ecd - shoot_control.shoot_motor_measure->last_ecd > HALF_ECD_RANGE)
    {
        shoot_control.ecd_count--;
    }
    else if (shoot_control.shoot_motor_measure->ecd - shoot_control.shoot_motor_measure->last_ecd < -HALF_ECD_RANGE)
    {
        shoot_control.ecd_count++;
    }

    if (shoot_control.ecd_count == FULL_COUNT)
    {
        shoot_control.ecd_count = -(FULL_COUNT - 1);
    }
    else if (shoot_control.ecd_count == -FULL_COUNT)
    {
        shoot_control.ecd_count = FULL_COUNT - 1;
    }

    //计算输出轴角度
    shoot_control.angle = (shoot_control.ecd_count * ECD_RANGE + shoot_control.shoot_motor_measure->ecd) * MOTOR_ECD_TO_ANGLE;
    //鼠标按键
    shoot_control.last_press_l = shoot_control.press_l;
    shoot_control.last_press_r = shoot_control.press_r;
    shoot_control.press_l = shoot_control.shoot_rc->mouse.press_l;
    shoot_control.press_r = shoot_control.shoot_rc->mouse.press_r;
    //右键长按计时
    if (shoot_control.press_r)
    {
        if (shoot_control.press_r_time < PRESS_LONG_TIME)
        {
            shoot_control.press_r_time++;
        }
    }
    else
    {
        shoot_control.press_r_time = 0;
    }

    //射击开关下档时间计时
    if (shoot_control.shoot_mode != SHOOT_STOP && switch_is_down(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]))
    {
        if (shoot_control.rc_s_time < RC_S_LONG_TIME)
        {
            shoot_control.rc_s_time++;
        }
    }
    else
    {
        shoot_control.rc_s_time = 0;
    }
    //单发射击计时
    if (shoot_control.shoot_mode == SHOOT_BULLET_ONCE)
    {
        if (shoot_control.bullet_time < ONCE_TRIGGER_TIME)
        {
            shoot_control.bullet_time++;
        }
    }
    else
    {
        shoot_control.bullet_time = 0;
    }

}

static void trigger_motor_turn_back(void)
{
    if( shoot_control.block_time < BLOCK_TIME)
    {
        shoot_control.speed_set = shoot_control.trigger_speed_set;
    }
    else
    {
        shoot_control.speed_set = -shoot_control.trigger_speed_set;
    }

    if(fabs(shoot_control.speed) < BLOCK_TRIGGER_SPEED && shoot_control.block_time < BLOCK_TIME)
    {
        shoot_control.block_time++;
        shoot_control.reverse_time = 0;
    }
    else if (shoot_control.block_time == BLOCK_TIME && shoot_control.reverse_time < REVERSE_TIME)
    {
        shoot_control.reverse_time++;
    }
    else
    {
        shoot_control.block_time = 0;
    }
}

/**
  * @brief          射击控制，控制拨弹电机角度，完成一次发射
  * @param[in]      void
  * @retval         void
  */
static void shoot_bullet_control(void)
{
    if( shoot_control.block_time < BLOCK_TIME)
    {
        shoot_control.speed_set = shoot_control.trigger_speed_set;
    }
    else
    {
        shoot_control.speed_set = -shoot_control.trigger_speed_set;
    }

    if(fabs(shoot_control.speed) < BLOCK_TRIGGER_SPEED && shoot_control.block_time < BLOCK_TIME)
    {
        shoot_control.block_time++;
        shoot_control.reverse_time = 0;
    }
    else if (shoot_control.block_time == BLOCK_TIME && shoot_control.reverse_time < REVERSE_TIME)
    {
        shoot_control.bullet_time--;
        shoot_control.reverse_time++;
    }
    else
    {
        shoot_control.block_time = 0;
    }

    if (shoot_control.bullet_time == ONCE_TRIGGER_TIME)
    {
        shoot_control.shoot_mode = SHOOT_READY;
    }
}

