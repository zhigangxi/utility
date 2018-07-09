#include "bangpai.h"
#include "net_msg.h"
#include "database.h"
#include "user.h"
#include "scene_manager.h"
#include "singleton.h"
#include <algorithm>
#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
 
const int sBangPaiZiJin[] = {20,40,80,150,250};
const int sWeiHuZiJin[] = {5,7,10,15,25};
struct STest
{
    int i;
    int j;
};
void CBangPai::InitZhongZhi(bool query)
{
    STest t;
    STimer ti;
    ti:help ‘tags’.
    const SPoint caiDiPoint[] = 
    {
        {1,10} ,{3,10} 	,{5,10} ,{7,10} ,	{9,10}, {11,10},
        {1,12} ,{3,12} 	,{5,12} ,{7,12} ,	{9,12}, {11,12},
        {1,14} ,{3,14} 	,{5,14} ,{7,14} ,	{9,14}, {11,14},
        {1,16} ,{3,16} 	,{5,16} ,{7,16} ,	{9,16}, {11,16},
        {1,18} ,{3,18} 	,{5,18} ,{7,18} ,	{9,18}, {11,18},
        {1,20} ,{3,20} 	,{5,20} ,{7,20} ,	{9,20}, {11,20},
        {1,22} ,{3,22} 	,{5,22} ,{7,22} ,	{9,22}, {11,22},
        {1,24} ,{3,24} 	,{5,24} ,{7,24} ,	{9,24}, {11,24},
        {18,10}, {20,10}, {22,10}, {24,10}, {26,10}, {28,10},
        {18,12}, {20,12}, {22,12}, {24,12}, {26,12}, {28,12},
        {18,14}, {20,14}, {22,14}, {24,14}, {26,14}, {28,14},
        {18,16}, {20,16}, {22,16}, {24,16}, {26,16}, {28,16},
        {18,18}, {20,18}, {22,18}, {24,18}, {26,18}, {28,18},
        {18,20}, {20,20}, {22,20}, {24,20}, {26,20}, {28,20},
        {18,22}, {20,22}, {22,22}, {24,22}, {26,22}, {28,22},
        {18,24}, {20,24}, {22,24}, {24,24}, {26,24}, {28,24}
    };
    /*
    1级帮：20人  50格田
    2级帮：40人，60格田
    3级帮：60人，70格田
    4级帮：80人，80格田
    5级帮：100人，96格田
    */
	/*
     *int pointSize = sizeof(caiDiPoint)/sizeof(caiDiPoint[0]);
	 */
	/*
     *m_zhongZhi.resize(pointSize);
	 */
    m_thiefList.resize(pointSize);
    //SZhongZhi zz;
        
	CGetDbConnect getDb;
	CDatabaseSql *pDb = getDb.GetDbConnect();
	getDb.
	boost::mutex::scoped_lock lk(m_mutex);
	for(int i = 0; i < pointSize; i++)
	{
		m_zhongZhi[i].x = caiDiPoint[i].x;
		m_zhongZhi[i].y = caiDiPoint[i].y;
		if(!query)
			continue;
		if(pDb != NULL)
		{
			char sql[256];
			sprintf(sql,"select item_id,state,role_id,time,grow_time,gain from bang_zz "\
				"where bang_id=%u and x=%d and y=%d",m_id,m_zhongZhi[i].x,m_zhongZhi[i].y);
			char **row;
			if(pDb->Query(sql) && ((row = pDb->GetRow()) != NULL))
			{
				m_zhongZhi[i].itemId = atoi(row[0]);
				m_zhongZhi[i].state = atoi(row[1]);
				m_zhongZhi[i].roleId = atoi(row[2]);
				m_zhongZhi[i].time = atoi(row[3]);
				m_zhongZhi[i].growTime = atoi(row[4]);
				m_zhongZhi[i].gain = atoi(row[5]);
			}
		}
	}
}
int 
void CBangPai::SaveZhongZhi(CDatabaseSql *pDb)
{
	if(pDb == NULL)
		return;
	char sql[256];
	list<int> intList;
	string str;
	intList.
	for(uint32 i = 0; i < m_zhongZhi.size(); i++)
	{
		if(m_zhongZhi[i].itemId != 0)
		{
			sprintf(sql,"INSERT INTO bang_zz (bang_id,item_id,state,x,y,role_id,time,grow_time,gain) "\
				"VALUES (%d,%d,%d,%d,%d,%d,%lu,%d,%d)",m_id,m_zhongZhi[i].itemId,m_zhongZhi[i].state
				,m_zhongZhi[i].x,m_zhongZhi[i].y,m_zhongZhi[i].roleId,m_zhongZhi[i].time
				,m_zhongZhi[i].growTime,m_zhongZhi[i].gain);
			pDb->Query(sql);
		}
	}
	Save();
}

void CBangPai::GetJieMeng(list<uint32> &jimengList)
{
    CBangPaiManager &bangPaiMgr = SingletonCBangPaiManager::instance();
    boost::mutex::scoped_lock lk(m_mutex);
    //jimengList = m_jieMengList;
    for(list<uint32>::iterator i = m_jieMengList.begin(); i != m_jieMengList.end(); i++)
    {
        if(bangPaiMgr.FindBangPai(*i) != NULL)
            jimengList.push_back(*i);
    }
}

void CBangPai::SetJieMing(char *row)
{
    if(row == NULL)
        return;
    char *p[JIE_MENG_MAX_NUM];
    int num = SplitLine(p,JIE_MENG_MAX_NUM,row);
    for(int i = 0; i < num; i++)
    {
        m_jieMengList.push_back(atoi(p[i]));
    }
}

bool CBangPai::AddJieMeng(uint32 bId)
{
    boost::mutex::scoped_lock lk(m_mutex);
    if(m_jieMengList.size() < JIE_MENG_MAX_NUM)
    {
        m_jieMengList.push_back(bId);
        return true;
    }
    return false;
}

void CBangPai::DelJieMeng(uint32 bId)
{
    m_delJMTime = GetSysTime();
    boost::mutex::scoped_lock lk(m_mutex);
    m_jieMengList.remove(bId);
}

bool CBangPai::JieMeng(CBangPai *pBangPai)
{
    list<uint32> jiemeng1;
    list<uint32> jiemeng2;
    GetJieMeng(jiemeng1);
    pBangPai->GetJieMeng(jiemeng2);
    if(find(jiemeng1.begin(),jiemeng1.end(),pBangPai->GetId()) != jiemeng1.end())
        return true;
    if(find(jiemeng2.begin(),jiemeng2.end(),m_id) != jiemeng2.end())
        return true;
    return false;
}

bool CBangPai::ZhongZhi(SZhongZhi &zhongZhi,CUser *pUser,uint16 itemId)
{
    boost::mutex::scoped_lock lk(m_mutex);
    uint32 pointSize = 0;
    if(m_level == 1)
        pointSize = 50;
    else if(m_level == 2)
        pointSize = 60;
    else if(m_level == 3)
        pointSize = 70;
    else if(m_level == 4)
        pointSize = 80;
    else if(m_level >= 5)
        pointSize = m_zhongZhi.size();//96;//sizeof(caiDiPoint)/sizeof(caiDiPoint[0]);
    uint32 zhongZhiPos = pointSize;
    int num = 0;
    for(uint32 i = 0; i < pointSize; i++)
    {
        if((zhongZhiPos == pointSize) && (m_zhongZhi[i].itemId == 0))
        {
            zhongZhiPos = i;
        }
        if((m_zhongZhi[i].itemId != 0) && (m_zhongZhi[i].roleId == pUser->GetRoleId()))
        {
            if(++num >= 4)
                return false;
        }
    }
    if(zhongZhiPos == pointSize)
        return false;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(itemId);
    if((pItem->type == EITMissionCanSave)
        && (pItem->addSudu > 0))
    {
        m_zhongZhi[zhongZhiPos].itemId = itemId;
        m_zhongZhi[zhongZhiPos].roleName = pUser->GetName();
        m_zhongZhi[zhongZhiPos].state = 0;//状态0正常，1成熟，2加速，3破坏
        m_zhongZhi[zhongZhiPos].roleId = pUser->GetRoleId();//种植角色id
        m_zhongZhi[zhongZhiPos].time = GetSysTime();//种植时间
        m_zhongZhi[zhongZhiPos].growTime = 0;
        m_zhongZhi[zhongZhiPos].gain = 10;
        m_thiefList[zhongZhiPos].clear();
        zhongZhi = m_zhongZhi[zhongZhiPos];
        return true;
    }
    else
    {
        return false;
    }
    return false;
}

void CBangPai::MakeHistory(CNetMessage &msg)
{
    boost::mutex::scoped_lock lk(m_mutex);
    msg<<(uint8)m_histroy.size();
    for(list<string>::iterator i = m_histroy.begin(); i != m_histroy.end(); i++)
    {
        msg<<*i;
    }
}

bool CBangPai::PoHuai(CUser *pUser,CBangPai *pBangPai,CNetMessage &msg)
{
    if(pUser->GetBangPai() == m_id)
    {
        msg<<PRO_ERROR<<"本帮资源无法破坏";
        return false;
    }
    uint8 x,y;
    pUser->GetFacePos(x,y);
    boost::mutex::scoped_lock lk(m_mutex);
    for(uint32 i = 0; i < m_zhongZhi.size(); i++)
    {
        if((m_zhongZhi[i].x == x) && (m_zhongZhi[i].y == y))
        {
            if((m_zhongZhi[i].state & EZZSPoHuai) != 0)
            {
                msg<<PRO_ERROR<<"已经被破坏过";
                return false;
            }
            if((m_zhongZhi[i].state & EZZSShuiTuLiuShi) != 0)
            {
                m_zhongZhi[i].state = EZZSPoHuai|EZZSShuiTuLiuShi;
            }
            else if(m_zhongZhi[i].state != EZZChengShu)
            {
                m_zhongZhi[i].state = EZZSPoHuai;
            }
            else
            {
                msg<<PRO_ERROR<<"已成熟不能破坏";
                return false;
            }
            char buf[256];
            snprintf(buf,256,"%s帮破坏本帮%s的资源",pBangPai->GetName().c_str(),
                m_zhongZhi[i].roleName.c_str());
            m_histroy.push_front(buf);
            if(m_histroy.size() > 20)
            {
                m_histroy.pop_back();
            }
            msg<<PRO_SUCCESS<<"破坏成功,降低作物成长速度";
            return true;
        }
    }
    msg<<PRO_ERROR<<"无法破坏";
    return false;
}
//op=1 加速,op=2 破坏,op=3 修复
bool CBangPai::DoFarm(int op,uint32 ind,SZhongZhi &zz)
{
    boost::mutex::scoped_lock lk(m_mutex);
    if(ind >= m_zhongZhi.size())
        return false;
    switch(op)
    {
    case 1:
        if(m_zhongZhi[ind].state != EZZChengShu)
        {
            m_zhongZhi[ind].state = EZZSJiaSu;
            m_zhongZhi[ind].time = GetSysTime();
            zz = m_zhongZhi[ind];
            return true;
        }
        break;
    case 2:
        if((m_zhongZhi[ind].state | EZZSShuiTuLiuShi) != 0)
        {
            m_zhongZhi[ind].state = EZZSPoHuai|EZZSShuiTuLiuShi;
        }
        else if(m_zhongZhi[ind].state != EZZChengShu)
        {
            m_zhongZhi[ind].state = EZZSPoHuai;
        }
        zz = m_zhongZhi[ind];
        return true;
    case 3:
        if((m_zhongZhi[ind].state & (EZZSPoHuai|EZZSShuiTuLiuShi)) != 0)
        {
            m_zhongZhi[ind].state = EZZSNormal;
            zz = m_zhongZhi[ind];
            return true;
        }
        break;
    }
    return false;
}

bool CBangPai::GetZhongZhiInfo(uint32 ind,SZhongZhi &zz)
{
    boost::mutex::scoped_lock lk(m_mutex);
    if(ind >= m_zhongZhi.size())
        return false;
    zz = m_zhongZhi[ind];
    return true;
}

void CBangPai::MakeZZMsg(CNetMessage &msg)
{
    msg.ReWrite();
    msg.SetType(MSG_SERVER_LIST_OBJECT);
    uint16 pos = msg.GetDataLen();
    /*
    +-----+------+-----+-------+------+------+
      | NUM | TYPE | OID | STATE | POSX | POSY |
      +-----+------+-----+-------+------+------+
      |  1  |   1  |  2  |   1   |   1  |   1  |
      +-----+------+-----+-------+------+------+
    */
    boost::mutex::scoped_lock lk(m_mutex);
    uint8 num = 0;
    msg<<num;
    for(uint32 i = 0; i < m_zhongZhi.size(); i++)
    {
        if(m_zhongZhi[i].itemId != 0)
        {
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(m_zhongZhi[i].itemId);
            if(pItem == NULL)
                continue;
            //if(GetSysTime() - m_zhongZhi[i].time > pItem->addSudu*3600)
                //m_zhongZhi[i].state = 1;
            num++;
            msg<<(uint8)1<<(uint16)m_zhongZhi[i].itemId
                <<m_zhongZhi[i].state//状态0正常，1成熟，2加速，3破坏
                <<m_zhongZhi[i].x
                <<m_zhongZhi[i].y;
        }
    }
    msg.WriteData(pos,&num,1);
}

void CBangPai::MakeUserRes(uint32 roleId,CNetMessage &msg)
{
    uint16 pos = msg.GetDataLen();
    uint8 num = 0;
    msg<<num;
    boost::mutex::scoped_lock lk(m_mutex);
    for(uint32 i = 0; i < m_zhongZhi.size(); i++)
    {
        if((m_zhongZhi[i].itemId != 0) && (m_zhongZhi[i].roleId == roleId))
        {
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(m_zhongZhi[i].itemId);
            if(pItem == NULL)
                continue;
            //if(GetSysTime() - m_zhongZhi[i].time > pItem->addSudu*3600)
                //m_zhongZhi[i].state = 1;
            num++;
            msg<<(uint8)i<<(uint16)m_zhongZhi[i].itemId
                <<m_zhongZhi[i].state;//状态0正常，1成熟，2加速，3破坏;
            int leftTime = pItem->addSudu*3600 - m_zhongZhi[i].growTime;//成熟剩余时间
            if(m_zhongZhi[i].state == EZZSJiaSu)
            {
                leftTime = leftTime*100/120;
                msg<<(uint8)120;
            }
            else if((m_zhongZhi[i].state & (EZZSShuiTuLiuShi|EZZSPoHuai)) != 0)
            {
                uint8 beiLv = 100;
                if((m_zhongZhi[i].state & EZZSPoHuai) != 0)
                    beiLv -= 25;
                if((m_zhongZhi[i].state & EZZSShuiTuLiuShi) != 0)
                    beiLv -= 25;
                msg<<beiLv;
                leftTime = leftTime*100/beiLv;
            }
            else
            {
                msg<<(uint8)100;
            }
            msg<<leftTime;
        }
    }
    msg.WriteData(pos,&num,1);
}

bool CBangPai::ThieveResource(CUser *pUser,CBangPai *pBangPai,CNetMessage &msg)
{
    if(pUser->GetBangPai() == m_id)
    {
        msg<<PRO_ERROR<<"本帮资源无法偷窃";
        return false;
    }
    uint8 x,y;
    pUser->GetFacePos(x,y);
    boost::mutex::scoped_lock lk(m_mutex);
    
    for(uint32 i = 0; i < m_zhongZhi.size(); i++)
    {
        if((m_zhongZhi[i].x == x) && (m_zhongZhi[i].y == y))
        {
            if(m_zhongZhi[i].state != 1)
            {
                msg<<PRO_ERROR<<"尚未长成，无法偷取";
                return false;
            }
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(m_zhongZhi[i].itemId);
            if(pItem == NULL)
            {
                msg<<PRO_ERROR<<"物品不存在";
                return false;
            }
            if(pBangPai->GetLevel()-pItem->level > 1)
            {
                char buf[128] = {0};
                if(pBangPai->GetLevel() == 1)
                {
                    strcat(buf,"作物等级太低,您的帮派只能偷窃1级、2级的作物");
                }
                else if(pBangPai->GetLevel() == 5)
                {
                    strcat(buf,"作物等级太低,您的帮派只能偷窃4级、5级的作物");
                }
                else
                {
                    snprintf(buf,128,"作物等级太低,您的帮派只能偷窃%d级、%d级、%d级的作物",
                        pBangPai->GetLevel()-1,pBangPai->GetLevel(),pBangPai->GetLevel()+1);
                }
                msg<<PRO_ERROR<<buf;
                return false;
            }
            else if(pItem->level - pBangPai->GetLevel() > 1)
            {
                char buf[128] = {0};
                if(pBangPai->GetLevel() == 1)
                {
                    strcat(buf,"作物等级太高,您的帮派只能偷窃1级、2级的作物");
                }
                else if(pBangPai->GetLevel() == 5)
                {
                    strcat(buf,"作物等级太高,您的帮派只能偷窃4级、5级的作物");
                }
                else
                {
                    snprintf(buf,128,"作物等级太高,您的帮派只能偷窃%d级、%d级、%d级的作物",
                        pBangPai->GetLevel()-1,pBangPai->GetLevel(),pBangPai->GetLevel()+1);
                }
                msg<<PRO_ERROR<<buf;
                return false;
            }
            if(m_zhongZhi[i].gain <= 5)
            {
                msg<<PRO_ERROR<<"偷的差不多了，给别人留点吧";
                return false;
            }
            list<uint32>::iterator iter = m_thiefList[i].begin();
            for(; iter != m_thiefList[i].end(); iter++)
            {
                if(*iter == pUser->GetRoleId())
                {
                    msg<<PRO_ERROR<<"每个作物地只能偷一次";
                    return false;
                }
            }
            m_thiefList[i].push_front(pUser->GetRoleId());
            
            char buf[256];
            snprintf(buf,256,"%s帮盗取本帮%s的资源",pBangPai->GetName().c_str(),
                m_zhongZhi[i].roleName.c_str());
            m_histroy.push_front(buf);
            if(m_histroy.size() > 20)
            {
                m_histroy.pop_back();
            }
            char info[64];
            bool doub = false;
            if(m_zhongZhi[i].gain == 10)
            {
                doub = true;
                m_zhongZhi[i].gain--;
            }
            m_zhongZhi[i].gain--;
            if((m_zhongZhi[i].itemId >= 2056) && (m_zhongZhi[i].itemId <= 2060))
            {
                uint8 beiLv[] = {8,10,12,14,18};
                int res = pBangPai->GetRes1();
                int add = beiLv[m_zhongZhi[i].itemId - 2056];
                if(doub)
                    add *= 2;
                res += add;
                pUser->SetData32(5,pUser->GetData32(5)+add);
                if(res > 10000000)
                    res = 10000000;
                pBangPai->SetRes1(res);
                snprintf(info,64,"偷取百炼精铁%d,获得帮贡%d",add,add);
                pUser->SetData32(9,pUser->GetData32(9)+add);
            }
            else if((m_zhongZhi[i].itemId >= 2061) && (m_zhongZhi[i].itemId <= 2065))
            {
                uint8 beiLv[] = {8,10,13,15,21};
                int res = pBangPai->GetRes2();
                int add = beiLv[m_zhongZhi[i].itemId - 2061];
                if(doub)
                    add *= 2;
                res += add;
                pUser->SetData32(5,pUser->GetData32(5)+add);
                if(res > 10000000)
                    res = 10000000;
                pBangPai->SetRes2(res);
                snprintf(info,64,"偷取无极灵玉%d,获得帮贡%d",add,add);
                pUser->SetData32(9,pUser->GetData32(9)+add);
            }
            else if((m_zhongZhi[i].itemId >= 2066) && (m_zhongZhi[i].itemId <= 2070))
            {
                uint8 beiLv[] = {8,10,14,18,24};
                int res = pBangPai->GetRes3();
                int add = beiLv[m_zhongZhi[i].itemId - 2066];
                if(doub)
                    add *= 2;
                res += add;
                pUser->SetData32(5,pUser->GetData32(5)+add);
                if(res > 10000000)
                    res = 10000000;
                pBangPai->SetRes3(res);
                snprintf(info,64,"偷取紫叶清莲%d,获得帮贡%d",add,add);
                pUser->SetData32(9,pUser->GetData32(9)+add);
            }
            else if((m_zhongZhi[i].itemId >= 2071) && (m_zhongZhi[i].itemId <= 2075))
            {
                uint8 beiLv[] = {8,10,15,19,28};
                int res = pBangPai->GetRes4();
                int add = beiLv[m_zhongZhi[i].itemId - 2071];
                if(doub)
                    add *= 2;
                res += add;
                pUser->SetData32(5,pUser->GetData32(5)+add);
                if(res > 10000000)
                    res = 10000000;
                pBangPai->SetRes4(res);
                snprintf(info,64,"偷取千年玄晶%d,获得帮贡%d",add,add);
                pUser->SetData32(9,pUser->GetData32(9)+add);
            }
            msg<<PRO_SUCCESS<<info;
            return true;
        }
    }
    msg<<PRO_ERROR<<"物品不存在";
    return false;
}

int CBangPai::GainResource(CUser *pUser,int id)
{
    boost::mutex::scoped_lock lk(m_mutex);
    uint16 i = (uint16)id;
    if(i >= m_zhongZhi.size())
        return 0;
    int add = 0;
    if((m_zhongZhi[i].roleId == pUser->GetRoleId()) && (m_zhongZhi[i].state == 1))
    {
        if((m_zhongZhi[i].itemId >= 2056) && (m_zhongZhi[i].itemId <= 2060))
        {
            uint8 beiLv[] = {8,10,12,14,18};
            add = m_zhongZhi[i].gain * beiLv[m_zhongZhi[i].itemId - 2056];
            m_res1 += add;
            if(m_res1 > 10000000)
                m_res1 = 10000000;
        }
        else if((m_zhongZhi[i].itemId >= 2061) && (m_zhongZhi[i].itemId <= 2065))
        {
            uint8 beiLv[] = {8,10,13,15,21};
            add = m_zhongZhi[i].gain * beiLv[m_zhongZhi[i].itemId - 2061];
            m_res2 += add;
            if(m_res2 > 10000000)
                m_res2 = 10000000;
        }
        else if((m_zhongZhi[i].itemId >= 2066) && (m_zhongZhi[i].itemId <= 2070))
        {
            uint8 beiLv[] = {8,10,14,18,24};
            add = m_zhongZhi[i].gain * beiLv[m_zhongZhi[i].itemId - 2066];
            m_res3 += add;
            if(m_res3 > 10000000)
                m_res3 = 10000000;
        }
        else if((m_zhongZhi[i].itemId >= 2071) && (m_zhongZhi[i].itemId <= 2075))
        {
            uint8 beiLv[] = {8,10,15,19,28};
            add = m_zhongZhi[i].gain * beiLv[m_zhongZhi[i].itemId - 2071];
            m_res4 += add;
            if(m_res4 > 10000000)
                m_res4 = 10000000;
        }
        pUser->SetData32(5,pUser->GetData32(5)+2*add);
        
        CNetMessage msg;
        msg.SetType(MSG_SERVER_REMOVE_OBJECT);
        msg<<(uint16)m_zhongZhi[i].itemId<<m_zhongZhi[i].x<<m_zhongZhi[i].y;
        CSocketServer &sock = SingletonSocket::instance();
        CScene *pScene = pUser->GetScene();
        if(pScene != NULL)
        {
            list<uint32> userList;
            pScene->GetUserList(userList);
            COnlineUser &onlineUser = SingletonOnlineUser::instance();
            for(list<uint32>::iterator iter = userList.begin(); iter != userList.end(); iter++)
            {
                ShareUserPtr ptr = onlineUser.GetUserByRoleId(*iter);
                if(ptr.get() == NULL) 
                    continue;
                sock.SendMsg(ptr->GetSock(),msg);
            }
        }
        m_zhongZhi[i].itemId = 0;
        m_zhongZhi[i].state = 0;
        m_zhongZhi[i].roleId = 0;
        return add;
    }
    return 0;
}

void CBangPai::Timer()
{
    if(m_jisuanZZ == 0)
    {
        m_jisuanZZ = GetSysTime();
        return;
    }
    if(m_zaiHaiTime == 0)
    {
        m_zaiHaiTime = GetSysTime();
    }
    if(m_gonggaoTime == 0)
    {
        m_gonggaoTime = GetSysTime();
    }
    
    time_t t = GetSysTime();
    struct tm *pTm = localtime(&t);
    if(pTm != NULL)
    {
        if(m_ziJinDay == 0)
            m_ziJinDay = pTm->tm_yday;
        if(m_ziJinDay != pTm->tm_yday)
        {
            int zijin = 25 * 10000;
            if((m_level >= 1) && (m_level <= 5))
                zijin = sWeiHuZiJin[m_level-1]*10000;
            m_money -= zijin;
            m_ziJinDay = pTm->tm_yday;
        }
    }
    
    bool zaiHai = false;
    if(GetSysTime() - m_zaiHaiTime > 1800)
    {
        zaiHai = true;
        m_zaiHaiTime = GetSysTime();
    }
    
    boost::mutex::scoped_lock lk(m_mutex);
    if(GetSysTime() - m_gonggaoTime > 3600)
    {
        m_gonggaoTime = GetSysTime();
        CNetMessage msg;
        msg.SetType(PRO_MSG_CHAT);
        char buf[64];
        int ziji = 0;
            
        if(m_money < 0)
        {
            if((m_level >= 1) && (m_level <= 5))
                ziji = sBangPaiZiJin[m_level-1];
            snprintf(buf,64,"帮派处于负债状态,负债超过%d万帮派将被解散",ziji);
            if(m_money < -ziji*10000)
            {
                CGetDbConnect getDb;
                CDatabaseSql *pDb = getDb.GetDbConnect();
                if(pDb == NULL)
                    return;
                CBangPaiManager &bangPaiMgr = SingletonCBangPaiManager::instance();
                bangPaiMgr.Erase(m_id);
                boost::format fmt("update bang_pai set state=0,del_time=FROM_UNIXTIME(0) where id=%1% and state!=0");
                fmt%m_id;
                pDb->Query(fmt.str().c_str());
            }
        }
        else if(m_state == 0)
        {
            snprintf(buf,64,"本帮已被帮主申请解散，现在处于解散保护期内。");
        }
        else
        {
            if((m_level >= 1) && (m_level <= 5))
                ziji = sWeiHuZiJin[m_level-1];
            snprintf(buf,64,"本帮现为%d级帮，每天维护资金为%d万",m_level,ziji);
        }
        msg<<(uint8)4<<0<<"【系统】"<<buf;
        COnlineUser &onlineUser = SingletonOnlineUser::instance();
        CSocketServer &sock = SingletonSocket::instance();
        for(list<uint32>::iterator i = m_userList.begin(); i != m_userList.end(); i++)
        {
            ShareUserPtr ptr = onlineUser.GetUserByRoleId(*i);
            if(ptr.get() != NULL)
            {
                sock.SendMsg(ptr->GetSock(),msg);
            }
        }
    }
    
    for(uint32 i = 0; i < m_zhongZhi.size(); i++)
    {
        if(m_zhongZhi[i].itemId != 0)
        {
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(m_zhongZhi[i].itemId);
            if(pItem == NULL)
                continue;
            
            if(zaiHai && (m_zhongZhi[i].state != EZZChengShu) && (Random(0,4) == 0))
            {
                if((m_zhongZhi[i].state & EZZSPoHuai) != 0)
                {
                    m_zhongZhi[i].state = EZZSPoHuai|EZZSShuiTuLiuShi;
                }
                else
                {
                    m_zhongZhi[i].state = EZZSShuiTuLiuShi;
                }
            }
            if(m_zhongZhi[i].state == EZZSJiaSu)
            {
                m_zhongZhi[i].growTime += (int)(1.2*(GetSysTime() - m_jisuanZZ));
                if(GetSysTime() - m_zhongZhi[i].time > 1800)
                    m_zhongZhi[i].state = 0;
            }
            else if((m_zhongZhi[i].state & (EZZSShuiTuLiuShi|EZZSPoHuai)) != 0)
            {
                double beiLv = 1;
                if((m_zhongZhi[i].state & EZZSPoHuai) != 0)
                    beiLv -= 0.25;
                if((m_zhongZhi[i].state & EZZSShuiTuLiuShi) != 0)
                    beiLv -= 0.25;
                m_zhongZhi[i].growTime += (int)(beiLv*(GetSysTime() - m_jisuanZZ));
            }
            else
                m_zhongZhi[i].growTime += (GetSysTime() - m_jisuanZZ);
                
            if(m_zhongZhi[i].growTime  >= pItem->addSudu*3600)
            {
                if(m_zhongZhi[i].state != EZZChengShu)
                {
                    CNetMessage msg;
                    msg.SetType(MSG_SERVER_UPDATE_OBJECT);
                    msg<<(uint16)m_zhongZhi[i].itemId<<(uint8)EZZChengShu
                        <<m_zhongZhi[i].x<<m_zhongZhi[i].y;
                    CSocketServer &sock = SingletonSocket::instance();
                    CSceneManager &scene = SingletonSceneManager::instance();
                    CScene *pScene = scene.GetBangPaiScene(305,m_id);
                    if(pScene != NULL)
                    {
                        list<uint32> userList;
                        pScene->GetUserList(userList);
                        COnlineUser &onlineUser = SingletonOnlineUser::instance();
                        for(list<uint32>::iterator iter = userList.begin(); iter != userList.end(); iter++)
                        {
                            ShareUserPtr ptr = onlineUser.GetUserByRoleId(*iter);
                            if(ptr.get() == NULL) 
                                continue;
                            sock.SendMsg(ptr->GetSock(),msg);
                        }
                    }
                }
                m_zhongZhi[i].state = EZZChengShu;
            }
            if(m_zhongZhi[i].state == EZZChengShu)
            {
                if(m_zhongZhi[i].growTime - pItem->addSudu*3600 > 12*3600)
                {
                    CNetMessage msg;
                    msg.SetType(MSG_SERVER_REMOVE_OBJECT);
                    msg<<(uint16)m_zhongZhi[i].itemId<<m_zhongZhi[i].x<<m_zhongZhi[i].y;
                    CSocketServer &sock = SingletonSocket::instance();
                    CSceneManager &scene = SingletonSceneManager::instance();
                    CScene *pScene = scene.GetBangPaiScene(305,m_id);
                    if(pScene != NULL)
                    {
                        list<uint32> userList;
                        pScene->GetUserList(userList);
                        COnlineUser &onlineUser = SingletonOnlineUser::instance();
                        for(list<uint32>::iterator iter = userList.begin(); iter != userList.end(); iter++)
                        {
                            ShareUserPtr ptr = onlineUser.GetUserByRoleId(*iter);
                            if(ptr.get() == NULL) 
                                continue;
                            sock.SendMsg(ptr->GetSock(),msg);
                        }
                    }
                    char buf[256];
                    snprintf(buf,256,"%s的资源长时间无人收集,化为尘土",m_zhongZhi[i].roleName.c_str());
                    m_histroy.push_front(buf);
                    if(m_histroy.size() > 20)
                    {
                        m_histroy.pop_back();
                    }
                    m_zhongZhi[i].itemId = 0;
                    m_zhongZhi[i].state = 0;//状态0正常，1成熟，2加速，4破坏,8水土流失
                    m_zhongZhi[i].roleId = 0;//种植角色id
                    m_zhongZhi[i].time = 0;//种植时间
                    m_zhongZhi[i].growTime = 0;
                    m_zhongZhi[i].gain = 0;
                }
            }
        }
    }
    m_jisuanZZ = GetSysTime();
}

int CBangPai::GetRankMemNum(uint8 rank)
{
    int num = 0;
    boost::mutex::scoped_lock lk(m_mutex);
    for(list<SBangPaiMember>::iterator i = m_allMember.begin(); i != m_allMember.end(); i++)
    {
        if(i->rank == rank)
            num++;
    }
    return num;
}

bool CBangPai::AddMember(uint32 id,uint8 rank,bool noLimit)
{
    boost::mutex::scoped_lock lk(m_mutex);
    m_askJoinUser.remove(id);
    if(find(m_userList.begin(),m_userList.end(),id) != m_userList.end())
        return false;
    /*
    1级帮：20人  50格田
    2级帮：40人，60格田
    3级帮：60人，70格田
    4级帮：80人，80格田
    5级帮：100人，96格田
    */
    uint32 maxNum = 110;
    if(!noLimit)
    {
        if(m_level == 1)
            maxNum = 30;
        else if(m_level == 2)
            maxNum = 50;
        else if(m_level == 3)
            maxNum = 70;
        else if(m_level == 4)
            maxNum = 90;
    }
    if(m_userList.size() >= maxNum)
        return false;
    m_userList.push_back(id);
    SBangPaiMember m;
    m.rank = rank;
    m.roleId = id;
    m_allMember.push_back(m);
    return true;
}

void CBangPai::DismissBang(uint32 roleId)
{
    if(GetMemberRank(roleId) != EBRBangZhu)
        return;
    m_state = 0;
    boost::format fmt("update bang_pai set state=0 ,del_time=FROM_UNIXTIME(%1%) where id=%2% and state!=0");
    fmt%GetSysTime()%m_id;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb != NULL)
        pDb->Query(fmt.str().c_str());
}
void CBangPai::UndismissBang(uint32 roleId)
{
    if(GetMemberRank(roleId) != EBRBangZhu)
        return;
    m_state = 1;
    boost::format fmt("update bang_pai set state=1 where id=%1%");
    fmt%m_id;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb != NULL)
        pDb->Query(fmt.str().c_str());
}

bool CBangPai::IsAdmin(uint32 id)
{
    //if(m_state == 0)
        //return false;
    boost::mutex::scoped_lock lk(m_mutex);
    for(list<SBangPaiMember>::iterator i = m_allMember.begin(); i != m_allMember.end(); i++)
    {
        //cout<<i->roleId<<endl;
        if((i->roleId == id) && (i->rank < EBRBangZhong))
            return true;
    }
    return false;
}

uint8 CBangPai::GetMemberRank(uint32 id)
{
    boost::mutex::scoped_lock lk(m_mutex);
    for(list<SBangPaiMember>::iterator i = m_allMember.begin(); i != m_allMember.end(); i++)
    {
        if(i->roleId == id)
            return i->rank;
    }
    return 0;
}

void CBangPai::SetMemberRank(uint32 id,uint8 rank)
{
    boost::mutex::scoped_lock lk(m_mutex);
    for(list<SBangPaiMember>::iterator i = m_allMember.begin(); i != m_allMember.end(); i++)
    {
        if(i->roleId == id)
        {
            i->rank = rank;
            return;
        }
    }
}

bool CBangPai::IsAskJoin(uint32 id)
{
    boost::mutex::scoped_lock lk(m_mutex);
    return find(m_askJoinUser.begin(),m_askJoinUser.end(),id) != m_askJoinUser.end();
}

bool CBangPai::AddAskJoin(uint32 id)
{
    boost::mutex::scoped_lock lk(m_mutex);
    if(find(m_askJoinUser.begin(),m_askJoinUser.end(),id) != m_askJoinUser.end())
        return false;
        
    if(m_askJoinUser.size() >= 0xff)
        return false;
    m_askJoinUser.push_back(id);
    
    return true;
}

void CBangPai::DelMember(uint32 id)
{
    boost::mutex::scoped_lock lk(m_mutex);
    for(list<SBangPaiMember>::iterator i = m_allMember.begin(); i != m_allMember.end(); i++)
    {
        if(i->roleId == id)
        {
            m_allMember.erase(i);
            break;
        }
    }
    m_userList.remove(id);
}

void CBangPai::Read(char *str)
{
    if(str == NULL)
        return;
    int len = strlen(str)/2;
    if(len <= 0)
        return;
    //boost::scoped_ptr<uint8> ptr(new uint8[len]);
    uint8 *hex = new uint8[len];
    StrToHex(str,hex,len);
    
    CNetMessage msg;
    msg.WriteData(hex,len);
    
    uint8 num = 0;
    msg>>num;
    uint32 userId;
    uint8 rank;
    for(; num > 0; num--)
    {
        msg>>userId;
        msg>>rank;
        AddMember(userId,rank,true);
    }
    
    msg>>num;
    for(; num > 0; num--)
    {
        userId = 0;
        msg>>userId;
        if(userId != 0)
        {
            m_askJoinUser.push_back(userId);
        }
    }
    delete []hex;
}

void CBangPai::Save()
{
    CNetMessage msg;
    
    uint8 num = m_userList.size();
    msg<<num;
    list<SBangPaiMember>::iterator i = m_allMember.begin();
    for(; i != m_allMember.end(); i++)
    {
        msg<<i->roleId<<i->rank;
    }
    
    num = m_askJoinUser.size();
    msg<<num;
    list<uint32>::iterator iter = m_askJoinUser.begin();
    for(; iter != m_askJoinUser.end(); iter++)
    {
        msg<<*iter;
    }
    
    string str;
    HexToStr((uint8*)(msg.GetMsgData()->c_str() + 4),msg.GetDataLen() - 4,str);
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    boost::format fmt("update bang_pai set info = \"%1%\",level=%2%,fanrong=%3%,"\
                "money=%4%,huodong_time=%5%,kouhao='%6%',gonggao='%7%',beixz_time=%8%,"\
                "xz_time=%9%,xz_bangid=%10%,res1=%11%,res2=%12%,res3=%13%,res4=%14%,"\
                "bq_level=%15%,sy_level=%16%,bc_level=%17%,sl_level=%18%,jm_level=%19%,"\
                "zc_level=%20%,fzc_level=%21%,jie_meng='%22%',del_jm_time=%23% where id=%24%");
    char jiemeng[32] = {0};
    for(list<uint32>::iterator i = m_jieMengList.begin(); i != m_jieMengList.end(); i++)
    {
        if(i != m_jieMengList.begin())
            strcat(jiemeng,"|");
        sprintf(jiemeng+strlen(jiemeng),"%u",*i);
    }
    fmt%str 
        %(int)m_level
        %m_fanrong
        %m_money
        %(int)m_huodongTime
        %m_kouhao
        %m_gonggao
        %m_beiXZTime
	    %m_XZTime
	    %m_XZBangId//宣战帮派id
	    %m_res1
	    %m_res2
	    %m_res3
	    %m_res4
	    %m_bqLevel
    	%m_syLevel
    	%m_bcLevel
    	%m_slLevel
    	%m_jmLevel
    	%m_zcLevel 
    	%m_fzcLevel
    	%jiemeng
    	%m_delJMTime
	    %m_id;
        //%(int)m_state;
    pDb->Query(fmt.str().c_str());
}

void CBangPai::SetBeiXuanZhan(time_t t)//设置被宣战
{
    m_beiXZTime = t;
}

bool CBangPai::IsBeiXuanZhan()//是否被宣战
{
    return m_beiXZTime + 3600*20 < GetSysTime();
}

void CBangPai::SetXuanZhanBang(uint32 bId,time_t t)//设置宣战帮派id
{
    m_XZBangId = bId;
    m_XZTime = t;//GetSysTime();
}
int CBangPai::GetXuanZhanBang()//得到宣战帮派id
{
    if(m_XZTime + 3600*20 > GetSysTime())
        return m_XZBangId;
    return 0;
}

CBangPai *CBangPaiManager::FindBangPai(uint32 id)
{
    CBangPai *pBangPai = NULL;
    boost::mutex::scoped_lock lk(m_mutex);
    m_bangPaiList.Find(id,pBangPai);
    return pBangPai;
}

CBangPai *CBangPaiManager::CreateBangPai(CUser *pUser,const char *name)
{
    if(pUser == NULL)
        return NULL;
        
    CBangPai *pBangPai = new CBangPai;
    m_curId++;
    pBangPai->SetId(m_curId);
    pBangPai->SetName(name);
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return NULL;
    boost::format fmt("INSERT INTO bang_pai (id,name) VALUES (%1%,\"%2%\")");
    fmt%m_curId%name;
    if(pDb->Query(fmt.str().c_str()))
    {
        pBangPai->AddMember(pUser->GetRoleId(),EBRBangZhu);
        boost::mutex::scoped_lock lk(m_mutex);
        pBangPai->InitZhongZhi(false);
        m_bangPaiList.Insert(m_curId,pBangPai);
        return pBangPai;
    }
    delete pBangPai;
    return NULL;
}

void CBangPaiManager::Erase(uint32 id)
{
    boost::mutex::scoped_lock lk(m_mutex);
    m_bangPaiList.Erase(id);
}

void CBangPaiManager::DelBangPai(uint32 id)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    boost::format fmt("update bang_pai set state=0,del_time=FROM_UNIXTIME(%1%) where id=%2% and state!=0");
    fmt%GetSysTime()%id;
    pDb->Query(fmt.str().c_str());
}

bool CBangPaiManager::Init()
{
    m_curId = 0;
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return false;
                          //0   1   2   3       4       5       6           7       8
    if(!pDb->Query("SELECT id,name,info,level,fanrong,money,huodong_time,kouhao,gonggao,"\
                      //9   10      11          12      13    14    15  16      17
                    "state,title,beixz_time,xz_bangid,xz_time,res1,res2,res3,res4,"\
                    //18        19      20      21          22      23      24          25  26      28
                    "bq_level,sy_level,bc_level,sl_level,jm_level,zc_level,fzc_level,state,jie_meng,del_jm_time FROM bang_pai where state=1 "\
                    "or UNIX_TIMESTAMP(now()) - UNIX_TIMESTAMP(del_time)<7*24*3600 order by id"))
    {
        return false;
    }
    char **row;
    while((row = pDb->GetRow()) != NULL)
    {
        CBangPai *pBangPai = new CBangPai;
        m_curId = atoi(row[0]);
        pBangPai->SetId(m_curId);
        pBangPai->SetName(row[1]);
        pBangPai->Read(row[2]);
        pBangPai->SetLevel(atoi(row[3]));
        pBangPai->SetFanRong(atoi(row[4]));
        pBangPai->SetMoney(atoi(row[5]));
        pBangPai->SetHuoDongTime(atoi(row[6]));
        pBangPai->SetKouHao(row[7]);
        pBangPai->SetGongGao(row[8]);
        //pBangPai->SetState(atoi(row[9]));
        pBangPai->SetTitle(atoi(row[10]));
        pBangPai->SetBeiXuanZhan(atoi(row[11]));
        pBangPai->SetXuanZhanBang(atoi(row[12]),atoi(row[13]));
        pBangPai->SetRes1(atoi(row[14]));
        pBangPai->SetRes2(atoi(row[15]));
        pBangPai->SetRes3(atoi(row[16]));
        pBangPai->SetRes4(atoi(row[17]));
        pBangPai->m_bqLevel = atoi(row[18]);
        pBangPai->m_syLevel = atoi(row[19]);
        pBangPai->m_bcLevel = atoi(row[20]);
        pBangPai->m_slLevel = atoi(row[21]);
        pBangPai->m_jmLevel = atoi(row[22]);
    	pBangPai->m_zcLevel = atoi(row[23]);
    	pBangPai->m_fzcLevel = atoi(row[24]);
        pBangPai->InitZhongZhi();
        pBangPai->SetState(atoi(row[25]));
        pBangPai->SetJieMing(row[26]);
        pBangPai->SetDelJMTime(atoi(row[27]));
        m_bangPaiList.Insert(m_curId,pBangPai);
    }
    if(pDb->Query("select max(id) from bang_pai"))
    {
        char **row = pDb->GetRow();
        if((row != NULL) && (row[0] != NULL))
            m_curId = atoi(row[0])+1;
    }
    return true;
}

static bool EachBangPai(CBangPai *pBangPai,vector<CBangPai*> *pBangPaiList)
{
    pBangPaiList->push_back(pBangPai);
    return true;
}

struct SSortBangPai
{
    bool operator()(CBangPai *const &b1,CBangPai *const &b2)
    {
        return b1->GetFanRong() > b2->GetFanRong();
        //return true;
    }
};

void CBangPaiManager::MakeXZBang(CNetMessage &msg,int bId)
{
    if(bId == 0)
    {
        msg<<(uint8)0;
        return;
    }
    CBangPai *pBangPai = NULL;
    if(bId != 0)
        pBangPai = FindBangPai(bId);
    uint8 num = 0;
    uint8 pos = msg.GetDataLen();
    msg<<num;
    
    vector<CBangPai*> bangpaiList;
    {
        boost::mutex::scoped_lock lk(m_mutex);
        m_bangPaiList.ForEach(boost::bind(EachBangPai,_2,&bangpaiList));
    }
    const int ONE_PAGE_NUM = 20;
    vector<CBangPai*>::iterator i = bangpaiList.begin();
    
    if(bangpaiList.size() > 0)
    {
        SSortBangPai bangSort;
        std::sort(bangpaiList.begin(),bangpaiList.end(),bangSort);
    }
    for(; i != bangpaiList.end(); i++)
    {
        uint8 state = 0;
        if(bId != 0)
        {
            if((*i)->GetXuanZhanBang() == bId)
                state = 1;
            else if((pBangPai != NULL) && (pBangPai->GetXuanZhanBang() == (int)(*i)->GetId()))
                state = 1;
            else
                continue;
            msg<<(*i)->GetId()<<(*i)->GetName()<<(int)(*i)->GetFanRong()
                <<state<<(uint8)(*i)->GetTitle();
            num++;
            if(num >= ONE_PAGE_NUM)
                break;
        }
    }
    
    msg.WriteData(pos,&num,sizeof(num));
}

void CBangPaiManager::Timer()
{
    vector<CBangPai*> bangpaiList;
    {
        boost::mutex::scoped_lock lk(m_mutex);
        m_bangPaiList.ForEach(boost::bind(EachBangPai,_2,&bangpaiList));
    }
    for(uint32 i = 0; i < bangpaiList.size(); i++)
    {
        bangpaiList[i]->Timer();
    }
}

void CBangPaiManager::SaveZhongZhi()
{
    vector<CBangPai*> bangpaiList;
    {
        boost::mutex::scoped_lock lk(m_mutex);
        m_bangPaiList.ForEach(boost::bind(EachBangPai,_2,&bangpaiList));
    }
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    pDb->Query("delete from bang_zz");
    for(uint32 i = 0; i < bangpaiList.size(); i++)
    {
        bangpaiList[i]->SaveZhongZhi(pDb);
    }
}

void CBangPaiManager::MakeBangPaiList(uint8 page,CNetMessage &msg,int bId,bool haveMeBang)
{
    if(page > 0)
        page--;
    
    CBangPai *pBangPai = NULL;
    if(bId != 0)
    {
        pBangPai = FindBangPai(bId);
        if(pBangPai == NULL)
            return;
    }
    
    uint8 num = 0;
    uint8 pos = msg.GetDataLen();
    msg<<num;
    
    vector<CBangPai*> bangpaiList;
    {
        boost::mutex::scoped_lock lk(m_mutex);
        m_bangPaiList.ForEach(boost::bind(EachBangPai,_2,&bangpaiList));
    }
    if(bangpaiList.size() > 0)
    {
        SSortBangPai bangSort;
        std::sort(bangpaiList.begin(),bangpaiList.end(),bangSort);
    }
    
    const int ONE_PAGE_NUM = 20;
    vector<CBangPai*>::iterator i = bangpaiList.begin();
    for(int j = 0; j < page*ONE_PAGE_NUM; j++)
    {
        i++;
        if(i == bangpaiList.end())
            break;
    }
    
    for(; i != bangpaiList.end(); i++)
    {
        if(!haveMeBang && ((int)(*i)->GetId() == bId))
            continue;
        uint8 state = 0;
        if(bId != 0)
        {
            if((*i)->GetXuanZhanBang() == bId)
                state = 1;
            else if((pBangPai != NULL) && (pBangPai->GetXuanZhanBang() == (int)(*i)->GetId()))
                state = 1;
            if(pBangPai->JieMeng(*i))
                state |= 2;
        }
        msg<<(*i)->GetId()<<(*i)->GetName()<<(int)(*i)->GetFanRong()
            <<state<<(uint8)(*i)->GetTitle();
        num++;
        if(num >= ONE_PAGE_NUM)
            break;
    }
    
    msg.WriteData(pos,&num,sizeof(num));
}
