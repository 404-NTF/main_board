#include "bsp_usart.h"
#include "main.h"
#include "bsp_led.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart1_tx;

static uint8_t command_cmd[7] = {0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x0A};

void usart1_tx_dma_init(void)
{

    //enable the DMA transfer for the receiver and tramsmit request
    //使能DMA串口接收和发送
    SET_BIT(huart1.Instance->CR3, USART_CR3_DMAR);
    SET_BIT(huart1.Instance->CR3, USART_CR3_DMAT);

    //disable DMA
    //失效DMA
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
    //失效DMA
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
}

void INS_set(fp32 cali_scale[3], fp32 cali_offset[3]) {
    HAL_UART_Transmit(&huart6, (uint8_t*)"\x0A", 1, 500);
}
