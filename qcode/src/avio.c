/*************************************************************************
	> File Name: 
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/shm.h>
#include <zbar.h>

#include "sample_comm.h"



	


zbar_image_scanner_t *scanner = NULL;



VIDEO_NORM_E	gs_enNorm		= VIDEO_ENCODING_MODE_NTSC; //VIDEO_ENCODING_MODE_PAL;  //VIDEO_ENCODING_MODE_AUTO

PIC_SIZE_E		gs_enSize[2]	= {PIC_HD720, PIC_VGA};//{PIC_VGA}; //{PIC_HD720};

SAMPLE_VI_CONFIG_S stViConfig = {0};
VPSS_GRP VpssGrp = 0;
VPSS_CHN VpssChn = 1;
VENC_CHN VencChn = 1;	
VPSS_GRP_ATTR_S stVpssGrpAttr;
VPSS_CHN_ATTR_S stVpssChnAttr;
VPSS_CHN_MODE_S stVpssChnMode;

HI_U32 g_u32BlkCnt = 4;

void HI_AVIO_SignalHandle(int signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {	

		
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}


/*********************************************************************/
HI_S32 SAMPLE_PROC_VI_StartChn(VI_CHN ViChn, RECT_S *pstCapRect, SIZE_S *pstTarSize, SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_S32 s32Ret;
    VI_CHN_ATTR_S stChnAttr;
    ROTATE_E enRotate = ROTATE_NONE;
    SAMPLE_VI_CHN_SET_E enViChnSet = VI_CHN_SET_NORMAL;

    if(pstViConfig)
    {
        enViChnSet = pstViConfig->enViChnSet;
        enRotate = pstViConfig->enRotate;
    }

    /* step  5: config & start vicap dev */
    memcpy(&stChnAttr.stCapRect, pstCapRect, sizeof(RECT_S));
    stChnAttr.enCapSel = VI_CAPSEL_BOTH;
    /* to show scale. this is a sample only, we want to show dist_size = D1 only */
    stChnAttr.stDestSize.u32Width = pstTarSize->u32Width;
    stChnAttr.stDestSize.u32Height = pstTarSize->u32Height;
    stChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;   /* sp420 or sp422 */

    stChnAttr.bMirror = HI_FALSE;
    stChnAttr.bFlip = HI_FALSE;

    switch(enViChnSet)
    {
        case VI_CHN_SET_MIRROR:
            stChnAttr.bMirror = HI_TRUE;
            break;

        case VI_CHN_SET_FLIP:
            stChnAttr.bFlip = HI_TRUE;
            break;
            
        case VI_CHN_SET_FLIP_MIRROR:
            stChnAttr.bMirror = HI_TRUE;
            stChnAttr.bFlip = HI_TRUE;
            break;
            
        default:
            break;
    }

    stChnAttr.s32SrcFrameRate = -1;
    stChnAttr.s32DstFrameRate = -1;
    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
	

    s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    if(ROTATE_NONE != enRotate)
    {
        s32Ret = HI_MPI_VI_SetRotate(ViChn, enRotate);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VI_SetRotate failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    s32Ret = HI_MPI_VI_EnableChn(ViChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }


#if 1
	VI_CHN ViExtChn = VIU_EXT_CHN_START;
	VI_EXT_CHN_ATTR_S stExtChnAttr;
	CROP_INFO_S stExtChnCrop;
	/* First enable vi device and vi chn */
	/* Init ext-chn attr */
	stExtChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stExtChnAttr.s32BindChn = 0;
	stExtChnAttr.s32DstFrameRate = -1;
	stExtChnAttr.s32SrcFrameRate = -1;
	
	stExtChnAttr.stDestSize.u32Width = 320;
	stExtChnAttr.stDestSize.u32Height = 240;
	/* Set attribute for vi ext-chn */
	HI_MPI_VI_SetExtChnAttr(ViExtChn, &stExtChnAttr);
	if (HI_SUCCESS != s32Ret)
	{
		printf("Set vi ext-chn attr err:0x%x\n", s32Ret);
		return s32Ret;
	}
	/* Get attribute for vi ext-chn */
	s32Ret = HI_MPI_VI_GetExtChnAttr (ViExtChn, & stExtChnAttr);
	if (HI_SUCCESS != s32Ret)
	{
		printf("Get vi ext-chn attr err:0x%x\n", s32Ret);
		return s32Ret;
	}
#if 1	

	stExtChnCrop.bEnable = HI_TRUE;
	stExtChnCrop.stRect.s32X = 320;
	stExtChnCrop.stRect.s32Y = 160;
	stExtChnCrop.stRect.u32Width = 640;
	stExtChnCrop.stRect.u32Height = 480;
	
	/* Set crop attribute for vi ext-chn */
	HI_MPI_VI_SetExtChnCrop(ViExtChn, &stExtChnCrop);
	if (HI_SUCCESS != s32Ret)
	{
		printf("Set vi ext-chn crop attr err:0x%x\n", s32Ret);
		return s32Ret;
	}

#endif
    s32Ret = HI_MPI_VI_EnableChn(ViExtChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
#endif
    return HI_SUCCESS;
}



HI_S32 SAMPLE_PROC_VI_StartVi(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_S32 i, s32Ret = HI_SUCCESS;
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_U32 u32DevNum = 1;
    HI_U32 u32ChnNum = 1;
    SIZE_S stTargetSize;
    RECT_S stCapRect;
    SAMPLE_VI_MODE_E enViMode;

    if(!pstViConfig)
    {
        SAMPLE_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }
    enViMode = pstViConfig->enViMode;

    /******************************************
     step 1: mipi configure
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_StartMIPI(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("%s: MIPI init failed!\n", __FUNCTION__);
        return HI_FAILURE;
    }     

    /******************************************
     step 2: configure sensor and ISP (include WDR mode).
     note: you can jump over this step, if you do not use Hi3516A interal isp. 
    ******************************************/
    s32Ret = SAMPLE_COMM_ISP_Init(pstViConfig->enWDRMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("%s: Sensor init failed!\n", __FUNCTION__);
        return HI_FAILURE;
    }

    /******************************************
     step 3: run isp thread 
     note: you can jump over this step, if you do not use Hi3516A interal isp.
    ******************************************/
    s32Ret = SAMPLE_COMM_ISP_Run();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("%s: ISP init failed!\n", __FUNCTION__);
	    /* disable videv */
        return HI_FAILURE;
    }	
	
    /******************************************************
     step 4 : config & start vicap dev
    ******************************************************/
    for (i = 0; i < u32DevNum; i++)
    {
        ViDev = i;		
        s32Ret = SAMPLE_COMM_VI_StartDev(ViDev, enViMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("%s: start vi dev[%d] failed!\n", __FUNCTION__, i);
            return HI_FAILURE;
        }
    }	
	
	
	

	
    /******************************************************
    * Step 5: config & start vicap chn (max 1) 
    ******************************************************/	
	ViChn = 0;

	stCapRect.s32X = 0;
	stCapRect.s32Y = 0;
	stCapRect.u32Width = 1280;
	stCapRect.u32Height = 720;	
	stTargetSize.u32Width = stCapRect.u32Width;
	stTargetSize.u32Height = stCapRect.u32Height;	
	
	s32Ret = SAMPLE_PROC_VI_StartChn(ViChn, &stCapRect, &stTargetSize, pstViConfig);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_COMM_ISP_Stop();
		return HI_FAILURE;
	}	
}


/*********************************************************************/

int HI_AVIO_Init(void)
{
	int s32Ret = HI_SUCCESS;
	
    VB_CONF_S stVbConf;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    char c,i;
	
    memset(&stVbConf,0,sizeof(VB_CONF_S));
#if 1	
	VB_CONF_S *xx = NULL;
	if (s32Ret = HI_MPI_VB_GetConf(&stVbConf))
	{
		//系统还未配置
		printf("xxxxxx %#x\n", s32Ret);
		
	}else{
		printf("pool:u32MaxPoolCnt = 0x%x\n", stVbConf.u32MaxPoolCnt);
		for (i = 0; i < stVbConf.u32MaxPoolCnt;++i){
			if (stVbConf.astCommPool[i].u32BlkSize)
				printf("[%d]. size = %d, cnt = %d\n", i, stVbConf.astCommPool[i].u32BlkSize, stVbConf.astCommPool[i].u32BlkCnt);
			else
				break;
		}
	}
	
	//exit(0);
#endif	
	SAMPLE_COMM_VI_GetSizeBySensor(&gs_enSize[0]);
	
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, gs_enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = g_u32BlkCnt + 2;
	
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, gs_enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt = g_u32BlkCnt * 2;	



	
	
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_1080P_CLASSIC_0;
    }
	
	
	VB_POOL VbPool;
	VB_BLK VbBlk;
	HI_U32 u32BlkSize2 = 768;
	HI_U32 u32BlkCnt2 = 15;
	HI_U32 u32Addr;
	/* create a video buffer pool*/
	VbPool = HI_MPI_VB_CreatePool(u32BlkSize2,u32BlkCnt2, "anonymous");
	if ( VB_INVALID_POOLID == VbPool )
	{
		printf("create vb err\n");
		//return HI_FAILURE;
	}	
	
	
	
#if 1	
	if (s32Ret = HI_MPI_VB_GetConf(&stVbConf))
	{
		//系统还未配置
		printf("xxxxxx %#x\n", s32Ret);
		
	}else{
		printf("pool:u32MaxPoolCnt = 0x%x\n", stVbConf.u32MaxPoolCnt);
		for (i = 0; i < stVbConf.u32MaxPoolCnt;++i){
			if (stVbConf.astCommPool[i].u32BlkSize)
				printf("[%d]. size = %d, cnt = %d\n", i, stVbConf.astCommPool[i].u32BlkSize, stVbConf.astCommPool[i].u32BlkCnt);
			else
				break;
		}
	}
	
	//exit(0);
#endif		
	
	
	exit(0);
    stViConfig.enViMode   = SENSOR_TYPE;		
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    stViConfig.enWDRMode  = WDR_MODE_NONE;
    s32Ret = SAMPLE_PROC_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

#if 0	
	stVpssGrpAttr.u32MaxW = 640;
	stVpssGrpAttr.u32MaxH = 480;
	stVpssGrpAttr.bIeEn = HI_FALSE;
	stVpssGrpAttr.bNrEn = HI_TRUE;
	stVpssGrpAttr.bHistEn = HI_FALSE;
	stVpssGrpAttr.bDciEn = HI_FALSE;
	stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
	stVpssGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	
	s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_VENC_1080P_CLASSIC_2;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_VENC_1080P_CLASSIC_3;
	}	
	
	stVpssChnMode.enChnMode      = VPSS_CHN_MODE_USER;
	stVpssChnMode.bDouble        = HI_FALSE;
	stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stVpssChnMode.u32Width       = 640;
	stVpssChnMode.u32Height      = 480;
	stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
	memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
	stVpssChnAttr.s32SrcFrameRate = -1;
	stVpssChnAttr.s32DstFrameRate = -1;
	//s32Ret = HI_MPI_VPSS_SetRotate(VpssGrp, VpssChn,  ROTATE_180);
	
	s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Enable vpss chn failed!\n");
		goto END_VENC_1080P_CLASSIC_4;
	}	
#endif	
 
	return HI_SUCCESS;
    /******************************************
     step 7: exit process
    ******************************************/
END_VENC_1080P_CLASSIC_4:	

	SAMPLE_COMM_VPSS_DisableChn(0, 1);
END_VENC_1080P_CLASSIC_3:       
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_2:   
    SAMPLE_COMM_VPSS_StopGroup(0);
	
END_VENC_1080P_CLASSIC_1:	
    SAMPLE_COMM_VI_StopVi(&stViConfig);

END_VENC_1080P_CLASSIC_0:	
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;  
}

int HI_AVIO_Deinit(void)
{
	
	
	
	return 0;
}


void zbar_qcode(void)
{

    HI_S32 ViFd;
	VI_CHN_STAT_S stStat;
    VIDEO_FRAME_INFO_S stFrame;
	
	HI_U32 phy_addr, size;
	HI_CHAR* pUserPageAddr[2];
	char* pVBufVirt_Y;
	char* pMemContent;
	
    HI_S32 s32Ret;
	int vichnnl = 1;	//扩展通道
	HI_U32 u32OldDepth = -1U;

    if (HI_MPI_VI_SetFrameDepth(vichnnl, 1))
    {
        printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", vichnnl);
        return;
    }

	/******************************************************************/
    scanner = zbar_image_scanner_create();
    zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);		

	char *raw = (char *)malloc(320*240);
	int width = 320, height = 240, h;
	zbar_image_t *image= NULL;
	image = zbar_image_create();
	zbar_image_set_format(image, *(int*)"Y800");
	zbar_image_set_size(image, width, height);
	
	printf("stream start\r\n");
    while (1)
    {
		usleep(90000);
		s32Ret = HI_MPI_VI_GetFrame(vichnnl, &stFrame, 2000);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("HI_MPI_VI_GetFrame failed with %#x!\n",s32Ret);
			return;
		}

		size = stFrame.stVFrame.u32Stride[0] * stFrame.stVFrame.u32Height * 3 >> 1;

		phy_addr = stFrame.stVFrame.u32PhyAddr[0];
		pUserPageAddr[0] = (HI_CHAR*) HI_MPI_SYS_Mmap(phy_addr, size);
		pVBufVirt_Y = pUserPageAddr[0];

		for (h = 0; h < stFrame.stVFrame.u32Height; h++)
		{
			pMemContent = pVBufVirt_Y + h * stFrame.stVFrame.u32Stride[0];
			memcpy(raw + h * stFrame.stVFrame.u32Width, pMemContent, stFrame.stVFrame.u32Width);				
		}

		//printf("get w  = %d, h = %d\n", stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height);

		zbar_image_set_data(image, raw, stFrame.stVFrame.u32Width * stFrame.stVFrame.u32Height, NULL);
		int n = zbar_scan_image(scanner, image);
		//printf("zbar scan image n = %d\n", n);
		if (n)
		{
			const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
			for(; symbol; symbol = zbar_symbol_next(symbol)) 
			{
				zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
				const char *data = zbar_symbol_get_data(symbol);

				printf("decoded %s symbol \"%s\"\n", zbar_get_symbol_name(typ), data);

			}
		}

		s32Ret = HI_MPI_VI_ReleaseFrame(vichnnl, &stFrame);
    }
	

	printf("stream stop...\r\n");
    return;	
}

