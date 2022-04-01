/**
 ******************************************************************************
 * 文件:bsp_w5500.h
 * 作者:   zzx
 * 版本:   V0.01
 * 日期:     2014-12-01
 * 内容简介:
 *
 ******************************************************************************
 *  					文件历史
 *
 * 版本号-----  日期   -----   作者    ------   说明
 * v0.1                       2014-12-01          zzx                 创建该文件
 *
 *
 *
 ******************************************************************************
 **/

#ifndef  _NTP_W5500_H__
#define  _NTP_W5500_H__

//----------------------------------------------------------------------------
//includes
//----------------------------------------------------------------------------
#include "mlos_dtd.h"
#include "ntp_w5500_def.h"
#include "ntp_def.h"


//----------------------------------------------------------------------------
//文件条件编译
//----------------------------------------------------------------------------

#if 1

//---------------------------------------------------------------------------------
//寄存器操作接口
//---------------------------------------------------------------------------------

#define w5500_r_buf(addr,buf,len) 	w5500_rw_buf(addr,buf,len,1)
#define w5500_w_buf(addr,buf,len) 	w5500_rw_buf(addr,buf,len,0)

#define setMR(mr) \
	w5500_w_byte(_MR,mr)

#define getMR() \
		w5500_r_byte(_MR)

#define setGAR(gar) \
		w5500_w_buf(GAR,gar,4)

#define getGAR(gar) \
		w5500_r_buf(GAR,gar,4)

#define setSUBR(subr) \
		w5500_w_buf(SUBR, subr,4)

#define getSUBR(subr) \
		w5500_r_buf(SUBR, subr, 4)

#define setSHAR(shar) \
		w5500_w_buf(SHAR, shar, 6)

#define getSHAR(shar) \
		w5500_r_buf(SHAR, shar, 6)

#define setSIPR(sipr) \
		w5500_w_buf(SIPR, sipr, 4)

#define getSIPR(sipr) \
		w5500_r_buf(SIPR, sipr, 4)

#define setINTLEVEL(intlevel)  {\
		w5500_w_byte(INTLEVEL,   (mlu8)(intlevel >> 8)); \
		w5500_w_byte(_W5500_OFFSET_INC(INTLEVEL,1), (mlu8) intlevel); \
	}

#define getINTLEVEL() \
		((w5500_r_byte(INTLEVEL) << 8) + w5500_r_byte(_W5500_OFFSET_INC(INTLEVEL,1)))

#define setIR(ir) \
		w5500_w_byte(IR, (ir & 0xF0))

#define getIR() \
		(w5500_r_byte(IR) & 0xF0)

#define setIMR(imr) \
		w5500_w_byte(IMR, imr)

#define getIMR() \
		w5500_r_byte(IMR)

#define setSIR(sir) \
		w5500_w_byte(SIR, sir)

#define getSIR() \
		w5500_r_byte(SIR)

#define setSIMR(simr) \
		w5500_w_byte(SIMR, simr)

#define getSIMR() \
		w5500_r_byte(SIMR)

#define setRTR(rtr)   {\
		w5500_w_byte(_RTR,   (mlu8)(rtr >> 8)); \
		w5500_w_byte(_W5500_OFFSET_INC(_RTR,1), (mlu8) rtr); \
	}

#define getRTR() \
		((w5500_r_byte(RTR) << 8) + w5500_r_byte(_W5500_OFFSET_INC(_RTR,1)))

#define setRCR(rcr) \
		w5500_w_byte(RCR, rcr)

#define getRCR() \
		w5500_r_byte(RCR)

#define setPTIMER(ptimer) \
		w5500_w_byte(PTIMER, ptimer)

#define getPTIMER() \
		w5500_r_byte(PTIMER)

#define setPMAGIC(pmagic) \
		w5500_w_byte(PMAGIC, pmagic)

#define getPMAGIC() \
		w5500_r_byte(PMAGIC)

#define setPHAR(phar) \
		w5500_w_buf(PHAR, phar, 6)

#define getPHAR(phar) \
		w5500_r_buf(PHAR, phar, 6)

#define setPSID(psid)  {\
		w5500_w_byte(PSID,   (mlu8)(psid >> 8)); \
		w5500_w_byte(_W5500_OFFSET_INC(PSID,1), (mlu8) psid); \
	}

#define getPSID() \
		((w5500_r_byte(PSID) << 8) + w5500_r_byte(_W5500_OFFSET_INC(PSID,1)))

#define setPMRU(pmru) { \
		w5500_w_byte(PMRU,   (mlu8)(pmru>>8)); \
		w5500_w_byte(_W5500_OFFSET_INC(PMRU,1), (mlu8) pmru); \
	}

#define getPMRU() \
		((w5500_r_byte(PMRU) << 8) + w5500_r_byte(_W5500_OFFSET_INC(PMRU,1)))

#define getUIPR(uipr) \
		w5500_r_buf(UIPR,uipr,6)

#define getUPORTR() \
	((w5500_r_byte(UPORTR) << 8) + w5500_r_byte(_W5500_OFFSET_INC(UPORTR,1)))

#define setPHYCFGR(phycfgr) \
		w5500_w_byte(PHYCFGR, phycfgr)

#define getPHYCFGR() \
		w5500_r_byte(PHYCFGR)

#define getVERSIONR() \
		w5500_r_byte(VERSIONR)

#define setSn_MR(sn, mr) \
		w5500_w_byte(Sn_MR(sn),mr)

#define getSn_MR(sn) \
	w5500_r_byte(Sn_MR(sn))

#define setSn_CR(sn, cr) \
		w5500_w_byte(Sn_CR(sn), cr)

#define getSn_CR(sn) \
		w5500_r_byte(Sn_CR(sn))

#define setSn_IR(sn, ir) \
		w5500_w_byte(Sn_IR(sn), (ir & 0x1F))

#define getSn_IR(sn) \
		(w5500_r_byte(Sn_IR(sn)) & 0x1F)

#define setSn_IMR(sn, imr) \
		w5500_w_byte(Sn_IMR(sn), (imr & 0x1F))

#define getSn_IMR(sn) \
		(w5500_r_byte(Sn_IMR(sn)) & 0x1F)

#define getSn_SR(sn) \
		w5500_r_byte(Sn_SR(sn))

#define setSn_PORT(sn, port)  { \
		w5500_w_byte(Sn_PORT(sn),   (mlu8)(port >> 8)); \
		w5500_w_byte(_W5500_OFFSET_INC(Sn_PORT(sn),1), (mlu8) port); \
	}

#define getSn_PORT(sn) \
		((w5500_r_byte(Sn_PORT(sn)) << 8) + w5500_r_byte(_W5500_OFFSET_INC(Sn_PORT(sn),1)))

#define setSn_DHAR(sn, dhar) \
		w5500_w_buf(Sn_DHAR(sn), dhar, 6)

#define getSn_DHAR(sn, dhar) \
		w5500_r_buf(Sn_DHAR(sn), dhar, 6)

#define setSn_DIPR(sn, dipr) \
		w5500_w_buf(Sn_DIPR(sn), dipr, 4)

#define getSn_DIPR(sn, dipr) \
		w5500_r_buf(Sn_DIPR(sn), dipr, 4)

#define setSn_DPORT(sn, dport) { \
		w5500_w_byte(Sn_DPORT(sn),   (mlu8) (dport>>8)); \
		w5500_w_byte(_W5500_OFFSET_INC(Sn_DPORT(sn),1), (mlu8)  dport); \
	}

#define getSn_DPORT(sn) \
		((w5500_r_byte(Sn_DPORT(sn)) << 8) + w5500_r_byte(_W5500_OFFSET_INC(Sn_DPORT(sn),1)))

#define setSn_MSSR(sn, mss) { \
		w5500_w_byte(Sn_MSSR(sn),   (mlu8)(mss>>8)); \
		w5500_w_byte(_W5500_OFFSET_INC(Sn_MSSR(sn),1), (mlu8) mss); \
	}

#define getSn_MSSR(sn) \
		((w5500_r_byte(Sn_MSSR(sn)) << 8) + w5500_r_byte(_W5500_OFFSET_INC(Sn_MSSR(sn),1)))

#define setSn_TOS(sn, tos) \
		w5500_w_byte(Sn_TOS(sn), tos)

#define getSn_TOS(sn) \
		w5500_r_byte(Sn_TOS(sn))

#define setSn_TTL(sn, ttl) \
		w5500_w_byte(Sn_TTL(sn), ttl)

#define getSn_TTL(sn) \
		w5500_r_byte(Sn_TTL(sn))
#define setSn_RXBUF_SIZE(sn, rxbufsize) \
		w5500_w_byte(Sn_RXBUF_SIZE(sn),rxbufsize)

#define getSn_RXBUF_SIZE(sn) \
		w5500_r_byte(Sn_RXBUF_SIZE(sn))

#define setSn_TXBUF_SIZE(sn, txbufsize) \
		w5500_w_byte(Sn_TXBUF_SIZE(sn), txbufsize)

#define getSn_TXBUF_SIZE(sn) \
		w5500_r_byte(Sn_TXBUF_SIZE(sn))

//uint16_t getSn_TX_FSR(mlu8 sn);

#define getSn_TX_RD(sn) \
		((w5500_r_byte(Sn_TX_RD(sn)) << 8) + w5500_r_byte(_W5500_OFFSET_INC(Sn_TX_RD(sn),1)))

#define setSn_TX_WR(sn, txwr) { \
		w5500_w_byte(Sn_TX_WR(sn),   (mlu8)(txwr>>8)); \
		w5500_w_byte(_W5500_OFFSET_INC(Sn_TX_WR(sn),1), (mlu8) txwr); \
		}

#define getSn_TX_WR(sn) \
		((w5500_r_byte(Sn_TX_WR(sn)) << 8) + w5500_r_byte(_W5500_OFFSET_INC(Sn_TX_WR(sn),1)))

//uint16_t getSn_RX_RSR(mlu8 sn);

#define setSn_RX_RD(sn, rxrd) { \
		w5500_w_byte(Sn_RX_RD(sn),   (mlu8)(rxrd>>8)); \
		w5500_w_byte(_W5500_OFFSET_INC(Sn_RX_RD(sn),1), (mlu8) rxrd); \
	}

#define getSn_RX_RD(sn) \
		((w5500_r_byte(Sn_RX_RD(sn)) << 8) + w5500_r_byte(_W5500_OFFSET_INC(Sn_RX_RD(sn),1)))

#define getSn_RX_WR(sn) \
		((w5500_r_byte(Sn_RX_WR(sn)) << 8) + w5500_r_byte(_W5500_OFFSET_INC(Sn_RX_WR(sn),1)))

#define setSn_FRAG(sn, frag) { \
		w5500_w_byte(Sn_FRAG(sn),  (mlu8)(frag >>8)); \
		w5500_w_byte(_W5500_OFFSET_INC(Sn_FRAG(sn),1), (mlu8) frag); \
	}

#define getSn_FRAG(sn) \
		((w5500_r_byte(Sn_FRAG(sn)) << 8) + w5500_r_byte(_W5500_OFFSET_INC(Sn_FRAG(sn),1)))

#define setSn_KPALVTR(sn, kpalvt) \
		w5500_w_byte(Sn_KPALVTR(sn), kpalvt)

#define getSn_KPALVTR(sn) \
		w5500_r_byte(Sn_KPALVTR(sn))

#define getSn_RxMAX(sn) \
		(getSn_RXBUF_SIZE(sn) << 10)

//uint16_t getSn_TxMAX(mlu8 sn);
#define getSn_TxMAX(sn) \
		(getSn_TXBUF_SIZE(sn) << 10)


//----------------------------------------------------------------------------
// export func
//----------------------------------------------------------------------------

void w5500_init(mlu8 *ip, mlu8 *mask, mlu8 *gateway, mlu8 *mac);
Netcardptr w5500_network_card(void);

//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif
#endif

