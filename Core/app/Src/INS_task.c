/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       INS_task.c/h
  * @brief      use bmi088 to calculate the euler angle. no use ist8310, so only
  *             enable data ready pin to save cpu time.enalbe bmi088 data ready
  *             enable spi DMA to save the time spi transmit
  *             主要利用陀螺仪bmi088，磁力计ist8310，完成姿态解算，得出欧拉角，
  *             提供通过bmi088的data ready 中断完成外部触发，减少数据等待延迟
  *             通过DMA的SPI传输节约CPU时间.
  * @note
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. done
  *  V2.0.0     Nov-11-2019     RM              1. support bmi088, but don't support mpu6500
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */

#include "INS_task.h"

#include "main.h"

#include "cmsis_os.h"

#include "bsp_usart.h"

static fp32 INS_gyro[3] = {0.0f, 0.0f, 0.0f};
static fp32 INS_accel[3] = {0.0f, 0.0f, 0.0f};
static fp32 INS_mag[3] = {0.0f, 0.0f, 0.0f};
static fp32 INS_quat[4] = {0.0f, 0.0f, 0.0f, 0.0f};
fp32 INS_angle[3] = {0.0f, 0.0f, 0.0f};      //euler angle, unit rad.欧拉角 单位 rad

bool_t bmi088_ist8310_inited = 0;
bool_t command_callback;

uint8_t timeout;


/**
  * @brief          imu任务, 初始化 bmi088, ist8310, 计算欧拉角
  * @param[in]      pvParameters: NULL
  * @retval         none
  */

void INS_task(void const *pvParameters)
{
    timeout = 0;

    bmi088_ist8310_init();
    while (bmi088_ist8310_inited == 0)
    {
        timeout++;
        osDelay(100);
        if (timeout > 1)
        {
            timeout = 0;
            bmi088_ist8310_init();
        }
    }
    while (1)
    {
        command_callback = 0;

        timeout = 0;
        bmi088_ist8310_read();

        while (command_callback == 0)
        {
            timeout++;
            osDelay(100);
            if (timeout > 1)
            {
                timeout = 0;
                bmi088_ist8310_read();
            }
        }
        osDelay(100);

    }
}

void set_init(void) {
    bmi088_ist8310_inited = 1;
}

void set_value(fp32 gyro[3], fp32 accel[3], fp32 mag[3], fp32 quat[4], fp32 angle[3]) {
    INS_gyro[0] = gyro[0];
    INS_gyro[1] = gyro[1];
    INS_gyro[2] = gyro[2];
    INS_accel[0] = accel[0];
    INS_accel[1] = accel[1];
    INS_accel[2] = accel[2];
    INS_mag[0] = mag[0];
    INS_mag[1] = mag[1];
    INS_mag[2] = mag[2];
    INS_quat[0] = quat[0];
    INS_quat[1] = quat[1];
    INS_quat[2] = quat[2];
    INS_quat[3] = quat[3];
    INS_angle[0] = angle[0];
    INS_angle[1] = angle[1];
    INS_angle[2] = angle[2];
    command_callback = 1;
}

/**
  * @brief          校准陀螺仪
  * @param[out]     陀螺仪的比例因子，1.0f为默认值，不修改
  * @param[out]     陀螺仪的零漂，采集陀螺仪的静止的输出作为offset
  * @param[out]     陀螺仪的时刻，每次在gyro_offset调用会加1,
  * @retval         none
  */
void INS_cali_gyro(fp32 cali_scale[3], fp32 cali_offset[3], uint16_t *time_count)
{
    INS_cali(cali_scale, cali_offset, time_count);
}

/**
  * @brief          校准陀螺仪设置，将从flash或者其他地方传入校准值
  * @param[in]      陀螺仪的比例因子，1.0f为默认值，不修改
  * @param[in]      陀螺仪的零漂
  * @retval         none
  */
void INS_set_cali_gyro(fp32 cali_scale[3], fp32 cali_offset[3])
{
    INS_set(cali_scale, cali_offset);
}

/**
  * @brief          获取四元数
  * @param[in]      none
  * @retval         INS_quat的指针
  */
const fp32 *get_INS_quat_point(void)
{
    return INS_quat;
}

/**
  * @brief          获取欧拉角, 0:yaw, 1:pitch, 2:roll 单位 rad
  * @param[in]      none
  * @retval         INS_angle的指针
  */
const fp32 *get_INS_angle_point(void)
{
    return INS_angle;
}

/**
  * @brief          获取角速度,0:x轴, 1:y轴, 2:roll轴 单位 rad/s
  * @param[in]      none
  * @retval         INS_gyro的指针
  */
extern const fp32 *get_gyro_data_point(void)
{
    return INS_gyro;
}

/**
  * @brief          获取加速度,0:x轴, 1:y轴, 2:roll轴 单位 m/s2
  * @param[in]      none
  * @retval         INS_accel的指针
  */
extern const fp32 *get_accel_data_point(void)
{
    return INS_accel;
}

/**
  * @brief          获取加速度,0:x轴, 1:y轴, 2:roll轴 单位 ut
  * @param[in]      none
  * @retval         INS_mag的指针
  */
extern const fp32 *get_mag_data_point(void)
{
    return INS_mag;
}
