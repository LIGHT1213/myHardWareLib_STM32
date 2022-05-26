#include "pid_test.h"


void Init_CPid(void *args, float f_p, float f_i, float f_d, float f_set, float f_u)
{
    pidDelta_t *handle = (pidDelta_t *)args;

    handle->f_kp = f_p; //比例系数
    handle->f_ki = f_i; //积分系数
    handle->f_kd = f_d; //微分系数

    handle->f_err = 0; //本次误差
    handle->f_errLast = 0; //上次误差
    handle->f_errPrev = 0; //上上次误差

    handle->f_kpU = 0; //比例输出率
    handle->f_kiU = 0; //积分输出率
    handle->f_kdU = 0; //微分输出率
    handle->f_kU = 0; //控制输出率
    handle->f_lastKU = 0; //上次控制率

    handle->f_tarVal = f_set; //目标值

    handle->f_uLimit = f_u; //控制率输出限制

    handle->uin_cnt = 0;
    handle->uch_flag = 1;

}

//==================================================================================================
//| 函数名称 | Cal
//|----------|--------------------------------------------------------------------------------------
//| 函数功能 | 进行PID的计算
//|----------|--------------------------------------------------------------------------------------
//| 输入参数 | f_tar -- 本次计算的目标值，f_real -- 本次反馈值
//|----------|--------------------------------------------------------------------------------------
//| 返回参数 | 控制率输出值
//|----------|--------------------------------------------------------------------------------------
//| 函数设计 |
//==================================================================================================
float calPid(void *args, float f_tar, float f_real)
{
    pidDelta_t *handle = (pidDelta_t *)args;

    float f_deltaU;

    handle->uin_cnt++;

    handle->f_tarVal = f_tar;

    handle->f_err = handle->f_tarVal - f_real; //计算本次控制误差，目标减实际

    //如果第一次调节
    if (handle->uch_flag == 1)
    {
        handle->uch_flag = 0;

        handle->f_errLast = handle->f_err;
    }

    handle->f_kpU = handle->f_kp *(handle->f_err - handle->f_errLast);
    handle->f_kiU = handle->f_ki * handle->f_err;
    handle->f_kdU = handle->f_kd * (handle->f_err - 2 * handle->f_errLast + handle->f_errPrev);

    //printk("f_kpU = %f, f_kiU = %f, f_kdU = %f\r\n", handle->f_kpU, handle->f_kiU, handle->f_kdU);

    //计算控制率增量
    f_deltaU = handle->f_kpU + handle->f_kiU + handle->f_kdU;
    //计算控制率输出
    handle->f_kU = f_deltaU + handle->f_lastKU;

    //printk("f_kU = %f, f_deltaU = %f, f_lastKU = %f\r\n",handle->f_kU, f_deltaU, handle->f_lastKU);

	if(handle->f_uLimit!=0)
	{
		if (handle->f_kU > handle->f_uLimit)
		{
			handle->f_kU = handle->f_uLimit;
		}
		else if (handle->f_kU < 0)
		{
			handle->f_kU = 0;
		}
	}
    handle->f_lastKU = handle->f_kU;
    handle->f_errPrev = handle->f_errLast;
    handle->f_errLast = handle->f_err;

    return handle->f_kU;
}
float calPid_converse(void *args, float f_tar, float f_real)
{
    pidDelta_t *handle = (pidDelta_t *)args;

    float f_deltaU;

    handle->uin_cnt++;

    handle->f_tarVal = f_tar;

    handle->f_err =  f_real-handle->f_tarVal; //计算本次控制误差，实际减目标

    //如果第一次调节
    if (handle->uch_flag == 1)
    {
        handle->uch_flag = 0;

        handle->f_errLast = handle->f_err;
    }

    handle->f_kpU = handle->f_kp *(handle->f_err - handle->f_errLast);
    handle->f_kiU = handle->f_ki * handle->f_err;
    handle->f_kdU = handle->f_kd * (handle->f_err - 2 * handle->f_errLast + handle->f_errPrev);


    //计算控制率增量
    f_deltaU = handle->f_kpU + handle->f_kiU + handle->f_kdU;
    //计算控制率输出
    handle->f_kU = f_deltaU + handle->f_lastKU;

	if(handle->f_uLimit!=0)
	{
		if (handle->f_kU > handle->f_uLimit)
		{
			handle->f_kU = handle->f_uLimit;
		}
		else if (handle->f_kU < 0)
		{
			handle->f_kU = 0;
		}
	}
    handle->f_lastKU = handle->f_kU;
    handle->f_errPrev = handle->f_errLast;
    handle->f_errLast = handle->f_err;


    return handle->f_kU;
}

