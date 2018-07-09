#include "user.h"
#include "scene_manager.h"
#include "utility.h"
#include "net_msg.h"
#include "singleton.h"
#include "database.h"
#include "call_script.h"
#include "md5.h"
#include "script_call.h"
#include <algorithm>
#include <functional>      // For greater<int>( )
#include <boost/format.hpp>
#include <time.h>
#include <vector>
#include <sys/socket.h>
using namespace std;

extern void TransportUser(CUser *pUser,int sceneId,uint8 x,uint8 y,uint8 face);
extern int WWGetWinBang();
extern bool InWorldWar();

CUser::CUser():m_step(1),m_pScene(NULL)//,m_pOleScene(NULL)
{
    int i;
    for(i = 0; i < MAX_MISSION_NUM; i++)
    {
        m_mission[i].id = 0;
    }
    for(i = 0; i < MAX_SAVE_NUM; i++)
    {
        m_shortArray[i] = 0;
    }
    m_yewaiShiYao = 0;
    
    SetFight(0,0);
    
    m_readIgnore = false;    
    m_askForJoinTeam = 0;
    m_teamId = 0;
    m_face = 8;
    m_chuZhanPet = 0xff;
    m_gensuiPet = 0xff;
    m_qiPet = 0xff;
    m_petNum = 0;
    m_tradeUserId = 0;
    m_tradeMoney = 0;
    
    m_userOp = 0;
    m_userPara = 0;
    m_target = 0;
    m_petOp = 0;
    m_petPara = 0;
    m_petTar = 0;
    m_maxMp = 0;
    //m_enterSceneCall = 0;
    m_inJump = false;
    m_autoFightTurn =   0;
    m_tongBao = 0;
    m_logout = false;
    m_roleId = 0;
    m_jianYuTime = 0;
    memset(m_role,0,sizeof(m_role));
    m_admin = 0;
    //timeval tv;
    //gettimeofday(&tv,NULL);
    m_moveTime = 0;
    m_moveErrTimes = 0;
    m_saveDataTime = GetSysTime();
    m_addTiliTime = GetSysTime();
    m_huodongTime = GetSysTime();
    m_lastHeartTime = GetSysTime();
    m_heartTimes = 0;
    m_heartErrTimes = 0;
    m_openPack = 0;
    MAX_PACKAGE_NUM = 36;
    m_sDoubleEnd = 0;   
    memset(m_package,0,sizeof(m_package));
    memset(m_shopItemId,0,sizeof(m_shopItemId));
    m_inTrade = false;
    m_bangZhanScore = 0;
    m_XZBangId = 0;
    m_tempLeaveTeam = 0;
    m_activityTime = 0;
    m_guanFight = 0;
    m_giftBitset = 0;
    m_gift.type = 0;//1物品，2经验、3潜能，4道行
    m_gift.val = 0;
    m_gift.num = 0;
    m_giftTime = 0;
    m_bdTongBao = 0;
}

void CUser::Init()
{
    /*********************
    当门派为“水”时，气血公式为：
    等级*60+(体质-等级)*17+(水相)*170+120+装备附加
    当门派为“火”时，气血公式为：
    等级*35+(体质-等级)*10+(水相)*100+80+装备附加
    其他门派气血公式为：
    等级*50+(体质-等级)*15+(水相)*150+100+装备附加
    **********************/
    
    //等级*50+(体质-等级)*15+(水相)*150+100+装备附加
    m_maxHp = (uint32)(m_level*50+(GetTiZhi()-m_level)*15+GetShui()*150+100+GetItemQiXue()) + GetQiPetHp();
    
    //等级*35+(灵性-等级)*10.5+(木相)*105+70+装备附加
    m_maxMp = (uint32)(m_level*35+(GetLingLi()-m_level)*10.5+GetMu()*105+70+GetItemFaLi()+ GetQiPetMp());
    
    //m_damage = (int)(m_level*16+(GetLiLiang()-m_level)*6+GetTu()*60+100+GetItemGongji());
    //等级*13+(力量-等级)*5+(土相)*50+装备附加
    //等级*13+(力量-等级)*5+土相*25+装备附加
    m_damage = (int)(m_level*13+(GetLiLiang()-m_level)*5+GetTu()*25+GetItemGongji());
    switch(m_xiang)
    {
    case EXShuiXiang://3－水
        m_maxHp = (uint32)(m_level*60+(GetTiZhi()-m_level)*17+GetShui()*170+120+GetItemQiXue())+GetQiPetHp();
        break;
    case EXHuoXiang://4－火
        m_maxHp = (uint32)(m_level*35+(GetTiZhi()-m_level)*10+GetShui()*100+80+GetItemQiXue())+GetQiPetHp();
        break;
    case EXTuXiang://5－土
        //等级*16+(力量-等级)*6+(土相)*60+装备附加
        //等级*15+(力量-等级)*5+土相*25 +装备附加
        m_damage = (int)(m_level*15+(GetLiLiang()-m_level)*5+GetTu()*25+GetItemGongji());
        break;
    case EXMuXiang:
        //等级*13+(力量-等级)*5+(土相)*40+(木相)*20+装备附加
        //等级*13+(力量-等级)*5+土相*20+木相*10 +装备附加
        m_damage = (int)(m_level*13+(GetLiLiang()-m_level)*5+GetTu()*20+GetMu()*10+GetItemGongji());
        break;
    default:
        break;
    }
            
    /************
    当门派为“木”时，防御公式为：
    等级*5+(耐力-等级)*1.8+30+装备附加
    其他门派的防御公式为：
    等级*9+(耐力-等级)*2.8+60+装备附加
    *************/
    if(m_xiang == EXMuXiang)
        m_recovery = (int)(m_level*5+(GetNaiLi()-m_level)*1.8+30+GetItemfangYu());
    else
        m_recovery = (int)(m_level*9+(GetNaiLi()-m_level)*2.8+60+GetItemfangYu());

    /*************
    当门派为“火”时，速度公式为：
    等级*2.7+(敏捷-等级)*1+(火相)*10+10+装备附加
    当门派为“金”时，速度公式为：
    等级*1.7+(敏捷-等级)*0.6+(火相)*6+10+装备附加
    其他门派速度公式为：
    等级*2.2+(敏捷-等级)*0.8+(火相)*8+10+装备附加
    **************/
    /*
    当门派为“火”时，速度公式为：
    等级*5+(敏捷-等级)*0.8+(火相)*8+20+装备附加
    当门派为“金”时，速度公式为：
    等级*1.6+(敏捷-等级)*0.8+(火相)*8+装备附加
    其他门派速度公式为：
    等级*2.2+(敏捷-等级)*0.8+(火相)*8+10+装备附加
    */
    if(m_xiang == EXHuoXiang)
        //m_speed = (int)(m_level*2.7 + (GetMinJie()-m_level)+GetHuo()*10+10+GetItemSudu());
        m_speed = (int)(m_level*5+GetMinJie()-m_level*0.8+GetHuo()*8+20+GetItemSudu());
    else if(m_xiang == EXJinXiang)
        m_speed = (int)(m_level*1.6 + (GetMinJie()-m_level)*0.8+GetHuo()*8+GetItemSudu());
    else 
        m_speed = (int)(m_level*2.2 + (GetMinJie()-m_level)*0.8+GetHuo()*8+10+GetItemSudu());
    if(m_mp > m_maxMp)
        m_mp = m_maxMp;
    if(m_hp > m_maxHp)
        m_hp = m_maxHp;
}

void CUser::UpdateInfo()
{
    /*
    TYPE=28 法攻
    TYPE=29 命中 
    TYPE=30 回避
    */
    SendUpdateInfo(16,GetTiZhi());//m_tizhi);
    SendUpdateInfo(17,GetLiLiang());//m_liliang);
    SendUpdateInfo(18,GetMinJie());//m_minjie);
    SendUpdateInfo(19,GetLingLi());//m_lingli);
    SendUpdateInfo(20,GetNaiLi());//m_naili);
    SendUpdateInfo(4,m_shuxingdian);
    
    SendUpdateInfo(21,GetJin());//m_jin);
    SendUpdateInfo(22,GetMu());//m_mu);
    SendUpdateInfo(23,GetShui());//m_shui);
    SendUpdateInfo(24,GetHuo());//m_huo);
    SendUpdateInfo(25,GetTu());//m_tu);
    SendUpdateInfo(5,m_xiangxingdian);
    SendUpdateInfo(9,m_hp);
    SendUpdateInfo(10,m_maxHp);
    //cout<<"update maxMp:"<<(int)m_maxMp<<endl;
    SendUpdateInfo(11,m_mp);
    SendUpdateInfo(12,m_maxMp);
    SendUpdateInfo(13,m_damage);
    SendUpdateInfo(14,m_speed);
    SendUpdateInfo(15,m_recovery);
    int fagong;
    uint16 mingzhong,huibi;
    GetViewPara(fagong,mingzhong,huibi);
    SendUpdateInfo(28,fagong);
    SendUpdateInfo(29,mingzhong);
    SendUpdateInfo(30,huibi);
    
    SendUpdateInfo(33,GetItemLianjiLv());
    SendUpdateInfo(34,GetItemBaoJiLv());
    SendUpdateInfo(35,GetItemFanJiLv());
    SendUpdateInfo(36,GetItemFanZhenLv());
    SendUpdateInfo(37,GetFaShuFanTanLv());
    SendUpdateInfo(38,GetFaShuBaoJi());
}

void CUser::GetFacePos(uint8 &x,uint8 &y)
{ 
    x = m_xPos; y = m_yPos;
    switch (m_face)
    {
    case 2:
        y = m_yPos-1;
        break;
    case 4:
        x = m_xPos-1;
        break;
    case 6:
        x = m_xPos + 1;
        break;
    case 8:
        y = m_yPos + 1;
        break;
    }
}

void CUser::Move(uint8 direction,uint8 step)//DIR=2 向上 DIR=4 向左 DIR=6 向右 DIR=8 向下
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    switch (direction)
    {
    case 2:
        m_yPos -= step;
        break;
    case 4:
        m_xPos -= step;
        break;
    case 6:
        m_xPos += step;
        break;
    case 8:
        m_yPos += step;
        break;
    default:
        return;
    }
    m_face = direction;
}

void CUser::SetFace(uint8 face)
{
    if((face == 2) || (face == 4) 
        || (face == 6) || (face == 8))
    {
        m_face = face;
    }
}
void CUser::EnterScene(CScene *pScene)
{
    UserJump(true);
    if(pScene == NULL)
        return;
    
    CCallScript *pScript = pScene->GetScript();
    if(pScript != NULL)
    {
        //SetCall(
        //SetCall(pScene->GetId()+10000,"EnterScene");
        if((m_teamId == 0) || (m_teamId == m_roleId))
        {
            pScript->Call("EnterScene","u",this);
            //SetCallScript(pScene->GetId()+10000);
            SetCallScript(pScript->GetScriptId());
#ifdef DEBUG
            cout<<"call script:"<<pScene->GetId()+10000<<endl;
#endif
        }
    }
    
    pScene->ChangeScene(this,m_pScene);
    
    //m_pOleScene = m_pScene;
    m_pScene = pScene;
    if(m_pScene != NULL)
        SetData32(3,m_pScene->GetId());
    //if(m_pOleScene != NULL)
        //SetData16(3,m_pOleScene->GetId());
}

void CUser::EnterFuBen(uint16 sceneId)
{
    CSceneManager &sceneMgr = SingletonSceneManager::instance();
    m_pScene = sceneMgr.FindScene(GetData32(3));
    //m_pOleScene = sceneMgr.FindScene(GetData16(3));
    if(m_pScene == NULL)
    {
        m_pScene = sceneMgr.FindScene(301);
        m_pScene->ChangeScene(this,NULL);
        m_xPos = 8;
        m_yPos = 19;
        CNetMessage msg;
        msg.SetType(PRO_JUMP_SCENE);
        msg<<m_pScene->GetMapId()<<m_xPos<<m_yPos<<m_face;
        CSocketServer &sock = SingletonSocket::instance();
        sock.SendMsg(m_sock,msg);
        /*m_pScene = sceneMgr.FindScene(301);
        m_xPos = 8;
        m_yPos = 19;
        if(sceneId == LAN_RUO_DI_GONG_ID)
            m_pScene = sceneMgr.GetLanRuoDiGong();
        else
            m_pScene = sceneMgr.GetFuBen(sceneId);*/
    }
    else
    {
        m_pScene->ChangeScene(this,NULL);
    }
}

void CUser::SetEquip(char *pEquip)
{
    if((pEquip == NULL) || (strlen(pEquip) < sizeof(m_equipment)))
    {
        memset(m_equipment,0,sizeof(m_equipment));
        return;
    }
    StrToHex(pEquip,(uint8*)m_equipment,sizeof(m_equipment));
    //uint32 len = 0;
    //UnCompress(pEquip,(uint8*)m_equipment,&len);
}

SharePetPtr CUser::GetPet(uint8 pos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    if(pos < m_petNum)
    {
        return m_pet[pos];
    }    
    SharePetPtr pet;
    return pet;
}

SPet CUser::GetPetByPos(uint8 pos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    if((pos >= m_petNum) || (m_pet[pos].get() == NULL))
    {
        SPet pet = {0};
        return pet;
    }
    
    return *(m_pet[pos].get());
}

void CUser::PetRestore(uint8 pos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    if((pos >= m_petNum) || (m_pet[pos].get() == NULL))
    {
        return;
    }    
    m_pet[pos]->hp = m_pet[pos]->maxHp;
    m_pet[pos]->mp = m_pet[pos]->maxMp;
    UpdatePetInfo(pos,5,m_pet[pos]->hp);
    UpdatePetInfo(pos,6,m_pet[pos]->mp);
}

void CUser::SetPetZhongCheng(uint8 pos,int zhongcheng)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    if((pos >= m_petNum) || (m_pet[pos].get() == NULL))
    {
        return;
    }
    m_pet[pos]->zhongcheng = zhongcheng;
    UpdatePetInfo(pos,13,zhongcheng);
}

void CUser::MakePetInfo(SPet *pPet,uint8 state,CNetMessage &msg)
{
    //(等级*4+(灵性-等级)*1.2))*(法攻成长*0.13)+90
    msg<<(uint16)pPet->tmplId<<pPet->name<<pPet->type<<state<<pPet->xiang<<pPet->level
        <<pPet->exp<<pPet->hp<<pPet->maxHp<<pPet->mp<<pPet->maxMp
        <<pPet->GetDamage()<<pPet->GetSkillAttack()
        <<(uint16)pPet->speed<<pPet->GetRecovery()
        <<pPet->wuxue<<pPet->xiuWei<<pPet->shouming<<pPet->zhongcheng
        <<pPet->qinmi;
    MakeItemInfo(&(pPet->kaiJia),msg);
    //300000时，移动速度不变。每提升200000亲密度，移动速度提升1格，封顶提升3格
    msg<<(uint8)GetPetSpeed(pPet->qinmi);
    
    msg<<pPet->GetTiZhi(true)<<pPet->GetLiLiang(true)<<pPet->GetMinJie(true)
        <<pPet->GetLingXing(true)<<pPet->GetNaiLi(true)<<pPet->shuxingdian
        <<pPet->hpCZ<<pPet->mpCZ<<pPet->speedCZ<<pPet->attackCZ<<pPet->skillAttackCZ;
    uint8 num = 0;
    uint16 pos = msg.GetDataLen();
    msg<<num;
    for(uint8 i = 0; i < SPet::MAX_SKILL_NUM; i++)
    {
        if(pPet->skill[i] != 0)
        {
            num++;
            msg<<pPet->skill[i]<<pPet->skillLevel[i];
        }
    }
    msg.WriteData(pos,&num,1);
}

bool CUser::MakePetInfo(uint8 pos,CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    return NoLockMakePetInfo(pos,msg);
}

bool CUser::NoLockMakePetInfo(uint8 pos,CNetMessage &msg)
{
    if(pos >= MAX_PET_NUM)
    {
        pos = m_chuZhanPet;
        if(pos >= MAX_PET_NUM)
            return false;
    }

    SPet *pPet = m_pet[pos].get();
    if(pPet == NULL)
        return false;

    uint8 state = 0;
    if(m_chuZhanPet == pos)
    {
        state |= 0x2;
    }
    if(m_gensuiPet == pos)
    {
        state |= 0x1;
    }
    if(m_qiPet == pos)
    {
        state |= 0x8;
    }
    if(pPet->bangDing == 1)
    {
        state |= 0x4;
    }
    MakePetInfo(pPet,state,msg);
    return true;
}

void CUser::MakePet(CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    msg<<m_petNum;
    for(uint8 i = 0; i < m_petNum; i++)
    {
        NoLockMakePetInfo(i,msg);
    }
}

bool CUser::NoLockAddPet(SharePetPtr &pPet)
{
    uint8 maxNum = min(3 + m_level/10,(int)MAX_PET_NUM);
    if(m_petNum >= maxNum)
    {
        return false;
    }
    
    m_pet[m_petNum] = pPet;
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PET);
    msg<<(uint8)1<<m_petNum;
    NoLockMakePetInfo(m_petNum,msg);
    sock.SendMsg(m_sock,msg);
    
    m_petNum++;
    return true;
}

bool CUser::AddPet(SharePetPtr &pPet)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    return NoLockAddPet(pPet);
}

void CUser::SetPet(char *pPet)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if((pPet == NULL) || (strlen(pPet) < 4))
    {
        m_petNum = 0;
        m_chuZhanPet = 0xff;
        m_gensuiPet = 0xff;
        for(uint8 i = 0; i < MAX_PET_NUM; i++)
        {
            m_pet[i].reset();
        }
        return;
    }
    uint32 len = strlen(pPet)/2;
    char pTemp[len];
    StrToHex(pPet,(uint8*)pTemp,len);
    /*uint32 len = MAX_PET_NUM*sizeof(SPet)+12;
    uint8 pTemp[len];
    UnCompress(pPet,pTemp,&len);*/
    
    memcpy(&m_petNum,pTemp,sizeof(m_petNum));
    if((m_petNum > MAX_PET_NUM) || (m_petNum*sizeof(SPet) != (len-4)))
    {
        //m_petNum = MAX_PET_NUM;
        m_petNum = 0;
        return;
    }
    
    memcpy(&m_chuZhanPet,pTemp+1,sizeof(m_chuZhanPet));
    memcpy(&m_gensuiPet,pTemp+2,sizeof(m_gensuiPet));
    memcpy(&m_qiPet,pTemp+3,sizeof(m_qiPet));
    
    if(m_chuZhanPet >= m_petNum)
    {
        m_chuZhanPet = 0xff;
    }
    if(m_gensuiPet >= m_petNum)
    {
        m_gensuiPet = 0xff;
    }
    if(m_qiPet >= m_petNum)
    {
        m_qiPet = 0xff;
    }
    for(uint8 i = 0; i < m_petNum; i++)
    {
        SPet *pPet = new SPet;
        memcpy(pPet,pTemp + 4 + sizeof(SPet) * i,sizeof(SPet));
        m_pet[i].reset(pPet);
    }
}

void CUser::GetPet(string &str)
{
    //boost::recursive_mutex::scoped_lock lk(m_mutex);
    int len = m_petNum * sizeof(SPet) + 4;
    uint8 hex[len];
    hex[0] = m_petNum;
    hex[1] = m_chuZhanPet;
    hex[2] = m_gensuiPet;
    hex[3] = m_qiPet;
    
    if(m_petNum > MAX_PET_NUM)
    {
        m_petNum = MAX_PET_NUM;
    }
    
    for(uint8 i = 0; i < m_petNum; i++)
    {
        SPet *pPet = m_pet[i].get();
        if(pPet == NULL)
        {
            m_petNum = i;
            hex[0] = m_petNum;
            break;
        }
        memcpy(hex + 4 + sizeof(SPet) * i,pPet,sizeof(SPet));
    }
    HexToStr(hex,len,str);
    //Compress(hex,len,str);
}

void CUser::SetPackage(char *pPack)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if((pPack == NULL) || (strlen(pPack) < sizeof(m_package)))
    {
        return;
    }
    StrToHex(pPack,(uint8*)m_package,sizeof(m_package));
    //uint32 len = 0;
    //UnCompress(pPack,(uint8*)m_package,&len);
    for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if((m_package[i].tmplId == 1544) 
            || (m_package[i].tmplId == 1543)
            || (m_package[i].tmplId == 1542))
        {
            memset(m_package+i,0,sizeof(m_package[i]));
        }
    }
}

void CUser::SetBankItem(char *pBankItem)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    uint32 len = sizeof(m_bankItem)+10240;
    uint8 *p = new uint8[len];
    auto_ptr<uint8> autoDel(p);
    
    if(!UnCompress(pBankItem,p,len))
    {
        memset(m_bankItem,0,sizeof(m_bankItem));
        UnHexify((uint8*)m_bankItem,pBankItem);
        return;
    }
    
    CNetMessage msg;
    msg.WriteData(p,len);
    for(uint8 i = 0; i < MAX_BANK_ITEM_NUM; i++)
    {
        msg>>m_bankItem[i];
    }
    
    uint16 num = 0;
    msg>>num;    
    for(uint16 i = 0; i < num; i++)
    {
        uint16 pos = 0;
        uint8 val = 0;
        msg>>pos>>val;
        m_saveData8[pos] = val;
    }
    
    num = 0;
    msg>>num;
    for(uint16 i = 0; i < num; i++)
    {
        uint16 pos = 0;
        uint16 val = 0;
        msg>>pos>>val;
        m_saveData16[pos] = val;
    }
    
    num = 0;
    msg>>num;    
    for(uint16 i = 0; i < num; i++)
    {
        uint16 pos = 0;
        uint32 val = 0;
        msg>>pos>>val;
        m_saveData32[pos] = val;
    }
    
    num = 0;
    msg>>num;
    for(uint16 i = 0; i < num; i++)
    {
        SPet pet;
        msg>>pet;
        m_bankPet.push_back(pet);
    }
    uint8 titleNum = 0;
    msg>>titleNum;
    for(uint8 i = 0; i < titleNum; i++)
    {
        string str;
        msg>>str;
        if(str.size() > 0)
            m_textTitle.push_back(str);
    }
    msg>>m_useTextTitle;
}

void CUser::GetEquipment(string &str)
{
    //boost::recursive_mutex::scoped_lock lk(m_mutex);
    HexToStr((uint8*)m_equipment,sizeof(m_equipment),str);
    //Compress((uint8*)m_equipment,sizeof(m_equipment),str);
}

void CUser::GetPackage(string &str)
{
    HexToStr((uint8*)m_package,sizeof(m_package),str);
}

void CUser::GetBankItem(string &str)
{
    CNetMessage msg;
    for(uint8 i = 0; i < MAX_BANK_ITEM_NUM; i++)
    {
        msg<<m_bankItem[i];
    }
    uint16 num = m_saveData8.size();
    msg<<num;
    
    for(map<uint16,uint8>::iterator i = m_saveData8.begin(); i != m_saveData8.end(); i++)
    {
        msg<<i->first<<i->second;
    }
    
    num = m_saveData16.size();
    msg<<num;
    for(map<uint16,uint16>::iterator i = m_saveData16.begin(); i != m_saveData16.end(); i++)
    {
        msg<<i->first<<i->second;
    }
    
    num = m_saveData32.size();
    msg<<num;
    for(map<uint16,uint32>::iterator i = m_saveData32.begin(); i != m_saveData32.end(); i++)
    {
        msg<<i->first<<i->second;
    }
    
    num = m_bankPet.size();
    msg<<num;
    for(uint16 i = 0; i < num; i++)
    {
        msg<<m_bankPet[i];
    }
    num = m_textTitle.size();
    msg<<(uint8)num;
    for(list<string>::iterator i = m_textTitle.begin(); i != m_textTitle.end(); i++)
    {
        msg<<*i;
    }
    msg<<m_useTextTitle;
    Compress((uint8*)(msg.GetMsgData()->c_str() + 4),msg.GetDataLen(),str);
    //HexToStr((uint8*)m_bankItem,sizeof(m_bankItem),str);
    //Compress((uint8*)m_bankItem,sizeof(m_bankItem),str);
}

void CUser::MakeBankItemList(CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    uint8 num = 0;
    uint16 pos = msg.GetDataLen();
    msg<<num;
    for(uint8 i = 0; i < MAX_BANK_ITEM_NUM; i++)
    {
        if(m_bankItem[i].tmplId != 0)
        {
            num++;
            msg<<i;
            MakeItemInfo(m_bankItem+i,msg);
            msg<<m_bankItem[i].num;
        }
    }
    msg.WriteData(pos,&num,1);    
}

bool CUser::NoLockAddBankItem(SItemInstance &item,uint8 &tolSaveNum)
{
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
    if(pItem == NULL)
        return false;
        
    uint8 savePos[MAX_BANK_ITEM_NUM] = {0};
    uint8 saveNum[MAX_BANK_ITEM_NUM] = {0};
    int tolNum = 0;
    uint8 pos = 0;
    uint8 maxBankNum = 18;
    uint8 openNum = NoLockGetExtData8(1);
    time_t endTime = NoLockGetExtData32(1);
    if((openNum > 0) && (endTime > GetSysTime()))
    {
        maxBankNum += openNum;
        if(maxBankNum > MAX_BANK_ITEM_NUM)
            maxBankNum = MAX_BANK_ITEM_NUM;
    }
    
    for(uint8 i = 0; i < maxBankNum; i++)
    {
        if(tolNum >= item.num)
            break;
        if(m_bankItem[i].num >= EBankItemDieJIaNum)
            continue;
        if(((pItem->type == EITAddAttr) || (pItem->type == EITPetBook))
            && (m_bankItem[i] == item))
        {
            savePos[pos] = i;
            saveNum[pos] = EBankItemDieJIaNum - m_bankItem[i].num;
            tolNum += saveNum[pos];
            pos++;
        }
        else if((m_bankItem[i].tmplId == item.tmplId) &&
            (m_bankItem[i] == item) &&
            ((pItem->type == EITPKYaoPin) || 
            (pItem->type == EITNormalYaoPin) ||
            (pItem->type == EITMission) ||
            (pItem->type == EITCanDelMiss) ||
            (pItem->type == EITMissionCanSave) ||
            ((pItem->type == EITChargeItem) && (pItem->addXue == 0) && (pItem->id != 1809)
            && (pItem->id != 1815) && ((pItem->id < 1827) || (pItem->id > 1831)))))
        {
            if((pItem->type == EITChargeItem) && !(m_bankItem[i] == item))
                continue;
            savePos[pos] = i;
            saveNum[pos] = EBankItemDieJIaNum - m_bankItem[i].num;
            tolNum += saveNum[pos];
            pos++;
        }
    }
    for(uint8 i = 0; i < maxBankNum; i++)
    {
        if(tolNum >= item.num)
            break;
        if(m_bankItem[i].tmplId == 0)
        {
            savePos[pos] = i;
            saveNum[pos] = EBankItemDieJIaNum;
            tolNum += saveNum[pos];
            pos++;
        }
    }
    if(tolNum <= 0)
        return false;
    //tolSaveNum = tolNum;
    tolSaveNum = 0;
    for(uint8 i = 0; i < pos; i++)
    {
        if(item.num <= 0)
            break;
        if(m_bankItem[savePos[i]].tmplId == item.tmplId)
        {
            if(item.num > saveNum[i])
            {
                m_bankItem[savePos[i]].num = EBankItemDieJIaNum;
                item.num -= saveNum[i];
                tolSaveNum += saveNum[i];
            }
            else
            {
                m_bankItem[savePos[i]].num += item.num;
                tolSaveNum += item.num;
                return true;
            }
        }
        else
        {
            m_bankItem[savePos[i]] = item;
            tolSaveNum += item.num;
            return true;
        }
    }
    return true;
}

void CUser::NoLockDelBankItem(uint8 bankPos)
{
    memset(m_bankItem+bankPos,0,sizeof(SItemInstance));
}

bool CUser::MoveItemToBank(uint8 packPos,uint8 num)
{
    if(num == 0)
        return false;
    if(packPos >= MAX_PACKAGE_NUM)
        return false;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_package[packPos].tmplId == 0)
        return false;
    if((num == 0) || (num > m_package[packPos].num))
        return false;
    SItemInstance item = m_package[packPos];
    item.num = num;
    if(NoLockAddBankItem(item,num))
    {
        NoLockDelPackage(packPos,num);
        return true;
    }
    return false;
}

bool CUser::MoveItemToPack(uint8 bankPos,uint8 num)
{
    if(num == 0)
        return false;
    if(bankPos >= MAX_BANK_ITEM_NUM)
        return false;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_bankItem[bankPos].tmplId == 0)
        return false;
    if(num > m_bankItem[bankPos].num)
        return false;
    
    bool success = false;
    for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if(num <= 0)
        {
            break;
        }
        SItemInstance item = m_bankItem[bankPos];
        if(num >= EItemDieJiaNum)
        {
            num -= EItemDieJiaNum;
            item.num = EItemDieJiaNum;
        }
        else
        {
            item.num = num;
            num = 0;
        }
        if(NoLockAddPackage(item))
        {
            success = true;
            if(m_bankItem[bankPos].num > item.num)
                m_bankItem[bankPos].num -= item.num;
            else
                memset(m_bankItem+bankPos,0,sizeof(SItemInstance));
        }
        else
        {
            break;
        }
    }
    return success;
}

void CUser::SetShop(char *pShop)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if((pShop == NULL) || (strlen(pShop) < sizeof(m_shopItem)))
    {
        memset(m_shopItem,0,sizeof(m_shopItem));
        return;
    }
    StrToHex(pShop,(uint8*)m_shopItem,sizeof(m_shopItem));
}

void CUser::GetShop(string &str)
{
    //boost::recursive_mutex::scoped_lock lk(m_mutex);
    HexToStr((uint8*)m_shopItem,sizeof(m_shopItem),str);
    //Compress((uint8*)m_shopItem,sizeof(m_shopItem),str);
}

void CUser::SetBitSet(char *pBitset)
{
    if((pBitset == NULL) || (strlen(pBitset) < MAX_BITSET/8))
        return;
    uint8 hex[MAX_BITSET/8];
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    StrToHex(pBitset,hex,MAX_BITSET/8);
    m_bitset.reset();
    HexToBitset(hex,m_bitset);
}

void CUser::GetBitSet(string &str)
{
    uint8 hex[MAX_BITSET/8] = {0};
    //boost::recursive_mutex::scoped_lock lk(m_mutex);
    BitsetToHex(m_bitset,hex);
    HexToStr(hex,MAX_BITSET/8,str);
}

void CUser::MakeOtherEquip(CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    int i = 0;
    for(; i < EQUIPMENT_NUM; i++)
    {
        msg<<m_equipment[i].tmplId;
    }
}
void CUser::MakeEquip(CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        //msg<<m_equipment[i].tmplId;
        if(m_equipment[i].tmplId == 0)
            msg<<(uint16)0;
        else
            MakeItemInfo(m_equipment+i,msg);
            //msg<<i<<m_equipment[i].tmplId<<m_equipment[i].num;
    }
}

void CUser::MakePack(SItemInstance &item,uint8 pos,CNetMessage &msg)
{
    if(pos >= MAX_PACKAGE_NUM2)
        return;
    msg<<pos<<item.tmplId<<item.num;
    MakeItemInfo(m_package+pos,msg);
}

void CUser::MakePack(CNetMessage &msg)
{
    uint16 pos = msg.GetDataLen();
    uint8 packNum = 0;
    msg<<packNum;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i < MAX_PACKAGE_NUM2; i++)
    {
        if(m_package[i].tmplId == 0)
        {
            continue;
        }
        //MakePack(m_package[i],i,msg);
        msg<<i<<m_package[i].tmplId<<m_package[i].num;
        packNum++;
    }
    msg.WriteData(pos,&packNum,sizeof(packNum));
}

bool CUser::MakeEquitInfo(uint8 pos,CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(pos >= EQUIPMENT_NUM)
        return false;
    
    return MakeItemInfo(m_equipment+pos,msg);
}

void CUser::Add1104ItemExp(int exp)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if(m_package[i].tmplId == 1005)
        {
            uint32 *pVal = (uint32*)m_package[i].addAttrVal;
            if(m_package[i].level == 0)
                m_package[i].level = 1;
            *pVal += exp;
            uint32 maxExp = GetLevelUpExp(m_package[i].level);
            if(*pVal >= maxExp)
            {
                *pVal -= maxExp;
                m_package[i].level++;
            }
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[i],i,msg);
            sock.SendMsg(m_sock,msg);
            return;
        }
    }
}

bool CUser::MakePackInfo(uint8 pos,CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(pos >= MAX_PACKAGE_NUM2)
        return false;

    return MakeItemInfo(m_package+pos,msg);
}

void CUser::AddLevel(bool callScript)         //等级
{
    //atomic_exchange_and_add((int*)&m_level,1);
    m_level++;
    AddDiscipleAward(this);
    
    /*if(HaveBitSet(189))
    {
        uint16 levelTongbao[] = {10,50,  
            20,100, 
            30,180, 
            40,300, 
            50,450, 
            60,750, 
            70,1220,
            80,2420,
            90,4520 
        };
        
        for(uint8 i = 0; i < sizeof(levelTongbao)/sizeof(uint16)/2; i++)
        {
            if(m_level == levelTongbao[2*i])
            {
                AddTongBao(levelTongbao[2*i+1]);
                char buf[128];
                sprintf(buf,"升级返通宝活动赠送你%d通宝",levelTongbao[2*i+1]);
                SendPopMsg(this,buf);
                SaveDate(this,0xffff,levelTongbao[2*i+1]);
                break;
            }
        }
    }*/
    
    
    AddTiZhi(1);        //体质
    AddLiLiang(1);    //力量
    AddMinJie(1);      //敏捷
    AddLingLi(1);      //灵力
    AddNaiLi(1);        //耐力
    AddShuXinDian(5);
    if(m_level % 2 == 0)
    {
        if(m_xiangxingdian+m_jin+m_mu+m_shui+m_huo+m_tu < m_level*2)
            AddXiangXinDian(1);
    }
    Init();
    UpdateInfo();
    AddHp(m_maxHp-m_hp);
    AddMp(m_maxMp-m_mp);
    /*CCallScript script("10000.lua");
    
    script.Call("Levelup","u",this);
    SetCallScript(10000);*/
    CCallScript *pScript = GetScript();
    if(callScript)
    {
        if(pScript != NULL)
        {
            SetCallScript(pScript->GetScriptId());
            pScript->Call("Levelup","u",this);
        }
        m_callLevelScript = false;
    }
    else
    {
        m_callLevelScript = true;
    }
    if(m_level == 10)
    {
        SendSysMail(this,"1.可到越亭郡拜师。2.神秘礼盒奖励丰厚。3.问天求道可得潜能,金钱,商城道具等。4.主支线双倍升级快。5.去武器防具店换装。");
    }
    else if(m_level == 20)
    {
        SendSysMail(this,"1.通过任务、商城得强化道具来强化武器。2.通过任务、商城得归元露洗宠物。3.每日任务可得潜能,金钱和商城道具等。4.可去越亭郡拜师。5.神秘礼盒奖励丰厚。");
    }
    else if(m_level == 30)
    {
        SendSysMail(this,"1.副本鬼蜮快速升级。2.PK同级玩家不获魔障,1000魔障蹲监狱。3.素材收集可得潜能,道行和金钱。4.打怪掉蓝水晶做装。5.每日任务得潜能,金钱和各类道具。6.任务或商城可得归元露，强化道具。7.神秘礼盒奖励丰厚。");
    }
    else if(m_level == 40)
    {
        SendSysMail(this,"1.可建帮派增人气。2.可参加40级擂台赛。3.打怪掉的材料收集后饰品工匠处可合成。4.可PK各级玩家.PK同级玩家不获魔障。5.任务或商城可得归元露，强化道具。6.问天求道可得潜能金钱和各类道具。7.每日任务得潜能,金钱和各类道具。8.可用主支线任务,双倍,鬼蜮快速升级。9.神秘礼盒奖励丰厚。");
    }
    else if(m_level == 50)
    {
        SendSysMail(this,"1.可收徒弟得奖励。2.可参加50就擂台赛。3.任务或商城的强化道具可强化武器防具。4.可PK各级玩家.PK同级玩家不获魔障。5.可用主支线任务,双倍,鬼蜮快速升级。6.每日任务可得潜能,金钱和各类道具。7.开神秘礼盒得奖多。8.问天求道可得潜能金钱和各类道具。");
    }
    else if(m_level == 60)
    {
        SendSysMail(this,"1.可参加高级擂台赛。2.开神秘礼盒得奖多。3.任务或商城的强化道具可强化武器防具。4.问天求道可得潜能,金钱和各类道具。5.每日任务，素材收集可得潜能道行金钱。6.可PK各级玩家.PK同级玩家不获魔障。7.神秘礼盒奖励丰厚。");
    }
}

void CUser::CallScriptLevelUp()
{
    if(m_callLevelScript)
    {
        CCallScript *pScript = GetScript();
        if(pScript != NULL)
        {
            SetCallScript(pScript->GetScriptId());
            pScript->Call("Levelup","u",this);
        }
    }
    m_callLevelScript = false;
}
//给予绑定物品
bool CUser::AddBangDingPackage(int itemId,int num,const char *name)
{
    SItemInstance item = {0};
    item.tmplId = itemId;
    item.num = num;
    item.bangDing = 1;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
    if(pItem == NULL)
        return false;
    item.naijiu = pItem->naijiu;
    return AddPackage(item,name);
}

bool CUser::AddAttrPackage(int itemId,int attr,int attrVal,bool bang)
{
    SItemInstance item = {0};
    item.tmplId = itemId;
    if(bang)
        item.bangDing = 1;
    else
        item.bangDing = 0;
    item.addAttrType[0] = attr;
    item.addAttrVal[0] = attrVal;
    item.addAttrNum = 1;
    item.quality = EQTBlue;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
    if(pItem == NULL)
        return false;
    item.naijiu = pItem->naijiu;
    return AddPackage(item);
}

//给予蓝水晶          属性      属性值
bool CUser::AddBlueCrystal(int attr,int attrVal)
{
    SItemInstance item = {0};
    item.tmplId = 615;
    item.bangDing = 1;
    item.addAttrType[0] = attr;
    item.addAttrVal[0] = attrVal;
    item.addAttrNum = 1;
    item.quality = EQTBlue;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
    if(pItem == NULL)
        return false;
    item.naijiu = pItem->naijiu;
    return AddPackage(item);
}
//给予炼化石
bool CUser::AddLianHuaStone(int level,int num)
{
    SItemInstance item = {0};
    item.tmplId = 611;
    item.bangDing = 1;
    item.num = num;
    item.level = level;
    return AddPackage(item);
}
//给予等级强化装备
bool CUser::AddLevelPackage(int itemId,int level)
{
    SItemInstance item = {0};
    item.tmplId = itemId;
    item.bangDing = 1;
    item.level = level;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
    if(pItem == NULL)
        return false;
    item.naijiu = pItem->naijiu;
    return AddPackage(item);
}

bool CUser::AddPackage(SItemInstance &item,const char *name)
{
    if(name != NULL)
    {
        snprintf(item.name,sizeof(item.name),"%s",name);
    }
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
    if(pItem == NULL)
        return false;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(pItem->addXue != 0)
    {
        int *p = (int*)item.addAttrVal;
        if(*p == 0)
            *p = pItem->addXue;
    }

    return NoLockAddPackage(item);
}

bool CUser::CanDelPackage(uint8 pos)
{
    if(pos >= MAX_PACKAGE_NUM2) 
        return false;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    if(m_package[pos].tmplId == 0)
        return false;
        
    SItemTemplate *pItem = itemMgr.GetItem(m_package[pos].tmplId);
    if(pItem == NULL)
        return false;
    
    if((pItem->type == EITTeShu) || (pItem->type == EITMission) || (pItem->type == EITNvWaShi))
    {
        return false;
    }
    return true;
}

bool CUser::DelPackage(uint8 pos,uint8 num)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    return NoLockDelPackage(pos,num);
}

bool CUser::AddPackage(int itemId,uint8 num,const char *name)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SItemInstance item = {0};
    item.tmplId = itemId;
    item.num = num;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(itemId);
    if(pItem == NULL)
        return false;
    item.naijiu = pItem->naijiu;
    if(pItem->addXue != 0)
    {
        int *p = (int*)item.addAttrVal;
        *p = pItem->addXue;
    }
    if(name != NULL)
    {
        snprintf(item.name,sizeof(item.name),"%s",name);
    }
    if((itemId == 640) || (itemId == 641) || (itemId == 642))
    {
        item.level = 1;
    }
    return NoLockAddPackage(item,NULL,name);
    //return NoLockAddPackage(itemId,num,name);
}

uint8 CUser::GetExtData8(uint16 pos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    return NoLockGetExtData8(pos);
}
void  CUser::SetExtData8(uint16 pos,uint8 val)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    NoLockSetExtData8(pos,val);
}
      
uint16 CUser::GetExtData16(uint16 pos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    return NoLockGetExtData16(pos);
}
void  CUser::SetExtData16(uint16 pos,uint16 val)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    NoLockSetExtData16(pos,val);
}
      
uint32 CUser::GetExtData32(uint16 pos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    return NoLockGetExtData32(pos);
}

void  CUser::SetExtData32(uint16 pos,uint32 val)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    NoLockSetExtData32(pos,val);
}
      
uint8 CUser::NoLockGetExtData8(uint16 pos)
{
    map<uint16,uint8>::iterator i = m_saveData8.find(pos);
    if(i != m_saveData8.end())
        return i->second;
    return 0;
}

void  CUser::NoLockSetExtData8(uint16 pos,uint8 val)
{
    m_saveData8[pos] = val;
}
      
uint16 CUser::NoLockGetExtData16(uint16 pos)
{
    map<uint16,uint16>::iterator i = m_saveData16.find(pos);
    if(i != m_saveData16.end())
        return i->second;
    return 0;
}

void  CUser::NoLockSetExtData16(uint16 pos,uint16 val)
{
    m_saveData16[pos] = val;
}
      
uint32 CUser::NoLockGetExtData32(uint16 pos)
{
    map<uint16,uint32>::iterator i = m_saveData32.find(pos);
    if(i != m_saveData32.end())
        return i->second;
    return 0;
}

void  CUser::NoLockSetExtData32(uint16 pos,uint32 val)
{
    m_saveData32[pos] = val;
}

uint8 CUser::GetCurMaxPackNum()
{
    uint8 curMaxPackNum = 36+m_openPack*18;
    uint8 open4Num = NoLockGetExtData8(0);
    time_t endTime = NoLockGetExtData32(0);
    if((open4Num > 0) && (endTime > GetSysTime()))
    {
        curMaxPackNum += open4Num;
        if(curMaxPackNum > MAX_PACKAGE_NUM2)
            curMaxPackNum = MAX_PACKAGE_NUM2;
    }
    return curMaxPackNum;
}

bool CUser::NoLockAddPackage(SItemInstance &item,uint8 *pPos,bool trade,int *pAddQinMi)
{
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
    if(pItem == NULL)
        return false;
    if(trade && ((pItem->mod & EIMTJiaoYiBang) != 0))
    {
        item.bangDing = 1;
        if(pAddQinMi != NULL)
            *pAddQinMi = pItem->addXue;
    }
    
    if(item.num < 1)
        item.num = 1;
    if((pItem->type == EITAddAttr) && (pItem->addXue != 0))
    {
        item.bangDing = 1;
    }
    if((item.tmplId == 1837) || (item.tmplId == 1838))
        item.bangDing = 1;
    
    uint8 curMaxPackNum = GetCurMaxPackNum();
    
    if(pItem->type == EITAddAttr)
    {
        for(int i = 0; i < curMaxPackNum; i++)
        {
            if((m_package[i] == item) && (m_package[i].num + item.num <= EItemDieJiaNum))
            {
                m_package[i].num += item.num;
                CSocketServer &sock = SingletonSocket::instance();
                CNetMessage msg;
                msg.SetType(PRO_UPDATE_PACK);
                MakePack(m_package[i],i,msg);
                sock.SendMsg(m_sock,msg);
                
                if(pPos != NULL)
                    *pPos = i;
                return true;
            }
        }
    }
    uint16 itemId = item.tmplId;
    uint8 num = item.num;
    if((pItem->type == EITPKYaoPin) || 
        (pItem->type == EITNormalYaoPin) ||
        (pItem->type == EITMission) ||
        (pItem->type == EITCanDelMiss) ||
        (pItem->type == EITMissionCanSave) ||
        (pItem->type == EITPetBook) ||
        ((pItem->type == EITChargeItem) && (pItem->addXue == 0) && (pItem->id != 1809)
            && (pItem->id != 1815)  && ((pItem->id < 1827) || (pItem->id > 1831))))
    {
        for(int i = 0; i < curMaxPackNum; i++)
        {
            if((m_package[i] == item) && (m_package[i].num + num <= EItemDieJiaNum))
            {
                m_package[i].tmplId = itemId;
                m_package[i].naijiu = pItem->naijiu;
                m_package[i].num += num;
                CSocketServer &sock = SingletonSocket::instance();
                CNetMessage msg;
                msg.SetType(PRO_UPDATE_PACK);
                MakePack(m_package[i],i,msg);
                sock.SendMsg(m_sock,msg);
                
                if(pPos != NULL)
                    *pPos = i;
                return true;
            }
        }
    }
    
    for(int i = 0; i < curMaxPackNum; i++)
    {
        if(m_package[i].tmplId == 0)
        {
            m_package[i] = item;
            //m_package[i].naijiu = pItem->naijiu;
            if(m_package[i].num <= 0)
                m_package[i].num = 1;
            if(pItem->type == EITAddAttr)
            {
                if(pItem->addXue != 0)
                    m_package[i].bangDing = 1;
            }
            /*else if((pItem->addXue != 0))
            {
                int *p = (int*)m_package[i].addAttrVal;
                *p = pItem->addXue;
            }*/
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[i],i,msg);
            sock.SendMsg(m_sock,msg);
            
            if(pPos != NULL)
                *pPos = i;
            return true;
        }
    }
    SendSysInfo(this,"包裹已满,请尽快整理!");
    return false;
}

bool CUser::NoLockDelPackage(uint8 pos,uint8 num)
{
    if(pos < MAX_PACKAGE_NUM2)
    {
        if(m_package[pos].tmplId != 0)// && (m_package[pos].num >= num))
        {
            if(m_package[pos].num > num)
            {
                m_package[pos].num -= num;
            }
            else
            {
                memset(m_package + pos,0,sizeof(SItemInstance));
            }
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[pos],pos,msg);
            sock.SendMsg(m_sock,msg);
            
            return true;
        }
    }
    return false;
}

int CUser::JianDingItem(uint8 pos)
{
    if(pos >= MAX_PACKAGE_NUM)
        return -1;
    if(m_package[pos].tmplId != 615)
        return -1;
    if(m_package[pos].addAttrNum > 0)
        return -1;

    boost::recursive_mutex::scoped_lock lk(m_mutex);
            
    uint16 min,max,middle;
    uint8 type = BLUE_EQUIP[Random(0,sizeof(BLUE_EQUIP)-1)];
    GetAddAttrVal(type,min,middle,max);
    
    SItemInstance item = {0};
    item.num = 1;
    item.tmplId = 615;
    item.addAttrType[0] = type;              
    item.addAttrVal[0] = Random(min,middle); 
    item.addAttrNum = 1;  
    item.bangDing = m_package[pos].bangDing;
    uint8 addPos = 0;                   
    if(NoLockAddPackage(item,&addPos))
    {
        NoLockDelPackage(pos);
        return addPos;
    }
    
    /*m_package[pos].addAttrType[0] = type;
    m_package[pos].addAttrVal[0] = Random(min,middle);
    m_package[pos].addAttrNum = 1;*/
    
    /*CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PACK);
    MakePack(m_package[pos],pos,msg);
    sock.SendMsg(m_sock,msg);*/
    return -1;
}

uint8 CUser::GetWuQiType()
{
    uint16 id = m_equipment[EETWuQi].tmplId;
    if(id == 0)
        return 0;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(id);
    if(pItem != NULL)
    {
        return pItem->type;
    }
    return 0;
}

bool CanAddShuXing(SItemTemplate *pItem,SItemInstance *pInst)
{
    if((pItem->type <= EITXieZi) || (pItem->type == EITPetKaiJia))
    {
        if(pInst->naijiu > 0)
            return true;
    }
    else if(pItem->type <= EITShouZhuo)
    {
        return true;
    }
    return false;
}

//pos包裹中的位置，返回装备中的位置，< 0表示装备失败
int CUser::EquipItem(uint8 pos,string &errMsg)
{
    if(pos >= MAX_PACKAGE_NUM)
        return -1;
    
    uint8 equipPos = 0;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(m_package[pos].tmplId);
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    /*************
    1.武器要看相性
    2.衣服,帽子有性别
    3.等级是否够
    4.装备限制是否满足
    **************/
    if(pItem == NULL)
    {
        errMsg = "非法装备";
        return -1;
    }
    if((pItem->type <= EITGun) && (pItem->type != m_xiang))
    {
        errMsg = "相性不符，无法装备";
        return -1;
    }
    if(((pItem->type == EITYiFu) || (pItem->type == EITMaoZi)) && (pItem->sex != m_sex))
    {
        errMsg = "性别不符，无法装备";
        return -1;
    }
    if(pItem->level > m_level)
    {
        errMsg = "等级不够，无法装备";
        return -1;
    }
    uint16 val = m_package[pos].reqAttrVal;
    uint16 tizhi = GetTiZhi();
    uint16 liliang = GetLiLiang();
    uint16 minjie = GetMinJie();
    uint16 lingli = GetLingLi();
    uint16 naili = GetNaiLi();
    switch(m_package[pos].reqAttrType)
    {
        case ERATtizhi:
            if(tizhi < val)
            {
                errMsg = "体质不够，无法装备，请找冶炼大师转换装备限制";
                return -1;
            }
            break;
        case ERATliliang:
            if(liliang < val)
            {
                errMsg = "力量不够，无法装备，请找冶炼大师转换装备限制";
                return -1;
            }
            break;
        case ERATminjie:
            if(minjie < val)
            {
                errMsg = "敏捷不够，无法装备，请找冶炼大师转换装备限制";
                return -1;
            }
            break;
        case ERATlingli:
            if(lingli < val)
            {
                errMsg = "灵力不够，无法装备，请找冶炼大师转换装备限制";
                return -1;
            }
            break;
        case ERATnaili:
            if(naili < val)
            {
                errMsg = "耐力不够，无法装备，请找冶炼大师转换装备限制";
                return -1;
            }
            break;
    }
    switch(pItem->type)
    {
    case EITQiang://枪
    case EITZhua://爪
    case EITJian://剑
    case EITDao://刀
    case EITGun://棍
        equipPos = EETWuQi;
        break;
    
    case EITYiFu://衣服
        equipPos = EETKuiJia;
        break;
    case EITMaoZi://帽子
        equipPos = EETMaoZi;
        break;
    case EITYaoDai://腰带
        equipPos = EETYaoDai;
        break;
    case EITXieZi://鞋子
        equipPos = EETXieZi;
        break;
    case EITXiangLian://项链
        equipPos = EETXiangLian;
        break;
    case EITYuPei://玉佩
        equipPos = EETYuPei;
        break;
    case EITShouZhuo://手镯
        if(m_equipment[EETShouZhuo1].tmplId == 0)
            equipPos = EETShouZhuo1;
        else
            equipPos = EETShouZhuo2;
        break;
    default:
        errMsg = "此类型物品不能装备";
        return -1;
    }
    
    bool swapEquipPack = true;
    if(m_equipment[equipPos].tmplId != 0)
    {
        //SItemTemplate *pEquip = itemMgr.GetItem(m_equipment[equipPos].tmplId);
        //if((pEquip != NULL) && CanAddShuXing(pEquip,m_equipment + equipPos))
            //EquipAddAttr(m_equipment[equipPos],false);
        if(NoLockAddPackage(m_equipment[equipPos]))
        {
            m_equipment[equipPos] = m_package[pos];
            NoLockDelPackage(pos);
            swapEquipPack = false;
        }
    }
    if(swapEquipPack)
    {
        //std::swap(m_equipment[equipPos],m_package[pos]);
        SItemInstance item = m_package[pos];
        //NoLockDelPackage(pos);
        m_package[pos] = m_equipment[equipPos];
        UpdatePackage(pos);
        m_equipment[equipPos] = item;
    }
    //EquipmentAddAttr(true);
    
    /*if(CanAddShuXing(pItem,m_equipment + equipPos))
        EquipAddAttr(m_equipment[equipPos],true);*/
    UpdateEquip(equipPos);
    Init();
    UpdateInfo();
    UpdateSkill();
    return equipPos;
}
//pos装备中的位置，返回包裹中的位置，< 0表示卸装备失败
int CUser::UnEquipItem(uint8 pos)
{
    if(pos >= EQUIPMENT_NUM)
        return -1;
        
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(m_equipment[pos].tmplId);
    if(pItem == NULL)
        return -1;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    uint8 i = 0;
    for(; i < MAX_PACKAGE_NUM; i++)
    {
        if(m_package[i].tmplId == 0)
        {
            //EquipmentAddAttr(false);
            /*if(CanAddShuXing(pItem,m_equipment + pos))
                EquipAddAttr(m_equipment[pos],false);*/
            std::swap(m_equipment[pos],m_package[i]);
            Init();
            UpdateInfo();
            UpdateSkill();
            return i;
        }
    }
    
    return -1;
}

int CUser::GetRecovery()//防御
{
    return m_recovery + GetQiPetFangYu();
}

int CUser::GetItemfangYu()//防御
{
    int temp = 0;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem;
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        pItem = itemMgr.GetItem(m_equipment[i].tmplId);
        //ifCanAddShuXing
        if((pItem != NULL) && (m_equipment[i].tmplId != 0) && (pItem->type <= EITShouZhuo))
        {
            
            if((pItem->fangYu > 0) && (m_equipment[i].level > 0))
            {
                if(m_equipment[i].naijiu > 0)
                    temp += (int)(pItem->fangYu * GetQiangHuaBeiLv(m_equipment[i].level-1));
            }
            else if(pItem->type >= EITXiangLian)
            {
                temp += pItem->fangYu;
                temp += CalculateRate((int)pItem->fangYu,(int)m_equipment[i].naijiu,100);
            }
            else if(m_equipment[i].naijiu > 0)
            {
                temp += pItem->fangYu;
            }
            if(CanAddShuXing(pItem,m_equipment+i))//(m_equipment[i].naijiu > 0)
            {
                for(uint8 j = 0; j < m_equipment[i].addAttrNum; j++)
                {
                    if(m_equipment[i].addAttrType[j] == EAAfangyu)
                        temp += m_equipment[i].addAttrVal[j];
                }
            }
        }
    }
    //cout<<temp<<endl;
    return temp;
}

/*271
属性加成：木系攻击力降低20%，水系攻击力提升20%。
275
属性加成：土系攻击力降低20%，火系攻击力提升20%。
272
属性加成：火系攻击力降低20%，木系攻击力提升20%。
274
属性加成：金系攻击力降低20%，土系攻击力提升20%。
273
属性加成：水系攻击力降低20%，金系攻击力提升20%。*/
static void WWJiaCheng(CUser *pUser,int &damage)
{
    uint16 mapId = 0;
    CScene *pScene = pUser->GetScene();
    if(pScene == NULL)
        return;
    mapId = pScene->GetMapId();
    if(mapId == 271)
    {
        if(pUser->GetXiang() == EXMuXiang)
            damage -= damage*20/100;
        else if(pUser->GetXiang() == EXMuXiang)
            damage += damage*20/100;
    }
    else if(mapId == 275)
    {
        if(pUser->GetXiang() == EXTuXiang)
            damage -= damage*20/100;
        else if(pUser->GetXiang() == EXHuoXiang)
            damage += damage*20/100;
    }
    else if(mapId == 272)
    {
        if(pUser->GetXiang() == EXHuoXiang)
            damage -= damage*20/100;
        else if(pUser->GetXiang() == EXMuXiang)
            damage += damage*20/100;
    }
    else if(mapId == 274)
    {
        if(pUser->GetXiang() == EXJinXiang)
            damage -= damage*20/100;
        else if(pUser->GetXiang() == EXTuXiang)
            damage += damage*20/100;
    }
    else if(mapId == 273)
    {
        if(pUser->GetXiang() == EXShuiXiang)
            damage -= damage*20/100;
        else if(pUser->GetXiang() == EXJinXiang)
            damage += damage*20/100;
    }
}

int CUser::GetDamage()//物理伤害
{
    WWJiaCheng(this,m_damage);
    return m_damage;
}
int CUser::GetItemGongji()//攻击力
{
    int temp = 0;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem;
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        pItem = itemMgr.GetItem(m_equipment[i].tmplId);
        if((pItem != NULL) && (m_equipment[i].tmplId != 0) && (pItem->type <= EITShouZhuo))
        {
            if((pItem->gongji > 0) && (m_equipment[i].level > 0))
            {
                if(m_equipment[i].naijiu > 0)
                    temp += (int)(pItem->gongji * GetQiangHuaBeiLv(m_equipment[i].level-1));
            }
            else if(pItem->type >= EITXiangLian)
            {
                temp += pItem->gongji;
                temp += CalculateRate((int)pItem->gongji,(int)m_equipment[i].naijiu,100);
            }
            else if(m_equipment[i].naijiu > 0)
            {
                temp += pItem->gongji;
            }
            if(CanAddShuXing(pItem,m_equipment+i))
            {
                for(uint8 j = 0; j < m_equipment[i].addAttrNum; j++)
                {
                    if(m_equipment[i].addAttrType[j] == EAAshanghai)
                        temp += m_equipment[i].addAttrVal[j];
                }
            }
        }
    }
    return temp;
}

int CUser::GetMp()
{
    return m_mp;
}
int CUser::GetItemFaLi()//回蓝
{
    int temp = 0;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem;
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        pItem = itemMgr.GetItem(m_equipment[i].tmplId);
        if((pItem != NULL) && (m_equipment[i].tmplId != 0) && (pItem->type <= EITShouZhuo))
        {
            if(pItem->type >= EITXiangLian)
            {
                temp += pItem->addFaLi;
                temp += CalculateRate((int)pItem->addFaLi,(int)m_equipment[i].naijiu,100);
            }
            if(CanAddShuXing(pItem,m_equipment+i))
            {
                temp += pItem->addFaLi;
                for(uint8 j = 0; j < m_equipment[i].addAttrNum; j++)
                {
                    if(m_equipment[i].addAttrType[j] == EAAfali)
                        temp += m_equipment[i].addAttrVal[j];
                }
            }
        }
    }
    return temp;
}

int CUser::GetMaxMp()
{
    return m_maxMp;// + GetQiPetMp();
}
int CUser::GetMaxHp()
{
    return m_maxHp;// + GetQiPetHp();
}
int CUser::GetItemQiXue()//增加气血
{
    int temp = 0;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem;
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        if(m_equipment[i].tmplId == 0)
            continue;
        pItem = itemMgr.GetItem(m_equipment[i].tmplId);
        
        if(((pItem != NULL)) && (pItem->type <= EITShouZhuo))
        {
            if(pItem->type >= EITXiangLian)
            {
                temp += pItem->addQiXue;
                temp += CalculateRate((int)pItem->addQiXue,(int)m_equipment[i].naijiu,100);
            }
            if(CanAddShuXing(pItem,m_equipment+i))
            {
                temp += pItem->addQiXue;
                for(uint8 j = 0; j < m_equipment[i].addAttrNum; j++)
                {
                    if(m_equipment[i].addAttrType[j] == EAAqixue)
                        temp += m_equipment[i].addAttrVal[j];
                }
            }
        }
    }
    return temp;
}

int CUser::GetSpeed()//速度
{
    return m_speed + GetQiPetSpeed();
}

int CUser::GetItemSudu()//增加速度
{
    int temp = 0;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem;
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        pItem = itemMgr.GetItem(m_equipment[i].tmplId);
        if((pItem != NULL) && (m_equipment[i].tmplId != 0) && (pItem->type<= EITShouZhuo))
        {
            if(pItem->type >= EITXiangLian)
            {
                temp += pItem->addSudu;
                temp += CalculateRate((int)pItem->addSudu,(int)m_equipment[i].naijiu,100);
            }
            if(CanAddShuXing(pItem,m_equipment+i))
            {
                temp += pItem->addSudu;
                for(uint8 j = 0; j < m_equipment[i].addAttrNum; j++)
                {
                    if(m_equipment[i].addAttrType[j] == EAAsudu)
                        temp += m_equipment[i].addAttrVal[j];
                }
            }
        }
    }
    return temp;
}

int CUser::GetItemAddAttr(EAddAttrType type)
{
    int temp = 0;
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        if(((m_equipment[i].tmplId != 0) && (m_equipment[i].naijiu > 0))
            || ((m_equipment[i].tmplId >= 401) && (m_equipment[i].tmplId < 601)))
        {
            for(uint8 j = 0; j < SItemInstance::MAX_ADD_ATTR_NUM; j++)
            {
                /*if(type == EAAFashufantanlv)
                {
                    cout<<"fa shu fan tan:"<<m_equipment[i].addAttrVal[j]<<endl;
                }*/
                if(m_equipment[i].addAttrType[j] == type)
                {
                    temp += m_equipment[i].addAttrVal[j];
                }
            }
        }
    }
    return temp;
}

int CUser::GetItemLiliang()//力量(固定值)
{
    return GetItemAddAttr(EAAliliang) + GetItemAddAttr(EAAquanshuxing);
}

int CUser::GetItemLingli()//灵性(固定值)
{
    return GetItemAddAttr(EAAlingxing) + GetItemAddAttr(EAAquanshuxing);
}

int CUser::GetItemMinjie()//敏捷(固定值)
{
    return GetItemAddAttr(EAAminjie) + GetItemAddAttr(EAAquanshuxing);
}

int CUser::GetItemTizhi()//体质(固定值)
{
    return GetItemAddAttr(EAAtizhi) + GetItemAddAttr(EAAquanshuxing);
}

int CUser::GetItemNaili()//耐力(固定值)	
{
    return GetItemAddAttr(EAAnaili) + GetItemAddAttr(EAAquanshuxing);
}
    
int CUser::GetItemJin()//金相
{
    return GetItemAddAttr(EAAjin) + GetItemAddAttr(EAAquanxiangxing);
}

int CUser::GetItemMu()//木相
{
    return GetItemAddAttr(EAAmu) + GetItemAddAttr(EAAquanxiangxing);
}

int CUser::GetItemShui()//水相
{
    return GetItemAddAttr(EAAshui) + GetItemAddAttr(EAAquanxiangxing);
}

int CUser::GetItemHuo()//火相
{
    return GetItemAddAttr(EAAhuo) + GetItemAddAttr(EAAquanxiangxing);
}

int CUser::GetItemTu()//土相	
{
    return GetItemAddAttr(EAAtu) + GetItemAddAttr(EAAquanxiangxing);
}
    
int CUser::GetItemMingzhong()//命中率
{
    //当门派为“土”时，命中数提升=(力量-等级)*0.75+(体质-等级)*0.75+金相*4+水相*2+45+蓝属性附加值
    //其他门派时，命中数提升=(力量-等级)*1.5+(体质-等级)*1.5+金相*8+水相*4+60+蓝属性附加值
    /*int add = 0;
    if(m_xiang == EXTuXiang)
        add = (int)(m_level*3+(m_liliang-m_level)*0.75+(m_tizhi-m_level)*0.75+m_jin*4+m_shui*2+45);
    else
        add = (int)(m_level*4+(m_liliang-m_level)*1.5+(m_tizhi-m_level)*1.5+m_jin*8+m_shui*4+60);*/
    return GetItemAddAttr(EAAmingzhong);
}

int CUser::GetItemHuibi()//回避物理攻击
{
    /*int add = 0;
    //当门派为“木”时，闪避数提升=(敏捷-等级)*2+(耐力-等级)*1+火相*4+木相*7+100+蓝属性附加值
    //当门派为“金”和“水”时，闪避数提升=(敏捷-等级)*0.5+(耐力-等级)*0.25+火相*1+木相*3+30+蓝属性附加值
    //其他门派时，闪避数提升=(敏捷-等级)*1+(耐力-等级)*0.5+火相*2.5+木相*5.5+60+蓝属性附加值
    if(m_xiang == EXMuXiang)
    {
        add = int(m_level*8+(m_minjie-m_level)*2+(m_naili-m_level)+m_huo*7+100);
    }
    else if((m_xiang == EXJinXiang) || (m_xiang == EXShuiXiang))
    {
        add = int(m_level*2+(m_minjie-m_level)*0.5+(m_naili-m_level)*0.25+m_huo+30);
    }
    else
    {
        add = int(m_level*4+(m_minjie-m_level)+(m_naili-m_level)*0.5+m_huo*5.5+60);
    }*/
    return GetItemAddAttr(EAAhuibiwuli);
}

//连击率=力量*0.00006+土相*0.0007+土系附加（土系天生1%）+装备附加
int CUser::GetItemLianjiLv()//连击率
{
    int liliang = GetLiLiang();
    int tu = GetTu();
    int lianji = (int)((liliang*0.006+tu*0.07)*5) + GetItemAddAttr(EAAlianjilv);
    if(m_xiang == EXTuXiang)
    {
        lianji += 5;
    }
    else if(m_xiang == EXMuXiang)
    {
        //耐力*0.00006+土相*0.0007+装备附加，状态显示中为 实际值*500
        int naili = GetNaiLi();
        lianji = (int)((naili*0.006+tu*0.07)*5+GetItemAddAttr(EAAlianjilv));
    }
    return lianji;
}

int CUser::GetItemLianjiShu()//连击数
{
    return 2+GetItemAddAttr(EAAlianjishu);
}

//物理爆击率=耐力*0.00006+木相*0.0007+木系附加（木系天生1%）+装备附加
int CUser::GetItemBaoJiLv()//必杀率
{
    int naili = GetNaiLi();
    int mu = GetMu();
    int temp = (int)((naili*0.006+mu*0.07)*5) + GetItemAddAttr(EAAbishalv);
    if(m_xiang == EXMuXiang)
    {
        int liliang = GetLiLiang();
        //力量*0.00006+木相*0.0007+木系附加（木系天生1%）+装备附加，状态显示中为 实际值*500
        temp = (int)((liliang*0.006+mu*0.07)*5+GetItemAddAttr(EAAbishalv)+5);
    }
    //if(temp > 100)
        //temp = 100;
    return temp;
}

//反击率=体质*0.00006+水相*0.0007+水系附加（水系天生1%）+装备附加
int CUser::GetItemFanJiLv()//反击率
{
    int tizhi = GetTiZhi();
    int shui = GetShui();
    int temp = (int)((tizhi*0.006+shui*0.07)*5)+GetItemAddAttr(EAAfanjilv);
    if(m_xiang == EXShuiXiang)
        temp += 5;
    //if(temp > 100)
        //temp = 100;
    return temp;
}
//反震率=敏捷*0.00006+火相*0.0007+火系附加（火系天生1%）+装备附加
int CUser::GetItemFanZhenLv()//反震率
{
    int minjie = GetMinJie();
    int huo = GetHuo();
    int temp = (int)((minjie*0.006+huo*0.07)*5)+GetItemAddAttr(EAAfanzhenlv);
    if(m_xiang == EXHuoXiang)
        temp += 5;
    //if(temp > 100)
        //temp = 100;
    return temp;
}

int CUser::GetItemGongjiMpJiangDi()//攻击技能MP消耗降低
{
    return GetItemAddAttr(EAAgongjijiangdimp) + GetItemAddAttr(EAAquanjiangdimp);
}

int CUser::GetItemFuZhuMpJiangDi()//辅助技能MP消耗降低
{
    return GetItemAddAttr(EAAfuzhujiangdimp) + GetItemAddAttr(EAAquanjiangdimp);
}

int CUser::GetItemZhangAiMpJiangDi()//障碍技能MP消耗降低
{
    return GetItemAddAttr(EAAzhangaijiangdimp) + GetItemAddAttr(EAAquanjiangdimp);
}

    
int CUser::GetItemHuiBiFaGong()//回避法术攻击
{
    return GetItemAddAttr(EAAhuibifashugongji);
}
/*
160	辅主耐力强化
161	辅主体质强化
162	辅主敏捷强化
163	辅主提升防御
164	辅主提升法力
165	辅主提升气血
166	辅主提升速度
167	辅主金法抵挡
168	辅主木法抵挡
169	辅主水法抵挡
170	辅主火法抵挡
171	辅主土法抵挡
172	辅主物理抵挡
*/

int CUser::GetQiPetNaiLi() //1.5
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(160)*5);
}
int CUser::GetQiPetTiZhi() //1.5
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(161)*5);
}
int CUser::GetQiPetMinJie()//1.5
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(162)*5);
}
int CUser::GetQiPetFangYu()//9  
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(163)*30);
}
int CUser::GetQiPetMp()    //15 
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(164)*50);
}
int CUser::GetQiPetHp()    //15 
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(165)*50);
}
int CUser::GetQiPetSpeed() //1.5
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(166)*5);
}
int CUser::GetQiPetKangJin()//9
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(167)*30);
}
int CUser::GetQiPetKangMu()//9
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(168)*30);
}
int CUser::GetQiPetKangShui()//9
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(169)*30);
}
int CUser::GetQiPetKangHuo()//9
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(170)*30);
}
int CUser::GetQiPetKangTu()//9
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(171)*30);
}
int CUser::GetQiPetKangWuLi()//9
{
    if(m_qiPet >= m_petNum)
        return 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[m_qiPet];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 0;
    return (int)(pPet->GetSkillLevel(172)*30);
}

    //攻击伤害减少
int CUser::GetItemKangJin()//金抗性
{
    return GetItemAddAttr(EAAjinkang) + GetItemAddAttr(EAAquankang)
        + GetQiPetKangJin();
}

int CUser::GetItemKangMu()//木抗性
{
    return GetItemAddAttr(EAAmukang) + GetItemAddAttr(EAAquankang)
        + GetQiPetKangMu();
}

int CUser::GetItemKangShui()//水抗性
{
    return GetItemAddAttr(EAAshuikang) + GetItemAddAttr(EAAquankang)
        + GetQiPetKangShui();
}

int CUser::GetItemKangHuo()//火抗性
{
    return GetItemAddAttr(EAAhuokang) + GetItemAddAttr(EAAquankang)
        + GetQiPetKangHuo();
}

int CUser::GetItemKangTu()//土抗性
{
    return GetItemAddAttr(EAAtukang) + GetItemAddAttr(EAAquankang)
        + GetQiPetKangTu();
}
    
    //攻击命率中降低
int CUser::GetItemKangYiWang()//抗遗忘
{
    return GetItemAddAttr(EAAkangyiwang) + GetItemAddAttr(EAAquankang);
}

int CUser::GetItemKangZhongDu()//抗中毒
{
    return GetItemAddAttr(EAAkangzhongdu) + GetItemAddAttr(EAAquankang);
}

int CUser::GetItemKangBingDong()//抗冰冻
{
    return GetItemAddAttr(EAAkangbingdong) + GetItemAddAttr(EAAquankang);
}

int CUser::GetItemKangHunShui()//抗昏睡
{
    return GetItemAddAttr(EAAkanghuishui) + GetItemAddAttr(EAAquankang);
}

int CUser::GetItemKangHunLuan()//抗混乱
{
    return GetItemAddAttr(EAAkanghunluan) + GetItemAddAttr(EAAquankang);
}

int CUser::GetItemJiNeng()//全技能
{
    //return GetItemAddAttr(EAAquanjineng);
    return 0;
}

int CUser::GetItemHuShiAllKang()//忽视所有抗性
{
    return GetItemAddAttr(EAAhushikangxing);
}

int CUser::GetItemHuShiAllKangYiChang()//忽视所有抗异常	
{
    return GetItemAddAttr(EAAhushikangyichang);
}

int CUser::GetItemBaoJiZhuiJIa()
{
    return GetItemAddAttr(EAAbaojizhuijia);//爆击时追加伤害	
}

int CUser::GetAddBaoJiWeiLi()
{
	return GetItemAddAttr(EAAaddbaojiweili);//增加爆击威力	
}

int CUser::GetLianJiAddShangHai()
{
	return GetItemAddAttr(EAAlianjizhuijia);//连击时追加伤害	
}
int CUser::GetFanJiAddShangHai()
{
	return GetItemAddAttr(EAAfanjizhuijia);//反击时追加伤害	
}
int CUser::GetFanZhenDu()
{
    return 10+GetItemAddAttr(EAAfanzhendu);//反震度	
}

//法术爆击率=灵性*0.00006+金相*0.0007+金系附加（金系天生1%）+装备附加
int CUser::GetFaShuBaoJi()
{
    int lingli = GetLingLi();
    int jin = GetJin();
    int temp = (int)((lingli*0.006+jin*0.07)*5)+GetItemAddAttr(EAAfashubaoji);//法术爆击	
    if(m_xiang == EXJinXiang)
        temp += 5;
    return temp;
}
int CUser::GetFaShuBaoJiAdd()
{
	return GetItemAddAttr(EAAfashubaojizhuijia);//法术爆击追加	
}

int CUser::GetFaShuFanTan()
{
	return 10+GetItemAddAttr(EAAfashufantan);//法术反弹
}

int CUser::GetFaShuFanTanLv()
{
    int fafan = GetItemAddAttr(EAAFashufantanlv);//法术反弹率;
    //cout<<"法术反弹:"<<fafan<<endl;
    return fafan;
}

void CUser::MakeUpdateInfo(CNetMessage &msg,CUser *pUser)//int bId,int xzBId)
{
    int bId = pUser->GetBangPai();
    int xzBId = pUser->GetXZBangId();
    
    bool haveKuang = false;
    if(HaveItem(1543) || HaveItem(1544))
        haveKuang = true;
    uint8 state = 0;
    
    //uint16 mapId = GetMapId();
    //if((mapId >= 270) && (mapId <= 276))
    {
        if(HaveItem(1558) || 
           HaveItem(1559) || 
           HaveItem(1560) || 
           HaveItem(1561) || 
           HaveItem(1562))
        haveKuang = true;
    }
    
    if(xzBId != 0)
    {
        if((bId == (int)m_XZBangId) || (xzBId == (int)m_bangpai))
            state |= 4;
    }
    else
    {
        CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
        CBangPai *pBangPai1 = NULL;
        CBangPai *pBangPai2 = NULL;
        if(bId != 0)
            pBangPai1 = bPMgr.FindBangPai(bId);
        if(m_bangpai != 0)
            pBangPai2 = bPMgr.FindBangPai(m_bangpai);
        
        if((pBangPai1 != NULL) && (pBangPai2 != NULL))
        {
            if((pBangPai1->GetXuanZhanBang() == (int)pBangPai2->GetId()) 
                || (pBangPai2->GetXuanZhanBang() == (int)pBangPai1->GetId()))
            {
                state |= 4;
            }
        }
        if(InWorldWar())    
        {
            /*if(WWGetWinBang() != 0)
            {
                if(((WWGetWinBang() == (int)m_bangpai)&& ((int)bId != WWGetWinBang()))
                    || ((WWGetWinBang() == (int)bId) && ((int)m_bangpai != WWGetWinBang())))
                state |= 4;
            }*/
            if((IsFangShou(m_bangpai) && !IsFangShou(bId))
                    || (!IsFangShou(m_bangpai) && IsFangShou(bId)))
                    state |= 4;
        }
        if((pBangPai2 != NULL) && (pBangPai2->GetBangZhu() == m_roleId) && (WWGetWinBang() == (int)m_bangpai))
        {
            state |= 16;
        }
    }
    if(pUser->GetPkMiss(m_roleId) != 0)
        state |= 4;
           
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    msg<<m_roleId<<m_equipment[EETWuQi].tmplId<<m_equipment[EETWuQi].level
        //<<m_equipment[EETMaoZi].tmplId<<m_equipment[EETMaoZi].quality
        <<m_equipment[EETKuiJia].tmplId<<GetPingZhi();
        
    if(m_fightId != 0)
        state |= 1;
    if(haveKuang)
        state |= 2;
    if(GetData8(6) != 0)
        state |= 8;
        
    msg<<(uint8)state;  
    uint16 genSuiPetId = NoLOckGetGenSuiPetId();
    msg<<(uint16)m_title<<m_pkVal<<genSuiPetId;
    //GetGenSuiPet()
    if(genSuiPetId != 0)
    {
        SharePetPtr pPet = m_pet[m_gensuiPet];
        if(pPet.get() != NULL)
        {
            msg<<pPet->name<<GetPetQuality(pPet.get())<<pPet->kaiJia.tmplId;
            SItemInstance *pItem = &(pPet->kaiJia);
            uint8 quality = GetChongKaiQuality(pItem);
            msg<<quality;
        }
        else
        {
            msg<<(uint8)0;
        }
    }
    else
    {
        msg<<(uint8)0;
    }
    
    if(m_qiPet < m_petNum)
    {
        SPet *pPet = m_pet[m_qiPet].get();
        if(pPet != NULL)
        {
            msg<<(uint16)pPet->tmplId;
            //亲密度<100000 速度=1*8 亲密度>=100000 速度=1.5*8 亲密度>=300000 速度=2.0*8
            msg<<GetPetSpeed(pPet->qinmi)<<pPet->kaiJia.tmplId;
            SItemInstance *pItem = &(pPet->kaiJia);
            uint8 quality = GetChongKaiQuality(pItem);
            msg<<quality;
        }
        else
        {
            msg<<(uint16)0;
        }
    }
    else
    {
        msg<<(uint16)0;
    }
}
//当用户身上装备中不足5件达到绿字(大于3)水准时，用白色套装图片。
//当用户身上装备中有5件达到 绿字水准(大于等于3，换绿色套装图片。
//当用户身上装备中有5件达到 红字(大于等于5)以上水准，换金色套装图片。
uint8 CUser::GetPingZhi()
{
    uint8 num3Fen = 0;
    uint8 num5Fen = 0;
    
    uint16 min,mid,max;
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        int fen = 0;
        //uint8 num = 0;
        for(uint8 j = 0; j < SItemInstance::MAX_ADD_ATTR_NUM; j++)
        {
            if(m_equipment[i].addAttrType[j] != 0)
            {
                GetAddAttrVal(m_equipment[i].addAttrType[j],min,mid,max);
                if(max != 0)
                {
                    fen += m_equipment[i].addAttrVal[j]*100/max;
                    //num++;
                }
            }
        }
        uint8 quality = GetQuality(fen,6);
        if(quality >= 3)
            num3Fen++;
        if(quality >= 5)
            num5Fen++;
    }
    if(num5Fen >= 5)
        return 2;
    else if(num3Fen >= 5)
        return 1;
    return 0;
}

bool CUser::HaveLevel12Equip()
{
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        if(m_equipment[i].tmplId != 0)
        {
            SItemTemplate *pItem = itemMgr.GetItem(m_equipment[i].tmplId);
            if((pItem != NULL) && (pItem->level >= 80) && (m_equipment[i].level >= 12))
                return true;
        }
    }
    return false;
}

bool CUser::HaveAllWanMei()
{
    uint16 min,mid,max;
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        if(m_equipment[i].tmplId == 0)
            return false;
        int fen = 0;
        for(uint8 j = 0; j < SItemInstance::MAX_ADD_ATTR_NUM; j++)
        {
            if(m_equipment[i].addAttrType[j] != 0)
            {
                GetAddAttrVal(m_equipment[i].addAttrType[j],min,mid,max);
                if(max != 0)
                {
                    fen += m_equipment[i].addAttrVal[j]*100/max;
                }
            }
        }
        uint8 quality = GetQuality(fen,6);
        if(quality < 7)
            return false;
    }
    return true;
}

void CUser::MakePlayerInfo(CNetMessage &msg,CUser *pUser)
{
    int bId = pUser->GetBangPai();
    int xzBId = pUser->GetXZBangId();
    bool haveKuang = false;
    if(HaveItem(1543) || HaveItem(1544))
        haveKuang = true;
    
    uint16 mapId = GetMapId();
    //if((mapId >= 270) && (mapId <= 276))
    {
        if(HaveItem(1558) || 
            HaveItem(1559) || 
            HaveItem(1560) || 
            HaveItem(1561) || 
            HaveItem(1562))
        haveKuang = true;
    }
    uint8 state = 0;
    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
    if((mapId == 300) || (mapId == 305))
    {
        if(bId != (int)m_bangpai)
            state |= 4;
    }
    if(xzBId != 0)
    {
        if((bId == m_XZBangId) || (xzBId == (int)m_bangpai))
            state |= 4;
    }
    else
    {
        CBangPai *pBangPai1 = NULL;
        CBangPai *pBangPai2 = NULL;
        if(bId != 0)
            pBangPai1 = bPMgr.FindBangPai(bId);
        if(m_bangpai != 0)
            pBangPai2 = bPMgr.FindBangPai(m_bangpai);
        
        if((pBangPai1 != NULL) && (pBangPai2 != NULL))
        {
            if((pBangPai1->GetXuanZhanBang() == (int)pBangPai2->GetId()) 
                || (pBangPai2->GetXuanZhanBang() == (int)pBangPai1->GetId()))
            {
                state |= 4;
            }
        }
    }
    if(InWorldWar())    
    {
        /*if(WWGetWinBang() != 0)
        {
            if(((WWGetWinBang() == (int)m_bangpai)&& ((int)bId != WWGetWinBang()))
                || ((WWGetWinBang() == (int)bId) && ((int)m_bangpai != WWGetWinBang())))
            state |= 4;
        }*/
        if((IsFangShou(m_bangpai) && !IsFangShou(bId))
                    || (!IsFangShou(m_bangpai) && IsFangShou(bId)))
                    state |= 4;
    }
    if(pUser->GetPkMiss(m_roleId) != 0)
        state |= 4;
    CBangPai *pBangPai = bPMgr.FindBangPai(m_bangpai);
    if((pBangPai != NULL) && (pBangPai->GetBangZhu() == m_roleId) && (WWGetWinBang() == (int)m_bangpai))
    {
        state |= 16;
    }
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    msg<<m_roleId<<m_name<<m_useTextTitle.c_str();
    msg<<m_sex<<m_level<<m_xiang<<m_equipment[EETWuQi].tmplId<<m_equipment[EETWuQi].level
        //<<m_equipment[EETMaoZi].tmplId<<m_equipment[EETMaoZi].quality
        <<m_equipment[EETKuiJia].tmplId<<GetPingZhi();
        //m_equipment[EETKuiJia].quality;
        
    if(m_fightId != 0)
        state |= 1;
    if(haveKuang)
        state |= 2;
    if(GetData8(6) != 0)
        state |= 8;
    
    msg<<(uint8)state;    
    uint16 genSuiPetId = NoLOckGetGenSuiPetId();
    msg<<(uint16)m_title<<m_pkVal<<genSuiPetId;
    //GetGenSuiPet()
    if(genSuiPetId != 0)
    {
        SharePetPtr pPet = m_pet[m_gensuiPet];
        if(pPet.get() != NULL)
        {
            msg<<pPet->name<<GetPetQuality(pPet.get())<<pPet->kaiJia.tmplId;
            SItemInstance *pItem = &(pPet->kaiJia);
            uint8 quality = GetChongKaiQuality(pItem);
            msg<<quality;
        }
    }
    
    if(m_qiPet < m_petNum)
    {
        SPet *pPet = m_pet[m_qiPet].get();
        if(pPet != NULL)
        {
            msg<<(uint16)pPet->tmplId;
            msg<<GetPetSpeed(pPet->qinmi)<<pPet->kaiJia.tmplId;
            
            SItemInstance *pItem = &(pPet->kaiJia);
            uint8 quality = GetChongKaiQuality(pItem);
            msg<<quality;
        }
        else
        {
            msg<<(uint16)0;
        }
    }
    else
    {
        msg<<(uint16)0;
    }
}

uint16 CUser::GetGenSuiPetId()
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    return NoLOckGetGenSuiPetId();
}

uint16 CUser::NoLOckGetGenSuiPetId()
{
    uint16 petId = 0;
    if(m_gensuiPet < m_petNum)
    {
        SharePetPtr pPet = m_pet[m_gensuiPet];
        if(pPet.get() != NULL)
        {
            petId = pPet->tmplId;
        }
    }
    return petId;
}

void CUser::AssignPetAttr(uint8 pos,uint8 attr)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(pos > m_petNum)
        return;
    
    SPet *pPet = m_pet[pos].get();
    if((pPet == NULL) || (pPet->shuxingdian <= 0))
        return;
        
    switch(attr)
    {
    case 1:// 体质
        pPet->tizhi++;
        pPet->Init();
        UpdatePetInfo(pos,7,pPet->tizhi);
        break;
    case 2:// 力量
        pPet->liliang++;
        pPet->Init();
        UpdatePetInfo(pos,8,pPet->liliang);
        break;
    case 3:// 敏捷
        pPet->minjie++;
        pPet->Init();
        UpdatePetInfo(pos,9,pPet->minjie);
        break;
    case 4:// 灵力
        pPet->lingxing++;
        pPet->Init();
        UpdatePetInfo(pos,10,pPet->lingxing);
        break;
    case 5:// 耐力
        pPet->naili++;
        pPet->Init();
        UpdatePetInfo(pos,11,pPet->naili);
        break;
    default:
        return;
    }
    pPet->shuxingdian--;
    //SMonsterTmpl *pTmpl = SingletonMonsterManager::instance().GetTmpl(pPet->tmplId);
    //if(pTmpl != NULL)
    
    UpdatePetInfo(pos,15,pPet->maxHp);
    UpdatePetInfo(pos,16,pPet->maxMp);
    UpdatePetInfo(pos,19,pPet->speed);
    UpdatePetInfo(pos,17,pPet->GetDamage());
    UpdatePetInfo(pos,20,pPet->GetRecovery());
    
    //int skillAttack = (int)((pPet->level*4+(pPet->lingxing-pPet->level)*1.2)*(pPet->skillAttackCZ*0.13)+90);
    UpdatePetInfo(pos,18,pPet->GetSkillAttack());
}

void CUser::AssignPetKangXing(uint8 pos,uint8 attr)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if((attr < 1) || (attr > SPet::MAX_NAME_LEN))
        return;

    attr--;
    if(pos > m_petNum)
        return;
        
    SPet *pPet = m_pet[pos].get();
    if((pPet == NULL) || (pPet->shuxingdian <= 0))
        return;

    //pPet->kangxingdian--;
    //pPet->kangxing[pos]++;
    /*
    SMonsterTmpl *pTmpl = SingletonMonsterManager::instance().GetTmpl(pPet->tmplId);
    if(pTmpl != NULL)
        pPet->Init(pTmpl);*/
}

void CUser::SetGuanKanPet(uint8 pos)
{
    m_gensuiPet = pos;
    if(m_qiPet == pos)
        m_qiPet = 0xff;
    
    SPet *pPet = m_pet[pos].get();
    if(pPet != NULL)
    {
        uint8 state = 1;
        if(m_chuZhanPet == pos)
        {
            state |= 0x2;
        }
        if(pPet->bangDing == 1)
        {
            state |= 0x4;
        }
        UpdatePetInfo(pos,1,state);
    }
}

void CUser::SetPetType(uint8 pos,uint8 type)
{
    if(pos >= m_petNum)
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SPet *pPet = m_pet[pos].get();
    if(pPet != NULL)
    {
        if(type == 3)
            pPet->qiCheng = 1;
    }
}
void CUser::SetQiPet(uint8 pos)
{
    if(pos >= m_petNum)
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SPet *pPet = m_pet[pos].get();
    if(pPet != NULL)
    {
        if(pPet->qiCheng == 0)
        {
            CNetMessage msg;
            msg.SetType(MSG_SERVER_USE_RESULT);
            msg<<(uint8)12<<PRO_ERROR<<"不能骑乘";
            CSocketServer &sock = SingletonSocket::instance();
            sock.SendMsg(m_sock,msg);
            return;
        }
        if(pos < m_petNum)
            m_qiPet = pos;
        uint8 state = 0;
        if(m_gensuiPet == pos)
        {
            m_gensuiPet = 0xff;
        }
        if(pPet->bangDing == 1)
        {
            state |= 0x4;
        }
        if(m_chuZhanPet == pos)
        {
            m_chuZhanPet = 0xff;
        }
        state |= 0x8;
        UpdatePetInfo(pos,1,state);
        
        UpdateInfo();
    }
}

void CUser::SetChuZhanPet(uint8 pos)
{
    m_chuZhanPet = pos;
    if(m_qiPet == pos)
    {
        m_qiPet = 0xff;
        UpdateInfo();
    }
    SharePetPtr pet;
    if(GetChuZhanPet(pet))
    {
        SPet *pPet = pet.get();
        if(pPet == NULL)
            return;
        pPet->bangDing = 1;
        uint8 state = 0x2;
        if(m_gensuiPet == pos)
        {
            state |= 0x1;
        }
        if(pPet->bangDing == 1)
        {
            state |= 0x4;
        }
        UpdatePetInfo(pos,1,state);
    }
}

//0解绑成功，1 没有宠物解绑符，2 宠物非绑定、3 解绑时间未到
int CUser::PetJieBang(uint8 petPos)
{
    if((petPos >= m_petNum) || (petPos >= MAX_PET_NUM))
        return 2;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SharePetPtr pet = m_pet[petPos];
    SPet *pPet = pet.get();
    if(pPet == NULL)
        return 2;
    if(pPet->bangDing == 0)
        return 2;
    //time_t *jbTime = (time_t*)(pPet->skill+SPet::MAX_SKILL_NUM);
    if(pPet->jbTime+10*24*3600 > GetSysTime())
        return 3;
    
    uint8 pos = 0;
    for(; pos < MAX_PACKAGE_NUM; pos++)
    {
        if(m_package[pos].tmplId == 1834)
        {
            break;
        }
    }
    if(pos >= MAX_PACKAGE_NUM)
        return 1;
    
    pPet->jbTime = GetSysTime();
    pPet->bangDing = 0;
    NoLockDelPackage(pos);
    
    uint8 state = 0;
    if(m_chuZhanPet == petPos)
    {
        state |= 0x2;
    }
    if(m_gensuiPet == petPos)
    {
        state |= 0x1;
    }
    SaveJieBangPet(m_roleId,*pPet);
    UpdatePetInfo(petPos,1,state);
    return 0;
}

void CUser::SetPetIdle(uint8 pos)
{
    if(m_chuZhanPet == pos)
        m_chuZhanPet = 0xff;

    if(m_gensuiPet == pos)
        m_gensuiPet = 0xff;

    if(m_qiPet == pos)
        m_qiPet = 0xff;
    SharePetPtr pet;
    SPet *pPet = NULL;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        if(pos < m_petNum)
        {
            pet = m_pet[pos];
            pPet = pet.get();
        }
    }
    if(pPet != NULL)
    {
        uint8 state = 0;
        if(pPet->bangDing == 1)
        {
            state |= 0x4;
        }
        UpdatePetInfo(pos,1,state);
    }
}

void CUser::SendItemTimeOut()
{
    //1810 招摇鼓	使用后1小时内，遇敌间隔为原来的40%。队长有效。
    string info;
    if((m_fewMonsterEnd != 0) && (m_fewMonsterEnd < GetSysTime()))
    {
        m_fewMonsterEnd = 0;
        info.append("招摇鼓效力消失了");
    }
    //1811 震天鼓	使用后1小时内，遇敌数为当前可遇敌的最大值。队长有效。
    if((m_maxMonsterEnd != 0) && (m_maxMonsterEnd < GetSysTime()))
    {
        m_maxMonsterEnd = 0;
        if(info.size() > 0)
            info.append("|");
        info.append("震天鼓效力消失了");
    }
    //1812 弱怪灵牌	使用后1小时内，遇敌等级为当前可遇敌等级的最小值。队长有效。
    if((m_minMonsterEnd != 0) && (m_minMonsterEnd < GetSysTime()))
    {
        m_minMonsterEnd = 0;
        if(info.size() > 0)
            info.append("|");
        info.append("弱怪灵牌效力消失了");
    }
    //1813 驱敌香	使用后1小时内，在可遇怪等级最大值<人物等级的地图区域不遇敌。
    if((m_upMonsterEnd != 0) && (m_upMonsterEnd < GetSysTime()))
    {
        m_upMonsterEnd = 0;
        if(info.size() > 0)
            info.append("|");
        info.append("驱敌香效力消失了");
    }
    if((m_userDoubleEnd != 0) && (m_userDoubleEnd < GetSysTime()))
    {
        m_userDoubleEnd = 0;
        if(info.size() > 0)
            info.append("|");
        info.append("双倍修炼令效力消失了");
    }
    if((m_petDoubleEnd != 0) && (m_petDoubleEnd < GetSysTime()))
    {
        m_petDoubleEnd = 0;
        if(info.size() > 0)
            info.append("|");
        info.append("宠双修炼符效力消失了");
    }
    if((m_noPunishEnd != 0) && (m_noPunishEnd < GetSysTime()))
    {
        m_noPunishEnd = 0;
        if(info.size() > 0)
            info.append("|");
        info.append("秘传书效力消失了");
    }
    if(info.size() > 0)
        SendPopMsg(this,info.c_str());
}

void CUser::AddPkVal(int val)
{
    m_pkVal += val;
    SendUpdateInfo(40,m_pkVal);
    if(m_pkVal > MAX_PK_VAL)
    {
        if(m_pScene != NULL)
            m_pScene->LeaveTeam(this);
        TransportUser(this,SCENE_JIANYU,7,8,8);
        m_jianYuTime = 6*3600;
        char buf[64];
        snprintf(buf,63,"%s:被关进监狱",m_name);
        SysInfoToAllUser(buf);
    }
}
/*
void CUser::operator=(CUser &user)
{
}
*/
bool CUser::OpenPack(uint8 type,SItemInstance *pItem)
{
    uint8 num = pItem->addAttrType[0];
    time_t endTime = GetSysTime() + pItem->addAttrVal[0]*3600*24;
    switch(type)
    {
    case 1:
        {
            if(m_openPack != 1)
                return false;
            if(num > MAX_EXT_PACKAGE_NUM)
                return false;
            MAX_PACKAGE_NUM = 3*18+num;
            m_saveData8[0] = num;
            m_saveData32[0] = endTime;
        }
        break;
    case 2:
        {
            if(num > MAX_EXT_BANK_NUM)
                return false;
            m_saveData8[1] = num;
            m_saveData32[1] = endTime;
        }
        break;
    case 3:
        {
            if(num > MAX_EXT_PET_BANK_NUM)
                return false;
            m_saveData8[2] = num;
            m_saveData32[2] = endTime;
        }
        break;
    }
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(UPDATE_OPSN_PACK);
    msg<<type<<num<<(uint32)endTime;
    sock.SendMsg(m_sock,msg);
    return true;
}

CCallScript *CUser::NoLockUseItem(uint8 pos,int *pAddHp,int *pAddMp,uint8 val,uint8 val1,uint8 num)
{
    if(pos >= GetCurMaxPackNum())
        return 0;
    SItemInstance *pInst = m_package+pos;
    if(pInst == NULL)
        return 0;
    SItemTemplate *pItem = SingletonItemManager::instance().GetItem(pInst->tmplId);
    if(pItem == NULL)
        return 0;
    if(pAddHp != NULL)
        *pAddHp = -1;
    if(pAddMp != NULL)
        *pAddMp = -1;
    
    if((pItem->type == EITChargeItem) || (pItem->type == EITAddAttr))
    {
        SaveUseItem(m_roleId,m_package[pos],"使用",1);
    }
    if(pItem->id == 2076)
    {
        if(OpenPack(1,m_package+pos))
            NoLockDelPackage(pos);
    }
    else if(pItem->id == 2077)
    {
        if(OpenPack(2,m_package+pos))
            NoLockDelPackage(pos);
    }
    else if(pItem->id == 2078)
    {
        if(OpenPack(3,m_package+pos))
            NoLockDelPackage(pos);
    }
    else if(pItem->id == 1839)
    {
        NoLockDelPackage(pos);
        CNetMessage msg;
        msg.SetType(MSG_SERVER_VISUAL_EFFECT);
        msg<<(uint8)1;
        SendSceneMsg(msg,m_pScene);
        
        msg.ReWrite();
        msg.SetType(PRO_MSG_CHAT);
        char buf[128];
        snprintf(buf,sizeof(buf),"【%s】燃放了一个漂亮的礼花",m_name);
        msg<<(uint8)0<<0<<"【系统】"<<buf;
        SendSceneMsg(msg,m_pScene);
        return 0;
    }
    else if(pItem->id == 1833)
    {
        if(HaveItem(1558) || 
            HaveItem(1559) || 
            HaveItem(1560) || 
            HaveItem(1561) || 
            HaveItem(1562))
        {
            SendSysInfo(this,"携矿中不能传送");
            return 0;
        }
        if(GetSceneId() == SCENE_JIANYU)
        {
            return 0;
        }
        if(m_teamId != 0)
        {
            return 0;
        }
        if(HaveBitSet(156))
        {
            SendSysInfo(this,"任务中不能传送");
            return 0;
        }
        if(m_pScene == NULL)
            return 0;  
        uint16 mapId = m_pScene->GetMapId();
        if((mapId == 300) || (mapId == 305))
        {
            if((m_pScene->GetId()>>16) != (int)m_bangpai)
            {
                SendSysInfo(this,"非本帮领地，此功能暂时无法使用");
                return 0;
            }
        }
        
        switch(m_menpai)
        {
        case 1:
            NoLockDelPackage(pos);
            TransportUser(this,200,14,19,8);
            break;
        case 2:
            NoLockDelPackage(pos);
            TransportUser(this,210,14,17,8);
            break;
        case 3:
            NoLockDelPackage(pos);
            TransportUser(this,220,14,19,8);
            break;
        case 4:
            NoLockDelPackage(pos);
            TransportUser(this,230,17,20,8);
            break;
        case 5:
            NoLockDelPackage(pos);
            TransportUser(this,240,14,20,8);
            break;
        default:
            return 0;
        }
    }
    else if((pItem->id >= 1827) && (pItem->id <= 1831))
    {
        int *pLeftNaiJiu = (int*)pInst->addAttrVal;
        if(*pLeftNaiJiu == 0)
        {
            *pLeftNaiJiu = pItem->addXue;;
        }
        
        if(*pLeftNaiJiu > 0)
        {
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem;
            bool update = false;
            
            uint8 itemPos[EETWuQi+1];
            //武器--- 盔甲---- 腰带----- 帽子 ---- 靴子
            itemPos[0] = EETWuQi;
            itemPos[1] = EETKuiJia;
            itemPos[2] = EETYaoDai;
            itemPos[3] = EETMaoZi;
            itemPos[4] = EETXieZi;
            for(uint8 j = 0; j <= EETWuQi; j++)
            {
                uint8 i = itemPos[j];
                pItem = itemMgr.GetItem(m_equipment[i].tmplId);
                if(pItem != NULL)
                {
                    int naijiu = m_equipment[i].naijiu;
                    //实际损耗的耐久值*int(1+ (装备等级*装备等级/500))
                    //实际损耗的耐久值*装备等级/15
                    int useNaiJiu = (int)((pItem->naijiu - m_equipment[i].naijiu)*
                                    pItem->level/15);
                    if(useNaiJiu == 0)
                        useNaiJiu = 1;
                    if(naijiu < pItem->naijiu)
                    {
                        if(naijiu == 0)
                        {
                            update = true;
                        }
                        if(*pLeftNaiJiu >= useNaiJiu)
                        {
                            *pLeftNaiJiu -= useNaiJiu;
                            m_equipment[i].naijiu = pItem->naijiu;
                            UpdateEquip(i);
                            if(*pLeftNaiJiu == 0)
                            {
                                NoLockDelPackage(pos);
                                break;
                            }
                        }
                        else
                        {
                            //磨刀石剩余耐久/ int(1+ (装备等级*装备等级/500))
                            //磨刀石剩余耐久/ (装备等级/15)，
                            m_equipment[i].naijiu += (int)(*pLeftNaiJiu/(pItem->level/15.0));
                            *pLeftNaiJiu = 0;
                            NoLockDelPackage(pos);
                            UpdateEquip(i);
                            break;
                        }
                    }
                }
            }
            
            SharePetPtr pet = GetPet(m_chuZhanPet);
            SPet *pPet = pet.get();
            if((pPet != NULL) && (pPet->kaiJia.tmplId != 0))
            {
                SItemInstance &kaiji = pPet->kaiJia;
                pItem = itemMgr.GetItem(kaiji.tmplId);
                if(pItem != NULL)
                {
                    int naijiu = kaiji.naijiu;
                    int useNaiJiu = (int)((pItem->naijiu - kaiji.naijiu)*
                                    pItem->level/15);
                    if(useNaiJiu == 0)
                        useNaiJiu = 1;
                    if(naijiu < pItem->naijiu)
                    {
                        if(naijiu == 0)
                        {
                            update = true;
                        }
                        if(*pLeftNaiJiu >= useNaiJiu)
                        {
                            *pLeftNaiJiu -= useNaiJiu;
                            kaiji.naijiu = pItem->naijiu;
                            UpdatePet(m_chuZhanPet);
                            if(*pLeftNaiJiu == 0)
                            {
                                NoLockDelPackage(pos);
                            }
                        }
                        else
                        {
                            kaiji.naijiu += (int)(*pLeftNaiJiu/(pItem->level/15.0));
                            *pLeftNaiJiu = 0;
                            NoLockDelPackage(pos);
                            UpdatePet(m_chuZhanPet);
                        }
                    }
                }
            }
            
            if(update)
            {
                Init();
                UpdateInfo();
            }
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[pos],pos,msg);
            sock.SendMsg(m_sock,msg);
        }
        else
        {
            NoLockDelPackage(pos);
        }
    }
    else if((pItem->id >= 1818) && (pItem->id <= 1821))
    {
        if(m_level > pItem->level) 
            return 0;
            
        m_tizhi = m_level;
        m_liliang = m_level;
        m_minjie = m_level;
        m_lingli = m_level;
        m_naili = m_level;
        m_shuxingdian = m_level*5-5;
        
        m_jin = 0;
        m_mu = 0;
        m_shui = 0;
        m_huo = 0;
        m_tu = 0;
        m_xiangxingdian = m_level/2;
                
        //EquipmentAddAttr(true);
        SendUpdateInfo(16,GetTiZhi());//m_tizhi);
        SendUpdateInfo(17,GetLiLiang());//m_liliang);
        SendUpdateInfo(18,GetMinJie());//m_minjie);
        SendUpdateInfo(19,GetLingLi());//m_lingli);
        SendUpdateInfo(20,GetNaiLi());//m_naili);
        SendUpdateInfo(4,m_shuxingdian);
        
        SendUpdateInfo(21,GetJin());//m_jin);
        SendUpdateInfo(22,GetMu());//m_mu);
        SendUpdateInfo(23,GetShui());//m_shui);
        SendUpdateInfo(24,GetHuo());//m_huo);
        SendUpdateInfo(25,GetTu());//m_tu);
        SendUpdateInfo(5,m_xiangxingdian);
        NoLockDelPackage(pos);
        Init();
        UpdateInfo();
    }
    else if(pItem->id == 1810)
    {//1810 招摇鼓	使用后1小时内，遇敌间隔为原来的40%。队长有效。
        if(m_fewMonsterEnd < GetSysTime())
        {
            m_fewMonsterEnd = GetSysTime()+3600;
            NoLockDelPackage(pos);
        }
    }
    else if(pItem->id == 1811)
    {//1811 震天鼓	使用后1小时内，遇敌数为当前可遇敌的最大值。队长有效。
        if(m_maxMonsterEnd < GetSysTime())
        {
            m_maxMonsterEnd = GetSysTime()+3600;
            NoLockDelPackage(pos);
        }
    }
    else if(pItem->id == 1812)
    {//1812 弱怪灵牌	使用后1小时内，遇敌等级为当前可遇敌等级的最小值。队长有效。
        if(m_minMonsterEnd < GetSysTime())
        {
            m_minMonsterEnd = GetSysTime()+3600;
            NoLockDelPackage(pos);
        }
    }
    else if(pItem->id == 1813)
    {//1813 驱敌香	使用后1小时内，在可遇怪等级最大值<人物等级的地图区域不遇敌。
        if(m_upMonsterEnd < GetSysTime())
        {
            m_upMonsterEnd = GetSysTime()+3600;
            NoLockDelPackage(pos);
        }
    }
    else if(pItem->id == 1814)
    {//1814 清心散	可使驱敌香失效。
        if(m_upMonsterEnd > GetSysTime())
        {
            m_upMonsterEnd = 0;
            SendPopMsg(this,"驱敌香效力消失了");
            NoLockDelPackage(pos);
        }
    }
    else if(pItem->id == 1809)
    {
        if(HaveItem(1558) || 
            HaveItem(1559) || 
            HaveItem(1560) || 
            HaveItem(1561) || 
            HaveItem(1562))
        {
            SendSysInfo(this,"携矿中不能传送");
            return 0;
        }
        if(HaveBitSet(156))
        {
            SendSysInfo(this,"任务中不能传送");
            return 0;
        }
        if(GetSceneId() == SCENE_JIANYU)
        {
            return 0;
        }
        if((m_teamId != 0) && (m_roleId != m_teamId))
        {
            return 0;
        }
        if(m_pScene == NULL)
            return 0;  
        uint16 mapId = m_pScene->GetMapId();
        if((mapId == 300) || (mapId == 305))
        {
            if((m_pScene->GetId()>>16) != (int)m_bangpai)
            {
                SendSysInfo(this,"非本帮领地，此功能暂时无法使用");
                return 0;
            }
        }
        switch(val)
        {
        case 0:
            TransportUser(this,1,13,13,8);
            break;
        case 1:
            TransportUser(this,11,15,21,8);
            break;
        case 2:
            TransportUser(this,21,15,12,8);
            break;
        case 3:
            TransportUser(this,35,6,15,8);
            break;
        default:
            return 0;
        }
        pInst->naijiu--;
        if((pInst->naijiu <= 0) || (pInst->naijiu > 0x7fff))
        {
            NoLockDelPackage(pos);
        }
        else
        {
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[pos],pos,msg);
            sock.SendMsg(m_sock,msg);
        }
    }
    else if(pItem->id == 1815)
    {
        if(HaveBitSet(156))
        {
            SendSysInfo(this,"任务中不能传送");
            return 0;
        }
        
        if(m_pScene == NULL)
            return 0;
        uint16 mapId = m_pScene->GetMapId();
        if((mapId == 300) || (mapId == 305))
        {
            if((m_pScene->GetId()>>16) != (int)m_bangpai)
            {
                SendSysInfo(this,"非本帮领地，此功能暂时无法使用");
                return 0;
            }
        }
        
        if(GetSceneId() == SCENE_JIANYU)
        {
            return 0;
        }
        if(((mapId >= 260) && (mapId <= 266))
            || (mapId == 302)
            || (mapId == 304)
            || (mapId == 300)
            || (mapId == 305)
            || ((mapId >= 270) && (mapId <= 276)))
        {
            return 0;
        }
        if(val == 0)
        {
            if(HaveItem(1558) || 
                HaveItem(1559) || 
                HaveItem(1560) || 
                HaveItem(1561) || 
                HaveItem(1562))
            {
                SendSysInfo(this,"携矿中不能传送");
                return 0;
            }
            if(m_teamId != 0)
            {
                return 0;
            }
            if(val1 >= MAX_SAVE_POS)
                return 0;
            if((m_savePos[val1].sceneId == 0) 
                || (m_savePos[val1].x == 0)
                || (m_savePos[val1].y == 0))
            {
                return 0;
            }
            if(m_savePos[val1].sceneId >= 299)
                return 0;
                
            TransportUser(this,m_savePos[val1].sceneId,m_savePos[val1].x,m_savePos[val1].y,8);
            pInst->naijiu--;
            if((pInst->naijiu <= 0) || (pInst->naijiu > 0x7fff))
            {
                NoLockDelPackage(pos);
            }
            else
            {
                CSocketServer &sock = SingletonSocket::instance();
                CNetMessage msg;
                msg.SetType(PRO_UPDATE_PACK);
                MakePack(m_package[pos],pos,msg);
                sock.SendMsg(m_sock,msg);
            }
        }
        else if(val == 1)
        {
            if(m_pScene->InGuiYu() || m_pScene->InMatchScene())
                return 0;
            if(val1 >= MAX_SAVE_POS)
                return 0;
            if(m_pScene != NULL)
            {
                m_savePos[val1].sceneId = m_pScene->GetMapId();
                m_savePos[val1].x = m_xPos;
                m_savePos[val1].y = m_yPos;
            }
        }
    }
    else if(pItem->id == 1801)
    {
        if(m_userDoubleEnd < GetSysTime())
        {
            m_userDoubleEnd = GetSysTime()+3600;
            NoLockDelPackage(pos);
        }
    }
    else if(pItem->id == 1802) 
    {
        if(m_petDoubleEnd < GetSysTime())
        {
            m_petDoubleEnd = GetSysTime()+3600;
            NoLockDelPackage(pos);
        }
    }
    else if(pItem->id == 1803)
    {
        if(m_noPunishEnd < GetSysTime())
        {
            m_noPunishEnd = GetSysTime()+3600;
            NoLockDelPackage(pos);
        }
    }
    else if((pItem->id == 1804) || (pItem->id == 1822))
    {
        int *pLeftHp = (int*)pInst->addAttrVal;
        if(*pLeftHp == 0)
            *pLeftHp = pItem->addXue;
        int addHp = pItem->addXue;
        
        if(addHp > (m_maxHp - m_hp))
            addHp = m_maxHp - m_hp;
        if(addHp > pItem->addQiXue)
            addHp = pItem->addQiXue;
        if(addHp >= *pLeftHp)
        {
            addHp = *pLeftHp;
            NoLockDelPackage(pos);
        }
        *pLeftHp -= addHp;
        AddHp(addHp);
        if(pAddHp != NULL)
            *pAddHp = addHp;
        if(*pLeftHp > 0)
        {
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[pos],pos,msg);
            sock.SendMsg(m_sock,msg);
        }
    }
    else if(pItem->id == 1805)
    {
        int *pLeftMp = (int*)pInst->addAttrVal;
        if(*pLeftMp == 0)
            *pLeftMp = pItem->addXue;
        int addMp = pItem->addXue;
        
        if(addMp > (m_maxMp - m_mp))
            addMp = m_maxMp - m_mp;
        if(addMp > pItem->addQiXue)
            addMp = pItem->addQiXue;
        if(addMp >= *pLeftMp)
        {
            addMp = *pLeftMp;
            NoLockDelPackage(pos);
        }
        *pLeftMp -= addMp;
        AddMp(addMp);
        if(pAddMp != NULL)
            *pAddMp = addMp;
        if(*pLeftMp > 0)
        {
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[pos],pos,msg);
            sock.SendMsg(m_sock,msg);
        }
    }
    else if((pItem->type == EITTeShu) || (pItem->type == EITNvWaShi))
    {
        //return pItem->id+20000;
        if((pInst->naijiu > 0) && (pInst->tmplId != 1006))
            pInst->naijiu--;
        if((pInst->naijiu == 0) && (pItem->naijiu != 0))
        {
            NoLockDelPackage(pos);
        }
        else if(pInst->naijiu != 0)
        {
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[pos],pos,msg);
            sock.SendMsg(m_sock,msg);
        }
        return pItem->pScript;
    }
    else if((pItem->type == EITPKYaoPin) || (pItem->type == EITNormalYaoPin))
    {
        uint8 mod = pItem->mod;
        int addHp = -1;
        int addMp = -1;
        if((mod & EIMTHpPer) != 0)
        {
            addHp = CalculateRate((int)m_maxHp,(int)pItem->addQiXue,100);
        }
        if((mod & EIMTHpFix) != 0)
        {
            addHp = pItem->addQiXue*num;
        }
        if((mod & EIMTMpPer) != 0)
        {
            addMp = CalculateRate((int)m_maxMp,(int)pItem->addFaLi,100);
        }
        if((mod & EIMTMpFix) != 0)
        {
            addMp = pItem->addFaLi*num;
        }
        if((mod & EIMTZhongcheng) != 0)
        {
            return 0;
        }
        if((mod & EIMTShouming) != 0)
        {
            return 0;
        }
        if(pAddHp != NULL)
            *pAddHp = addHp;
        if(pAddMp != NULL)
            *pAddMp = addMp;
        if(addHp > 0)
        {
            if(addHp > m_maxHp - m_hp)
                addHp = m_maxHp - m_hp;
            AddHp(addHp);
        }
        if(addMp > 0)
        {
            if(addMp > m_maxMp - m_mp)
                addMp = m_maxMp - m_mp;
            AddMp(addMp);
        }
        NoLockDelPackage(pos,num);
    }
    return 0;
}

void CUser::UpdatePackage(uint8 pos)
{
    if(pos >= MAX_PACKAGE_NUM)
        return;
        
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PACK);
    MakePack(m_package[pos],pos,msg);
    sock.SendMsg(m_sock,msg);
}

void CUser::FightUseItem(uint8 pos,int &addHp,int &addMp,int maxHp,int maxMp,int hp,int mp,bool pkQieCuo)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SItemInstance *pInst = GetItem(pos);
    if(pInst == NULL)
        return;
    SItemTemplate *pItem = SingletonItemManager::instance().GetItem(pInst->tmplId);
    if(pItem == NULL)
        return;
    if((pkQieCuo) && ((pItem->type != EITPKYaoPin) && (pItem->id != 1805)))
        return;
        
    if((pItem->id == 1804) || (pItem->id == 1822))
    {
        int *pLeftHp = (int*)pInst->addAttrVal;
        if(*pLeftHp == 0)
            *pLeftHp = pItem->addXue;
        addHp = pItem->addXue;
        if(addHp > (maxHp - hp))
            addHp = maxHp - hp;
        if(addHp > pItem->addQiXue)
            addHp = pItem->addQiXue;
        if(addHp >= *pLeftHp)
        {
            addHp = *pLeftHp;
            SaveUseItem(m_roleId,m_package[pos],"战斗使用",1);
            NoLockDelPackage(pos);
        }
        *pLeftHp -= addHp;
        if(*pLeftHp > 0)
        {
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[pos],pos,msg);
            sock.SendMsg(m_sock,msg);
        }
    }
    else if(pItem->id == 1805)
    {
        int *pLeftMp = (int*)pInst->addAttrVal;
        if(*pLeftMp == 0)
            *pLeftMp = pItem->addXue;
        addMp = pItem->addXue;
        if(addMp > (maxMp - mp))
            addMp = maxMp - mp;
        if(addMp > pItem->addQiXue)
            addMp = pItem->addQiXue;
        if(addMp >= *pLeftMp)
        {
            addMp = *pLeftMp;
            SaveUseItem(m_roleId,m_package[pos],"战斗使用",1);
            NoLockDelPackage(pos);
        }        
        *pLeftMp -= addMp;
        if(*pLeftMp > 0)
        {
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[pos],pos,msg);
            sock.SendMsg(m_sock,msg);
        }
    }
    else if((pItem->type == EITPKYaoPin) || (pItem->type == EITNormalYaoPin))
    {
        uint8 mod = pItem->mod;
        if((mod & EIMTHpPer) != 0)
        {
            addHp = CalculateRate(maxHp,(int)pItem->addQiXue,100);
        }
        if((mod & EIMTHpFix) != 0)
        {
            addHp = pItem->addQiXue;
        }
        if((mod & EIMTMpPer) != 0)
        {
            addMp = CalculateRate(maxMp,(int)pItem->addFaLi,100);
        }
        if((mod & EIMTMpFix) != 0)
        {
            addMp = pItem->addFaLi;
        }
        if(addHp > maxHp - hp)
            addHp = maxHp - hp;
        if(addMp > maxMp - mp)
            addMp = maxMp - mp;
        NoLockDelPackage(pos);
    }
}

void CUser::UseItem(uint8 pos,int *pAddHp,int *pAddMp,uint8 val,uint8 val1,uint8 num)
{
    if(m_fightId != 0)
        return;
    CCallScript *pScript = NULL;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        pScript = NoLockUseItem(pos,pAddHp,pAddMp,val,val1,num);
    }
    if(pScript != NULL)
    {
        /*char buf[64];
        sprintf(buf,"%d.lua",scriptId);
        CCallScript script(buf);*/
        
        pScript->Call("Main","u",this);
        SetCallScript(pScript->GetScriptId());
    }
}

bool CUser::UseItemToPet(uint8 petPos,uint8 itemPos,int *pAddHp,int *pAddMp,int val)
{
    if(m_fightId != 0)
        return false;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    SItemInstance *pInst = GetItem(itemPos);
    if(pInst == NULL)
        return false;
    SItemTemplate *pItem = SingletonItemManager::instance().GetItem(pInst->tmplId);
    if(pItem == NULL)
        return false;
    
    if(petPos >= MAX_PET_NUM)
        return false;
    SPet *pPet = m_pet[petPos].get();
    if(pPet == NULL)
        return false;
    
    if((pItem->type == EITChargeItem) || (pItem->type == EITAddAttr))
    {
        if(pItem->id != 631)
            SaveUseItem(m_roleId,m_package[itemPos],"对宠物使用",1);
    }
    
    if(pItem->type == EITPetBook)
    {//宠物天书
        //int skillIds[] = {151,152,153,154,155,156,157,158,51,55,59,63,67,101,105,109,113,117};
        int itemSkills[] = {540,160,541,161,542,162,543,163,544,164,545,165,546,166,547,167,548,168,549,169,550,170,551,171,552,172,553,173,554,174,555,175,556,176,557,177,558,178,559,179,560,180,561,181,562,182,563,183,564,184,565,185,566,51,567,55,568,59,569,63,570,67,571,151,572,152,573,153,574,154,575,158,576,101,577,105,578,109,579,113,580,117,581,155,582,156,583,157,584,3,585,7,586,11,587,15,588,19};
        int skillId = 0;
        uint32 i;
        for(i = 0; i < sizeof(itemSkills)/sizeof(itemSkills[0]); i += 2)
        {
            if(pItem->id == itemSkills[i])
            {
                skillId = itemSkills[i+1];
                break;
            }
        }
        if(pPet->GetSkillLevel(skillId) != 0)
        {
            CNetMessage msg;
            msg.SetType(MSG_SERVER_USE_RESULT);
            msg<<(uint8)1<<PRO_ERROR<<skillId<<0;
            CSocketServer &sock = SingletonSocket::instance();
            sock.SendMsg(m_sock,msg);
            return false;
        }
        if(skillId == 0)
            return false;
        uint32 xiyou = sizeof(itemSkills)/sizeof(itemSkills[0]) - 17;
        if(i >= xiyou)
        {
            for(i = xiyou; i < sizeof(itemSkills)/sizeof(itemSkills[0]); i+=2)
            {
                if(pPet->GetSkillLevel(itemSkills[i]) != 0)
                {
                    pPet->DelSkill(itemSkills[i]);
                    uint8 skillLevel = pPet->AddSkill(skillId);
                    if(skillLevel >= 1)
                    {
                        CNetMessage msg;
                        msg.SetType(PRO_PET_SKILL);
                        msg<<petPos;
                        NoLockMakePetSkill(petPos,msg);
                        CSocketServer &sock = SingletonSocket::instance();
                        sock.SendMsg(m_sock,msg);
                        NoLockDelPackage(itemPos);
                        
                        msg.ReWrite();
                        msg.SetType(MSG_SERVER_USE_RESULT);
                        msg<<(uint8)1<<PRO_SUCCESS<<skillId<<itemSkills[i];
                        sock.SendMsg(m_sock,msg);
                        return true;
                    }
                    return false;
                }
            }
        }
        uint8 skillLevel;
        int gaiLv[] = {300000,100000,50000,25000,12500,6250,3125};
        int r = Random(0,1000000);
        uint8 pos = pPet->GetSkillNum();
        if((pos > 0) && (pos < sizeof(gaiLv)/sizeof(int)))
        {
            if(r < gaiLv[pos-1])
            {
                skillLevel = pPet->AddSkill(skillId);
                CNetMessage msg;
                msg.SetType(PRO_PET_SKILL);
                msg<<petPos;
                NoLockMakePetSkill(petPos,msg);
                CSocketServer &sock = SingletonSocket::instance();
                sock.SendMsg(m_sock,msg);
                NoLockDelPackage(itemPos);
                
                msg.ReWrite();
                msg.SetType(MSG_SERVER_USE_RESULT);
                msg<<(uint8)1<<PRO_SUCCESS<<skillId<<0;
                sock.SendMsg(m_sock,msg);
                return true;
            }
        }
        
        int fugaiId = 0;
        skillLevel = pPet->AddSkill(skillId,true,&fugaiId);
        if(skillLevel >= 1)
        {
            pPet->Init();
            
            CNetMessage msg;
            msg.SetType(PRO_PET_SKILL);
            msg<<petPos;
            NoLockMakePetSkill(petPos,msg);
            CSocketServer &sock = SingletonSocket::instance();
            sock.SendMsg(m_sock,msg);
            NoLockDelPackage(itemPos);
    
            msg.ReWrite();
            msg.SetType(MSG_SERVER_USE_RESULT);
            msg<<(uint8)1<<PRO_SUCCESS<<skillId<<fugaiId;
            sock.SendMsg(m_sock,msg);
            return true;
        }
        return false;
    }
    else if(pItem->id == 631)
    {
        if(pPet->kaiJia.tmplId != 0)
            return false;
        string before;
        HexToStr(*pPet,before);
        CMonsterManager &monsterMgr = SingletonMonsterManager::instance();
        int qinMi = m_pet[petPos]->qinmi;
        uint8 qiCheng = m_pet[petPos]->qiCheng;
        ShareMonsterPtr monster = monsterMgr.CreateMonster(m_pet[petPos]->tmplId,EMTBaoBao);
        if(monster.get() == NULL)
            return false;
        m_pet[petPos] = monsterMgr.CreatePet(monster.get(),true); 
        if(m_pet[petPos].get() == NULL)
            return false;
        m_pet[petPos]->bangDing = 1;
        m_pet[petPos]->qinmi = qinMi;
        m_pet[petPos]->qiCheng = qiCheng;
        
        CSocketServer &sock = SingletonSocket::instance();
        CNetMessage msg;
        msg.SetType(PRO_UPDATE_PET);
        msg<<(uint8)2<<petPos;
        NoLockMakePetInfo(petPos,msg);
        sock.SendMsg(m_sock,msg);
        
        string end;
        HexToStr(*m_pet[petPos].get(),end);
        SaveUseItem(m_roleId,m_package[itemPos],"对宠物使用",1,before,end);
        
        NoLockDelPackage(itemPos);
    }
    else if((pItem->id == 1804) || (pItem->id == 1822))
    {
        int *pLeftHp = (int*)pInst->addAttrVal;
        if(*pLeftHp == 0)
            *pLeftHp = pItem->addXue;
        int addHp = pItem->addXue;
        if(addHp > (pPet->maxHp - pPet->hp))
            addHp = pPet->maxHp - pPet->hp;
        if(addHp > pItem->addQiXue)
            addHp = pItem->addQiXue;
            
        if(addHp >= *pLeftHp)
        {
            addHp = *pLeftHp;
            NoLockDelPackage(itemPos);
        }
        *pLeftHp -= addHp;
        pPet->hp += addHp;
        if(addHp > 0)
            UpdatePetInfo(petPos,5,pPet->hp);
        if(pAddHp != NULL)
            *pAddHp = addHp;
        if(*pLeftHp > 0)
        {
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[itemPos],itemPos,msg);
            sock.SendMsg(m_sock,msg);
        }
    }
    else if(pItem->id == 1805)
    {
        int *pLeftMp = (int*)pInst->addAttrVal;
        if(*pLeftMp == 0)
            *pLeftMp = pItem->addXue;
        int addMp = pItem->addXue;
        if(addMp > (pPet->maxMp - pPet->mp))
            addMp = pPet->maxMp - pPet->mp;
        if(addMp > pItem->addQiXue)
            addMp = pItem->addQiXue;
            
        if(addMp >= *pLeftMp)
        {
            addMp = *pLeftMp;
            NoLockDelPackage(itemPos);
        }
        *pLeftMp -= addMp;
        pPet->mp += addMp;
        if(addMp > 0)
            UpdatePetInfo(petPos,6,pPet->mp);
        if(pAddMp != NULL)
            *pAddMp = addMp;
        if(*pLeftMp > 0)
        {
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[itemPos],itemPos,msg);
            sock.SendMsg(m_sock,msg);
        }
    }
    else if((pItem->type == EITPKYaoPin) || (pItem->type == EITNormalYaoPin))
    {
        uint8 mod = pItem->mod;
        int addHp = 0;
        int addMp = 0;
        if((mod & EIMTHpPer) != 0)
        {
            addHp = CalculateRate((int)pPet->maxHp,(int)pItem->addQiXue,100);
        }
        if((mod & EIMTHpFix) != 0)
        {
            addHp = pItem->addQiXue;
        }
        if((mod & EIMTMpPer) != 0)
        {
            addMp = CalculateRate((int)pPet->maxHp,(int)pItem->addFaLi,100);
        }
        if((mod & EIMTMpFix) != 0)
        {
            addMp = pItem->addFaLi;
        }
        if((mod & EIMTZhongcheng) != 0)
        {
            pPet->zhongcheng += pItem->addQiXue;
            if(pPet->zhongcheng > 100)
                pPet->zhongcheng = 100;
            UpdatePetInfo(petPos,13,pPet->zhongcheng);
        }
        if((mod & EIMTShouming) != 0)
        {
            pPet->shouming += pItem->addQiXue;
            pPet->shouMingTime = GetSysTime();
            if(pPet->shouming > 10000)
                pPet->shouming = 10000;
            UpdatePetInfo(petPos,12,pPet->shouming);
        }
        if(pAddHp != NULL)
            *pAddHp = addHp;
        if(pAddMp != NULL)
            *pAddMp = addMp;
        pPet->hp += addHp;
        
        pPet->mp += addMp;
        if(pPet->hp > pPet->maxHp)
            pPet->hp = pPet->maxHp;
        if(pPet->mp > pPet->maxMp)
            pPet->mp = pPet->maxMp;
        if(addHp > 0)
            UpdatePetInfo(petPos,5,pPet->hp);
        if(addMp > 0)
            UpdatePetInfo(petPos,6,pPet->mp);
        NoLockDelPackage(itemPos);
    }
    return true;
}

void CUser::XunYangPet(uint8 pos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_tili < 30)
        return;
    if(pos > m_petNum)
        return;
        
    SPet *pPet = m_pet[pos].get();
    if(pPet == NULL)
        return;
        
    AddTili(-30);
    pPet->zhongcheng += 30;
}

void CUser::DelPet(uint8 pos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if((pos >= m_petNum) || (pos >= MAX_PET_NUM))
        return;
    SaveDelPet(m_roleId,m_pet[pos].get());
    NoLockDelPet(pos);
}

void CUser::UpdatePetToBaby(uint8 pos)
{
    if(pos >= m_petNum)
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_pet[pos].get() == NULL)
        return;
        
    //CMonsterManager &monsterMgr = SingletonMonsterManager::instance();
    //ShareMonsterPtr monster = monsterMgr.CreateMonster(m_pet[pos]->tmplId,EMTBaoBao);
    //m_pet[pos] = monsterMgr.CreatePet(monster.get()); 
    SPet *pPet = m_pet[pos].get();
    sprintf(pPet->name,"%s","稀有青蛙宝宝");
    pPet->hpCZ = 75;//
    pPet->mpCZ = 70;//法术成长                        
    pPet->speedCZ = 40;//速度成长                     
    pPet->attackCZ = 60;//物攻成长                    
    pPet->skillAttackCZ = 10;//技能功能成长           
    pPet->type = EMTBaoBao;
    pPet->level = 15;
    pPet->qinmi = 0;        
    pPet->exp = 0;
    pPet->shuxingdian = 0;
    pPet->zhongcheng = 100;
    pPet->liliang = 30; 
    pPet->lingxing = 30;
    pPet->tizhi = 30;
    pPet->naili = 30;
    pPet->minjie = 30;
    pPet->shouming = 1000;
    pPet->bangDing = 1;
    pPet->Init();
    pPet->hp = pPet->maxHp;
    pPet->mp = pPet->maxMp;
    
    SetGuanKanPet(pos);
    SetChuZhanPet(pos);
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PET);
    msg<<(uint8)2<<pos;
    NoLockMakePetInfo(pos,msg);
    sock.SendMsg(m_sock,msg);
    
    
    //UpdatePetInfo(pos,1,7);
}

void CUser::NoLockDelPet(uint8 pos)
{
    if(pos >= m_petNum)
        return;
        
    if((m_chuZhanPet > pos) && (m_chuZhanPet < m_petNum))
    {
        m_chuZhanPet--;
    }
    else if(m_chuZhanPet == pos)
    {
        m_chuZhanPet = 0xff;
    }
    
    if((m_gensuiPet > pos) && (m_gensuiPet < m_petNum))
    {
        m_gensuiPet--;
    }
    else if(m_gensuiPet == pos)
    {
        m_gensuiPet = 0xff;
    }
    
    if((m_qiPet > pos) && (m_qiPet < m_petNum))
    {
        m_qiPet--;
    }
    else if(m_qiPet == pos)
    {
        m_qiPet = 0xff;
    }
    
    m_petNum--;
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PET);
    msg<<(uint8)0<<pos;
    sock.SendMsg(m_sock,msg);
    
    if(pos == (MAX_PET_NUM - 1))
    {
        m_pet[pos].reset();// = SharePetPtr();
        return;
    }
    for(uint8 i = pos; i < MAX_PET_NUM-1; i++)
    {
        m_pet[i] = m_pet[i+1];
    }
    m_pet[m_petNum].reset();
}

void CUser::UpdatePet(uint8 pos)
{
    if(pos >= m_petNum)
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PET);
    msg<<(uint8)2<<pos;
    NoLockMakePetInfo(pos,msg);
    sock.SendMsg(m_sock,msg);
}

bool CUser::GetChuZhanPet(SharePetPtr &pet)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_chuZhanPet > m_petNum)
        return false;
    pet = m_pet[m_chuZhanPet];
    return pet.get() != NULL;
}

SPet CUser::GetCZPet()
{
    SPet pet = {0};
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if((m_chuZhanPet > m_petNum) || (m_pet[m_chuZhanPet].get() == NULL))
        return pet;
    pet = *m_pet[m_chuZhanPet].get();
    return pet;
}

void CUser::SetCZPetWuXue(int wuxue)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if((m_chuZhanPet > m_petNum) || (m_pet[m_chuZhanPet].get() == NULL))
        return;
    m_pet[m_chuZhanPet]->wuxue = wuxue;
    UpdatePetInfo(m_chuZhanPet,3,wuxue);
}

void CUser::DelMission(int id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    int dbId = 0;
    for(int i = 0; i < MAX_MISSION_NUM; i++)
    {
        if(m_mission[i].id == id)
        {
            dbId = m_mission[i].dbId;
            if(i == MAX_MISSION_NUM - 1)
            {
                m_mission[i].id = 0;
            }
            else
            {
                for(; i < MAX_MISSION_NUM - 1; i++)
                {
                    if(m_mission[i+1].id != 0)
                    {
                        m_mission[i] = m_mission[i+1];
                    }
                    else
                    {
                        m_mission[i].id = 0;
                        break;
                    }
                }
                m_mission[MAX_MISSION_NUM-1].id = 0;
            }
            CGetDbConnect getDb;
            CDatabaseSql *pDb = getDb.GetDbConnect();
            boost::format fmt("delete from role_mission where id=%1%");
            fmt % dbId;
            if (pDb != NULL)
            {
                pDb->Query(fmt.str().c_str());
            }
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_TASK);
            msg<<(uint8)0<<(uint16)id;
            sock.SendMsg(m_sock,msg);
            return;
        }
    }
}

bool CUser::MissionFull()
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    for(int i = 0; i < MAX_MISSION_NUM; i++)
    {
        if(m_mission[i].id == 0)
        {
            return false;
        }
    }
    return true;
}

bool CUser::AddMission(int id,const char *pMiss)//任务
{
    if(pMiss == NULL)
        return false;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    for(int i = 0; i < MAX_MISSION_NUM; i++)
    {
        if(m_mission[i].id == id)
        {
            return false;
        }
    }
    
    for(int i = 0; i < MAX_MISSION_NUM; i++)
    {
        if(m_mission[i].id == 0)
        {
            CGetDbConnect getDb;
            CDatabaseSql *pDb = getDb.GetDbConnect();
            boost::format fmt("INSERT INTO role_mission (role_id,mission_id,mission_info)"\
                        " VALUES (%1%,%2%,'%3%');");
            fmt % m_roleId % id % pMiss;
            if ((pDb == NULL) || !pDb->Query(fmt.str().c_str()))
            {
                return false;
            }
            m_mission[i].dbId = pDb->InsertId();
            m_mission[i].id = id;
            m_mission[i].mission = pMiss;
            
            char *name = GetMissionName(id);
            if(name != NULL)
            {
                CSocketServer &sock = SingletonSocket::instance();
                CNetMessage msg;
                msg.SetType(PRO_UPDATE_TASK);
                msg<<(uint8)1<<(uint16)id<<name;
                sock.SendMsg(m_sock,msg);
            }
            return true;
        }
    }
    return false;
}

void CUser::UpdateMission(int id,const char *pMiss)//更新任务
{
    if(pMiss == NULL)
        return;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(int i = 0; i < MAX_MISSION_NUM; i++)
    {
        if(m_mission[i].id == id)
        {
            CGetDbConnect getDb;
            CDatabaseSql *pDb = getDb.GetDbConnect();
            boost::format fmt("update role_mission set mission_info='%1%' where id=%2%");
            fmt % pMiss % m_mission[i].dbId;
            if (pDb != NULL)
            {
                pDb->Query(fmt.str().c_str());
            }
            m_mission[i].id = id;
            m_mission[i].mission = pMiss;
            
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_TASK);
            msg<<(uint8)2<<(uint16)id;
            sock.SendMsg(m_sock,msg);
            break;
        }
    }
}

void CUser::MakeMission(CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    uint16 pos = msg.GetDataLen();
    uint8 num = 0;
    msg<<num;
    for(int i = 0; i < MAX_MISSION_NUM; i++)
    {
        if(m_mission[i].id != 0)
        {
            char *name = GetMissionName(m_mission[i].id);
            if(name != NULL)
            {
                msg<<m_mission[i].id;
                msg<<name;
            }
            num++;
        }
    }
    msg.WriteData(pos,&num,sizeof(num));
}

void CUser::GetHotList(list<HotInfo> &hotList)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    hotList = m_hotList;
}

void CUser::GetHotList(list<uint32> &hotList)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    hotList.clear();// = m_hotList;
    for(list<HotInfo>::iterator i = m_hotList.begin(); i != m_hotList.end(); i++)
    {
        hotList.push_back(i->hotId);
    }
}

bool CUser::SetHotVal(uint32 id,uint16 val)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(list<HotInfo>::iterator i = m_hotList.begin(); i != m_hotList.end(); i++)
    {
        if(i->hotId == id)
        {
            i->hotVal = val;
            return true;
        }
    }
    return false;
}

bool CUser::GetHotVal(uint32 id,uint16 &val)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(list<HotInfo>::iterator i = m_hotList.begin(); i != m_hotList.end(); i++)
    {
        if(i->hotId == id)
        {
            val = i->hotVal;
            return true;
        }
    }
    return false;
}

bool CUser::AddHot(uint32 id,uint16 hotVal)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_hotList.size() > MAX_HOT_NUM)
        return false;
    for(list<HotInfo>::iterator i = m_hotList.begin(); i != m_hotList.end(); i++)
    {
        if(i->hotId == id)
        {
            return false;
        }
    }
    HotInfo hot = {id,hotVal};
    m_hotList.push_back(hot);
    return true;
}

bool CUser::AddHot(uint32 id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_hotList.size() > MAX_HOT_NUM)
        return false;
    for(list<HotInfo>::iterator i = m_hotList.begin(); i != m_hotList.end(); i++)
    {
        if(i->hotId == id)
        {
            return false;
        }
    }
    HotInfo hot = {id,0};
    m_hotList.push_back(hot);
    return true;
}

void CUser::DelHot(uint32 id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(list<HotInfo>::iterator i = m_hotList.begin(); i != m_hotList.end(); i++)
    {
        if(i->hotId == id)
        {
            m_hotList.erase(i);
            return;
        }
    }
}

int CUser::EmptyPackage()
{
    uint8 num = 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if(m_package[i].tmplId == 0)
        {
            num++;
        }
    }
    return num;
}

bool CUser::TakeDownShopItem(uint8 pos)
{
    if(pos >= MAX_SHOP_ITEM_NUM)
        return false;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_shopItem[pos].type == SSITItem)
    {
        for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
        {
            if(m_package[i].tmplId == 0)
            {
                m_package[i] = m_shopItem[pos].item;
                memset(m_shopItem + pos,0,sizeof(SShopItem));
                
                CSocketServer &sock = SingletonSocket::instance();
                CNetMessage msg;
                msg.SetType(PRO_UPDATE_PACK);
                MakePack(m_package[i],i,msg);
                sock.SendMsg(m_sock,msg);
                return true;
            }
        }
    }
    else if(m_shopItem[pos].type == ESITPet)
    {
        uint8 maxNum = min(3 + m_level/10,(int)MAX_PET_NUM);
        if(m_petNum >= maxNum)
        {
            return false;
        }
        SPet *pPet = new SPet;
        *pPet = m_shopItem[pos].pet;
        SharePetPtr pet(pPet);
        NoLockAddPet(pet);
        
        memset(m_shopItem + pos,0,sizeof(SShopItem));
        return true;
    }
    return false;
}

bool CUser::CanSale(SItemInstance &item)
{
    if(item.bangDing == 1)
        return false;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
    if((pItem == NULL) || (pItem->type == EITTeShu) || (pItem->type == EITMission)
        || (pItem->type == EITCanDelMiss))
        return false;
    return true;
}

uint16 CUser::PutItemToShop(uint8 type,uint8 &pos,uint8 num,int money)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(type == SSITItem)
    {
        if(num <= 0)
            return 0;
        if(pos >= MAX_PACKAGE_NUM)
            return 0;
        if(m_package[pos].tmplId == 0)
            return 0;
        if(!CanSale(m_package[pos]))
            return 0;
            
        for(uint8 i = 0; i < MAX_SHOP_ITEM_NUM; i++)
        {
            if(m_shopItem[i].type == 0)
            {
                if(num > m_package[pos].num)
                    return 0;
                    
                uint16 id = m_package[pos].tmplId;
                m_shopItem[i].type = SSITItem;
                m_shopItem[i].money = money;
                m_shopItem[i].item = m_package[pos];
                m_shopItem[i].item.num = num;
                NoLockDelPackage(pos,num);
                pos = i;
                m_shopItemId[i] = GetShopItemId();
                return id;
            }
        }
    }
    else if(type == ESITPet)
    {
        if(pos > m_petNum)
            return 0;
        if(m_pet[pos].get() == NULL)
            return 0;
        if((m_pet[pos]->bangDing == 1) || (m_pet[pos]->kaiJia.tmplId != 0))
            return 0;
        for(uint8 i = 0; i < MAX_SHOP_ITEM_NUM; i++)
        {
            if(m_shopItem[i].type == 0)
            {
                m_shopItem[i].type = ESITPet;
                m_shopItem[i].money = money;
                m_shopItem[i].pet = *m_pet[pos].get();
                uint16 id = m_pet[pos]->tmplId;
                NoLockDelPet(pos);
                pos = i;
                m_shopItemId[i] = GetShopItemId();
                return id;
            }
        }
    }
    return 0;
}

void CUser::MakeShopItemList(CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    uint8 num = 0;
    msg<<m_name;
    uint16 pos = msg.GetDataLen();
    msg<<num;
    for(uint8 i = 0; i < MAX_SHOP_ITEM_NUM; i++)
    {
        if(m_shopItem[i].type == SSITItem)
        {
            msg<<i<<m_shopItem[i].item.tmplId<<(uint8)0<<m_shopItem[i].item.num<<m_shopItem[i].money;
            msg<<m_shopItemId[i];
            num++;
        }
        else if(m_shopItem[i].type == ESITPet)
        {
            msg<<i<<(uint16)m_shopItem[i].pet.tmplId<<(uint8)1<<(uint8)1<<m_shopItem[i].money;
            msg<<m_shopItemId[i];
            num++;
        }
    }
    msg.WriteData(pos,&num,sizeof(num));
}

bool CUser::BuyUserShopItem(CUser *pUser,uint8 pos,uint32 shopItemId,string &str)
{
    if(pUser->GetRoleId() == m_roleId)
    {
        str = "不能购买";
        return false;
    }
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    boost::recursive_mutex::scoped_lock lk1(pUser->m_mutex);
    if(pos >= MAX_SHOP_ITEM_NUM)
    {
        str = "不能购买";
        return false;
    }
    if(pUser->m_shopItemId[pos] != shopItemId)
    {
        str = "物品或者价格已修改，刷新后再购买";
        return false;
    }
    if((m_money < pUser->m_shopItem[pos].money) || (pUser->m_shopItem[pos].money < 0))
    {
        str = "金钱不够";
        return false;
    }
        
    if(pUser->m_shopItem[pos].type == SSITItem)
    {
        if(pUser->m_shopItem[pos].item.tmplId == 1843)
            snprintf(pUser->m_shopItem[pos].item.name,sizeof(pUser->m_shopItem[pos].item.name),"%s",pUser->GetName());
        if(NoLockAddPackage(pUser->m_shopItem[pos].item,NULL))
        {
            AddMoney(-pUser->m_shopItem[pos].money);
            pUser->AddMoney(pUser->m_shopItem[pos].money);
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_PSHOP_SOLD);
            msg<<m_roleId<<m_name<<pos<<pUser->m_shopItem[pos].item.tmplId<<(uint8)SSITItem
                <<pUser->m_shopItem[pos].item.num<<pUser->m_shopItem[pos].money;
            sock.SendMsg(pUser->GetSock(),msg);
            //SendSysInfo(pUser,"您的东西已卖出!");
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pTmpl = itemMgr.GetItem(pUser->m_shopItem[pos].item.tmplId);
            if(pTmpl != NULL)
            {
                char buf[64];
                snprintf(buf,63,"%s 购买了你的 %s",m_name,pTmpl->name.c_str());
                SendSysChannelMsg(pUser,buf);
            }
            string item;
            HexToStr(pUser->m_shopItem[pos].item,item);
            SaveUserShopItem(m_roleId,pUser->GetRoleId(),pUser->m_shopItem[pos].money,item);
            
            memset(pUser->m_shopItem + pos,0,sizeof(SShopItem));
            return true;
        }
        else
        {
            str = "背包已满";
            return false;
        }
    }
    else if(pUser->m_shopItem[pos].type == ESITPet)
    {
        uint8 maxNum = min(3 + m_level/10,(int)MAX_PET_NUM);
        if(m_petNum >= maxNum)
        {
            str = "空间已满无法携带";
            return false;
        }
        if(pUser->m_shopItem[pos].pet.level > m_level)
        {
            str = "宠物等级过高无法交易";
            return false;
        }
        if(!UserCanGetPet(this,pUser->m_shopItem[pos].pet.tmplId))
        {
            str = "对方宠物品种过高";
            return false;
        }
        AddMoney(-pUser->m_shopItem[pos].money);
        pUser->AddMoney(pUser->m_shopItem[pos].money);
        
        SPet *pPet = new SPet;
        *pPet = pUser->m_shopItem[pos].pet;
        pPet->qinmi = 0;
        SMonsterTmpl *pTmpl = SingletonMonsterManager::instance().GetTmpl(pUser->m_shopItem[pos].pet.tmplId);
        if(pTmpl == NULL)
        {
            strncpy(pPet->name,pTmpl->name.c_str(),SPet::MAX_NAME_LEN);
            if(pPet->type == EMTBaoBao)
                strcat(pPet->name,"宝宝");
        }
        
        CSocketServer &sock = SingletonSocket::instance();
        CNetMessage msg;
        msg.SetType(PRO_PSHOP_SOLD);
        msg<<m_roleId<<m_name<<pos<<pPet->tmplId<<(uint8)ESITPet
            <<(uint8)1<<pUser->m_shopItem[pos].money;
        sock.SendMsg(pUser->GetSock(),msg);
        
        string strPet;
        HexToStr(pUser->m_shopItem[pos].pet,strPet);
        SaveUserShopPet(m_roleId,pUser->GetRoleId(),pUser->m_shopItem[pos].money,strPet);
            
        memset(pUser->m_shopItem + pos,0,sizeof(SShopItem));
        
        SharePetPtr pet(pPet);
        NoLockAddPet(pet);
        
        return true;
    }
    str = "商品已卖出";
    return false;
}

void CUser::MakeShopItemInfo(uint8 pos,CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    msg<<m_roleId<<pos;
    if(pos >= MAX_SHOP_ITEM_NUM)
    {
        msg.ReWrite();
        msg.SetType(PRO_PSHOP_DETAIL);
        msg<<PRO_ERROR;
        return;
    }

    if(m_shopItem[pos].type == SSITItem)
    {
        msg<<m_shopItem[pos].item.tmplId<<(uint8)0;
        MakeItemInfo(&(m_shopItem[pos].item),msg);
    }
    else if(m_shopItem[pos].type == ESITPet)
    {
        msg<<(uint16)m_shopItem[pos].pet.tmplId<<(uint8)1;
        MakePetInfo(&m_shopItem[pos].pet,0,msg);
    }
    else
    {
        msg.ReWrite();
        msg.SetType(PRO_PSHOP_DETAIL);
        msg<<PRO_ERROR;
    }
}

/********
+----+------+-----+-------+-----+-------+
| OP | TYPE | IND | TYPE2 | IND | COUNT |
+----+------+-----+-------+-----+-------+
|  1 |   1  |  1  |   1   |  1  |   1   |
+----+------+-----+-------+-----+-------+
OP=5 
TYPE=0
IND=0|1|2 交易格子
TYPE2=0 物品
TYPE2=1 宠物
IND 为背包中物品格子或者宠物索引

设置金钱
+----+------+-------+
| OP | TYPE | MONEY |
+----+------+-------+
|  1 |   1  |   4   |    
+----+------+-------+
TYPE=1
*************/
bool CUser::SetTradeItem(CNetMessage &msg)
{
    if(m_tradeOk)
        return false;
    uint8 type;
    msg>>type;
    if(type == 1)
    {
        if(m_tradeMoney == 0)
        {
            msg>>m_tradeMoney;
            if((m_tradeMoney < 0) || (m_tradeMoney > m_money))
            {
                m_tradeMoney = 0;
                return false;
            }
            return true;
        }
        return false;
    }
    else if(type == 0)
    {
        uint8 ind = 0;
        uint8 pos = 0;//包裹中位置
        uint8 num = 0;
        msg>>ind>>type>>pos>>num;
        if(ind >= MAX_TRADE_NUM)
            return false;
        if(m_tradeMD5[ind][0] != 0)
            return false;
        if(type == 0)
        {
            if(pos >= MAX_PACKAGE_NUM)
                return false;
            if((m_tradeItemPos[ind] >= 0) || (!CanSale(m_package[pos])))
            {
                return false;
            }
            if((num > m_package[pos].num) || (num <= 0))
            {
                return false;
            }
        }
        else if(type == 1)
        {
            if(pos >= m_petNum)
                return false;
        }
        
        for(uint8 i = 0; i < MAX_TRADE_NUM; i++)
        {
            if((m_tradeItemPos[i] == pos) && (m_tradeType[i] == type))
            {
                return false;
            }
        }
        
        m_tradeItemPos[ind] = pos;
        m_tradeType[ind] = type;
        m_tradeNum[ind] = num;
        if(type == 0)
        {
            md5((uint8*)(m_package+pos),sizeof(m_package[pos]),m_tradeMD5[ind]);
            //cout<<m_package[pos].tmplId<<endl;
            //cout<<(int)pos<<endl;
            msg<<m_package[pos].tmplId;
            return true;
        }
        else if(type == 1)
        {
            SPet *pPet = m_pet[pos].get();
            if((pPet == NULL) || (pPet->bangDing == 1)
                || (pPet->kaiJia.tmplId != 0))
            {
                return false;
            }
            md5((uint8*)pPet,sizeof(SPet),m_tradeMD5[ind]);
            msg<<(uint16)pPet->tmplId;
            return true;
        }
        return false;
    }
    return false;
    /*uint8 num;
    for(uint8 i = 0; i < MAX_TRADE_NUM; i++)
    {
        msg>>m_tradeType[i]>>m_tradeItemPos[i]>>num;
        if(m_tradeType[i] > 1)
            return false;
    }
    msg>>m_tradeMoney;
    if((m_tradeMoney > m_money) || (m_tradeMoney < 0))
    {
        return false;
    }
    return true;*/
}

void CUser::GetTradeInfo(CNetMessage &msg)
{
    msg<<(uint8)5;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i < MAX_TRADE_NUM; i++)
    {
        if(m_tradeType[i] == 0)
        {//物品
            msg<<m_tradeType[i];
            if((m_tradeItemPos[i] >= 0) && (m_tradeItemPos[i] < MAX_PACKAGE_NUM))
            {
                uint8 pos = m_tradeItemPos[i];
                msg<<m_tradeItemPos[i]<<m_package[pos].tmplId<<m_package[pos].num;
            }
            else
            {
                msg<<(char)-1<<(uint16)0<<(uint8)0;
            }
        }
        else if(m_tradeType[i] == 1)
        {//宠物
            msg<<m_tradeType[i];
            if((m_tradeItemPos[i] >= 0) && (m_tradeItemPos[i] < MAX_PET_NUM))
            {
                uint8 pos = m_tradeItemPos[i];
                SPet *pPet = m_pet[pos].get();
                if(pPet != NULL)
                    msg<<m_tradeItemPos[i]<<(uint16)pPet->tmplId<<(uint8)1;
                else
                    msg<<(char)-1<<(uint16)0<<(uint8)0;
            }
            else
            {
                msg<<(char)-1<<(uint16)0<<(uint8)0;
            }
        }
    }
    msg<<m_tradeMoney;
}

bool CUser::GetTradeItem(SItemInstance *pItem[MAX_TRADE_NUM],uint8 &num)
{
    num = 0;
    for(uint8 i = 0; i < MAX_TRADE_NUM; i++)
    {
        pItem[i] = NULL;
        if(m_tradeType[i] == 0)
        {//物品
            uint8 pos = m_tradeItemPos[i];
            if(pos >= MAX_PACKAGE_NUM)
                return false;
            //cout<<"交易数量"<<(int)m_tradeNum[i]<<endl;
            //cout<<"包中数量"<<(int)m_package[pos].num<<endl;
            if((m_package[pos].tmplId != 0) && (m_tradeNum[i] <= m_package[pos].num))
            {
                //cout<<(int)pos<<endl;
                //cout<<m_package[pos].tmplId<<endl;
                uint8 md5Result[MD5_RESULT_SIZE];
                md5((uint8*)(m_package+pos),sizeof(m_package[pos]),md5Result);
                if(memcmp(md5Result,m_tradeMD5[i],sizeof(md5Result)) != 0)
                    return false;
                
                pItem[i] = m_package + pos;
                num++;
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

bool CUser::GetTradePet(SharePetPtr *pPet[MAX_TRADE_NUM],uint8 &num)
{
    num = 0;
    for(uint8 i = 0; i < MAX_TRADE_NUM; i++)
    {
        pPet[i] = NULL;
        if(m_tradeType[i] == 1)
        {//宠物
            uint8 pos = m_tradeItemPos[i];
            SharePetPtr pet = m_pet[pos];
            if((pos < MAX_PET_NUM) && (pet.get() != NULL))
            {
                uint8 md5Result[MD5_RESULT_SIZE];
                md5((uint8*)pet.get(),sizeof(SPet),md5Result);
                if(memcmp(md5Result,m_tradeMD5[i],sizeof(md5Result)) != 0)
                    return false;
                    
                pPet[num] = m_pet + pos;
                num++;
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

bool CUser::CanSavePackage(SItemInstance **pItem,uint8 num)
{
	if(num == 0)
		return true;
		
    uint8 emptyNum = 0;
    for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if(m_package[i].tmplId == 0)
        {
            emptyNum++;
        }
    }
    return emptyNum >= num;
}

uint8 CUser::CanSavePetNum()
{
    uint8 maxNum = min(3 + m_level/10,(int)MAX_PET_NUM);
    return maxNum - m_petNum;
}

void CUser::DelTradeItem()
{
    uint8 delPet[MAX_TRADE_NUM] = {0};
    uint8 num = 0;
    for(uint8 pos = 0; pos < MAX_TRADE_NUM; pos++)
    {
        if(m_tradeType[pos] == 0)
        {//物品
            NoLockDelPackage(m_tradeItemPos[pos],m_tradeNum[pos]);
        }
        else if(m_tradeType[pos] == 1)
        {//宠物
            //NoLockDelPet(m_tradeItemPos[pos]);
            delPet[num] = m_tradeItemPos[pos];
            num++;
        }
    }
    std::sort(delPet,delPet+num);
    if(num > 0)
    {
        for(int i = num-1; i >= 0; i--)
        {
            NoLockDelPet(delPet[i]);
        }
    }
}
//SharePetPtr *pPet[MAX_TRADE_NUM],uint8 &num)
bool CUser::CanGetPet(SharePetPtr *pPet[MAX_TRADE_NUM],uint8 num)
{
    for(uint8 i = 0; i < num; i++)
    {
        //if((pPet[i] != NULL) && (pPet[i]->get() != NULL))
            //cout<<pPet[i]->get()->name<<" "<<pPet[i]->get()->tmplId<<endl;
        if((pPet[i] == NULL) || (pPet[i]->get() == NULL) 
            || !UserCanGetPet(this,pPet[i]->get()->tmplId))
            return false;
    }
    return true;
}

bool CUser::TradeItem(CUser *pUser)
{
    if(pUser->GetRoleId() == m_roleId)
        return false;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    boost::recursive_mutex::scoped_lock lk1(pUser->m_mutex);
    
    if((m_money < m_tradeMoney) || (m_tradeMoney < 0))
        return false;
    
    if((pUser->m_money < pUser->m_tradeMoney) || (pUser->m_tradeMoney < 0))
        return false;

    SItemInstance *meTradeItem[MAX_TRADE_NUM];
    uint8 meTradeNum = 0;
    if(!GetTradeItem(meTradeItem,meTradeNum))
        return false;
    if(!pUser->CanSavePackage(meTradeItem,meTradeNum))
    {
        SendSysInfo(pUser,"背包已满交易失败");//该宠物品种等级过高，无法交易给对方");
        SendSysInfo(this,"对方背包已满失败");
        return false;
    }
    
    SItemInstance *otherTradeItem[MAX_TRADE_NUM];
    uint8 otherTradeNum = 0;
    if(!pUser->GetTradeItem(otherTradeItem,otherTradeNum))
        return false;
    if(!CanSavePackage(otherTradeItem,otherTradeNum))
    {
        SendSysInfo(this,"背包已满交易失败");
        SendSysInfo(pUser,"对方背包已满失败");
        return false;
    }
    
    SharePetPtr *meTradePet[MAX_TRADE_NUM];
    uint8 mePetNum = 0;
    if(!GetTradePet(meTradePet,mePetNum))
        return false;
    if(pUser->CanSavePetNum() < mePetNum)
    {
        SendSysInfo(pUser,"宠物数达上限");
        SendSysInfo(this,"对方宠物数达上限");
        return false;
    }
    for(uint8 i = 0; i < mePetNum; i++)
    {
        if((meTradePet[i] == NULL) || (meTradePet[i]->get() == NULL) 
            || (meTradePet[i]->get()->level > pUser->GetLevel()))
        {
            SendSysInfo(pUser,"宠物等级过高无法交易");
            SendSysInfo(this,"宠物等级过高无法交易");
            return false;
        }        
    }
    if(!pUser->CanGetPet(meTradePet,mePetNum))
    {
        SendSysInfo(pUser,"宠物品种过高");
        SendSysInfo(this,"对方宠物品种过高");
        return false;
    }
    
    SharePetPtr *otherTradePet[MAX_TRADE_NUM];
    uint8 otherPetNum = 0;
    if(!pUser->GetTradePet(otherTradePet,otherPetNum))
        return false;
    if(CanSavePetNum() < otherPetNum)
    {
        SendSysInfo(this,"宠物数达上限");
        SendSysInfo(pUser,"对方宠物数达上限");
        return false;
    }
    for(uint8 i = 0; i < otherPetNum; i++)
    {
        if((otherTradePet[i] == NULL) || (otherTradePet[i]->get() == NULL) 
            || (otherTradePet[i]->get()->level > m_level))
        {
            SendSysInfo(this,"宠物等级过高无法交易");
            SendSysInfo(pUser,"宠物等级过高无法交易");
            return false;
        }        
    }
    if(!CanGetPet(otherTradePet,otherPetNum))
    {
        SendSysInfo(this,"宠物品种过高");
        SendSysInfo(pUser,"对方宠物品种过高");
        return false;
    }
        
    uint8 i;
    string item1,item2,pet1,pet2;
    for(i = 0; i < MAX_TRADE_NUM; i++)
    {
        if(meTradeItem[i] != NULL)
        {
            SItemInstance item = *meTradeItem[i];
            item.num = m_tradeNum[i];
            if(item.tmplId == 1843)
                snprintf(item.name,sizeof(item.name),"%s",m_name);
            int addQinMi = 0;
            pUser->NoLockAddPackage(item,NULL,true,&addQinMi);
            uint16 hotVal = 0;
            if((addQinMi != 0) && GetHotVal(pUser->m_roleId,hotVal))
            {               
                SetHotVal(pUser->m_roleId,hotVal + addQinMi);
            }
            /*if(item.tmplId == 1844)
            {
                SetData32(10,GetData32(10)+5);
                pUser->SetData32(10,pUser->GetData32(10)+5);
            }*/
            if(item.tmplId == 1845)
            {
                SetData32(10,GetData32(10)+200*item.num);
                pUser->SetData32(10,pUser->GetData32(10)+200*item.num);
                
                pUser->SetData32(12,pUser->GetData32(12)+9*item.num);
            }
            HexToStr(item,item1);
            //DelTradeItem(i);
        }
        if(meTradePet[i] != NULL)
        {
            SPet *pPet = meTradePet[i]->get();
            if(pPet != NULL)
            {
                SMonsterTmpl *pTmpl = SingletonMonsterManager::instance().GetTmpl(pPet->tmplId);
                if(pTmpl != NULL)
                {
                    strcpy(pPet->name,pTmpl->name.c_str());
                    if(pPet->type == EMTBaoBao)
                        strcat(pPet->name,"宝宝");
                }
                pPet->qinmi = 0;
                if(pPet->zhongcheng > 30)
                    pPet->zhongcheng -= 30;
                else 
                    pPet->zhongcheng = 0;
                HexToStr(*pPet,pet1);
                pUser->NoLockAddPet(*meTradePet[i]);
                //DelTradeItem(i);
            }
        }
        if(otherTradeItem[i] != NULL)
        {
            SItemInstance item = *otherTradeItem[i];
            item.num = pUser->m_tradeNum[i];
            if(item.tmplId == 1843)
                snprintf(item.name,sizeof(item.name),"%s",pUser->GetName());
            int addQinMi = 0;
            NoLockAddPackage(item,NULL,true,&addQinMi);
            uint16 hotVal = 0;
            if((addQinMi != 0) && pUser->GetHotVal(m_roleId,hotVal))
            {               
                pUser->SetHotVal(m_roleId,hotVal + addQinMi);
            }
            /*if(item.tmplId == 1844)
            {
                SetData32(10,GetData32(10)+5);
                pUser->SetData32(10,pUser->GetData32(10)+5);
            }*/
            if(item.tmplId == 1845)
            {
                SetData32(10,GetData32(10)+200*item.num);
                pUser->SetData32(10,pUser->GetData32(10)+200*item.num);
                
                SetData32(12,GetData32(12)+9*item.num);
            }
            HexToStr(item,item2);
            //pUser->DelTradeItem(i);
        }
        if(otherTradePet[i] != NULL)
        {
            SPet *pPet = otherTradePet[i]->get();
            if(pPet != NULL)
            {
                SMonsterTmpl *pTmpl = SingletonMonsterManager::instance().GetTmpl(pPet->tmplId);
                if(pTmpl != NULL)
                    strcpy(pPet->name,pTmpl->name.c_str());
                pPet->qinmi = 0;
                if(pPet->zhongcheng > 30)
                    pPet->zhongcheng -= 30;
                else 
                    pPet->zhongcheng = 0;
                HexToStr(*pPet,pet2);
                NoLockAddPet(*otherTradePet[i]);
                //pUser->DelTradeItem(i);
            }
        }
    }    
    DelTradeItem();
    pUser->DelTradeItem();
    AddMoney(pUser->m_tradeMoney);
    AddMoney(-m_tradeMoney);
    
    pUser->AddMoney(m_tradeMoney);
    pUser->AddMoney(-pUser->m_tradeMoney);
    
    SaveTrade(m_roleId,m_tradeMoney,item1,pet1,pUser->GetRoleId(),pUser->m_tradeMoney,item2,pet2);
    
    pUser->m_tradeMoney = 0;
    m_tradeMoney = 0;
    
    return true;
}

void CUser::AddExp(int exp,bool callScript)            //经验
{
    if(exp < 0)
    {
        if(m_exp + exp < 0)
            exp = -m_exp;
    }
    /*if(m_exp >= MAX_EXP)
    {
        return;
    }*/
        
    int levelUpExp = GetLevelUpExp(m_level);
    if(m_exp + exp >= levelUpExp)
    {
        if(m_level+1 > MAX_LEVEL)
            return;
        AddLevel(callScript);
        m_exp = m_exp + exp - levelUpExp;
    }
    else
    {
        m_exp += exp;
    }
    SendUpdateInfo(8,exp,true);
}

void CUser::SendMsgToTeamMember(const char *msg)
{
    if(m_pScene == NULL)
        return;
    uint32 members[MAX_TEAM_MEMBER];
    uint8 num = m_pScene->GetTeamMem(m_teamId,members);
    
    for(uint8 i = 0; i < num; i++)
    {
        COnlineUser &m_onlineUser = SingletonOnlineUser::instance();
        ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(members[i]);
        CUser *pUser = ptr.get();
        if((pUser != NULL) && (pUser->GetRoleId() != m_roleId))
        {
            SendPopMsg(pUser,msg);
        }
    }
}

void CUser::AddTiZhi(int tizhi)        //体质
{
    //atomic_exchange_and_add((int*)&m_tizhi,tizhi);
    if(m_tizhi + tizhi < 0)
        return;
    m_tizhi += tizhi;
    //SendUpdateInfo(16,m_tizhi);
    SendUpdateInfo(16,GetTiZhi());
}

void CUser::AddLiLiang(int liliang)    //力量
{
    //atomic_exchange_and_add((int*)&m_liliang,liliang);
    if(m_liliang + liliang < 0)
        return;
    m_liliang += liliang;
    //SendUpdateInfo(17,m_liliang);
    SendUpdateInfo(17,GetLiLiang());
}

void CUser::AddMinJie(int minjie)      //敏捷
{
    if(m_minjie + minjie < 0)
        return;
    m_minjie += minjie;
    SendUpdateInfo(18,GetMinJie());
}

void CUser::AddLingLi(int lingli)      //灵力
{
    //atomic_exchange_and_add((int*)&m_lingli,lingli);
    if(m_lingli + lingli < 0)
        return;
    m_lingli += lingli;
    SendUpdateInfo(19,GetLingLi());
}

void CUser::AddNaiLi(int naili)        //耐力
{
    //atomic_exchange_and_add((int*)&m_naili,naili);
    if(m_naili + naili < 0)
        return;
    m_naili += naili;
    SendUpdateInfo(20,GetNaiLi());
}

void CUser::SendUpdateInfo(uint8 type,int val,bool add)
{
    CSocketServer &sock = SingletonSocket::instance();
    uint8 op = 0;
    if(add)
        op = 1;
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_CHAR);
    msg<<op<<type<<val;
    sock.SendMsg(m_sock,msg);
    /*if(type == 16)
    {
        cout<<m_name<<" 体质:"<<val<<endl;
    }*/
}

void CUser::AddShuXinDian(int shuxindian)//未分配属性点
{
    //atomic_exchange_and_add((int*)&m_shuxingdian,shuxindian);
    m_shuxingdian += shuxindian;
    SendUpdateInfo(4,m_shuxingdian);
}

void CUser::AddJin(int jin)            //金相性
{
    if(m_jin + jin < 0)
        return;
    m_jin += jin;
    SendUpdateInfo(21,GetJin());
}

void CUser::AddMu(int mu)              //木相性
{
    if(m_mu + mu< 0)
        return;
    m_mu += mu;
    SendUpdateInfo(22,GetMu());
}

void CUser::AddShui(int shui)          //水相性
{
    if(m_shui + shui < 0)
        return;
    m_shui += shui;
    SendUpdateInfo(23,GetShui());
}

void CUser::AddHuo(int huo)//火相性
{
    if(m_huo + huo < 0)
        return;
    m_huo += huo;
    SendUpdateInfo(24,GetHuo());
}

void CUser::AddTu(int tu)//土相性
{
    if(m_tu + tu < 0)
        return;
    m_tu += tu;
    SendUpdateInfo(25,GetTu());
}

void CUser::AddXiangXinDian(int xiangxindian)//未分配相性点
{
    m_xiangxingdian += xiangxindian;
    SendUpdateInfo(5,m_xiangxingdian);
}

void CUser::AddHp(int hp)//气血
{
    //atomic_exchange_and_add((int*)&m_hp,hp);
    m_hp += hp;
    if(m_hp > GetMaxHp())
        m_hp = GetMaxHp();
    else if(m_hp <= 0)
        m_hp = 0;
    SendUpdateInfo(9,m_hp);
}

void CUser::AddMp(int mp)//法力
{
    //atomic_exchange_and_add((int*)&m_mp,mp);
    m_mp += mp;
    if(m_mp > m_maxMp)
        m_mp = m_maxMp;
    else if(m_mp <= 0)
        m_mp = 0;
    SendUpdateInfo(11,m_mp);
}

void CUser::AddDamage(int damage)
{
    m_damage += damage;
    //atomic_exchange_and_add((int*)&m_damage,damage);
}

void CUser::AddSkillDamage(int skillDamage)
{
    atomic_exchange_and_add((int*)&m_skillDamage,skillDamage);
}

void CUser::AddRecovery(int recovery)
{
    atomic_exchange_and_add((int*)&m_recovery,recovery);
}

void CUser::AddSpeed(int speed)
{
    atomic_exchange_and_add((int*)&m_speed,speed);
}

void CUser::SetVal(int id,int val)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    m_intMap[id] = val;
    //m_intMap.insert(pair <int, int> (id,val));
}

int CUser::GetVal(int id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    map<int,int>::iterator i = m_intMap.find(id);
    if(i != m_intMap.end())
        return i->second;
    return 0;
}

void CUser::Clear()
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    m_userOp =          0;
    m_userPara =        0;
    m_target =          0;
    m_petOp =           0;
    m_petPara =         0; 
    m_petTar =          0;
    m_autoFightTurn =   0;
    SetFight(0,0);
    
    m_logout = false;
    m_step = 0;
    m_roleId = 0;
    m_jianYuTime = 0;
    m_hotList.clear();
    //m_scriptHeap.clear();
    m_inJump = false;
    for(int i = 0; i < MAX_MISSION_NUM; i++)
    {
        m_mission[i].dbId = 0;
        m_mission[i].id = 0;
        m_mission[i].mission.empty();
    }
    memset(m_role,0,sizeof(m_role));
    for(int i = 0; i < MAX_PET_NUM; i++)
    {
        m_pet[i].reset();
    }
    m_petNum = 0;//宠物数量
    m_chuZhanPet = 0xff;//出战宠物
    m_gensuiPet = 0xff;//跟随宠物
    
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        delete i->pNpc;
        delete i->pHumanData;
        i->pNpc = NULL;
        i->pHumanData = NULL;
    }
    m_npcList.clear();
    m_bitset.reset();
    m_saveDataTime = GetSysTime();
}

void CUser::LoadMission()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    boost::format fmt("select id,mission_id,mission_info from role_mission where role_id = %1%");
    fmt % m_roleId;
    if ((pDb == NULL) || !pDb->Query(fmt.str().c_str()))
    {
        return;
    }
    char **row;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    int num = 0;
    
    while((row = pDb->GetRow()) != NULL)
    {
        m_mission[num].dbId = atoi(row[0]);
        m_mission[num].id = atoi(row[1]);
        m_mission[num].mission = row[2];
        ++num;
        if(num >= MAX_MISSION_NUM)
            break;
    }
}

bool CUser::GetHumanData(uint32 roleId,HumanData &human,string &name)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    char sql[512];
    
    sprintf(sql,"select equipment,sex,name from role_info where id=%d",roleId);
    if ((pDb == NULL) || (!pDb->Query(sql)))
    {
        return false;
    }
    char **row = pDb->GetRow();
    if(row == NULL)
        return false;
    if(row != NULL)
    {
        SItemInstance *pEquip = new SItemInstance[EQUIPMENT_NUM];
        StrToHex(row[0],(uint8*)pEquip,sizeof(m_equipment));
        //StrToHex(pEquip,(uint8*)m_equipment,sizeof(m_equipment));
        human.weapon = pEquip[EETWuQi].tmplId;
        human.helmet = pEquip[EETMaoZi].tmplId;
        human.armor = pEquip[EETKuiJia].tmplId;
        human.level = pEquip[EETWuQi].level;
        human.sex = atoi(row[1]);
        human.helmetClass = pEquip[EETMaoZi].quality;
        human.armorClass = pEquip[EETKuiJia].quality;
        name = row[2];
        delete []pEquip;
        return true;
    }
    return false;
}
void CUser::SetNpc(char *row)
{
    if(row == NULL)
        return;
        
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        delete i->pNpc;
        delete i->pHumanData;
        i->pNpc = NULL;
        i->pHumanData = NULL;
    }
    m_npcList.clear();
    
    char *p[70];
    uint8 num = SplitLine(p,70,row);
    
    SNpcInstance npc;
    CNpcManager &npcManager = SingletonNpcManager::instance();
    
    for(uint8 i = 0; (i+7) <= num; i+=7)
    {
        npc.id = atoi(p[i]);
        npc.sceneId = atoi(p[i+1]);
        npc.x = atoi(p[i+2]);
        npc.y = atoi(p[i+3]);
        npc.timeOut = atoi(p[i+4]);
        SNpcTemplate *pNpc = npcManager.GetNpcTemplate(npc.id);
        if(pNpc == NULL)
            continue; 
        npc.pNpc = new SNpcTemplate;
        *(npc.pNpc) = *pNpc;
        npc.pNpc->name = p[i+5];
        uint32 roleId = atoi(p[i+6]);
        if(roleId != 0)
        {
            HumanData human;
            if(GetHumanData(roleId,human,npc.pNpc->name))
            {
                npc.type = 1;
                npc.pHumanData = new HumanData;
                human.roleId = roleId;
                *(npc.pHumanData) = human;
            }
        }
        m_npcList.push_back(npc);
    }
}

void CUser::SaveData()
{
    if(GetSysTime() - m_saveDataTime > SAVE_DATA_SPACE)
    {
        m_saveDataTime = GetSysTime();
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        if(pDb != NULL)
            NoLockSaveData(pDb);
    }
}

void CUser::NoLockSaveData(CDatabaseSql *pDb)
{
    if(m_roleId == 0)
        return;
        
    boost::format fmt("update role_info set sex=%2%,head=%3%,xiang=%4%,map_id=%5%,"\
        "x_pos=%6%,y_pos=%7%,level=%8%,exp=%9%,tizhi=%10%,liliang=%11%,minjie=%12%,lingli=%13%,"\
        "naili=%14% ,shuxindian=%15% ,jin=%16% ,mu=%17% ,shui=%18% ,huo=%19% ,tu=%20% ,"\
        "xiangxingdian=%21% ,hp=%22% ,mp=%23%,equipment=\"%24%\",package=\"%25%\",tili=%26%,"\
        "money=%27%,pet=\"%28%\",pk_val=%29%,daohang=%30%,title='%31%',hots=\"%32%\","\
        "bitset=\"%33%\",shop=\"%34%\",menpai=%35%,bangpai=%36%,save_val='%37%',"\
        "save_npc='%38%',qianneng=%39%,mp_gongxian=%40%,skills='%41%',"\
        "use_double_end=%42%,pet_double_end=%43%,no_punish_end=%44%,script_timer=\'%45%\'"\
        ",chat_channel=%46%,bank_money=%47%,bank_item='%48%' "\
        ",few_monster_end=%49%,max_monster_end=%50%,min_monster_end=%51%,up_monster_end=%52%"\
        ",monster_script='%53%',save_pos='%54%',jianyu_time=%55%,aq_time_space=%56%,"\
        "chat_time=%57%,pk_time=%58%,open_pack=%59%,script_double=%60%,save_data='%61%'"\
        ",login_time=%62%,state=~1&state where id = %1%");
    
    SetData16(4,GetData16(4)+(GetSysTime()-m_loginTime)/60);
    
    SetData32(12,GetData32(12)+(GetSysTime()-m_loginTime)/3600*9);
/*#ifdef QQ
    int add = (GetSysTime()-m_loginTime)/60/15*10;
    if(GetData16(7) + add > 400)
        add = 400 - GetData16(7);
    SetData16(7,GetData16(7) + add);
    SetData16(6,GetData16(6) + add);
#else
    
    int add = (GetSysTime()-m_loginTime)/3600*20;
    if(GetData16(7) + add > 100)
        add = 100 - GetData16(7);
    if(add > 0)
    {
        SetData16(7,GetData16(7) + add);
        SetData16(6,GetData16(6) + add);
    }
#endif*/
    m_loginTime = GetSysTime();
    
    string equip;
    string pack;
    string pet;
    string hots;
    string bit;
    string shop;
    string bankItem;
    string savaData;
    
    stringstream saveShort;
    stringstream npc;
    stringstream skills;
    stringstream scriptTimer;
    stringstream monsterScript;
    stringstream savePos;
    
    for(uint8 i = 0; i < MAX_TIMER; i++)
    {
        if((m_scriptTimer[i].id != 0) && (m_scriptTimer[i].endTime != 0))
        {
            scriptTimer<<(int)m_scriptTimer[i].id<<'|'<<(int)m_scriptTimer[i].endTime<<'|';
        }
    }
    for(int i = 0; i < MAX_SAVE_NUM; i++)
    {
        saveShort<<(int)m_shortArray[i];
        if(i != MAX_SAVE_NUM - 1)
            saveShort<<'|'; 
    }
    
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        npc<<(int)i->id<<'|'
        <<(int)i->sceneId<<'|'
        <<(int)i->x<<'|'
        <<(int)i->y<<'|'
        <<(int)i->timeOut<<'|'
        <<i->pNpc->name<<'|';
        if((i->type != 0) && (i->pHumanData != NULL))
            npc<<(int)i->pHumanData->roleId<<'|';
        else
            npc<<0<<'|';
    }
    
    list<UserSkill>::iterator iSkill = m_userSkill.begin();
    for(; iSkill != m_userSkill.end(); iSkill++)
    {
        skills<<(int)iSkill->id<<'|'<<(int)iSkill->level<<'|';
    } 
    multimap<int,int>::iterator iterMonster = m_monsterScript.begin();
    for(; iterMonster != m_monsterScript.end(); iterMonster++)
    {
        monsterScript<<(int)iterMonster->first<<'|'<<(int)iterMonster->second<<'|';
    }
    for(uint8 i = 0; i < MAX_SAVE_POS; i++)
    {
        savePos<<m_savePos[i].sceneId<<'|'
            <<(int)m_savePos[i].x<<'|'
            <<(int)m_savePos[i].y<<'|';
    }
    GetEquipment(equip);
    GetPackage(pack);
    GetPet(pet);
    WriteHots(hots);
    GetBitSet(bit);
    GetShop(shop);
    GetBankItem(bankItem);
    WriteSaveData(savaData);
    int mapId = 1;
    if(m_pScene != NULL)
    {
        mapId = m_pScene->GetMapId();
        if(m_pScene->InGuiYu())
        {
            //TransportUser(pUser,1,13,13,8);
            mapId = 27;
            m_xPos = 4,
            m_yPos = 22;
        }
        if((mapId >= 260) && (mapId <= 266))
        {
            mapId = BANG_PAI_SCENE_ID;
            m_xPos = 15;
            m_yPos = 15;
        }
        if(mapId == 307)
        {
            mapId = 11;
            m_xPos = 15;
            m_yPos = 21;
        }
        if(mapId == 306)
        {
            mapId = 24;
            m_xPos = 15;
            m_yPos = 9;
        }
    }
    string title;
    GetTitleStr(title);
    fmt % GetRoleId()
        % (int)m_sex%(int)m_head%(int)m_xiang%mapId
        % (int)m_xPos%(int)m_yPos%(int)m_level%m_exp%m_tizhi%m_liliang%m_minjie%m_lingli
        % m_naili%m_shuxingdian%m_jin%m_mu%m_shui%m_huo%m_tu
        % m_xiangxingdian%m_hp%m_mp
        % equip % pack
        % m_tili % m_money
        % pet % m_pkVal % m_daohang % title
        % hots % bit % shop % (int)m_menpai % m_bangpai
        % saveShort.str()
        % npc.str()
        % m_qianneng
        % m_menPaiGongXian
        % skills.str()
        % m_userDoubleEnd
        % m_petDoubleEnd
        % m_noPunishEnd
        % scriptTimer.str()
        % (int)m_chatChannel
        % m_bankMoney
        % bankItem
        % m_fewMonsterEnd//遇敌步数增加
        % m_maxMonsterEnd//遇敌最大
        % m_minMonsterEnd//遇敌最少
        % m_upMonsterEnd//只遇到>=人物等级的怪
        % monsterScript.str()
        % savePos.str()
        % m_jianYuTime
        % m_answerTimeSpace
        % m_chatTime
        % m_pkTime
        % (int)m_openPack
        % m_sDoubleEnd
        % savaData
        % GetSysTime();
        
    if (pDb != NULL)
    {
        pDb->Query(fmt.str().c_str());
    }
}

void CUser::SaveData(CDatabaseSql *pDb,bool lock)
{
    if((pDb == NULL) || (m_pScene == NULL))
        return;
    char sql[256];
    snprintf(sql,256,"INSERT INTO login_log (role_id,level,login_time) VALUES (%u,%d,from_unixtime(%lu))",m_roleId,m_level,m_loginTime);
    pDb->Query(sql);
    //EquipmentAddAttr(false);
    if(m_jianYuTime > 0)
    {
        if(GetSysTime() - m_loginTime < m_jianYuTime)
            m_jianYuTime -= GetSysTime() - m_loginTime;
        else
            m_jianYuTime = 0;
    }
    //用户等级*用户等级*500
    if(m_money > m_level*m_level*625 + 1000)
        m_money = m_level*m_level*625+1000;

    uint32 mRoleId = GetData32(6);
    if(mRoleId != 0)
    {
        COnlineUser &onlineUser = SingletonOnlineUser::instance();
        ShareUserPtr ptr = onlineUser.GetUserByRoleId(mRoleId);
        CUser *pMarry = ptr.get();
        if(pMarry != NULL)
        {
            int onlineTime = min(GetSysTime() - m_loginTime,GetSysTime()-pMarry->m_loginTime);
            int add = onlineTime/60/30*10;
            if(add > 0)
            {
                SetData32(10,GetData32(10)+add);
                pMarry->SetData32(10,GetData32(10));
            }
        }
    }
    
    m_answerTimeSpace += GetSysTime() - m_answerTime;
    if(lock)
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        NoLockSaveData(pDb);
    }
    else
    {
        NoLockSaveData(pDb);
    }
}


void CUser::WriteHots(string &hots)
{
    int size = m_hotList.size();
    if(size <= 0)
    {
        return;
    }
    if(size > MAX_HOT_NUM)
        size = MAX_HOT_NUM;
        
    uint16 hexHots[size*3+1];
    uint8 *p = (uint8*)hexHots;
    p[0] = HOT_LIST_VERSION;
    p++;
    
    uint8 count = 0;
    for(list<HotInfo>::iterator i = m_hotList.begin(); i != m_hotList.end(); i++)
    {
        //hexHots[count] = *i;
        memcpy(p+6*count,&(i->hotId),4);
        memcpy(p+6*count+4,&(i->hotVal),2);
        count++;
        if(count >= size)
            break;
    }
    HexToStr((uint8*)hexHots,sizeof(hexHots)-1,hots);
}

bool CUser::MakeQiangHuaInfo(uint8 itemPos,uint8 stonePos,uint8 stoneLianhua,uint8 num,CNetMessage &msg)
{
    if((itemPos >= MAX_PACKAGE_NUM) || (stonePos >= MAX_PACKAGE_NUM))
        return false;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    SItemInstance *pItem = m_package + itemPos;
    SItemInstance *pStone = m_package + stonePos;
    if((pItem->tmplId == 0)
        || (pItem->level >= MAX_QIANGHUA_LEVEL))
    {
        return false;
    }
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pTmpl = itemMgr.GetItem(pItem->tmplId);
    if(pTmpl == NULL)
        return false;
        
    if((pTmpl->type > EITXieZi) && (pTmpl->type != EITPetKaiJia))
    {
        return false;
    }
    //pTmpl = itemMgr.GetItem(pStone->tmplId);
    //if(pTmpl->type != EITAddAttr)
    int gaiLv = GetQiangHuaGaiLv(pItem->level);
    if(pStone->tmplId != 610)
        gaiLv = 0;
        
    if(stoneLianhua < MAX_PACKAGE_NUM)
    {
        SItemInstance *pLianHua = m_package + stoneLianhua;
        if((pLianHua->tmplId == 611) && (pLianHua->num >= num))
        {
            int chengBen;
            if(pTmpl->level > 40)
            {
                chengBen = (int)(pTmpl->level*GetLeveUpChengBen(pItem->level)/100);
            }
            else
            {
                chengBen = (int)(pTmpl->level*GetLeveUpChengBen(pItem->level)/200);
            }
            int stoneGaiLv = (int)(100*(num * GetStoneChengBen(pLianHua->level)+1)/(chengBen+1));
            if(stoneGaiLv > 100)
                stoneGaiLv = 100;
#ifdef DEBUG
            cout<<"成本:"<<chengBen<<" 概率:"<<stoneGaiLv<<endl;
#endif
            if(stoneGaiLv > gaiLv)
            {
                if(stoneGaiLv < gaiLv + (num+2)/4)
                    gaiLv += (num+2)/4;
                else
                    gaiLv = stoneGaiLv;
            }
            else
            {
                gaiLv += (num+2)/4;
            }
        }
    }
    
    uint16 src = 0;
    uint16 Levelup = 0;
    if(pTmpl->gongji != 0)
    {
        if(pItem->level > 0)
            src = (uint16)(pTmpl->gongji * GetQiangHuaBeiLv(pItem->level-1));
        else 
            src = pTmpl->gongji;
        
        Levelup = (uint16)(pTmpl->gongji * GetQiangHuaBeiLv(pItem->level));
    }
    else if(pTmpl->fangYu != 0)
    {
        if(pItem->level > 0)
            src = (uint16)(pTmpl->fangYu * GetQiangHuaBeiLv(pItem->level-1));
        else 
            src = pTmpl->fangYu;
        
        Levelup = (uint16)(pTmpl->fangYu * GetQiangHuaBeiLv(pItem->level));
    }
    msg<<(uint8)gaiLv<<src<<Levelup;
    return true;
}

//0成功，1，失败，2不能装备
int CUser::QiangHuaPackage(uint8 itemPos,uint8 stonePos,uint8 stoneLianhua,uint8 num)
{
    if((itemPos >= MAX_PACKAGE_NUM) || (stonePos >= MAX_PACKAGE_NUM))
        return 2;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    SItemInstance *pItem = m_package + itemPos;
    SItemInstance *pStone = m_package + stonePos;
    if((pItem->tmplId == 0)
        || (pItem->level >= MAX_QIANGHUA_LEVEL)
        || (pStone->tmplId == 0))
    {
        return 2;
    }
    SItemInstance item = *pItem;
    
    uint8 bangDing = pStone->bangDing;
    
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pTmpl = itemMgr.GetItem(pItem->tmplId);
    if(pTmpl == NULL)
    {
        return 2;
    }
    if((pTmpl->type > EITXieZi) && (pTmpl->type != EITPetKaiJia))
    {
        return 2;
    }
    //pTmpl = itemMgr.GetItem(pStone->tmplId);
    //if(pTmpl->type != EITAddAttr)
    if(pStone->tmplId != 610)
        return 2;
        
    int gaiLv = GetQiangHuaGaiLv(pItem->level);
    string before;
    if(stoneLianhua < MAX_PACKAGE_NUM)
    {
        SItemInstance *pLianHua = m_package + stoneLianhua;
        if((pLianHua->tmplId == 611) && (pLianHua->num >= num))
        {
            //string before;
            HexToStr(*pLianHua,before);
            int chengBen;
            if(pTmpl->level > 40)
            {
                chengBen = (int)(pTmpl->level*GetLeveUpChengBen(pItem->level)/100);
            }
            else
            {
                chengBen = (int)(pTmpl->level*GetLeveUpChengBen(pItem->level)/200);
            }
            int stoneGaiLv = (int)(100*(num * GetStoneChengBen(pLianHua->level)+1)/(chengBen+1));
            if(stoneGaiLv > gaiLv)
                gaiLv = stoneGaiLv;
            NoLockDelPackage(stoneLianhua,num);
            m_package[itemPos].bangDing = 1;
        }
    }
    int flag;
    if(Random(0,100) <= gaiLv)
    {
        pItem->level++;
        flag = 0;
    }
    else
    {
        pItem->level = GetQiangHuaReturn(pItem->level);
        flag = 1;
    }
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PACK);
    if(bangDing == 1)
        m_package[itemPos].bangDing = bangDing;
    MakePack(m_package[itemPos],itemPos,msg);
    sock.SendMsg(m_sock,msg);
    NoLockDelPackage(stonePos);
    string end;
    HexToStr(m_package[itemPos],end);
    SaveUseItem(m_roleId,item,"强化",num,before,end);
    return flag;
}

int CUser::MakeGreenItem(uint8 item,uint8 stone)
{
    if((item >= MAX_PACKAGE_NUM) || (stone >= MAX_PACKAGE_NUM)) 
        return 2;        
    
    SItemInstance *pItem = m_package + item;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pTmpl = itemMgr.GetItem(pItem->tmplId);
    if((pTmpl != NULL) && (pTmpl->type > EITXieZi))
    {
        return 2;
    }
    string before;
    HexToStr(*pItem,before);
    
    SItemInstance *pStone = m_package + stone;
    if((pItem->tmplId == 0) || (pStone->tmplId != 622) || (pItem->quality == EQTWhite))
        return 2;
        
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    uint8 bangding = pStone->bangDing;
    
    uint8 type[3] = {0};
    uint8 typeNum = Random(1,3);
    for(uint8 j = 0; j < typeNum; j++)
    {
        for(uint8 i = 0; i < 100; i++)
        {
            type[j] = RandSelect(GREEN_EQUIP,sizeof(GREEN_EQUIP));
            if((j == 1) && (type[j] == type[j-1]))
                continue;
            if((j == 2) && ((type[j] == type[0]) || (type[j] == type[1])))
                continue;
            if(CanAddAttr(type[j],pTmpl->type))
                break;
        }
        if(type[j] == 0)
            return 1;   
    }
        
    for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
    {
        if((pItem->addAttrType[i] != 0) && !InBlueEquip(pItem->addAttrType[i]))
        {
            pItem->addAttrType[i] = 0;
            if(pItem->addAttrNum > 0)
                pItem->addAttrNum--;
        }
    }
    
    bool success = false;
    for(uint8 j = 0; j < typeNum; j++)
    {
        for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
        {
            if(pItem->addAttrType[i] == 0)
            {
                pItem->addAttrType[i] = type[j];
                uint16 min = 0,middle = 0,max= 0;
                GetAddAttrVal(type[j],min,middle,max);
                pItem->addAttrVal[i] = Random(min,middle);
                
                pItem->addAttrNum ++;
                pItem->quality = EQTGreen;
                success = true;
                break;
            }
        }
    }
    
    if(success)
    {
        string end;
        HexToStr(*pItem,end);
        CSocketServer &sock = SingletonSocket::instance();
        CNetMessage msg;
        msg.SetType(PRO_UPDATE_PACK);
        if(bangding == 1)
            m_package[item].bangDing = bangding;
        MakePack(m_package[item],item,msg);
        sock.SendMsg(m_sock,msg);
        SaveUseItem(m_roleId,m_package[stone],"做绿装使用",1,before,end);
        NoLockDelPackage(stone);
        return 0;
    }
    else
    {
        return 2;
    }
}

int CUser::SelectGreenItemAttr(uint8 item,uint8 attrInd,uint8 shuijing,uint8 stone,uint8 stoneNum)
{
    if((item >= MAX_PACKAGE_NUM) || (shuijing >= MAX_PACKAGE_NUM) 
        || (attrInd >= SItemInstance::MAX_ADD_ATTR_NUM)) 
        return 2;
        
    SItemInstance *pItem = m_package + item;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pTmpl = itemMgr.GetItem(pItem->tmplId);
    if((pTmpl != NULL) && (pTmpl->type > EITShouZhuo))
    {
        return 2;
    }
    string before;
    HexToStr(*pItem,before);
    
    uint8 type = pItem->addAttrType[attrInd];
    if(type == 0)
        return 2;
        
    SItemInstance *pShuiJing = m_package + shuijing;  
    if((pItem->tmplId == 0) || (pShuiJing->tmplId != 612))
    {
        return 2;
    }
    if(stone >= MAX_PACKAGE_NUM)
    {
        uint16 min=0,middle=0,max = 0;
        GetAddAttrVal(type,min,middle,max);
        uint16 old = pItem->addAttrVal[attrInd];
        pItem->addAttrVal[attrInd] = Random(min,middle);
        if(pItem->addAttrVal[attrInd] == old)
            pItem->addAttrVal[attrInd] = old+1;
            
        CSocketServer &sock = SingletonSocket::instance();
        CNetMessage msg;
        msg.SetType(PRO_UPDATE_PACK);
        MakePack(m_package[item],item,msg);
        sock.SendMsg(m_sock,msg);
        string end;
        HexToStr(*pItem,end);
        SaveUseItem(m_roleId,m_package[shuijing],"选绿属性使用",1,before,end);
        NoLockDelPackage(shuijing);
        return 0;
    }
    SItemInstance *pStone = m_package + stone;
    
    if((pStone->tmplId != 611) || (pStone->num < stoneNum))
    {
        return 2;
    }
    
    uint8 duan = 0;//Random(0,9);
    if(SelectGreenAttr(duan,pStone->level,stoneNum))
    {
#ifdef DEBUG
        cout<<"duan:"<<(int)duan<<endl;
#endif                
        uint16 min=0,middle=0,max = 0;
        GetAddAttrVal(type,min,middle,max);
        uint16 attr = 0;
        if(duan == 0)
            attr = Random(middle,(int)(max*0.1));
        else
            attr = Random(max*duan/10,max*(duan+1)/10);
        if(attr == pItem->addAttrVal[attrInd])
            pItem->addAttrVal[attrInd] = attr+1;
        else
            pItem->addAttrVal[attrInd] = attr;
        
        CSocketServer &sock = SingletonSocket::instance();
        CNetMessage msg;
        msg.SetType(PRO_UPDATE_PACK);
        m_package[item].bangDing = 1;
        MakePack(m_package[item],item,msg);
        sock.SendMsg(m_sock,msg);
        string end;
        HexToStr(*pItem,end);
        SaveUseItem(m_roleId,m_package[shuijing],"选绿属性使用",1,before,end);
        SaveUseItem(m_roleId,m_package[stone],"选绿属性使用",stoneNum,before,end);
        NoLockDelPackage(shuijing);
        NoLockDelPackage(stone,stoneNum);
        return 0;
    }
    else
    {
        uint16 min=0,middle=0,max = 0;
        GetAddAttrVal(type,min,middle,max);
        pItem->addAttrVal[attrInd] = Random(min,middle);
        
        CSocketServer &sock = SingletonSocket::instance();
        CNetMessage msg;
        msg.SetType(PRO_UPDATE_PACK);
        m_package[item].bangDing = 1;
        MakePack(m_package[item],item,msg);
        sock.SendMsg(m_sock,msg);
        string end;
        HexToStr(*pItem,end);
        SaveUseItem(m_roleId,m_package[shuijing],"选绿属性使用",1,before,end);
        SaveUseItem(m_roleId,m_package[stone],"选绿属性使用",1,before,end);
        NoLockDelPackage(shuijing);
        NoLockDelPackage(stone,stoneNum);
        return 0;
    }
}

//0成功，1，失败，2不能炼化，stone1,stone2,stone3为黑水晶位置
int CUser::LianHuaPackage(uint8 itemPos,uint8 stone1,uint8 stone2,uint8 stone3)
{
    if(itemPos >= MAX_PACKAGE_NUM)
        return 2;
    
    uint8 stone[3];
    uint8 num = 0;
    if(stone1 < MAX_PACKAGE_NUM)
    {
        stone[num] = stone1;
        num++;
    }
    if(stone2 < MAX_PACKAGE_NUM)
    {
        stone[num] = stone2;
        num++;
    }
    if(stone3 < MAX_PACKAGE_NUM)
    {
        stone[num] = stone3;
        num++;
    }
    if(num <= 0)
        return 2;
        
    SItemInstance *pItem = m_package + itemPos;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pTmpl = itemMgr.GetItem(pItem->tmplId);
    if((pTmpl != NULL) && (pTmpl->type > EITXieZi))
    {
        return 2;
    }
    string before;
    HexToStr(*pItem,before);
    
    SItemInstance *pStone[num];
    
    uint8 bangDing = 0;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i < num; i++)
    {
        pStone[i] = m_package + stone[i];
        if(pStone[i]->bangDing == 1)
            bangDing = 1;
        if((pStone[i]->tmplId != 615) || (pStone[i]->addAttrNum != 1)
            || !CanAddAttr(pStone[i]->addAttrType[0],pTmpl->type)
            || (pTmpl->level < pStone[i]->level))
            return 2;
    }
    if(num == 2)
    {
        if(pStone[0]->addAttrType[0] == pStone[1]->addAttrType[0])
            return 2;
    }
    else if(num == 3)
    {
        if((pStone[0]->addAttrType[0] == pStone[1]->addAttrType[0])
            || (pStone[0]->addAttrType[0] == pStone[2]->addAttrType[0])
            || (pStone[1]->addAttrType[0] == pStone[2]->addAttrType[0]))
            return 2;
    }
    
    if((pItem->tmplId == 0)
        || (pItem->quality != EQTWhite))
    {
        return 2;
    }
    
    pItem->addAttrNum = num;   
    pItem->quality = EQTBlue;
    for(uint8 i = 0; i < num; i++)
    {
        pItem->addAttrType[i] = pStone[i]->addAttrType[0];
        pItem->addAttrVal[i] = pStone[i]->addAttrVal[0];
        string end;
        HexToStr(*pItem,end);
        SaveUseItem(m_roleId,m_package[stone[i]],"炼化使用",1,before,end);
        NoLockDelPackage(stone[i]);
    }
    pItem->reqAttrType = Random(ERATtizhi,ERATnaili);
    pItem->reqAttrVal = Random(pTmpl->level*2,pTmpl->level*3);
    if(bangDing == 1)
        pItem->bangDing = bangDing;
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PACK);
    MakePack(m_package[itemPos],itemPos,msg);
    sock.SendMsg(m_sock,msg);
    
    return 0;
}

//0成功，1，失败，2不能炼化，tianHuanShi天换石(623)为位置(),lianHuaShi炼化石位置(611)
int CUser::TianHuanPackage(uint8 itemPos,uint8 tianHuanShi)//,uint8 lianHuaShi)
{
    if((itemPos >= MAX_PACKAGE_NUM) 
        || (tianHuanShi >= MAX_PACKAGE_NUM))
        return 2;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    SItemInstance *pItem = m_package + itemPos;
    if((pItem->tmplId == 0)
        || (pItem->quality == EQTWhite)
        || (m_package[tianHuanShi].tmplId != 623))
    {
        return 2;
    }
    string before;
    HexToStr(*pItem,before);
    
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pTmpl = itemMgr.GetItem(pItem->tmplId);
    if((pTmpl != NULL) && (pTmpl->type > EITXieZi))
    {
        return 2;
    }
    
    pItem->reqAttrType = Random(ERATtizhi,ERATnaili);
    pItem->reqAttrVal = Random(pTmpl->level*2,pTmpl->level*3);
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PACK);
    MakePack(m_package[itemPos],itemPos,msg);
    sock.SendMsg(m_sock,msg);
    string end;
    HexToStr(*pItem,end);
    SaveUseItem(m_roleId,m_package[tianHuanShi],"天换使用",1,before,end);
    NoLockDelPackage(tianHuanShi);
    return 0;
}

bool CUser::ModifyPetName(uint8 pos,string &name,string &errMsg)
{
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        if(pos >= MAX_PET_NUM)
        {
            errMsg = "没有此宠物";
            return false;
        }
        SPet *pPet = m_pet[pos].get();
        if(pPet == NULL)
        {
            errMsg = "没有此宠物";
            return false;
        }
        if(pPet->level < 35)
        {
            errMsg = "宠物大于35级才能改名";
            return false;
        }
        if(pPet->qinmi < 10000)
        {
            errMsg = "宠物亲密大于10000才能改名";
            return false;
        }
        if(IsIllegalMsg(name.c_str()))
        {
            errMsg = "名字中含有非法字符";
            return false;
        }
        pPet->qinmi -= 1000;
        strncpy(pPet->name,name.c_str(),SPet::MAX_NAME_LEN);
    }
    UpdatePet(pos);
    return true;
}

/*********
攻击一次，武器耐久-1		
被打一次，4件防具随机一件耐久-1		
死一次，全身耐久-10%（总耐久*10%）
**********/
void CUser::DecreaseWuQiNaiJiu(int naijiu)
{
    if(m_equipment[EETWuQi].tmplId != 0)
    {
        if(m_equipment[EETWuQi].naijiu > 0)
        {
            if(m_equipment[EETWuQi].naijiu >= naijiu)
                m_equipment[EETWuQi].naijiu -= naijiu;
            else 
                m_equipment[EETWuQi].naijiu = 0;
            UpdateEquip(EETWuQi);
            if(m_equipment[EETWuQi].naijiu == 0)
            {
                //EquipAddAttr(m_equipment[EETWuQi],false);
                Init();
                UpdateInfo();
            }
        }
    }
}

bool CUser::HaveZeroEquip()
{
    for(uint8 i = 0; i <= EETWuQi; i++)
    {
        if((m_equipment[i].tmplId != 0) && (m_equipment[i].naijiu == 0))
        {
            return true;
        }
    }
    return false;
}

void CUser::DecreaseFangJuNaiJiu(int naijiu)
{
    //uint8 equip[EETWuQi];
    //uint8 num = 0;
    for(uint8 i = 0; i < EETWuQi; i++)
    {
        if((m_equipment[i].tmplId != 0) && (m_equipment[i].naijiu != 0))
        {
            if(m_equipment[i].naijiu > naijiu)
                m_equipment[i].naijiu -= naijiu;
            else 
                m_equipment[i].naijiu = 0;
            UpdateEquip(i);
            if(m_equipment[i].naijiu == 0)
            {
                //EquipAddAttr(m_equipment[i],false);
                Init();
                UpdateInfo();
            }
        }
    }
    /*uint8 pos = RandSelect(equip,num);
    if(pos < EETWuQi)
    {
        if((m_equipment[pos].tmplId != 0) && (m_equipment[pos].naijiu > 0))
        {
            m_equipment[pos].naijiu--;
            UpdateEquip(pos);
            if(m_equipment[pos].naijiu == 0)
                EquipAddAttr(m_equipment[EETWuQi],false);
        }
    }*/
}

void CUser::DecreaseAllNaiJiu(int naijiu)
{
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    for(uint8 i = 0; i <= EETWuQi; i++)
    {
        if((m_equipment[i].tmplId != 0) && (m_equipment[i].naijiu > 0))
        {
            SItemTemplate *pItem = itemMgr.GetItem(m_equipment[i].tmplId);
            if(pItem != NULL)
            {
                int des = CalculateRate((int)pItem->naijiu,10,100);
                if(naijiu != 0)
                    des = naijiu;
                    
                if(m_equipment[i].naijiu > des)
                {
                    m_equipment[i].naijiu -= des;
                    UpdateEquip(i);
                }
                else
                {
                    if(m_equipment[i].naijiu != 0)
                    {
                        //EquipAddAttr(m_equipment[EETWuQi],false);
                        m_equipment[i].naijiu = 0;
                        UpdateEquip(i);
                        Init();
                        UpdateInfo();
                    }
                }
            }
            if(naijiu != 0)
                break;
        }
    }
}

/*
1级，10级：要修理的耐久值*装备等级*0.025
20级~50级：要修理的耐久值*装备等级*0.04
60级以后： 要修理的耐久值*装备等级*0.045
*/
int CUser::GetRepairFee()
{
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem;
    
    int needMoney = 0;
    for(uint8 i = 0; i <= EETWuQi; i++)
    {
        pItem = itemMgr.GetItem(m_equipment[i].tmplId);
        if((pItem != NULL) && (pItem->naijiu != m_equipment[i].naijiu))
        {
            int add = 0;
            if((pItem->level >= 1) && (pItem->level <= 10))
            {
                add =(int)(pItem->level*
                        (pItem->naijiu - m_equipment[i].naijiu)*0.025);
            }
            else if((pItem->level >= 20) && (pItem->level <= 50))
            {
                add =(int)(pItem->level*
                        (pItem->naijiu - m_equipment[i].naijiu)*0.04);
            }
            else
            {
                add =(int)(pItem->level*
                        (pItem->naijiu - m_equipment[i].naijiu)*0.045);
            }
            if(add <= 0)
                add = 1;
            needMoney += add;
        }
    }
    SharePetPtr pet = GetPet(m_chuZhanPet);
    SPet *pPet = pet.get();
    if((pPet != NULL) && (pPet->kaiJia.tmplId != 0))
    {
        SItemInstance &kaiji = pPet->kaiJia;
        pItem = itemMgr.GetItem(kaiji.tmplId);
        if(pItem != NULL)
        {
            int add = 0;
            if((pItem->level >= 1) && (pItem->level <= 10))
            {
                add =(int)(pItem->level*
                        (pItem->naijiu - kaiji.naijiu)*0.025);
            }
            else if((pItem->level >= 20) && (pItem->level <= 50))
            {
                add =(int)(pItem->level*
                        (pItem->naijiu - kaiji.naijiu)*0.04);
            }
            else
            {
                add =(int)(pItem->level*
                        (pItem->naijiu - kaiji.naijiu)*0.045);
            }
            if(add <= 0)
                add = 1;
            needMoney += add;
        }
    }
    return needMoney;
}

void CUser::RepairTotel()
{
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem;
    bool update = false;
    
    for(uint8 i = 0; i <= EETWuQi; i++)
    {
        pItem = itemMgr.GetItem(m_equipment[i].tmplId);
        if(pItem != NULL)
        {
            int naijiu = m_equipment[i].naijiu;
            if(m_equipment[i].naijiu != pItem->naijiu)
            {
                m_equipment[i].naijiu = pItem->naijiu;
                UpdateEquip(i);
            }
            if(naijiu == 0)
            {
                update = true;
                //if(CanAddShuXing(pItem,m_equipment + i))
                    //EquipAddAttr(m_equipment[i],true);
            }
        }
    }
    SharePetPtr pet = GetPet(m_chuZhanPet);
    SPet *pPet = pet.get();
    if((pPet != NULL) && (pPet->kaiJia.tmplId != 0))
    {
        SItemInstance &kaiji = pPet->kaiJia;
        pItem = itemMgr.GetItem(kaiji.tmplId);
        if(pItem != NULL)
        {
            int naijiu = kaiji.naijiu;
            if(kaiji.naijiu != pItem->naijiu)
            {
                kaiji.naijiu = pItem->naijiu;
                UpdatePet(m_chuZhanPet);
            }
            if(naijiu == 0)
            {
                update = true;
            }
        }
    }
    if(update)
    {
        Init();
        UpdateInfo();
    }
}

//0成功，1，失败，2不能修理
int CUser::RepairItem(uint8 pos)
{
    if(pos > MAX_PACKAGE_NUM)
        return 2;
    if(m_package[pos].tmplId == 0)
        return 2;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(m_package[pos].tmplId);
    if(pItem == NULL)
        return 2;
    
    if((pItem->type > EITXieZi) && (pItem->type != EITPetKaiJia))
        return 2;
    
    m_package[pos].naijiu = pItem->naijiu;
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PACK);
    MakePack(m_package[pos],pos,msg);
    sock.SendMsg(m_sock,msg);
    return 0;
}

void CUser::SetSaveVal(uint8 index,int val)
{
    if(index < MAX_SAVE_NUM - 1)
        m_shortArray[index] = val;
}

int CUser::GetSaveVal(uint8 index)
{
    if(index < MAX_SAVE_NUM - 1)
        return m_shortArray[index];
    return 0;
}

void CUser::SetSaveVal(char *msg)
{
    if(msg == NULL)
        return;
    char *p[MAX_SAVE_NUM] = {NULL};
    SplitLine(p,MAX_SAVE_NUM,msg);
    for(int i = 0; i < MAX_SAVE_NUM; i++)
    {
        if(p[i] == NULL)
            m_shortArray[i] = 0;
        else
            m_shortArray[i] = atoi(p[i]);
    }
    /*if(m_shortArray[MAX_SAVE_NUM-2] == 0)
    {
        time_t t = GetSysTime();
        tm *pTm = localtime(&t);
        if(pTm != NULL)
            m_shortArray[MAX_SAVE_NUM-2] = pTm->tm_yday;
    }*/
}

//const int NORMAL_GIFT[] = {10*60,30*60,60*60,0};//每日礼盒时间
const int NORMAL_GIFT[] = {1*60,3*60,5*60,0};//每日礼盒时间
//const int NORMAL_GIFT[] = {10*60,30*60,60*60,0};//每日礼盒时间
const int XINSHOU_GIFT[] = {60,3*60,5*60,10*60,15*60,0};//新手礼盒时间

//type = 1新手礼盒
void CUser::GetGift(uint8 type)
{
    if(m_giftBitset == 0)
        return;
        
    if(m_gift.type == 0)
        return;
    
    if(type == 1)
    {
        //int times[] = {60,3*60,5*60,10*60,15*60,0};//新手礼盒时间
        if(m_gift.pos < sizeof(XINSHOU_GIFT)/sizeof(XINSHOU_GIFT[0])-1)
        {
            SendUpdateInfo(47,XINSHOU_GIFT[m_gift.pos+1]/60);
        }
    }
    else
    {
        //int times[] = {10*60,30*60,60*60,0};//每日礼盒时间
        if(m_gift.pos < sizeof(NORMAL_GIFT)/sizeof(NORMAL_GIFT[0])-1)
        {
            SendUpdateInfo(47,NORMAL_GIFT[m_gift.pos+1]/60);
        }
    }
    if(m_gift.type == 1)
    {
        if(m_gift.val == 2)
        {//金 ID2:强化+2，金5水5火5，全属性强化+5，全相+1，法暴+1。
            SItemInstance item = {0};
            item.tmplId = m_gift.val;
            item.bangDing = 1;
            item.level = 2;
            item.num = 1;
            item.addAttrType[0] = EAAjin;
            item.addAttrVal[0] = 5;
            item.addAttrType[1] = EAAshui;
            item.addAttrVal[1] = 5;
            item.addAttrType[2] = EAAhuo;
            item.addAttrVal[2] = 5;
            item.addAttrType[3] = EAAquanshuxing;
            item.addAttrVal[3] = 5;
            item.addAttrType[4] = EAAquanxiangxing;
            item.addAttrVal[4] = 1;
            item.addAttrType[5] = EAAfashubaoji;
            item.addAttrVal[5] = 1;
            
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
            if(pItem != NULL)
                item.naijiu = pItem->naijiu;
            AddPackage(item);
        }
        else if(m_gift.val == 52)
        {//木 ID52:强化+2,土5水5木5，全属性强化+5，全相+1，狂暴+1。
            SItemInstance item = {0};
            item.tmplId = m_gift.val;
            item.bangDing = 1;
            item.level = 2;
            item.num = 1;
            item.addAttrType[0] = EAAtu;
            item.addAttrVal[0] = 5;
            item.addAttrType[1] = EAAshui;
            item.addAttrVal[1] = 5;
            item.addAttrType[2] = EAAmu;
            item.addAttrVal[2] = 5;
            item.addAttrType[3] = EAAquanshuxing;
            item.addAttrVal[3] = 5;
            item.addAttrType[4] = EAAquanxiangxing;
            item.addAttrVal[4] = 1;
            item.addAttrType[5] = EAAaddbaojiweili;
            item.addAttrVal[5] = 1;
            
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
            if(pItem != NULL)
                item.naijiu = pItem->naijiu;
            AddPackage(item);
        }
        else if(m_gift.val == 102)
        {//水 ID102:强化+2，金5水5火5，全属性强化+5，全相+1，法暴+1。
            SItemInstance item = {0};
            item.tmplId = m_gift.val;
            item.bangDing = 1;
            item.level = 2;
            item.num = 1;
            item.addAttrType[0] = EAAjin;
            item.addAttrVal[0] = 5;
            item.addAttrType[1] = EAAshui;
            item.addAttrVal[1] = 5;
            item.addAttrType[2] = EAAhuo;
            item.addAttrVal[2] = 5;
            item.addAttrType[3] = EAAquanshuxing;
            item.addAttrVal[3] = 5;
            item.addAttrType[4] = EAAquanxiangxing;
            item.addAttrVal[4] = 1;
            item.addAttrType[5] = EAAfashubaoji;
            item.addAttrVal[5] = 1;
            
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
            if(pItem != NULL)
                item.naijiu = pItem->naijiu;
            AddPackage(item);
        }
        else if(m_gift.val == 152)
        {//火 ID152:强化+2，金5水5火5，全属性强化+5，全相+1，法暴+1。
            SItemInstance item = {0};
            item.tmplId = m_gift.val;
            item.bangDing = 1;
            item.level = 2;
            item.num = 1;
            item.addAttrType[0] = EAAjin;
            item.addAttrVal[0] = 5;
            item.addAttrType[1] = EAAshui;
            item.addAttrVal[1] = 5;
            item.addAttrType[2] = EAAhuo;
            item.addAttrVal[2] = 5;
            item.addAttrType[3] = EAAquanshuxing;
            item.addAttrVal[3] = 5;
            item.addAttrType[4] = EAAquanxiangxing;
            item.addAttrVal[4] = 1;
            item.addAttrType[5] = EAAfashubaoji;
            item.addAttrVal[5] = 1;
            
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
            if(pItem != NULL)
                item.naijiu = pItem->naijiu;
            AddPackage(item);
        }
        else if(m_gift.val == 202)
        {//土 ID202:强化+2,土5水5木5，全属性强化+5，全相+1，狂暴+1。
            SItemInstance item = {0};
            item.tmplId = m_gift.val;
            item.bangDing = 1;
            item.level = 2;
            item.num = 1;
            item.addAttrType[0] = EAAtu;
            item.addAttrVal[0] = 5;
            item.addAttrType[1] = EAAshui;
            item.addAttrVal[1] = 5;
            item.addAttrType[2] = EAAmu;
            item.addAttrVal[2] = 5;
            item.addAttrType[3] = EAAquanshuxing;
            item.addAttrVal[3] = 5;
            item.addAttrType[4] = EAAquanxiangxing;
            item.addAttrVal[4] = 1;
            item.addAttrType[5] = EAAaddbaojiweili;
            item.addAttrVal[5] = 1;
            
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(item.tmplId);
            if(pItem != NULL)
                item.naijiu = pItem->naijiu;
            AddPackage(item);
        }
        else
        {
            AddBangDingPackage(m_gift.val,m_gift.num);
            if(type != 1)
            {
                if((m_gift.val == 1805)
                    || (m_gift.val == 612)
                    || (m_gift.val == 622)
                    || (m_gift.val == 1801)
                    || (m_gift.val == 1802)
                    || (m_gift.val == 1809)
                    || (m_gift.val == 1804)
                    || (m_gift.val == 610))
                {
                    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
                    SItemTemplate *pItem = itemMgr.GetItem(m_gift.val);
                    if(pItem != NULL)
                    {
                        char buf[128];
                        snprintf(buf,sizeof(buf),"恭喜%s在每日宝箱中获得%s%d个",m_name,pItem->name.c_str(),m_gift.num);
                        SysInfoToAllUser(buf);
                    }
                }
            }
        }
    }
    else if(m_gift.type == 2)
        AddExp(m_gift.val);
    else if(m_gift.type == 3)
        AddQianNeng(m_gift.val);
    else if(m_gift.type == 4)
        AddDaoHang(m_gift.val);
    SetBitSet(m_giftBitset);
    m_giftBitset = 0;
    m_gift.type = 0;
}

/*221-225
新手宝箱 1分钟，3分钟，5分钟，10分钟，15分钟
每日宝箱 10分钟，30分钟，1小时的时间点上
227-是否领取过每日宝箱（每日清除）*/
bool CUser::NewUserGift()//新手宝箱
{
    if(m_level >= 20)
        return false;
    if(HaveBitSet(225))
        return false;
    if(m_giftBitset != 0)
    {
        m_giftTime = GetSysTime();
        return true;
    }
    //int times[] = {60,3*60,5*60,10*60,15*60,0};
    for(uint32 i = 0; i < sizeof(XINSHOU_GIFT)/sizeof(XINSHOU_GIFT[0]-1); i++)
    {
        if(!HaveBitSet(221+i))
        {
            if(m_giftTime == 0)
            {
                SendUpdateInfo(47,XINSHOU_GIFT[i]/60);
                m_giftTime = GetSysTime();
            }
            if(GetSysTime() - m_giftTime >= XINSHOU_GIFT[i])
            {
                m_giftBitset = 221+i;
                m_gift.type = 1;
                m_gift.num = 1;
                m_gift.pos = i;
                if(i == 0)
                {
                    m_gift.val = 1805;
                }
                else if(i == 1)
                {
                    /*土 ID202:强化+2,土5水5木5，全属性强化+5，全相+1，狂暴+1。
                    木 ID52:强化+2,土5水5木5，全属性强化+5，全相+1，狂暴+1。
                    金 ID2:强化+2，金5水5火5，全属性强化+5，全相+1，法暴+1。
                    水 ID102:强化+2，金5水5火5，全属性强化+5，全相+1，法暴+1。
                    火 ID152:强化+2，金5水5火5，全属性强化+5，全相+1，法暴+1。*/
                    if(m_xiang == EXJinXiang)
                        m_gift.val = 2;
                    else if(m_xiang == EXMuXiang)
                        m_gift.val = 52;
                    else if(m_xiang == EXShuiXiang)
                        m_gift.val = 102;
                    else if(m_xiang == EXHuoXiang)
                        m_gift.val = 152;
                    else if(m_xiang == EXTuXiang)
                        m_gift.val = 202;
                }
                else if(i == 2)
                {
                    m_gift.val = 651;//1809;
                    m_gift.num = 20;
                }
                else if(i == 3)
                {
                    m_gift.val = 750;
                    m_gift.num = 10;
                }
                else if(i == 4)
                {
                    m_gift.val = 1801;
                }
                CSocketServer &sock = SingletonSocket::instance();
                CNetMessage msg;
                msg.SetType(MSG_NEW_USER_GIFT);
                
                CItemTemplateManager &itemMgr = SingletonItemManager::instance();
                SItemTemplate *pTmpl = itemMgr.GetItem(m_gift.val);
                if(pTmpl != NULL)
                {
                    msg<<m_gift.val<<m_gift.num<<pTmpl->name.c_str();
                    sock.SendMsg(m_sock,msg);
                }
            }
            return true;
        }
    }
    return true;
}

void CUser::SelectUserGift(uint16 *gifts,uint16 num)
{
    uint8 r = Random(0,100);
    list<uint16> selectPos;
    for(uint16 i = 0; i < num/5; i++)
    {
        if(r <= gifts[5*i+4])
        {
            selectPos.push_back(i);
            m_gift.type = gifts[5*i];
            m_gift.val = gifts[5*i+1];
            m_gift.num = gifts[5*i+2];
            break;
        }
    }
    CNetMessage msg;
    if(m_gift.type == 1)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        
        CItemTemplateManager &itemMgr = SingletonItemManager::instance();
        SItemTemplate *pTmpl = itemMgr.GetItem(m_gift.val);
        if(pTmpl != NULL)
        {
            msg<<m_gift.type<<m_gift.val<<m_gift.num<<pTmpl->name.c_str();
        }
    }
    else if(m_gift.type == 2)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"经验";
    }
    else if(m_gift.type == 3)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"潜能";
    }
    else if(m_gift.type == 4)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"道行";
    }
    
    //msg<<(uint8)9<<m_gift.type<<m_gift.val<<m_gift.num;
    
    //for(uint16 times = 0; times < 8; times++)
    uint8 times = 0;
    for(int j = 0; j < 100; j++)
    {
        uint8 r = Random(0,100);
        for(uint16 i = 0; i < num/5; i++)
        {
            if((find(selectPos.begin(),selectPos.end(),i) == selectPos.end()) &&
                (r <= gifts[5*i+3]))
            {
                msg<<(uint8)gifts[5*i]<<(uint16)gifts[5*i+1]<<(uint8)gifts[5*i+2];
                selectPos.push_back(i);
                times++;
                break;
            }
            if(times >= 8)
                break;
        }
        if(times >= 8)
            break;
    }
    CSocketServer &sock = SingletonSocket::instance();
    sock.SendMsg(m_sock,msg);
}

void CUser::NormalUserGift0(uint16 )
{
    uint16 gifts[] = 
    {//type,id,num,出现概率，获得概率
        2,500,1,5,14,
        2,700,1,10,19,
        2,1000,1,13,22,
        3,1000,1,18,35,
        3,1400,1,23,40,
        3,2000,1,26,43,
        4,200,1,31,56,
        4,300,1,36,61,
        4,500,1,39,64,
        1,676,5,42,67,
        1,677,5,46,69,
        1,726,5,49,72,
        1,601,1,52,74,
        1,602,1,55,76,
        1,603,1,58,77,
        1,604,1,61,79,
        1,605,1,64,81,
        1,606,1,67,82,
        1,607,1,70,84,
        1,608,1,73,86,
        1,609,1,76,87,
        1,1812,1,79,90,
        1,1814,1,82,92,
        1,615,1,85,97,
        1,623,1,88,100,
        1,1801,1,91,0,
        1,615,3,94,0,
        1,1804,1,97,0,
        1,610,1,100,0,
    };
    SelectUserGift(gifts,sizeof(gifts)/sizeof(gifts[0]));
    /*
    uint8 r = Random(0,100);
    //uint16 pos[9] = {0};
    for(uint16 i = 0; i < sizeof(gifts)/sizeof(gifts[0])/5; i++)
    {
        if(r <= gifts[5*i+4])
        {
            //pos[0] = i;
            m_gift.type = gifts[5*i];
            m_gift.val = gifts[5*i+1];
            m_gift.num = gifts[5*i+2];
            break;
        }
    }
    CNetMessage msg;
    if(m_gift.type == 1)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        
        CItemTemplateManager &itemMgr = SingletonItemManager::instance();
        SItemTemplate *pTmpl = itemMgr.GetItem(m_gift.val);
        if(pTmpl != NULL)
        {
            msg<<m_gift.type<<m_gift.val<<m_gift.num<<pTmpl->name.c_str();
        }
    }
    else if(m_gift.type == 2)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"经验";
    }
    else if(m_gift.type == 3)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"潜能";
    }
    else if(m_gift.type == 4)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"道行";
    }
    
    //msg<<(uint8)9<<m_gift.type<<m_gift.val<<m_gift.num;
    
    for(uint16 times = 0; times < 8; times++)
    {
        uint8 r = Random(0,100);
        for(uint16 i = 0; i < sizeof(gifts)/sizeof(gifts[0])/5; i++)
        {
            if(r <= gifts[5*i+3])
            {
                msg<<(uint8)gifts[5*i]<<(uint16)gifts[5*i+1]<<(uint8)gifts[5*i+2];
                break;
            }
        }
    }
    CSocketServer &sock = SingletonSocket::instance();
    sock.SendMsg(m_sock,msg);*/
}

void CUser::NormalUserGift1(uint16 )
{
    uint16 gifts[] = 
    {//type,id,num,出现概率，获得概率
        2,1000,1,3,11,    
        2,1500,1,6,14,    
        2,2000,1,9,16,    
        3,2000,1,12,28,   
        3,2500,1,15,33,   
        3,3000,1,17,36,   
        4,500,1,20,48,    
        4,700,1,22,53,    
        4,900,1,24,56,    
        1,800,1,27,60,    
        1,615,1,57,80,    
        1,676,10,60,83,   
        1,677,10,63,85,   
        1,601,1,66,87,    
        1,602,1,69,89,    
        1,603,1,72,90,    
        1,604,1,75,92,    
        1,605,1,78,94,    
        1,606,1,81,95,    
        1,607,1,84,97,    
        1,608,1,87,99,    
        1,609,1,90,100,   
        1,1801,1,92,0,  
        1,1802,1,94,0,  
        1,1804,1,96,0,  
        1,1805,1,98,0,  
        1,1817,1,100,0, 
    };
    SelectUserGift(gifts,sizeof(gifts)/sizeof(gifts[0]));
    /*
    uint8 r = Random(0,100);
    for(uint16 i = 0; i < sizeof(gifts)/sizeof(gifts[0])/5; i++)
    {
        if(r <= gifts[5*i+4])
        {
            m_gift.type = gifts[5*i];
            m_gift.val = gifts[5*i+1];
            m_gift.num = gifts[5*i+2];
            break;
        }
    }
    CNetMessage msg;
    if(m_gift.type == 1)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        
        CItemTemplateManager &itemMgr = SingletonItemManager::instance();
        SItemTemplate *pTmpl = itemMgr.GetItem(m_gift.val);
        if(pTmpl != NULL)
        {
            msg<<m_gift.type<<m_gift.val<<m_gift.num<<pTmpl->name.c_str();
        }
    }
    else if(m_gift.type == 2)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"经验";
    }
    else if(m_gift.type == 3)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"潜能";
    }
    else if(m_gift.type == 4)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"道行";
    }
    
    //msg<<(uint8)9<<m_gift.type<<m_gift.val<<m_gift.num;
    
    for(uint16 times = 0; times < 8; times++)
    {
        uint8 r = Random(0,100);
        for(uint16 i = 0; i < sizeof(gifts)/sizeof(gifts[0])/5; i++)
        {
            if(r <= gifts[5*i+3])
            {
                msg<<(uint8)gifts[5*i]<<(uint16)gifts[5*i+1]<<(uint8)gifts[5*i+2];
                break;
            }
        }
    }
    CSocketServer &sock = SingletonSocket::instance();
    sock.SendMsg(m_sock,msg);*/
}

void CUser::NormalUserGift2(uint16 )
{
    uint16 gifts[] = 
    {//type,id,num,出现概率，获得概率
        2,1000,1,5,12,
        2,2000,1,10,17,
        2,3000,1,15,20,
        3,2000,1,20,30,
        3,3000,1,25,35,
        3,4000,1,30,38,
        4,500,1,35,50,
        4,700,1,40,55,
        4,1000,1,45,58,
        1,676,10,50,61,
        1,677,10,55,64,
        1,623,2,58,69,
        1,1812,1,61,74,
        1,800,1,66,80,
        1,615,1,76,90,
        1,1805,1,79,91,
        1,612,1,82,92,
        1,622,1,85,93,
        1,1801,1,88,95,
        1,1802,1,91,96,
        1,1809,1,94,97,
        1,1804,1,97,98,
        1,610,1,100,100,
    };
    SelectUserGift(gifts,sizeof(gifts)/sizeof(gifts[0]));
    /*
    uint8 r = Random(0,100);
    for(uint16 i = 0; i < sizeof(gifts)/sizeof(gifts[0])/5; i++)
    {
        if(r <= gifts[5*i+4])
        {
            m_gift.type = gifts[5*i];
            m_gift.val = gifts[5*i+1];
            m_gift.num = gifts[5*i+2];
            break;
        }
    }
    CNetMessage msg;
    if(m_gift.type == 1)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        
        CItemTemplateManager &itemMgr = SingletonItemManager::instance();
        SItemTemplate *pTmpl = itemMgr.GetItem(m_gift.val);
        if(pTmpl != NULL)
        {
            msg<<m_gift.type<<m_gift.val<<m_gift.num<<pTmpl->name.c_str();
        }
    }
    else if(m_gift.type == 2)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"经验";
    }
    else if(m_gift.type == 3)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"潜能";
    }
    else if(m_gift.type == 4)
    {
        msg.SetType(MSG_NORMAL_USER_GIFT);
        msg<<m_gift.type<<m_gift.val<<m_gift.num<<"道行";
    }
    
    //msg<<(uint8)9<<m_gift.type<<m_gift.val<<m_gift.num;
    
    for(uint16 times = 0; times < 8; times++)
    {
        uint8 r = Random(0,100);
        for(uint16 i = 0; i < sizeof(gifts)/sizeof(gifts[0])/5; i++)
        {
            if(r <= gifts[5*i+3])
            {
                msg<<(uint8)gifts[5*i]<<(uint16)gifts[5*i+1]<<(uint8)gifts[5*i+2];
                break;
            }
        }
    }
    CSocketServer &sock = SingletonSocket::instance();
    sock.SendMsg(m_sock,msg);*/
}

bool CUser::NormalUserGift()//每日宝箱
{
    //int times[] = {10*60,30*60,60*60,0};
    if(m_level >= 40)
        return false;
    if(m_giftBitset != 0)
    {
        m_giftTime = GetSysTime();
        return true;
    }
    for(uint32 i = 0; i < sizeof(NORMAL_GIFT)/sizeof(NORMAL_GIFT[0])-1; i++)
    {
        if(!HaveBitSet(221+i))
        {
            if(m_giftTime == 0)
            {
                m_giftTime = GetSysTime()-GetData8(2)*60;
                int leftTime = NORMAL_GIFT[i]/60 - GetData8(2);
                if(leftTime < 0)
                    leftTime = 0;
                SendUpdateInfo(47,leftTime);
            }
            if(GetSysTime() - m_giftTime >= NORMAL_GIFT[i])
            {
                m_giftBitset = 221+i;
                m_gift.pos = i;
                if(i == 0)
                {
                    NormalUserGift0(NORMAL_GIFT[i+1]/60);
                }
                else if(i == 1)
                {
                    NormalUserGift1(NORMAL_GIFT[i+1]/60);
                }
                else if(i == 2)
                {
                    NormalUserGift2(NORMAL_GIFT[i+1]/60);
                }
            }
            else
            {
                SetData8(2,(GetSysTime() - m_giftTime)/60);
            }
            return true;
        }
    }
    return false;
}

void CUser::UpdateMission()
{
    uint32 day = 0;
    time_t t = GetSysTime();
    tm *pTm = localtime(&t);
    if(pTm->tm_yday != (int)day)
    {
        day = pTm->tm_yday;
    }
    time_t endTime = GetExtData32(0);
    if((endTime != 0) && (endTime < GetSysTime()))
    {
        SetExtData32(0,0);
        SendUpdateInfo(45,0);
    }
    endTime = GetExtData32(1);
    if((endTime != 0) && (endTime < GetSysTime()))
    {
        SetExtData32(1,0);
        SendUpdateInfo(45,1);
    }
    endTime = GetExtData32(2);
    if((endTime != 0) && (endTime < GetSysTime()))
    {
        SetExtData32(2,0);
        SendUpdateInfo(45,2);
    }
    
    if(m_shortArray[MAX_SAVE_NUM-2] != day)
    {
        m_shortArray[MAX_SAVE_NUM-2] = day;
        //SetSaveVal(0,0);
        SetSaveVal(3,0);
        SetSaveVal(9,0);
        ClearBitSet(4);
        ClearBitSet(8);
        for(int i = 210; i <= 220; i++)
        {
            ClearBitSet(i);
        }
        SetData8(1,0);
        SetData8(4,0);
        SetData8(5,0);
        SetData16(5,0);
        
        SetData8(9,0);
        
        SetData16(4,0);
        //SetData16(7,0);
        
        SetData8(8,0);
        
        ClearBitSet(163);
        SetData16(1,0);//清除比赛积分
        ClearBitSet(184);
        ClearBitSet(188);
        ClearBitSet(191);
        ClearBitSet(196);
        
        ClearBitSet(221);
        ClearBitSet(222);
        ClearBitSet(223);
        
        SetExtData8(3,0);
        SetData8(2,0);
    }
    if(m_shortArray[MAX_SAVE_NUM-1] < (uint32)GetClearTime())
    {
        SetSaveVal(4,0);
        SetData32(5,0);
        SetData32(9,0);
        
        ClearBitSet(98);
        ClearBitSet(99);
        ClearBitSet(160);
        ClearBitSet(161);
        ClearBitSet(162);
        ClearBitSet(183);
        
        SetData16(2,0);
        
        //SetData16(6,0);
         //10,11,12,13,14世界大战金木水火土
        SetData8(10,0);
        SetData8(11,0);
        SetData8(12,0);
        SetData8(13,0);
        SetData8(14,0);
        SetData8(15,0);
        //3 世界大战积分
        SetData16(3,0);
        ClearBitSet(187);
        
        DelPackageById(1558,-1);
        DelPackageById(1559,-1);
        DelPackageById(1560,-1);
        DelPackageById(1561,-1);
        DelPackageById(1562,-1);
        
        SetData32(12,0);
        m_shortArray[MAX_SAVE_NUM-1] = (uint32)GetClearTime();//GetSysTime();
    }
//#ifdef QQ
    if(!NewUserGift())//新手宝箱
    {
        NormalUserGift();//每日宝箱
    }
//#endif
}

int CUser::DuiHuanTB()
{
    int tongbao = 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 pos = 0; pos < MAX_PACKAGE_NUM; pos++)
    {
        if(m_package[pos].tmplId == 1840)
        {
            tongbao += 1000*m_package[pos].num;
            NoLockDelPackage(pos,m_package[pos].num);
        }
        else if(m_package[pos].tmplId == 1841)
        {
            tongbao += 100*m_package[pos].num;
            NoLockDelPackage(pos,m_package[pos].num);
        }
        else if(m_package[pos].tmplId == 1842)
        {
            tongbao += 10*m_package[pos].num;
            NoLockDelPackage(pos,m_package[pos].num);
        }
    }
    return tongbao;
}

bool CUser::DelPackageById(int id,int num)
{
    if(num == 0)
        return false;
    if(num < 0)
        num = 20*18;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 pos = 0; pos < MAX_PACKAGE_NUM2; pos++)
    {
        if(m_package[pos].tmplId == id)
        {
            if(m_package[pos].num > num)
            {
                m_package[pos].num -= num;
                num = 0;
            }
            else
            {
                num -= m_package[pos].num;
                memset(m_package + pos,0,sizeof(SItemInstance));
            }
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[pos],pos,msg);
            sock.SendMsg(m_sock,msg);
        }
        if(num <= 0)
        {
            return true;
        }
    }
    return false;
}

CUser::~CUser()
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        delete i->pNpc;
        delete i->pHumanData;
        i->pNpc = NULL;
        i->pHumanData = NULL;
    }
}

void CUser::MakeNpc(int npcId,CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        if(i->id == npcId)
        {
            SNpcTemplate *pNpc = i->pNpc;
            if(pNpc == NULL)
                return;
            msg<<pNpc->name<<(uint16)i->sceneId<<i->x<<i->y;
            return;
        }
    }
}

CCallScript *CUser::FindNpcScript(int npcId)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        if(i->id == npcId)
        {
            return i->pNpc->pScript;
        }
    }
    return NULL;
}

uint32 CUser::GetHumanNcpRoleId(int npcId)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        if((i->id == npcId) && (i->pHumanData != NULL))
        {
            return i->pHumanData->roleId;
        }
    }
    return 0;
}

const char *CUser::GetNpcName(int npcId)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        if(i->id == npcId)
        {
            return i->pNpc->name.c_str();
        }
    }
    return NULL;
}

void CUser::AddNpc(int scenseId,SNpcInstance &npc)
{
    //int npcId = 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    /*for(list<SNpcInstance>::iterator i = m_npcList.begin(); i != m_npcList.end(); i++)
    {
        if(npcId < i->id)
            npcId = i->id;
    }
    npc.id = ++npcId;*/
    m_npcList.push_back(npc);
}


uint16 CUser::DelNpc(int npcId,SNpcInstance &npc)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        if(npcId == i->id)
        {
            npc = *i;
            m_npcList.erase(i);
            return npc.sceneId;
        }
    }
    return 0;
}

int CUser::AddNpcInfo(int scenseId,CNetMessage &msg)
{
    int num = 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_npcList.empty())
        return 0;
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        if((scenseId == i->sceneId) && (i->pNpc != NULL))
        {
            //pInst = &(i->npc);
            //msg<<i->id<<i->pNpc->name<<i->x<<i->y<<i->pNpc->pic;
            i->MakeNpcInfo(msg);
            num++;
        }
    }
    return num;
}

bool CUser::FindNpcNear(SNpcInstance &npc)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    uint8 x,y;
    GetFacePos(x,y);
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        //uint8 x,y;
        //x = abs(m_xPos - i->x);
        //y = abs(m_yPos - i->y);
        if(((m_pScene->GetMapId() == i->sceneId) && (i->pNpc != NULL))
            && ((x == i->x) && (y == i->y)))
        {
            npc = *i;
            return true;
        }
    }
    return false;
}

CCallScript *CUser::GetTimeOutNpcScript()
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    list<SNpcInstance>::iterator i = m_npcList.begin();
    for(; i != m_npcList.end(); i++)
    {
        if((i->timeOut > 0) && (i->timeOut < GetSysTime()))
        {
            if(i->pNpc != NULL)
                return i->pNpc->pScript;
            else 
                return NULL;
        }
    }
    return NULL;
}

uint16 CUser::GetSceneId()
{
    if(m_pScene != NULL)
        return m_pScene->GetId();
    return 0;
}

uint16 CUser::GetMapId()
{
    if(m_pScene != NULL)
        return m_pScene->GetMapId();
    return 0;
}

bool CUser::ReadData(uint32 roleId,uint16 *pSceneId)
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb != NULL)
    {
        char buf[128];
        snprintf(buf,128,"select money,bd_money from user_info where id=%u",m_userId);
        if(pDb->Query(buf))
        {
            char **row = pDb->GetRow();
            if(row != NULL)
            {
                m_tongBao = atoi(row[0]);
                m_bdTongBao = atoi(row[1]);
            }
        }
    }
    
                              //0 1     2    3     4      5       6      7      8      9    10
    boost::format fmt("select id ,name ,sex ,head ,xiang ,map_id ,x_pos ,y_pos ,level ,exp ,tizhi ,"\
        //11        12      13      14      15      16    17    18  19  20    21            22  23
        "liliang ,minjie ,lingli ,naili ,shuxindian ,jin ,mu ,shui ,huo ,tu ,xiangxingdian ,hp ,mp, "\
        //24        25      26  27     28  29       30    31    32  33      34  35     36
        "equipment,package,tili,money,pet,pk_val,daohang,title,hots,bitset,shop,menpai,bangpai,"\
        //37          38    39      40          41      42              43              44
        "save_val,save_npc,qianneng,mp_gongxian,skills,use_double_end,pet_double_end,no_punish_end"\
            //45        46          47          48          49              50
        ",script_timer,chat_channel,bank_money,bank_item,few_monster_end,max_monster_end,"\
        //51            52              53              54          55      56
        "min_monster_end,up_monster_end,monster_script,save_pos,jianyu_time,aq_time_space,"\
        //57        58   59     60      61          62          63
        "chat_time,state,admin,pk_time,open_pack,script_double,save_data"\
        " from role_info where id=%1%");
        
    fmt % roleId;
    
    char **row = NULL;
    
    uint16 sceneId = 0;
    if ((pDb != NULL)
        && (pDb->Query(fmt.str().c_str()))
        && ((row = pDb->GetRow()) != NULL))
    {
        if(atoi(row[58]) == 2)
            return false;
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        m_loginTime = GetSysTime();
        m_answerTimes = 0;
        m_callLevelScript = false;
        sceneId = (uint16)atoi(row[5]);
        SetName(row[1]);
        SetHead(atoi(row[3]));
        SetSex(atoi(row[2]));
        SetXiang(atoi(row[4]));
        SetPos(atoi(row[6]),atoi(row[7]));
        SetLevel(atoi(row[8]));
        SetExp(atoi(row[9]));
        SetTiZhi(atoi(row[10]));
        SetLiLiang(atoi(row[11]));
        SetMinJie(atoi(row[12]));
        SetLingLi(atoi(row[13]));
        SetNaiLi(atoi(row[14]));
        SetShuXinDian(atoi(row[15]));
        SetJin(atoi(row[16]));
        SetMu(atoi(row[17]));
        SetShui(atoi(row[18]));
        SetHuo(atoi(row[19]));
        SetTu(atoi(row[20]));
        SetXiangXinDian(atoi(row[21]));
        SetHp(atoi(row[22]));
        SetMp(atoi(row[23]));
        SetEquip(row[24]);
        SetPackage(row[25]);
        //SetTili(atoi(row[26]));
        m_tili = atoi(row[26]);
        //SetMoney(atoi(row[27]));
        m_money = (uint32)atoi(row[27]);
        SetPet(row[28]);
        SetPkVal(atoi(row[29]));
        SetDaoHang(atoi(row[30]));
        //SetTitle(atoi(row[31]));
        //m_title = atoi(row[31]);
        //ReadTitle(row[31]);
        ReadTitle(row[31]);
        SetBitSet(row[33]);
        SetShop(row[34]);
        //SetMenPai(atoi(row[35]));
        m_menpai = atoi(row[35]);
        SetBangPai(atoi(row[36]));
        SetSaveVal(row[37]);
        SetNpc(row[38]);
        SetQianNeng(atoi(row[39]));
        SetMPGongXian(atoi(row[40]));
        SetSkills(row[41]);
        //m_enterSceneCall = atoi(row[42]);
        m_userDoubleEnd = atoi(row[42]);
        m_petDoubleEnd = atoi(row[43]);
        m_noPunishEnd = atoi(row[44]);
        m_chatChannel = atoi(row[46]);
        m_bankMoney = atoi(row[47]);
        SetBankItem(row[48]);
        m_fewMonsterEnd = atoi(row[49]);
        m_maxMonsterEnd = atoi(row[50]);
        m_minMonsterEnd = atoi(row[51]);
        m_upMonsterEnd = atoi(row[52]);
        m_jianYuTime = atoi(row[55]);
        m_answerTime = GetSysTime();
        m_askTime = 0;
        m_answerTimeSpace = atoi(row[56]);
        m_chatTime = atoi(row[57]);
        m_admin = atoi(row[59]);
        m_pkTime = atoi(row[60]);
        m_openPack = atoi(row[61]);
        //OpenPackage(m_openPack);
        MAX_PACKAGE_NUM = GetCurMaxPackNum();
            
        m_sDoubleEnd = atoi(row[62]);
        ReadSaveData(row[63]);
        
        if(m_jianYuTime > 0)
        {
            sceneId = SCENE_JIANYU;
            m_xPos = 7;
            m_yPos = 8;
        }
            
        memset(m_scriptTimer,0,sizeof(m_scriptTimer));
        if(row[45] != NULL)
        {
            char *p[MAX_TIMER*2] = {NULL};
            uint8 num = SplitLine(p,MAX_TIMER*2,row[45]);
            for(uint8 i = 0; i < num/2; i++)
            {
                if(p[2*i] != NULL)
                    m_scriptTimer[i].id = atoi(p[2*i]);
                if(p[2*i+1] != NULL)    
                    m_scriptTimer[i].endTime = atoi(p[2*i+1]);
            }
        }
        if(row[53] != NULL)
        {
            char *p[20];
            uint8 num = SplitLine(p,20,row[53]);
            int monsterId = 0;
            int scriptId = 0;
            m_monsterScript.clear();
            for(uint8 i = 0; i < num/2; i++)
            {
                monsterId = atoi(p[2*i]);
                scriptId = atoi(p[2*i+1]);
                m_monsterScript.insert(pair<int, int>(monsterId, scriptId));
            }
        }
        else
        {
            m_monsterScript.clear();
        }
        if(row[54] != NULL)
        {
            uint8 maxNum = 3*MAX_SAVE_POS;
            char *p[maxNum];
            uint8 num = SplitLine(p,maxNum,row[54]);
            memset(m_savePos,0,sizeof(m_savePos));
            for(uint8 i = 0; i < num/3; i++)
            {
                m_savePos[i].sceneId = atoi(p[3*i]);
                m_savePos[i].x = atoi(p[3*i+1]);
                m_savePos[i].y = atoi(p[3*i+2]);
            }
        }
        else
        {
            memset(m_savePos,0,sizeof(m_savePos));
        }
        Init();
        
        if(pSceneId != NULL)
        {
            *pSceneId = sceneId;
        }
        
        ReadHots(row[32]);
        
        return true;
    }
    return false;
}

void CUser::ReadHots(char *row)
{
    if(row == NULL) 
        return;
    int len = strlen(row);
    if(len <= 0)
        return;
        
    len /= 2;
    uint8 hexHots[len];
    StrToHex(row,hexHots,len);
    
    if(hexHots[0] == HOT_LIST_VERSION)
    {
        len--;
        if(len % 6 != 0)
            return;
        uint32 id;
        uint16 hotVal;
        uint8 *p = hexHots+1;
        for(uint8 i = 0; i < len/6; i++)
        {
            memcpy(&id,p+6*i,4);
            memcpy(&hotVal,p+6*i+4,2);
            AddHot(id,hotVal);
        }
    }
    else
    {
        if(len % 4 != 0)
            return;    
        uint32 id;
        for(uint8 i = 0; i < len/4; i++)
        {
            memcpy(&id,hexHots+4*i,4);
            AddHot(id);
        }
    }
}

bool CUser::CanChat()
{
    return GetSysTime() - m_chatTime > CHAT_SPACE;
}

void CUser::SetSkill(int skillId,int level)
{
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_SKILL);
    msg<<(uint16)skillId<<(uint16)level;
    sock.SendMsg(m_sock,msg);
            
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    list<UserSkill>::iterator i = m_userSkill.begin();
    for(; i != m_userSkill.end(); i++)
    {
        if(i->id == skillId)
        {
            i->level = level;
            return;
        }
    } 
    UserSkill skill;
    skill.id = skillId;
    skill.level = level;
    m_userSkill.push_back(skill);
}

void CUser::UpdateSkill()
{
    if(GetItemJiNeng() == 0)
        return;
        
    SendUpdateInfo(41,GetItemJiNeng());
}

int CUser::GetSrcSkillLevel(int skillId)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    list<UserSkill>::iterator i = m_userSkill.begin();
    for(; i != m_userSkill.end(); i++)
    {
        if(i->id == skillId)
        {
            return i->level;
        }
    } 
    return 0;
}

int CUser::GetSkillLevel(int skillId)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    list<UserSkill>::iterator i = m_userSkill.begin();
    for(; i != m_userSkill.end(); i++)
    {
        if(i->id == skillId)
        {
            return i->level + GetItemJiNeng();
        }
    } 
    return 0;
}

void CUser::MakeUserSkill(CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    msg<<(uint8)m_userSkill.size();
    list<UserSkill>::iterator i = m_userSkill.begin();
    for(; i != m_userSkill.end(); i++)
    {
        msg<<i->id<<(uint16)(i->level);
    } 
    msg<<(uint16)GetItemJiNeng();
}

void CUser::NoLockMakePetSkill(uint8 pos,CNetMessage &msg)
{
    uint16 len = msg.GetDataLen();
    
    uint8 num = 0;
    msg<<num;
    
    if(pos >= m_petNum)
        return;
    SPet *pPet = m_pet[pos].get();
    if(pPet == NULL)
        return;
    
    for(uint8 i = 0; i < SPet::MAX_SKILL_NUM; i++)
    {
        if(pPet->skill[i] != 0)
        {
            num++;
            msg<<pPet->skill[i]<<pPet->skillLevel[i];
        }
    }
    msg.WriteData(len,&num,1);
}
void CUser::MakePetSkill(uint8 pos,CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    NoLockMakePetSkill(pos,msg);
}

void CUser::SetSkills(char *row)
{
    m_userSkill.clear();
    if(row == NULL)
        return;
    char *p[30];
    uint8 num = SplitLine(p,30,row);
    for(int i = 0; 2*(i+1) < num; i++)
    {
        UserSkill skill;
        skill.id = atoi(p[2*i]);
        skill.level = atoi(p[2*i+1]);
        if(skill.id == 5)
            skill.id = 21;
        else if(skill.id == 17)
            skill.id = 22;
        else if(skill.id == 109)
            skill.id = 121;
            
        m_userSkill.push_back(skill);
    }
}

bool CUser::HaveItem(int id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(int i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if(m_package[i].tmplId == id)
        {
            return true;
        }        
    }
    return false;
}

uint8 CUser::HaveEmptyPack()
{
    uint8 num = 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(int i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if(m_package[i].tmplId == 0)
        {
            num++;
        }        
    }
    return num;
}

void CUser::AddTimer(int id,int waitTime)
{
    for(uint8 i = 0; i < MAX_TIMER; i++)
    {
        if(m_scriptTimer[i].id == 0)
        {
            m_scriptTimer[i].id = id;
            m_scriptTimer[i].endTime = GetSysTime() + waitTime*60;
            return;
        }
    }
}

void CUser::DelTimer(int id)
{
    for(uint8 i = 0; i < MAX_TIMER; i++)
    {
        if(m_scriptTimer[i].id == id)
        {
            m_scriptTimer[i].id = 0;
            m_scriptTimer[i].endTime = 0;
            return;
        }
    }
}

void CUser::TimeOut()
{
    if((GetSceneId() == SCENE_JIANYU)
        //&& (m_jianYuTime >= 0) 
        && (GetSysTime() - m_loginTime > m_jianYuTime))
    {
        m_jianYuTime = 0;
        m_pkVal = 0;
        if(m_pScene != NULL)
            m_pScene->LeaveTeam(this);
        TransportUser(this,1,13,13,8);
        SendUpdateInfo(40,m_pkVal);
        return;
    }
    uint16 mapId = 0;
    if(m_pScene == NULL)
        return;
        
    mapId = m_pScene->GetMapId();
    
    if(!InWorldWar() && (mapId >= 271) && (mapId <= 276))
    {
        if((m_bangpai == 0) || ((int)m_bangpai != WWGetWinBang()))
        {
            m_pScene->LeaveTeam(this);
            TransportUser(this,270,12,14,8);
            return;
        }
    }
    if((mapId >= 260) && (mapId <= 266))
    {
        if((GetHour() == BANG_ZHAN_END_TIME) && (m_fightId == 0))
        {
            m_pScene->LeaveTeam(this);
            TransportUser(this,BANG_PAI_SCENE_ID,15,15,8);
            return;
        }
    }
    if(InWorldWar() && (m_teamId != 0))
    {
        CUser *p = m_pScene->GetTeamMember1(m_teamId);
        if(p != NULL)
        {
            if((IsFangShou(p->GetBangPai()) && !IsFangShou(m_bangpai)) || ((!IsFangShou(p->GetBangPai()) && IsFangShou(m_bangpai))))
            {
                m_pScene->LeaveTeam(this);
            }
            else if((p = m_pScene->GetTeamMember2(m_teamId)) != NULL)
            {
                if((IsFangShou(p->GetBangPai()) && !IsFangShou(m_bangpai)) || ((!IsFangShou(p->GetBangPai()) && IsFangShou(m_bangpai))))
                {
                    m_pScene->LeaveTeam(this);
                }
            }
        }
    }
    
    if((GetData8(6) != 0) && (GetSysTime() - GetData32(8) >= 2*3600))
    {
        SetData8(6,0);
        SendUpdateInfo(44,0);
    }
    
    UpdateMission();
    if(GetSysTime() - m_addTiliTime > ADD_TILI_TIME_SPACE)
    {
        AddTili(Random(5,10));
        m_addTiliTime = GetSysTime();
    }
    ClearTimeoutTitle();
    
    for(uint8 i = 0; i < MAX_TIMER; i++)
    {
        if((m_scriptTimer[i].id != 0) && (m_scriptTimer[i].endTime < GetSysTime()))
        {
            /*int id = 10000;        
            char buf[16];
            sprintf(buf,"%d.lua",id);
            CCallScript script(buf);
            SetCallScript(id);*/
            CCallScript *pScript = GetScript();
            if(pScript != NULL)
            {            
                pScript->Call("TimeOut","ui",this,m_scriptTimer[i].id);
                SetCallScript(pScript->GetScriptId());
                m_scriptTimer[i].id = 0;
                m_scriptTimer[i].endTime = 0;
            }
        }
    }
    SendItemTimeOut();
    
    if((m_askTime != 0) && (GetSysTime() - m_askTime > ANSWER_QUESTION_TIME))
    {
        string msg = m_name;
        msg.append("惹怒了星灵仙子，受到了惩罚");
        SendMsgToTeamMember(msg.c_str());
        SendPopMsg(this,"你答题错误，惹怒了星灵仙子，她生气了。快去星寿村向她道歉吧!");
        //shutdown(m_sock,SHUT_RD);
        m_askTime = 0;
        SetBitSet(0);
    }
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    //SaveData();
    for(uint8 i = 0; i < m_petNum; i++)
    {
        SharePetPtr pet = m_pet[i];
        SPet *pPet = pet.get();
        if((pPet != NULL) && (pPet->shouming > 0))
        {
            if(GetSysTime() - pPet->shouMingTime > 3600)
            {
                int desShouMing = 4*((GetSysTime() - pPet->shouMingTime)/3600);
                if(pPet->shouming > desShouMing)
                    pPet->shouming -= desShouMing;
                else 
                    pPet->shouming = 0;
                UpdatePetInfo(i,12,pPet->shouming);
                pPet->shouMingTime = GetSysTime();
            }
        }
    }
    /*for(uint8 i = 0; i < MAX_SHOP_ITEM_NUM; i++)
    {
        if(m_shopItem[i].type == ESITPet)
        {
            if(GetSysTime() - m_shopItem[i].pet.shouMingTime > 3600)
            {
                int desShouMing = 4*((GetSysTime() - pPet->shouMingTime)/3600);
                if(pPet->shouming > desShouMing)
                    pPet->shouming -= desShouMing;
                else 
                    pPet->shouming = 0;
                //UpdatePetInfo(i,12,pPet->shouming);
                pPet->shouMingTime = GetSysTime();
            }
        }
    }*/
}

void CUser::AddMoney(int add)
{
    m_money += add;
    SendUpdateInfo(26,m_money);
    if(m_money > m_level*m_level*625 + 1000)
    {
        //您携带金钱上线为：
        char buf[64];
        snprintf(buf,63,"您携带金钱上限为:%d，多余的钱会在下线时被删。请尽快转移",m_level*m_level*625 + 1000);
        SendPopMsg(this,buf);
    }
}

bool CUser::CompoundStone(uint8 pos,uint8 num)
{
    if(pos >= MAX_PACKAGE_NUM)
        return false;
    if(num < 3)
        return false;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SItemInstance *pStone = m_package+pos;
    if(pStone->num < num)
        return false;
    if((pStone->tmplId != 611)
        && (pStone->tmplId != 640)
        && (pStone->tmplId != 641)
        && (pStone->tmplId != 642))
        return false;
        
    SItemInstance compoundStone = *pStone;
    compoundStone.level++;
    
    uint8 compoundPos = 0xff;
    //uint8 bangDing = 0;
    
    for(int i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if((m_package[i] == compoundStone) && (m_package[i].num + num/3 <= EItemDieJiaNum))
        {
            
            compoundPos = i;
            break;
        }
    }
    if(compoundPos > MAX_PACKAGE_NUM)
    {
        for(int i = 0; i < MAX_PACKAGE_NUM; i++)
        {
            if(m_package[i].tmplId == 0)
            {
                compoundPos = i;
                break;
            }
        }
    }
    if(compoundPos > MAX_PACKAGE_NUM)
    {
        return false;
    }
    uint8 comNum = num/3;
    num = comNum + m_package[compoundPos].num;
    m_package[compoundPos] = compoundStone;
    m_package[compoundPos].num = num;
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PACK);
    m_package[compoundPos].bangDing = m_package[pos].bangDing;//bangDing;
    MakePack(m_package[compoundPos],compoundPos,msg);
    sock.SendMsg(m_sock,msg);
    
    string end;
    HexToStr(m_package[compoundPos],end);
    SaveUseItem(m_roleId,m_package[pos],"合成",comNum * 3,"",end);
    NoLockDelPackage(pos,comNum * 3);
    return true;
}

int CUser::GetSkillDamage(int skillId)//攻击技能伤害
{
    int xiang = 0;
    switch(m_xiang)
    {
    case EXJinXiang://金
        xiang = GetJin();
        break;
    case EXMuXiang://2－木
        xiang = GetMu();
        break;
    case EXShuiXiang://3－水
        xiang = GetShui();
        break;
    case EXHuoXiang://4－火
        xiang = GetHuo();
        break;
    case EXTuXiang://5－土
        xiang = GetTu();
        break;
    default:
        break;
    }
    /**********
    当门派为“金”时，法术伤害公式为：
    等级*14+(灵性-等级)*3.6+金相*43.2+法术基础伤害*法术等级/3+法术基础伤害*12+装备附加
    其他门派
    等级*12+(灵性-等级)*3+(本相+金相)*18+法术基础伤害*法术等级/3+法术基础伤害*10+装备附加
    等级*12+(灵性-等级)*3+本相*20+金相*39+法术基础伤害*法术等级/3+法术基础伤害*10+装备附加
    *************/
    uint8 jie = GetSkillJie(skillId);
    int base = 0;
    if(jie == 1)
        base = 40;
    else if(jie == 2)
        base = 20;
    else if(jie == 3)
        base = 8;
    else if(jie == 4)
        base = 60;
    int lingli = GetLingLi();
    int jin = GetJin();
    //int 
    //当门派为“木”时，法术伤害公式为：
    //等级*12+(灵性-等级)*3+金相*39+法术基础伤害*法术等级/3+法术基础伤

    if(m_xiang == EXJinXiang)
        //m_skillDamage = (int)(m_level*14+(lingli-m_level)*3.6+jin*43.2+
                            //base*GetSkillLevel(skillId)/3+base*12+GetItemGongji());
        m_skillDamage = (int)((m_level*3+lingli-m_level+jin*2)*base/12
                            +jin*20+base*GetSkillLevel(skillId)+GetItemGongji());
        //（用户等级*3+灵性-等级+金相*2）*技能基础攻击/12+金相*20+技能基础攻击*技能等级+装备附加
    else if((m_xiang == EXShuiXiang) || (m_xiang == EXHuoXiang))
        //m_skillDamage = (int)(m_level*12+(lingli-m_level)*3+jin*39+
                            //base*GetSkillLevel(skillId)/3+base*10+GetItemGongji());
        m_skillDamage = (int)((m_level*2+lingli-m_level+xiang*2)*
                            base/12+jin*20+base*GetSkillLevel(skillId)+GetItemGongji());
        //（用户等级*2+灵性-等级+本相*2）*技能基础攻击/12+金相*20+技能基础攻击*技能等级+装备附加
    else
        //m_skillDamage = (int)(m_level*12+(lingli-m_level)*3+xiang*20+jin*39+
                            //base*GetSkillLevel(skillId)/3+base*10+GetItemGongji());
        m_skillDamage = (int)((m_level*2+lingli-m_level)*
                            base/12+jin*20+base*GetSkillLevel(skillId)+GetItemGongji());
        //（用户等级*2+灵性-等级）*技能基础攻击/12+金相*20+技能基础攻击*技能等级+装备附加
    return m_skillDamage;
}

void CUser::GetViewPara(int &fagong,uint16 &mingzhong,uint16 &huibi)
{
    /*********************
    当门派为“金”时，法术伤害公式为：
    (等级*19+(灵性-等级)*7.2+(本相+金相)*36)/120+1200+装备附加
    其他门派的法术伤害公式为：
    (等级*16+(灵性-等级)*6+(本相+金相)*30)/120+1200+装备附加
    ***********************/
    int tizhi = GetTiZhi();
    int liliang = GetLiLiang();
    int minjie = GetMinJie();
    int naili = GetNaiLi();
    int mu = GetMu();
    int huo = GetHuo();
    int jin = GetJin();
    int shui = GetShui();
    int tu = GetTu();
    int lingli = GetLingLi();
    int benxiang = 0;
    switch(m_xiang)
    {
    case EXJinXiang:
        benxiang = jin;
        break;
    case EXMuXiang:
        benxiang = mu;
        break;
    case EXShuiXiang:
        benxiang = shui;
        break;
    case EXHuoXiang:
        benxiang = huo;
        break;
    case EXTuXiang:
        benxiang = tu;
        break;
    }
    if(m_xiang == EXJinXiang)
    {//等级*14+(灵性-等级)*3.6+金相*43.2+400+装备附加
        //fagong = (int)(m_level*14+(lingli-m_level)*3.6
                        //+jin*43.2+400+GetItemGongji());
        //用户等级*10+（灵性-等级）*5+金相*30+装备附加
        fagong = (int)(m_level*10+(lingli-m_level)*5+jin*30+GetItemGongji());
    }
    else if((m_xiang == EXHuoXiang) || (m_xiang == EXShuiXiang))
    {
        //fagong = (int)(m_level*12+(lingli-m_level)*3+jin*39+
                            //380+GetItemGongji());
        //用户等级*8+（灵性-等级）*5+金相*20+本相*10+装备附加
        fagong = (int)(m_level*8+(lingli-m_level)*5+jin*20+benxiang*10+GetItemGongji());
    }
    else
    {//等级*12+(灵性-等级)*3+本相*20+金相*39+380+装备附加
        //fagong = (int)(m_level*12+(lingli-m_level)*3
                        //+benxiang*20 + jin*39+380+GetItemGongji());
        //用户等级*8+（灵性-等级）*5+金相*20+装备附加
        fagong = (int)(m_level*8+(lingli-m_level)*5+jin*20+GetItemGongji());
    }
    
    if(m_xiang == EXTuXiang)//等级*3+(力量-等级)*0.75+(体质-等级)*0.75+金相*4+水相*2+45
        mingzhong = (int)(m_level*3+(liliang-m_level)*0.75+(tizhi-m_level)*0.75+jin*4+shui*2+45);
    else//等级*4+(力量-等级)*1.5+(体质-等级)*1.5+金相*8+水相*4+60
        mingzhong = (int)(m_level*4+(liliang-m_level)*1.5+(tizhi-m_level)*1.5+jin*8+shui*4+60);
    
    if(m_xiang == EXMuXiang)//等级*8+(敏捷-等级)*2+(耐力-等级)*1+火相*4+木相*7+100
        huibi = (int)(m_level*8+(minjie-m_level)*2+(naili-m_level)+huo*4+mu*7+100);
    else if((m_xiang == EXJinXiang) || (m_xiang == EXShuiXiang))
        //等级*2+(敏捷-等级)*0.5+(耐力-等级)*0.25+火相*1+木相*3+30
        huibi = (int)(m_level*2+(minjie-m_level)*0.5+(naili-m_level)*0.25
                        +huo+mu*3+30);
    else//等级*4+(敏捷-等级)*1+(耐力-等级)*0.5+火相*2.5+木相*5.5+60
        huibi = (int)(m_level*4+(minjie-m_level)+(naili-m_level)*0.5+huo*2.5+mu*5.5+60);
    mingzhong += GetItemMingzhong();
    huibi += GetItemHuibi();
}

static void GetBlueAttr(SItemInstance *m_package,uint8 pos1,uint8 pos2,uint8 pos3,map<uint8,uint16> &attrVal)
{
    SItemInstance *pItem = m_package + pos1;
    for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
    {
        if((pItem->addAttrType[i] != 0) && InBlueEquip(pItem->addAttrType[i]))
        {
            attrVal.insert(make_pair(pItem->addAttrType[i],pItem->addAttrVal[i]));
        }
    }
    
    pItem = m_package + pos2;
    for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
    {
        map<uint8,uint16>::iterator iter = attrVal.find(pItem->addAttrType[i]);
        if((pItem->addAttrType[i] != 0) && InBlueEquip(pItem->addAttrType[i]))
        {
            if(iter != attrVal.end())
            {
                if(pItem->addAttrVal[i] > iter->second)
                    iter->second = pItem->addAttrVal[i];
            }
            else
            {
                attrVal.insert(make_pair(pItem->addAttrType[i],pItem->addAttrVal[i]));
            }
        }
    }
    
    pItem = m_package + pos3;
    for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
    {
        map<uint8,uint16>::iterator iter = attrVal.find(pItem->addAttrType[i]);
        if((pItem->addAttrType[i] != 0) && InBlueEquip(pItem->addAttrType[i]))
        {
            if(iter != attrVal.end())
            {
                if(pItem->addAttrVal[i] > iter->second)
                    iter->second = pItem->addAttrVal[i];
            }
            else
            {
                attrVal.insert(make_pair(pItem->addAttrType[i],pItem->addAttrVal[i]));
            }
        }
    }
}

/*uint8 CUser::GetItemPos(uint16 id)
{
    for(int i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if(m_package[i].tmplId == id)
        {
            return i;
        }        
    }
    return 0;
}*/

int CUser::CompoundItem1(int id)
{
    uint16 heChengItem[] = {601,602,603,401,
                            604,605,606,451,
                            607,608,609,501};
    for(uint8 i = 0; i < sizeof(heChengItem)/sizeof(uint16)/4; i++)
    {
        uint8 pos1 = 0xff,pos2 = 0xff,pos3 = 0xff;
        if(id == heChengItem[4*i+3])
        {
            for(uint8 j = 0; j < MAX_PACKAGE_NUM; j++)
            {
                if(m_package[j].tmplId == heChengItem[4*i])
                {
                    pos1 = j;
                }
                else if(m_package[j].tmplId == heChengItem[4*i+1])
                {
                    pos2 = j;
                }
                else if(m_package[j].tmplId == heChengItem[4*i+2])
                {
                    pos3 = j;
                }
            }
            if((pos1 != 0xff) && (pos2 != 0xff) && (pos3!= 0xff))
            {
                return CompoundItem(pos1,pos2,pos3);
            }
            return 0;
        }
    }
    if(((id >= 402) && (id <= 412))
        || ((id >= 452) && (id <= 462))
        || ((id >= 502) && (id <= 512)))
    {
        uint8 pos1 = 0xff,pos2 = 0xff,pos3 = 0xff;
        for(int i = 0; i < MAX_PACKAGE_NUM; i++)
        {
            if(m_package[i].tmplId == id - 1)
            {
                pos1 = i;
            }        
            else if(m_package[i].tmplId == id - 1)
            {
                pos2 = i;
            }
            else if(m_package[i].tmplId == id - 1)
            {
                pos3 = i;
            }
        }
        if((pos1 != 0xff) && (pos2 != 0xff) && (pos3!= 0xff))
        {
            return CompoundItem(pos1,pos2,pos3);
        }
        return 0;
    }
    return 0;
}

//合成物品，返回合成物品id，0时表示合成失败，-1表示包裹已满
int CUser::CompoundItem(uint8 pos1,uint8 pos2,uint8 pos3)
{
    if((pos1 >= MAX_PACKAGE_NUM) || (pos2 >= MAX_PACKAGE_NUM) 
        || (pos3 >= MAX_PACKAGE_NUM))
    {
        return 0;
    }
    if((pos1 == pos2) || (pos1 == pos3) || (pos2 == pos3))
        return 0;
    uint16 items[3];
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    items[0] = m_package[pos1].tmplId;
    items[1] = m_package[pos2].tmplId;
    items[2] = m_package[pos3].tmplId;
    
    std::sort(items,items+3);
    
    uint16 itemId = 0;
    uint16 heChengItem[] = {601,602,603,401,
                            604,605,606,451,
                            607,608,609,501};
    for(uint8 i = 0; i < sizeof(heChengItem)/sizeof(uint16)/4; i++)
    {
        if(memcmp(items,heChengItem+4*i,sizeof(items)) == 0)
        {
            itemId = heChengItem[4*i+3];
            break;
        }
    }
    map<uint8,uint16> attrMap;
    if((items[0] == items[1]) && (items[0] == items[2]))
    {
        if(((items[0] >= 401) && (items[0] <= 411))
            || ((items[0] >= 451) && (items[0] <= 461))
            || ((items[0] >= 501) && (items[0] <= 511)))
        {
            itemId = items[0] + 1;
            GetBlueAttr(m_package,pos1,pos2,pos3,attrMap);
        }
    }
    if(itemId == 0)
        return 0;
    bool haveEmptyPackage = false;
    
    if((m_package[pos1].num == 1) 
        || (m_package[pos2].num == 1)
        || (m_package[pos3].num == 1))
    {
        haveEmptyPackage = true;
    }
    else
    {
        for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
        {
            if(m_package[i].tmplId == 0)
            {
                haveEmptyPackage = true;
                break;
            }
        }
    }
    if(!haveEmptyPackage)
        return -1;
    uint8 bangDing = 0;
    if((m_package[pos1].bangDing == 1) 
        || (m_package[pos2].bangDing == 1)
        || (m_package[pos3].bangDing == 1))
    {
        bangDing = 1;
    }
    
    
    SItemInstance inst = {0};
    inst.num = 1;
    inst.tmplId = itemId;
    inst.bangDing = bangDing;
    
    if(!attrMap.empty())
    {
        uint8 num = 0;
        /*for(uint8 i = 0; i < sizeof(BLUE_EQUIP); i++)
        {
            iter = attrMap.find(BLUE_EQUIP[i]);
            if(iter != attrMap.end())
            {
                inst.addAttrType[num] = iter->first;
                inst.addAttrVal[num] = iter->second;
                num++;
                if(num >= 3)
                    break;
            }
        }*/
        for(uint8 i = 0; i < 3; i++)
        {
            uint8 attrNum = attrMap.size();
            if(attrNum == 0)
                break;
            int r = Random(0,attrNum-1);
            map<uint8,uint16>::iterator iter = attrMap.begin();
            for(uint8 j = 0; j < r; j++)
            {
                iter ++;
            }
            inst.addAttrType[num] = iter->first;
            inst.addAttrVal[num] = iter->second;
            num++;
            attrMap.erase(iter);
        }
        inst.quality = EQTBlue;
        inst.addAttrNum = num;
    }
    string end;
    HexToStr(inst,end);
    
    SaveUseItem(m_roleId,m_package[pos1],"合成物品",1,"",end);
    NoLockDelPackage(pos1);
    SaveUseItem(m_roleId,m_package[pos2],"合成物品",1,"",end);
    NoLockDelPackage(pos2);
    SaveUseItem(m_roleId,m_package[pos3],"合成物品",1,"",end);
    NoLockDelPackage(pos3);
    NoLockAddPackage(inst);
    //SaveUseItem(m_roleId,inst,"合成物品",1);
    return itemId;
}

bool CUser::JianDingShiPin(uint8 pos)
{
    if(pos >= MAX_PACKAGE_NUM)
        return false;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    SItemInstance *pItem = m_package + pos;
    if((pItem->tmplId < 401) || (pItem->tmplId > 511))
        return false;
        
    uint8 addAttrType[SItemInstance::MAX_ADD_ATTR_NUM] = {0};
    uint16 addAttrVal[SItemInstance::MAX_ADD_ATTR_NUM] = {0};
    uint8 num = 0;
    for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
    {
        if(pItem->addAttrType[i] != 0)
        {
            if(!InBlueEquip(pItem->addAttrType[i]))
            {
                addAttrType[num] = pItem->addAttrType[i];
                addAttrVal[num] = pItem->addAttrVal[i];
                num++;
            }
            pItem->addAttrType[i] = 0;
            pItem->addAttrVal[i] = 0;
        }
    }
    pItem->addAttrNum = 0;
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pTmpl = itemMgr.GetItem(pItem->tmplId);
    //uint8 itemType = 0;
    if(pTmpl == NULL)
        return false;
        //itemType = pTmpl->type;
    
    vector<uint8> types;    
    for(uint8 i = 0; i < sizeof(BLUE_EQUIP); i++)
    {
        if(CanAddAttr(BLUE_EQUIP[i],pTmpl->type))
        {
            types.push_back(BLUE_EQUIP[i]);
        }
    }
    uint8 ind = 0;
    uint16 min = 0,max = 1,middle = 1;
    num = Random(1,3);
    for(uint8 i = 0; i < num; i++)
    {
        ind = Random(0,types.size()-1);
        uint8 type = types[ind];//BLUE_EQUIP[Random(0,sizeof(BLUE_EQUIP)-1)];
        types.erase(types.begin()+ind);
        min = 0;max = 1; middle = 1;
        GetAddAttrVal(type,min,middle,max);
        pItem->addAttrType[i] = type;
        pItem->addAttrVal[i] = Random(min,middle);
    }
    pItem->addAttrNum = num;
    for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
    {
        if((addAttrType[i] == 0) || (pItem->addAttrNum+1 > SItemInstance::MAX_ADD_ATTR_NUM))
            break;
        pItem->addAttrType[pItem->addAttrNum] = addAttrType[i];
        pItem->addAttrVal[pItem->addAttrNum] = addAttrVal[i];
        pItem->addAttrNum++;
    }
    m_package[pos].quality = EQTBlue;
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PACK);
    MakePack(m_package[pos],pos,msg);
    sock.SendMsg(m_sock,msg);
    
    return true;
}

bool CUser::XiangQian(uint8 pos,uint8 stone)
{
    if((pos >= MAX_PACKAGE_NUM) || (stone >= MAX_PACKAGE_NUM))
        return false;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    SItemInstance *pItem = m_package + pos;
    if((pItem->tmplId < 401) || (pItem->tmplId > 511))
        return false;
    string before;
    HexToStr(*pItem,before);
    
    SItemInstance *pStone = m_package + stone;
    if(pStone->tmplId != 630)
        return false;
     
    for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
    {
        if((pItem->addAttrType[i] != 0) && !InBlueEquip(pItem->addAttrType[i]))
        {
            pItem->addAttrType[i] = 0;
            pItem->addAttrVal[i] = 0;
            if(pItem->addAttrNum > 0)
                pItem->addAttrNum--;
        }
    }
       
    pItem->addAttrNum += Random(1,3);
    
    uint16 min,max,middle;
    
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pTmpl = itemMgr.GetItem(pItem->tmplId);
    uint8 itemType = 0;
    if(pTmpl != NULL)
        itemType = pTmpl->type;
    
    vector<uint8> types;    
    for(uint8 i = 0; i < sizeof(GREEN_EQUIP); i++)
    {
        if(CanAddAttr(GREEN_EQUIP[i],pTmpl->type))
        {
            types.push_back(GREEN_EQUIP[i]);
        }
    }
    uint8 ind = 0;
    for(uint8 i = 0; i < pItem->addAttrNum; i++)
    {
        if(pItem->addAttrType[i] == 0)
        {
            ind = Random(0,types.size()-1);
            uint8 type = types[ind];//BLUE_EQUIP[Random(0,sizeof(BLUE_EQUIP)-1)];
            types.erase(types.begin()+ind);
            GetAddAttrVal(type,min,middle,max);
            pItem->addAttrType[i] = type;
            pItem->addAttrVal[i] = Random(min,middle);
        }
    }
    pItem->naijiu = Random(10,150);
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PACK);
    MakePack(m_package[pos],pos,msg);
    sock.SendMsg(m_sock,msg);
    string end;
    HexToStr(*pItem,end);
    
    SaveUseItem(m_roleId,m_package[stone],"镶嵌使用",1,before,end);
    NoLockDelPackage(stone);
    return true;
}
/***************
    TYPE=1 状态
   TYPE=2 属性点
   TYPE=3 武学
   TYPE=4 经验
   TYPE=5 气血
   TYPE=6 法力
   TYPE=7 体质
   TYPE=8 力量
   TYPE=9 敏捷
   TYPE=10 灵力
   TYPE=11 耐力
   TYPE=12 寿命
   TYPE=13 忠诚度
   TYPE=14 亲密度
    ****************/
void CUser::UpdatePetInfo(uint8 petPos,uint8 type,int val)
{
    if(petPos > m_petNum)
        return;
        
    CSocketServer &sock = SingletonSocket::instance();
    
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PET_PARAM);
    msg<<petPos<<type<<val;
    sock.SendMsg(m_sock,msg);
}

void CUser::GetDropItem(string &item)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    item = m_dropItem;
    m_dropItem.clear();
}

//掉落未绑定装备
void CUser::DropItem(string &name)
{
    //未绑定装备位置
    uint8 equipPos[EQUIPMENT_NUM];
    uint8 num = 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        if((m_equipment[i].tmplId != 0) && (m_equipment[i].bangDing == 0))
        {
            equipPos[num] = i;
            num++;
        }
    }
    if(num == 0)
        return;
    uint8 delPos = RandSelect(equipPos,num);
    if(delPos < EQUIPMENT_NUM)
    {
        CItemTemplateManager &itemMgr = SingletonItemManager::instance();
        SItemTemplate *pItem = itemMgr.GetItem(m_equipment[delPos].tmplId);
        if(pItem != NULL)
            name.append(pItem->name);
        if(m_equipment[delPos].naijiu != 0)
        {
            Init();
            UpdateInfo();
        }
        m_dropItem.clear();
        HexToStr(m_equipment[delPos],m_dropItem);
        memset(m_equipment+delPos,0,sizeof(SItemInstance));
        UpdateEquip(delPos);
    }
}

void CUser::UpdateEquip(uint8 pos)
{
    if(pos >= EQUIPMENT_NUM)
        return;
        
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_SERVER_UPDATE_EQUIP);
    msg<<pos;//<<m_equipment[pos].tmplId;
    MakeItemInfo(m_equipment+pos,msg);
    sock.SendMsg(m_sock,msg);
}

struct SAddHpMpItem
{
    uint8 pos;
    uint16 id;
};

bool AddHpMpLess(const SAddHpMpItem &item1,const SAddHpMpItem &item2)
{
   return item1.id<item2.id;
}

void CUser::UseAddHpItem()
{
    //1804,651-682							
    SAddHpMpItem addHpItemPos[MAX_PACKAGE_NUM2] = {{0,0}};
    uint8 num = 0;
    for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if((m_package[i].tmplId == 1804) || (m_package[i].tmplId == 1822)
            || ((m_package[i].tmplId >= 651) && (m_package[i].tmplId <= 682)))
        {
            addHpItemPos[num].id = m_package[i].tmplId;
            addHpItemPos[num].pos = i;
            num++;
        }
    }
    std::sort(addHpItemPos,addHpItemPos+num,AddHpMpLess);
    if(num == 0)
        return;
    if(addHpItemPos[num-1].id == 1804)
        std::swap(addHpItemPos[num-1],addHpItemPos[0]);
    for(uint8 i = 0; i < num; i++)
    {
        if(m_hp < m_maxHp)
        {
            int hp = m_hp;
            while(m_package[addHpItemPos[i].pos].num > 0)
            {
                UseItem(addHpItemPos[i].pos);
                if(hp == m_hp)
                    break;
                else
                    hp = m_hp;
            }
        }
        if(m_chuZhanPet < m_petNum)
        {
            SPet *pPet = m_pet[m_chuZhanPet].get();
            if((pPet != NULL) && (pPet->hp < pPet->maxHp))
            {
                int hp = pPet->hp;
                while(m_package[addHpItemPos[i].pos].num > 0)
                {
                    UseItemToPet(m_chuZhanPet,addHpItemPos[i].pos);
                    if(hp == pPet->hp)
                        break;
                    else
                        hp = pPet->hp;
                }
            }
        }
    }
}

void CUser::UserAddMpItem()
{
    //1806,701-730
    SAddHpMpItem addHpItemPos[MAX_PACKAGE_NUM2] = {{0,0}};
    uint8 num = 0;
    for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if((m_package[i].tmplId == 1805) 
            || ((m_package[i].tmplId >= 701) && (m_package[i].tmplId <= 730)))
        {
            addHpItemPos[num].id = m_package[i].tmplId;
            addHpItemPos[num].pos = i;
            num++;
        }
    }
    std::sort(addHpItemPos,addHpItemPos+num,AddHpMpLess);
    if(num == 0)
        return;
    if(addHpItemPos[num-1].id == 1805)
        std::swap(addHpItemPos[num-1],addHpItemPos[0]);
    
    for(uint8 i = 0; i < num; i++)
    {
        if(m_mp < m_maxMp)
        {
            int mp = m_mp;
            while(m_package[addHpItemPos[i].pos].num > 0)
            {
                UseItem(addHpItemPos[i].pos);
                if(mp == m_mp)
                    break;
                else
                    mp = m_mp;
            }
        }
        if(m_chuZhanPet < m_petNum)
        {
            SPet *pPet = m_pet[m_chuZhanPet].get();
            if((pPet != NULL) && (pPet->mp < pPet->maxMp))
            {
                int mp = pPet->mp;
                while(m_package[addHpItemPos[i].pos].num > 0)
                {
                    UseItemToPet(m_chuZhanPet,addHpItemPos[i].pos);
                    if(mp == pPet->mp)
                        break;
                    else
                        mp = pPet->mp;
                }
            }
        }
    }
}

void CUser::MoveItem(uint8 srcPos,uint8 tarPos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    uint8 curMaxPackNum = GetCurMaxPackNum();
    if((srcPos >= MAX_PACKAGE_NUM2) || (tarPos >= MAX_PACKAGE_NUM2))
        return;
    if(srcPos == tarPos)
        return;
        
    CSocketServer &sock = SingletonSocket::instance();
    
    if((m_package[srcPos].tmplId == 0) || (m_package[tarPos].tmplId == 0))
    {//有一个格子是空，交换位置
        if((tarPos >= curMaxPackNum) && (m_package[tarPos].tmplId == 0))
            return;
        
        if((srcPos >= curMaxPackNum) && (m_package[srcPos].tmplId == 0))
            return;
            
        std::swap(m_package[srcPos],m_package[tarPos]);
        CNetMessage msg;
        msg.SetType(PRO_UPDATE_PACK);
        MakePack(m_package[srcPos],srcPos,msg);
        sock.SendMsg(m_sock,msg);
        msg.ReWrite();
        msg.SetType(PRO_UPDATE_PACK);
        MakePack(m_package[tarPos],tarPos,msg);
        sock.SendMsg(m_sock,msg);
        return;
    }
    
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItemSrc = itemMgr.GetItem(m_package[srcPos].tmplId);
    SItemTemplate *pItemTar = itemMgr.GetItem(m_package[tarPos].tmplId);
    if((pItemSrc == NULL) || (pItemTar == NULL))
        return;

    bool canDieJia = false;            
    if((pItemSrc->id == pItemTar->id) && (m_package[srcPos] == m_package[tarPos]))
    {//类型相同
        if((pItemSrc->type == EITPKYaoPin) || 
            (pItemSrc->type == EITAddAttr) ||
            (pItemSrc->type == EITNormalYaoPin) ||
            (pItemSrc->type == EITMission) ||
            (pItemSrc->type == EITCanDelMiss) ||
            (pItemSrc->type == EITMissionCanSave) ||
            (pItemSrc->type == EITPetBook) || 
            (pItemSrc->type == EITChargeItem))
        {
            canDieJia = true;
        }
    }
    if((pItemSrc->id == 1809) || (pItemSrc->id == 1815)
        || (pItemSrc->id == 683) || (pItemSrc->id == 684)
        || (pItemSrc->id == 1804) || (pItemSrc->id == 1805)
        || (pItemSrc->id == 1822)
        || ((pItemSrc->id >= 1827) && (pItemSrc->id <= 1831)))
        canDieJia = false;
        
    if(canDieJia)
    {
        if(tarPos >= curMaxPackNum)
            return;
        uint8 tolNum = m_package[srcPos].num + m_package[tarPos].num;
        if(tolNum <= EItemDieJiaNum)
        {
            m_package[tarPos].num = tolNum;
            memset(m_package+srcPos,0,sizeof(SItemInstance));
        }
        else
        {
            m_package[tarPos].num = EItemDieJiaNum;
            m_package[srcPos].num = tolNum - EItemDieJiaNum;
        }
    }
    else
    {
        if((srcPos >= curMaxPackNum) || (tarPos >= curMaxPackNum))
            return;
        std::swap(m_package[srcPos],m_package[tarPos]);
    }
    
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PACK);
    MakePack(m_package[srcPos],srcPos,msg);
    sock.SendMsg(m_sock,msg);
    msg.ReWrite();
    msg.SetType(PRO_UPDATE_PACK);
    MakePack(m_package[tarPos],tarPos,msg);
    sock.SendMsg(m_sock,msg);
}

const char *CUser::GetCall(int &script)
{
    script = m_script;
    return m_scriptCall.c_str();
    /*boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_scriptHeap.size() <= 0)
    {
        script = 0;
        return "";
    }
    list<SScriptCall>::iterator i = m_scriptHeap.end();
    i--;
    script = i->scriptId;
    string &val = i->func;
    if(m_scriptHeap.size() > 1)
        m_scriptHeap.pop_front();
    return val.c_str();*/
}

void CUser::SetCall(int script,const char *call)
{
    m_script = script;
    if(call != NULL)
        m_scriptCall = call;
    /*********
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SScriptCall scriptCall;
    if(call == NULL)
        scriptCall.func.clear();
    else
        scriptCall.func = call;
    scriptCall.scriptId = script;
    m_scriptHeap.push_back(scriptCall);
    *************/
}

void CUser::SetCallScript(int script)
{
    m_script = script;
    /************88
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SScriptCall scriptCall;
    scriptCall.scriptId = 0;
    if(m_scriptHeap.size() <= 0)
    {
        scriptCall.scriptId = script;
        return;
    }
    list<SScriptCall>::iterator i = m_scriptHeap.end();
    i--;
    if(i->scriptId == 0)
    {
        i->scriptId = script;
    }
    else
    {
        m_scriptHeap.push_back(scriptCall);
    }
    **************/
}

void CUser::SetCallFun(const char *call)
{
    if(call != NULL)
        m_scriptCall = call;
    /*if(call == NULL)
        return;
        
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SScriptCall scriptCall;
    scriptCall.scriptId = 0;
    scriptCall.func = call;
        
    if(m_scriptHeap.size() <= 0)
    {
        m_scriptHeap.push_back(scriptCall);
        return;
    }
    list<SScriptCall>::iterator i = m_scriptHeap.end();
    i--;
    if(i->func.empty())
    {
        i->func = call;
    }
    else
    {
        m_scriptHeap.push_back(scriptCall);
    }*/
}

void CUser::BangDingItem(uint8 pos)
{
    if(pos >= MAX_PACKAGE_NUM)
        return;
    m_package[pos].bangDing = 1;
}

bool CUser::HaveBaobaoPet(int id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i < m_petNum; i++)
    {
        SPet *pPet = m_pet[i].get();
        if(pPet != NULL)
        {
            if((pPet->tmplId == (uint32)id) && (pPet->type == EMTBaoBao))
                return true;
        }
    }
    return false;
}

bool CUser::HavePet(int id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i < m_petNum; i++)
    {
        SPet *pPet = m_pet[i].get();
        if(pPet != NULL)
        {
            if(pPet->tmplId == (uint32)id)
                return true;
        }
    }
    return false;
}

void CUser::AddTongBao(int tongbao,uint8 type)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    
    if(type == 1)
    {
        char sqlBuf[128];
        sprintf(sqlBuf,"update user_info set bd_money=bd_money+%d where id = %d",tongbao,m_userId);
        if (pDb != NULL)
        {
            if(pDb->Query(sqlBuf))
            {
                m_bdTongBao += tongbao;
                SendUpdateInfo(46,m_bdTongBao);
            }
        }
    }
    else
    {
        char sqlBuf[128];
        sprintf(sqlBuf,"update user_info set money=money+%d where id = %d",tongbao,m_userId);
        if (pDb != NULL)
        {
            if(pDb->Query(sqlBuf))
            {
                m_tongBao += tongbao;
                SendUpdateInfo(27,m_tongBao);
            }
        }
    }
}

void CUser::AddMonsterScript(int monsterId,int scriptId)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    m_monsterScript.insert(pair<int, int>(monsterId, scriptId));
}

void CUser::DelMonsterScript(int monsterId,int scriptId)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    //m_monsterScript.erase(monsterId);
    typedef multimap<int,int>::iterator mapIter;
    pair<mapIter, mapIter> f;
    f = m_monsterScript.equal_range(monsterId);
    multimap<int,int>::iterator e = f.first;
    for(; e != f.second; e++)
    {
        if(e->second == scriptId)
        {
            m_monsterScript.erase(e);
            break;
        }
    }
}

int CUser::FindMonsterScript(int monsterId,list<int> *pList)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    map <int, int>::iterator f = m_monsterScript.begin();//find(monsterId);
    for(; f != m_monsterScript.end(); f++)
    {
        if(f->first == monsterId)
            pList->push_back(f->second);
    }
    //if(f != m_monsterScript.end())
        //return f->second;
    return 0;
}

void CUser::MakeSavePosInfo(CNetMessage &msg)
{
    msg<<MAX_SAVE_POS;
    for(uint8 i = 0; i < MAX_SAVE_POS; i++)
    {
        msg<<m_savePos[i].sceneId<<m_savePos[i].x<<m_savePos[i].y;
    }
}

int CUser::GetBangState()
{
    CBangPai *pBangPai = SingletonCBangPaiManager::instance().FindBangPai(m_bangpai);
    if(pBangPai == NULL)
        return 0;
    return pBangPai->GetDimisss();
}

int CUser::GetBangRank()
{
    CBangPai *pBangPai = SingletonCBangPaiManager::instance().FindBangPai(m_bangpai);
    if(pBangPai == NULL)
        return 0;
    return pBangPai->GetMemberRank(m_roleId);
}

void CUser::DismissBang()// 解散帮派
{
    CBangPai *pBangPai = SingletonCBangPaiManager::instance().FindBangPai(m_bangpai);
    if(pBangPai == NULL)
        return;
    pBangPai->DismissBang(m_roleId);
}

void CUser::UndismissBang()// 解除解散状态
{
    CBangPai *pBangPai = SingletonCBangPaiManager::instance().FindBangPai(m_bangpai);
    if(pBangPai == NULL)
        return;
    pBangPai->UndismissBang(m_roleId);
}

void CUser::UpdateBangPai()
{
    CBangPai *pBangPai = SingletonCBangPaiManager::instance().FindBangPai(m_bangpai);
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_MY_BANG);
    if(pBangPai == NULL)
    {
        msg<<0;
        sock.SendMsg(m_sock,msg);
        return;
    }
    uint8 rank = pBangPai->GetMemberRank(m_roleId);
    msg<<m_bangpai<<pBangPai->GetName()<<rank;
    sock.SendMsg(m_sock,msg);
}

int CUser::GetItemNum(int id)
{
    int num = 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 pos = 0; pos < MAX_PACKAGE_NUM; pos++)
    {
        if(m_package[pos].tmplId == id) 
        {
            num += m_package[pos].num;
        }
    }
    return num;
}

void CUser::SetFight(uint32 fightId,uint8 pos)
{
    m_fightId = fightId;
    m_fightPos = pos;
    if(m_fightId == 0)
    {
        m_fightEndTime = GetSysTime();
    }
}

bool CUser::CanMeetEnemy()
{
    return GetSysTime() - m_fightEndTime > 10;
}

void CUser::SaveSellItem(uint8 pos,uint8 num)
{
    if((pos >= MAX_PACKAGE_NUM) || (m_package[pos].tmplId == 0))
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    SaveUseItem(m_roleId,m_package[pos],"卖店",num);
}

void CUser::SaveDelItem(uint8 pos)
{
    if((pos >= MAX_PACKAGE_NUM) || (m_package[pos].tmplId == 0))
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    SaveUseItem(m_roleId,m_package[pos],"丢弃",1);
}

uint16 CUser::GetTiZhi()
{
    return m_tizhi + GetItemTizhi() + GetQiPetTiZhi();
}
uint16 CUser::GetLiLiang()
{
    return m_liliang + GetItemLiliang();
}
uint16 CUser::GetMinJie()
{
    return m_minjie + GetItemMinjie() + GetQiPetMinJie();
}
uint16 CUser::GetLingLi()
{
    return m_lingli + GetItemLingli();
}
uint16 CUser::GetNaiLi()
{
    //cout<<m_naili + GetItemNaili() + GetQiPetNaiLi()<<endl;
    return m_naili + GetItemNaili() + GetQiPetNaiLi();
}
uint16 CUser::GetJin()
{
    return m_jin + GetItemJin();
}
uint16 CUser::GetMu()
{
    return m_mu + GetItemMu();
}
uint16 CUser::GetShui()
{
    return m_shui + GetItemShui();
}
uint16 CUser::GetHuo()
{
    return m_huo + GetItemHuo();
}
uint16 CUser::GetTu()
{
    return m_tu + GetItemTu();
}

void CUser::MakeSaveShuXing(string &shuxing)
{
    HexToStr(m_tizhi,shuxing);
    HexToStr(m_liliang,shuxing);
    HexToStr(m_minjie,shuxing);
    HexToStr(m_lingli,shuxing);
    HexToStr(m_naili,shuxing);
    HexToStr(m_shuxingdian,shuxing);
    HexToStr(m_jin,shuxing);
    HexToStr(m_mu,shuxing);
    HexToStr(m_shui,shuxing);
    HexToStr(m_huo,shuxing);
    HexToStr(m_tu,shuxing);
    HexToStr(m_xiangxingdian,shuxing);
}

void CUser::MakeSaveEquip(string &equip)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i < EQUIPMENT_NUM; i++)
    {
        if(m_equipment[i].tmplId != 0)
            HexToStr(m_equipment[i],equip);
    }
}

void CUser::SetMenPai(uint8 menpai)
{
    m_menpai = menpai;
    SendUpdateInfo(39,m_menpai);
}

void CUser::ClearAnswer()
{
    m_answerTime = GetSysTime();
    m_answerTimeSpace = 0;
    m_answerTimes = 0;
}

uint16 CUser::GetAnswerSpace()
{
    //cout<<m_answerTimeSpace + (GetSysTime() - m_answerTime)<<endl;
    return m_answerTimeSpace + (GetSysTime() - m_answerTime);
}

void CUser::SetName(const char *name)
{
    if(name != NULL)
    {
        snprintf(m_name,MAX_NAME_LEN-1,"%s",name);
    }
}

void CUser::SetMoveTime(uint64 t)
{
    m_moveTime = t;
}

uint64 CUser::GetMoveTime()
{
    return m_moveTime;
}

void CUser::SetErrMoveTimes(uint8 t)
{
    m_moveErrTimes = t;
}

uint8 CUser::GetMoveErrTimes()
{
    return m_moveErrTimes;
}

void CUser::SetTili(int tili)
{//最大值为 50+人物等级*2
    int max = 50+m_level*2;
    if(tili > max)
        m_tili = max;
    else
        m_tili = tili;
    SendUpdateInfo(42,m_tili);
}

void CUser::AddTili(int add)
{
    int max = 50+m_level*2;
    m_tili += add;
    if(m_tili > max)
        m_tili = max;
    SendUpdateInfo(42,m_tili);
}

bool CUser::CanFightHuoDong()
{
    if(GetSysTime() - m_huodongTime > 4*60)
    {
        return true;
    }
    return false;
}

int CUser::GetLeftDoubleTime()
{
    if(m_sDoubleEnd <= GetSysTime())
        return 0;
    return m_sDoubleEnd - GetSysTime();
}

void CUser::SetDouble(int hour)
{
    m_sDoubleEnd = GetSysTime()+3600*hour;
}

void CUser::ReadSaveData(char *row)
{
    memset(m_save8,0,sizeof(m_save8));
    memset(m_save16,0,sizeof(m_save16));
    memset(m_save32,0,sizeof(m_save32));
    if(row == NULL)
        return;
    uint8 num = 0;
    CNetMessage msg;
    int len = strlen(row);
    uint8 hex[len/2];
    StrToHex(row,hex,len);
    msg.WriteData(hex,len);
    msg>>num;
    if(num <= UINT8_NUM)
    {
        for(uint8 i = 0; i < num; i++)
        {
            msg>>m_save8[i];
        }
    }
    msg>>num;
    if(num <= UINT16_NUM)
    {
        for(uint8 i = 0; i < num; i++)
        {
            msg>>m_save16[i];
        }
    }
    msg>>num;
    if(num <= UINT32_NUM)
    {
        for(uint8 i = 0; i < num; i++)
        {
            msg>>m_save32[i];
        }
    }
}

void CUser::WriteSaveData(string &str)
{
    CNetMessage msg;
    
    msg<<UINT8_NUM;
    for(uint8 i = 0; i < UINT8_NUM; i++)
    {
        msg<<m_save8[i];
    }
    msg<<UINT16_NUM;
    for(uint8 i = 0; i < UINT16_NUM; i++)
    {
        msg<<m_save16[i];
    }
    msg<<UINT32_NUM;
    for(uint8 i = 0; i < UINT32_NUM; i++)
    {
        msg<<m_save32[i];
    }
    str.clear();
    HexToStr((uint8*)(msg.GetMsgData()->c_str() + 4),msg.GetDataLen() - 4,str);
}

int CUser::ChaiFenSX(uint8 weaponPos,uint8 attrPos)
{
    if((weaponPos >= MAX_PACKAGE_NUM) 
        || (attrPos >= SItemInstance::MAX_KAIJIA_ATTR_NUM))
    {
        return 1;
    }
    
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pWeaItem = itemMgr.GetItem(m_package[weaponPos].tmplId);
    if(pWeaItem == NULL)
    {
        return 3;
    }
    
    uint8 i = 0;
    if(pWeaItem->type == EITPetKaiJia)
    {
        uint16 itemId = 0;
        if(attrPos < 3)
        {
            itemId = 633;
        }
        else if(attrPos < 6)
        {
            itemId = 634;
        }
        else if(attrPos < SItemInstance::MAX_KAIJIA_ATTR_NUM)
        {
            itemId = 635;
        }
        for(; i < MAX_PACKAGE_NUM; i++)
        {
            if(m_package[i].tmplId == 0)
                continue;
            if((m_package[i].tmplId == itemId)
                && (m_package[i].addAttrNum == 0))
            {
                break;
            }
        }
    }
    else
    {
        for(; i < MAX_PACKAGE_NUM; i++)
        {
            if(m_package[i].tmplId == 0)
                continue;
            SItemTemplate *pStoItem = itemMgr.GetItem(m_package[i].tmplId);
            if(pStoItem != NULL)
            {
                if((pStoItem->type == EITAddAttr)
                    && ((pStoItem->addXue & (1<<(pWeaItem->type-1))) != 0)
                    && (m_package[i].addAttrNum == 0))
                {
                    break;
                }
            }
        }
    }
    if(i >= MAX_PACKAGE_NUM)
        return 1;
    return ChaiFen(weaponPos,attrPos,i);
}

//拆分,0成功，1 拆分石不符 2 已拆分过的石头 3 物品不符 4 包裹满
int CUser::ChaiFen(uint8 weaponPos,uint8 attrPos,uint8 stonePos)
{
    if((weaponPos >= MAX_PACKAGE_NUM) 
        || (attrPos >= SItemInstance::MAX_KAIJIA_ATTR_NUM)
        || (stonePos >= MAX_PACKAGE_NUM))
    {
        return 1;
    }
    SItemInstance *pWeapon = m_package + weaponPos;
    SItemInstance *pStone = m_package + stonePos;
    if(pWeapon->tmplId == 0)
    {
        return 3;
    }
    if(pStone->tmplId == 0)
    {
        return 1;
    }
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pWeaItem = itemMgr.GetItem(pWeapon->tmplId);
    if(pWeaItem == NULL)
    {
        return 3;
    }
    SItemTemplate *pStoItem = itemMgr.GetItem(pStone->tmplId);
    if(pStoItem == NULL)
    {
        return 1;
    }
    if(pStone->addAttrNum != 0)
    {
        return 2;
    }
    if((pStoItem->type != EITAddAttr) && (pStoItem->type != EITPetKaiJia))
    {
        return 1;
    }
    if(pStoItem->type != EITAddAttr)
    {
        if((pStoItem->addXue & (1<<(pWeaItem->type-1))) == 0)
        {
            return 1;
        }
    }
    else if(pStoItem->type == EITPetKaiJia)
    {
        uint16 itemId = 0;
        if(attrPos < 3)
        {
            itemId = 633;
        }
        else if(attrPos < 6)
        {
            itemId = 634;
        }
        else if(attrPos < SItemInstance::MAX_KAIJIA_ATTR_NUM)
        {
            itemId = 635;
        }
        if(m_package[stonePos].tmplId != itemId)
            return 1;
    }
    if(pWeapon->addAttrType[attrPos] == 0)
    {
        return 3;
    }
    if(pStone->num == 1)
    {
        pStone->addAttrNum = 1;
        pStone->addAttrType[0] = pWeapon->addAttrType[attrPos];
        pStone->addAttrVal[0] = pWeapon->addAttrVal[attrPos];
        CSocketServer &sock = SingletonSocket::instance();
        CNetMessage msg;
        msg.SetType(PRO_UPDATE_PACK);
        pStone->bangDing = 1;
        MakePack(*pStone,stonePos,msg);
        sock.SendMsg(m_sock,msg);
    }
    else
    {
        SItemInstance item = *pStone;
        item.num = 1;
        item.addAttrNum = 1;
        item.addAttrType[0] = pWeapon->addAttrType[attrPos];
        item.addAttrVal[0] = pWeapon->addAttrVal[attrPos];
        item.bangDing = 1;
        if(!NoLockAddPackage(item))
            return 4;
        NoLockDelPackage(stonePos,1);
    }
    
    NoLockDelPackage(weaponPos,1);
    return 0;
}

//融合，0成功，1 石头不符 2 石头未拆分过 3 物品不符
int CUser::RongHe(uint8 weaponPos,uint8 stonePos)
{
    if((weaponPos >= MAX_PACKAGE_NUM) 
        || (stonePos >= MAX_PACKAGE_NUM))
    {
        return 1;
    }
    SItemInstance *pWeapon = m_package + weaponPos;
    SItemInstance *pStone = m_package + stonePos;
    if(pWeapon->tmplId == 0)
    {
        return 3;
    }
    if(pStone->tmplId == 0)
    {
        return 1;
    }
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pWeaItem = itemMgr.GetItem(pWeapon->tmplId);
    if(pWeaItem == NULL)
    {
        return 3;
    }
    SItemTemplate *pStoItem = itemMgr.GetItem(pStone->tmplId);
    if(pStoItem == NULL)
    {
        return 1;
    }
    if(pStone->addAttrNum != 1)
    {
        return 2;
    }
    if(pStoItem->type != EITAddAttr)
    {
        return 1;
    }
    if((pStoItem->addXue & (1<<(pWeaItem->type-1))) == 0)
    {
        return 1;
    }
    uint8 attrPos = 0xff;
    for(uint8 i = 0; i < SItemInstance::MAX_ADD_ATTR_NUM; i++)
    {
        if(pWeapon->addAttrType[i] == pStone->addAttrType[0])
        {
            attrPos = i;
            break;
        }
    }
    if(attrPos >= SItemInstance::MAX_ADD_ATTR_NUM)
        return 1; 
    
    string before;
    HexToStr(m_package[weaponPos],before);
    
    pWeapon->addAttrVal[attrPos] = pStone->addAttrVal[0];
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PACK);
    pWeapon->bangDing = 1;
    MakePack(*pWeapon,weaponPos,msg);
    sock.SendMsg(m_sock,msg);
    
    string end;
    HexToStr(m_package[weaponPos],end);
    SaveUseItem(m_roleId,m_package[stonePos],"合成蓝水晶",
                1,before,end);
    NoLockDelPackage(stonePos,1);
    return 0;
}

void CUser::ClearTimeoutTitle()
{
    for(uint8 i = 0; i < MAX_TITLE_NUM; i++)
    {
        if((m_titleList[i].id != 0) && (m_titleList[i].id != EUTShiFu) && (GetSysTime() - m_titleList[i].begin > 7*24*3600))
        {
            if(m_titleList[i].id == m_title)
                m_title = 0;
            m_titleList[i].id = 0;
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(MSG_CLIENT_USE_TITLE);
            msg<<(uint8)1<<(uint16)m_titleList[i].id;
            sock.SendMsg(m_sock,msg);
        }
    }
}

void CUser::SetTitle(uint16 title)
{
    if(title == 0)
    {
        m_title = 0;
        return;
    }
    for(uint8 i = 0; i < MAX_TITLE_NUM; i++)
    {
        if(m_titleList[i].id == title)
        {
            m_title = title;
            break;
        }
    }
}

void CUser::AddTitle(uint16 title)
{
    for(uint8 i = 0; i < MAX_TITLE_NUM; i++)
    {
        if(m_titleList[i].id == title)
        {
            m_titleList[i].id = title;
            m_titleList[i].begin = GetSysTime();
            return;
        }
    }
    for(uint8 i = 0; i < MAX_TITLE_NUM; i++)
    {
        if(m_titleList[i].id == 0)
        {
            m_titleList[i].id = title;
            m_titleList[i].begin = GetSysTime();
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(MSG_CLIENT_USE_TITLE);
            msg<<(uint8)0<<(uint16)title;
            sock.SendMsg(m_sock,msg);
            break;
        }
    }
}

void CUser::ReadTitle(char *row)
{
    memset(m_titleList,0,sizeof(m_titleList));
    m_title = 0;
    if(strlen(row) <= 10)
        return;
    uint32 len = sizeof(m_titleList);
    UnCompress(row,(uint8*)m_titleList,len);
    m_title = m_titleList[MAX_TITLE_NUM].id;
}

void CUser::GetTitleStr(string &str)
{
    m_titleList[MAX_TITLE_NUM].id = m_title;
    Compress((uint8*)m_titleList,sizeof(m_titleList),str);
}

void CUser::GetTitleMsg(CNetMessage &msg)
{
    uint16 pos = msg.GetDataLen();
    msg<<(uint8)0;
    uint8 num = 0;
    for(uint8 i = 0; i < MAX_TITLE_NUM; i++)
    {
        if(m_titleList[i].id != 0)
        {
            num++;
            msg<<m_titleList[i].id;
            if(m_title == m_titleList[i].id)
                msg<<(uint8)1;
            else
                msg<<(uint8)0;
        }
    }
    msg.WriteData(pos,&num,1);
}

void CUser::SetShiFu()
{
    AddTitle(EUTShiFu);
}

int CUser::GetHeChengVal(uint8 pos1,uint8 pos2)
{
    if((pos1 >= MAX_PACKAGE_NUM) || (pos2 >= MAX_PACKAGE_NUM))
        return 0;
    if((m_package[pos1].tmplId != 615) || (m_package[pos2].tmplId != 615))
        return 0;
    if((pos1 == pos2) && (m_package[pos1].num <= 1))
        return 0;
        
    if(m_package[pos1].addAttrType[0] != m_package[pos2].addAttrType[0])
    {
        return 0;
    }
    //2颗属性值总和/1.7，取整。当新生成值小于2颗原料水晶中的最大值时，取原料水晶中的最大值。
    //当2颗蓝水晶总和小于11时，实际生成值+1。最大值为此项的极值。
    uint16 tol = m_package[pos1].addAttrVal[0] + m_package[pos2].addAttrVal[0];
    uint16 val = (int)(tol/1.7);
    if(tol < 11)
        val++;
    uint16 maxVal = std::max(m_package[pos1].addAttrVal[0],m_package[pos2].addAttrVal[0]);
    if(val < maxVal)
        val = maxVal;
    uint16 min = 0;
    uint16 middle = 0;
    uint16 max = 0;
    GetAddAttrVal(m_package[pos1].addAttrType[0],min,middle,max);
    if(val > max)
        val = max;
    return val;
}

int CUser::HeChengKaiJia(int target ,uint8 kaijia)
{
    const uint16 minKaiJIaId = 800;
    const uint16 maxKaiJiaId = 810;
    if((target < minKaiJIaId) || (target > maxKaiJiaId))
        return 1;
        
    uint8 level = target - minKaiJIaId + 1;
    int bangDing = 0;
    if(kaijia < MAX_PACKAGE_NUM)
    {
        if(m_package[kaijia].bangDing == 1)
            bangDing = 1;
        if(m_package[kaijia].tmplId + 1 != target)
            return 1;
    }
    else if(target != minKaiJIaId)
    {
        return 1;
    }
    
    uint16 caiLiao[3] = {640,641,642};
    int caiLiaoNum[3] = {0};
    for(uint8 i = 0; i < 3; i++)
    {
        for(uint8 j = 0; j < MAX_PACKAGE_NUM; j++)
        {
            if((m_package[j].tmplId == caiLiao[i]) 
                && (m_package[j].level == level))
            {
                caiLiaoNum[i] += m_package[j].num;
                if(m_package[j].bangDing == 1)
                    bangDing = 1;
            }
        }
        if(caiLiaoNum[i] < 3)
            return 1;
    }
    
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(target);
    if(pItem == NULL)
        return 1;
    
    SItemInstance item = {0};
    if(kaijia < MAX_PACKAGE_NUM)
    {
        item = m_package[kaijia];
    }
    item.tmplId = target;
    item.num = 1;
    item.naijiu = pItem->naijiu;
    item.level = 0;
    
    item.bangDing = bangDing;
        
    if(!AddPackage(item))
    {
        return 1;
    }
    for(uint8 i = 0; i < 3; i++)
    {
        uint8 num = 3;
        for(uint8 j = 0; j < MAX_PACKAGE_NUM; j++)
        {
            if((m_package[j].tmplId == caiLiao[i]) 
                && (m_package[j].level == level))
            {
                if(m_package[j].num >= num)
                {
                    NoLockDelPackage(j,num);
                    break;
                }
                else
                {
                    NoLockDelPackage(j,m_package[j].num);
                    num -= m_package[j].num;
                }
            }
        }
    }
    if(kaijia < MAX_PACKAGE_NUM)
    {
        NoLockDelPackage(kaijia);
    }
    return 0;
}

bool CUser::HeChengLanShuiJing(uint8 pos1,uint8 pos2,uint8 hechengfu)
{
    if(hechengfu >= MAX_PACKAGE_NUM)
        return false;
    if(m_package[hechengfu].tmplId != 616)
        return false;
          
    uint16 val = GetHeChengVal(pos1,pos2);
    if(val == 0)
        return false;

    SItemInstance item = {0};
    if((m_package[pos1].bangDing == 1)
        || (m_package[pos2].bangDing == 1)
        || (m_package[hechengfu].bangDing == 1))
    {
        item.bangDing = 1;
    }
    item.num = 1;
    item.tmplId = 615;
    item.addAttrType[0] = m_package[pos1].addAttrType[0];              
    item.addAttrVal[0] = val; 
    item.addAttrNum = 1;  
    if(NoLockAddPackage(item))
    {
        string before;
        HexToStr(m_package[pos1],before);
        string end;
        HexToStr(m_package[pos2],end);
        SaveUseItem(m_roleId,item,"合成蓝水晶",
                1,before,end);
        NoLockDelPackage(pos1);
        NoLockDelPackage(pos2);
        NoLockDelPackage(hechengfu);
        //SaveUseItem(m_roleId,
        return true;
    }
    return false;
}

bool CUser::KaiJiaXiangQian(uint8 kaijiaPos,uint8 kongPos,uint8 stonePos,string &msg)
{
    if((kaijiaPos >= MAX_PACKAGE_NUM)
        || (kongPos >= SItemInstance::MAX_KAIJIA_ATTR_NUM)
        || (stonePos >= MAX_PACKAGE_NUM))
    {
        msg = "材料不符，镶嵌失败。";
        return false;
    }
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(m_package[kaijiaPos].tmplId);
    if((pItem == NULL) || (pItem->type != EITPetKaiJia))
    {
        msg = "材料不符，镶嵌失败。";
        return false;
    }
    if(m_package[kaijiaPos].addAttrType[kongPos] != 0)
    {
        msg = "材料不符，镶嵌失败。";
        return false;
    }
    
    if(m_package[stonePos].addAttrType[0] == 0)
    {
         msg = "只能镶嵌带属性的宝石";
        return false;
    }
    if(kongPos < 3)
    {
        if((m_package[stonePos].tmplId == 633) 
            && (m_package[stonePos].addAttrType[0] != 0))
        {
            if(!m_package[kaijiaPos].AddKong(kongPos,
                m_package[stonePos].addAttrType[0],
                m_package[stonePos].addAttrVal[0]))
            {
                msg = "材料不符，镶嵌失败。";
                return false;
            }
            NoLockDelPackage(stonePos);
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[kaijiaPos],kaijiaPos,msg);
            sock.SendMsg(m_sock,msg);
            return true;
        }
    }
    else if(kongPos < 6)
    {
        if((m_package[stonePos].tmplId == 634) && (m_package[stonePos].addAttrType[0] != 0))
        {
            if(!m_package[kaijiaPos].AddKong(kongPos,
                m_package[stonePos].addAttrType[0],
                m_package[stonePos].addAttrVal[0]))
            {
                msg = "材料不符，镶嵌失败。";    
                return false;
            }
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[kaijiaPos],kaijiaPos,msg);
            NoLockDelPackage(stonePos);
            sock.SendMsg(m_sock,msg);
            return true;
        }
    }
    else if(kongPos < SItemInstance::MAX_KAIJIA_ATTR_NUM)
    {
        if((m_package[stonePos].tmplId == 635) && (m_package[stonePos].addAttrType[0] != 0))
        {
            if(!m_package[kaijiaPos].AddKong(kongPos,
                m_package[stonePos].addAttrType[0],
                m_package[stonePos].addAttrVal[0]))
            {
                msg = "材料不符，镶嵌失败。";    
                return false;
            }
            NoLockDelPackage(stonePos);
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[kaijiaPos],kaijiaPos,msg);
            sock.SendMsg(m_sock,msg);
            return true;
        }
    }
    msg = "材料不符，镶嵌失败。";    
    return false;
}

//宠物学习技能
bool CUser::PetStudySkill(uint8 petPos,uint16 skillId)
{
    CNetMessage msg;
    msg.SetType(MSG_SERVER_USE_RESULT);
    msg<<(uint8)10;//<<PRO_ERROR;
    CSocketServer &sock = SingletonSocket::instance();
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    if((petPos >= m_petNum) || (m_pet[petPos].get() == NULL))
    {
        msg<<PRO_ERROR<<"无此宠物";
        sock.SendMsg(m_sock,msg);
        return false;
    }
    SPet *pPet = m_pet[petPos].get();
    uint8 level = pPet->GetSkillLevel(skillId);
    if(level == 0)
    {
        msg<<PRO_ERROR<<"无此技能";
        sock.SendMsg(m_sock,msg);
        return false;
    }
    if(level >= 100)
    {
        msg<<PRO_ERROR<<"技能已达上线";
        sock.SendMsg(m_sock,msg);
        return false;
    }
    
    const int LEARNSKILL_XIUWEI_COST[] = {4000,6100,10000,14000,19000,23000,28000,33000,38000,43000,48000,54000,60000,66000,72000,78000,85000,92000,99000,110000,110000,120000,130000,140000,150000,160000,170000,180000,190000,200000,210000,230000,240000,250000,270000,280000,300000,310000,330000,350000,360000,380000,400000,420000,440000,460000,480000,510000,530000,560000,580000,610000,640000,660000,690000,720000,750000,790000,820000,860000,890000,930000,970000,1000000,1000000,1100000,1100000,1200000,1200000,1300000,1300000,1400000,1400000,1500000,1500000,1600000,1700000,1700000,1800000,1800000,1900000,2000000,2100000,2100000,2200000,2300000,2400000,2400000,2500000,2600000,2700000,2800000,2900000,3000000,3100000,3200000,3300000,3400000,3500000,3600000};
    const int LEARNSKILL_DAOHANG_COST[] = {400,600,1000,1400,1900,2300,2800,3300,3800,4300,4800,5400,6000,6600,7200,7800,8500,9200,9900,11000,11000,12000,13000,14000,15000,16000,17000,18000,19000,20000,21000,23000,24000,25000,27000,28000,30000,31000,33000,35000,36000,38000,40000,42000,44000,46000,48000,51000,53000,56000,58000,61000,64000,66000,69000,72000,75000,79000,82000,86000,89000,93000,97000,100000,100000,110000,110000,120000,120000,130000,130000,140000,140000,150000,150000,160000,170000,170000,180000,180000,190000,200000,210000,210000,220000,230000,240000,240000,250000,260000,270000,280000,290000,300000,310000,320000,330000,340000,350000,360000};
    const int LEARNSKILL_MONEY_COST[] = {3000,5500,9200,13000,17000,21000,25000,30000,34000,39000,44000,49000,54000,59000,65000,70000,77000,83000,89000,96000,100000,110000,120000,130000,130000,140000,150000,160000,170000,180000,190000,200000,220000,230000,240000,250000,270000,280000,300000,310000,330000,340000,360000,380000,400000,420000,440000,460000,480000,500000,520000,550000,570000,600000,620000,650000,680000,710000,740000,770000,800000,840000,870000,910000,940000,980000,1000000,1100000,1100000,1100000,1200000,1200000,1300000,1300000,1400000,1400000,1500000,1500000,1600000,1700000,1700000,1800000,1800000,1900000,2000000,2100000,2100000,2200000,2300000,2400000,2400000,2500000,2600000,2700000,2800000,2900000,3000000,3100000,3200000,3300000};
    
    int money = LEARNSKILL_MONEY_COST[level-1];
    int xiuwei = LEARNSKILL_XIUWEI_COST[level-1];
    int daohang = LEARNSKILL_DAOHANG_COST[level-1];
    
    if(m_money < money)
    {
        msg<<PRO_ERROR<<"游戏币不够";
        sock.SendMsg(m_sock,msg);
        return false;
    }
    if(pPet->xiuWei < xiuwei)
    {
        msg<<PRO_ERROR<<"宠物修为不够";
        sock.SendMsg(m_sock,msg);
        return false;
    }
    if(m_daohang < daohang)
    {
        msg<<PRO_ERROR<<"道行不够";
        sock.SendMsg(m_sock,msg);
        return false;
    }
    pPet->xiuWei -= xiuwei;
    UpdatePetInfo(petPos,23,pPet->xiuWei);
    
    m_daohang -= daohang;
    SendUpdateInfo(6,m_qianneng);
    SendUpdateInfo(7,m_daohang);
    AddMoney(-money);
    
    int itemSkills[] = {540,160,541,161,542,162,543,163,544,164,545,165,546,166,547,167,548,168,549,169,550,170,551,171,552,172,553,173,554,174,555,175,556,176,557,177,558,178,559,179,560,180,561,181,562,182,563,183,564,184,565,185,566,51,567,55,568,59,569,63,570,67,571,151,572,152,573,153,574,154,575,158,576,101,577,105,578,109,579,113,580,117,581,155,582,156,583,157,584,3,585,7,586,11,587,15,588,19};
    int itemId = 0;
    uint32 i;
    for(i = 0; i < sizeof(itemSkills)/sizeof(itemSkills[0]); i += 2)
    {
        if(skillId == itemSkills[i+1])
        {
            itemId = itemSkills[i];
            break;
        }
    }
    
    bool haveBook = false;
    for(uint8 pos = 0; pos < MAX_PACKAGE_NUM; pos++)
    {
        if(m_package[pos].tmplId == itemId)
        {
            if(m_package[pos].num > 1)
            {
                m_package[pos].num--;
            }
            else
            {
                memset(m_package + pos,0,sizeof(SItemInstance));
            }
            msg.ReWrite();
            msg.SetType(PRO_UPDATE_PACK);
            MakePack(m_package[pos],pos,msg);
            sock.SendMsg(m_sock,msg);
            haveBook = true;
            break;
        }
    }
    if(!haveBook)
    {
        msg<<PRO_ERROR<<"没有技能书";
        sock.SendMsg(m_sock,msg);
        return false;
    }

    /*const uint8 gaiLv[] = {100,99,98,97,96,95,94,93,92,91,89,87,85,83,81,79,77,75,73,71,69,67,65,63,61,59,57,55,53,51,51,50,50,50,50,49,49,49,48,48,48,47,47,47,47,46,46,46,45,45,45,44,44,44,43,43,43,43,42,42,42,41,41,41,40,40,40,40,39,39,39,38,38,38,37,37,37,37,36,36,36,35,35,35,34,34,34,34,33,33,33,32,32,32,31,31,31,31,30,30};
    if(Random(0,100) > gaiLv[level-1])
    {
        msg<<PRO_ERROR<<"学习失败";
        sock.SendMsg(m_sock,msg);
        return false;
    }*/
        
    pPet->AddSkill(skillId);
    
    pPet->Init();
    msg.ReWrite();
    msg.SetType(PRO_UPDATE_PET);
    msg<<(uint8)2<<petPos;
    NoLockMakePetInfo(petPos,msg);
    sock.SendMsg(m_sock,msg);
    
    msg.ReWrite();
    msg.SetType(MSG_SERVER_USE_RESULT);
    msg<<(uint8)10;//<<PRO_ERROR;
    msg<<PRO_SUCCESS;
    sock.SendMsg(m_sock,msg);
    return true;
}

void CUser::DesKaiJiaNaijiu(int val)
{
    SharePetPtr pet;
    SPet *pPet = NULL;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        if(m_chuZhanPet >= m_petNum)
            return;
        pet = m_pet[m_chuZhanPet];
        pPet = pet.get();
        if(pPet == NULL)
            return;
    }
    if(pPet->kaiJia.tmplId == 0)
        return;
        
    if(pPet->kaiJia.naijiu > val)
    {
        pPet->kaiJia.naijiu -= val;
        UpdatePetInfo(m_chuZhanPet,22,pPet->kaiJia.naijiu);
    }
    else
    {
        if(pPet->kaiJia.naijiu != 0)
        {
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(pPet->kaiJia.tmplId);
            if(pItem == NULL)
                return;
            pPet->Init();
            UpdatePetInfo(m_chuZhanPet,20,pPet->GetRecovery());
            UpdatePetInfo(m_chuZhanPet,19,pPet->speed);
            UpdatePetInfo(m_chuZhanPet,22,0);
        }
        pPet->kaiJia.naijiu = 0;
    }
}

//装备宠物铠甲
bool CUser::PetKaiJia(uint8 petPos,uint8 kaiJiaPos)
{
    if(kaiJiaPos >= MAX_PACKAGE_NUM)
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        if((petPos >= m_petNum) || (m_pet[petPos].get() == NULL)
            || (m_pet[petPos]->kaiJia.tmplId == 0) || (m_pet[petPos]->kaiJia.name[0] == SItemInstance::CAN_NOT_XIE_XIA))
            return false;
        if(NoLockAddPackage(m_pet[petPos]->kaiJia))
        {
            CItemTemplateManager &itemMgr = SingletonItemManager::instance();
            SItemTemplate *pItem = itemMgr.GetItem(m_pet[petPos]->kaiJia.tmplId);
            if(pItem == NULL)
                return false;
            m_pet[petPos]->kaiJia.tmplId = 0;
            
            m_pet[petPos]->Init();
            CSocketServer &sock = SingletonSocket::instance();
            CNetMessage msg;
            msg.SetType(PRO_UPDATE_PET);
            msg<<(uint8)2<<petPos;
            NoLockMakePetInfo(petPos,msg);
            sock.SendMsg(m_sock,msg);
            
            return true;
        }
        return false;
    }
    
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    SItemTemplate *pItem = itemMgr.GetItem(m_package[kaiJiaPos].tmplId);
    if(pItem == NULL)
        return false;
    if(pItem->type != EITPetKaiJia)
        return false;
        
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    if((petPos >= m_petNum) || (m_pet[petPos].get() == NULL))
        return false;
    if(m_pet[petPos]->level < pItem->level)
        return false;
    if(m_pet[petPos]->kaiJia.tmplId != 0)
        return false;
        
    m_pet[petPos]->kaiJia = m_package[kaiJiaPos];
    NoLockDelPackage(kaiJiaPos);
    
    m_pet[petPos]->Init();
    
    CSocketServer &sock = SingletonSocket::instance();
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PET);
    msg<<(uint8)2<<petPos;
    NoLockMakePetInfo(petPos,msg);
    sock.SendMsg(m_sock,msg);
    return true;
}

int CUser::GetPetById(int id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    for(uint8 i = 0; i < m_petNum; i++)
    {
        SPet *pPet = m_pet[i].get();
        if(pPet != NULL)
        {
            if(pPet->tmplId == (uint16)id)
                return i;
        }
    }
    return -1;
}

SItemInstance *CUser::GetItemById(int id)
{
    for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if(m_package[i].tmplId == id)
            return m_package+i;
    }
    return NULL;
}
void CUser::AddPetQinMi(uint8 pos,int qinmi)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(pos >= m_petNum)
        return;
    SPet *pPet = m_pet[pos].get();
    if(pPet != NULL)
    {
        uint8 oldSpeed = GetPetSpeed(pPet->qinmi);
        pPet->qinmi += qinmi;
        UpdatePetInfo(pos,14,pPet->qinmi);
        uint8 speed = GetPetSpeed(pPet->qinmi);
        if(speed != oldSpeed)
            UpdatePetInfo(pos,21,speed);
    }
}

bool CUser::HaveNameItem(uint16 itemId,const char *name)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(int i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if((m_package[i].tmplId == itemId) && (strcmp(m_package[i].name,name) == 0))
        {
            NoLockDelPackage(i);
            return true;
        }
    }
    return false;
}

bool CUser::AddIgnore(uint32 roleId)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_ignoreList.size() >= MAX_HOT_NUM)
        return false;
    for(list<HotInfo>::iterator i = m_ignoreList.begin(); i != m_ignoreList.end(); i++)
    {
        if(i->hotId == roleId)
        {
            return false;
        }
    }
    HotInfo h = {roleId,0};
    m_ignoreList.push_back(h);
    return true;
}

void CUser::DelIgnore(uint32 roleId)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(list<HotInfo>::iterator i = m_ignoreList.begin(); i != m_ignoreList.end(); i++)
    {
        if(i->hotId == roleId)
        {
            m_ignoreList.erase(i);
            return;
        }
    }
}

void CUser::GetIgnoreList(list<HotInfo> &ignoreList)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    ignoreList = m_ignoreList;
}

bool CUser::HaveIgnore(uint32 roleId)
{
    ReadIgnore();
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(list<HotInfo>::iterator i = m_ignoreList.begin(); i != m_ignoreList.end(); i++)
    {
        if(i->hotId == roleId)
        {
            return true;
        }
    }
    return false;
}

void CUser::ReadIgnore()
{
    if(m_readIgnore)
        return;
    m_readIgnore = true;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    char sql[128];
    snprintf(sql,128,"select ignore_id,val from role_ignore where role_id=%u",m_roleId);
    if(!pDb->Query(sql))    
        return;
        
    char **row;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    while((row = pDb->GetRow()) != NULL)
    {
        HotInfo h = {atoi(row[0]),atoi(row[1])};
        m_ignoreList.push_back(h);
    }
}

int CUser::GetPkMiss(uint32 roleId)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(int i = 0; i < MAX_MISSION_NUM; i++)
    {
        if(m_mission[i].id >= 10000)
        {
            char buf[64];
            strcpy(buf,m_mission[i].mission.c_str());
            char *p[2];
            if((SplitLine(p,2,buf) == 2) && (atoi(p[1]) == (int)roleId))
                return m_mission[i].id;
        }
    }
    return 0;
}

void CUser::XiShouPet(uint8 ind)
{
    if(ind >= m_petNum)
        return;
        
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if(m_package[i].tmplId == 1006) 
        {
            SPet *pPet = m_pet[ind].get();
            if(pPet == NULL)
                return;
            m_package[i].naijiu++;
            m_package[i].addAttrVal[0] += pPet->hpCZ;//气血成长                 
            m_package[i].addAttrVal[1] += pPet->mpCZ;//法术成长             
            m_package[i].addAttrVal[2] += pPet->speedCZ;//速度成长          
            m_package[i].addAttrVal[3] += pPet->attackCZ;//物攻成长            
            m_package[i].addAttrVal[4] += pPet->skillAttackCZ;//技能功能成长
            UpdatePackage(i);
            //NoLockDelPet(ind);
            return;
        }
    }
}

bool CUser::AddQiLing()
{
    uint8 i = 0;
    int hp = 0;
    int mp = 0;
    int speed = 0;
    int attack = 0;
    int skillattack = 0;
    int naijiu = 0;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(; i < MAX_PACKAGE_NUM; i++)
    {
        if(m_package[i].tmplId == 1006) 
        {
            naijiu = m_package[i].naijiu;
            hp = m_package[i].addAttrVal[0];//气血成长                 
            mp = m_package[i].addAttrVal[1];//法术成长             
            speed = m_package[i].addAttrVal[2];//速度成长          
            attack = m_package[i].addAttrVal[3];//物攻成长            
            skillattack  = m_package[i].addAttrVal[4];//技能功能成长
            break;
        }
    }
    if(naijiu == 0)
        return false;
        
    SPet *pPet = new SPet;
    memset(pPet,0,sizeof(SPet));
    pPet->tmplId = 102;
    pPet->level = 1;
    sprintf(pPet->name,"麒麟宝宝");
    
    if(naijiu - GetData8(7) > 0)
        naijiu = naijiu - GetData8(7);
        
    pPet->hpCZ = hp/naijiu;//气血成长                  
    pPet->mpCZ = mp/naijiu;//法术成长                
    pPet->speedCZ = speed/naijiu;//速度成长              
    pPet->attackCZ = attack/naijiu;//物攻成长           
    pPet->skillAttackCZ = skillattack/naijiu;//技能功能成长     
    
    pPet->Init();
    
    SharePetPtr ptr(pPet);
    if(NoLockAddPet(ptr))
    {
        NoLockDelPackage(i);
        return true;
    }
    return false;
}

void CUser::UseFightEnd()
{//683,684
    int petHp = 0;
    int petMaxHp = 0;
    int petMp = 0;
    int petMaxMp = 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i < MAX_PACKAGE_NUM; i++)
    {
        if(m_package[i].tmplId == 683)
        {
            int *pLeft = (int*)m_package[i].addAttrVal;
            if(m_chuZhanPet < m_petNum)
            {
                SPet *pPet = m_pet[m_chuZhanPet].get();
                if(pPet != NULL)
                {
                    if(pPet->hp < pPet->maxHp)
                    {
                        int add = min(pPet->maxHp - pPet->hp,*pLeft);
                        *pLeft -= add;
                        pPet->hp += add;
                        UpdatePetInfo(m_chuZhanPet,5,pPet->hp);
                    }
                    petHp = pPet->hp;
                    petMaxHp = pPet->maxHp;
                }
            }
            if(m_hp < m_maxHp)
            {
                int add = min(m_maxHp - m_hp,*pLeft);
                *pLeft -= add;
                AddHp(add);
            }
            if(*pLeft <= 0)
            {
                NoLockDelPackage(i);
            }
            else
            {
                CSocketServer &sock = SingletonSocket::instance();
                CNetMessage msg;
                msg.SetType(PRO_UPDATE_PACK);
                MakePack(m_package[i],i,msg);
                sock.SendMsg(m_sock,msg);
            }
        }
        else if(m_package[i].tmplId == 684)
        {
            int *pLeft = (int*)m_package[i].addAttrVal;
            if(m_chuZhanPet < m_petNum)
            {
                SPet *pPet = m_pet[m_chuZhanPet].get();
                if(pPet != NULL)
                {
                    if(pPet->mp < pPet->maxMp)
                    {
                        int add = min(pPet->maxMp - pPet->mp,*pLeft);
                        *pLeft -= add;
                        pPet->mp += add;
                        UpdatePetInfo(m_chuZhanPet,6,pPet->mp);
                    }
                    petMp = pPet->mp;
                    petMaxMp = pPet->maxMp;
                }
            }
            if(m_mp < m_maxMp)
            {
                int add = min(m_maxMp - m_mp,*pLeft);
                *pLeft -= add;
                AddMp(add);
            }
            if(*pLeft <= 0)
            {
                NoLockDelPackage(i);
            }
            else
            {
                CSocketServer &sock = SingletonSocket::instance();
                CNetMessage msg;
                msg.SetType(PRO_UPDATE_PACK);
                MakePack(m_package[i],i,msg);
                sock.SendMsg(m_sock,msg);
            }
        }
        if((petHp >= petMaxHp) && (petMp >= petMaxMp) && (m_hp >= m_maxHp) && (m_mp >= m_maxMp))
        {
            return;
        }
    }
}

void CUser::MakeBankPet(CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    msg<<(uint8)m_bankPet.size();
    for(uint8 i = 0; i < m_bankPet.size(); i++)
    {
        msg<<(uint16)m_bankPet[i].tmplId<<(uint16)m_bankPet[i].tmplId<<m_bankPet[i].name<<m_bankPet[i].type<<m_bankPet[i].xiang
            <<m_bankPet[i].level<<m_bankPet[i].exp<<m_bankPet[i].hp<<m_bankPet[i].maxHp<<m_bankPet[i].mp<<m_bankPet[i].maxMp
            <<m_bankPet[i].GetDamage()<<m_bankPet[i].GetSkillAttack()<<(uint16)m_bankPet[i].speed<<m_bankPet[i].GetRecovery()
            <<m_bankPet[i].wuxue<<m_bankPet[i].xiuWei<<m_bankPet[i].shouming<<m_bankPet[i].zhongcheng<<m_bankPet[i].qinmi;
        MakeItemInfo(&(m_bankPet[i].kaiJia),msg);
    //300000时，移动速度不变。每提升200000亲密度，移动速度提升1格，封顶提升3格
        msg<<(uint8)GetPetSpeed(m_bankPet[i].qinmi);
        msg<<m_bankPet[i].GetTiZhi(true)<<m_bankPet[i].GetLiLiang(true)<<m_bankPet[i].GetMinJie(true)
            <<m_bankPet[i].GetLingXing(true)<<m_bankPet[i].GetNaiLi(true)<<m_bankPet[i].shuxingdian
            <<m_bankPet[i].hpCZ<<m_bankPet[i].mpCZ<<m_bankPet[i].speedCZ<<m_bankPet[i].attackCZ<<m_bankPet[i].skillAttackCZ;     
        uint8 num = 0;
        uint16 pos = msg.GetDataLen();
        msg<<num;
        for(uint8 j = 0; j < SPet::MAX_SKILL_NUM; j++)
        {
            if(m_bankPet[i].skill[j] != 0)
            {
                num++;
                msg<<m_bankPet[i].skill[j]<<m_bankPet[i].skillLevel[j];
            }
        }
        msg.WriteData(pos,&num,1);
    }
}

uint8 CUser::GetCurMaxBankPetNum()
{
    uint8 curMaxPackNum = 1;
    uint8 openNum = NoLockGetExtData8(2);
    time_t endTime = NoLockGetExtData32(2);
    if((openNum > 0) && (endTime > GetSysTime()))
    {
        curMaxPackNum += openNum;
        if(curMaxPackNum > MAX_EXT_PET_BANK_NUM+1)
            curMaxPackNum = MAX_EXT_PET_BANK_NUM+1;
    }
    return curMaxPackNum;
}
//pos宠物位置
bool CUser::SaveBankPet(uint8 pos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if((pos >= m_petNum) || (pos >= MAX_PET_NUM))
        return false;
    SPet *pPet = m_pet[pos].get();
    if(pPet == NULL)
        return false;
    if(m_bankPet.size() >= GetCurMaxBankPetNum())//MAX_EXT_PET_BANK_NUM+1)
        return false;
    m_bankPet.push_back(*pPet);
    NoLockDelPet(pos);
    return true;
}

//pos在宠物商店中的位置
bool CUser::GetBankPet(uint8 pos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    if(pos >= m_bankPet.size())
        return false;
        
    SPet *pPet = new SPet;
    *pPet = m_bankPet[pos];
    SharePetPtr pet(pPet);
    
    if(!NoLockAddPet(pet))
    {
        return false;
    }
    m_bankPet.erase(m_bankPet.begin()+pos);
    return true;
}

static const char* ALL_USER_TEXT_TITLE[] = 
{
    "一帮之主","炉火纯青","登峰造极","初为人师","桃李天下","驯养大师","斩妖除魔","鬼界杀手","妖界猎手","除妖尊者","修真者","修仙者","幸运之星","无冕之王"
};

//内容：(byte)数量 （char*）称号(byte)type（0未获得，1获得，2使用）
void CUser::QueryTextTitle(CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    uint8 num = 0;
    uint16 pos = msg.GetDataLen();
    msg<<num;
    for(list<string>::iterator i = m_textTitle.begin(); i != m_textTitle.end(); i++)
    {
        uint8 state = 1;
        if(*i == m_useTextTitle)
        {
            state = 2;
        }
        num++;
        msg<<*i<<state;
        
    }
    for(uint8 i = 0; i < sizeof(ALL_USER_TEXT_TITLE)/sizeof(ALL_USER_TEXT_TITLE[0]); i++)
    {
        if(find(m_textTitle.begin(),m_textTitle.end(),ALL_USER_TEXT_TITLE[i]) == m_textTitle.end())
        {
            msg<<ALL_USER_TEXT_TITLE[i]<<(uint8)0;
            num++;
        }
    }
    msg.WriteData(pos,&num,1);
}

void CUser::AddTextTitle(const char *pTitle)
{
    if(pTitle == NULL)
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(list<string>::iterator i = m_textTitle.begin(); i != m_textTitle.end(); i++)
    {
        if(*i == pTitle)
        {
            return;
        }        
    }
    m_textTitle.push_back(pTitle);
}

void CUser::UseTextTitle(const char *pTitle)
{
    if(pTitle == NULL)
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(strlen(pTitle) <= 0)
    {
        m_useTextTitle.clear();
        return;
    }
    for(list<string>::iterator i = m_textTitle.begin(); i != m_textTitle.end(); i++)
    {
        if(*i == pTitle)
        {
            m_useTextTitle = pTitle;
            return;
        }        
    }
}

void CUser::DelTextTitle(const char *pTitle)
{
    if(pTitle == NULL)
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(list<string>::iterator i = m_textTitle.begin(); i != m_textTitle.end(); i++)
    {
        if(*i == pTitle)
        {
            m_textTitle.erase(i);
            if(m_useTextTitle == pTitle)
                m_useTextTitle.clear();
            return;
        }        
    }
}

