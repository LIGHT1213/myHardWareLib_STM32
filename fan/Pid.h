#ifndef __PID_H
#define __PID_H
#include "main.h"
typedef struct
{

    // 比例
    float f_kpU;
    // 积分
    float f_kiU;
    // 微分
    float f_kdU;
    // 控制率U输出
    float f_kU;
    // 上次控制率
    float f_lastKU;
    //设定值
    float f_tarVal;
    //调试计数器
    uint32_t uin_cnt;
    // 比例
    float f_kp;
    // 积分
    float f_ki;
    // 微分
    float f_kd;

    // 本次误差
    float f_err;
    // 上一次误差
    float f_errLast;
    // 上上次误差
    float f_errPrev;
    //控制率输出上限
    float f_uLimit;
    //第一次控制时，防止过大
    uint8_t uch_flag;


} pidDelta_t;
float calPid(void *args, float f_tar, float f_real);
float calPid_converse(void *args, float f_tar, float f_real);
#endif