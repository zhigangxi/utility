#ifndef _HUO_DONG_H_
#define _HUO_DONG_H_
#include "self_typedef.h"

const int MIAO_SHA_TIME = 23;
const int MING_GUAI_TIME = 21;
const int WU_GUI_BEGIN_TIME = 11;
//马拉松时间
const int MLS_TIME = 18;

void HDTimer();
void UseYuGan(int id);
void UseShiTou(int id);
bool InSaiPao();
int UserTouZhu(uint32 roleId,int first,int second,int num);
void SaveTouZhu();
void ReadTouZhu();

#endif
