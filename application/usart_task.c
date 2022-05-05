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

static void usart_init(uint8_t *rx1_buf, uint8_t *rx2_buf, uint16_t dma_buf_num);

static void decode_rx_data(uint8_t *rx_bufs, uint16_t length);

void usart_task(void const *pvParameters) {
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

        //失效DMA
        __HAL_DMA_DISABLE(&hdma_usart6_rx);

        //获取接收数据长度,长度 = 设定长度 - 剩余长度
        this_time_rx_len = TOTAL_LENGTH - hdma_usart6_rx.Instance->NDTR;

        //重新设定数据长度
        hdma_usart6_rx.Instance->NDTR = TOTAL_LENGTH;

        if ((hdma_usart6_rx.Instance->CR & DMA_SxCR_CT) == RESET)
        {
            //设定缓冲区1
            hdma_usart6_rx.Instance->CR |= DMA_SxCR_CT;
            
            //使能DMA
            __HAL_DMA_ENABLE(&hdma_usart6_rx);

            decode_rx_data(rx_buf[0], this_time_rx_len);
            //记录数据接收时间
        }
        else
        {
            //设定缓冲区0
            DMA2_Stream1->CR &= ~(DMA_SxCR_CT);
            
            //使能DMA
            __HAL_DMA_ENABLE(&hdma_usart6_rx);

            //处理遥控器数据
            decode_rx_data(rx_buf[1], this_time_rx_len);
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

static void decode_rx_data(uint8_t rx_bufs[TOTAL_LENGTH], uint16_t length) {
    
    if (length == NORMAL_LENGTH && rx_bufs[0] == 0x06)
    {
        set_init();
        //记录数据接收时间
        detect_hook(RM_IMU_TOE);
    }
    else if (length == BMI088_IST8310_LENGTH && rx_bufs[0] == 0x08)
    {
        fp32 INS_gyro[3], INS_accel[3], INS_mag[3], INS_quat[4], INS_angle[3];
        uint8_t temp;
        data_t data;
        data.value = rx_bufs;
        data.length = BMI088_IST8310_LENGTH;
        data.curser = 0;
        get_uint8_t(&data, &temp, 1);
        get_fp32(&data, INS_gyro, 3);
        get_fp32(&data, INS_accel, 3);
        get_fp32(&data, INS_mag, 3);
        get_fp32(&data, INS_quat, 4);
        get_fp32(&data, INS_angle, 3);
        set_value(INS_gyro, INS_accel, INS_mag, INS_quat, INS_angle);
        //记录数据接收时间
        detect_hook(RM_IMU_TOE);
    }
}
