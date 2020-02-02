/*************************************************************************
	> File Name: 
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 ************************************************************************/
#ifndef __ZXY_QCODE_H__
#define __ZXY_QCODE_H__

#include "pool.h"

typedef void *(PV_Qcode);

int QCODE_Create(T_PoolInfo *_pPool, int _u32Width, int _u32Height, PV_Qcode *_pOut);

void QCODE_Destroy(PV_Qcode);

char *QCODE_GrayInput(PV_Qcode _pQcode, char *_pData, char *_pOBuffer, int *_pOLen);





#endif


