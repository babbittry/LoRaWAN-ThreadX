void RegionCN470InitDefaults( InitDefaultsParams_t* params )

            RegionNvmGroup2->ChannelsDefaultMask[0] = 0xFF00;
            RegionNvmGroup2->ChannelsDefaultMask[1] = 0x0000;
            RegionNvmGroup2->ChannelsDefaultMask[2] = 0x0000;
            RegionNvmGroup2->ChannelsDefaultMask[3] = 0x0000;
            RegionNvmGroup2->ChannelsDefaultMask[4] = 0x0000;
            RegionNvmGroup2->ChannelsDefaultMask[5] = 0x0000;

Thd_LmHandlerProcess_Entry

      APP_LOG(TS_ON, VLEVEL_H, "Thd_LmHandlerProcess_RescheduleFlag: %d \r\n", Thd_LmHandlerProcess_RescheduleFlag);
    }
    else
    {
      tx_thread_suspend(&Thd_LmHandlerProcessId);
      APP_LOG(TS_ON, VLEVEL_H, "Thd_LmHandlerProcess resumed from suspend \r\n");

1. TX ctrl \ RX ctrl 引脚的配置
2. RS232\485的功能
3. downlink 功能
4. RTOS 的任务创建、时间调度

协议栈：
flash：58.5kb
ram：43.2kb

协议栈 + ThreadX：
flash：95.8kb
ram：52.66kb

STM32WLE5CC
48 MHz 
256 Kbytes of Flash memory
64 Kbytes of SRAM

任务启动流程：
tx_application_define() ->　MX_LoRaWAN_Init() -> App_Main_Thread_Entry() （TCB: App_MainThread） -> LoRaWAN_Init()



MCPS
MAC Common Part Sublayer
作用：data transmissions and data receptions

MLME
MAC layer management entity
作用：manage the LoRaWAN network

MIB
MAC information base
作用：store important runtime information and holds the configuration of the LoRaMAC layer.
