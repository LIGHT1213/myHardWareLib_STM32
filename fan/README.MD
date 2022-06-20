# 风扇模组代码使用说明

[TOC]



## 一、版本号

v0.02

## 二、 依赖

PID模块

风扇的有反馈信号并且反馈信号连接到单片机的定时器输入捕获引脚

风扇属于可控制的即可以通过控制信号控制风扇转速

## 三、使用说明

### 1.文件构成

风扇模组代码想要正常工作最少需要两个模组（5个文件）。具体如下图

![](https://raw.githubusercontent.com/LIGHT1213/picture/master/20220526135811.png)

### 2.单片机外设配置

将定时器设置为输入捕获并打开该定时器的中断（绝大多数情况下是global interrupt），具体如下图

![image-20220526140040874](https://raw.githubusercontent.com/LIGHT1213/picture/master/image-20220526140040874.png)

![image-20220526140112541](https://raw.githubusercontent.com/LIGHT1213/picture/master/image-20220526140112541.png)

注意这里的需要计算该定时器所在总线的频率除prescaler（分频系数）,得出输入捕获频率用于后面代码，注意这里如果有多个定时器，将这几个定时器的输入捕获频率和counter period设置至一致，这将十分重要。

### 3. 代码说明

目前版本的代码只针对兼容STM32 HAL库的芯片（包括STM32或者相关兼容芯片）做出兼容

#### 3.1 需要根据实际情况修改的代码

##### 3.1.1 fan.h文件

![image-20220526140213418](https://raw.githubusercontent.com/LIGHT1213/picture/master/image-20220526140213418.png)

宏定义 FAN_NUM(风扇数量)，PWM_MODE(PWM极性)，MOTOR_DEAD_ZONE(风扇电机死区，这个值可以不修改（默认为0），取决于对开机后风扇启动速度的要求)，TIM_IC_FREQ(根据上一步中的实际值填写)，TIM_ICOF_MAX(涉及到意外停转后多久报错)。

##### 3.1.2 fan.c文件

无需修改

##### 3.1.3 pid.c文件

无需修改

##### 3.1.4 pid.h文件

无需修改

##### 3.1.5 callback.c 文件

无需修改

#### 3.2 初始化代码

这里以一个风扇为例子，其控制脚被连接在定时器8的通道3，反馈脚被连接在定时器4的通道4

##### 3.2.1 相关中断启动和外设启动

首先启动pwm输出以及输入捕获

```c
HAL_TIM_Base_Start(&htim8);
HAL_TIM_PWM_Init(&htim8);//初始化定时器8的pwm输出
__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);  //将默认输出设置为0占空比
HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);//启动PWM输出
HAL_TIM_Base_Start_IT(&htim4);
HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_4);//启动PWM输入捕获
```

其次初始化控制结构体

```c
fanControl_t fanC;    
fanControlInit_t fanInitC=
{
    .fanSpeedMax=4200,//风扇最大转速
    .fanSpeedMin=1440,//风扇最小转速
    .fbHtim=&htim4,//反馈引脚定时器
    .fbTimChannel=TIM_CHANNEL_4,//反馈引脚定时器通道
    .ctrlHtim=&htim8,//控制引脚定时器
    .ctrlTimChannel=TIM_CHANNEL_3,//控制引脚定时器通道
    .fanPulseNum=2//转一圈的返回脉冲数（这个值一般都是2）
};
fanInit(&fanC,&fanInitC);
```

将对应函数放到中断回调函数中

```c
void captureCallback(TIM_HandleTypeDef *htim);//输入捕获回调函数
void icOverflowCntCallback(TIM_HandleTypeDef *htim);//计数溢出回调函数
//以上两个函数在pid.h中声明
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    captureCallback(htim);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM4||htim->Instance == TIM9)
    {
        icOverflowCntCallback(htim);
    }
}
//HAL_TIM_IC_CaptureCallback和HAL_TIM_PeriodElapsedCallback为HAL库提供的中断回调函数
```

启动溢出中断

```c
__HAL_TIM_ENABLE_IT(&htim4,TIM_IT_UPDATE);
```

将目标速度输入和控制线程放入循环中（循环时间在500 ms到1000 ms能正常运行其他时长需要测试）

```c
fanC.targetSpeed=speed;//调整目标转速
fanControlTask();//控制函数
```

#### 3.3 可能的客制化点和相应的位置

##### 3.3.1 风扇控速PID参数调整

在fan.c中的**fanPidInit**函数中

![image-20220526140300063](https://raw.githubusercontent.com/LIGHT1213/picture/master/image-20220526140300063.png)



