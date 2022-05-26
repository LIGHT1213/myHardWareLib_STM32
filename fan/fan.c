#include "fan_test.h"
fanControl_t *fanList[FAN_NUM]= {NULL};
static void setFanPWM(fanControl_t *fan,float duty);
static void fanAdd(fanControl_t *fan);
static void fanPidInit(fanControl_t *fan);
static void getCurrentSpeed(fanControl_t *fan);
static void resetIcofCnt(fanControl_t *fan);
static void processIcValue(fanControl_t *fan,uint32_t _icCnt);
static float calcTheFreq(fanControl_t *fan,uint32_t _icValue);
//==================================================================================================
//| 函数名称 | fanInit
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 初始化某一个风扇参数
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | fan_实体控制句柄,*fanSetPwm_风扇PWM设置函数 ,fanSpeedMax_风扇转速最大值,fanSpeedMin_风扇转速最大值,fanPulseNum_风扇转速最小值
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 1成功，0失败
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |
//==================================================================================================
uint8_t fanInit(fanControl_t *fan,fanControlInit_t *fanInitStruct)
{
    fan->fanSpeedMax=fanInitStruct->fanSpeedMax;
    fan->fanSpeedMin=fanInitStruct->fanSpeedMin;
    fan->fanPulseNum=fanInitStruct->fanPulseNum;
    fan->targetSpeed=1000;
    fan->fbHtim=fanInitStruct->fbHtim;
    fan->fbTimChannel=fanInitStruct->fbTimChannel;
    fan->ctrlHtim=fanInitStruct->ctrlHtim;
    fan->ctrlTimChannel=fanInitStruct->ctrlTimChannel;
    switch(fanInitStruct->fbTimChannel)
    {
        case(TIM_CHANNEL_1):
        {
            fan->_timActiveChannel=HAL_TIM_ACTIVE_CHANNEL_1;
            break;
        }
        case(TIM_CHANNEL_2):
        {
            fan->_timActiveChannel=HAL_TIM_ACTIVE_CHANNEL_2;
            break;
        }
        case(TIM_CHANNEL_3):
        {
            fan->_timActiveChannel=HAL_TIM_ACTIVE_CHANNEL_3;
            break;
        }
        case(TIM_CHANNEL_4):
        {
            fan->_timActiveChannel=HAL_TIM_ACTIVE_CHANNEL_4;
            break;
        }
        default:
        {
            return 0;//通道不和规则
        }
    }
    fan->ic_edge = IC_RISE_EDGE_1;
    fan->cap_val1 = 0;
    fan->cap_val2 = 0;
    fan->cap_sum = 0;
    if(fan->fanSpeedMax==fan->fanSpeedMin||fan->fbHtim==NULL||fan->fanSpeedMax<=0||fan->fanSpeedMin<=0||fan->fanPulseNum==0)
    {
        return 0;//相关风扇控制句柄指定，初始化失败或者预设的风扇转速的最大值和最小值相等也会导致初始化失败
    }
    fanAdd(fan);//将风扇控制句柄添加至控制数组中
    fanPidInit(fan);//将风扇控制PID初始化
    return 1;
}
//==================================================================================================
//| 函数名称 | setFanPWM
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 |控制风扇PWM
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |
//==================================================================================================
static void setFanPWM(fanControl_t *fan,float duty)
{
    uint32_t compare;
    duty=duty>100?100:duty;//保证占空比不大于100
    duty=duty<0?0:duty;//保证占空比不小于0
	compare=fan->ctrlHtim->Instance->ARR*(duty/100);
    __HAL_TIM_SET_COMPARE(fan->ctrlHtim,fan->ctrlTimChannel,compare);
}
//==================================================================================================
//| 函数名称 | fanControlTask
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 所有已经初始化风扇转速控制函数
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |
//==================================================================================================
void fanControlTask()
{
    for(int i=0; i<FAN_NUM; i++)
    {
        if(fanList[i]!=NULL)
        {
            float duty;
            //限定最大最小输出值
            if(fanList[i]->targetSpeed>fanList[i]->fanSpeedMax)
            {
                fanList[i]->targetSpeed=fanList[i]->fanSpeedMax;
            }
            if(fanList[i]->targetSpeed<fanList[i]->fanSpeedMin || fanList[i]->targetSpeed>50000)//屏蔽负转速
            {
                fanList[i]->targetSpeed=fanList[i]->fanSpeedMin;
            }
			
            getCurrentSpeed(fanList[i]);//计算当前风扇转速
            duty=calPid(&fanList[i]->fanPidCore,fanList[i]->targetSpeed,fanList[i]->currentSpeed);//计算pid
            if(PWM_MODE==1)//PWM_MODE=1则当PWM为100％时转速为满
            {
				if(duty<=MOTOR_DEAD_ZONE)//保证最低pwm输出为死区pwm
				{
					//fanList[i]->fanSetPwm(MOTOR_DEAD_ZONE);
                    setFanPWM(fanList[i],MOTOR_DEAD_ZONE);
				}
				else
				{
                    //fanList[i]->fanSetPwm(duty+MOTOR_DEAD_ZONE);
                    setFanPWM(fanList[i],duty+MOTOR_DEAD_ZONE);
				}
            }
            else
            {
				if(100.0-duty-MOTOR_DEAD_ZONE>=100-MOTOR_DEAD_ZONE)//保证最低pwm输出为死区pwm
                {
                    //fanList[i]->fanSetPwm(100-MOTOR_DEAD_ZONE);
                    setFanPWM(fanList[i],100-MOTOR_DEAD_ZONE);
                }
				else
                {
                    //fanList[i]->fanSetPwm(100.0-duty-MOTOR_DEAD_ZONE);
                    setFanPWM(fanList[i],100.0-duty-MOTOR_DEAD_ZONE);
                }
				
            }
        }
    }
}
//==================================================================================================
//| 函数名称 | fanAdd
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 将风扇添加至控制列表
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | *fan_风扇控制句柄
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |私有函数
//==================================================================================================
static void fanAdd(fanControl_t *fan)
{
    for(int i=0; i<FAN_NUM; i++)
    {
        if(fanList[i]==NULL)
        {
            fanList[i]=fan;
			break;
        }
    }
}
//==================================================================================================
//| 函数名称 | fanPidInit
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 风扇转速PID控制
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | *fan_风扇控制句柄
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |私有函数
//==================================================================================================
static void fanPidInit(fanControl_t *fan)
{
  
  //这组PID在循环时间为500ms到1000ms能够很好的工作，其他的工作循环时间需要自行测试
    fan->fanPidCore.f_kp = 0.00001; //比例系数
    fan->fanPidCore.f_ki = 0.00275; //积分系数    
    fan->fanPidCore.f_kd =0.00005; //微分系数

    fan->fanPidCore.f_err = 0; //本次误差
    fan->fanPidCore.f_errLast = 0; //上次误差
    fan->fanPidCore.f_errPrev = 0; //上上次误差

    fan->fanPidCore.f_kpU = 0; //比例输出率
    fan->fanPidCore.f_kiU = 0; //积分输出率
    fan->fanPidCore.f_kdU = 0; //微分输出率
    fan->fanPidCore.f_kU = 0; //控制输出率
    fan->fanPidCore.f_lastKU = 0; //上次控制率

    fan->fanPidCore.f_tarVal = fan->targetSpeed; //目标值

    fan->fanPidCore.f_uLimit = 0; //控制率输出限制

    fan->fanPidCore.uin_cnt = 0;
    fan->fanPidCore.uch_flag = 1;
}
//==================================================================================================
//| 函数名称 | CaptureCallback
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 输入捕获回调函数
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | 触发的定时器
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |私有函数
//==================================================================================================
void captureCallback(TIM_HandleTypeDef *htim)
{
    fanControl_t *currentFan=NULL;
    for(int i=0;i<FAN_NUM; i++)
    {
        
        if(fanList[i]->fbHtim->Instance==htim->Instance&&fanList[i]->_timActiveChannel==htim->Channel)
        {
            currentFan=fanList[i];
            break;
        }
    }
    if(currentFan==NULL)
    {
        return;
    }


    if(currentFan->ic_edge == IC_RISE_EDGE_1)
    {

        resetIcofCnt(currentFan);
        currentFan->cap_val1=__HAL_TIM_GET_COMPARE(currentFan->fbHtim, currentFan->fbTimChannel);
        currentFan->ic_edge = IC_RISE_EDGE_2;
    }
    else
    {
        currentFan->cap_val2=__HAL_TIM_GET_COMPARE(currentFan->fbHtim, currentFan->fbTimChannel);
        if(currentFan->_overFlow== 0)
        {
            currentFan->cap_sum = currentFan->cap_val2 - currentFan->cap_val1;
        }
        else if(currentFan->_overFlow == 1) 
		{ //溢出一个计数周期
            currentFan->cap_sum = (currentFan->fbHtim->Instance->ARR - currentFan->cap_val1) + currentFan->cap_val2;
        } 
		else
		{ //溢出N个计数周期
            currentFan->cap_sum = (currentFan->fbHtim->Instance->ARR - currentFan->cap_val1) + currentFan->fbHtim->Instance->ARR *(currentFan->_overFlow-1) +currentFan->cap_val2;
        }
        processIcValue(currentFan,currentFan->cap_sum);
        currentFan->ic_edge = IC_RISE_EDGE_1;
    }
}
//==================================================================================================
//| 函数名称 | calcTheFreq
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 计算反馈频率
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |私有函数
//==================================================================================================
static float calcTheFreq(fanControl_t *fan,uint32_t _icValue)
{
    static float feq;
	if(_icValue<=100)
	{
		return feq;//防止在关机时，所得频率过大
	}
	else
	{
		return feq=TIM_IC_FREQ / _icValue;
	}
}

static void processIcValue(fanControl_t *fan,uint32_t _icCnt)
{
    fan->_fbFreqBuff=calcTheFreq(fan,_icCnt);
}
//==================================================================================================
//| 函数名称 | resetIcofCnt
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 重置溢出数据
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |私有函数
//==================================================================================================
static void resetIcofCnt(fanControl_t *fan)
{
    fan->_overFlow=0;
}
//==================================================================================================
//| 函数名称 | TIM_IcOverflowCntCallback
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 计数溢出中断回调函数
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |私有函数
//==================================================================================================
void icOverflowCntCallback(TIM_HandleTypeDef *htim)
{
    fanControl_t *currentFan=NULL;
    for(int i=0;i<FAN_NUM; i++)
    {
        
        if(fanList[i]->fbHtim->Instance==htim->Instance)
        {
            currentFan=fanList[i];
            if(currentFan->_overFlow < TIM_ICOF_MAX) 
            {
                currentFan->_overFlow ++;
            } 
            else 
            {
                currentFan->_overFlow =0;
                currentFan->_fbFreqBuff = 0;
            }
        }
    }


}
//==================================================================================================
//| 函数名称 | getCurrentSpeed
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 获取风扇转速
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | *fan_风扇控制句柄
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 无
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |私有函数
//==================================================================================================
static void getCurrentSpeed(fanControl_t *fan)
{
    fan->currentSpeed=60*fan->_fbFreqBuff/fan->fanPulseNum;
}