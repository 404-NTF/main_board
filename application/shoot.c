/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       shoot.c/h
  * @brief      �������.
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
#include "bsp_led.h"

#define shoot_laser_on()    laser_on()      //���⿪���궨��
#define shoot_laser_off()   laser_off()     //����رպ궨��
#define bullet_box_on()     pwm_on()        //���ֿ����궨��
#define bullet_box_off()    pwm_off()       //���ֹرպ궨��


/**
  * @brief          ���״̬�����ã�ң�����ϲ�һ�ο��������ϲ��رգ��²�1�η���1�ţ�һֱ�����£���������䣬����3min׼��ʱ�������ӵ�
  * @param[in]      void
  * @retval         void
  */
static void shoot_set_mode(void);
/**
  * @brief          ������ݸ���
  * @param[in]      void
  * @retval         void
  */
static void shoot_feedback_update(void);

/**
  * @brief          ��ת��ת����
  * @param[in]      void
  * @retval         void
  */
static void trigger_motor_turn_back(void);

/**
  * @brief          ������ƣ����Ʋ�������Ƕȣ����һ�η���
  * @param[in]      void
  * @retval         void
  */
static void shoot_bullet_control(void);



shoot_control_t shoot_control;          //�������
fric_control_t fric_control;          //�������


/**
  * @brief          �����ʼ������ʼ��PID��ң����ָ�룬���ָ��
  * @param[in]      void
  * @retval         ���ؿ�
  */
void shoot_init(void)
{

    static const fp32 Trigger_speed_pid[3] = {TRIGGER_ANGLE_PID_KP, TRIGGER_ANGLE_PID_KI, TRIGGER_ANGLE_PID_KD};
    static const fp32 Fric_speed_pid[3] = {FRIC_PID_KP, FRIC_PID_KI, FRIC_PID_KD};
    shoot_control.shoot_mode = SHOOT_STOP;
    //ң����ָ��
    shoot_control.shoot_rc = get_remote_control_point();
    //���ָ��
    shoot_control.shoot_motor_measure = get_trigger_motor_measure_point();
    //��ʼ��PID
    PID_init(&shoot_control.trigger_motor_pid, PID_POSITION, Trigger_speed_pid, TRIGGER_READY_PID_MAX_OUT, TRIGGER_READY_PID_MAX_IOUT);
    uint8_t i;
    for (i = 0; i < 2; i++) {
        PID_init(&fric_control.fric_motor_pid[i], PID_POSITION, Fric_speed_pid, FRIC_PID_MAX_OUT, FRIC_PID_MAX_IOUT);
        fric_control.fric_moter[i].fric_motor_measure = get_fric_motor_measure_point(i);
        fric_control.fric_moter[i].give_current = 0;
        fric_control.fric_moter[i].speed = 0.0f;
    }
    fric_control.speed_set = 0.0f;
    //��������
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
  * @brief          ���ѭ��
  * @param[in]      void
  * @retval         ����can����ֵ
  */
int16_t shoot_control_loop(void)
{

    shoot_set_mode();        //����״̬��
    shoot_feedback_update(); //��������

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
        //���ò����ֵ��ٶ�
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
        //���ò����ֵĲ����ٶ�,��������ת��ת����
        shoot_control.trigger_speed_set = CONTINUE_TRIGGER_SPEED;
        trigger_motor_turn_back();
    }

    if(shoot_control.shoot_mode == SHOOT_STOP)
    {
        if (shoot_control.laser_mode == LASER_MODE_ON)
        {
            shoot_laser_off(); //����ر�
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
            shoot_laser_on(); //���⿪��
            shoot_control.laser_mode = LASER_MODE_ON;
        }
        //���㲦���ֵ��PID
        PID_calc(&shoot_control.trigger_motor_pid, shoot_control.speed, shoot_control.speed_set);
        shoot_control.given_current = (int16_t)(shoot_control.trigger_motor_pid.out);
        if(shoot_control.shoot_mode < SHOOT_BULLET)
        {
            shoot_control.given_current = 0;
        }

        shoot_control.fric_mode = FRIC_MODE_ON;
    }

    if (shoot_control.fric_mode == FRIC_MODE_ON) {
        fric_control.speed_set = FRIC_SPEED_ON;
    } else {
        fric_control.speed_set = FRIC_SPEED_OFF;
    }
    

    if (shoot_control.fric_mode == FRIC_MODE_OFF) {
        shoot_control.fric_off_time++;
    } else {
        shoot_control.fric_off_time = 0;
    }

    uint8_t i;
    for (i = 0; i < 2; i++) {
        PID_calc(&fric_control.fric_motor_pid[i], fric_control.fric_moter[i].speed, fric_control.speed_set);
        fric_control.fric_moter[i].give_current = fric_control.fric_motor_pid[i].out;
    }

    CAN_cmd_fric(fric_control.fric_moter[0].give_current, -fric_control.fric_moter[1].give_current);    

    return shoot_control.given_current;
}

/**
  * @brief          ���״̬�����ã�ң�����ϲ�һ�ο��������ϲ��رգ��²�1�η���1�ţ�һֱ�����£���������䣬����3min׼��ʱ�������ӵ�
  * @param[in]      void
  * @retval         void
  */
static void shoot_set_mode(void)
{
    static int8_t last_s = RC_SW_UP;
    static uint16_t last_v = 0;
    bool_t chassis_stop = gimbal_cmd_to_shoot_stop();

    //���ֿ����ж�
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

    //�ϲ��жϣ� һ�ο������ٴιر�
    if ((switch_is_up(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_up(last_s) && shoot_control.shoot_mode == SHOOT_STOP))
    {
        shoot_control.shoot_mode = SHOOT_READY;
    }
    else if ((switch_is_up(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_up(last_s) && shoot_control.shoot_mode != SHOOT_STOP))
    {
        shoot_control.shoot_mode = SHOOT_STOP;
    }

    //�����е��� ����ʹ�ü��̿���Ħ����
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
        //�²�һ�λ�����갴��һ�Σ��������״̬
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
        //���м��˳����״̬
        if (switch_is_mid(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && switch_is_down(last_s))
        {
            shoot_control.shoot_mode = SHOOT_READY;
        }
    }
    
    if (shoot_control.shoot_mode > SHOOT_READY)
    {
        //��곤��һֱ�������״̬ ��������
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
    //�����̨״̬�� ����״̬���͹ر����
    if (chassis_stop)
    {
        shoot_control.shoot_mode = SHOOT_STOP;
    }

    last_s = shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL];
    last_v = shoot_control.shoot_rc->key.v;
}
/**
  * @brief          ������ݸ���
  * @param[in]      void
  * @retval         void
  */
static void shoot_feedback_update(void)
{

    static fp32 speed_fliter_1 = 0.0f;
    static fp32 speed_fliter_2 = 0.0f;
    static fp32 speed_fliter_3 = 0.0f;

    //�����ֵ���ٶ��˲�һ��
    static const fp32 fliter_num[3] = {1.725709860247969f, -0.75594777109163436f, 0.030237910843665373f};

    //���׵�ͨ�˲�
    speed_fliter_1 = speed_fliter_2;
    speed_fliter_2 = speed_fliter_3;
    speed_fliter_3 = speed_fliter_2 * fliter_num[0] + speed_fliter_1 * fliter_num[1] + (shoot_control.shoot_motor_measure->speed_rpm * MOTOR_RPM_TO_SPEED) * fliter_num[2];
    shoot_control.speed = speed_fliter_3;

    fric_control.fric_moter[0].speed = FRIC_MOTOR_RPM_TO_VECTOR_SEN * fric_control.fric_moter[0].fric_motor_measure->speed_rpm;
    fric_control.fric_moter[1].speed = -FRIC_MOTOR_RPM_TO_VECTOR_SEN * fric_control.fric_moter[1].fric_motor_measure->speed_rpm;
    
    //���Ȧ�����ã� ��Ϊ�������תһȦ�� �������ת 36Ȧ������������ݴ������������ݣ����ڿ��������Ƕ�
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

    //���������Ƕ�
    shoot_control.angle = (shoot_control.ecd_count * ECD_RANGE + shoot_control.shoot_motor_measure->ecd) * MOTOR_ECD_TO_ANGLE;
    //��갴��
    shoot_control.last_press_l = shoot_control.press_l;
    shoot_control.last_press_r = shoot_control.press_r;
    shoot_control.press_l = shoot_control.shoot_rc->mouse.press_l;
    shoot_control.press_r = shoot_control.shoot_rc->mouse.press_r;
    //�Ҽ�������ʱ
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

    //��������µ�ʱ���ʱ
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
    //���������ʱ
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
  * @brief          ������ƣ����Ʋ�������Ƕȣ����һ�η���
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

