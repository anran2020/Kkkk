/**
 ******************************************************************************
 * 文件:    ctp_msg_idn.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:    
 * 内容简介:     通讯协议 消息 id 定义生声明
 *                   
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                                                                            创建该文件
 *
 *
 *
 ******************************************************************************
 **/
#ifndef  _CTP_MSG_IDN_H__
#define _CTP_MSG_IDN_H__

//----------------------------------------------------------------------------
//define
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//消息 iD 定义
//----------------------------------------------------------------------------

typedef enum {

	CTP_MSG_ack=0,
	CTP_MSG_connect,
	CTP_MSG_sample,
	CTP_MSG_control,
	CTP_MSG_search,
	CTP_MSG_iapRerun,
	CTP_MSG_iapStart,
	CTP_MSG_iapData,
	CTP_MSG_iapBackup,
CTP_MSG_caliNotic,//9	通知进入修调
CTP_MSG_caliStart,//	10	启动修调/复检/计量
CTP_MSG_caliSample,//	11	采样修调数据
CTP_MSG_caliUpdateCode,//	12	更新KB值
CTP_MSG_caliStop,//	13	整箱退出修调/复检/计量
CTP_MSG_readProtPara,//	14	读取保护参数
CTP_MSG_writeProtPara,//	15	设置保护参数
CTP_MSG_MSConnect,//	16	
CTP_MSG_MSSample,//	17	
CTP_MSG_calivoltageNotice,//	18	电压修调通知报文
CTP_MSG_calivoltageStart,//	19	电压修调启动报文
CTP_MSG_calivoltageSample,//	20	电压修调采样报文
CTP_MSG_calivoltageUpdateCode,//	21	电压更新修调码报文
CTP_MSG_calivoltageStop,//	22	电压停止修调报文
CTP_MSG_iapRerunV2,//	23	多帧格式报文，收到此命令时，运行状态与升级类型一致，则立马做相应回复，否则跳转到对应运行状态后回复
CTP_MSG_iapStartV2,//	24	启动升级,增加升级类型BOOT等
CTP_MSG_iapDataV2,//	25	升级文件的数据
CTP_MSG_backupDataV2,//	26	读取程序备份的数据
CTP_MSG_autoConnect,//	27	lc自主联机
CTP_MSG_cmbSample,//	28	并机设备采样报文，单通道采样
CTP_MSG_cmbCtrl	29	并机设备通道控制，单通道控制


	CTP_MSG_commException,					//CAN离线，通讯异常
	CTP_MSG_MAX,						//消息id 最大值
	
}CANtpMsgIDN;





//----------------------------------------------------------------------------
//							end  of file
//----------------------------------------------------------------------------
#endif

