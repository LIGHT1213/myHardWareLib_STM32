#include "fan_test.h"

//==================================================================================================
//| 函数名称 | HAL_TIM_IC_CaptureCallback
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 输入捕获中断回调
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | 
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |库函数
//==================================================================================================
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    captureCallback(htim);
}
//==================================================================================================
//| 函数名称 | HAL_TIM_PeriodElapsedCallback
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 计数溢出中断回调函数
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |库函数
//==================================================================================================
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM4||htim->Instance == TIM9)
    {
        icOverflowCntCallback(htim);
    }
    // #endif
    else  if(htim->Instance == TIM7)
    {
        HAL_IncTick();
    }
}