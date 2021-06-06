#include "main.h"
#include "shell_port.h"
#include "cmsis_os.h"
#include "freertos.h"
//#include "userShell.h"
void shellWrite(char data)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&data, 1, 0xff);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
			osSemaphoreRelease(shellBinarySemHandle);
      
      HAL_UART_Receive_DMA(&huart1, (uint8_t *)(&tempData), 1);
        //ReceiveFlag=1;
    }
}