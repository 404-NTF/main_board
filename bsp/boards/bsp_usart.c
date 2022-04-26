#include "bsp_usart.h"
#include "main.h"
#include "bsp_led.h"
#include "calibrate_task.h"

#include "encoder.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart1_tx;

void usart1_tx_dma_init(void)
{

    //enable the DMA transfer for the receiver and tramsmit request
    //ʹ��DMA���ڽ��պͷ���
    SET_BIT(huart1.Instance->CR3, USART_CR3_DMAR);
    SET_BIT(huart1.Instance->CR3, USART_CR3_DMAT);

    //disable DMA
    //ʧЧDMA
    __HAL_DMA_DISABLE(&hdma_usart1_tx);

    while(hdma_usart1_tx.Instance->CR & DMA_SxCR_EN)
    {
        __HAL_DMA_DISABLE(&hdma_usart1_tx);
    }

    hdma_usart1_tx.Instance->PAR = (uint32_t) & (USART1->DR);
    hdma_usart1_tx.Instance->M0AR = (uint32_t)(NULL);
    hdma_usart1_tx.Instance->NDTR = 0;


}
void usart1_tx_dma_enable(uint8_t *data, uint16_t len)
{
    //disable DMA
    //ʧЧDMA
    __HAL_DMA_DISABLE(&hdma_usart1_tx);

    while(hdma_usart1_tx.Instance->CR & DMA_SxCR_EN)
    {
        __HAL_DMA_DISABLE(&hdma_usart1_tx);
    }

    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_tx, DMA_HISR_TCIF7);

    hdma_usart1_tx.Instance->M0AR = (uint32_t)(data);
    __HAL_DMA_SET_COUNTER(&hdma_usart1_tx, len);

    __HAL_DMA_ENABLE(&hdma_usart1_tx);
}

void laser_on(void)
{
    HAL_UART_Transmit(&huart6, (uint8_t*)"\x01", 1, 500);
}

void laser_off(void)
{
    HAL_UART_Transmit(&huart6, (uint8_t*)"\x02", 1, 500);
}

void pwm_on(void)
{
    HAL_UART_Transmit(&huart6, (uint8_t*)"\x03", 1, 500);
}

void pwm_off(void)
{
    HAL_UART_Transmit(&huart6, (uint8_t*)"\x04", 1, 500);
}

void bmi088_ist8310_read(void) {
    HAL_UART_Transmit(&huart6, (uint8_t*)"\x06", 1, 500);
}

void INS_cali(fp32 cali_scale[3], fp32 cali_offset[3], uint16_t *time_count) {
    HAL_UART_Transmit(&huart6, (uint8_t*)"\x08", 1, 500);
    uint8_t datas[CALI_GYRO_LENGTH];
    data_t data;
    data.value = datas;
    data.length = CALI_GYRO_LENGTH;
    set_fp32(&data, cali_scale, 3);
    set_fp32(&data, cali_offset, 3);
    set_uint16_t(&data, time_count, 1);
    HAL_UART_Transmit(&huart6, datas, CALI_GYRO_LENGTH, 500);
}

void INS_set(fp32 cali_scale[3], fp32 cali_offset[3]) {
    HAL_UART_Transmit(&huart6, (uint8_t*)"\x0A", 1, 500);
    uint8_t datas[SET_GYRO_LENGTH];
    data_t data;
    data.value = datas;
    data.length = SET_GYRO_LENGTH;
    set_fp32(&data, cali_scale, 3);
    set_fp32(&data, cali_offset, 3);
    HAL_UART_Transmit(&huart6, datas, SET_GYRO_LENGTH, 500);
}

void send_control_temperature(void) {
    uint8_t data = 0x0D;
    HAL_UART_Transmit(&huart6, &data, 1, 500);
    data = (uint8_t)get_control_temperature();
    HAL_UART_Transmit(&huart6, &data, 1, 500);
}
