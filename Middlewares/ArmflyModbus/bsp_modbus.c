#include "bsp_modbus.h"

/*
*********************************************************************************************************
*	函 数 名: bsp_GetRunTime
*	功能说明: 获取CPU运行时间，单位1ms。最长可以表示 24.85天，如果你的产品连续运行时间超过这个数，则必须考虑溢出问题
*	形    参:  无
*	返 回 值: CPU运行时间，单位1ms
*********************************************************************************************************
*/
EXECUTION_TIME bsp_GetRunTime(void)
{
	EXECUTION_TIME runtime;

	DISABLE_INT();  	/* 关中断 */

	runtime = tx_time_get;	/* 这个变量在Systick中断中被改写，因此需要关中断进行保护 */

	ENABLE_INT();  		/* 开中断 */

	return runtime;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_CheckRunTime
*	功能说明: 计算当前运行时间和给定时刻之间的差值。处理了计数器循环。
*	形    参:  _LastTime 上个时刻
*	返 回 值: 当前时间和过去时间的差值，单位1ms
*********************************************************************************************************
*/
EXECUTION_TIME bsp_CheckRunTime(EXECUTION_TIME _LastTime)
{
	EXECUTION_TIME now_time;
	EXECUTION_TIME time_diff;

	DISABLE_INT();  	/* 关中断 */

	now_time = tx_time_get;	/* 这个变量在Systick中断中被改写，因此需要关中断进行保护 */

	ENABLE_INT();  		/* 开中断 */
	
	if (now_time >= _LastTime)
	{
		time_diff = now_time - _LastTime;
	}
	else
	{
		time_diff = 0x7FFFFFFF - _LastTime + now_time;
	}

	return time_diff;
}