
//----------------------------------------------------------------------------
// 文件条件编译
//----------------------------------------------------------------------------
#include"u_app_config.h"

#if TRAY_ENABLE

//----------------------------------------------------------------------------
//include
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "basic.h"
#include "define.h"
#include "enum.h"
#include "type.h"
#include "cause.h"
#include "func.h"
#include "timer.h"
#include "log.h"
#include "entry.h"
#include "flow.h"
#include "box.h"
#include "host.h"
#include "channel.h"
#include "protect.h"


void chnEnterIdle(Channel *chn)
{
    Cell *cell;
    Cell *cellCri;

    chn->chnProtBuf.idleTimeStampSec = sysTimeSecGet();
    chn->chnProtBuf.idleVolBaseValid = False;
    chn->chnProtBuf.flowIdleVolIntvlRiseBaseValid = False;

    if (NULL != chn->series)
    {
        for (cell=chn->series->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            cell->chnProtBuf.idleTimeStampSec = sysTimeSecGet();
            cell->chnProtBuf.idleVolBaseValid = False;
            cell->chnProtBuf.flowIdleVolIntvlRiseBaseValid = False;
        }
    }
    return;
}

void chnStepSwPre(Channel *chn)
{
    Cell *cell;
    Cell *cellCri;
    ChnProtBuf *chnProtBuf;

    chn->capCtnu = 0;
    chnProtBuf = &chn->chnProtBuf;
    chnProtBuf->idleVolFluctSmlCnt = 0;
    chnProtBuf->quietVolDownBaseValid = False;
    chnProtBuf->cccVolDownAbnmValid = False;
    chnProtBuf->ccVolIntvlFluctBaseValid = False;
    chnProtBuf->cccVolRiseCtnuCnt = False;
    chnProtBuf->cccVolDownCtnuCnt = False;
    chnProtBuf->ccdVolRiseAbnmValid = False;
    chnProtBuf->cvcCurRiseAbnmValid = False;
    chnProtBuf->flowIdleVolIntvlRiseBaseValid = False;
    chnProtBuf->flowCcdcVolIntvlFluctBaseValid = False;
    chnProtBuf->idleVolBaseValid = False;

    if (NULL != chn->series)
    {
        for (cell=chn->series->cell,cellCri=cell+chn->chnCellAmt; cell<cellCri; cell++)
        {
            chnProtBuf = &cell->chnProtBuf;
            chnProtBuf->idleVolFluctSmlCnt = 0;
            chnProtBuf->quietVolDownBaseValid = False;
            chnProtBuf->cccVolDownAbnmValid = False;
            chnProtBuf->ccVolIntvlFluctBaseValid = False;
            chnProtBuf->cccVolRiseCtnuCnt = False;
            chnProtBuf->cccVolDownCtnuCnt = False;
            chnProtBuf->ccdVolRiseAbnmValid = False;
            chnProtBuf->cvcCurRiseAbnmValid = False;
            chnProtBuf->flowIdleVolIntvlRiseBaseValid = False;
            chnProtBuf->flowCcdcVolIntvlFluctBaseValid = False;
            chnProtBuf->idleVolBaseValid = False;
        }
    }
    return;
}

/*是否刚好下位机正常截止*/
b8 chnStepBeJustNmlEnd(Channel *chn)
{
    if (gDevMgr->isTraySmpl)
    {
        Tray *tray;
        TraySmpl *traySmpl;
        TrayChnSmpl *trayChnSmpl;
        
        tray = chn->box->tray;
        traySmpl = ((TraySmplRcd *)tray->smplMgr.smplBufAddr)->traySmpl;
        trayChnSmpl = &traySmpl->chnSmpl[chn->chnIdxInTray];
        if (ChnUpStateRun==trayChnSmpl->chnUpState
            && trayChnSmpl->causeCode>0 && trayChnSmpl->causeCode<6)
        {
            return True;
        }
    }

    return False;
}
//----------------------------------------------------------------------------
// end of file
//----------------------------------------------------------------------------
#endif// 文件条件编译


