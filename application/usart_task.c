#include "cmsis_os.h"
#include "main.h"

#include "bsp_usart.h"

#include "usart_task.h"
#include "detect_task.h"
#include "INS_task.h"

#include "decoder.h"

extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_rx;

static uint8_t rx_buf[2][TOTAL_LENGTH];
static uint16_t rx_length;

static void usart_init(uint8_t *rx1_buf, uint8_t *rx2_buf, uint16_t dma_buf_num);

static void decode_rx_data(uint8_t *rx_bufs);

void usart_task(void const *pvParameters) {
    rx_length = NORMAL_LENGTH;
    usart_init(rx_buf[0], rx_buf[1], TOTAL_LENGTH);
    while (1)
    {
        osDelay(500);
    }
}

void USART6_IRQHandler(void) {
    if(huart6.Instance->SR & UART_FLAG_RXNE)//接收到数据
    {
        __HAL_UART_CLEAR_PEFLAG(&huart6);
    }
    else if(USART6->SR & UART_FLAG_IDLE)
    {
        static uint16_t this_time_rx_len = 0;

        __HAL_UART_CLEAR_PEFLAG(&huart6);

        //disable DMA
        //失效DMA
        __HAL_DMA_DISABLE(&hdma_usart6_rx);

        //get receive data length, length = set_data_length - remain_length
        //获取接收数据长度,长度 = 设定长度 - 剩余长度
        this_time_rx_len = TOTAL_LENGTH - hdma_usart6_rx.Instance->NDTR;

        //reset set_data_lenght
        //重新设定数据长度
        hdma_usart6_rx.Instance->NDTR = TOTAL_LENGTH;

        if ((hdma_usart6_rx.Instance->CR & DMA_SxCR_CT) == RESET)
        {
            /* Current memory buffer used is Memory 0 */
            //设定缓冲区1
            hdma_usart6_rx.Instance->CR |= DMA_SxCR_CT;
            
            //使能DMA
            __HAL_DMA_ENABLE(&hdma_usart6_rx);

            if(this_time_rx_len == rx_length)
            {
                decode_rx_data(rx_buf[0]);
                //记录数据接收时间
                detect_hook(RM_IMU_TOE);
            }
        }
        else
        {
            /* Current memory buffer used is Memory 1 */
            //设定缓冲区0
            DMA2_Stream1->CR &= ~(DMA_SxCR_CT);
            
            //使能DMA
            __HAL_DMA_ENABLE(&hdma_usart6_rx);

            if(this_time_rx_len == rx_length)
            {
                //处理遥控器数据
                decode_rx_data(rx_buf[1]);
                //记录数据接收时间
                detect_hook(RM_IMU_TOE);
            }
        }
    }
}

static void usart_init(uint8_t *rx1_buf, uint8_t *rx2_buf, uint16_t dma_buf_num) {
    //使能DMA串口接收
    SET_BIT(huart6.Instance->CR3, USART_CR3_DMAR);

    //使能空闲中断
    __HAL_UART_ENABLE_IT(&huart6, UART_IT_IDLE);

    //失效DMA
    __HAL_DMA_DISABLE(&hdma_usart6_rx);
    while(hdma_usart6_rx.Instance->CR & DMA_SxCR_EN)
    {
        __HAL_DMA_DISABLE(&hdma_usart6_rx);
    }

    hdma_usart6_rx.Instance->PAR = (uint32_t) & (USART6->DR);
    //内存缓冲区1
    hdma_usart6_rx.Instance->M0AR = (uint32_t)(rx1_buf);
    //内存缓冲区2
    hdma_usart6_rx.Instance->M1AR = (uint32_t)(rx2_buf);
    //数据长度
    hdma_usart6_rx.Instance->NDTR = dma_buf_num;
    //使能双缓冲区
    SET_BIT(hdma_usart6_rx.Instance->CR, DMA_SxCR_DBM);

    //使能DMA
    __HAL_DMA_ENABLE(&hdma_usart6_rx);

}

static void decode_rx_data(uint8_t *rx_bufs) {
    if (rx_length == NORMAL_LENGTH)
    {
        switch (*rx_bufs)
        {
        case 0x05: set_init();                          break;
        case 0x07: rx_length = BMI088_IST8310_LENGTH;   break;
        case 0x09: cali_gyro_comp();                    break;
        case 0x0B: set_ins_ok();                        break;
        case 0x0C: send_control_temperature();          break;
        }
    }
    else if (rx_length == BMI088_IST8310_LENGTH)
    {
        fp32 INS_gyro[3], INS_accel[3], INS_mag[3], INS_quat[4], INS_angle[3];
        data_t data;
        data.value = rx_bufs;
        data.length = BMI088_IST8310_LENGTH;
        get_fp32(&data, INS_gyro, 3);
        get_fp32(&data, INS_accel, 3);
        get_fp32(&data, INS_mag, 3);
        get_fp32(&data, INS_quat, 4);
        get_fp32(&data, INS_angle, 3);
        set_value(INS_gyro, INS_accel, INS_mag, INS_quat, INS_angle);
        rx_length = NORMAL_LENGTH;
    }
}
