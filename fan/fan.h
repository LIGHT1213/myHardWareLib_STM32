#ifndef __FAN_H
#define __FAN_H
#include "main.h"
#include "pid_.h"
//风扇数量
#define FAN_NUM 2

//风扇PWM的极性，1：100%pwm满速，0：0％pwm满速
#define PWM_MODE 1
//风扇驱动死区（启动所需要的最小的PWM）------------>如果对风扇的启动速度要求极高（控制反应延迟<2*ControlCycleTime），这个值请测试并设置,
#define MOTOR_DEAD_ZONE 0
/* IC capture Parameters define*/
#define        TIM_IC_FREQ                          50000 //定时器输入捕获频率     
#define        TIM_ICOF_MAX                         0x0a  //输入捕获溢出次数，溢出会清0频率，在这里为13.17秒。这个值和定时器频率和period值有关系
/* 定时器捕获通道枚举定义*/


#define        IC_RISE_EDGE_1                       0  //edge1，第一个上升沿
#define        IC_RISE_EDGE_2                       1  //edge2，第二个上升沿
typedef struct
{
    //风扇最大转速,这个值应当略小于理论值
    float fanSpeedMax;
    //风扇最小转速,这个值应当略大于理论值
    float fanSpeedMin;
    //风扇转一圈的脉冲数
    uint8_t  fanPulseNum;//这个数值一般都是2

    //反馈所在的定时器
    TIM_HandleTypeDef *fbHtim;
    //反馈所在的定时器通道
    uint8_t fbTimChannel;

    //控制所在的定时器
    TIM_HandleTypeDef *ctrlHtim;
    //控制所在的定时器通道
    uint8_t ctrlTimChannel;
}fanControlInit_t;

typedef struct
{

    pidDelta_t fanPidCore;
    //风扇目标转速
    float targetSpeed;
    //当前风扇转速
    float currentSpeed;
    //当前风扇得到反馈信号频率
    float feedBackFeq;

    //风扇最大转速,这个值应当略小于理论值
    float fanSpeedMax;
    //风扇最小转速,这个值应当略大于理论值
    float fanSpeedMin;
    //风扇转一圈的脉冲数
    uint8_t  fanPulseNum;//这个数值一般都是2

    //反馈所在的定时器
    TIM_HandleTypeDef *fbHtim;
    //反馈所在的定时器通道
    uint8_t fbTimChannel;

    //控制所在的定时器
    TIM_HandleTypeDef *ctrlHtim;
    //控制所在的定时器通道
    uint8_t ctrlTimChannel;
    //活动定时器通道
    HAL_TIM_ActiveChannel _timActiveChannel;
    //该风扇的溢出计数
    uint32_t _overFlow;
    //风扇反馈频率
    float _fbFreqBuff;
    
    
    uint8_t ic_edge;
    uint32_t cap_val1;
    uint32_t cap_val2;
    uint32_t cap_sum;
    /* data */
} fanControl_t;



//风扇初始化，用于指向PWM控制函数,指向可用pwm控制等
uint8_t fanInit(fanControl_t *fan,fanControlInit_t *fanInitStruct);
//风扇控制任务，把这个函数放在任务循环中即可
void fanControlTask();
//输入捕获回调函数
void captureCallback(TIM_HandleTypeDef *htim);
//计数溢出回调函数
void icOverflowCntCallback(TIM_HandleTypeDef *htim);
#endif // !__FAN_H
