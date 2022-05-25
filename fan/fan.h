#ifndef __FAN_H
#define __FAN_H
#include "main.h"
#include "pid.h"
//风扇数量
#define FAN_NUM 1

//风扇PWM的极性，1：100%pwm满速，0：0％pwm满速
#define PWM_MODE 0
//同一个定时器上接的风扇数量
#define SPD_FB_CHAN_NUM 1
//风扇驱动死区（启动所需要的最小的PWM）
#define MOTOR_DEAD_ZONE 23
/* IC capture Parameters define*/
#define        TIM_IC_FREQ                         100000  //定时器输入捕获频率
#define        TIM_ICOF_MAX                         0x05  //输入捕获溢出次数，溢出会清0频率，在这里为6.5536秒。这个值和定时器频率和period值有关系
typedef enum {
    TIM_IC1 = 0,
//    TIM_IC2,
//    TIM_IC3,
//    TIM_IC4,
    TIM_IC_MAX,
} TIM_IC_Typedef;

/* 定时器捕获通道枚举定义*/


#define        IC_RISE_EDGE_1                       0  //edge1，第一个上升沿
#define        IC_RISE_EDGE_2                       1  //edge2，第二个上升沿


typedef struct
{

    pidDelta_t fanPidCore;
    //设定PWM的占空比的函数，duty应当≥0并且≤100
    void(* fanSetPwm)(float duty);
    //风扇目标转速
    uint16_t targetSpeed;
    //当前风扇转速
    uint16_t currentSpeed;
    //当前风扇得到反馈信号频率
    uint16_t feedBackFeq;

    //风扇最大转速,这个值应当略小于理论值
    uint16_t fanSpeedMax;
    //风扇最小转速,这个值应当略大于理论值
    uint16_t fanSpeedMin;
    //风扇转一圈的脉冲数
    uint8_t  fanPulseNum;//这个数值一般都是2
    /* data */
} fanControl_t;



//风扇初始化，用于指向PWM控制函数,指向可用pwm控制等
uint8_t fanInit(fanControl_t *fan,void (*fanSetPwm),uint16_t fanSpeedMax,uint16_t fanSpeedMin,uint8_t fanPulseNum);
//风扇控制任务，把这个函数放在任务循环中即可
void fanControlTask();
///风扇反馈信号频率获取
void setFanFeedbackFeq(fanControl_t *fan,uint16_t feedBackFeq);
#endif // !__FAN_H
