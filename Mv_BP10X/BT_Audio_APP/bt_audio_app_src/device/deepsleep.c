#include "app_config.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "gpio.h"
#include "timeout.h"
#include "audio_adc.h"
#include "dac.h"
#include "clk.h"
#include "chip_info.h"
#include "otg_device_hcd.h"
#include "rtc.h"
#include "irqn.h"
#include "debug.h"
#include "adc_key.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adc_key.h"
//#include "OrioleReg.h"//for test
#include "hdmi_in_api.h"
#include "sys.h"
#include "sadc_interface.h"
#include "watchdog.h"
#include "backup.h"
#include "ir_key.h"
#include "app_message.h"
#include "reset.h"
#include "bt_stack_service.h"

#if	defined(CFG_FUNC_DEEPSLEEP_EN) || defined(CFG_FUNC_MAIN_DEEPSLEEP_EN)
HDMIInfo  			 *gHdmiCt;

inline void GIE_ENABLE(void);
inline void SystemOscConfig(void);
inline void SleepMainAppTask(void);
void SleepAgainConfig(void);

void WakeupMain(void);

inline void SleepMain(void);
void LogUartConfig(bool InitBandRate);

#define CHECK_SCAN_TIME				5000		//����ȷ����Ч����Դ ɨ����ʱms��

static uint32_t sources;
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
uint32_t alarm = 0;
#endif

__attribute__((section(".driver.isr")))void WakeupInterrupt(void)
{
	sources |= Power_WakeupSourceGet();
	Power_WakeupSourceClear();
}


void SystermGPIOWakeupConfig(PWR_SYSWAKEUP_SOURCE_SEL source,PWR_WAKEUP_GPIO_SEL gpio,PWR_SYSWAKEUP_SOURCE_EDGE_SEL edge)
{
	if(gpio < 32)
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   (1 << gpio));
		GPIO_RegOneBitClear(GPIO_A_OE, (1 << gpio));
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			GPIO_RegOneBitSet(GPIO_A_PU, (1 << gpio));//��ΪоƬ��GPIO���ڲ�����������,ѡ���½��ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitClear(GPIO_A_PD, (1 << gpio));
		}
		else if(edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			GPIO_RegOneBitClear(GPIO_A_PU, (1 << gpio));//��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitSet(GPIO_A_PD, (1 << gpio));
		}
	}
	else if(gpio < 41)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   (1 << (gpio - 32)));
		GPIO_RegOneBitClear(GPIO_B_OE, (1 << (gpio - 32)));
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			GPIO_RegOneBitSet(GPIO_B_PU, (1 << (gpio - 32)));//��ΪоƬ��GPIO���ڲ�����������,ѡ���½��ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitClear(GPIO_B_PD, (1 << (gpio - 32)));
		}
		else if( edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			GPIO_RegOneBitClear(GPIO_B_PU, (1 << (gpio - 32)));//��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitSet(GPIO_B_PD, (1 << (gpio - 32)));
		}
	}
	else if(gpio == 41)
	{

		BACKUP_WriteEnable();

		BACKUP_C0RegSet(BKUP_GPIO_C0_REG_IE_OFF, TRUE);
		BACKUP_C0RegSet(BKUP_GPIO_C0_REG_OE_OFF, FALSE);
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PU_OFF, TRUE);
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PD_OFF, FALSE);
		}
		else if( edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PU_OFF, FALSE);//��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PD_OFF, TRUE);
		}
		BACKUP_WriteDisable();
	}

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(source, gpio, edge);
	Power_WakeupEnable(source);

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();
}

void SystermIRWakeupConfig(IR_MODE_SEL ModeSel, IR_IO_SEL GpioSel, IR_CMD_LEN_SEL CMDLenSel)
{
	Clock_BTDMClkSelect(RC_CLK32_MODE);
	Reset_FunctionReset(IR_FUNC_SEPA);
	IRKeyInit();
	IR_WakeupEnable();


	if(GpioSel == IR_GPIOB6)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX6);
		GPIO_RegOneBitSet(GPIO_B_IN,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OUT, GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX6);
	}
	else if(GpioSel == IR_GPIOB7)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX7);
		GPIO_RegOneBitSet(GPIO_B_IN,   GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_OUT, GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX7);
	}
	else
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX29);
		GPIO_RegOneBitSet(GPIO_A_IN,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OUT, GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX29);
	}
	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE9_IR, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE9_IR);
}



#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
//RTC���� ���������RTC�ж�
void SystermRTCWakeupConfig(uint32_t SleepSecond)
{
	//RTC_REG_TIME_UNIT start;
	RTC_ClockSrcSel(OSC_12M);
	RTC_IntDisable();
	RTC_IntFlagClear();
	RTC_WakeupDisable();

	alarm = RTC_SecGet() + SleepSecond;
	RTC_SecAlarmSet(alarm);
	RTC_WakeupEnable();
	RTC_IntEnable();

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	NVIC_EnableIRQ(Rtc_IRQn);
	NVIC_SetPriority(Rtc_IRQn, 1);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE7_RTC, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE7_RTC);
}
#endif //CFG_PARA_WAKEUP_RTC



void SystermDeepSleepConfig(void)
{
	//GPIOA0~A31��B0~B7�ĸ��ù��ܣ�������ѡ��ر�
	GPIO_PortAModeSet(GPIOA0, 0x0000);
	GPIO_PortAModeSet(GPIOA1, 0x0000);
	GPIO_PortAModeSet(GPIOA2, 0x0000);
	GPIO_PortAModeSet(GPIOA3, 0x0000);
	GPIO_PortAModeSet(GPIOA4, 0x0000);
	GPIO_PortAModeSet(GPIOA5, 0x0000);
	GPIO_PortAModeSet(GPIOA6, 0x0000);//A6����UART0 Tx
	GPIO_PortAModeSet(GPIOA7, 0x0000);
	GPIO_PortAModeSet(GPIOA8, 0x0000);
	GPIO_PortAModeSet(GPIOA9, 0x0000);
	GPIO_PortAModeSet(GPIOA10, 0x0000);//A10����UART1 Tx
	GPIO_PortAModeSet(GPIOA11, 0x0000);
	GPIO_PortAModeSet(GPIOA12, 0x0000);
	GPIO_PortAModeSet(GPIOA13, 0x0000);
	GPIO_PortAModeSet(GPIOA14, 0x0000);
	GPIO_PortAModeSet(GPIOA15, 0x0000);
	GPIO_PortAModeSet(GPIOA16, 0x0000);
 	GPIO_PortAModeSet(GPIOA17, 0x0000);
 	GPIO_PortAModeSet(GPIOA18, 0x0000);
 	GPIO_PortAModeSet(GPIOA19, 0x0000);
 	GPIO_PortAModeSet(GPIOA20, 0x0000);
 	GPIO_PortAModeSet(GPIOA21, 0x0000);
 	GPIO_PortAModeSet(GPIOA22, 0x0000);
	GPIO_PortAModeSet(GPIOA23, 0x0000);
	GPIO_PortAModeSet(GPIOA24, 0x0000);
	GPIO_PortAModeSet(GPIOA25, 0x0000);
	GPIO_PortAModeSet(GPIOA26, 0x0000);
#if (CFG_PARA_WAKEUP_GPIO_CEC != WAKEUP_GPIOA27)
	GPIO_PortAModeSet(GPIOA27, 0x0000);
#endif
	GPIO_PortAModeSet(GPIOA28, 0x0000);
	GPIO_PortAModeSet(GPIOA29, 0x0000);
//  GPIO_PortAModeSet(GPIOA30, 0x0000);//����ΪSW���ص��Կ� bkd
//  GPIO_PortAModeSet(GPIOA31, 0x0000);

//	GPIO_PortBModeSet(GPIOB0, 0x000);//
//	GPIO_PortBModeSet(GPIOB1, 0x000);
	GPIO_PortBModeSet(GPIOB2, 0x000);
	GPIO_PortBModeSet(GPIOB3, 0x000);
	GPIO_PortBModeSet(GPIOB4, 0x000);
	GPIO_PortBModeSet(GPIOB5, 0x000);
	GPIO_PortBModeSet(GPIOB6, 0x000);
	GPIO_PortBModeSet(GPIOB7, 0x000);

	SleepAgainConfig();//������ͬ

	SleepMain();

#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	if(gHdmiCt == NULL)
	{
		HDMI_CEC_DDC_Init();
		gHdmiCt->hdmiReportStatus = 0;
	}
//	WaitMs(10);
	while(!HDMI_CEC_IsReadytoDeepSleep(10));//����10ms�����Ϻ���18msû��ͨ�ź��ж���Ӧ��
#endif
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
	alarm = 0;
#else

	//Clock_LOSCDisable(); //����RTCӦ���򲻹ر�32K���� BKD mark sleep
//	BACKUP_32KDisable(OSC32K_SOURCE);// bkd add

#endif
}

//���ö������Դʱ��sourceͨ���������������ظ���
void WakeupSourceSet(void)
{
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
	SystermRTCWakeupConfig(CFG_PARA_WAKEUP_TIME_RTC);
#endif	
#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_ADCKEY, CFG_PARA_WAKEUP_GPIO_ADCKEY, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_IR)
	SystermIRWakeupConfig(CFG_PARA_IR_SEL, CFG_RES_IR_PIN, CFG_PARA_IR_BIT);
#endif	
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_CEC, CFG_PARA_WAKEUP_GPIO_CEC, SYSWAKEUP_SOURCE_BOTH_EDGES_TRIG);
#endif	
}

{

	uint32_t GpioAPU_Back,GpioAPD_Back,GpioBPU_Back,GpioBPD_Back;

	WDG_Disable();

#ifdef CFG_APP_BT_MODE_EN
	BT_ModuleClose();
#endif
	
	GpioAPU_Back = GPIO_RegGet(GPIO_A_PU);
	GpioAPD_Back = GPIO_RegGet(GPIO_A_PD);
	GpioBPU_Back = GPIO_RegGet(GPIO_B_PU);
	GpioBPD_Back = GPIO_RegGet(GPIO_B_PD);
	SystermDeepSleepConfig();
	WakeupSourceSet();
	Power_GotoDeepSleep();
	while(!SystermWackupSourceCheck())
	{
		SleepAgainConfig();
		Power_WakeupDisable(0xff);
		WakeupSourceSet();
		Power_GotoDeepSleep();
	}
	Power_WakeupDisable(0xff);
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	HDMI_CEC_DDC_DeInit();
#endif
	//GPIO�ָ�������
	GPIO_RegSet(GPIO_A_PU, GpioAPU_Back);
	GPIO_RegSet(GPIO_A_PD, GpioAPD_Back);
	GPIO_RegSet(GPIO_B_PU, GpioBPU_Back);
	GPIO_RegSet(GPIO_B_PD, GpioBPD_Back);
	WDG_Feed();
	WakeupMain();
	SysTickInit();
	WDG_Feed();
#if defined(CFG_FUNC_DISPLAY_EN)
    DispInit(0);
#endif
}


bool SystermWackupSourceCheck(void)
{
#ifdef CFG_RES_ADC_KEY_SCAN
	AdcKeyMsg AdcKeyVal;
#endif

#ifdef CFG_PARA_WAKEUP_SOURCE_IR
	IRKeyMsg IRKeyMsg;

#endif
	TIMER WaitScan;

#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY)
	SarADC_Init();
	AdcKeyInit();
#endif

//********************
	//����IO����
	LogUartConfig(FALSE);//�˴��������clk�����ʣ���Ϊ��ʱ�������䡣
	SysTickInit();
	DBG("Scan:%x", (int)sources);
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
	if(sources & CFG_PARA_WAKEUP_SOURCE_RTC)
	{
		sources = 0;//����Դ����
		DBG("Alarm!%d", RTC_SecGet());
		return TRUE;
	}
	else if(alarm)//����RTC�����¼���ʧ
	{
		uint32_t NowTime;
		NowTime = RTC_SecGet();
		if(NowTime + 2 + CHECK_SCAN_TIME / 1000 > alarm)//������ڶ������Դ��rtc������ǰ����()���Ա��ⶪʧ
		{
			DBG("Timer");
			sources = 0;//����Դ����
			alarm = 0;
			return TRUE;
		}
	}
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	bool CecRest = FALSE;//cec����״̬(18ms����)������Ӱ����������ͻ��ѡ�
	HDMI_HPD_CHECK_IO_INIT();
#endif

	TimeOutSet(&WaitScan, CHECK_SCAN_TIME);
	while(!IsTimeOut(&WaitScan)
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
			|| !CecRest
#endif
		)
	{
#ifdef CFG_PARA_WAKEUP_SOURCE_IR
		if(sources & CFG_PARA_WAKEUP_SOURCE_IR)
		{
			IRKeyMsg = IRKeyScan();
			if(IRKeyMsg.index != IR_KEY_NONE && IRKeyMsg.type != IR_KEY_UNKOWN_TYPE)
			{
				DBG("IRID:%d,type:%d\n", IRKeyMsg.index, IRKeyMsg.type);
				SetGlobalKeyValue(IRKeyMsg.type,IRKeyMsg.index);
				if(GetGlobalKeyValue() == MSG_DEVICE_SERVICE_DEEPSLEEP)
				{
					sources = 0;
					ClrGlobalKeyValue();
					return TRUE;
				}
				ClrGlobalKeyValue();
			}
		}
#endif
#ifdef CFG_PARA_WAKEUP_SOURCE_ADCKEY
		if(sources & CFG_PARA_WAKEUP_SOURCE_ADCKEY)
		{
			AdcKeyVal = AdcKeyScan();
			if(AdcKeyVal.index != ADC_CHANNEL_EMPTY && AdcKeyVal.type != ADC_KEY_UNKOWN_TYPE)
			{
				DBG("KeyID:%d,type:%d\n", AdcKeyVal.index, AdcKeyVal.type);
				SetGlobalKeyValue(AdcKeyVal.type,AdcKeyVal.index);
				if(GetGlobalKeyValue() == MSG_DEVICE_SERVICE_DEEPSLEEP)
				{
					sources = 0;
					ClrGlobalKeyValue();
					return TRUE;
				}
				ClrGlobalKeyValue();
			}
		}
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
		HDMI_CEC_Scan();
		if(gHdmiCt->hdmi_poweron_flag)
		{
			DBG("CEC PowerOn\n");
			return TRUE;
		}
		if(IsTimeOut(&WaitScan))//��ʱ֮��ȴ�������ƽ��ͬʱ����scan��
		{
			CecRest = HDMI_CEC_IsReadytoDeepSleep(6);
		}
#endif

	}
	sources = 0;
//	SysTickDeInit();
	return FALSE;
}


void SleepAgainConfig(void)
{
	GPIO_RegSet(GPIO_A_IE,0x00000000);
	GPIO_RegSet(GPIO_A_OE,0x00000000);
	GPIO_RegSet(GPIO_A_OUTDS,0x00000000);//bkd GPIO_A_REG_OUTDS
	GPIO_RegSet(GPIO_A_PD,0xffffffff
#if defined(CFG_PARA_WAKEUP_GPIO_CEC) && defined(CFG_PARA_WAKEUP_SOURCE_CEC)//cec�˿ڲ������������ã���Ҫcec״̬���ϡ�
			& ~ BIT(CFG_PARA_WAKEUP_GPIO_CEC)
#endif
			);
	GPIO_RegSet(GPIO_A_PU,0x00000000);//��ʱ��flash��CS��������0x00400000
	GPIO_RegSet(GPIO_A_ANA_EN,0x00000000);
	GPIO_RegSet(GPIO_A_PULLDOWN0,0x00000000);//bkd
	GPIO_RegSet(GPIO_A_PULLDOWN1,0x00000000);//bkd

	GPIO_RegSet(GPIO_B_IE,0x00);
	GPIO_RegSet(GPIO_B_OE,0x00); 
	GPIO_RegSet(GPIO_B_OUTDS,0x00); // bkd mark GPIO_B_REG_OUTDS
	GPIO_RegSet(GPIO_B_PD,0xff);//B2��B3������B4,B5���� 0x1cc
	GPIO_RegSet(GPIO_B_PU,0x00);//B0��B1���� 0x03
	GPIO_RegSet(GPIO_B_ANA_EN,0x00);
	GPIO_RegSet(GPIO_B_PULLDOWN,0x00);//bkd mark GPIO_B_REG_PULLDOWN
}

#endif//CFG_FUNC_DEEPSLEEP_EN



















