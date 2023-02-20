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

### 目前使用的资源：

#### 协议栈：

flash：58.5kb
ram：43.2kb

#### 协议栈 + ThreadX：

flash：107.2kb
ram：56.3kb

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