/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       INS_task.c/h
  * @brief      use bmi088 to calculate the euler angle. no use ist8310, so only
  *             enable data ready pin to save cpu time.enalbe bmi088 data ready
  *             enable spi DMA to save the time spi transmit
  *             ��Ҫ����������bmi088��������ist8310�������̬���㣬�ó�ŷ���ǣ�
  *             �ṩͨ��bmi088��data ready �ж�����ⲿ�������������ݵȴ��ӳ�
  *             ͨ��DMA��SPI�����ԼCPUʱ��.
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
fp32 INS_angle[3] = {0.0f, 0.0f, 0.0f};      //euler angle, unit rad.ŷ���� ��λ rad

bool_t bmi088_ist8310_inited = 0;
bool_t command_callback;

uint8_t timeout;


/**
  * @brief          imu����, ��ʼ�� bmi088, ist8310, ����ŷ����
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
  * @brief          У׼������
  * @param[out]     �����ǵı������ӣ�1.0fΪĬ��ֵ�����޸�
  * @param[out]     �����ǵ���Ư���ɼ������ǵľ�ֹ�������Ϊoffset
  * @param[out]     �����ǵ�ʱ�̣�ÿ����gyro_offset���û��1,
  * @retval         none
  */
void INS_cali_gyro(fp32 cali_scale[3], fp32 cali_offset[3], uint16_t *time_count)
{
    INS_cali(cali_scale, cali_offset, time_count);
}

/**
  * @brief          У׼���������ã�����flash���������ط�����У׼ֵ
  * @param[in]      �����ǵı������ӣ�1.0fΪĬ��ֵ�����޸�
  * @param[in]      �����ǵ���Ư
  * @retval         none
  */
void INS_set_cali_gyro(fp32 cali_scale[3], fp32 cali_offset[3])
{
    INS_set(cali_scale, cali_offset);
}

/**
  * @brief          ��ȡ��Ԫ��
  * @param[in]      none
  * @retval         INS_quat��ָ��
  */
const fp32 *get_INS_quat_point(void)
{
    return INS_quat;
}

/**
  * @brief          ��ȡŷ����, 0:yaw, 1:pitch, 2:roll ��λ rad
  * @param[in]      none
  * @retval         INS_angle��ָ��
  */
const fp32 *get_INS_angle_point(void)
{
    return INS_angle;
}

/**
  * @brief          ��ȡ���ٶ�,0:x��, 1:y��, 2:roll�� ��λ rad/s
  * @param[in]      none
  * @retval         INS_gyro��ָ��
  */
extern const fp32 *get_gyro_data_point(void)
{
    return INS_gyro;
}

/**
  * @brief          ��ȡ���ٶ�,0:x��, 1:y��, 2:roll�� ��λ m/s2
  * @param[in]      none
  * @retval         INS_accel��ָ��
  */
extern const fp32 *get_accel_data_point(void)
{
    return INS_accel;
}

/**
  * @brief          ��ȡ���ٶ�,0:x��, 1:y��, 2:roll�� ��λ ut
  * @param[in]      none
  * @retval         INS_mag��ָ��
  */
extern const fp32 *get_mag_data_point(void)
{
    return INS_mag;
}
