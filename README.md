# LoRaWAN-ThreadX
 A STM32WL - LoRaWAN project, based on ThreadX(Azure RTOS).

基于 STM32WLE5CCU6 芯片的 LoRaWAN 终端设备。使用的 RTOS 是 ThreadX(Azure RTOS).



### 一些信息：

- 外部晶振：32.768KHz、32MHz
- region：CN470
- 基础工程的构建软件：STM32 CubeMX
- 硬件地址：https://oshwhub.com/dfasdf/e77-400m22s
- 串口：921600-8-N-1
- LED 灯： PC13（低电平点亮）
- Modbus RTU 超时时间，1750us：48 * 1750 / 48000000 = 0.00175s = 1.75ms = 1750us.



### Modbus Poll 的设置

串口：115200 - 8 -N - 1

Read / Write Definition: 

​	Slave ID: 1

​	Function: 04 Read Input Registrer(3x)

​	Address: 0

​    Quantity: 4





### CubeMX 生成工程后，需要修改的项

#### 1. 函数 `void RegionCN470InitDefaults( InitDefaultsParams_t* params )`

这里是修改 SubBand，目前我用的 SubBand 2。请根据实际情况修改。

```C
            RegionNvmGroup2->ChannelsDefaultMask[0] = 0xFF00;
            RegionNvmGroup2->ChannelsDefaultMask[1] = 0x0000;
            RegionNvmGroup2->ChannelsDefaultMask[2] = 0x0000;
            RegionNvmGroup2->ChannelsDefaultMask[3] = 0x0000;
            RegionNvmGroup2->ChannelsDefaultMask[4] = 0x0000;
            RegionNvmGroup2->ChannelsDefaultMask[5] = 0x0000;
```

#### 2. 函数 `Thd_LmHandlerProcess_Entry`

这里是 CubeMX 代码库有问题，这两句 log 缺少 `\r`, 导致在有些串口查看软件上出现格式错误。

```C
      APP_LOG(TS_ON, VLEVEL_H, "Thd_LmHandlerProcess_RescheduleFlag: %d \r\n", Thd_LmHandlerProcess_RescheduleFlag);
    }
    else
    {
      tx_thread_suspend(&Thd_LmHandlerProcessId);
      APP_LOG(TS_ON, VLEVEL_H, "Thd_LmHandlerProcess resumed from suspend \r\n");
```

#### 3. 宏定义 `LED_PERIOD_TIME`

500ms 的时间有点久，改为20ms

```C
#define LED_PERIOD_TIME 20
```

#### 4. 错误处理函数 `Error_Handler(__FILE__, __LINE__)`

在错误处理函数中打印出错的文件名和行。搜索替换：

源：`Error_Handler();`

替换为：`Error_Handler(__FILE__, __LINE__);`

`main.c` 和 `main.h` 中的 `void Error_Handler(void)` 替换为`void Error_Handler(char *file, uint32_t line)`

#### 5. 添加 EXECUTION PROFILE 有关内容

##### 5.1 添加汇编的宏定义

```C
TX_EXECUTION_PROFILE_ENABLE, TX_ENABLE_EXECUTION_CHANGE_NOTIFY
```

##### 5.2 `tx_api.h` 添加 EXECUTION PROFILE 有关内容

```C
#if (defined(TX_EXECUTION_PROFILE_ENABLE) && !defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY))
#include "tx_execution_profile.h"
#endif
```

##### 5.3 `tx_initialize_kernel_enter.c` 添加 EXECUTION PROFILE 有关内容

```C
#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
extern VOID _tx_execution_initialize(VOID);
#endif
```

```C
#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    /* Initialize Execution Profile Kit.  */
    _tx_execution_initialize();
#endif
```

### TODO:

- 利用 Trace 功能，测试不同优化等级下，以及是否开启 Link-Time Optimization、One ELF Section per Function 对程序执行时间、Firm 大小的影响

  - **Optimize for Time：优化时间，即优化代码中费时的地方** 【设置编译器命令行：-Otime】 比如有些算法，本身代码量就比较大，运行需要很长时间（假如需要2秒），这个时候勾选上该功能，会发现运行时间有比较明显的减少（或许不到1秒时间）。
  - **One ELF Section per Function：优化每一个函数 ELF 段** 【设置编译器命令行：--split_sections】 每个函数都会产生一个 ELF 段，勾选上，允许优化每一个 ELF 段，通过这个选项， 可以在最后生成的二进制文件中将冗余的函数排除掉。

- 查看射频部分的 TX ctrl \ RX ctrl 引脚的配置

- 为什么最大栈使用（MaxStack）比当前使用栈（CurStack）还要大？例如下面的 Thread StoreContext、Thread StopJoin、App Task IDLE

  ```
  ===============================================================
  OS CPU Usage = 90.00%
  ===============================================================
    任务优先级  任务栈大小    当前使用栈   最大栈使用    任务名
     Prio     StackSize   CurStack    MaxStack   Taskname
     17         1020        127         127      App Task print
     10         1532        227        1087      App LoRaWAN Main Thread
     10         1532        115        1079      Thread LmHandlerProcess
     10         1020        147          95      Thread StoreContext
     10         1020        115          75      Thread StopJoin
      0         1020        155         155      System Timer Thread
     30          508        103         103      App Task STAT
     31          252         79          71      App Task IDLE
  ===============================================================
  ```

  

### 目前使用的资源：

#### LoRaWan 协议栈：

flash：58.5kb
ram：43.2kb

#### LoRaWan 协议栈 + ThreadX：

flash：107.2kb
ram：56.3kb

#### FreeModbus协议栈 + LoRaWan 协议栈 + ThreadX：

flash：110kb
ram：56.67kb

####  STM32WLE5CC 资源
48 MHz 
256 Kbytes of Flash memory
64 Kbytes of SRAM

### LoRa 部分的任务启动流程

tx_application_define() ->　MX_LoRaWAN_Init() -> App_Main_Thread_Entry() （TCB: App_MainThread） -> LoRaWAN_Init()

### 名词解释：

MCPS
MAC Common Part Sublayer
作用：data transmissions and data receptions

MLME
MAC layer management entity
作用：manage the LoRaWAN network

MIB
MAC information base
作用：store important runtime information and holds the configuration of the LoRaMAC layer.