//程序名：HT1621B_driver.C。
//编写人：李治清
//编写日期：2015.09.30
//系统晶体：96MHz
//MCU类型：M3
//接口定义：HT_dat-1621的data数据线，HT_wr-1621的写时钟同步线，HT_cs-1621的使能/复位线，
//          HT_rd-1621的读时钟同步线。
//使用说明：
//修改日志：
/*************************************************************************************/
//头文件包含
#include "gpio.h"
#include "HT1621B_driver.h"
#include "app_config.h"
#include "seg_panel.h"
#include "dev_detect_driver.h"
#include "sys_app.h"
#include "task_decoder.h"
#include "bt_app_func.h"
#include "bt_control_api.h"
#include "sd_card.h"
#include "eq.h"
#include "eq_params.h"
#include "clk.h"
#include "fsinfo.h"
#include "sys_vol.h"
#include "sound_remind.h"
#include "mixer.h"
#include "breakpoint.h"
#include "recorder_control.h"
#include "nvm.h"
#include "browser.h"
#include "lrc.h"
#include "timer_api.h"
#include "string_convert.h"
#include "player_control.h"
#include "rtc_control.h"
#include "radio_control.h"

#ifdef    FUNC_HT1621_DISPLAY_EN
/***********************************************************************************/
//常用控制命令
#define   SYS_dis    0x00   //关闭系统时钟
#define   SYS_on     0x01   //开系统时钟
#define   LCD_off    0x02   //关闭LCD显示
#define   LCD_on     0x03   //开LCD显示
#define   timer_dis  0x04   //关闭时钟输出
#define   WDT_dis    0x05   //关闭看门狗
#define   tone_off   0x08   //关闭声音输出
#define   XTAL       0x18   //系统晶体选择
#define   BIAS       0x29   //偏压及占空比设置

TIMER  ColDispTimer;
TIMER  PrevDispTimer;
TIMER  UpdataDispTimer;
DISP_MODULE_ID    CurDisplayId  =  DISP_MODULE_NONE;              //当前需要显示的内容
 //上一次显示的内容，用于当某个内容需要显示固定时间，在显示过程中不能刷新当前显示内容的问题�
static DISP_MODULE_ID    PrevDisplayId = DISP_MODULE_NONE;
static bool ColDispFlag;

static uint8_t const NumberTab [10] = {0xFD,0x0D,0xDB,0x9F,0x2F,0xBE,0xFE,0x1D,0xFF,0xBF};
//.......................................0.....1...2....3....4....5....6.....7....8....9.//
//为了在刷新显示数据时不影响其他标志的显示，所以将空闲BIT为1，在刷新数据时用“逻辑与”

uint8_t  DisplayBuffer [13] = {0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x67,0x00,0x00,0x00,0x00,0x00}; 	

/*************************************************************************************/
#ifdef FUNC_HT1621_DISPLAY_EN
static __INLINE void HT1621DisplayDelay(void)
{
	__nop();
}
#endif
/*************************************************************************************/
static  void  Init_1621(void)
{
      HT1621_CS_SET();
	    HT1621_CLK_SET();
	    HT1621_DAT_SET();
	    HT1621DisplayDelay();
	    HT1621_CS_CLR();
	    HT1621_CS_CLR();
	    HT1621_CLK_CLR();
	    HT1621_CLK_CLR();
}
/*************************************************************************************/
static  void  Sendchar_h(uint8_t dat,uint8_t count)                 //dat的count位写入1621，高位在前
{
      for (;count > 0;count--)
	    {
          if ((dat&0x80) == 0x00)
	           HT1621_DAT_CLR();
	        else
	           HT1621_DAT_SET();
	       HT1621_CLK_CLR();
	       HT1621DisplayDelay();
	       HT1621_CLK_SET();
	       dat <<= 1;
      }
}
/*************************************************************************************/
static  void  Sendchar_l(uint8_t dat,uint8_t count)                 //dat的count位写入1621，低位在前
{
      for (;count > 0;count--)
	    {
          if ((dat&0x01) == 0x00)
	           HT1621_DAT_CLR();
	        else
	           HT1621_DAT_SET();
	       HT1621_CLK_CLR();
	       HT1621DisplayDelay();
	       HT1621_CLK_SET();
	       dat >>= 1;
      }
}
/*************************************************************************************/
static  void  write_cmd(uint8_t command)                          //写1621控制命令
{   
      HT1621_CS_CLR();
	    Init_1621();
      Sendchar_h(0x80,3);
      Sendchar_h(command,9);
      HT1621_CS_SET();
}
/*************************************************************************************/
void  HT1621DisplayClr(void)    //清除1621所有显示、清屏
{
	    uint8_t count = 0;
	    for(count = 0; count < 13; count++)
			{
				DisplayBuffer[count] = 0x00;
			}
}
/*************************************************************************************/
void  SendHt1621DispMsg(DISP_MODULE_ID  DispId)               //传送显示命令
{
	    CurDisplayId = DispId;
}
/*************************************************************************************/
static void  DisplayModuleFlag(void)    //刷新模式和秒标志
{
	     //先清除所有模式图标。
	    DISP_MODEFLAG_OFF();
	    if(gSys.CurModuleID != MODULE_ID_RTC)
			//如果不是在RTC模式，将秒图标关闭。
			{
				 DISP_COL_OFF();
			}
	    if(gSys.CurModuleID == MODULE_ID_PLAYER_USB)
				//显示USB图标
			{
				 DISP_USB_ON();
			}
			else if(gSys.CurModuleID == MODULE_ID_PLAYER_SD)
			{
				 if((GetCurReadCard() == TFTOSD_CARD_TYPE) || \
					   (GetCurReadCard() == SD_CARD_TYPE))
				 //在SD卡模式。显示SD图标
				 {
					   DISP_SDCARD_ON();
				 }
				 else
					//在TF卡模式，显示TF图标。
				 {
					   DISP_TFCARD_ON();
				 }
			}
			else if (gSys.CurModuleID == MODULE_ID_RADIO)
			//显示FM图标
			{
				  DISP_FM_ON();
			}
}
/************************************************************************************/
static void DisplayWelcome(void)        //显示欢迎信息
{
	    uint8_t  count = 0;
			HT1621_CS_CLR();
	    Init_1621();
      Sendchar_h(0xa0,3);
      Sendchar_h(0x18,6);
      for (count = 0;count < 13 ;count++)
	    {
			  if(count == 6)
				{
           Sendchar_l(0x05,8);
				}
				else if(count == 7)
				{
					 Sendchar_l(0x67,8);
				}
				else
				{
					 Sendchar_l(0x00,8);
				}
      }
      HT1621_CS_SET();
			TimeOutSet(&PrevDispTimer, 3000);
}
/************************************************************************************/
void  HT1621DisplayInit(void)                         //初始化HT1621，并显示欢迎信息
{
      write_cmd(BIAS);                                        //LCD偏压设置
      write_cmd(XTAL);                                      //时钟源选择
	    write_cmd(timer_dis);                               //关闭时钟输出
	    write_cmd(WDT_dis);                                //关闭看门狗
	    write_cmd(tone_off);                                  //关闭声音输出
      write_cmd(SYS_on);                                  //开系统时钟
      HT1621DisplayDelay();                              //适当延时
	    TimeOutSet(&ColDispTimer, 0);
	    TimeOutSet(&PrevDispTimer, 0);           //启动显示相关定时器
      write_cmd(LCD_on);                                  //开LCD显示
	    DisplayWelcome();                                  //显示欢迎信息
}
/************************************************************************************/
static void DisplayTimer(void)        //显示播放时间
{
	        DISP_COL_ON();                       //秒符号常亮，表示正在播放
	        DisplayBuffer[8] |= 0xF7;
				  DisplayBuffer[8] &= NumberTab[gPlayContrl->CurPlayTime / 1000 / 60 / 10];
				  DisplayBuffer[7] |= 0xF7;
	        DisplayBuffer[7] &= NumberTab[gPlayContrl->CurPlayTime/ 1000 / 60 % 10];
				  DisplayBuffer[6] |= 0xF7;
	        DisplayBuffer[6] &= NumberTab[gPlayContrl->CurPlayTime / 1000 % 60 / 10];
				  DisplayBuffer[5] |= 0xF7;
	        DisplayBuffer[5] &= NumberTab[gPlayContrl->CurPlayTime / 1000 % 60 % 10];
}
/************************************************************************************/
static void DispSongNumber(void)        //显示歌曲曲目
{
	        DisplayBuffer[8] |= 0xF7;
					DisplayBuffer[8] &= NumberTab[gPlayContrl->CurFileIndex / 1000 ];
				  DisplayBuffer[7] |= 0xF7;
	        DisplayBuffer[7] &= NumberTab[gPlayContrl->CurFileIndex % 1000 / 100];
				  DisplayBuffer[6] |= 0xF7;
	        DisplayBuffer[6] &= NumberTab[gPlayContrl->CurFileIndex % 100  / 10];
				  DisplayBuffer[5] |= 0xF7;
	        DisplayBuffer[5] &= NumberTab[gPlayContrl->CurFileIndex % 10];
	        TimeOutSet(&PrevDispTimer, 2000);
}
/************************************************************************************/
static void DispAudioVol(void)        //显音量
{
	        DisplayBuffer[8] |= 0xF7;
					DisplayBuffer[8] &= 0xED;    // 字母"U"
				  DisplayBuffer[7] |= 0xF7;
	        DisplayBuffer[7] &= 0xE8;     //字母“L”
				  DisplayBuffer[6] |= 0xF7;
	        DisplayBuffer[6] &= NumberTab[(uint8_t)gSys.Volume  / 10];
				  DisplayBuffer[5] |= 0xF7;
	        DisplayBuffer[5] &= NumberTab[(uint8_t)gSys.Volume % 10];
	        TimeOutSet(&PrevDispTimer, 2000);
}
/************************************************************************************/
static void DispRtcTimer(void)        //显示实时时钟信息�
{
	     //显示年信息
	     if((GetRtcControlState() != RTC_STATE_IDLE) && (GetRtcControlSubState() == RTC_SET_YEAR)\
				   && (ColDispFlag == FALSE))
			 //当前正在设置年份，闪烁年份标志；以下消隐
			 {
				  DisplayBuffer[12] |= 0xD7;
					DisplayBuffer[12] &= DISP_HIDDEN | 0x20;
					DisplayBuffer[11] |= 0xF7;
					DisplayBuffer[11] &= DISP_HIDDEN;
				  DisplayBuffer[10] |= 0xF7;
					DisplayBuffer[10] &= DISP_HIDDEN;
				  DisplayBuffer[9] |= 0xF7;
					DisplayBuffer[9] &= DISP_HIDDEN;
			 }
			 else
			 {
					DisplayBuffer[12] |= 0xD7;
					DisplayBuffer[12] &= NumberTab[sRtcControl->DataTime.Year / 1000] | 0x20;
					DisplayBuffer[11] |= 0xF7;
					DisplayBuffer[11] &= NumberTab[sRtcControl->DataTime.Year % 1000 / 100];
				  DisplayBuffer[10] |= 0xF7;
					DisplayBuffer[10] &= NumberTab[sRtcControl->DataTime.Year % 100 / 10];
				  DisplayBuffer[9] |= 0xF7;
					DisplayBuffer[9] &= NumberTab[sRtcControl->DataTime.Year % 10];
			 }
			  //显示月信息
			 if((GetRtcControlState() != RTC_STATE_IDLE) && (GetRtcControlSubState() == RTC_SET_MON)\
				   && (ColDispFlag == FALSE))
			 //消隐月份显示
			 {
				  DISP_MONDEC_OFF();
				  DisplayBuffer[4] |= 0x0F;
					DisplayBuffer[4] &= ((DISP_HIDDEN >> 4) | 0xF0);
					DisplayBuffer[3] |= 0x70;
					DisplayBuffer[3] &= ((DISP_HIDDEN << 4) | 0x0F);
					DISP_MON_OFF();
			 }
			 else
			 //显示月信息
			 {
				  if((sRtcControl->DataTime.Mon / 10) > 0)
					{
						DISP_MONDEC_ON();
					}
					else
					{
						DISP_MONDEC_OFF();
					}
					DisplayBuffer[4] |= 0x0F;
					DisplayBuffer[4] &= ((NumberTab[sRtcControl->DataTime.Mon % 10] >> 4) | 0xF0);
					DisplayBuffer[3] |= 0x70;
					DisplayBuffer[3] &= ((NumberTab[sRtcControl->DataTime.Mon % 10] << 4) | 0x0F);
					DISP_MON_ON();
			 }
			 //显示天信息
			 if((GetRtcControlState() != RTC_STATE_IDLE) && (GetRtcControlSubState() == RTC_SET_DAY)\
				   && (ColDispFlag == FALSE))
			 //消隐天显示信息
			 {
				  DisplayBuffer[2] |= 0xF7;
					DisplayBuffer[2] &= DISP_HIDDEN;
					DisplayBuffer[1] |= 0xF7;
					DisplayBuffer[1] &= DISP_HIDDEN;
					DISP_DATE_OFF();
			 }
			 else
			 //显示天信息
			 {
					DisplayBuffer[2] |= 0xF7;
					DisplayBuffer[2] &= ((sRtcControl->DataTime.Date / 10) == 0)? DISP_HIDDEN : (NumberTab[sRtcControl->DataTime.Date / 10]);
					DisplayBuffer[1] |= 0xF7;
					DisplayBuffer[1] &= NumberTab[sRtcControl->DataTime.Date % 10];
					DISP_DATE_ON();
			 }
			 //显示小时信息
			 if((GetRtcControlState() != RTC_STATE_IDLE) && (GetRtcControlSubState() == RTC_SET_HOUR)\
				   && (ColDispFlag == FALSE))
			 //消隐小时
			 {
					DisplayBuffer[8] &= DISP_HIDDEN;
	        DisplayBuffer[7] &= DISP_HIDDEN;
			 }
			 else
			 //显示小时
			 {
					DisplayBuffer[8] |= 0xF7;
					DisplayBuffer[8] &= NumberTab[sRtcControl->DataTime.Hour / 10];
				  DisplayBuffer[7] |= 0xF7;
	        DisplayBuffer[7] &= NumberTab[sRtcControl->DataTime.Hour % 10];
			 }
			 //显示分钟信息
			 if((GetRtcControlState() != RTC_STATE_IDLE) && (GetRtcControlSubState() == RTC_SET_MIN)\
				   && (ColDispFlag == FALSE))
			 //消隐分钟
			 {
				  DisplayBuffer[6] &= DISP_HIDDEN;
	        DisplayBuffer[5] &= DISP_HIDDEN;
			 }
			 else
			 //显示分钟
			 {
				  DisplayBuffer[6] |= 0xF7;
	        DisplayBuffer[6] &= NumberTab[sRtcControl->DataTime.Min / 10];
				  DisplayBuffer[5] |= 0xF7;
	        DisplayBuffer[5] &= NumberTab[sRtcControl->DataTime.Min % 10];
			 }
}
/************************************************************************************/
static void DisplayRadioFreq(void)   //显示FM频率
{
#ifdef FUNC_RADIO_EN
	        DISP_DOT_ON();                           //开小数点                 
				  DISP_MHz_ON();                           //开频率单位显示       
	        DisplayBuffer[8] |= 0xF7;
	        DisplayBuffer[8] &= (sRadioControl->Freq / 1000 == 0)?DISP_HIDDEN : NumberTab[sRadioControl->Freq / 1000 ];
				  DisplayBuffer[7] |= 0xF7;
	        DisplayBuffer[7] &=  (sRadioControl->Freq / 100 == 0)?DISP_HIDDEN : NumberTab[sRadioControl->Freq % 1000 / 100];
				  DisplayBuffer[6] |= 0xF7;
	        DisplayBuffer[6] &=  (sRadioControl->Freq / 10 == 0)?DISP_HIDDEN : NumberTab[sRadioControl->Freq % 100  / 10];
				  DisplayBuffer[5] |= 0xF7;
	        DisplayBuffer[5] &= NumberTab[sRadioControl->Freq % 10];   
#endif	
}
/************************************************************************************/
static void DisplayRadioNumber(void)   //显示FM电台目录
{
#ifdef FUNC_RADIO_EN
	        DisplayBuffer[8] |= 0xF7;
	        DisplayBuffer[8] &= DISP_HIDDEN;   //消隐
				  DisplayBuffer[7] |= 0xF7;
	        DisplayBuffer[7] &=  0x7B;                   //字母“P”
				  DisplayBuffer[6] |= 0xF7; 
	        DisplayBuffer[6] &=  NumberTab[((uint8_t)sRadioControl->CurStaIdx +1) / 10];
				  DisplayBuffer[5] |= 0xF7;
	        DisplayBuffer[5] &= NumberTab[((uint8_t)sRadioControl->CurStaIdx+1)  % 10]; 
          TimeOutSet(&PrevDispTimer, 1000);	
#endif
}
/************************************************************************************/
static void DisplayString(void)   //显示相关模式下的字符串信息
{
	        if(gSys.CurModuleID == MODULE_ID_BLUETOOTH)
					{
	            DisplayBuffer[8] |= 0xF7;
	            DisplayBuffer[8] &= 0xEE;                    //字母“b”
				      DisplayBuffer[7] |= 0xF7;
	            DisplayBuffer[7] &=  0xE8;                   //字母“L”
				      DisplayBuffer[6] |= 0xF7; 
	            DisplayBuffer[6] &=  0xED;                   //字母“U”
				      DisplayBuffer[5] |= 0xF7;
	            DisplayBuffer[5] &= 0xFA;                    //字母“E”
					}
					else if(gSys.CurModuleID == MODULE_ID_PLAYER_SD || gSys.CurModuleID == MODULE_ID_PLAYER_USB)
					{
						  DisplayBuffer[8] |= 0xF7;
	            DisplayBuffer[8] &= 0x7B;                    //字母“p”
				      DisplayBuffer[7] |= 0xF7;
	            DisplayBuffer[7] &=  0x7F;                   //字母“A”
				      DisplayBuffer[6] |= 0xF7; 
	            DisplayBuffer[6] &=  0xED;                   //字母“U”
				      DisplayBuffer[5] |= 0xF7;
	            DisplayBuffer[5] &= 0xBE;                    //字母“S”
					}
					else if(gSys.CurModuleID == MODULE_ID_LINEIN)
					{
						  DisplayBuffer[8] |= 0xF7;
	            DisplayBuffer[8] &= DISP_HIDDEN;    //消隐
				      DisplayBuffer[7] |= 0xF7;
	            DisplayBuffer[7] &=  0x7F;                   //字母“A”
				      DisplayBuffer[6] |= 0xF7; 
	            DisplayBuffer[6] &=  0xED;                  //字母“U”
				      DisplayBuffer[5] |= 0xF7;
	            DisplayBuffer[5] &= 0x6F;                    //字母“H”
					}
}
/************************************************************************************/
static void  HT1621DisplaySet(void)
{
	   //显示停留的时间到了，或者当前正在显示的内容需要刷新
	  if(IsTimeOut(&PrevDispTimer) || CurDisplayId == PrevDisplayId)
		{
			  DISP_RTCDATE_OFF();                                      //清除日期信息
			  DISP_MAINFLAG_OFF();                                    //关闭主屏幕的其他图标
			  switch(CurDisplayId)
				{
					case  DISP_MODULE_PLAYTIMER:              //显示播放时间
						 DisplayTimer();
					   break;
					case  DISP_MODULE_SONGNUM:                //显示歌曲曲目
						DispSongNumber();
					  break;
					case  DISP_MODULE_AUDIOVOL:                //显示音量级别信息
						DispAudioVol();
					  break;
					case  DISP_MODULE_RTCTIMER:                //显示实时时钟信息
						DispRtcTimer();
						break;
					case  DISP_MODULE_RADIOFREQ:             //显示FM收台频率
						DisplayRadioFreq();
            break;	
          case  DISP_MODULE_RADIONUM:              //显示FM电台目录
						DisplayRadioNumber();
            break;					
          case  DISP_MODULE_WELCOME:               //显示欢迎信息
						//DisplayWelcome();
            break;
          case  DISP_MODULE_STRING:                     //显示相关模式下的字符串
						DisplayString();
            break;					
					default:
						break;
				}
				PrevDisplayId = CurDisplayId;
				CurDisplayId = DISP_MODULE_NONE;
		}
}
/*************************************************************************************/
void  HT1621UpdataDisplay(void)    //刷新HT1621显示�
{
	    uint8_t  count = 0;
	    static bool DisplayState;
	    //这里判断是否有有效的信息需要显示
	    if(CurDisplayId  !=  DISP_MODULE_NONE && CurDisplayId  !=  DISP_MODULE_WELCOME)
			{
				  DisplayModuleFlag();
	        HT1621DisplaySet();
			}
			//判断是否需要显示秒图标。
		 if(IsTimeOut(&ColDispTimer) && IsTimeOut(&PrevDispTimer) && (gSys.CurModuleID == MODULE_ID_RTC))
		 {
			    //这里不断传送显示实时时钟命令，是为了在设置状态时相应设置位闪烁，这里可以去掉
			    SendHt1621DispMsg(DISP_MODULE_RTCTIMER);
				  TimeOutSet(&ColDispTimer, 500);
				  ColDispFlag = !ColDispFlag;
				  if(ColDispFlag )
					{
						 DISP_COL_ON();
						//设置闹钟状态，闹钟图标闪烁
						 if(GetRtcControlState() == RTC_STATE_SET_ALARM)
						 {
							 DISP_ALARM_ON();
						 }
					}
					else
					{
						 DISP_COL_OFF();
						//设置闹钟状态，闹钟图标闪烁
						 if(GetRtcControlState() == RTC_STATE_SET_ALARM)
						 {
							 DISP_ALARM_OFF();
						 }
					}
			}
			//这里防止写HT1621时出现重入问题，时钟混乱而无显示
	    if(DisplayState == FALSE)                        
		  {
				  DisplayState = TRUE;
				  HT1621_CS_CLR();
	        Init_1621();
          Sendchar_h(0xa0,3);
          Sendchar_h(0x18,6);
          for (count = 0;count < 13 ;count++)
	        {
                Sendchar_l(DisplayBuffer[count],8);
          }
          HT1621_CS_SET();
				  DisplayState = FALSE;
			}
}

#endif
