#ifndef _WIFI_MCU_COMMAND_H_
#define _WIFI_MCU_COMMAND_H_

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

//****************************************************************************************
//					MCU 端命令集
//****************************************************************************************

//******************************************************************
//	3 出厂开机
//******************************************************************
#define MCU_CAP_PRJ 	0x01	//MCU回复项目名,这个必须回复，格式要求为“branchname_modelname”，例如“MUZO_W001”,下划线前为品牌名，下划线后为型号。
#define MCU_PTV_XXX		0x02	//设置使用的MCU 协议的版本号，默认000；
#define MCU_ALI_PID		0x03	//阿里产品ID设置
#define MCU_SPY_NAM 	0x04	//如果需要Spotify认证，回复此项设置modename，如果不回复默认与project_name一致
#define MCU_SPY_BRN 	0x05	//如果需要Spotify认证，回复此项设置brandname，如果不回复默认默认与project_name一致
#define MCU_SPY_MOD 	0x06	//如果需要Spotify认证，回复此项设置modedisplayname，如果不回复默认默认与modename一致
#define MCU_SPY_BRD 	0x07	//如果需要Spotify认证，回复此项设置branddisplayname，如果不回复默认默认与brandname一致
#define MCU_SPY_TYP 	0x08	//如果需要Spotify认证，回复此项设置类型
#define MCU_CAP_001 	0x09	//-MCU一些开关变量设置，用16进制表示，前面不要加0x
#define MCU_CAP_002 	0x0A	//-MCU一些开关变量设置，用16进制表示，前面不要加0x
#define MCU_CAP_LAU 	0x0B	//-MCU一设置默认语言
#define MCU_CAP_STM 	0x0C	//-MCU一些开关变量设置，用16进制表示，前面不要加0x
#define MCU_CAP_PLM 	0x0D	//-MCU一些开关变量设置，用16进制表示，前面不要加0x
#define MCU_CAP_EXT 	0x0E	//目前为0表示没有外置灯效，否则表示有外置灯控。
#define MCU_PRESETN 	0x0F	//设置预置按键数量，最多为10个，为了向前兼容，0表示默认6个预置（不回复默认为0），-1表示没有预置功能。注意n前面有个+，这个指令在正常开机后发送也有效。
#define MCU_SET_PWD 	0x10	//设置设备AP模式的默认密码，密码为明文，不要包含特殊字符或者与MCU协议的定义冲突。密码最少8字节，最多15字节。这个指令在正常开机后发送无效。
#define MCU_VMX_XXX 	0x11	//语音提示最高允许的音量，nnn为0~100的数字，如001, 022, 100；一旦当前音量超过允许音量，软件会做降音处理，这个值设置<10或者大于90无效。
#define MCU_SID_VAL 	0x12	//用于设置设备默认SSID名称, 请在所有其他设置项设置完成后，最后回复这个指令设置SSID，这个必须回复。
#define MCU_ALV_NUM   0x13
#define MCU_PAR_XXX   0x14  //设置MCU升级文件的格式和发送文件包的长度。
#define MCU_FMT_XXX   0x15  //MCU设置WIFi  IIS数据格式。

//******************************************************************
//	4 正常开机/重启/恢复出厂/休眠唤醒/电池
//******************************************************************
#define MCU_BOT_DON   0x1A  	//模组在收到MCU+BOT+DON时会回复AXX+BOT+DON（用于心跳检测）
#define MCU_GET_UID 	0x1B	//获取当前模组固件版本的项目uuid，模组回复 AXX+UID+uuid&，项目的uuid是8个字节的ascii码，以FF开头
#define MCU_GET_MAC 	0x1C	//获取当前模组的MAC地址，模组回复 AXX+MAC+%02X:%02X:%02X:%02X:%02X:%02X&
#define MCU_DEV_RST 	0x1D	//MCU让WiFi模组重启
#define MCU_POW_OFF 	0x1E  	//MCU也可以发送MCU+POW+OFF给模组去关闭一些进程，如果WiFi回复AXX+POW+OFF或者2秒内无回复，MCU即可给模组断电；如果WiFi回复AXX+BURNING，不可关机
#define MCU_POW_SAV 	0x1F	//MCU 让WiFi模组进入休眠状态
#define MCU_POW_RES 	0x20	//MCU 让WiFi模组唤醒
#define MCU_POW_STA 	0x21	//获取WiFi 模组状态，模组回复 AXX+POW+%03d，其中数字可能为：000： 正常  001： 省电模式(RF关闭)    002：固件升级中 003：设备重启中
#define MCU_FACTORY 	0x22	//MCU也可以发送指令给模组恢复出厂设置：MCU+FACTORY 如果MCU发送MCU+FCRYPOW，则模组恢复出厂后不会重启，而是发送AXX+POW+OFF给MCU请求断电。
#define MCU_BAT_ON  	0x23	//MCU通知模组，电池处于充电状态
#define MCU_BAT_OFF 	0x24	//MCU通知模组，电池处于放电状态
#define MCU_BAT_VAL 	0x25	//MCU通知模组电池电量，xxx为百分比，例如001表示1%，100表示100%；如果MCU从未通知模组电池电量，则模组认为没有电池存在。
#define MCU_BAT_LOW 	0x26	//MCU通知模块电池低电

//******************************************************************
//	6  MCU在线升级
//******************************************************************
#define MCU_VER_NUM 	0x30	//MCU发送当前的版本号
#define MCU_UPG_STA 	0x31	//MCU同意升级开始
#define MCU_UPG_RUN 	0x32	//MCU升级成功继续
#define MCU_UPG_FAL 	0x33	//MCU升级失败终止升级
#define MCU_UPG_END 	0x34	//MCU升级已完成

//******************************************************************
//	7 网络命令
//******************************************************************
//WPS配网
#define MCU_WIF_WPS 	0x38	//按下WPS按键发的指令，MCU控制模块开始WPS扫描，扫描WPS Server，（此时如果有路由器按下WPS按键，或者另一个模块收到MCU+WIF+WSS指令，则此模块会连接之）�
#define MCU_WIF_WSS 	0x39	//MCU控制模块进入WPS Server模式,WSS模式和WPS模式是两个对应的模式，WPS模式的设备会自动连接WSS模式的设备，不需要输入密码
#define MCU_WIF_EPS 	0x3A	//MCU控制模块退出WPS/WSS配网模式

//控制Station
#define MCU_STA_ON  	0x3D 	//Enable WiFi Station，默认开机是enable的 
#define MCU_STA_OFF 	0x3E	//关闭WiFi Station，模组会断开与路由器或者主音箱的链接
#define MCU_STA_DEL 	0x3F	//断开当前路由器链接并删除这个路由器记录

//控制AP
#define MCU_RA0_ON		0x42	//打开AP热点功能
#define MCU_RA0_OFF 	0x43	//隐藏AP热点功能

//网络状态通知
#define MCU_STA_GET 	0x46	//- MCU通知模块发送路由器连接情况
#define MCU_RA0_GET 	0x47	//- MCU通知模块发送设备被连接情况
#define MCU_ETH_GET 	0x48	//- MCU通知模块发送设备以太网连接情况
#define MCU_WWW_GET 	0x49	//- MCU通知模块发送设备上网情况

//******************************************************************
//	8 播放命令
//******************************************************************
//播放控制
#define MCU_PLY_PAU 	0x50	//-	MCU控制模块，如果暂停则开始播放，如果播放中则暂停播放
#define MCU_PLY_PLA		0x51	//-	MCU控制模块恢复播放，如果不是暂停状态则忽略
#define MCU_PLY_PUS		0x52	//-MCU控制模块暂停播放，如果不是播放状态则忽略
#define MCU_PLY_STP		0x53	//MCU控制模块停止播放，并把当前播放歌单清空
#define MCU_PLY_PUQ 	0x54	//MCU控制模块播放上次的歌单，如果是开机，则播放上次关机时正在播放的歌单
#define MCU_PLY_NXT		0x55	//-	MCU控制模块播放下一首
#define MCU_PLY_PRV		0x56	//-	MCU控制模块播放上一首

//音量控制
#define MCU_VOL_ADD 	0x5A	//-	MCU控制模块调节音量，xxx为0~100，模块会通过AXX+VOL+xxx返回实际音量
#define MCU_CUR_VOL 	0x5B	//-	MCU通知模块当前音量，xxx为0~100，注意：是-xxx不是+xxx，这表示模组只会更新当前音量的值，不会实际调节音量，也不会回复MCU
#define MCU_VOL_GET 	0x5C	//-	MCU通知模块发送音量，模块会回复AXX+VOL+xxx
#define MCU_MUT_SET 	0x5D	//-	MCU控制模块音量静音, xxx为0或者1, 如果MCU自行控制音量，此指令无效
#define MCU_MUT_GET 	0x5E	//-	MCU通知模块发送静音状态，模块会回复AXX+MUT+xxx
#define MCU_CUR_MUT 	0x5F	//-	如果MCU自行控制音量，MCU通知模块静音状态，xxx为0或者1

//预置歌单和切换
#define MCU_KEY_XNN 	0x63	//MCU通知模块有按键事件发生, nn参数代表按键值 (10进制，01–99), x表示消息:  2 长按； 3按下；4松开; 其他值为短按
#define MCU_KEY_NXT 	0x64	// MCU通知模块播放下一个按键预置歌单
#define MCU_KEY_PRE 	0x65	// MCU通知模块播放上一个按键预置歌单
#define MCU_PRE_SET 	0x66	//把当前播放的歌单预置到空白按键，如果都已经有预置内容，则依次覆盖。
#define MCU_PRE_NNN 	0x67	//把当前播放的歌单预置到指定按键，nnn可以从001到009

//播放状态通知
#define MCU_PLY_GET		0x6A	//-	MCU通知模块发送播放状态，模块会回复AXX+PLY+xxx
#define MCU_PLP_GET		0x6B	//- MCU通知模块发送当前播放Repeat模式
#define MCU_PLP_XXX		0x6C	//循环模式设置：MCU可以通过MCU+PLP+GET来获取当前模式(000 loop all 001 single loop 002 shuffle 003 shuffle once Others: no loop)

//指定播放USB/TF卡某首歌曲
#define MCU_MSX_NNN 	0x6F	//MCU通知模块播放USB(x=1)或TF卡(x=2)上第nnn首歌曲(n从1开始)

//******************************************************************
//	9  时间和定时器命令
//******************************************************************
#define MCU_RTC_GET 	0x75	//查询当前系统时间
#define MCU_WEK_GET 	0x76	//查询当前系统日期
#define MCU_SND_RTC 	0x77	//设置WiFi 的RTC
#define MCU_ALM_NXT 	0x78	//获取下一个闹钟的时间.

//******************************************************************
//	10 多房间功能命令
//******************************************************************
#define MCU_SLV_CHK		0x80	//MCU查询当前设备子音箱状态
#define MCU_SLV_GET		0x81	//-MCU获取子音箱的数量
#define MCU_JNGROUP 	0x82	//-MCU一键组网功能目前定义为：如果设备处于联网状态，则把同一网段先的其他设备拉为子音箱。如果设备处于不联网状态，则把其他不联网的设备拉为子音箱
#define MCU_UNGROUP 	0x83	//-MCU一键退网功能

//******************************************************************
//	11 AUX/BT/RCA/USB 模式切换命令
//******************************************************************
#define MCU_PLM_SUB   	0x88  	//- 通过按键切换模式时，MCU发送 MCU+PLM-xxx给模组。请注意后一个指令是-不是+，模组只会修改模式，不会做任何事情。
#define MCU_PLM_ADD   	0x89  	//- 通过按键切换模式时，MCU发送 MCU+PLM+xxx给模组。请注意后一个指令是+不是-，模组会修改模式，同时从I2S data-in抓数据同步转发到各个子音箱播放
#define MCU_PLM_GET 	0x8A	//- 查询当前模式
#define MCU_USB_GET 	0x8B	//-查询u盘是否存在，模组会回复AXX+USB+001或者AXX+USB+000
#define MCU_MMC_GET 	0x8C	//-查询T卡是否存在，模组会回复AXX+MMC+001或者AXX+MMC+000

//******************************************************************
//	12  MCU配置
//******************************************************************
//配置网络
#define MCU_AP_SCAN 	0x90	//MCU请求模组扫描并发送网络中的AP列表，模组以AXX+WAN+LST返回
#define MCU_AP_CONN 	0x91	//MCU请求模组连接指定的AP，指定ap的SSID和密码，SSID和密码之间用 : 隔开。
#define MCU_STA_INF 	0x92	//MCU查询模组当前的网络连接信息，模组返回AXX+STA+INFessid&，其中essid是当前连接的路由器的名字。
#define MCU_WCH_SET 	0x93	//设置不联网时，默认WiFi信道
#define MCU_PWD_SET 	0x94	//设置WiFi AP模式的默认密码

//设置名称
#define MCU_SID_SET		0x99	//设置SSID信号名称
#define MCU_NAM_SET 	0x9A	//设置设备的名称（DLNA，Airplay显示的名称）

//******************************************************************
//	13  LCD显示屏
//******************************************************************
#define MCU_INF_GET 	0xA0	//获取详细的设备信息，具体格式可参考 HttpAPI getStatusEx指令。以JSON格式编码，比较长，有数百个字节。
#define MCU_PINF_GET 	0xA1	//获取播放的详细信息，具体格式可参考 HttpAPI getPlayerStatus指令。以JSON格式编码；如果歌曲名比较长的话，会有数百个字符。
#define MCU_WEA_GET 	0xA2	//获取天气信息，上网状态才有效（AXX+WWW+001），格式为：城市;日期;星期几;天气;最高温度;最低温度;风;PM2.5
#define MCU_DEV_GET 	0xA3	//获取设备的简要信息，格式为：SSID;版本类型;设备名称;连接的路由器的名字;信号强度;电池状态;电池电量其中版本类型如果为backup，表示系统进入异常模式
#define MCU_MEA_GET 	0xA4	//MCU主动查询当前的歌曲信息Metadata，回复内容默认编码是UTF8格式，回复格式和AXX+MEA+DAT命令一样
#define MCU_MEA_GBK 	0xA5	//MCU主动查询当前的歌曲信息Metadata，回复内容编码是GBK格式，回复格式和AXX+MEA+DAT命令一样

//******************************************************************
//	14  阿里小智平台特殊命令
//******************************************************************
#define MCU_PLY_LKE		0xAB	//MCU通知模块收藏/喜欢
#define MCU_CHL_NXT 	0xAC	//MCU通知模块下一个频道
#define MCU_REC_ON  	0xAD	//当短按录音键后，MCU发送该命令开始录音
#define MCU_REC_OFF 	0xAE	//当再次短按录音键后，MCU发送该命令，结束录音
#define MCU_AIM_ON  	0xAF	//当长按群聊键1秒后，MCU发送该命令开始发起群聊录音
#define MCU_AIM_OFF 	0xB0	//当群聊键弹起时，MCU发送该命令，结束录音并发送
#define MCU_AIM_PLY 	0xB1	//当短按按群聊键后，MCU发送该命令，告知模块播放群聊消息

//******************************************************************
//	15 产测命令
//******************************************************************
#define MCU_FCRYPOW 	0xB3	//如果MCU发送MCU+FCRYPOW，则模组恢复出厂后不会重启，而是发送AXX+POW+OFF给MCU请求断电。

//******************************************************************
//	16 透传
//******************************************************************
//处于同一个多房间的MCU之间可以传递指令
#define MCU_M2S_NNN 	0xB7	//设备是Master时，MCU发送MCU+M2S+nnn指令，模组会把指令转发给所有的Slave
#define MCU_S2M_NNN 	0xB8	//设备是Slave时，MCU发送MCU+S2M+nnn指令，模组会把指令转发给Master

//App和MCU之间也可以透传数据
#define MCU_PAS_DATA 	0xBD	//MCU发给App, WiFi模组收到含有MCU+PAS+的MCU指令时，会继续接收数据直到发现 & 符号，然后发送 MCU+PAS+data&指令给App

//******************************************************************
//	18 语音提示命令
//******************************************************************
#define MCU_LAU_XXX 	0xCA	//-MCU回复当前语言
#define MCU_LAU_GET 	0xCD	//-MCU查询当前语言
#define MCU_PMT_GET 	0xCE	//-MCU查询语音提示状态

//******************************************************************
//	19 I2S bypass
//******************************************************************
#define MCU_BYP_000 	0xD3 	//关闭i2s bypass mode
#define MCU_BYP_001 	0xD4	//打开 i2s bypass mode
#define MCU_BYP_GET 	0xD5	//查询I2S bypass 模式

//******************************************************************
//	20 语音识别命令
//******************************************************************
#define MCU_TALK_ON  	0xDA	//- MCU发送启动语音识别命令
#define MCU_TLK_OFF 	0xDB	//- 当语音识别键弹起时，MCU发送该命令，结束语音识别
#define MCU_TLK_TIM 	0xDC	//-MCU通知模块语音播报时间，默认不支持
#define MCU_WEATHER 	0xDD	//-MCU通知模块语音播报天气，默认不支持
#define MCU_TLK_ENA 	0xDE	//-MCU通知模块语音播报天气，默认不支持
#define MCU_TLK_DIS 	0xDF	//-MCU通知模块语音播报天气，默认不支持

//******************************************************************
//	21 EQ设置
//******************************************************************
#define MCU_PAS_EQ_TREBLE	0xE2	//回复EQ Treble值
#define MCU_PAS_EQ_BASS 	0xE3	//回复EQ Bass值


#ifdef __cplusplus
}
#endif//__cplusplus

#endif

