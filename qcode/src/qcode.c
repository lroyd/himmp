/*************************************************************************
	> File Name: 
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 ************************************************************************/
#include <zbar.h>

#include "qcode.h"

typedef struct _tagQcode{
	zbar_image_scanner_t	*in_pScanner;
	zbar_image_t			*in_pImage;
	int						m_u32Width;
	int						m_u32Height
}T_Qcode;

int QCODE_Create(T_PoolInfo *_pPool, int _u32Width, int _u32Height, PV_Qcode *_pOut)
{
	T_Qcode *pQcode = POOL_ALLOC_T(_pPool, T_Qcode);//
	if (!pQcode) return -1;
	
    pQcode->in_pScanner = zbar_image_scanner_create();
	if (!pQcode->in_pScanner) return -1;
	
	pQcode->in_pImage = zbar_image_create();
	if (!pQcode->in_pImage) return -1;
	
    zbar_image_scanner_set_config(pQcode->in_pScanner, 0, ZBAR_CFG_ENABLE, 1);	
	zbar_image_set_format(pQcode->in_pImage, *(int*)"Y800");
	zbar_image_set_size(pQcode->in_pImage, _u32Width, _u32Height);	
	pQcode->m_u32Width		= _u32Width;
	pQcode->m_u32Height		= _u32Height;
	
	if (_pOut) *_pOut = pQcode;
	return 0;
}
	

void QCODE_Destroy(PV_Qcode _pQcode)
{
	T_Qcode *pQcode = (T_Qcode *)_pQcode;
	
	if (pQcode->in_pScanner){
		
	}
	
	if (pQcode->in_pImage){
		
	}	
	
	//pQcode由外层释放
}


char *QCODE_GrayInput(PV_Qcode _pQcode, char *_pData, char *_pOBuffer, int *_pOLen)
{
	T_Qcode *pQcode = (T_Qcode *)_pQcode;
	if (!pQcode) return NULL;
	zbar_image_set_data(pQcode->in_pImage, _pData, pQcode->m_u32Width * pQcode->m_u32Height, NULL);
	if (zbar_scan_image(pQcode->in_pScanner, pQcode->in_pImage)){
		const zbar_symbol_t *symbol = zbar_image_first_symbol(pQcode->in_pImage);
		for(; symbol; symbol = zbar_symbol_next(symbol)) {
			zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
			const char *data = zbar_symbol_get_data(symbol);
			if (_pOLen) *_pOLen = strlen(data);
			if (_pOBuffer) memcpy(_pOBuffer, data, strlen(data));
			//printf("decoded %s symbol \"%s\"\n", zbar_get_symbol_name(typ), data);
			return _pOBuffer;
		}
	}	
	return NULL;
}





