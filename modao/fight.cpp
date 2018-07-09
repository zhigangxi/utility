#include "fight.h"
#include "utility.h"
#include "protocol.h"
#include "net_msg.h"
#include "socket_server.h"
#include "singleton.h"
#include "call_script.h"
#include "script_call.h"
#include "main.h"
#include "database.h"
#include <algorithm>
#include <functional>      // For greater<int>( )
#include <boost/format.hpp>
#include <sys/socket.h>
using namespace std;

template<typename Type> uint8 CFight::AddTmpl(Type val,uint8 pos)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    return AddTmplNoLock(val,pos);
}
template<typename Type> uint8 CFight::AddTmplNoLock(Type val,uint8 pos)
{
    if((pos > 0) && (pos <= MAX_MEMBER))
    {
        if(m_members[pos-1].memPtr.empty())
        {
            m_memNum++;
        }
        m_members[pos-1].memPtr = val;
        m_members[pos-1].state = 0;
        m_members[pos-1].select = false;
        m_members[pos-1].stateAddData1 = 0;//状态的附带参数
        m_members[pos-1].stateAddData2 = 0;//状态的附带参数
        m_members[pos-1].stateTurn = 0;
        m_members[pos-1].option = 0;
        m_members[pos-1].para = 0;
        m_members[pos-1].target = 0;
        m_members[pos-1].protecter = 0;
        
        m_members[pos-1].damageImprove = 0;
        m_members[pos-1].hpImprove = 0;
        m_members[pos-1].recoveryImporve = 0;
        m_members[pos-1].speedImprove = 0;
        m_members[pos-1].dodgeImprove = 0;
        
        m_members[pos-1].damageTrun = 0;
        m_members[pos-1].hpTurn = 0;
        m_members[pos-1].recoveryTurn = 0;
        m_members[pos-1].speedTurn = 0;
        m_members[pos-1].dodgeTurn = 0;
        
        m_members[pos-1].petMask = 0;
        m_members[pos-1].normalDamageTurn = 0;
        m_members[pos-1].normalDamageImprove = 0;
        m_members[pos-1].useSkill23 = false;
        //m_members[pos-1].dropItem.clear();
        return pos;
    }
    return 0;
}

void CFight::Clear()
{
    m_canTaoPao = true;
    m_visibleMonsterId = 0; 
    monsterId1 = 0;
    monsterId2 = 0;
    m_memNum = 0;
    m_userOpTime = 0;
    m_fightIsEnd = false;
    m_useZhaoHuanSkill = false;
    m_beginTime = GetSysTime();
    m_turnBegin = 0;
    m_beginTurnMask = 0;
    m_pScene = NULL;
    
    m_delNpcX = 0;
    m_delNpcy = 0;
    m_diaoxiangId = 0;
    m_fightTurn = 0;
    m_jiangliType = 0;
    
    SFightMember emMember = {0};
    emMember.memPtr = boost::any();
    
    for(uint8 i = 0; i < MAX_MEMBER; i++)
    {
        m_members[i] = emMember;
    } 
}

void CFight::DelMember(uint8 pos)
{
    if((pos > 0) && (pos <= MAX_MEMBER))
    {
        CUser *pUser = GetUser(pos);
        if(pUser != NULL)
        {
            pUser->SetFight(0,0);
        }
        m_members[pos-1].memPtr = boost::any();
        m_memNum--;
    }
}

uint8 CFight::AddUser(ShareUserPtr user,uint8 pos)
{
    uint8 userPos = AddTmpl(user,pos);
    SharePetPtr pet;
    if(user->GetChuZhanPet(pet))
    {
        uint8 &zhongcheng = pet->zhongcheng;
        if(user->HaveBitSet(157))
        {
            AddPet(user.get(),pet,userPos+1);
            if(!NotDecreaseNaiJiu())
            {
                if(Random(1,6) == 1)
                {
                    zhongcheng--;
                    user->UpdatePetInfo(user->GetChuZhanPet(),13,pet->zhongcheng);
                }
            }
        }
        else if((pet->level > user->GetLevel()) && (pet->level - user->GetLevel() > 10))
        {
            SendSysInfo(user.get(),"等级太高无法出战");
        }
        else if(pet->shouming <= 0)
        {
            SendSysInfo(user.get(),"宠物寿命太低不能出战");
        }
        else
        {
            if(zhongcheng < 40)
            {
                if(Random(0,100) > 10)
                {
                    //boost::format fmt("宠物忠诚度太底，不愿意出战。");
                    //fmt%pet->name;
                    SendSysInfo(user.get(),"忠诚太低不愿出战");
                }
                else
                {
                    AddPet(user.get(),pet,userPos+1);
                    if(!NotDecreaseNaiJiu())
                    {
                        if(Random(1,6) == 1)
                        {
                            zhongcheng--;
                            user->UpdatePetInfo(user->GetChuZhanPet(),13,pet->zhongcheng);
                        }
                    }
                }
            }
            else if(zhongcheng < 70)
            {
                if(Random(0,100) > 50)
                {
                    SendSysInfo(user.get(),"忠诚太低不愿出战");
                }
                else
                {
                    AddPet(user.get(),pet,userPos+1);
                    if(!NotDecreaseNaiJiu())
                    {
                        if(Random(1,6) == 1)
                        {
                            zhongcheng--;
                            user->UpdatePetInfo(user->GetChuZhanPet(),13,pet->zhongcheng);
                        }
                    }
                }
            }
            else
            {
                AddPet(user.get(),pet,userPos+1);
                if(!NotDecreaseNaiJiu())
                {
                    if(Random(1,6) == 1)
                    {
                        zhongcheng--;
                        user->UpdatePetInfo(user->GetChuZhanPet(),13,pet->zhongcheng);
                    }
                }
            }
        }
    }
    return userPos;
}

uint8 CFight::AddMonster(ShareMonsterPtr monster,uint8 pos)
{
    if(monsterId1 == 0)
        monsterId1 = monster->tmplId;
    else if(monsterId1 != monster->tmplId)
        monsterId2 = monster->tmplId;
    return AddTmpl(monster,pos);
}

uint8 CFight::AddPet(CUser *pUser,SharePetPtr pet,uint8 pos)
{
    if((m_members[pos-2].petMask & (1<<pUser->GetChuZhanPet())) != 0)
    {
#ifdef DEBUG
        cout<<"add pet error"<<endl;
        cout<<(int)m_members[pos-2].petMask<<":"<<(int)pUser->GetChuZhanPet()<<endl;
#endif        
        return 0;
    }
    m_members[pos-2].petMask |= 1 << pUser->GetChuZhanPet();
    return AddTmplNoLock(pet,pos);
}

CUser *CFight::GetUser(uint8 pos)
{
    if((pos > 0) && (pos <= MAX_MEMBER))
    {
        if(!m_members[pos-1].memPtr.empty()
            && (m_members[pos-1].memPtr.type() == typeid(ShareUserPtr)))
        {
            return (boost::any_cast<ShareUserPtr>(m_members[pos-1].memPtr)).get();
        }
    }
    return NULL;
}

void CFight::BroadcastMsg(CNetMessage &msg)
{
    CSocketServer &sock = SingletonSocket::instance();
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 i = 0; i <= MAX_MEMBER; i++)
    {
        CUser *pUser = GetUser(i);
        if(pUser != NULL)
            sock.SendMsg(pUser->GetSock(),msg);
    }
}

SMonsterInst *CFight::GetMonster(uint8 pos)
{
    if((pos > 0) && (pos <= MAX_MEMBER))
    {
        if(!m_members[pos-1].memPtr.empty()
            && (m_members[pos-1].memPtr.type() == typeid(ShareMonsterPtr)))
        {
            return (boost::any_cast<ShareMonsterPtr>(m_members[pos-1].memPtr)).get();
        }
    }
    return NULL;
}
   
SPet *CFight::GetPet(uint8 pos)    
{
    if((pos > 0) && (pos <= MAX_MEMBER))
    {
        if(!m_members[pos-1].memPtr.empty()
            && (m_members[pos-1].memPtr.type() == typeid(SharePetPtr)))
        {
            return (boost::any_cast<SharePetPtr>(m_members[pos-1].memPtr)).get();
        }
    }
    return NULL;
}

void CFight::GuanZhan(CUser *pGUser)
{
    CNetMessage msg;
    msg.SetType(ENTER_GUANZHAN);
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    int num = 0;
    
    if(m_type == EFTMatch)
        msg<<(uint8)EFTPlayerPk;
    else
        msg<<m_type;
        
    uint16 pos = msg.GetDataLen();
    msg<<m_memNum;
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        CUser *pUser = GetUser(pos);
        SMonsterInst *pMonster = GetMonster(pos);
        SPet *pPet = GetPet(pos);
        if(pUser != NULL)
        {
            int hp = pUser->GetHp();
            if(GetState(pos) == EMSDied)
            {
                hp = 0;
            }
            num++;
            msg<<(uint8)1//玩家
                <<(uint8)(pos)<<pUser->GetRoleId()<<pUser->GetName()
                <<pUser->GetLevel()<<pUser->GetXiang()<<pUser->GetSex()
                <<(uint8)pUser->GetWuQiType()//武器，
                <<pUser->GetMaxHp()<<hp<<pUser->GetMaxMp()<<pUser->GetMp();
        }
        else if(pMonster != NULL)
        {
            int hp = pMonster->hp;
            if(GetState(pos) == EMSDied)
            {
                continue;
            }
            msg<<(uint8)0//怪
                <<(uint8)(pos)<<pMonster->tmplId<<pMonster->name
                <<(uint8)pMonster->level<<pMonster->maxHp<<hp;
            if(m_type == EFTScript)
                msg<<(uint8)EMTNormal;
            else
                msg<<pMonster->type;
            num++;
        }
        else if(pPet != NULL)
        {
            int hp = pPet->hp;
            if(GetState(pos) == EMSDied)
            {
                continue;
            }
            num++;
            msg<<(uint8)2<<pos<<pPet->tmplId<<pPet->name<<pPet->level<<pPet->maxHp
                <<hp<<pPet->maxMp<<pPet->mp;
        }
        
        if(num == m_memNum)
        {
            break;
        }
    }
    msg.WriteData(pos,&num,1);
    CSocketServer &sock = SingletonSocket::instance();
    sock.SendMsg(pGUser->GetSock(),msg);
    m_guanZhanSock.push_back(pGUser->GetSock());
}

void CFight::LeaveGuanZhan(CUser *pUser)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    m_guanZhanSock.remove(pUser->GetSock());
}

void CFight::SendGuanZhanOver()
{
    CSocketServer &sock = SingletonSocket::instance();
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CNetMessage msg;
    msg.SetType(GUANGZHAN_BATTLE_OVER);
    for(list<int>::iterator i = m_guanZhanSock.begin(); i != m_guanZhanSock.end(); i++)
    {
        sock.SendMsg(*i,msg);
    }
}

bool CFight::ReBegin(CSocketServer &sock,ShareUserPtr pReBeginUser)
{
    if(pReBeginUser.get() == NULL)
        return false;
        
    CNetMessage msg;
    msg.SetType(PRO_ENTER_BATTLE);
    bool flag = false;
    
    list<uint32> userList;
    if(m_pScene != NULL)
        m_pScene->GetUserList(userList);
        
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    int num = 0;
    
    if(m_type == EFTMatch)
        msg<<(uint8)EFTPlayerPk;
    else
        msg<<m_type;
        
    uint16 pos = msg.GetDataLen();
    msg<<m_memNum;
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        CUser *pUser = GetUser(pos);
        SMonsterInst *pMonster = GetMonster(pos);
        SPet *pPet = GetPet(pos);
        if((pUser == NULL) && (GetState(pos) >= EMSDied))
            continue;
        if(pUser != NULL)
        {
            //m_members[pos-1].srcHp = pUser->GetHp();
            //m_members[pos-1].srcMp = pUser->GetMp();
            uint8 state = GetState(pos);
            if(pReBeginUser->GetRoleId() == pUser->GetRoleId())
            {
                pUser->SetAutoFightTurn(0);
                AddTmplNoLock(pReBeginUser,pReBeginUser->GetFightPos());
                flag = true;
            }
            int hp = pUser->GetHp();
            if(state == EMSDied)
            {
                hp = 0;
            }
            SetState(pos,state);
            num++;
            msg<<(uint8)1//玩家
                <<(uint8)(pos)<<pUser->GetRoleId()<<pUser->GetName()
                <<pUser->GetLevel()<<pUser->GetXiang()<<pUser->GetSex()
                <<(uint8)pUser->GetWuQiType()//武器，
                <<pUser->GetMaxHp()<<hp<<pUser->GetMaxMp()<<pUser->GetMp();
        }
        else if(pMonster != NULL)
        {
            msg<<(uint8)0//怪
                <<(uint8)(pos)<<pMonster->tmplId<<pMonster->name
                <<(uint8)pMonster->level<<pMonster->maxHp<<pMonster->hp;
            if(m_type == EFTScript)
                msg<<(uint8)EMTNormal;
            else
                msg<<pMonster->type;
            num++;
        }
        else if(pPet != NULL)
        {
            num++;
            msg<<(uint8)2<<pos<<pPet->tmplId<<pPet->name<<pPet->level<<pPet->maxHp
                <<pPet->hp<<pPet->maxMp<<pPet->mp;
        }
        
        if(num == m_memNum)
        {
            break;
        }
    }
    msg.WriteData(pos,&num,1);
    if(flag)
        sock.SendMsg(pReBeginUser->GetSock(),msg);
    UpdateUserInfo(pReBeginUser.get(),userList);
    return flag;
}

void CFight::NoLockBeginFight(CSocketServer &sock,CNetMessage &msg,list<uint32> &userList)
{
    int num = 0;
    
    if(m_type == EFTMatch)
        msg<<(uint8)EFTPlayerPk<<m_memNum;
    else
        msg<<m_type<<m_memNum;
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        m_members[pos-1].turnBegin = 0;//GetSysTime();
        CUser *pUser = GetUser(pos);
        SMonsterInst *pMonster = GetMonster(pos);
        SPet *pPet = GetPet(pos);
        if(pUser != NULL)
        {
            if(!NotDecreaseNaiJiu())
            {
                pUser->DecreaseWuQiNaiJiu(2);
                pUser->DecreaseFangJuNaiJiu(1);
            }
            m_members[pos-1].srcHp = pUser->GetHp();
            m_members[pos-1].srcMp = pUser->GetMp();
            m_members[pos-1].srcPkVal = pUser->GetPkVal();
            m_members[pos-1].srcDaoHang = pUser->GetDaoHang();
            m_members[pos-1].srcExp = pUser->GetExp();
            
            if(m_type == EFTMatch)
            {
                if(pUser->GetMp() != pUser->GetMaxMp())
                {
                    pUser->AddMp(pUser->GetMaxMp() - pUser->GetMp());
                }
                if(pUser->GetHp() != pUser->GetMaxHp())
                {
                    pUser->AddHp(pUser->GetMaxHp() - pUser->GetHp());
                }
            }
            if((m_type != EFTMeetMonster) && (m_type != EFTScript))
            {
                CancelAutoFight(pUser);
                pUser->SaveAutoFight(0,0,0,0,0,0);
            }
            num++;
            msg<<(uint8)1//玩家
                <<(uint8)(pos)<<pUser->GetRoleId()<<pUser->GetName()
                <<pUser->GetLevel()<<pUser->GetXiang()<<pUser->GetSex()
                <<(uint8)pUser->GetWuQiType()//武器，
                <<pUser->GetMaxHp()<<pUser->GetHp()<<pUser->GetMaxMp()<<pUser->GetMp();
        }
        else if(pMonster != NULL)
        {
            msg<<(uint8)0//怪
                <<(uint8)(pos)<<pMonster->tmplId<<pMonster->name
                <<(uint8)pMonster->level<<pMonster->maxHp<<pMonster->hp;
            if(m_type == EFTScript)
                msg<<(uint8)EMTNormal;
            else
                msg<<pMonster->type;
            num++;
        }
        else if(pPet != NULL)
        {
            num++;
            msg<<(uint8)2<<pos<<pPet->tmplId<<pPet->name<<pPet->level<<pPet->maxHp
                <<pPet->hp<<pPet->maxMp<<pPet->mp;
        }
        
        if(num == m_memNum)
        {
            break;
        }
    }
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        CUser *pUser = GetUser(pos);
        if(pUser != NULL)
        {
            sock.SendMsg(pUser->GetSock(),msg);
            UpdateUserInfo(pUser,userList);
        }
    }
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        SMonsterInst *pMonster = GetMonster(pos);
        if((pMonster != NULL) && (pMonster->chatMsg.size() > 0))
        {
            CNetMessage chat;
            chat.SetType(PRO_MSG_CHAT);
            chat<<(uint8)5<<(int)pos<<pMonster->name<<pMonster->chatMsg;
            BroadcastMsg(chat);
        }
    }
    TongJiMsg();
}

void CFight::BeginFight(CSocketServer &sock,CScene *pScene)
{
    CNetMessage msg;
    msg.SetType(PRO_ENTER_BATTLE);
    m_pScene = pScene;
    //m_flghtBegin = true;
    
    list<uint32> userList;
    if(m_pScene != NULL)
        m_pScene->GetUserList(userList);
    
    //m_sendPkInfo = true;
    m_beginTime = GetSysTime();
    SMonsterInst *pShanTao = NULL;
    SMonsterInst *pMonster = NULL;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        NoLockBeginFight(sock,msg,userList);
        pMonster = GetMonster(1);
        pShanTao = GetMonster(3);
    }
    if(pMonster == NULL)
    {
        if((pShanTao != NULL)
           && (pShanTao->GetCeLue() == CL_SHAN_TAN))
        {
            msg.ReWrite();
            msg.SetType(PRO_MSG_CHAT);
            msg<<(uint8)5<<3<<"山桃宝宝"<<"我是山桃宝宝";
            BroadcastMsg(msg);
        }
        return;
    }
    uint16 ceLue = pMonster->GetCeLue();
    if(ceLue == CL_TAO_PAO)
    {
        msg.ReWrite();
        msg.SetType(PRO_MSG_CHAT);
        msg<<(uint8)5<<1<<"小妖"<<"修仙的来了，大家快跑。";
        BroadcastMsg(msg);
    }
}

int CFight::CalculateDamage(uint8 src,uint8 target)
{
    int damage = 0;
    int recovery = 0;
    CUser *pUser = GetUser(src);
    SMonsterInst *pMonster = GetMonster(src);
    SPet *pPet = GetPet(src);
    if(pUser != NULL)
    {
        damage = pUser->GetDamage();
    }
    else if(pMonster != NULL)
    {
        damage = pMonster->attack;
#ifdef DEBUG
        cout<<pMonster->name<<" 伤害:"<<damage<<endl;
#endif
    }
    else if(pPet != NULL)
    {
        damage = pPet->GetDamage();
    }
    damage += GetRecoveryImporve(src);
    if(src > 0)
        damage += m_members[src-1].normalDamageImprove;// = imporveDamage; 
        
    pUser = GetUser(target);
    pMonster = GetMonster(target);
    pPet = GetPet(target);
    
    if(pUser != NULL)
    {
        recovery = pUser->GetRecovery();
        recovery += pUser->GetQiPetKangWuLi();
#ifdef DEBUG
        cout<<pUser->GetName()<<" 防御:"<<damage<<endl;
#endif
    }
    else if(pMonster != NULL)
    {
        recovery = pMonster->recovery;
        if(pMonster->GetCeLue() == DI_MING_ZHONG_NOT_QILING)
        {
            pPet = GetPet(src);
            if((pPet != NULL) && (pPet->tmplId == 102))
            {
                damage *= 2;
            }
        }
    }
    else if(pPet != NULL)
    {
        recovery = pPet->GetRecovery();
    }
    
    //damage += GetDamageImprove(src);
    //recovery += GetRecoveryImporve(target);
    damage = damage - recovery;
    if(damage <= 0)
    {
        damage = 1;
    }
    
    int add = CalculateRate(damage,4,100);
    add = CalculateRate(add,Random(0,100),100);
    
    damage += add;
    
    if(GetRecoveryImporve(target) != 0)    
        damage /= 2;
    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo) || (m_type == EFTMatch))
        damage /= 2;
    if(damage < 1)
        damage = 1;
    return damage;
}

void CFight::GetLiveMember(uint8 *arr,uint8 &num)
{
    num = 0;
    for(uint8 i = 0; i < MAX_MEMBER; i++)
    {
        if(!m_members[i].memPtr.empty() && (GetState(i+1) < EMSDied))
        {
            arr[num] = i+1;
            num++;
        }
    }
}

int CFight::CalculateHitRatio(uint8 src,uint8 target)
{
    CUser *pUser = GetUser(src);
    SMonsterInst *pMonster = GetMonster(src);
    SPet *pPet = GetPet(src);
    
    int level = 0;
    int liliang = 0;
    int tizhi = 0;
    int jin = 0;
    int shui = 0;
    /**********
    当门派为“土”时，命中数公式为：
    等级*3+(力量-等级)*0.75+(体质-等级)*0.75+金相*4+水相*2+45
    其他门派命中数公式为：
    等级*4+(力量-等级)*1.5+(体质-等级)*1.5+金相*8+水相*4+60
    ***********/

    uint16 mingzhong = 0;
    if(pUser != NULL)
    {
        int fagong;
        uint16 huibi;
        pUser->GetViewPara(fagong,mingzhong,huibi);
        //mingzhong = (int)(level*4+(liliang-level)*1.5+(tizhi-level)*1.5+jin*8+shui*4+60);
    }
    else if(pMonster != NULL)
    {
        level = pMonster->level;
        liliang = pMonster->liliang;
        mingzhong = (int)(level*4+(liliang-level)*1.5);
        uint16 ceLue = pMonster->GetCeLue();
        if((ceLue == CL_FENG_YA_LANG)
            || (ceLue == CL_FY_XIONG_LANG))
        {
            mingzhong += 2000;
        }
        mingzhong += pMonster->mingzhong;
    }
    else if(pPet != NULL)
    {
        level = pPet->level;
        liliang = pPet->GetLiLiang();
        mingzhong = (int)(level*4+(liliang-level)*1.5+(tizhi-level)*1.5+jin*8+shui*4+60)
            + pPet->GetItemAddAttr(EAAmingzhong);
    }
    
#ifdef DEBUG    
    if(pUser != NULL)
        cout<<pUser->GetName()<<":";
    else if(pMonster != NULL)
        cout<<pMonster->name<<":";
    else if(pPet != NULL)
        cout<<pPet->name<<":";
    cout<<"命中:"<<mingzhong<<endl;
#endif
    /*********
    当门派为“木”时，回避数公式为：
    等级*8+(敏捷-等级)*2+(耐力-等级)*1+火相*4+木相*7+100
    当门派为“金”和“水”时，回避数公式为：
    等级*2+(敏捷-等级)*0.5+(耐力-等级)*0.25+火相*1+木相*3+30
    其他门派的回避数公式为：
    等级*4+(敏捷-等级)*1+(耐力-等级)*0.5+火相*2.5+木相*5.5+60
    *************/
    uint16 huibi = 0;
    int minjie = 0;
    int naili = 0;
    pUser = GetUser(target);
    pMonster = GetMonster(target);
    pPet = GetPet(target);
    if(pUser != NULL)
    {
        int fagong;
        uint16 mingzhong;
        pUser->GetViewPara(fagong,mingzhong,huibi);
    }
    else if(pMonster != NULL)
    {
        level = pMonster->level;
        minjie = pMonster->minjie;
        huibi = (int)(level*4);
    }
    else if(pPet != NULL)
    {
        level = pPet->level;
        minjie = pPet->GetMinJie();
        naili = pPet->GetNaiLi();
        if(pPet->xiang == EXMuXiang)
        {
            huibi = (int)(level*8+(minjie-level)*2+(naili-level)+100);
        }
        else if((pPet->xiang == EXJinXiang) || (pPet->xiang == EXShuiXiang))
        {
            huibi = (int)(level*2+(minjie-level)*0.5+(naili-level)*0.25+30);
        }
        else
        {
            huibi = (int)(level*4+(minjie-level)+(naili-level)*0.5+60);
        }
    }
#ifdef DEBUG    
    if(pUser != NULL)
        cout<<pUser->GetName()<<":";
    else if(pMonster != NULL)
        cout<<pMonster->name<<":";
    else if(pPet != NULL)
        cout<<pPet->name<<":";
    cout<<"回避:"<<huibi<<endl;
#endif    
    //50%*（攻击方命中数/防御方回避数）+装备附加，最小值5%，最大值100%
    if(huibi <= 0)
        huibi = 1;
    int mingzhongLv = (int)(mingzhong*50/huibi);
    if(mingzhongLv < 5)
        mingzhongLv = 5;
    mingzhongLv -= GetDodgeImprove(target);
    if(mingzhongLv < 0)
        mingzhongLv = 0;
    if((pMonster != NULL) && ((pMonster->GetCeLue() == DI_MING_ZHONG) || (pMonster->GetCeLue() == DI_MING_ZHONG_NOT_QILING)))
    {
        pPet = GetPet(src);
        bool f = true;
        if((pMonster->GetCeLue() == DI_MING_ZHONG_NOT_QILING) && (pPet != NULL) && (pPet->tmplId == 102))
            f = false;
        
        if((f) && (mingzhongLv > pMonster->addData))
            mingzhongLv = pMonster->addData;
    }
    return mingzhongLv;
}

//连击数为1表示不连击
uint8 CFight::GetLianJiShu(uint8 pos,int &lianjiAdd)
//(CUser *pUser,int &lianjiAdd,SMonsterInst *pMonster = NULL)
{
    uint8 lianjilv = 0;
    CUser *pUser = GetUser(pos);
    SMonsterInst *pMonster = GetMonster(pos);
    SPet *pPet = GetPet(pos);
    
    if(pMonster != NULL)
    {
        if(pMonster->lianji > 0)
        {
            if(Random(0,100) < pMonster->lianji )
            {
                lianjiAdd = pMonster->lianjiAdd;
                return pMonster->lianjiShu;
            }
            return 1;
        }
        if(pMonster->GetCeLue() == CE_LUE_BA_XIA)
        {
            lianjilv = 20;
            if(Random(0,100) < lianjilv)
            {
                return 2;
            }
            return 1;
        }
        else if(pMonster->GetSkillLevel(120) > 0)  
        {
            if(Random(0,1) == 0)
            {
                lianjiAdd = 500;
                return 3;
            }
        }
    }
    else if(pUser != NULL)
    {
        lianjilv = pUser->GetItemLianjiLv()/5;
        if(lianjilv <= 0)
            return 1;
        if(Random(0,100) < lianjilv)
        {
            lianjiAdd = pUser->GetLianJiAddShangHai();
            return pUser->GetItemLianjiShu();
        }
    }
    else if(pPet != NULL)
    {
        lianjilv = pPet->GetItemLianjiLv()/5;
        if(lianjilv <= 0)
            return 1;
        if(Random(0,100) < lianjilv)
        {
            lianjiAdd = pPet->GetLianJiAddShangHai();
            return pPet->GetItemLianjiShu();
        }
    }
    return 1;
}

static bool IsFanJi(CUser *pUser,SMonsterInst *pMonster)
{
    uint8 fanjilv = 0;
    if(pUser != NULL)
        fanjilv = pUser->GetItemFanJiLv()/5;
    else if(pMonster != NULL)
    {
        if(pMonster->GetCeLue() == CE_LUE_BA_XIA)
        {
            fanjilv = 20;   
            //cout<<"反击"<<endl;
        }
        else if(pMonster->fanJiLv > 0)
        {
            if(Random(0,100) <= pMonster->fanJiLv)
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }
    else 
        return false;
        
    if(fanjilv <= 0)
        return false;
    if(Random(0,100) < fanjilv)
        return true;
    return false;
}

int CFight::GetFanZhen(uint8 tar)
//(CUser *pUser,SMonsterInst *pMonster=NULL)
{
    SMonsterInst *pMonster = GetMonster(tar);
    CUser *pUser = GetUser(tar);
    SPet *pPet = GetPet(tar);
    
    if(pMonster != NULL) 
    {
        if(pMonster->fanshang > 0)
        {
            if(Random(0,100) <= pMonster->fanshang)
            {
                return pMonster->fanshangadd;
            }
            return 0;
        }
        if(pMonster->GetCeLue() == CL_LAN_RUO_LAOLAO)
        {
            return 10;
        }
        else if(pMonster->GetCeLue() == CE_FANZHENG)
        {
            if(Random(0,9) == 0)
                return 100;
        }
    }
    else if(pUser != NULL)
    {
        uint8 fanzhenlv = pUser->GetItemFanZhenLv()/5;
        if(fanzhenlv <= 0)
            return 0;
        if(Random(0,100) <= fanzhenlv)
        {
            return pUser->GetFanZhenDu();
        }
    }
    else if(pPet != NULL)
    {
        uint8 fanzhenlv = pPet->GetItemFanZhenLv()/5;
        if(fanzhenlv <= 0)
            return 0;
        if(Random(0,100) <= fanzhenlv)
        {
            return pPet->GetFanZhenDu();
        }
    }
    return 0;
}
int CFight::GetBaoJi(uint8 src,int &baojiAdd,int baojiLvAdd)
{
    CUser *pUser = GetUser(src);
    SPet *pPet = GetPet(src);
    SMonsterInst *pMonster = GetMonster(src);
    if(pUser != NULL)
    {
        int baojilv = pUser->GetItemBaoJiLv()/5;
        baojilv += baojiLvAdd;
        
        if(baojilv <= 0)
            return 0;
        if(Random(0,100) < baojilv)
        {
            baojiAdd = pUser->GetItemBaoJiZhuiJIa()/5;
            return 50 + pUser->GetAddBaoJiWeiLi()/5;
        }
    }
    else if(pPet != NULL)
    {
        int baojilv = pPet->GetItemBaoJiLv()/5;
        baojilv += baojiLvAdd;
        
        if(baojilv <= 0)
            return 0;
        if(Random(0,100) < baojilv)
        {
            baojiAdd = pPet->GetItemBaoJiZhuiJIa()/5;
            return 50 + pPet->GetAddBaoJiWeiLi()/5;
        }
    }
    else if(pMonster != NULL)
    {
        int baojilv = pMonster->baojilv;
        
        if(baojilv <= 0)
            return 0;
        if(Random(0,100) < baojilv)
        {
            baojiAdd = pMonster->baojiadd;
            return 50;
        }
    }
    return 0;
}
extern void InitLanRuoMonster2Fu(ShareMonsterPtr &ptr,uint8 monsterLevel,uint16 monsterId = 0);
extern void InitLanRuoMonster2(ShareMonsterPtr &ptr,uint8 monsterLevel);
//extern void InitLanRuoMonster4(ShareMonsterPtr &ptr,uint8 monsterLevel);
void InitGuiYuMonster(ShareMonsterPtr &ptr,uint8 monsterLevel,uint8 level);

uint8 CFight::ZhaoHuanLang(uint8 src,CNetMessage &msg)
{
    if((GetState(src) == EMSForget) || (m_pScene == NULL))
        return 0;
    for(uint8 i = 0; i < GROUP2_BEGIN; i++)
    {
        if((GetMonster(i+1) == NULL) || (GetState(i+1) == EMSDied))
        {
            ShareMonsterPtr ptr;
            ptr = m_pScene->CreateQCFightMonster(12,EMTTongLing,53,"亲卫雄狼",
                7,7,1,0);
            if(ptr.get() == NULL)
                return 0;
            ptr->AddSkill(24,100);
            ptr->AddSkill(153,100);
            ptr->AddSkill(155,100);
            ptr->SetCeLue(CL_FY_XIONG_LANG);
            AddTmplNoLock(ptr,i+1);
            msg<<src<<(uint8)EOTZhaoHuan<<PRO_SUCCESS;
            msg<<(uint8)1;
            msg<<(uint8)0//怪
                <<(uint8)(i+1)<<ptr->tmplId<<ptr->name
                <<(uint8)ptr->level<<ptr->maxHp<<ptr->hp<<(uint8)EMTNormal<<"";
            return 1;
        }
    }
    return 0;
}

uint8 CFight::ZhaoHuanShiHou(uint8 src,CNetMessage &msg)
{
    if((GetState(src) == EMSForget) || (m_pScene == NULL))
        return 0;
    for(uint8 i = 0; i < GROUP2_BEGIN; i++)
    {
        if((GetMonster(i+1) == NULL) || (GetState(i+1) == EMSDied))
        {
            ShareMonsterPtr ptr = m_pScene->CreateQCFightMonster(17,EMTTongLing,43,"石猴",
                8,7,7,3);
            ptr->AddSkill(7,60);
            ptr->AddSkill(152,90);
            ptr->lianji = 60;
            ptr->lianjiShu = 2;
            ptr->lianjiAdd = 200;
            ptr->mingzhong = 1000;
            AddTmplNoLock(ptr,i+1);
            msg<<src<<(uint8)EOTZhaoHuan<<PRO_SUCCESS;
            msg<<(uint8)1;
            msg<<(uint8)0//怪
                <<(uint8)(i+1)<<ptr->tmplId<<ptr->name
                <<(uint8)ptr->level<<ptr->maxHp<<ptr->hp<<(uint8)EMTNormal<<"";
            return 1;
        }
    }
    return 0;
}
uint8 CFight::ZhaoHuanNiu(uint8 src,CNetMessage &msg)
{
    if((GetState(src) == EMSForget) || (m_pScene == NULL))
        return 0;
    for(uint8 i = 0; i < GROUP2_BEGIN; i++)
    {
        if((GetMonster(i+1) == NULL) || (GetState(i+1) == EMSDied))
        {
            ShareMonsterPtr ptr;
            uint8 xiang = 0;
            uint8 r = Random(1,3);
            if(r == 1)
            {
                xiang = 5;
                ptr = m_pScene->CreateQCFightMonster(33,EMTTongLing,65,"牛头妖王",
                       9,9,8,xiang);
                ptr->AddSkill(152,100);
                ptr->AddSkill(24,100);
            }
            else if(r == 2)
            {
                xiang = 5;
                ptr = m_pScene->CreateQCFightMonster(36,EMTTongLing,65,"蟹将王",
                       10,9,1,xiang);
                ptr->AddSkill(11,90);
                ptr->AddSkill(12,90);
                ptr->AddSkill(61,90);
            }
            else
            {
                xiang = 5;
                ptr = m_pScene->CreateQCFightMonster(39,EMTTongLing,65,"骷髅王",
                       7,8,8,xiang);
                ptr->AddSkill(53,100);
                ptr->AddSkill(61,100);
                ptr->AddSkill(65,100);
            }
            AddTmplNoLock(ptr,i+1);
            msg<<src<<(uint8)EOTZhaoHuan<<PRO_SUCCESS;
            msg<<(uint8)1;
            msg<<(uint8)0//怪
                <<(uint8)(i+1)<<ptr->tmplId<<ptr->name
                <<(uint8)ptr->level<<ptr->maxHp<<ptr->hp<<(uint8)EMTNormal<<"";
            return 1;
        }
    }
    return 0;
}

uint8 CFight::ZhaoHuanGuMu(uint8 src,CNetMessage &msg)
{
    if((GetState(src) == EMSForget) || (m_pScene == NULL))
        return 0;
    for(uint8 i = 0; i < GROUP2_BEGIN; i++)
    {
        if((GetMonster(i+1) == NULL) || (GetState(i+1) == EMSDied))
        {
            ShareMonsterPtr ptr;
            string info;
            uint8 xiang = 0;
            if(Random(0,100) < 20)
            {
                ptr = m_pScene->CreateQCFightMonster(21,EMTTongLing,1,"古木天露",
                   1,1,1,xiang);
                ptr->AddSkill(153,100);
                ptr->maxHp += 20000;
                ptr->hp = ptr->maxHp;
                ptr->SetCeLue(CL_GU_MU_TIAN_LU);
                info = "有本事来捉我啊";
            }
            else
            {
                xiang = Random(1,5);
                ptr = m_pScene->CreateQCFightMonster(16,EMTTongLing,62,"毒木",
                   3,8,8,xiang);
                ptr->AddSkill(4*(xiang-1)+3,90);
                ptr->AddSkill(4*(xiang-1)+4,90);
                ptr->AddSkill(57,60);
                ptr->AddSkill(152,90);
                ptr->daohang = 250000;
                ptr->fashubaojilv = 60;
                ptr->fashubaojiadd = 60;
                ptr->fanshang = 10;
                ptr->fanshangadd = 40;
                ptr->SetCeLue(CL_DU_MU);
            }
            AddTmplNoLock(ptr,i+1);
            msg<<src<<(uint8)EOTZhaoHuan<<PRO_SUCCESS;
            msg<<(uint8)1;
            msg<<(uint8)0//怪
                <<(uint8)(i+1)<<ptr->tmplId<<ptr->name
                <<(uint8)ptr->level<<ptr->maxHp<<ptr->hp<<(uint8)EMTNormal<<info;
            return 1;
        }
    }
    return 0;
}

uint8 CFight::ZhaoHuanHuLu(uint8 src,CNetMessage &msg)
{
    if((GetState(src) == EMSForget) || (m_pScene == NULL))
        return 0;
    for(uint8 i = 0; i < GROUP2_BEGIN; i++)
    {
        if((GetMonster(i+1) == NULL) || (GetState(i+1) == EMSDied))
        {
            ShareMonsterPtr ptr;
            ptr = m_pScene->CreateQCFightMonster(100,EMTTongLing,50,"仙葫芦宝宝",
                1,1,1,1);
            if(ptr.get() == NULL)
                return 0;
            ptr->hp = 1;
            ptr->maxHp = 1;
            ptr->SetCeLue(CL_SHUYAO);
            AddTmplNoLock(ptr,i+1);
            msg<<src<<(uint8)EOTZhaoHuan<<PRO_SUCCESS;
            msg<<(uint8)1;
            msg<<(uint8)0//怪
                <<(uint8)(i+1)<<ptr->tmplId<<ptr->name
                <<(uint8)ptr->level<<ptr->maxHp<<ptr->hp<<(uint8)EMTNormal<<"有本事捕捉我呀";
            return 1;
        }
    }
    return 0;
}

uint8 CFight::ZhaoHuanSkill(uint8 src,CNetMessage &msg)
{
    if(GetState(src) == EMSForget)
        return 0;
    SMonsterInst *pMonster = GetMonster(src);
    if((pMonster != NULL) && (pMonster->GetCeLue() == CL_LAN_RUO_LAOLAO))
    {
        for(uint8 i = 0; i < GROUP2_BEGIN; i++)
        {
            if((GetMonster(i+1) == NULL) || (GetState(i+1) == EMSDied))
            {
                ShareMonsterPtr ptr;
                int r = Random(0,2);
                if(r == 0)
                    InitLanRuoMonster2(ptr,pMonster->level);
                else if(r == 1)
                    InitLanRuoMonster2Fu(ptr,pMonster->level);
                else 
                    InitGuiYuMonster(ptr,pMonster->level,3);
                    //InitLanRuoMonster4(ptr,pMonster->level);
                if(ptr.get() == NULL)
                    return 0;
                AddTmplNoLock(ptr,i+1);
                msg<<src<<(uint8)EOTZhaoHuan<<PRO_SUCCESS;
                msg<<(uint8)1;
                msg<<(uint8)0//怪
                    <<(uint8)(i+1)<<ptr->tmplId<<ptr->name
                    <<(uint8)ptr->level<<ptr->maxHp<<ptr->hp<<(uint8)EMTNormal<<"";
                break;
            }
        }
        return 1;
    }
    
    pMonster = GetMonster(3);
    if(pMonster == NULL)
        return 0;
    
    uint8 num = 0;
    msg<<src<<(uint8)EOTZhaoHuan<<PRO_SUCCESS;
    uint16 len = msg.GetDataLen();
    msg<<(uint8)num;
    for(uint8 i = 0; i < GROUP2_BEGIN; i++)
    {
        if((GetMonster(i+1) == NULL) || (GetState(i+1) == EMSDied))
        {
            SMonsterInst *pInst = new SMonsterInst;
            InitHuanYingMonster(pInst,pMonster->level,false);
            ShareMonsterPtr ptr(pInst);
            AddTmplNoLock(ptr,i+1);
            num++;
            msg<<(uint8)0//怪
                <<(uint8)(i+1)<<pInst->tmplId<<pInst->name
                <<(uint8)pInst->level<<pInst->maxHp<<pInst->hp<<(uint8)EMTNormal
                <<"我是幻影";
        }
    }
    msg.WriteData(len,&num,1);
    /*uint8 pos = 2;
    uint8 r = 0;
    while((r = Random(0,5)) == pos)
    {
    }
    std::swap(m_members[r],m_members[pos]);*/
    return 1;
}

uint8 CFight::NormalButtle(uint8 src,uint8 target,CNetMessage &msg)
{
    uint8 tarState = GetState(target);
    uint8 srcState = GetState(src);
    int damage = 0;
    uint8 pound = 1;
    if((tarState >= EMSDied) || IsEmpty(target))
    {
        uint8 targets[GROUP2_BEGIN];
        uint8 num = 0;
        GetAnotherGroup(src,targets,num);
        GetExcept(target,targets,num);
        
        if(num <= 0)
            return 0;
        target = RandSelect(targets,num);
        tarState = GetState(target);
    }    
    else if(tarState == EMSFrost)
    {
        msg<<src<<(uint8)EOTNormal<<target;
        msg<<HIT_TARGET<<(uint8)0;
        msg<<(uint8)1<<0<<(uint8)0<<0<<0;
        return 1;
    }
    
    CUser *pUserSrc = GetUser(src);
    
    if(pUserSrc != NULL)
    {
        if(!NotDecreaseNaiJiu())
            pUserSrc->DecreaseWuQiNaiJiu(3);
    }
    
    msg<<src<<(uint8)EOTNormal<<target;
    
    /***********************************
    1．	命中数：等级*4+（力量点-等级）*1.5
    2．	回避数：等级*4+（力量点-等级）
    3．	攻击方命中率：（攻击方命中数/（防御方回避数*2））^2
    *************************************/
    int hitRatio = CalculateHitRatio(src,target);
    if(Random(0,100) > hitRatio)
    {
        //没有命中目标
        msg<<MISS;
        return 1;
    }
    CUser *pUserTar = GetUser(target);
    if(pUserTar != NULL)
    {
        if(!NotDecreaseNaiJiu())
            pUserTar->DecreaseFangJuNaiJiu(2);
    }
    if(tarState == EMSDizzy)
    {
        SetState(target,0);
    }
    msg<<HIT_TARGET;
    
    //计算伤害
    damage = CalculateDamage(src,target);
    
    uint8 protecter = GetProtecter(target);
    int protectDamage = 0;
    if((protecter != 0) 
        && (GetState(protecter) < EMSDied)
        && (GetState(protecter) != EMSDizzy))
    {
        damage /= 2;
        protectDamage = damage;
        DecreaseHp(protecter,damage);
    }
    pound = 1;
        
    uint8 fanzhen = 0;
    int fanzhenVal = 0;
    uint8 baoji = 0;//1表示爆击、2表示连击
    if(srcState == EMSsiwangchanmian)
    {
        damage = CalculateRate(damage,(int)m_members[src-1].stateAddData1,100);
        baoji = 2;
        //pound = 0x82;//高位为1表示连击不衰减
        pound = 2;
        msg<<(uint8)baoji;
        msg<<pound<<damage<<damage;
        DecreaseHp(target,2*damage);
        SetState(src,0);
    }
    if((tarState == EMSshenlongzhao) && (m_members[target-1].stateAddData1 > 0))
    {
        m_members[target-1].stateAddData1--;
        if(damage < m_members[target-1].stateAddData2)
        {
            damage = 0;
            m_members[target-1].stateAddData2 -= damage;
        }
        else
        {
            damage -= m_members[target-1].stateAddData2;
        }
        if(m_members[target-1].stateAddData1 == 0)
        {
            SetState(target,0);
        }
    }
    else if(tarState == EMSqiankunzhao)
    {
        if(srcState == EMSqiankunzhao)
        {
            SetState(tarState,0);
        }
        else
        {
            fanzhen = 3;//特殊反震
            fanzhenVal = CalculateRate(damage,(int)m_members[target-1].stateAddData1,100);
            if(fanzhenVal <= 0)
                fanzhenVal = 1;
            DecreaseHp(src,fanzhenVal);
        }
        damage = 0;
        SetState(target,0);
    }
    /*
    暴击
    连击
    反击
    反振
    */
    //pound连击
        
    if(pound == 1)
    {//计算连击
        int lianjiAdd = 0;
        pound = GetLianJiShu(src,lianjiAdd);//GetMonster(src));
        uint16 pos = 0;
        int temp = damage;
        /*if(damage == 0)
            temp = CalculateDamage(src,target);
        else 
            temp = damage; */
        
        SMonsterInst *pMonster = GetMonster(target);
        if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
        {
            SPet *pPet = GetPet(src);
            if((pPet == NULL) || (pPet->tmplId != 101))
                damage /= 10;
        }
        else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
        {
            SPet *pPet = GetPet(src);
            if((pPet == NULL) || (pPet->tmplId != 101))
                damage /= 5;
        }
        else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
        {
            SPet *pPet = GetPet(src);
            if(pPet == NULL)
                damage /= 10;
        }
        else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
        {
            SPet *pPet = GetPet(src);
            if(pPet == NULL)
                damage = 1;
        }
        if((pMonster != NULL) && (pMonster->noAdd))
        {
            lianjiAdd = 0;
        }
        
        if(pound > 1)
        {
            damage += lianjiAdd;
            baoji = 2;
            msg<<(uint8)baoji;
            pos = msg.GetDataLen();
            msg<<pound<<damage;
        }
        
        DecreaseHp(target,damage,src);
        
        int fanzhendu = GetFanZhen(target);
        if((fanzhendu > 0) && (fanzhen == 0))
        {
            fanzhen = 2;
            fanzhenVal = CalculateRate(damage,fanzhendu,100);
            if(fanzhenVal <= 0)
                fanzhenVal = 1;
            DecreaseHp(src,fanzhenVal);
            uint8 i = 1;
            msg.WriteData(pos,&i,1);
            pound = 1;
        }
        
        //if(fanzhen == 0)
        int dam = 0;
        for(uint8 i = 1; i < pound; i++)
        {
            temp /= 2;
            dam = temp + lianjiAdd;
            SMonsterInst *pMonster = GetMonster(target);
            if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
            {
                SPet *pPet = GetPet(src);
                if((pPet == NULL) || (pPet->tmplId != 101))
                    dam /= 10;
            }
            else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
            {
                SPet *pPet = GetPet(src);
                if((pPet == NULL) || (pPet->tmplId != 101))
                    dam /= 5;
            }
            else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
            {
                SPet *pPet = GetPet(src);
                if(pPet == NULL)
                    dam /= 10;
            }
            else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
            {
                SPet *pPet = GetPet(src);
                if(pPet == NULL)
                    dam = 1;
            }
            
            DecreaseHp(target,dam,src);
            msg<<dam;
            if(GetState(target) == EMSDied)
            {
                i++;
                msg.WriteData(pos,&i,1);
                break;
            }
            if((fanzhen == 0) && IsFanJi(pUserTar,GetMonster(target)))
            {
                fanzhen = 1;//反击
                fanzhenVal = CalculateDamage(target,src);
                
                SMonsterInst *pMonster = GetMonster(src);
                if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
                {
                    SPet *pPet = GetPet(target);
                    if((pPet == NULL) || (pPet->tmplId != 101))
                        fanzhenVal /= 10;
                }
                else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
                {
                    SPet *pPet = GetPet(target);
                    if((pPet == NULL) || (pPet->tmplId != 101))
                        fanzhenVal /= 5;
                }
                else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
                {
                    SPet *pPet = GetPet(src);
                    if(pPet == NULL)
                        fanzhenVal /= 10;
                }
                else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
                {
                    SPet *pPet = GetPet(src);
                    if(pPet == NULL)
                        fanzhenVal = 1;
                }
                                
                DecreaseHp(src,fanzhenVal);
                i++;
                msg.WriteData(pos,&i,1);
                break;
            }
            int fanzhendu = GetFanZhen(target);
            if((fanzhendu > 0) && (fanzhen == 0))
            {
                fanzhen = 2;
                fanzhenVal = CalculateRate(dam,fanzhendu,100);
                if(fanzhenVal <= 0)
                    fanzhenVal = 1;
                DecreaseHp(src,fanzhenVal);
                i++;
                msg.WriteData(pos,&i,1);
                break;
            }
        }
    }
    
    if(pound == 1)
    {//计算爆击
        int baojiAdd = 0;
        int baojidu = GetBaoJi(src,baojiAdd);
        SMonsterInst *pMonster = GetMonster(target);
        if((pMonster != NULL) && (pMonster->noAdd))
        {
            baojidu = 0;
        }
        if(baojidu > 0)
        {
            baoji = 1;
            int temp = CalculateRate(damage,baojidu,100) + baojiAdd;
            damage += temp;
            DecreaseHp(target,temp,src);
        }
    }
//NormalButtleEnd
    if((fanzhen == 0) && (GetState(target) != EMSDied))
    {
        int fanzhendu = GetFanZhen(target);
        if(IsFanJi(pUserTar,GetMonster(target)))
        {
            fanzhen = 1;//反击
            fanzhenVal = CalculateDamage(target,src);
            SMonsterInst *pMonster = GetMonster(src);
            if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
            {
                SPet *pPet = GetPet(target);
                if((pPet == NULL) || (pPet->tmplId != 101))
                    fanzhenVal /= 10;
            }
            else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
            {
                SPet *pPet = GetPet(target);
                if((pPet == NULL) || (pPet->tmplId != 101))
                    fanzhenVal /= 5;
            }
            else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
            {
                SPet *pPet = GetPet(src);
                if(pPet == NULL)
                    fanzhenVal /= 10;
            }
            else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
            {
                SPet *pPet = GetPet(src);
                if(pPet == NULL)
                    fanzhenVal = 1;
            }
            DecreaseHp(src,fanzhenVal);
        }
        else if(fanzhendu > 0)
        {
            fanzhen = 2;
            fanzhenVal = CalculateRate(damage,fanzhendu,100);
            SMonsterInst *pMonster = GetMonster(src);
            if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
            {
                SPet *pPet = GetPet(target);
                if((pPet == NULL) || (pPet->tmplId != 101))
                    fanzhenVal /= 10;
            }
            else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
            {
                SPet *pPet = GetPet(target);
                if((pPet == NULL) || (pPet->tmplId != 101))
                    fanzhenVal /= 5;
            }
            else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
            {
                SPet *pPet = GetPet(src);
                if(pPet == NULL)
                    fanzhenVal /= 10;
            }
            else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
            {
                SPet *pPet = GetPet(src);
                if(pPet == NULL)
                    fanzhenVal = 1;
            }
            if(fanzhenVal <= 0)
                fanzhenVal = 1;
            DecreaseHp(src,fanzhenVal);
        }
    }
    if(baoji != 2)
    {//==2时表示连击，连击在前面已经处理过
        msg<<(uint8)baoji;
        msg<<pound<<damage;
    }
    msg << (uint8)fanzhen<<fanzhenVal<<protectDamage;

    return 1;
}

int CFight::GetSuccesExp(uint8 pos,int *pMoney)
{
    uint8 begin;
    uint8 end;
    if(pos > GROUP2_BEGIN)
    {
        begin = 1;
        end = GROUP2_BEGIN + 1;
    }
    else
    {
        begin = GROUP2_BEGIN + 1;
        end = MAX_MEMBER + 1;
    }
    
    CUser *pUser = GetUser(pos);
    SPet *pPet = GetPet(pos);
    uint8 level = 0xff;
    if(pUser != NULL)
    {
        level = pUser->GetLevel();
    }
    else if(pPet != NULL)
    {
        level = pPet->level;
        pUser = GetUser(pos-1);
    }
        
    if(level == 0xff)
        return 0;
        
    int exp = 0;
    for(uint8 i = begin; i < end; i++)
    {
        SMonsterInst *pMonster = GetMonster(i);
        if(pMonster != NULL)
        {
            int cha = abs(pMonster->level - level);
            //1-INT(ABS(人等级-怪物等级)/5)*0.1）*怪物经验和物品掉落概率
            if((pUser != NULL) && pUser->NoExperiencePunish())
            {
                exp += pMonster->exp;
            }
            else if(cha <= 5)
            {
                exp += pMonster->exp;
            }
            else
            {
                float per = 1 - (cha-5)*0.06;
                if(per <= 0)
                    per = 0.0;
                exp += (int)(per*pMonster->exp);
            }
            if(cha < 10)
            {
                if(pMoney != NULL)
                    *pMoney = Random(5,20+level/10);
            }
        }
    }
    if(InHuoDong())
    {
        exp *= GetHuoDongBeiLv();
    }
    return exp;
}

uint8 CFight::OneGroupAllDie()
{
    if(ChuShiFightEnd())
    {
        return 1;
    }
    
    bool group1AllDied = true;
    bool group2AllDied = true;
    for(uint8 pos = 0; pos < GROUP2_BEGIN; pos++)
    {
        if((!m_members[pos].memPtr.empty())
            && (m_members[pos].state < EMSDied))
        {
            group1AllDied = false;
            break;
        }
    }
    for(uint8 pos = GROUP2_BEGIN; pos < MAX_MEMBER; pos++)
    {
        if((!m_members[pos].memPtr.empty())
            && (m_members[pos].state < EMSDied))
        {
            group2AllDied = false;
            break;
        }
    }
    if(group1AllDied)
        return 1;
    if(group2AllDied)
        return 2;
    return 0;
}

int CFight::GiveItemByMonster(CUser *pUser,SMonsterInst *pInst)
{
    if(pUser->HaveBitSet(0))
        return 0;
    if(pInst == NULL)
        return 0;
    SMonsterTmpl *pMonster = pInst->pMonster;
    if(pMonster == NULL)
        return 0;
    int monsterLevel = pInst->level;
       
    //int end = 10000 + abs(pUser->GetLevel() - monsterLevel)/5*3000;
    //（1-INT(ABS(人等级-怪物等级)/5)*0.1）
    //float per = 1-abs(pUser->GetLevel())/5
    //end = 
    //掉落惩罚=1-（怪物等级–人物等级-5）*0.08，结果小于0则算0
    double per = 1 - (abs(pUser->GetLevel() - monsterLevel) - 5) * 0.08;
    if(per < 0)
        return 0;
    int end = (int)(10000/per);
    uint8 userNum = 0;
    for(uint8 i = 0; i < MAX_MEMBER; i++)
    {
        if(m_members[i].memPtr.type() == typeid(ShareUserPtr))
            userNum++;
    }
    if(userNum == 2)
    {
        end = (int)(end*3/2);
    }
    else if(userNum == 3)
    {
        end *= 2;
    }
    int num = Random(0,end);
    
    //cout<<"random num:"<<num<<endl;
    SDropItem *pDropItem;
    int dropNum;
    if(pInst->type == EMTTongLing)
    {
        dropNum = pMonster->headDropNum;
        pDropItem = pMonster->pHeadDropItem;
    }
    else if(InHuoDong())
    {
        dropNum = pMonster->dropNum;
        pDropItem = pMonster->pDropItem;
    }
    else 
    {
        dropNum = pMonster->dropNum;
        pDropItem = pMonster->pDropItem;
    }
    for(int i = 0; i < dropNum; i++)
    {
        if((num >= pDropItem[i].begin) && (num <= pDropItem[i].end))
        {
            if(pInst->type == EMTTongLing)
            {
                SItemTemplate *pItem = SingletonItemManager::instance().GetItem(pDropItem[i].itemId);
                if(pItem == NULL)
                    continue;
                if(pItem->type <= EITShouZhuo)
                {
                    SItemInstance item = {0};
                    item.quality = EQTBlue;
                    item.tmplId = pItem->id;
                    item.naijiu = pItem->naijiu;
                    item.num = 1;
                    item.reqAttrType = Random(ERATtizhi,ERATnaili);
                    item.reqAttrVal = Random(pItem->level*2,pItem->level*3);
                    uint8 num = Random(1,3);
                    uint8 count = 0;
                    for(uint8 i = 0; i < 100; i++)
                    {//判断生成不重复属性
                        if(count >= num)
                            break;
                        uint16 min,middle,max;
                        uint8 addrType = RandSelect(BLUE_EQUIP,sizeof(BLUE_EQUIP));
                        bool canAdd = true;
                        for(uint8 j = 0; j < count; j++)
                        {
                            if(addrType == item.addAttrType[j])
                            {
                                canAdd = false;
                                break;
                            }
                        }
                        if(canAdd && CanAddAttr(addrType,pItem->type))
                        {
                            item.addAttrType[count] = addrType;//RandSelect(BLUE_EQUIP,sizeof(BLUE_EQUIP));
                            GetAddAttrVal(item.addAttrType[count],min,middle,max);
                            item.addAttrVal[count] = Random(min,max/20);
                            count++;
                        }
                    }
                    item.addAttrNum = count;
                    pUser->AddPackage(item);
                }
                else
                {
                    pUser->AddPackage(pDropItem[i].itemId);
                }
            }
            else
            {
                pUser->AddPackage(pDropItem[i].itemId);
            }
            return pDropItem[i].itemId;
        }
    }
    return 0;
}

void CFight::DropItem(CUser *pUser,uint8 pos,CNetMessage &msg)
{
    uint16 len = msg.GetDataLen();
    msg<<(uint8)0;
    uint8 num = 0;
    if(pos < GROUP2_BEGIN)
    {
        for(uint8 pos = GROUP2_BEGIN; pos < MAX_MEMBER; pos++)
        {
            SMonsterInst *pMonster = GetMonster(pos+1);
            if(pMonster != 0)
            {
                uint16 id = GiveItemByMonster(pUser,pMonster);
                if(id > 0)
                {
                    num++;
                    msg<<id;
                }
            }
        }
    }
    else
    {
        for(uint8 pos = 0; pos < GROUP2_BEGIN; pos++)
        {
            SMonsterInst *pMonster = GetMonster(pos+1);
            if(pMonster != 0)
            {
                uint16 id = GiveItemByMonster(pUser,pMonster);
                if(id > 0)
                {
                    num++;
                    msg<<id;
                }
            }
        }
    }
    msg.WriteData(len,&num,1);
}

int CFight::GetGongTolDaoHang()
{
    int val = 0;
    CUser *pUser;
    for(uint8 i = 1; i <= GROUP2_BEGIN; i++)
    {
        if(GetState(i) == EMSEscape)
            continue;
        pUser = GetUser(i);
        if(pUser != NULL)
        {
            val += pUser->GetDaoHang();
        }
    }
    return val;
}
int CFight::GetFangTolDaoHang()
{
    int val = 0;
    CUser *pUser;
    for(uint8 i = GROUP2_BEGIN; i <= MAX_MEMBER; i++)
    {
        if(GetState(i) == EMSEscape)
            continue;
        pUser = GetUser(i);
        if(pUser != NULL)
        {
            val += pUser->GetDaoHang();
        }
    }
    return val;
}

void CFight::UpdateUserInfo(CUser *pUser,list<uint32> &userList)
{
    if(pUser == NULL)
        return;
    
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PLAYER);
    
    CSocketServer &sock = SingletonSocket::instance();
    COnlineUser &onlineUser = SingletonOnlineUser::instance();
    
    list<uint32>::iterator iter = userList.begin();
    for(; iter != userList.end(); iter++)
    {
        ShareUserPtr p = onlineUser.GetUserByRoleId(*iter);
        CUser *pU = p.get();
        if((pU != NULL) && (pU->GetRoleId() != pUser->GetRoleId()))
        {
            msg.ReWrite();
            msg.SetType(PRO_UPDATE_PLAYER);
            pUser->MakeUpdateInfo(msg,pU);
            sock.SendMsg(pU->GetSock(),msg);
        }
    }
}

void CFight::GuiJieJiangLi(CUser *pUser,CNetMessage &msg)
{
    if(pUser->HaveBitSet(0))
        return;
    //90秒内战斗结束，奖励：1~2条青鱼(649)
    //90~120秒内战斗结束，奖励：2~4条青鱼（70%）或者 1条黄鱼（30%）
    //120秒以上的奖励：1条黄鱼(650)
    int useTime = GetUseTime();
    //cout<<useTime<<endl;
    if(useTime <= 90)
    {
        int num = Random(2,4);
        pUser->AddPackage(649,num);
        char buf[64];
        sprintf(buf,"得到%d个青鱼",num);
        SendSysInfo1(pUser,buf);
    }
    else if(useTime > 120)
    {
        pUser->AddPackage(650,2);
        SendSysInfo1(pUser,"得到2个黄鱼");
    }
    else
    {
        if(Random(0,100) > 30)
        {
            int num = Random(4,8);
            pUser->AddPackage(649,num);
            char buf[64];
            sprintf(buf,"得到%d个青鱼",num);
            SendSysInfo1(pUser,buf);
        }
        else
        {
            pUser->AddPackage(650,2);
            SendSysInfo1(pUser,"得到2个黄鱼");
        }
    }
    /*int num = Random(1,3);
    pUser->AddBangDingPackage(648,num);
    char buf[64];
    sprintf(buf,"得到%d个雪球",num);
    if(Random(0,100) < 50)
    {
        strcat(buf,"|得到圣诞礼券");
        pUser->AddPackage(646);
    }
    SendSysInfo1(pUser,buf);*/
    
    /*const uint16 DROP_ITEM[] = 
    {
        610,20,// 强化宝石   	20%
        676,55,// 跌打膏药    35%
        726,90,// 紫金丹	35%
        1801,95,// 双卡	5%
        1802,100// 宠双	5%
        //1844,100,//加友好度
    };
     
    uint8 dropNum = 0;
    uint16 pos = msg.GetDataLen();
    msg<<dropNum;
    uint16 dropItem = 0;
    
    uint8 r = Random(0,100);
    char buf[32];
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    for(uint8 i = 0; i < sizeof(DROP_ITEM)/sizeof(uint16)/2; i++)
    {
        int gaiLv = DROP_ITEM[2*i+1];
        uint16 itemId = DROP_ITEM[2*i];
        if(r <= gaiLv)
        {
            if(pUser->AddBangDingPackage(itemId))
            {
                dropItem = itemId;
                dropNum++;
                msg<<dropItem;
                SaveGuiJieJl(pUser,itemId);
                SItemTemplate *pTmpl = itemMgr.GetItem(dropItem);
                if(pTmpl != NULL)
                    snprintf(buf,32,"得到:%s",pTmpl->name.c_str());
                break;
            }
        }
    }
    //int val = pUser->AddDaoHang(600);
    pUser->AddBangDingPackage(1817);
    //pUser->AddPackage(1844);
    sprintf(buf+strlen(buf),"|得到宠物天书");
    SendSysInfo1(pUser,buf);
    msg.WriteData(pos,&dropNum,1);
    
    if((dropItem == 1801) || (dropItem == 1802) || (dropItem == 610))
    {
        string info = pUser->GetName();
        info.append("杀死圣诞雪人：获得");
        msg<<dropItem;
        SItemTemplate *pTmpl = itemMgr.GetItem(dropItem);
        if(pTmpl != NULL)
        {
            info.append(pTmpl->name);
        }
        SysInfoToAllUser(info.c_str());
    }
    */
}

void CFight::NianShouJiangLi(CUser *pUser,CNetMessage &msg)
{
    /*
    七色玲珑=15%                                        
 凝玉芬芳=15%                                                 
 五彩锦绣=20%                                                 
 碧水凝天=30%                                                 
 雍容华贵=10%                                                 
 礼品盒=10%概率（每天100个后概率为0%，雍容华贵概率提高至20%） */
    const uint16 DROP_ITEM[] = 
    {
        2051,15,
        2052,35,
        2053,45,
        2054,75,
        2055,90,
        1100,100
    };
     
    uint8 dropNum = 0;
    uint16 pos = msg.GetDataLen();
    msg<<dropNum;
    uint16 dropItem = 0;
    //uint8 num = GetMonsterNum(52);
    
    uint8 r = Random(0,100);
    for(uint8 i = 0; i < sizeof(DROP_ITEM)/sizeof(uint16)/2; i++)
    {
        int gaiLv = DROP_ITEM[2*i+1];
        uint16 itemId = DROP_ITEM[2*i];
        if(r <= gaiLv)
        {
            if(itemId == 1100)
            {
                if(GetLeftDropNum() <= 0)
                {
                    itemId = 2053;
                }
                else
                {
                    SetLeftDropNum(GetLeftDropNum()-1);
                }
            }
            //cout<<(int)r<<" "<<DROP_ITEM[2*i]<<endl;
            if(pUser->AddPackage(itemId))
            {
                dropItem = itemId;
                dropNum++;
                msg<<dropItem;
                break;
            }
        }
    }
    msg.WriteData(pos,&dropNum,1);
    pUser->SetHuoDongFightTime(GetSysTime());
    return;
}

void CFight::GuiYuEnd()
{
    if(m_type != EFTGuiYu)
        return;
    if(m_pScene == NULL)
        return;
    
    bool flag = true;
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        SMonsterInst *pMonster = GetMonster(pos);
        if(pMonster != NULL)
        {
            if(GetState(pos) != EMSDied)
            {
                flag = false;
                break;
            }
        }
    }
    
    if(flag)
    {
        m_pScene->DelVisibleMonster(m_visibleMonsterId);
        m_pScene->AddGuiYuMonster();
        m_pScene->AddShiYao();
        m_pScene->AddLanRuoMonster(true);
    }
    else
    {
        SVisibleMonster monster;
        m_pScene->FindVisibleMonster(m_visibleMonsterId,monster,0);
    }
}

extern void SetAddTime(uint16 mId,time_t t);

void CFight::VisibleMonsterMisEnd()
{
    if(m_type != EFVisibleMonsterMis)
    {
        return;
    }
    if(m_pScene == NULL)
        return;
    bool delMonster = true;
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        SMonsterInst *pMonster = GetMonster(pos);
        if(pMonster != NULL)
        {
            if(GetState(pos) != EMSDied)
            {
                delMonster = false;
                break;
            }
        }
    }
    if(delMonster)
    {
        m_pScene->DelVisibleMonster(m_visibleMonsterId);
        SetAddTime(m_visibleMonsterId,GetSysTime()+60);
    }
    else
    {
        SVisibleMonster monster;
        m_pScene->FindVisibleMonster(m_visibleMonsterId,monster,0);
    }
}

void CFight::NianShouEnd()
{
    if(m_type != EFGuiJie)
        return;
    if(m_pScene == NULL)
        return;
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        SMonsterInst *pMonster = GetMonster(pos);
        if(pMonster != NULL)
        {
            if(GetState(pos) == EMSDied)
            {
                m_pScene->DelVisibleMonster(pMonster->daohang);
            }
            else
            {
                SVisibleMonster monster;
                m_pScene->FindVisibleMonster(pMonster->daohang,monster,0);
            }
            return;
        }
    }
}

void CFight::SendMatchInfo(uint8 dieGroup)
{
    if((dieGroup < 1) || (dieGroup > 2))
        return;
    string info;
    dieGroup--;
    for(uint8 pos = (1-dieGroup)*GROUP2_BEGIN; pos < (2-dieGroup)*GROUP2_BEGIN; pos++)
    {
        CUser *pUser = GetUser(pos+1);
        if(pUser != NULL)
        {
            info.append(pUser->GetName());
            info.append(" ");
        }
    }
    info.append("在擂台赛击败 ");
    
    for(uint8 pos = dieGroup*GROUP2_BEGIN; pos < (dieGroup+1)*GROUP2_BEGIN; pos++)
    {
        CUser *pUser = GetUser(pos+1);
        if(pUser != NULL)
        {
            info.append(pUser->GetName());
            info.append(" ");
        }
    }
    
    SysInfoToAllUser(info.c_str());
}

void CFight::TongJiMsg()
{
    if(m_type != EFTPlayerPk)
        return;
        
    CUser *pUser1 = NULL;
    CUser *pUser2 = NULL;
    pUser1 = GetUser(3);
    pUser2 = GetUser(9);
    if((pUser1 == NULL) || (pUser2 == NULL))
        return;
    
    if(pUser1->GetPkMiss(pUser2->GetRoleId()))
    {
        CNetMessage chat;
        chat.SetType(PRO_MSG_CHAT);
        chat<<(uint8)5<<pUser1->GetRoleId()<<pUser1->GetName()<<"有人雇我杀你，认命吧";
        BroadcastMsg(chat);
    }
    /*else if(pUser2->GetPkMiss(pUser1->GetRoleId()))
    {
        chat<<(uint8)5<<pUser2->GetRoleId()<<pUser2->GetName()<<"有人雇我杀你，认命吧";
    }*/    
}

extern void FinishPkMiss(CUser *pUser,int missId);
bool CFight::TongJiMiss()
{
    CUser *pDieUser = NULL;
    CUser *pLiveUser = NULL;
    CUser *pUser = NULL;
    pUser = GetUser(3);
    if(pUser != NULL)
    {
        if(GetState(3) < EMSDied)
            pLiveUser = pUser;
        //else 
            //pDieUser = pUser;
    }
    pUser = GetUser(9);
    if(pUser != NULL)
    {
        if(GetState(9) == EMSDied)
            pDieUser = pUser;
    }
    if((pDieUser == NULL) || (pLiveUser == NULL))
        return false;
    int missId = pLiveUser->GetPkMiss(pDieUser->GetRoleId());
    if(missId != 0)
    {
        FinishPkMiss(pLiveUser,missId);
        pDieUser->SetData8(9,pDieUser->GetData8(9)+1);
        char buf[128];
        snprintf(buf,128,"你被%s通缉杀掉。",pLiveUser->GetName());
        SendSysMail(pDieUser,buf);
        return true;
    }
    return false;
}

void CFight::SendPkInfo(uint8 dieGroup)
{
    uint8 userNum = 0;
    CUser *pUser[2] = {0};
    for(uint8 i = 1; i <= MAX_MEMBER; i++)
    {
        CUser *p = GetUser(i);
        if(p != NULL)
        {
            userNum++;
            if(i <= GROUP2_BEGIN)
                pUser[0] = GetUser(i);
            else
                pUser[1] = GetUser(i);
        }
    }
    if(userNum != 2)
        return;
    if((pUser[0] == NULL) || (pUser[1] == NULL))
        return;
    if(abs(pUser[0]->GetLevel() - pUser[1]->GetLevel()) > 5)
        return;
    if((dieGroup < 1) || (dieGroup > 2))
        return;
    string info;
    dieGroup--;
    for(uint8 pos = (1-dieGroup)*GROUP2_BEGIN; pos < (2-dieGroup)*GROUP2_BEGIN; pos++)
    {
        CUser *pUser = GetUser(pos+1);
        if(pUser != NULL)
        {
            info.append(pUser->GetName());
            info.append(" ");
        }
    }
    if(m_pScene != NULL)
    {
        info.append("在");
        info.append(m_pScene->GetName());
    }
    info.append("击败 ");
    
    for(uint8 pos = dieGroup*GROUP2_BEGIN; pos < (dieGroup+1)*GROUP2_BEGIN; pos++)
    {
        CUser *pUser = GetUser(pos+1);
        if(pUser != NULL)
        {
            info.append(pUser->GetName());
            info.append(" ");
        }
    }
    
    SysInfoToAllUser(info.c_str());
}

bool CFight::NotDecreaseNaiJiu()
{
    return (m_type == EFTPlayerQieCuo) || (m_type == EFTMatch);
}

void CFight::Mission21(CUser *pUser,int state)
{
    if((pUser->GetTeam() == 0) || (pUser->GetTeam() == pUser->GetRoleId()))
    {
        uint16 ceLue = 0;
        SMonsterInst *pMonster = NULL;
        for(uint8 i = 1; i <= MAX_MEMBER; i++)
        {
            pMonster = GetMonster(i);
            if((pMonster != NULL) 
                && ((pMonster->GetCeLue() == CL_MISSION21_BAOBAO) 
                || (pMonster->GetCeLue() == CL_MISSION21_BOSS)))
            {
                ceLue = pMonster->GetCeLue();
                break;
            }
        }
        if((ceLue == CL_MISSION21_BAOBAO)
            || (ceLue == CL_MISSION21_BOSS))
        {
            if(pUser == NULL)
                return;
            const char *pMission = pUser->GetMission(21);
            if(pMission == NULL)
                return;
            char buf[128];
            snprintf(buf,128,"%s",pMission);
            char *p[3];
            if(SplitLine(p,3,buf) == 3)
            {
                if(atoi(p[2]) != 0)
                    return;
                bool updateMiss = false;
                if((state == 0) && (ceLue == CL_MISSION21_BAOBAO))
                {
                    if(pMonster->tmplId == 47)
                        updateMiss = pUser->AddPackage(2038);//九尾狐妖残骸    
                    else if(pMonster->tmplId == 48)
                        updateMiss = pUser->AddPackage(2041);//凤凰残骸	    
                    else if(pMonster->tmplId == 49)
                        updateMiss = pUser->AddPackage(2044);//玄武残骸	    
                    else
                        updateMiss = pUser->AddPackage(2047);//青龙残骸
                }
                else
                {
                    if(state == 0)
                    {
                        uint8 r = Random(0,4);
                        if(r == 0)
                            updateMiss = pUser->AddPackage(2040);//九尾狐妖残骸    
                        else if(r == 1)
                            updateMiss = pUser->AddPackage(2043);//凤凰残骸	    
                        else if(r == 2)
                            updateMiss = pUser->AddPackage(2046);//玄武残骸	    
                        else if(r == 2)
                            updateMiss = pUser->AddPackage(2049);//玄武残骸	    
                        else 
                            updateMiss = pUser->AddPackage(2050);//青龙残骸

                    }
                    else
                    {
                        uint8 r = Random(0,3);
                        if(r == 0)
                            updateMiss = pUser->AddPackage(2038);//九尾狐妖残骸    
                        else if(r == 1)
                            updateMiss = pUser->AddPackage(2041);//凤凰残骸	    
                        else if(r == 2)
                            updateMiss = pUser->AddPackage(2044);//玄武残骸	    
                        else
                            updateMiss = pUser->AddPackage(2047);//青龙残骸
                    }
                }
                if(updateMiss)
                {
                    uint16 mId = atoi(p[0]);
                    int monsterId = atoi(p[1]);
                    snprintf(buf,sizeof(buf),"%d|%d|1",mId,monsterId);
                    pUser->UpdateMission(21,buf);
                }
            }
        }
    }
}

extern CMainClass *gpMain;
extern bool ExecMonsterScript(CUser *pUser,int monsterId,int num);

void CFight::OtherTypeUserFightEnd(CUser *pUser,uint8 pos,SPet *pPet,list<uint32> &userList,
                                int state,int exp,int money,uint8 res)
{
    CNetMessage msg;
    msg.SetType(PRO_BATTLE_OVER);
    CSocketServer &sock = SingletonSocket::instance();
    int gongTolDaoHang = GetGongTolDaoHang();
    int fangTolDaoHang = GetFangTolDaoHang();
    bool giveJiangLi = false;
    if(m_type == EFTMeetMonster)
    {
        Mission21(pUser,state);
        if(pUser->UserDouble())
            exp *= 2;
        if(pUser->ScriptDouble())
            exp *= 2;
        //[5，20+（等级/10）]随机。按场次掉钱。
        msg<<res<<exp<<money;
        pUser->AddMoney(money);
        pUser->AddExp(exp,true);
        pUser->Add1104ItemExp(exp);
        if(state == 0)
        {//物品掉落
            DropItem(pUser,pos,msg);
        }
        bool callScript = true;
        if((monsterId1 != 0) && (state == 0) && ExecMonsterScript(pUser,monsterId1,GetMonsterNum(monsterId1)))
        {
            callScript = false;
        }
        if((monsterId2 != 0) && (state == 0) && ExecMonsterScript(pUser,monsterId2,GetMonsterNum(monsterId2)))
        {
            callScript = false;
        }
        if(callScript)
        {
            CCallScript *pScript = GetScript();//fightEnd("10000.lua");
            if(pScript != NULL)
            {
                pUser->SetCallScript(pScript->GetScriptId());
                pScript->Call("FightEnd","uif",pUser,state,this);
            }
        }
    }
    else if(m_type == EFTScript)
    {
        Mission21(pUser,state);
        msg<<res<<0<<0<<(uint8)0;
        
        int script = 0;
        string call = pUser->GetCall(script);
    
        if(!call.empty())
        {
            //char name[16];
            //snprintf(name,16,"%d.lua",script);
            CCallScript *pCallScript = FindScript(script);//(name);
            if(pCallScript != NULL)
            {
                pUser->SetCallScript(pCallScript->GetScriptId());
                //cout<<"脚本战斗结束"<<script<<":"<<call.c_str()<<state<<endl;
                pCallScript->Call(call.c_str(),"uif",pUser,state,this);
                CScene *pScene = pUser->GetScene();
                if((state != 0) && 
                    ((pScene->GetMapId() == LAN_RUO_DI_GONG_ID)|| (pScene->GetMapId() == 304)))
                {
                    m_mutex.unlock();
                    pScene->TempLeaveTeam(pUser);
                    if(state == 1)
                        TransportUser(pUser,301,14,20,8);
                    m_mutex.lock();
                }
            }
        }
        if((state == 0) && (m_delNpcX != 0) && (m_pScene != NULL))
        {
            m_pScene->DelNpc(m_delNpcX,m_delNpcy);
        }
    }
    else if(m_type == EFTNianShou)
    {
        if(state == 0)
        {//胜利
            msg<<PRO_SUCCESS<<0<<0;
            if(giveJiangLi)
                msg<<(uint8)0;
            else
            {
                NianShouJiangLi(pUser,msg);
                giveJiangLi = true;
            }
        }
        else
        {
            m_mutex.unlock();
            CScene *pScene = pUser->GetScene();
            if(pScene != NULL)
                pScene->TempLeaveTeam(pUser);
            if(state == 1)
                TransportUser(pUser,1,13,13,8);
            m_mutex.lock();
            msg<<PRO_ERROR<<0<<0<<(uint8)0;
        }
    }
    else if(m_type == EFGuiJie)
    {
        if(state == 0)
        {//胜利
            msg<<PRO_SUCCESS<<0<<0;
            GuiJieJiangLi(pUser,msg);
        }
        else
        {
            m_mutex.unlock();
            CScene *pScene = pUser->GetScene();
            if(pScene != NULL)
                pScene->TempLeaveTeam(pUser);
            if(state == 1)
                TransportUser(pUser,1,13,13,8);
            m_mutex.lock();
            msg<<PRO_ERROR<<0<<0<<(uint8)0;
        }
    }
    else if(m_type == EFTGuiYu)
    {
        if(state == 0)
        {
            msg<<PRO_SUCCESS<<1<<0<<(uint8)0;
            pUser->AddExp(1);
            CCallScript *pScript = FindScript(78);//fightEnd("10000.lua");
            if(pScript != NULL)
            {
                int num = 0;
                for(uint8 i = 0; i < MAX_MEMBER; i++)
                {
                    if(GetMonster(i+1) != NULL)
                        num++;
                }
                pScript->Call("FightEnd","ui",pUser,num);
            }
        }
        else
        {
            m_mutex.unlock();
            CScene *pScene = pUser->GetScene();
            if((pScene != NULL) && (pScene->GetMapId() != 306))
            {
                if((pScene->GetMapId() == LAN_RUO_DI_GONG_ID)
                    || (pScene->GetMapId() == 304))
                {
                    pScene->TempLeaveTeam(pUser);
                    if(state == 1)
                        TransportUser(pUser,301,14,20,8);
                }
                else
                {
                    pScene->TempLeaveTeam(pUser);
                    if(state == 1)
                        TransportUser(pUser,1,13,13,8);
                }
            }
            m_mutex.lock();
            msg<<PRO_ERROR<<0<<0<<(uint8)0;
        }
    }
    else
    {
        msg<<res<<0<<0<<(uint8)0;
        CancelAutoFight(pUser);
        pUser->SaveAutoFight(0,0,0,0,0,0);
    }
    int petExt = -1;
    if((pPet != NULL) && (GetState(pos+2) < EMSDied))
    {
        pPet->Init();
        petExt = 0;
        if(res == PRO_SUCCESS)
        {
            if(pUser->HaveBitSet(0))
                petExt = 0;
            else
                petExt = GetSuccesExp(pos+2);
            if(pUser->ScriptDouble())
            {
                exp *= 2;
                petExt *= 2;
            }
            if(pUser->UserDouble())
                petExt *= 2;
            if(pUser->PetDouble())
                petExt *= 2;
        }
        pPet->exp += petExt;
        uint32 reqExp = 0;
        if(pPet->tmplId >= 100)
            reqExp = GetLevelUpExp(pPet->level);
        else
            reqExp = GetLevelUpExp(pPet->level)/2;
            
        if(pPet->exp >= (int)reqExp)
        {
            pPet->exp -= reqExp;
            pPet->AddLevel();
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),2,pPet->shuxingdian);     
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),2,pPet->shuxingdian);     
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),5,pPet->hp);              
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),6,pPet->mp);              
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),7,pPet->tizhi);           
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),8,pPet->liliang);         
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),9,pPet->minjie);          
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),10,pPet->lingxing);       
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),11,pPet->naili);          
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),13,pPet->zhongcheng); 
            //pUser->pUser->UpdatePetInfo(pUser->GetChuZhanPet(),14,pPet->qinmi);
        }
        
        pPet->qinmi++;
        pUser->UpdatePet(pUser->GetChuZhanPet());
    }
    else if(!NotDecreaseNaiJiu() && (pPet != NULL) && (GetState(pos+2) == EMSDied) && (m_visibleMonsterId != 0xffff))
    {
        pPet->Init();
        if(pPet->qinmi > 0)
        {
            int des = pPet->qinmi/10;
            if(des <= 0)
                des = 1;
            pPet->qinmi -= des;
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),14,pPet->qinmi);
            //15、骑宠死亡提示亲密度下降。
            char buf[64];
            snprintf(buf,64,"宠物死亡,亲密度丢失%d",des);
            SendSysInfo(pUser,buf);
        }
        if(pPet->zhongcheng > 20)
        {
            pPet->zhongcheng -= 20;
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),13,pPet->zhongcheng);
        }
        else 
        {
            if(pPet->shouming > 500)
            {
                pPet->shouming -= 500;
                pUser->UpdatePetInfo(pUser->GetChuZhanPet(),12,pPet->shouming);
            }
            else
            {
                pPet->shouming = 0;
                pUser->UpdatePetInfo(pUser->GetChuZhanPet(),12,pPet->shouming);
            }
            pPet->zhongcheng = 0;
            pUser->UpdatePetInfo(pUser->GetChuZhanPet(),13,pPet->zhongcheng);
        }
        //pUser->UpdatePet(pUser->GetChuZhanPet());
    }
    msg<<petExt;
    if(GetState(pos+1) != EMSEscape)
    {
        sock.SendMsg(pUser->GetSock(),msg);
        if(pUser->HaveZeroEquip())
        {
            SendSysInfo(pUser,"装备已损坏，请尽快修理");
        }
        if(JiangCheng(state,pUser,pos,gongTolDaoHang,fangTolDaoHang))
        {
            m_mutex.unlock();
            CScene *pScene = pUser->GetScene();
            if(pScene != NULL)
                pScene->TempLeaveTeam(pUser);
            if(pUser->GetSceneId() != SCENE_JIANYU)
            {
                pUser->DelPackageById(1558,-1);
                pUser->DelPackageById(1559,-1);
                pUser->DelPackageById(1560,-1);
                pUser->DelPackageById(1561,-1);
                pUser->DelPackageById(1562,-1);
                TransportUser(pUser,1,13,13,8);
                if((m_pScene != NULL) && ((m_pScene->GetId()>>16) != (int)pUser->GetBangPai()) &&
                    ((m_pScene->GetMapId() == 300) || (m_pScene->GetMapId() == 305)))
                {
                    CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
                    CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
                    CBangPai *pBangPai1 = bPMgr.FindBangPai(m_pScene->GetId()>>16);
                    if((pBangPai != NULL) && (pBangPai1 != NULL))
                    {
                        pBangPai->SetMoney(pBangPai->GetMoney()-2000);
                        SendPopMsg(pUser,"偷窃被抓，扣除2000帮派资金");
                        CNetMessage msg;
                        msg.SetType(PRO_MSG_CHAT);
                        char buf[128];
                        //CBangPai *pBangPai1 = bPMgr.FindBangPai(m_pScene->GetId()>>16);
                        snprintf(buf,128,"本帮成员%s在%s帮偷窃过程中失手被杀",
                            pUser->GetName(),pBangPai1->GetName().c_str());
                        msg<<(uint8)4<<0<<"【系统】"<<buf;
                        COnlineUser &onlineUser = SingletonOnlineUser::instance();
                        CSocketServer &sock = SingletonSocket::instance();
                        list<uint32> userList;
                        pBangPai->GetMember(userList);
                        for(list<uint32>::iterator i = userList.begin(); i != userList.end(); i++)
                        {
                            ShareUserPtr ptr = onlineUser.GetUserByRoleId(*i);
                            if(ptr.get() != NULL)
                            {
                                sock.SendMsg(ptr->GetSock(),msg);
                            }
                        }
                    }
                }
            }
            m_mutex.lock();
        }
        else
        {
            SendUserPos(pUser);
        }
        UpdateUserInfo(pUser,userList);
    }
    if(pUser->IsLogout())
    {
        if(gpMain != NULL)
        {
            gpMain->UserLogOut(pUser);
        }
    }
    else
    {
        if(pUser->HaveBitSet(0))
        {
            SendPopMsg(pUser,"你惹怒了星灵仙子，快去星寿村向她道歉吧。否则打怪做任务得不到任何奖励");
        }
    }
}


void CFight::MeiYingEnd(CUser *pUser,uint8 pos,SPet *pPet,list<uint32> &userList,
                                int state,uint8 res)
{
    CNetMessage msg;
    msg.SetType(PRO_BATTLE_OVER);
    CSocketServer &sock = SingletonSocket::instance();
    msg<<res<<0<<0<<(uint8)0<<0;
    
    if(pPet != NULL)
    {
        pUser->UpdatePet(pUser->GetChuZhanPet());
    }
    
    sock.SendMsg(pUser->GetSock(),msg);
    if(pUser->HaveZeroEquip())
    {
        SendSysInfo(pUser,"装备已损坏，请尽快修理");
    }
    SendUserPos(pUser);
    UpdateUserInfo(pUser,userList);

    if(pUser->IsLogout())
    {
        if(gpMain != NULL)
            gpMain->UserLogOut(pUser);
    }
    
    if(m_type == EFTDiaoXiang)
    {
        int script = 0;
        string call = pUser->GetCall(script);
        if(!call.empty())
        {
            CCallScript *pCallScript = FindScript(script);//(name);
            if(pCallScript != NULL)
            {
                pUser->SetCallScript(pCallScript->GetScriptId());
                pCallScript->Call(call.c_str(),"uif",pUser,state,this);
            }
        }
        if(state == 0)
        {
            m_mutex.unlock();
            CScene *pScene = pUser->GetScene();
            if(pScene == NULL)
            {
                m_mutex.lock();
                return;
            }
            CSceneManager &scene = SingletonSceneManager::instance();
            CScene *pGroup = scene.FindScene(pScene->GetGroupId());
            if(pGroup != NULL)
            {
                pGroup->SetDiaoXiangBang(pUser->GetBangPai(),m_diaoxiangId);
                char buf[64];
                CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
                CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
                if(pBangPai != NULL)
                {
                    snprintf(buf,64,"%s帮占领一处雕像",pBangPai->GetName().c_str());
                    SendSysInfoToGroup(pGroup->GetGroupId(),buf);
                }
            }
            m_mutex.lock();
        }
        else
        {
            m_mutex.unlock();
            CScene *pScene = pUser->GetScene();
            if(pScene == NULL)
            {
                m_mutex.lock();
                return;
            }
            CSceneManager &scene = SingletonSceneManager::instance();
            CScene *pGroup = scene.FindScene(pScene->GetGroupId());
            if(pGroup != NULL)
            {
                int jifen = pGroup->GetOtherJiFen(pUser->GetBangPai())+2;
                pGroup->SetOtherJiFen(pUser->GetBangPai(),jifen);
                if(pUser->DelPackageById(1543,-1) && (m_type == EFTPlayerPk))
                    jifen++;
                if(pUser->DelPackageById(1544,-1) && (m_type == EFTPlayerPk))
                    jifen++;
                
                pGroup->SetOtherJiFen(pUser->GetBangPai(),jifen);
                
                if((0xffff & pGroup->GetId()) == (int)pUser->GetBangPai())
                {
                    TransportUser(pUser,261,6,11,8);
                }
                else
                {
                    TransportUser(pUser,260,9,10,8);
                }
            }
            m_mutex.lock();
        }
        return;
    }
    if(state != 0)
    {
        m_mutex.unlock();
        CScene *pScene = pUser->GetScene();
        if(pScene == NULL)
        {
            m_mutex.lock();
            return;
        }
        pScene->TempLeaveTeam(pUser);
        CSceneManager &scene = SingletonSceneManager::instance();
        CScene *pGroup = scene.FindScene(pScene->GetGroupId());
        if(pGroup == NULL)
        {
            m_mutex.lock();
            return;
        }
        int jifen = pGroup->GetOtherJiFen(pUser->GetBangPai())+2;
        
        if(pUser->DelPackageById(1543,-1) && (m_type == EFTPlayerPk))
            jifen += 2;
        if(pUser->DelPackageById(1544,-1) && (m_type == EFTPlayerPk))
            jifen += 2;
        
        pGroup->SetOtherJiFen(pUser->GetBangPai(),jifen);
        
        if((0xffff & pGroup->GetId()) == (int)pUser->GetBangPai())
        {
            TransportUser(pUser,261,6,11,8);
        }
        else
        {
            TransportUser(pUser,260,9,10,8);
        }
        CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
        CBangPai *pBangPai = bPMgr.FindBangPai(pUser->GetBangPai());
        if(pBangPai != NULL)
        {
            char buf[64];
            snprintf(buf,64,"%s帮一名成员被击杀",pBangPai->GetName().c_str());
            SendSysInfoToGroup(pGroup->GetGroupId(),buf);
        }
        m_mutex.lock();
    }
    else
    {
        pUser->SetBangZhanScore(pUser->GetBangZhanScene()+1);
    }
}

void CFight::MatchUserFightEnd(CUser *pUser,uint8 pos,SPet *pPet,list<uint32> &userList,
                                int state,uint8 res)
{
    CNetMessage msg;
    msg.SetType(PRO_BATTLE_OVER);
    CSocketServer &sock = SingletonSocket::instance();
    msg<<res<<0<<0<<(uint8)0<<0;
        
    CancelAutoFight(pUser);
    pUser->SaveAutoFight(0,0,0,0,0,0);
    if(pPet != NULL)
    {
        pUser->UpdatePet(pUser->GetChuZhanPet());
    }
    
    sock.SendMsg(pUser->GetSock(),msg);
    if(pUser->HaveZeroEquip())
    {
        SendSysInfo(pUser,"装备已损坏，请尽快修理");
    }
    SendUserPos(pUser);
    UpdateUserInfo(pUser,userList);

    if(pUser->IsLogout())
    {
        if(gpMain != NULL)
            gpMain->UserLogOut(pUser);
    }
    int srcHp = m_members[pos].srcHp;
    int srcMp = m_members[pos].srcMp;
    if(srcHp != pUser->GetHp())
    {
        pUser->AddHp(srcHp-pUser->GetHp());
    }
    if(srcMp != pUser->GetMp())
    {
        pUser->AddMp(srcMp-pUser->GetMp());
    }
    short jifen = 0;//pUser->GetData16(1);
    if(state == 0)
    {
        jifen = GetAnotherGroupNum(pos);
    }
    else
    {
        jifen -= 1;
        //pUser->SetData8(1,pUser->GetData8(1)+1);
    }
    //pUser->SetData16(1,jifen);
    if(m_pScene != NULL)
        m_pScene->SetUserJiFen(pUser->GetRoleId(),(char*)pUser->GetName(),jifen);
        
    /*if(pUser->GetData8(1) >= 3)
    {
        m_mutex.unlock();
        CScene *pScene = pUser->GetScene();
        if(pScene != NULL)
            pScene->TempLeaveTeam(pUser);
        TransportUser(pUser,29,11,7,8);
        m_mutex.lock();
    }*/
}

void CFight::UserFightEnd(uint8 pos,list<uint32> &userList)
{
    CUser *pUser = GetUser(pos+1);
    if(pUser == NULL)
        return;
    pUser->UseFightEnd();
    
    int curHp = pUser->GetHp();
    pUser->Init();
    if(curHp != pUser->GetHp())
    {
        pUser->SendUpdateInfo(10,pUser->GetMaxHp());
    }
    uint8 res = PRO_ERROR;
    int exp = 0;
    
    int state = 0;//state:0胜利、1 死亡、2 逃跑
    if(GetState(pos+1) < EMSDied)
        state = 0;
    else if(GetState(pos+1) == EMSDied)
        state = 1;
    else 
        state = 2;
    
    AddJiangLi(pUser,state);
    pUser->SetFight(0,0);
    SPet *pPet = GetPet(pos+2);
    int money = 0;//Random(5,20+pUser->GetLevel()/10);
    
    if((pPet != NULL) && (GetState(pos+2) < EMSDied))
        state = 0;
    if(state == 0)
    {
        res = PRO_SUCCESS;
        if(pUser->HaveBitSet(0))
        {
            exp = 0;
        }
        else
        {
            exp = GetSuccesExp(pos+1,&money);
        }
    }
    bool bangzhan = false;
    if((m_type == EFMeiYIng) || (m_type == EFTDiaoXiang))
        bangzhan = true;
    else if(m_type == EFTPlayerPk)
    {
        CScene *pScene = pUser->GetScene();
        if((pScene->GetMapId() >= 260) && (pScene->GetMapId() <= 266))
        {
            bangzhan = true;
        }
    }
    if(bangzhan)
    {
        MeiYingEnd(pUser,pos,pPet,userList,
                                state,res);
        return;
    }
    if(m_type == EFTMatch)
    {
        MatchUserFightEnd(pUser,pos,pPet,userList,
                                state,res);
    }
    else
    {
        OtherTypeUserFightEnd(pUser,pos,pPet,userList,
                                state,exp,money,res);
    }
}
void CFight::AddJiangLi(CUser *pUser,int state)
{
    if(state != 0)
        return;
    switch(m_jiangliType)
    {
    /***************
    30只怪奖励
    每场: 经验：人物等级*100
    ***************/
    case EFJLType1:
        {
            pUser->AddExp(pUser->GetLevel()*30);
            pUser->AddQianNeng(pUser->GetLevel()*100);
            char buf[128] = {0};
            sprintf(buf,"得到%d经验|得到%d潜能",pUser->GetLevel()*30   
                    ,pUser->GetLevel()*100);
            int r = Random(0,100);
            //5%项链图样
            if(r < 5)
            {//601
                pUser->AddPackage(601);
                strcat(buf,"|得到项链图样");
            }
            //5%残破项圈
            else if(r < 10)
            {//602
                pUser->AddPackage(602);
                strcat(buf,"|残破项圈");
            }
            //5%金丝线
    	    else if(r < 15)
            {//603
                pUser->AddPackage(603);
                strcat(buf,"|金丝线");
            }
            //5%戒指图样
            else if(r < 20)
            {//604
                pUser->AddPackage(604);
                strcat(buf,"|戒指图样");
            }
        	//5%残破戒指
        	else if(r < 25)
            {//605
                pUser->AddPackage(605);
                strcat(buf,"|残破戒指");
            }
        	//5%天然玉
        	else if(r < 30)
            {//606
                pUser->AddPackage(606);
                strcat(buf,"|天然玉");
            }
        	//5%护腕图样
        	else if(r < 35)
            {//607
                pUser->AddPackage(607);
                strcat(buf,"|护腕图样");
            }
        	//3%残破护腕
        	else if(r < 38)
            {//608
                pUser->AddPackage(608);
                strcat(buf,"|残破护腕");
            }
        	//2%翡翠石
        	else if(r < 40)
            {//609
                pUser->AddPackage(609);
                strcat(buf,"|翡翠石");
            }
            else if(r < 60)
            {
                pUser->AddPackage(615);
                strcat(buf,"|蓝水晶");
            }
            SendSysInfo1(pUser,buf);
        }
        break;

    /*
    侍女奖励
	经验：人物等级*500
	潜能：人物等级*500
    */
    case EFJLType2:
        {
            pUser->AddExp(pUser->GetLevel()*100);
            pUser->AddQianNeng(pUser->GetLevel()*500);
            char buf[128] = {0};
            sprintf(buf,"得到%d经验|得到%d潜能",pUser->GetLevel()*100   
                    ,pUser->GetLevel()*500);
            int r = Random(0,100);
            //10%空白蓝水晶
            if(r < 10)
            {//605
                pUser->AddPackage(615);
                strcat(buf,"|蓝水晶");
            }
            //10%项链图样
            else if(r < 20)
            {//601
                pUser->AddPackage(601);
                strcat(buf,"|项链图样");
            }
            //5%残破项圈
            else if(r < 25)
            {//602
                pUser->AddPackage(602);
                strcat(buf,"|残破项圈");
            }
    	    //10%金丝线
    	    else if(r < 35)
    	    {//603
    	        pUser->AddPackage(603);
    	        strcat(buf,"|金丝线");
    	    }
    	    //5%戒指图样
    	    else if(r < 40)
    	    {//604
    	        pUser->AddPackage(604);
    	        strcat(buf,"|戒指图样");
    	    }
    	    //5%残破戒指
    	    else if(r < 45)
    	    {//605
    	        pUser->AddPackage(605);
    	        strcat(buf,"|残破戒指");
    	    }
    	    //5%天然玉
    	    else if(r < 50)
    	    {//606
    	        pUser->AddPackage(606);
    	        strcat(buf,"|天然玉");
    	    }
    	    //5%护腕图样
    	    else if(r < 55)
    	    {//607
    	        pUser->AddPackage(607);
    	        strcat(buf,"|护腕图样");
    	    }
    	    //5%残破护腕
    	    else if(r < 60)
    	    {//608
    	        pUser->AddPackage(608);
    	        strcat(buf,"|残破护腕");
    	    }
    	    //5%翡翠石
    	    else if(r < 65)
    	    {//609
    	        pUser->AddPackage(609);
    	        strcat(buf,"|翡翠石");
    	    }
            //10%：铜项圈
            else if(r < 75)
            {//401
                pUser->AddPackage(401);
                strcat(buf,"|铜项圈");
            }
            //10%：铜戒指
            else if(r < 85)
            {//451
                pUser->AddPackage(451);
                strcat(buf,"|铜戒指");
            }
            //5%：铜护腕
            else if(r < 90)
            {//501
                pUser->AddPackage(501);
                strcat(buf,"|铜护腕");
            }
            //10%：蓝水晶合成符（绑）
            else 
            {//616
                pUser->AddBangDingPackage(616);
                strcat(buf,"|水晶合成卷");
            }
            SendSysInfo1(pUser,buf);
        }        
        break;
    
    /*
    逃跑怪奖励
    经验：人物等级*100
    道行：人物等级*12
    */
    case EFJLType3:
        {
            pUser->AddExp(pUser->GetLevel()*30);
            pUser->AddDaoHang(pUser->GetLevel()*12);
            pUser->AddQianNeng(pUser->GetLevel()*100);
            char buf[128] = {0};
            sprintf(buf,"得到%d经验|得到%d道行|得到%d潜能",pUser->GetLevel()*30
                ,pUser->GetLevel()*12,pUser->GetLevel()*100);
            int r = Random(0,100);
            //30%蓝水晶合成符
            //70%空白蓝水晶
            if(r < 30)
            {
                pUser->AddBangDingPackage(616);
                strcat(buf,"|水晶合成卷");
            }
            else
            {
                pUser->AddPackage(615);
                strcat(buf,"|蓝水晶");
            }
            SendSysInfo1(pUser,buf);
        }        
        break;
    
    /*
    爆炸怪奖励
    经验：人物等级*100
    道行：人物等级*12
    50%蓝水晶合成符
    50%空白蓝水晶
    */
    case EFJLType4:
        {
            pUser->AddExp(pUser->GetLevel()*30);
            pUser->AddDaoHang(pUser->GetLevel()*12);
            pUser->AddQianNeng(pUser->GetLevel()*100);
            char buf[128] = {0};
            sprintf(buf,"得到%d经验|得到%d道行|得到%d潜能",pUser->GetLevel()*30,
                pUser->GetLevel()*12,pUser->GetLevel()*100);
            int r = Random(0,100);
            //50%蓝水晶合成符
            //50%空白蓝水晶
            if(r < 50)
            {
                pUser->AddPackage(616);
                strcat(buf,"|水晶合成卷");
            }
            else
            {
                pUser->AddPackage(615);
                strcat(buf,"|蓝水晶");
            }
            SendSysInfo1(pUser,buf);
        }
        break;
    
    /*
    姥姥奖励
	经验：人物等级*1000
	潜能：人物等级*1000
	道行：人物等级*15
    */
    case EFJLType5:
        {
            pUser->AddExp(pUser->GetLevel()*300);
            pUser->AddDaoHang(pUser->GetLevel()*15);
            pUser->AddQianNeng(pUser->GetLevel()*1000);
            //掉落：掉落3种东西，其中蓝水晶1个，合成符5个，20级饰品选一。
	        //带蓝属性的蓝水晶（绑）*1
	        //从所有蓝属性中随机生产一条，属性值为：
            //70%  最高值的 5%~10%
            //30%  最高值的 10%~30%
            //蓝水晶合成符（绑）*5
            //铜项圈 或 铜戒指 或 铜护腕 取一。
            //401,451,501
            pUser->AddPackage(615);
            pUser->AddBangDingPackage(616,5);
            char buf[128] = {0};
            sprintf(buf,"得到%d经验|得到%d道行|得到%d潜能",pUser->GetLevel()*300
                        ,pUser->GetLevel()*15,pUser->GetLevel()*1000);
                        
            int r = Random(0,2);
            if(r == 0)
            {
                pUser->AddPackage(401);
                strcat(buf,"|铜项圈");
            }
            else if(r == 1)
            {
                pUser->AddPackage(451);
                strcat(buf,"|铜戒指");
            }
            else 
            {
                pUser->AddPackage(501);
                strcat(buf,"|铜护腕");
            }
            r = Random(0,sizeof(BLUE_EQUIP)-1);
            uint16 min = 0,max = 1,middle = 1;
            uint8 type = BLUE_EQUIP[r];
            GetAddAttrVal(type,min,middle,max);
            r = Random(0,100);
            int val = 0;
            if(r < 70)
            {
                val = (int)(max * Random(5,10)/100);
            }
            else
            {
                val = (int)(max * Random(10,30)/100);
            }
            pUser->AddBlueCrystal(type,val);
            strcat(buf,"|蓝水晶");
            SendSysInfo1(pUser,buf);
        }
        break;
    case EFJLType6:
        {
            char buf[64] = {0};
            if(pUser->GetLevel() < 30)
            {
                pUser->AddExp(200);
                strcat(buf,"得到经验200");
            }
            if(pUser->GetMission(169) != NULL)
            {
                pUser->UpdateMission(169,"1");
                strcat(buf,"|消除灾患|任务完成");
            }
            SendSysInfo1(pUser,buf);
        }
        break;
    case EFJLType7:
        {
            char buf[64] = {0};
            if(pUser->GetLevel() < 40)
            {
                pUser->AddExp(500);
                strcat(buf,"得到经验500");
            }
            if(pUser->GetMission(178) != NULL)
            {
                pUser->UpdateMission(178,"1");
                strcat(buf,"|恐怖尸患|任务完成");
            }
            SendSysInfo1(pUser,buf);
        }
        break;
    case EFJLType8:
        {
            char buf[64] = {0};
            if(pUser->GetLevel() < 45)
            {
                pUser->AddExp(1000);
                strcat(buf,"得到经验1000");
            }
            if(pUser->GetMission(182) != NULL)
            {
                pUser->UpdateMission(182,"1");
                strcat(buf,"|狐妖三姐妹|任务完成");
            }
            SendSysInfo1(pUser,buf);
        }
        break;
    case EFJLType9:
        if(pUser->HaveBitSet(0))
            return;
        GiveJiangLi9(pUser);
        break;
    case EFJLType10:
        GiveJiangLi10(pUser);
        break;
    case EFJLType11:
        GiveJiangLi11(pUser);
        break;
    }
}

int CFight::GetShiYaoExp(uint8 pos)
{
    uint8 begin;
    uint8 end;
    if(pos > GROUP2_BEGIN)
    {
        begin = 1;
        end = GROUP2_BEGIN + 1;
    }
    else
    {
        begin = GROUP2_BEGIN + 1;
        end = MAX_MEMBER + 1;
    }
    
    CUser *pUser = GetUser(pos);
    SPet *pPet = GetPet(pos);
    uint8 level = 0xff;
    int exp = 0;
    if(pUser != NULL)
    {
        level = pUser->GetLevel();
        exp = level * 40;
    }
    else if(pPet != NULL)
    {
        level = pPet->level;
        pUser = GetUser(pos-1);
    }
        
    if(level == 0xff)
        return 0;
    
    for(uint8 i = begin; i < end; i++)
    {
        SMonsterInst *pMonster = GetMonster(i);
        if(pMonster != NULL)
        {
            int cha = abs(pMonster->level - level);
            //1-INT(ABS(人等级-怪物等级)/5)*0.1）*怪物经验和物品掉落概率
            if((pUser != NULL) && pUser->NoExperiencePunish())
            {
            }
            else if(cha <= 5)
            {
            }
            else
            {
                float per = 1 - (cha-5)*0.06;
                if(per <= 0)
                    per = 0.0;
                exp = (int)(per*exp);
            }
            break;
        }
    }
    int userNum = 0;
    for(uint8 i = 1; i <= MAX_MEMBER; i++)
    {
        if(GetUser(i) != NULL)
        {
            userNum++;
            if(userNum > 1)
            {
                exp *= 3;
                break;
            }
        }
    }
    return exp;
}
void CFight::GiveJiangLi9(CUser *pUser)
{
    /*
    经验:当前等级的单怪经验值*6*3*15（原有实力）
    道行:人等级*2			
    必掉2种
    经验(等级*10+1000)	1~3件		
    道行(等级*10)	1~3件		
    馒头	1~3件		675
    春卷	1~3件		676
    烤肉串	1~3件		677
    五花肉	1~3件		678
    红烧鸡	1~3件		679
    佛跳墙	1~3件		680
    项链图样	1~5件		601
    残破项圈	1~5件		602
    戒指图样	1~5件		604
    残破戒指	1~5件		605
    护腕图样	1~5件		607
    残破护腕	1~5件		608
    仙木LV1	1~5件		640
    鬼翎LV1	1~5件		641
    兽骨LV1	1~5件		642
    */
    uint16 items[] = {675,676,677,678,679,680,601,602,604,605,607,608,640,641,642};
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    uint8 num = sizeof(items)/sizeof(items[0]);
    
    int exp =  GetShiYaoExp(pUser->GetFightPos());
    int daohang = pUser->GetLevel() * 2;
    char buf[256];
    int len = 0;
    for(uint8 i = 0; i < 2; i++)
    {
        uint8 pos = Random(0,num);
        if(pos == num)
        {
            daohang += pUser->GetLevel()*10*Random(1,3);
        }
        else
        {
            SItemTemplate *pTmpl = itemMgr.GetItem(items[pos]);
            uint8 num = Random(1,3);
            if((pTmpl != NULL) && pUser->AddPackage(items[pos],num))
            {
                sprintf(buf+len,"得到%s×%d|",pTmpl->name.c_str(),num);
                len = strlen(buf);
            }
        }
    }
    pUser->AddExp(exp);
    pUser->AddDaoHang(daohang);
    sprintf(buf+len,"得到经验%d|",exp);
    len = strlen(buf);
    sprintf(buf+len,"得到道行%d",daohang);
    SendSysInfo1(pUser,buf);
}
void CFight::GiveJiangLi10(CUser *pUser)
{
    /*
    必掉			
    经验:人等级*180			
    道行:人等级*18			
    必掉6种			
    经验(等级*10+1000)	3~10件		
    道行(等级*10)	3~10件		
    金丝线	1~3件		603
    天然玉	1~3件		606
    翡翠石	1~3件		609
    天换石	1~3件		623
    蓝水晶	1~3件		615
    仙木LV2	1~5件		640
    仙木LV3	1~5件		640
    鬼翎LV2	1~5件		641
    鬼翎LV3	1~5件		641
    兽骨LV2	1~5件		642
    兽骨LV3	1~5件		642
    师门令	1~3件		1833
    礼花	1~3件		1839
    */
    uint16 items[] = {
        3,603 ,
        3,606 ,
        3,609 ,
        3,623 ,
        3,615 ,
        5,640 ,
        5,640 ,
        5,641 ,
        5,641 ,
        5,642 ,
        5,642 ,
        3,1833,
        3,1839
    };
    
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    uint8 num = sizeof(items)/sizeof(items[0])/2;
    
    int exp = pUser->GetLevel() * 180;
    int daohang = pUser->GetLevel() * 18;
    char buf[512];
    int len = 0;
    
    /*
    第四背包2076
    规格：1~2格
    时间：1~3天
    掉落概率：30%
    
    仓库2077
    规格：1~6格
    时间：1~3天
    掉落概率：30%
    
    宠物仓库2078
    规格：1格
    时间：1~3天
    掉落概率：30%
    */
    buf[0] = 0;
    if((Random(0,100) < 30) && (pUser->AddAttrPackage(2076,Random(1,2),Random(1,3))))
    {
        strcat(buf,"得到第四背包|");        
    }
    if((Random(0,100) < 30) && (pUser->AddAttrPackage(2077,Random(1,6),Random(1,3))))
    {
        strcat(buf,"仓库栏|");
    }
    if((Random(0,100) < 30) && (pUser->AddAttrPackage(2078,1,Random(1,3))))
    {
        strcat(buf,"宠物仓库|");
    }
    if((Random(0,100) < 10) && (pUser->AddBangDingPackage(683)))
    {
        strcat(buf,"血晶石|");
    }
    if((Random(0,100) < 10) && (pUser->AddBangDingPackage(684)))
    {
        strcat(buf,"灵晶石|");
    }
    len = strlen(buf);
    
    for(uint8 i = 0; i < 6; i++)
    {
        uint8 pos = Random(0,num+1);
        if(pos == num)
        {
            exp += (pUser->GetLevel()*10+1000)*Random(3,10);
        }
        else if(pos == num+1)
        {
            daohang += pUser->GetLevel()*10*Random(3,10);
        }
        else
        {
            SItemTemplate *pTmpl = itemMgr.GetItem(items[2*pos+1]);
            uint8 num = Random(1,items[2*pos]);
            bool add = false;
            if(pTmpl != NULL)
            {
                int level = Random(2,3);
                if((items[2*pos+1] >= 640) && (items[2*pos+1] <= 642))
                {
                    for(uint8 j = 0; j < num; j++)
                        add = pUser->AddLevelPackage(items[2*pos+1],level);
                }
                else    
                    add = pUser->AddBangDingPackage(items[2*pos+1],num);
                if(add)
                {
                    sprintf(buf+len,"得到%s×%d|",pTmpl->name.c_str(),num);
                    len = strlen(buf);
                }
            }
        }
    }
    pUser->AddExp(exp);
    pUser->AddDaoHang(daohang);
    sprintf(buf+len,"得到经验%d|",exp);
    len = strlen(buf);
    sprintf(buf+len,"得到道行%d",daohang);
    SendSysInfo1(pUser,buf);
}

void CFight::GiveJiangLi11(CUser *pUser)
{
    /*
    必掉			
经验:人等级*300			
道行:人等级*30			
必掉10件			
仙木LV2	1~5件		640
仙木LV3	1~5件		640
鬼翎LV2	1~5件		641
鬼翎LV3	1~5件		641
兽骨LV2	1~5件		642
兽骨LV3	1~5件		642
师门令	1~5件		1833
礼花	1~5件		1839
双倍修炼令	1件		1139
宠双修炼符	1件		1206
回归符	1件		1121
弱怪灵牌	1件		1207
飞行符	1件		1205
圣水晶	1件		1427
女娲石	1件		1283
绿水晶	1件		1556
强化宝石	1件		1551
水晶合成卷	1件		1451
*/
    uint16 items[] = {
        5,  640 ,
        5,  640 ,
        5,  641 ,
        5,  641 ,
        5,  642 ,
        5,  642 ,
        5,  1833,
        5,  1839,
        1  ,1801,
        1  ,1802,
        1  ,1809,
        1  ,1812,
        1  ,1815,
        1,  612 ,
        1,  630 ,
        1,  622 ,
        1,  610 ,
        1,  616 
    };
    
    
    CItemTemplateManager &itemMgr = SingletonItemManager::instance();
    uint8 num = sizeof(items)/sizeof(items[0])/2;
    
    int exp = pUser->GetLevel() * 300;
    int daohang = pUser->GetLevel() * 30;
    char buf[512];
    int len = 0;
    
    /*
    第四背包2076
    规格：3~6格
    时间：1~3天
    掉落概率：50%
    
    仓库2077
    规格：7~18格
    时间：1~3天
    掉落概率：50%
    
    宠物仓库2078
    规格：2格
    时间：1~3天
    掉落概率：50%
    */
    buf[0] = 0;
    if((Random(0,100) < 50) && (pUser->AddAttrPackage(2076,Random(3,6),Random(1,3))))
    {
        strcat(buf,"得到第四背包|");        
    }
    if((Random(0,100) < 50) && (pUser->AddAttrPackage(2077,Random(7,18),Random(1,3))))
    {
        strcat(buf,"仓库栏|");
    }
    if((Random(0,100) < 50) && (pUser->AddAttrPackage(2078,2,Random(1,3))))
    {
        strcat(buf,"宠物仓库|");
    }
    if((Random(0,100) < 50) && (pUser->AddBangDingPackage(683)))
    {
        strcat(buf,"血晶石|");
    }
    if((Random(0,100) < 50) && (pUser->AddBangDingPackage(684)))
    {
        strcat(buf,"灵晶石|");
    }
    len = strlen(buf);
    
    for(uint8 i = 0; i < 10; i++)
    {
        uint8 pos = Random(0,num+1);
        if(pos == num)
        {
            exp += (pUser->GetLevel()*10+1000)*Random(3,10);
        }
        else if(pos == num+1)
        {
            daohang += pUser->GetLevel()*10*Random(3,10);
        }
        else
        {
            SItemTemplate *pTmpl = itemMgr.GetItem(items[2*pos+1]);
            uint8 num = Random(1,items[2*pos]);
            bool add = false;
            if(pTmpl != NULL)
            {
                int level = Random(2,3);
                if((items[2*pos+1] >= 640) && (items[2*pos+1] <= 642))
                {
                    for(uint8 j = 0; j < num; j++)
                        add = pUser->AddLevelPackage(items[2*pos+1],level);
                }
                else    
                    add = pUser->AddBangDingPackage(items[2*pos+1],num);
                if(add)
                {
                    sprintf(buf+len,"得到%s×%d|",pTmpl->name.c_str(),num);
                    len = strlen(buf);
                }
            }
        }
    }
    pUser->AddExp(exp);
    pUser->AddDaoHang(daohang);
    sprintf(buf+len,"得到经验%d|",exp);
    len = strlen(buf);
    sprintf(buf+len,"得到道行%d",daohang);
    SendSysInfo1(pUser,buf);
    
    const char *info = pUser->GetMission(164);
    if(info == NULL)
        return;
    
    snprintf(buf,sizeof(buf),"%s",info);
    char *p[2];
    num = SplitLine(p,2,buf);
    if(num != 2)
        return;
    num = atoi(p[0]);
    int i = atoi(p[1]);
    if((num == 21) && (i > 3) && (i < 7) && (Random(0,1) == 0))
    {
        if(pUser->AddPackage(1563))
        {
            snprintf(buf,sizeof(buf),"%d|%d",num,i+1);
            pUser->UpdateMission(164,buf);
        }
    }
}

bool CFight::ChuShiFightEnd()
{
    if(m_type != EFTScript)
        return false;
        
    for(uint8 pos = 0; pos < MAX_MEMBER; pos++)
    {
        SMonsterInst *pMonster = GetMonster(pos+1);
        if((pMonster != NULL) && (pMonster->GetCeLue() == CE_HUAN_YING)
            && (pMonster->recovery < MAX_INT) && (GetState(pos+1) == EMSDied))
        {
            return true;
        }
    }
    return false;
}

bool CFight::IsFightEnd()
{
    list<uint32> userList;
    if(m_pScene != NULL)
        m_pScene->GetUserList(userList);
        
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    //cout<<m_fightIsEnd<<endl;
    if(m_fightIsEnd)
        return true;
    if(GetSysTime() - m_beginTime > m_timeOut)
    {
        m_fightIsEnd = true;
        CSocketServer &sock = SingletonSocket::instance();
        CNetMessage msg;
        msg.SetType(PRO_BATTLE_OVER);
        msg<<0<<0;
        for(uint8 pos = 0; pos < MAX_MEMBER; pos++)
        {
            CUser *pUser = GetUser(pos+1);
            if(pUser != NULL)
            {
                pUser->SetFight(0,0);
                if(pUser->IsLogout())
                {
                    if(gpMain != NULL)
                        gpMain->UserLogOut(pUser);
                }
                else
                {
                    sock.SendMsg(pUser->GetSock(),msg);
                    UpdateUserInfo(pUser,userList);
                }
                pUser->UseFightEnd();
            }
        }
        GuiYuEnd();
        NianShouEnd();
        VisibleMonsterMisEnd();
        return true;
    }
    
    uint8 type = OneGroupAllDie();
    if(type != 0)
    {
        m_fightIsEnd = true;
        if(m_type == EFTPlayerPk)
        {
            SendPkInfo(type);
        }
        
        if(m_type == EFTMatch)
            SendMatchInfo(type);
        for(uint8 pos = 0; pos < MAX_MEMBER; pos++)
        {
            UserFightEnd(pos,userList);
        }
        
        GuiYuEnd();
        NianShouEnd();
        VisibleMonsterMisEnd();
        return true;
    }
    return false;
}

//pos 0-11
bool CFight::IsGongFang(uint8 pos)
{
    return pos < GROUP2_BEGIN;
}

void CFight::GetGongFangInfo(uint8 &gongLevel,uint8 &fangLevel,
                         int &gongPkVal,int &fangPkVal,
                         uint8 &gongNum,uint8 &fangNum,
                         int &gongBang,int &fangBang)
{
    gongLevel = 0;
    fangLevel = 0;
    gongPkVal = 0;
    fangPkVal = 0;
    gongNum = 0;
    fangNum = 0;
    gongBang = 0;
    fangBang = 0;
    CUser *pUser;
    for(uint8 i = 1; i <= MAX_MEMBER; i++)
    {
        pUser = GetUser(i);
        if(pUser == NULL)
            continue;
        if(i <= GROUP2_BEGIN)
        {//攻击方
            if(pUser->GetLevel() > gongLevel)
                gongLevel = pUser->GetLevel();
            if(pUser->GetPkVal() >= gongPkVal)
            {
                gongPkVal = GetPkVal(i);//pUser->GetPkVal();
                gongBang = pUser->GetBangPai();
            }
            gongNum++;
        }
        else
        {//防御方
            if(pUser->GetLevel() > fangLevel)
                fangLevel = pUser->GetLevel();
            if(pUser->GetPkVal() >= fangPkVal)
            {
                fangPkVal = GetPkVal(i);//pUser->GetPkVal();
                fangBang = pUser->GetBangPai();
            }
            fangNum++;
        }
    }
}

//isGong true 表示是攻击方，victory true表示胜利，false表示失败
void CFight::GiveJiangCheng(CUser *pUser,bool isGong,bool victory,
                            uint8 gongLevel,uint8 fangLevel,
                            int gongPkVal,int fangPkVal,
                            int gongTolDaoHang,int fangTolDaoHang,
                            stringstream &info)
{
    if(gongPkVal - fangPkVal >= 500)
    {
        if(isGong)
        {
            if(victory)
            {
                info<<"增加魔障:"<<200;
                pUser->AddPkVal(200);
            }
            else
            {
                //int daohang = pUser->GetDaoHang()/5;
                int daohang = pUser->GetDaoHang()/20;
                if(daohang > 0)
                {
                    info<<"减少道行:"<<daohang;
                    pUser->AddDaoHang(-daohang);
                }
            }
        }
        else
        {
            if(victory)
            {
            }
            else
            {
                int daohang = pUser->GetDaoHang()/20;
                if(daohang > 0)
                {
                    info<<"减少道行:"<<daohang;
                    pUser->AddDaoHang(-daohang);
                }
            }
        }
    }
    else if(fangPkVal - gongPkVal >= 500)
    {
        if(isGong)
        {
        }
        else
        {//防御方
            if(!victory)
            {
                //int daohang = pUser->GetDaoHang()/5;
                int daohang = pUser->GetDaoHang()/20;
                if(daohang > 0)
                {
                    info<<"减少道行:"<<daohang;
                    pUser->AddDaoHang(-daohang);
                }
            }
        }
    }
    else if(gongLevel >= fangLevel)
    {
        if(isGong)
        {
            if(victory)
            {
                int pkVal = (gongLevel - fangLevel)*10;
                info<<"增加魔障:"<<pkVal;
                pUser->AddPkVal(pkVal);
            }
            else
            {
                int daohang = (int)(pUser->GetDaoHang()*0.05/(gongLevel - fangLevel+1));
                if(daohang > 0)
                {
                    info<<"减少道行:"<<daohang;
                    pUser->AddDaoHang(-daohang);
                }
            }
        }
        else
        {//防御方
            if(victory)
            {
            }
            else
            {
                int daohang = (int)(pUser->GetDaoHang()*0.05/(gongLevel - fangLevel+1));
                if(daohang > 0)
                {
                    info<<"减少道行:"<<daohang;
                    pUser->AddDaoHang(-daohang);
                }
            }
        }
    }
    else
    {
        if(isGong)
        {
            if(victory)
            {
                info<<"增加魔障:"<<1;
                pUser->AddPkVal(1);
            }
            else
            {
                int daohang = pUser->GetDaoHang()/20;
                if(daohang > 0)
                {
                    info<<"减少道行:"<<daohang;
                    pUser->AddDaoHang(-daohang);
                }
            }
        }
        else
        {//防御方
            if(victory)
            {
            }
            else
            {
                int daohang = (int)(pUser->GetDaoHang()*0.05/(fangLevel-gongLevel+1));
                if(daohang > 0)
                {
                    info<<"减少道行:"<<daohang;
                    pUser->AddDaoHang(-daohang);
                }
            }
        }
    }
}

//战斗结束，对玩家进行奖惩,返回是否需要传送
bool CFight::JiangCheng(uint8 state,CUser *pUser,uint8 pos,int gongTolDaoHang,int fangTolDaoHang)
{
    if(pUser == NULL)
        return false;
    bool needTransport = false;
    stringstream info;
    int srcHp = m_members[pos].srcHp;
    int srcMp = m_members[pos].srcMp;
    
    if((state == 0) && (pUser->GetHp() <= 0))
    {
        pUser->AddHp(1);
    }
    if((pUser->GetLevel() <= 10) && (state == 1) 
        && ((m_type == EFTPlayerPk) || (m_type == EFTMeetMonster)))
    {
        if(pUser->GetHp() <= 0)
        {
            pUser->AddHp(pUser->GetMaxHp());
        }
        return true;
    }
    if((state == 1) && (m_type == EFTScript))
    {
        if(pUser->GetHp() <= 0)
        {
            pUser->AddHp(1);
        }
        return false;
    }
    if((state == 1) && (m_type == EFTMeetMonster))
    {
        /***************
        打怪死亡惩罚：		
        1．	掉落经验（当级升级所需经验的5%）	
        2．	10%概率掉落身上装备中的一件未绑定装备
        *****************/
        pUser->DecreaseAllNaiJiu();
        info<<"所有装备耐久减少10%";
        if(pUser->GetHp() <= 0)
        {
            pUser->AddHp(pUser->GetMaxHp());
        }
        needTransport = true;
        int addExp = GetLevelUpExp(pUser->GetLevel());
        addExp = (int)(addExp*5/100);
        pUser->AddExp(-addExp);
        info<<"|经验减少:"<<addExp;
        if(Random(1,10) == 1)
        {
            string name;
            pUser->DropItem(name);
            if(name.size() > 0)
            {
                info<<"|掉落装备:"<<name;
            }
        }
    }
    if(m_type == EFTPlayerQieCuo)
    {
        if(srcHp > pUser->GetHp())
        {
            pUser->AddHp(srcHp-pUser->GetHp());
            pUser->AddMp(srcMp-pUser->GetMp());
        }
    }
    if(m_type == EFTPlayerPk)
    {
        uint8 gongLevel;
        uint8 fangLevel;
        uint8 gongNum;
        uint8 fangNum;
        int gongPkVal;
        int fangPkVal;
        int bId1,gongB,fangB;
        GetGongFangInfo(gongLevel,fangLevel,gongPkVal,fangPkVal,gongNum,fangNum,
                        gongB,fangB);
        int bId2 = pUser->GetBangPai();
        bool havePunish = true;
        bool isGongFang = IsGongFang(pos);
        if(isGongFang)
            bId1 = fangB;
        else
            bId1 = gongB;
        if((m_pScene != NULL) && ((m_pScene->GetMapId() == 300) || (m_pScene->GetMapId() == 305)))
        {
            havePunish = false;
        }
        else if(!InWorldWar() && (bId1 != 0) && (bId2 != 0))
        {
            CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
            CBangPai *pBangPai1 = bPMgr.FindBangPai(bId1);
            CBangPai *pBangPai2 = bPMgr.FindBangPai(bId2);
            if((pBangPai1 != NULL) && (pBangPai2 != NULL))
            {
                if((pBangPai1->GetXuanZhanBang() == bId2)
                    || (pBangPai2->GetXuanZhanBang() == bId1))
                {
                    if((isGongFang && (abs(pUser->GetLevel() - fangLevel) < 20))
                        || (abs(pUser->GetLevel() - gongLevel) < 20))
                    {
                        if(state == 0)
                        {
                            char buf[128];
                            snprintf(buf,127,"%s为%s帮贡献2繁荣度",pUser->GetName(),pBangPai2->GetName().c_str());
                            SysInfoToAllUser(buf);
                            pBangPai2->SetFanRong(pBangPai2->GetFanRong() + 2);
                        }
                        else
                            pBangPai2->SetFanRong(pBangPai2->GetFanRong() - 1);
                    }
                    havePunish = false;
                }
            }
        }
        if(pos == 8)
        {
            if(TongJiMiss())
                havePunish = true;
        }
        if((m_pScene != NULL) && (m_pScene->InGuiYu()))
        {
            if(state == 1)
                needTransport = true;
        }
        else if((state == 1) && (m_pScene != NULL) 
                && ((m_pScene->GetMapId() == 300) || (m_pScene->GetMapId() == 305)))
        {
            needTransport = true;
        }
        if(havePunish)
        {
            if(state == 1)
            {//PK死亡
                if(m_visibleMonsterId == 0xffff)
                {
                    pUser->DelPackageById(1558,-1);
                    pUser->DelPackageById(1559,-1);
                    pUser->DelPackageById(1560,-1);
                    pUser->DelPackageById(1561,-1);
                    pUser->DelPackageById(1562,-1);
                    m_mutex.unlock();
                    CScene *pScene = pUser->GetScene();
                    if(pScene != NULL)
                        pScene->TempLeaveTeam(pUser);
                    if(state == 1)
                        TransportUser(pUser,58,21,12,8);
                    m_mutex.lock();
                }
                else
                {
                    pUser->DecreaseAllNaiJiu();
                    info<<"所有装备耐久减少10%";
                    if(pUser->GetHp() <= 0)
                    {
                        pUser->AddHp(pUser->GetMaxHp());
                    }
                    needTransport = true;
                    if(Random(1,10) == 1)
                    {
                        string name;
                        pUser->DropItem(name);
                        if(name.size() > 0)
                        {
                            info<<"|掉落装备:"<<name<<"|";
                        }
                    }
                    int addExp = GetLevelUpExp(pUser->GetLevel());
                    addExp = (int)(addExp*5/100);
                    pUser->AddExp(-addExp);
                    info<<"|经验减少:"<<addExp;
                    
                    if((fangNum > 0) && (gongNum > 0) && havePunish)
                    {
                        GiveJiangCheng(pUser,isGongFang,false,
                                    gongLevel,fangLevel,
                                    gongPkVal,fangPkVal,
                                    gongTolDaoHang/fangNum,fangTolDaoHang/gongNum,info);
                    }
                }
            }
            else if(state == 0)
            {//PK胜利
                if((m_visibleMonsterId == 0xffff) && ((int)pUser->GetBangPai() == WWGetWinBang()))
                {
                    //1.杀死50~59级玩家最少获得3点积分。2.杀死60~69级玩家最少获得5点积分。3.杀死70级以上玩家最少获得10点积分。
                    uint8 me = pos+1;
                    int jifen = 0;
                    int level1 = 0;
                    int level2 = 0;
                    uint8 i = 0;
                    if(me <= GROUP2_BEGIN)
                    {
                        for(i = GROUP2_BEGIN; i < MAX_MEMBER; i++)
                        {
                            CUser *pU = GetUser(i+1);
                            if(pU != NULL)
                            {
                                level1 = pUser->GetLevel();
                                level2 = pU->GetLevel();
                                if(abs(level1 - level2) <= 5)
                                    jifen += 5;
                                else if(level2 - level1 > 10)
                                    jifen += 10;
                                else if(level2 - level1 > 5)
                                    jifen += 7;                            
                                else 
                                    jifen += 1;
                            }
                            if((jifen < 3) && (level2 >= 50) && (level2 <= 59))
                            {
                                jifen = 3;
                            }
                            if((jifen < 5) && (level2 >= 60) && (level2 <= 69))
                            {
                                jifen = 5;
                            }
                            if((jifen < 10) && (level2 >= 70))
                            {
                                jifen = 10;
                            }
                        }
                    }
                    else
                    {
                        for(i = 0; i < GROUP2_BEGIN; i++)
                        {
                            CUser *pU = GetUser(i+1);
                            if(pU != NULL)
                            {
                                level1 = pUser->GetLevel();
                                level2 = pU->GetLevel();
                                if(abs(level1 - level2) <= 5)
                                    jifen += 5;
                                else if(level2 - level1 > 10)
                                    jifen += 10;
                                else if(level2 - level1 > 5)
                                    jifen += 7;                            
                                else 
                                    jifen += 1;
                            }
                        }
                    }
                    bool have100 = true;
                    bool have300 = true;
                    if(pUser->GetData16(3) > 100)
                        have100 = false;
                    if(pUser->GetData16(3) > 300)
                        have300 = false;
                    pUser->SetData16(3,pUser->GetData16(3)+jifen);
                    if((have100 && (pUser->GetData16(3) >= 100)) || (have300 && (pUser->GetData16(3) >= 300)))
                    {
                        char buf[256];
                        snprintf(buf,256,"%s英勇杀敌,守护资源,已经取得%d积分。",pUser->GetName(),pUser->GetData16(3));
                        SysInfoToAllUser(buf);
                    }
                    AddTolJiFen(jifen);
                }
                else if(m_visibleMonsterId != 0xffff)
                {
                    bool punish = false;
                    for(uint8 i = 1; i <= MAX_MEMBER; i++)
                    {
                        if(GetState(i) == EMSDied)
                        {
                            punish = true;
                            break;
                        }
                    }
                    if(punish)
                    {
                        if((fangNum > 0) && (gongNum > 0) && havePunish)
                        {
                            GiveJiangCheng(pUser,isGongFang,true,
                                        gongLevel,fangLevel,
                                        gongPkVal,fangPkVal,
                                        gongTolDaoHang/fangNum,fangTolDaoHang/gongNum,info);
                        }
                    }
                }
            }
        }
    }
    if(info.str().size() > 0)
        SendPopMsg(pUser,info.str().c_str());
    return needTransport;
}

void CFight::DecreaseMp(uint8 pos,int mp)
{
    CUser *pUser = GetUser(pos);
    SMonsterInst *pMonster = GetMonster(pos);
    SPet *pPet = GetPet(pos);
    if(pUser != NULL)
    {
        pUser->AddMp(-mp);
    }
    else if(pMonster != NULL)
    {
        pMonster->mp -= mp;
        if(pMonster->mp < 0)
        {
            pMonster->mp = 0;
        }
    }
    else if(pPet != NULL)
    {
        pPet->mp -= mp;
        if(pPet->mp < 0)
        {
            pPet->mp = 0;
        }
    }
}

void CFight::DecreaseHp(uint8 pos,int hp,uint8 src)
{
    if(GetState(pos) >= EMSDied)
        return;
        
    CUser *pUser = GetUser(pos);
    SMonsterInst *pMonster = GetMonster(pos);
    SPet *pPet = GetPet(pos);
    if(pUser != NULL)
    {
        pUser->AddHp(-hp);
        if(pUser->GetHp() <= 0)
        {
            //pUser->SetHp(pUser->GetMaxHp());
            //DelMember(pos);
            SetState(pos,EMSDied);
            pUser->AddHp(1);
        }
    }
    else if(pMonster != NULL)
    {
        pMonster->hp -= hp;
        if(pMonster->hp <= 0)
        {
            pMonster->hp = 0;
            SetState(pos,EMSDied);
            if(pMonster->recovery == MAX_INT)
                m_useZhaoHuanSkill = false;
            //uint16 celue = pMonster->GetCeLue(); 
            if(pMonster->tmplId == 20)
            {
                CUser *pUser = GetUser(src);
                if(pUser == NULL)
                    pUser = GetUser(src-1);
                if(pUser == NULL)
                    return;
                const char *info = pUser->GetMission(20);
                if(info != NULL)
                {
                    char buf[32];
                    snprintf(buf,sizeof(buf),"%s",info);
                    char *p[2];
                    int num = SplitLine(p,2,buf);
                    if(num == 2)
                    {
                        num = atoi(p[1]);
                        int i = atoi(p[0]);
                        if((atoi(p[0]) == 9) && (atoi(p[1]) < 10000))
                        {
                            //葫芦杀死毒蛾num+5 不用葫芦num+1 杀死毒蛾大仙num+500,不用葫芦杀死毒蛾大仙num+100
                            SPet *pPet = GetPet(src);
                            int add = 0;
                            if((pPet != NULL) && (pPet->tmplId == 100))
                            {
                                if(pMonster->GetCeLue() == CL_DU_E_DA_XIAN)
                                    add = 500;
                                else 
                                    add = 5;
                            }
                            if(add == 0)
                            {
                                if(pMonster->GetCeLue() == CL_DU_E_DA_XIAN)
                                    add = 100;
                                else 
                                    add = 1;
                            }
                            if(pUser != NULL)
                            {
                                num += add;
                                if(num > 10000)
                                    num = 10000;
                                snprintf(buf,sizeof(buf),"%d|%d",i,num);
                                pUser->UpdateMission(20,buf);
                            }
                        }
                    }
                }
            }
            SPet *pPet = GetPet(src);
            if((pPet != NULL) && (pPet->tmplId == 100))
            {
                CUser *pUser = GetUser(src - 1);
                if(pUser == NULL)
                    return;
                const char *info = pUser->GetMission(17);
                if(info != NULL)
                {
                    char buf[32];
                    snprintf(buf,sizeof(buf),"%s",info);
                    char *p[2];
                    int num = SplitLine(p,2,buf);
                    if(num == 2)
                    {
                        num = atoi(p[1]);
                        int i = atoi(p[0]);
                        if((num<1000) && (i == 10) && (abs(pMonster->level-pPet->level) <= 10))
                        {
                            num++;
                            snprintf(buf,sizeof(buf),"%d|%d",i,num);
                            pUser->UpdateMission(17,buf);
                        }
                    }
                }
            }
            if((pMonster->GetCeLue() == CL_ONLY_LANG) 
                && (pPet != NULL) && (pPet->tmplId == 101))
            {
                CUser *pUser = GetUser(src - 1);
                if(pUser == NULL)
                    return;
                const char *info = pUser->GetMission(20);
                if(info != NULL)
                {
                    char buf[32];
                    snprintf(buf,sizeof(buf),"%s",info);
                    char *p[3];
                    int num = SplitLine(p,3,buf);
                    if(num == 3)
                    {
                        num = atoi(p[1]);
                        int i = atoi(p[0]);
                        int j = atoi(p[2]);
                        if((num<30) && (i == 15))
                        {
                            num++;
                            snprintf(buf,sizeof(buf),"%d|%d|%d",i,num,j);
                            pUser->UpdateMission(20,buf);
                        }
                    }
                }
            }
        }
        else if(pMonster->hp > pMonster->maxHp)
        {
            pMonster->hp = pMonster->maxHp;
        }
    }
    else if(pPet != NULL)
    {
        pPet->hp -= hp;
        CUser *pUser = GetUser(pos - 1);
        if(pUser != NULL)
        {
            pUser->DesKaiJiaNaijiu(2);
        }
        if(pPet->hp <= 0)
        {
            pPet->hp = 1;//pPet->maxHp;
            if(pUser != NULL)
            {
                pUser->UpdatePetInfo(pUser->GetChuZhanPet(),5,pPet->hp);
            }
            SetState(pos,EMSDied);
        }
        else if(pPet->hp > pPet->maxHp)
        {
            pPet->hp = pPet->maxHp;
        }
    }
}

void CFight::ImproveRevovery(uint8 pos,int recovery)
{
    CUser *pUser = GetUser(pos);
    SMonsterInst *pMonster = GetMonster(pos);
    if(pUser != NULL)
    {
        pUser->AddRecovery(recovery);
    }
    else if(pMonster != NULL)
    {
        pMonster->recovery -= recovery;
    }
}

bool CFight::AllUserOption()
{
    for(uint8 pos = 0; pos < MAX_MEMBER; pos++)
    {
        if(!m_members[pos].memPtr.empty()
            && (m_members[pos].memPtr.type() == typeid(ShareUserPtr)))
            //&& (m_members[pos].state < EMSDied))
        {
            CUser *pUser = (boost::any_cast<ShareUserPtr>(m_members[pos].memPtr)).get();
            if((pUser != NULL) && (!pUser->IsAutoFight()))
            {
                if(m_members[pos].state >= EMSDied)
                {//人物死亡
                    if(GetPet(pos+2) == NULL) 
                        continue;
                    //有宠物
                    if(GetState(pos+2) >= EMSDied)
                    //宠物死亡
                        continue;
                    if(!m_members[pos].select)
                    {//宠物没死，并且没有选择
                        return false;
                    }
                    else
                    {
                        continue;
                    }
                }
                if(m_members[pos].select)
                    continue;
                if((m_members[pos].state == EMSFrost) || (m_members[pos].state == EMSConfusion))
                {
                    if((GetPet(pos+2) != NULL) && (GetState(pos+2) < EMSDied))
                    {
                        return false;
                    }
                    continue;
                }
                return false;
            }
        }
    }
    return true;
}

int CFight::GetMonsterNum(int id)
{
    //boost::recursive_mutex::scoped_lock lk(m_mutex);
    int num = 0;
    for(uint8 pos = 0; pos < MAX_MEMBER; pos++)
    {
        SMonsterInst *pMonster = GetMonster(pos+1);
        if((pMonster != NULL) && (pMonster->tmplId == id))
        {
            num++;
        }
    }
    return num;
}

void CFight::TurnOver(uint8 pos)
{
    m_userOpTime = 0;//GetSysTime();;
    if((pos > 0) && (pos <= MAX_MEMBER))
    {
        SMonsterInst *pMonster = GetMonster(pos);
        if(pMonster != NULL)
        {
            uint16 ceLue = pMonster->GetCeLue();
            if((ceLue == FIRST_HULUO) || (ceLue == CL_HL_DIE_END))
            {
                for(uint8 i = 1; i <= MAX_MEMBER; i++)
                {
                    SPet *pPet = GetPet(i);
                    if((pPet != NULL) && (pPet->tmplId == 100))
                    {
                        if(GetState(i) >= EMSDied)
                        {
                            SetAllUserDie();
                            break;
                        }
                    }
                }
            }
        }
        if(pMonster != NULL)
        {
            uint16 ceLue = pMonster->GetCeLue();
            if(ceLue == CL_MONSTER_DIE_END)
            {
                if(GetState(pos) >= EMSDied)
                {
                    SetAllUserDie();
                }
            }
            else if(ceLue == CL_MEI_HAN)
            {
                for(uint8 i = 1; i <= MAX_MEMBER; i++)
                {
                    SMonsterInst *pM = GetMonster(i);
                    if((pM != NULL) && (pM->tmplId == 101))
                    {
                        if(GetState(i) >= EMSDied)
                        {
                            SetAllUserDie();
                            break;
                        }
                    }
                }
            }
            else if(ceLue == CL_SHUYAO)
            {
                uint16 monsterId = 100;
                for(uint8 i = 1; i <= MAX_MEMBER; i++)
                {
                    SMonsterInst *pMonster = GetMonster(i);
                    if((pMonster != NULL) && (pMonster->tmplId == monsterId))
                    {
                        if(GetState(i) >= EMSDied)
                        {
                            SetAllUserDie();
                            break;
                        }
                    }
                }
            }
        }
        
        if(m_members[pos-1].state >= EMSDied)
        {
            CUser *pUser = GetUser(pos);
            if(pUser != NULL)
            {
                if(pUser->IsAutoFight())
                {
                    m_members[pos-1].select = true;
                    int turn = pUser->GetAutoFightTurn();
                    if(--turn <= 0)
                    {
                        pUser->SetAutoFightTurn(0);
                        CNetMessage msg;
                        msg.SetType(PRO_BATTLE);
                        msg<<(uint8)3;//通知客户端取消战斗
                        CSocketServer &sock = SingletonSocket::instance();
                        sock.SendMsg(pUser->GetSock(),msg);
                    }
                    else
                    {
                        pUser->SetAutoFightTurn(turn);
                    }
                }
                else
                {
                    m_members[pos-1].select = false;
                }
            }
            return;
        }
        m_members[pos-1].protecter = 0;
        m_members[pos-1].recoveryImporve = 0;
        CUser *pUser = GetUser(pos);
        if(pUser != NULL)
        {
            if((m_members[pos-1].turnBegin != 0)
                && (GetSysTime() - m_members[pos-1].turnBegin < 3))
            {
                //shutdown(pUser->GetSock(),SHUT_RD);
                //return;
                SendPopMsg(pUser,"你手脚过快，惹怒了星灵仙子，她不高兴了。快去星寿村向她道歉吧!");
                pUser->SetBitSet(0);
            }
            //cout<<GetSysTime() - m_members[pos-1].turnBegin<<endl;
            m_members[pos-1].turnBegin = GetSysTime();
            if(pUser->IsAutoFight())
            {
                m_members[pos-1].select = true;
                int turn = pUser->GetAutoFightTurn();
                if(--turn <= 0)
                {
                    pUser->SetAutoFightTurn(0);
                    CNetMessage msg;
                    msg.SetType(PRO_BATTLE);
                    msg<<(uint8)3;//通知客户端取消战斗
                    CSocketServer &sock = SingletonSocket::instance();
                    sock.SendMsg(pUser->GetSock(),msg);
                }
                else
                {
                    pUser->SetAutoFightTurn(turn);
                }
            }
            else if(m_members[pos-1].useSkill23)
            {
                m_members[pos-1].select = true;
            }
            else
            {
                m_members[pos-1].select = false;
            }
        }
        
        if(m_members[pos-1].stateTurn > 0)
            m_members[pos-1].stateTurn--;
        if(m_members[pos-1].stateTurn <= 0)
            m_members[pos-1].state = 0;   
                     
        if(m_members[pos-1].damageTrun != 0)
        {
            if(--m_members[pos-1].damageTrun == 0)
            {
                m_members[pos-1].damageImprove = 0;
            }
        }
            
        if(m_members[pos-1].hpTurn != 0)
        {
            if(--m_members[pos-1].hpTurn == 0)
            {
                CUser *pUser = GetUser(pos);
                SPet *pPet = GetPet(pos);
                SMonsterInst *pMonster = GetMonster(pos);
                if(pUser != NULL)
                {
                    pUser->SetMaxHp(pUser->GetMaxHp()-m_members[pos-1].hpImprove);
                    if(pUser->GetHp() > pUser->GetMaxHp())
                        pUser->SetHp(pUser->GetMaxHp());
                }
                else if(pPet != NULL)
                {
                    pPet->maxHp -= m_members[pos-1].hpImprove;
                    if(pPet->hp > pPet->maxHp)
                        pPet->hp = pPet->maxHp;
                }
                else if(pMonster != NULL)
                {
                    pMonster->maxHp -= m_members[pos-1].hpImprove;
                    if(pMonster->hp > pMonster->maxHp)
                        pMonster->hp = pMonster->maxHp;
                }
                m_members[pos-1].hpImprove = 0;
            }
        }
            
        if(m_members[pos-1].recoveryTurn != 0)
        {
            if(--m_members[pos-1].recoveryTurn == 0)
            {
                ImproveRevovery(pos,-m_members[pos-1].recoveryImporve);
                m_members[pos-1].recoveryImporve = 0;
            }
        }
        if(m_members[pos-1].speedTurn != 0)
        {
            if(--m_members[pos-1].speedTurn == 0)
            {
                m_members[pos-1].speedImprove = 0;
            }
        }
        if(m_members[pos-1].dodgeTurn != 0)
        {
            if(--m_members[pos-1].dodgeTurn == 0)
            {
                m_members[pos-1].dodgeImprove = 0;
            }
        }
        if(m_members[pos-1].normalDamageTurn != 0)
        {
            if(--m_members[pos-1].normalDamageTurn == 0)
            {
                m_members[pos-1].normalDamageImprove = 0;
            }
        }
    }
}

int CFight::CalculateSkillDamage(uint8 src,uint8 target,int skillId)
{
    int damage = 0;
    int recovery = 0;
    CUser *pUser = GetUser(src);
    SMonsterInst *pMonster = GetMonster(src);
    SPet *pPet = GetPet(src);
    uint8 srcXiang = 0;
    uint8 targetXiang = 0;
    
    //cout<<endl<<"src:"<<(int)src<<" tar:"<<(int)target<<endl;
    if(pUser != NULL)
    {
        damage = pUser->GetSkillDamage(skillId);
        //cout<<pUser->GetName()<<"伤害:"<<damage<<endl;
        srcXiang = pUser->GetXiang();
    }
    else if(pMonster != NULL)
    {
        damage = pMonster->GetSkillDamage(skillId);
        srcXiang = pMonster->xiang;
    }
    else if(pPet != NULL)
    {
        damage = pPet->GetSkillDamage(skillId);

        srcXiang = pPet->xiang;
    }
    if(GetDamageImprove(src) > 0)
        damage += GetDamageImprove(src);
    pUser = GetUser(target);
    pMonster = GetMonster(target);
    pPet = GetPet(target);
    if(pUser != NULL)
    {
        recovery = pUser->GetRecovery();
        uint8 skillXiang = GetSkillXiang(skillId);

        switch(skillXiang)
        {
        case EXJinXiang:
            recovery += pUser->GetItemKangJin();
            break;
        case EXMuXiang:
            recovery += pUser->GetItemKangMu();
            break;
        case EXShuiXiang:
            recovery += pUser->GetItemKangShui();
            break;
        case EXHuoXiang:
            recovery += pUser->GetItemKangHuo();
            break;
        case EXTuXiang:
            recovery += pUser->GetItemKangTu();
            break;
        }
        targetXiang = pUser->GetXiang();
        //cout<<pUser->GetName()<<"防御:"<<recovery<<endl;
    }
    else if(pMonster != NULL)
    {
        recovery = pMonster->recovery;
        if(pMonster->GetCeLue() == CE_LUE_BA_XIA)
        {
            recovery += 300;
        }
        else if(pMonster->GetCeLue() == CE_LUE_QI_LING)
        {
            recovery += 500;
        }
        else if(pMonster->GetCeLue() == CL_LAN_RUO_BOS)
        {
            recovery += 2000;
        }
        else if(CL_LAN_RUO_BOS_FU == pMonster->GetCeLue())
        {
            recovery += 2000;
        }
        else if(CL_LAN_RUO_LAOLAO == pMonster->GetCeLue())
        {
            recovery += 3000;
        }
        recovery += pMonster->quanKang;
        targetXiang = pMonster->xiang;
    }
    else if(pPet != NULL)
    {
        recovery = pPet->GetRecovery();
        targetXiang = pPet->xiang;
    }
    damage = damage - recovery;
    //金克木，木克土，土克水，水克火，火克金
    if(((srcXiang == EXJinXiang) && (targetXiang == EXMuXiang))
        || ((srcXiang == EXMuXiang) && (targetXiang == EXTuXiang))
        || ((srcXiang == EXTuXiang) && (targetXiang == EXShuiXiang))
        || ((srcXiang == EXShuiXiang) && (targetXiang == EXHuoXiang))
        || ((srcXiang == EXHuoXiang) && (targetXiang == EXJinXiang)))
    {
        damage = (int)(damage * 1.4);
    }
        
    if(damage <= 0)
        damage = 1;
    int add = CalculateRate(damage,4,100);
    add = CalculateRate(add,Random(0,100),100);
    damage += add;
    
    if(GetRecoveryImporve(target) != 0)
        damage /= 2;
    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo) || (m_type == EFTMatch))
        damage /= 2;
    if(damage < 1)
        damage = 1;
    //cout<<"法术伤害:"<<damage<<endl;
    return damage;
}

int CFight::GetMaxHp(uint8 pos)
{
    if((pos < 1) || (pos > MAX_MEMBER))
        return 0;
    if(m_members[pos-1].memPtr.empty())
        return 0;
    if(m_members[pos-1].memPtr.type() == typeid(ShareUserPtr))
    {
        CUser *pUser = (boost::any_cast<ShareUserPtr>(m_members[pos-1].memPtr)).get();
        return pUser->GetMaxHp();
    }
    else if(m_members[pos-1].memPtr.type() == typeid(ShareMonsterPtr))
    {
        SMonsterInst *pMonster = (boost::any_cast<ShareMonsterPtr>(m_members[pos-1].memPtr)).get();
        return pMonster->maxHp;
    }
    else if(m_members[pos-1].memPtr.type() == typeid(SharePetPtr))
    {
        SPet *pPet = (boost::any_cast<SharePetPtr>(m_members[pos-1].memPtr)).get();
        return pPet->maxHp;
    }
    return 0;
}

static bool FaShuBaoJi(CUser *pUser,int &add,SMonsterInst *pMonster = NULL)
{
    int gailv = 0;
    if(pUser == NULL)
    {
        if(pMonster != NULL) 
        {
            uint8 fashubaojilv = pMonster->fashubaojilv;
            if(fashubaojilv > 0)
            {
                if(Random(0,100) <= fashubaojilv)
                {
                    add = pMonster->fashubaojiadd;
                    return true;
                }
                return false;
            }
            if(pMonster->GetCeLue() == CE_FANZHENG)
            {
                if(Random(0,4) == 0)
                {
                    add = 500;
                    return true;
                }
            }
            else if(pMonster->GetCeLue() == CL_LAN_RUO_BOS)
            {
                gailv = 300/5;
                if(Random(0,100) < gailv)
                {
                    add = 1000;
                    return true;
                }
            }
        }
        return false;
    }
    gailv = pUser->GetFaShuBaoJi()/5;
    if(Random(0,100) < gailv)
    {
        add = pUser->GetFaShuBaoJiAdd();
        return true;
    }
    
    return false;
}

//法术反弹百分比
static int FaShuFanTan(CUser *pUser,SMonsterInst *pMonster = NULL)
{
    if(pUser == NULL)
    {
        if(pMonster != NULL)
        {
            if((pMonster->GetCeLue() == CE_LUE_CHAO_FENG)
                || (pMonster->GetCeLue() == CE_LUE_QI_LING))
            {
                if(Random(0,100) <= 20)
                    return 20;
                return 0;
            }
            //else if(pMonster->GetSkillLevel(57) > 0)
            else if(pMonster->GetCeLue() == CE_FANZHENG)
            {
                if(Random(0,4) == 0)
                    return 50;
            }
            else if(pMonster->GetCeLue() == CL_LAN_RUO_LAOLAO)
            {
                return 10;
            }
            if(pMonster->faShuFanTanLv > 0)
            {
                if(Random(0,100) <= pMonster->faShuFanTanLv)
                {
                    return pMonster->faShuFanTan;
                }
            }
        }
        return 0;
    }
    int gailv = pUser->GetFaShuFanTanLv()/5;
    
    if(Random(0,100) < gailv)
    {
        return pUser->GetFaShuFanTan();
    }
    return 0;
}

int CFight::GetTargetDaoHang(uint8 target)
{
    CUser *pUser = GetUser(target);
    int targetDaohang = 0;
    if(pUser != NULL)
    {
        //pUser->DecreaseFangJuNaiJiu();
        targetDaohang = pUser->GetDaoHang();
    }
    else
    {
        SMonsterInst *pMonster = GetMonster(target);
        if(pMonster != NULL)
        {
            //怪物道行=等级*等级*等级/775
            targetDaohang = pMonster->daohang;//pMonster->level*pMonster->level*pMonster->level/775;
            //cout<<targetDaohang<<endl;
        }
        else
        {
            SPet *pPet = GetPet(target);
            if(pPet != NULL)
                targetDaohang = pPet->wuxue;
        }
    }
    if(targetDaohang == 0)
        return 1;
    //cout<<"目标道行"<<targetDaohang<<endl;
    return targetDaohang;
}

int CFight::SkillUseMp(uint8 pos,uint8 skillId,uint8 skillLevel,
                        uint8 skillType,uint8 skillJie)
{
    int useMp = 0;
    //int petMp = 0;
    
    if(skillType == ESTDamageHp)
    {//咒术 
        /*
        1阶：技能等级*6+25
        2阶：技能等级*18+60
        3阶：技能等级*30+90
        4阶：技能等级*20+120
        */
        //2阶：技能等级*14+26
        //3阶：技能等级*19+76
        switch(skillJie)
        {
        case 1:
            useMp = (int)(skillLevel*6+25);
            break;
        case 2:
            useMp = (int)(skillLevel*14+26);
            break;
        case 3:
            useMp = (int)(skillLevel*20+76);
            break;
        case 4:
            useMp = (int)(skillLevel*20+120);
            break;
        }
    }
    else if(skillType <= ESTFrost)
    {//负面    
        /*负面技能,辅助技能
        1阶：MP消耗公式：技能等级*15+100
        2阶：MP消耗公式：技能等级*35+90
        3阶：MP消耗公式：技能等级*65+90
        4阶：MP消耗公式：技能等级*40+240*/
        switch(skillJie)
        {
        case 1:
            useMp = (int)(skillLevel*15+100);
            break;
        case 2:
            useMp = (int)(skillLevel*35+90);
            break;
        case 3:
            useMp = (int)(skillLevel*65+90);
            break;
        case 4:
            useMp = (int)(skillLevel*40+240);
            break;
        } 
    }
    else if(skillType <= ESTImproveRecovery)
    {//辅助系：
        switch(skillJie)
        {
        case 1:
            useMp = (int)(skillLevel*15+100);
            break;
        case 2:
            useMp = (int)(skillLevel*35+90);
            break;
        case 3:
            useMp = (int)(skillLevel*65+90);
            break;
        case 4:
            useMp = (int)(skillLevel*40+240);
            break;
        }
    }
    else 
    {
        //技能等级*技能等级/10+1
        useMp = (int)(skillLevel*skillLevel/3.5+300);
        /*switch(skillJie)
        {
        case 1:
            useMp = (int)(skillLevel*10+50);
            break;
        case 2:
            useMp = (int)(skillLevel*25+45);
            break;
        case 3:
            useMp = (int)(skillLevel*55+45);
            break;
        case 4:
            useMp = (int)(skillLevel*30+120);
            break;
        }*/
    }
    CUser *pUser = GetUser(pos);
    SMonsterInst *pMonster = GetMonster(pos);
    SPet *pPet = GetPet(pos);
    if(pUser != NULL)
    {
        if(pUser->GetMp() < useMp)
            return -1;
    }
    else if(pMonster != NULL)
    {
        return 0;
        //if(pMonster->mp < useMp)
            //return -1;
    }
    else if(pPet != NULL)
    {
        if(pPet->mp < useMp)
            return -1;
    }
    DecreaseMp(pos,useMp);
    return useMp;
}
/*
static void GetQianKunInfo(uint8 skillLevel,uint8 &turn,uint8 &times,uint16 &damage)
{
    if(skillLevel <= 1)
    {
        turn = 2;
        times = 1;
        damage = 20000;
    }
    else if(skillLevel <= 20)
    {
        turn = 2;
        times = 2;
        damage = 21000;
    }
    else if(skillLevel <= 40)
    {
        turn = 3;
        times = 2;
        damage = 26000;
    }
    else if(skillLevel <= 60)
    {
        turn = 3;
        times = 3;
        damage = 33000;
    }
    else if(skillLevel <= 80)
    {
        turn = 4;
        times = 3;
        damage = 43000;
    }
    else if(skillLevel <= 100)
    {
        turn = 4;
        times = 4;
        damage = 55000;
    }
    else if(skillLevel <= 120)
    {
        turn = 5;
        times = 4;
        damage = 70000;
    }
    else if(skillLevel <= 140)
    {
        turn = 5;
        times = 5;
        damage = 90000;
    }
    else if(skillLevel <= 160)
    {
        turn = 6;
        times = 5;
        damage = 110000;
    }
    else
    {
        turn = 6;
        times = 6;
        damage = 135000;
    }
}
*/

//负面技能命中率： （基础命中率+技能等级*0.0017）*（攻方道行/防方道行），封顶值90%，最小值10%
static bool FuMianSkillMiss(int skillId,int skillLevel,int gongDaoHang,int fangDaoHang)
{
    int base = 0;
    switch(GetSkillJie(skillId))
    {
    case 1:
        base = 20;
        break;
    case 2:
        base = 10;
        break;
    case 3:
        base = 20;
        break;
    case 4:
        base = 40;
        break;
    }
    if(fangDaoHang <= 0)
        fangDaoHang = 1;
    int gailv = (int)((base + skillLevel * 0.17)*gongDaoHang/fangDaoHang);
    //cout<<gailv<<endl;
    if(gailv > 90)
        gailv = 90;
    else if(gailv < 10)
        gailv = 10;
    cout<<gailv<<endl;
    return Random(0,100) > gailv;
}

uint8 CFight::UseSkill121(CNetMessage &msg,CUser *pUser,int skillLevel,uint8 src,uint8 target)
{
    /*
    +-------+-----+--------+-----+-------+------+---------+-------+------+
    | SKILL | NUM | TARGET | RES | BAOJI | RESV | RESPOND | RESV2 | MANA |
    +-------+-----|--------+-----+-------+------+---------+-------+------+
    |   1   |  1  |   1    |  1  |   1   |  4   |    1    |   4   |   4  |
    +-------+-----+--------|-----+-------+------+---------+-------+------+
    SKILL 技能ID
    TARGET 目标ID 
    RES 技能施加到该对应目标的效果
    BAOJI 是否法术爆击
    RESV 技能施加到该对应目标的效果值
    RESV2 中毒效果
    医疗术 RESV2 加上限
    咒术 RESV2 宠物保护 
    还魂术 RESV2 本体减少的血
    */
    //技能等级*80+120
    int useMp = pUser->GetSrcSkillLevel(121)*80+120;
    if(useMp > pUser->GetMp())
        return 0;
    DecreaseMp(src,useMp);
    
    CUser *pTarUser = GetUser(target);
    SMonsterInst *pTarMonster = GetMonster(target);
    SPet *pTarPet = GetPet(target);
    int tarLevel = 0;
    if(pTarUser != NULL)
    {
        tarLevel = pTarUser->GetLevel();
    }
    else if(pTarMonster != NULL)
    {
        tarLevel = pTarMonster->level;
    }
    else if(pTarPet != NULL)
    {
        tarLevel = pTarPet->level;
    }
    
    if(skillLevel < tarLevel)
    {
        return 0;
    }
    int addHp = 0;
    int decHp = 0;
    decHp = CalculateRate(pUser->GetMaxHp(),60,100);
    DecreaseHp(src,decHp);
    if(GetState(target) == EMSDied)
    {
        addHp = (int)(200*(1+skillLevel*0.1));
        DecreaseHp(target,-addHp,src);
        SetState(target,0);
    }
    else
    {
        addHp = (int)(400*(1+skillLevel*0.1));
        DecreaseHp(target,-addHp,src);
        SetState(target,0);
    }
    msg<<src<<(uint8)EOTSkill<<(uint8)121<<(uint8)1<<target<<PRO_SUCCESS
        <<(uint8)0<<addHp<<(uint8)0<<decHp<<useMp;
    return 1;
}

uint8 CFight::UseSkill23(CNetMessage &msg,CUser *pUser,int skillLevel,uint8 src,uint8 target)
{
    if(!m_members[src-1].useSkill23)
    {//第一轮使用
        //技能等级*18+22
        int useMp = (int)(pUser->GetSrcSkillLevel(23)*18+22);
        if(useMp > pUser->GetMp())
            return 0;
        DecreaseMp(src,useMp);
        msg<<src<<(uint8)12<<useMp;
        m_members[src-1].useSkill23 = true;
        SetSpeedImprove(src,pUser->GetSkillLevel(23)*2);
        SetSpeedTurn(src,1);
        return 1;
    }
    //第二轮使用
    m_members[src-1].useSkill23 = false;
    
    int damage = 0;
    uint8 tarState = GetState(target);
    int hitRatio = CalculateHitRatio(src,target);
    if(Random(0,100) > hitRatio)
    {
        msg<<src<<(uint8)EOTSkill<<(uint8)23<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<damage
            <<(uint8)0<<0<<0<<0;
        return 1;
    }
    
    if((tarState == EMSFrost) || (skillLevel <= 0)
        || (tarState == EMSqiankunzhao) || (tarState == EMSshenlongzhao))
    {
        if(tarState == EMSqiankunzhao)
            SetState(target,0);
        msg<<src<<(uint8)EOTSkill<<(uint8)23<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<damage
            <<(uint8)0<<0<<0<<0;
        return 1;
    }
    
    int baojiAdd = 0;
    int baojidu = GetBaoJi(src,baojiAdd,50);
    
    //(木相*25+120*技能等级/3)* (1.8+狂暴撕裂附加值) 
    //(木相*25+120*技能等级/3)* (1.6+狂暴撕裂附加值) 
    //(木相*25+120*技能等级/3)* (0.5+技能等级/100) 
    damage = (int)((pUser->GetMu()*25+120*skillLevel/3.0)*(skillLevel/100.0+0.5))
            + pUser->GetDamage();
    damage += m_members[src-1].normalDamageImprove;// = imporveDamage; 
    
    CUser *pTarUser = GetUser(target);
    SMonsterInst *pTarMonster = GetMonster(target);
    SPet *pTarPet = GetPet(target);
    
    if((pTarMonster != NULL) && (pTarMonster->noAdd))
    {
        baojidu = 0;
    }
    int recovery = 0;
    uint8 targetXiang = 0;
    if(pTarUser != NULL)
    {
        recovery = pTarUser->GetRecovery();
        targetXiang = pTarUser->GetXiang();
    }
    else if(pTarMonster != NULL)
    {
        recovery = pTarMonster->recovery;
        targetXiang = pTarMonster->xiang;
    }
    else if(pTarPet != NULL)
    {
        recovery = pTarPet->GetRecovery();
        targetXiang = pTarPet->xiang;
    }
    
    damage -= recovery;
    
    if(targetXiang == EXShuiXiang)
        damage = (int)(damage*1.3);
        
    if(GetRecoveryImporve(target) != 0)
        damage /= 2;
    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo) || (m_type == EFTMatch))
        damage /= 2;
    if(damage < 0)
        damage = 1;
    
    SMonsterInst *pMonster = GetMonster(target);
    if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 5;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage = 1;
    }
    
    if(targetXiang == EXHuoXiang)
        damage = (int)(damage*1.05);
    else if(targetXiang == EXTuXiang)
        damage = (int)(damage*1.3);
         
    if(tarState == EMSDizzy)
    {
        SetState(target,0);
    }
    
    msg<<src<<(uint8)EOTSkill<<(uint8)23<<(uint8)1;
    
    msg<<target<<PRO_SUCCESS;//<<(uint8)0<<damage;//<<(uint8)0<<(uint8)0;
    
    uint8 protecter = GetProtecter(target);
    int protectDamage = 0;
    if((protecter != 0) 
        && (GetState(protecter) < EMSDied)
        && (GetState(protecter) != EMSDizzy))
    {
        damage /= 2;
        protectDamage = damage;
        DecreaseHp(protecter,damage);
    }
    
    if(baojidu > 0)
    {
        damage += CalculateRate(damage,baojidu,100) + baojiAdd;
        //damage = CalculateRate(damage,120,100) + add;
        msg<<(uint8)1<<damage;
    }
    else
    {
        msg<<(uint8)0<<damage;
    }
        
    DecreaseHp(target,damage,src);
    
    //CUser *pUserTar = GetUser(target);
    int fanzhendu = 0;
    //if(pUserTar != 0)
    fanzhendu = GetFanZhen(target);
    if((fanzhendu > 0) && (tarState < EMSDied))
    {
        int fanzhenVal = CalculateRate(damage,fanzhendu,100);
        if(fanzhenVal <= 0)
            fanzhenVal = 1;
        msg<<(uint8)1<<fanzhenVal;
        DecreaseHp(src,fanzhenVal);
    }
    else
    {
        msg<<(uint8)0<<0;
    }
    msg<<protectDamage<<0;
    return 1;
}
/*********
怪使用此功能
当前气血/最大气血>0.3时，技能部分公式为：土相*12+90*技能等级/3+90
当前气血/最大气血<=30%时，先取段：x = 当前气血*100/最大气血
倍率 = x * x * 0.0015 - 0.205 * x + 8
技能部分公式为：（100*技能等级/3+100）*倍率
*********/
uint8 CFight::UseSkill24(CNetMessage &msg,SMonsterInst *pMonster,int skillLevel,uint8 src,uint8 target)
{
    int damage = 0;
    uint8 tarState = GetState(target);
    //技能等级*18+22
    int useMp = (int)(skillLevel*18+22);
    DecreaseMp(src,useMp);
    int hitRatio = CalculateHitRatio(src,target);
    if(Random(0,100) > hitRatio)
    {
        msg<<src<<(uint8)11<<(uint8)24<<target<<PRO_ERROR<<useMp;
        return 1;
    }
    if((tarState == EMSFrost) || (skillLevel <= 0)
        || (tarState == EMSqiankunzhao) || (tarState == EMSshenlongzhao))
    {
        if(tarState == EMSqiankunzhao)
            SetState(target,0);
        msg<<src<<(uint8)11<<(uint8)24<<target<<PRO_SUCCESS<<(uint8)0<<(uint8)1<<0<<(uint8)0<<0<<0<<useMp;
        return 1;
    }
    //当前气血/最大气血>0.3时，技能部分公式为：土相*12+90*技能等级/3+90
	//当前气血/最大气血<=30%时，先取段：x = 当前气血*100/最大气血
	//技能部分公式为：（土相*12+90*技能等级/3+90）*倍率
	float rate = pMonster->maxHp;
	if(rate == 0)
	    rate = 1;
	rate = pMonster->hp/rate;
	if(rate <= 0.3)
	{
	    rate = pMonster->maxHp;
    	if(rate == 0)
    	    rate = 1;
	    float x = pMonster->hp*100/rate;
	    //x * x * 0.0015 - 0.205 * x + 8
	    //倍率 = 0.00133* x * x -0.113* x +3.2
	    x = 0.00133* x * x -0.113* x +3.2;
	    //倍率封顶值：2+技能等级/75
	    float maxX = 2+skillLevel/75.0;
	    if(x > maxX)
	        x = maxX;
	    damage = (int)((90*skillLevel/3+90)*x) + pMonster->attack;
	}
	else
	{
	    damage = (int)(90*skillLevel/3+90) + pMonster->attack;
	}
	damage += m_members[src-1].normalDamageImprove;// = imporveDamage; 
	
    CUser *pTarUser = GetUser(target);
    SMonsterInst *pTarMonster = GetMonster(target);
    SPet *pTarPet = GetPet(target);
    
    uint8 targetXiang = 0;
    
    int recovery = 0;
    if(pTarUser != NULL)
    {
        recovery = pTarUser->GetRecovery();
        targetXiang = pTarUser->GetXiang();
    }
    else if(pTarMonster != NULL)
    {
        recovery = pTarMonster->recovery;
        targetXiang = pTarMonster->xiang;
    }
    else if(pTarPet != NULL)
    {
        recovery = pTarPet->GetRecovery();
        targetXiang = pTarPet->xiang;
    }
    
    msg<<src<<(uint8)11<<(uint8)24<<target<<PRO_SUCCESS;
    damage -= recovery;
    
    if(targetXiang == EXJinXiang)
        damage = (int)(damage*1.05);
    else if(targetXiang == EXShuiXiang)
        damage = (int)(damage*1.6);
        
    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo) || (m_type == EFTMatch))
        damage /= 2;
    if(GetRecoveryImporve(target) != 0)
        damage /= 2;
    
    {
        SMonsterInst *pMonster = GetMonster(target);
        if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
        {
            SPet *pPet = GetPet(src);
            if((pPet == NULL) || (pPet->tmplId != 101))
                damage /= 10;
        }
        else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
        {
            SPet *pPet = GetPet(src);
            if((pPet == NULL) || (pPet->tmplId != 101))
                damage /= 5;
        }
        else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
        {
            SPet *pPet = GetPet(src);
            if(pPet == NULL)
                damage /= 10;
        }
        else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
        {
            SPet *pPet = GetPet(src);
            if(pPet == NULL)
                damage = 1;
        }
    }
          
    uint8 protecter = GetProtecter(target);
    
    int protectDamage = 0;
    if((protecter != 0) 
        && (GetState(protecter) < EMSDied)
        && (GetState(protecter) != EMSDizzy))
    {
        damage /= 2;
        protectDamage = damage;
        DecreaseHp(protecter,damage);
    }
    
    if(damage < 0)
        damage = 1;
                
    if(tarState == EMSDizzy)
    {
        SetState(target,0);
    }
    
    int lianjiAdd = 0;
    if(GetLianJiShu(src,lianjiAdd) > 1)
    {
        SMonsterInst *pMonster = GetMonster(target);
        if((pMonster != NULL) && (pMonster->noAdd))
        {
            lianjiAdd = 0;
        }
        damage += lianjiAdd;
        msg<<(uint8)2;
        msg<<(uint8)2<<damage<<(damage/2);
        damage += damage/2;
    }
    else
    {
        msg<<(uint8)0;
        msg<<(uint8)1<<damage;
    }
    
    DecreaseHp(target,damage,src);
        
    CUser *pUserTar = GetUser(target);
    /*if(pUserTar == NULL)
    {
        msg<<(uint8)0<<0;
    }*/
    if(GetState(target) < EMSDied)
    {
        int fanzhendu = 0;
        //if(pUserTar != 0)
        fanzhendu = GetFanZhen(target);
        if((fanzhendu > 0) && (tarState < EMSDied))
        {
            int fanzhenVal = CalculateRate(damage,fanzhendu,100);
            SMonsterInst *pMonster = GetMonster(src);
            if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
            {
                SPet *pPet = GetPet(target);
                if(pPet == NULL)
                    fanzhenVal = 1;
            }
            if(fanzhenVal <= 0)
                fanzhenVal = 1;
            msg<<(uint8)2<<fanzhenVal;
            DecreaseHp(src,fanzhenVal);
        }
        else if(IsFanJi(pUserTar,GetMonster(target)))
        {
            //反击
            int fanjiVal = CalculateDamage(target,src);
            SMonsterInst *pMonster = GetMonster(src);
            if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
            {
                SPet *pPet = GetPet(src);
                if(pPet == NULL)
                    fanjiVal = 1;
            }
            msg<<(uint8)1<<fanjiVal;
            DecreaseHp(src,fanjiVal);
        }
        else
        {
            msg<<(uint8)0<<0;
        }
    }
    else
    {
        msg<<(uint8)0<<0;
    }
    msg<<protectDamage<<useMp;
    return 1;
}

uint8 CFight::UseSkill24(CNetMessage &msg,CUser *pUser,int skillLevel,uint8 src,uint8 target)
{
    int damage = 0;
    uint8 tarState = GetState(target);
    //技能等级*18+22
    int useMp = (int)(pUser->GetSrcSkillLevel(24)*18+22);
    if(useMp > pUser->GetMp())
        return 0;
    DecreaseMp(src,useMp);
    int hitRatio = CalculateHitRatio(src,target);
    if(Random(0,100) > hitRatio)
    {
        msg<<src<<(uint8)11<<(uint8)24<<target<<PRO_ERROR<<useMp;
        return 1;
    }
    if((tarState == EMSFrost) || (skillLevel <= 0)
        || (tarState == EMSqiankunzhao) || (tarState == EMSshenlongzhao))
    {
        if(tarState == EMSqiankunzhao)
            SetState(target,0);
        msg<<src<<(uint8)11<<(uint8)24<<target<<PRO_SUCCESS<<(uint8)0<<(uint8)1<<0<<(uint8)0<<0<<0<<useMp;
        return 1;
    }
    //当前气血/最大气血>0.3时，技能部分公式为：土相*12+90*技能等级/3+90
	//当前气血/最大气血<=30%时，先取段：x = 当前气血*100/最大气血
	//技能部分公式为：（土相*12+90*技能等级/3+90）*倍率
	float rate = pUser->GetMaxHp();
	if(rate == 0)
	    rate = 1;
	rate = pUser->GetHp()/rate;
	if(rate <= 0.3)
	{
	    rate = pUser->GetMaxHp();
    	if(rate == 0)
    	    rate = 1;
	    float x = pUser->GetHp()*100/rate;
	    //x * x * 0.0015 - 0.205 * x + 8
	    //倍率 = 0.00133* x * x -0.113* x +3.2
	    x = 0.00133* x * x -0.113* x +3.2;
	    //倍率封顶值：2+技能等级/75
	    float maxX = 2+skillLevel/75.0;
	    if(x > maxX)
	        x = maxX;
	    damage = (int)((pUser->GetTu()*12+90*skillLevel/3+90)*x) + pUser->GetDamage();
	}
	else
	{
	    damage = (int)(pUser->GetTu()*12+90*skillLevel/3+90) + pUser->GetDamage();
	}
	damage += m_members[src-1].normalDamageImprove;// = imporveDamage; 
	
    CUser *pTarUser = GetUser(target);
    SMonsterInst *pTarMonster = GetMonster(target);
    SPet *pTarPet = GetPet(target);
    
    uint8 targetXiang = 0;
    
    int recovery = 0;
    if(pTarUser != NULL)
    {
        recovery = pTarUser->GetRecovery();
        targetXiang = pTarUser->GetXiang();
    }
    else if(pTarMonster != NULL)
    {
        recovery = pTarMonster->recovery;
        targetXiang = pTarMonster->xiang;
    }
    else if(pTarPet != NULL)
    {
        recovery = pTarPet->GetRecovery();
        targetXiang = pTarPet->xiang;
    }
    
    msg<<src<<(uint8)11<<(uint8)24<<target<<PRO_SUCCESS;
    damage -= recovery;
    
    if(targetXiang == EXJinXiang)
        damage = (int)(damage*1.05);
    else if(targetXiang == EXShuiXiang)
        damage = (int)(damage*1.6);
        
    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo) || (m_type == EFTMatch))
        damage /= 2;
    if(GetRecoveryImporve(target) != 0)
        damage /= 2;
    
    {
        SMonsterInst *pMonster = GetMonster(target);
        if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
        {
            SPet *pPet = GetPet(src);
            if((pPet == NULL) || (pPet->tmplId != 101))
                damage /= 10;
        }
        else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
        {
            SPet *pPet = GetPet(src);
            if((pPet == NULL) || (pPet->tmplId != 101))
                damage /= 5;
        }
        else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
        {
            SPet *pPet = GetPet(src);
            if(pPet == NULL)
                damage /= 10;
        }
        else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
        {
            SPet *pPet = GetPet(src);
            if(pPet == NULL)
                damage = 1;
        }
    }
    
    uint8 protecter = GetProtecter(target);
    
    int protectDamage = 0;
    if((protecter != 0) 
        && (GetState(protecter) < EMSDied)
        && (GetState(protecter) != EMSDizzy))
    {
        damage /= 2;
        protectDamage = damage;
        DecreaseHp(protecter,damage);
    }
    
    if(damage < 0)
        damage = 1;
                
    if(tarState == EMSDizzy)
    {
        SetState(target,0);
    }
    
    int lianjiAdd = 0;
    if(GetLianJiShu(src,lianjiAdd) > 1)
    {
        SMonsterInst *pMonster = GetMonster(target);
        if((pMonster != NULL) && (pMonster->noAdd))
        {
            lianjiAdd = 0;
        }
        damage += lianjiAdd;
        msg<<(uint8)2;
        msg<<(uint8)2<<damage<<(damage/2);
        damage += damage/2;
    }
    else
    {
        msg<<(uint8)0;
        msg<<(uint8)1<<damage;
    }
    
    DecreaseHp(target,damage,src);
        
    CUser *pUserTar = GetUser(target);
    /*if(pUserTar == NULL)
    {
        msg<<(uint8)0<<0;
    }*/
    if(GetState(target) < EMSDied)
    {
        int fanzhendu = 0;
        //if(pUserTar != 0)
        fanzhendu = GetFanZhen(target);
        if((fanzhendu > 0) && (tarState < EMSDied))
        {
            int fanzhenVal = CalculateRate(damage,fanzhendu,100);
            if(fanzhenVal <= 0)
                fanzhenVal = 1;
            msg<<(uint8)2<<fanzhenVal;
            DecreaseHp(src,fanzhenVal);
        }
        else if(IsFanJi(pUserTar,GetMonster(target)))
        {
            //反击
            int fanjiVal = CalculateDamage(target,src);
            msg<<(uint8)1<<fanjiVal;
            DecreaseHp(src,fanjiVal);
        }
        else
        {
            msg<<(uint8)0<<0;
        }    
    }
    else
    {
        msg<<(uint8)0<<0;
    }    
    msg<<protectDamage<<useMp;
    return 1;
}
uint8 CFight::UseSkill22(CNetMessage &msg,CUser *pUser,int skillLevel,uint8 src,uint8 target)
{
    /*
    ACTION=11 破甲碎击                       1反击、2反震
    +--------+-----+-------+-------+---------+---------+------+-------+------+
    | TARGET | HIT | BAOJI | POUND | DAMAGE  | RESPOND | RESV | BAOHU | MANA |
    +--------+-----+-------+-------+---------+---------+------+-------+------+
    |    1   |  1  |   1   |   1   | 4*POUND |   1     |  4   |   4   |   4  |
    +--------+-----+-------+-------+---------+---------+------+-------+------+
    */
    int damage = 0;
    uint8 tarState = GetState(target);
    int useMp = pUser->GetSrcSkillLevel(22)*5+25;
    if(useMp > pUser->GetMp())
        return 0;
    DecreaseMp(src,useMp);
    int hitRatio = CalculateHitRatio(src,target);
    if(Random(0,100) > hitRatio)
    {
        msg<<src<<(uint8)11<<(uint8)22<<target<<PRO_ERROR<<useMp;
        return 1;
    }
    if((tarState == EMSFrost) || (skillLevel <= 0)
        || (tarState == EMSqiankunzhao) || (tarState == EMSshenlongzhao))
    {
        if(tarState == EMSqiankunzhao)
            SetState(target,0);
        msg<<src<<(uint8)11<<(uint8)22<<target<<PRO_SUCCESS<<(uint8)0<<(uint8)1<<0<<(uint8)0<<0<<0<<useMp;
        return 1;
    }
    //土相*15+110*技能等级/3+110
    damage = (int)(pUser->GetTu()*15+110*skillLevel/3+10) + pUser->GetDamage();
    damage += m_members[src-1].normalDamageImprove;// = imporveDamage; 
    CUser *pTarUser = GetUser(target);
    SMonsterInst *pTarMonster = GetMonster(target);
    SPet *pTarPet = GetPet(target);
    
    uint8 targetXiang = 0;
    int recovery = 0;
    if(pTarUser != NULL)
    {
        recovery = pTarUser->GetRecovery();
        targetXiang = pTarUser->GetXiang();
    }
    else if(pTarMonster != NULL)
    {
        recovery = pTarMonster->recovery;
        targetXiang = pTarMonster->xiang;
    }
    else if(pTarPet != NULL)
    {
        recovery = pTarPet->GetRecovery();
        targetXiang = pTarPet->xiang;
    }
    
    msg<<src<<(uint8)11<<(uint8)24<<target<<PRO_SUCCESS;
    damage -= recovery;
    
    if(targetXiang == EXShuiXiang)
        damage = (int)(damage*1.6);
        
    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo) || (m_type == EFTMatch))
        damage /= 2;
    if(GetRecoveryImporve(target) != 0)
        damage /= 2;
    
    SMonsterInst *pMonster = GetMonster(target);
    if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 5;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage = 1;
    }
          
    uint8 protecter = GetProtecter(target);
    
    int protectDamage = 0;
    if((protecter != 0) 
        && (GetState(protecter) < EMSDied)
        && (GetState(protecter) != EMSDizzy))
    {
        damage /= 2;
        protectDamage = damage;
        DecreaseHp(protecter,damage);
    }
    
    if(damage < 0)
        damage = 1;
                
    if(tarState == EMSDizzy)
    {
        SetState(target,0);
    }
    
        
    int lianjiAdd = 0;
    if(GetLianJiShu(src,lianjiAdd) > 1)
    {
        SMonsterInst *pMonster = GetMonster(target);
        if((pMonster != NULL) && (pMonster->noAdd))
        {
            lianjiAdd = 0;
        }
        damage += lianjiAdd;
        msg<<(uint8)2;
        msg<<(uint8)2<<damage<<(damage/2);
        damage += damage/2;
    }
    else
    {
        msg<<(uint8)0;
        msg<<(uint8)1<<damage;
    }
    
    DecreaseHp(target,damage,src);
        
    CUser *pUserTar = GetUser(target);
    if(GetState(target) < EMSDied)
    {
        int fanzhendu = 0;
        //if(pUserTar != 0)
        fanzhendu = GetFanZhen(target);
        if((fanzhendu > 0) && (tarState < EMSDied))
        {
            int fanzhenVal = CalculateRate(damage,fanzhendu,100);
            if(fanzhenVal <= 0)
                fanzhenVal = 1;
            msg<<(uint8)2<<fanzhenVal;
            DecreaseHp(src,fanzhenVal);
        }
        else if(IsFanJi(pUserTar,GetMonster(target)))
        {
            //反击
            int fanjiVal = CalculateDamage(target,src);
            msg<<(uint8)1<<fanjiVal;
            DecreaseHp(src,fanjiVal);
        }
        else
        {
            msg<<(uint8)0<<0;
        }    
    }
    else
    {
        msg<<(uint8)0<<0;
    }
    msg<<protectDamage<<useMp;
    return 1;
}

uint8 CFight::UseSkill21(CNetMessage &msg,CUser *pUser,int skillLevel,uint8 src,uint8 target)
{
    int damage = 0;
    uint8 tarState = GetState(target);
    int useMp = pUser->GetSrcSkillLevel(21)*5+25;
    if(useMp > pUser->GetMp())
        return 0;
    DecreaseMp(src,useMp);
    int hitRatio = CalculateHitRatio(src,target);
    if(Random(0,100) > hitRatio)
    {
        msg<<src<<(uint8)EOTSkill<<(uint8)21<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<damage
            <<(uint8)0<<0<<0<<useMp;
        return 1;
    }
    
    if((tarState == EMSFrost) || (skillLevel <= 0)
        || (tarState == EMSqiankunzhao) || (tarState == EMSshenlongzhao))
    {
        if(tarState == EMSqiankunzhao)
            SetState(target,0);
        msg<<src<<(uint8)EOTSkill<<(uint8)21<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<damage
            <<(uint8)0<<0<<0<<useMp;
        return 1;
    }
    
    int baojiAdd = 0;
    int baojidu = GetBaoJi(src,baojiAdd);
    if(baojidu <= 0)
        baojiAdd = 0;
    
    //110+110*技能等级/3*(随机数(1.2+狂暴撕裂附加值) -狂暴撕裂附加值+0.4)
    //(木相*25+110*技能等级/3)*(随机数(1.2+狂暴撕裂附加值) -狂暴撕裂附加值+0.4)
    //(木相*25+110*技能等级/3)*(随机数(1.6+2*狂暴撕裂附加值) -狂暴撕裂附加值+0.2)
    damage = (int)(pUser->GetMu()*25+110+110*skillLevel/3.0*(Random(1,160+2*baojiAdd)/100.0-baojiAdd/100.0+0.2))
            + pUser->GetDamage();
    damage += m_members[src-1].normalDamageImprove;// = imporveDamage; 
    
    CUser *pTarUser = GetUser(target);
    SMonsterInst *pTarMonster = GetMonster(target);
    SPet *pTarPet = GetPet(target);
    
    if((pTarMonster != NULL) && (pTarMonster->noAdd))
    {
        baojidu = 0;
    }
    
    uint8 targetXiang = 0;
    int recovery = 0;
    if(pTarUser != NULL)
    {
        recovery = pTarUser->GetRecovery();
        targetXiang = pTarUser->GetXiang();
    }
    else if(pTarMonster != NULL)
    {
        recovery = pTarMonster->recovery;
        targetXiang = pTarMonster->xiang;
    }
    else if(pTarPet != NULL)
    {
        recovery = pTarPet->GetRecovery();
        targetXiang = pTarPet->xiang;
    }
    
    damage -= recovery;
    if(targetXiang == EXShuiXiang)
        damage = (int)(damage*1.3);
        
    if(GetRecoveryImporve(target) != 0)
        damage /= 2;
    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo) || (m_type == EFTMatch))
        damage /= 2;
    if(damage < 0)
        damage = 1;
    
    SMonsterInst *pMonster = GetMonster(target);
    if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 5;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage = 1;
    }
    
    if(tarState == EMSDizzy)
    {
        SetState(target,0);
    }
    
    
    msg<<src<<(uint8)EOTSkill<<(uint8)21<<(uint8)1;
    
    msg<<target<<PRO_SUCCESS;//<<(uint8)0<<damage;//<<(uint8)0<<(uint8)0;
    
    uint8 protecter = GetProtecter(target);
    int protectDamage = 0;
    if((protecter != 0) 
        && (GetState(protecter) < EMSDied)
        && (GetState(protecter) != EMSDizzy))
    {
        damage /= 2;
        protectDamage = damage;
        DecreaseHp(protecter,damage);
    }
    
    if(baojidu > 0)
    {
        //cout<<"普通攻击:"<<damage<<endl;
        damage += CalculateRate(damage,baojidu,100) + baojiAdd;
        //cout<<"爆击度:"<<baojidu<<" 爆击:"<<damage<<endl;
        //damage = CalculateRate(damage,120,100) + add;
        msg<<(uint8)1<<damage;
    }
    else
    {
        msg<<(uint8)0<<damage;
    }
        
    DecreaseHp(target,damage,src);
    
    //CUser *pUserTar = GetUser(target);
    int fanzhendu = 0;
    //if(pUserTar != 0)
    fanzhendu = GetFanZhen(target);
    if((fanzhendu > 0) && (tarState < EMSDied))
    {
        int fanzhenVal = CalculateRate(damage,fanzhendu,100);
        if(fanzhenVal <= 0)
            fanzhenVal = 1;
        msg<<(uint8)1<<fanzhenVal;
        DecreaseHp(src,fanzhenVal);
    }
    else
    {
        msg<<(uint8)0<<0;
    }
    msg<<protectDamage<<useMp;
    return 1;
}

uint8 CFight::UseSkill21(CNetMessage &msg,SMonsterInst *pM,int skillLevel,uint8 src,uint8 target)
{
    int damage = 0;
    uint8 tarState = GetState(target);
    int useMp = skillLevel*5+25;
    DecreaseMp(src,useMp);
    int hitRatio = CalculateHitRatio(src,target);
    if(Random(0,100) > hitRatio)
    {
        msg<<src<<(uint8)EOTSkill<<(uint8)21<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<damage
            <<(uint8)0<<0<<0<<useMp;
        return 1;
    }
    
    if((tarState == EMSFrost) || (skillLevel <= 0)
        || (tarState == EMSqiankunzhao) || (tarState == EMSshenlongzhao))
    {
        if(tarState == EMSqiankunzhao)
            SetState(target,0);
        msg<<src<<(uint8)EOTSkill<<(uint8)21<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<damage
            <<(uint8)0<<0<<0<<useMp;
        return 1;
    }
    
    int baojiAdd = 0;
    int baojidu = GetBaoJi(src,baojiAdd);
    if(baojidu <= 0)
        baojiAdd = 0;
    
    //110+110*技能等级/3*(随机数(1.2+狂暴撕裂附加值) -狂暴撕裂附加值+0.4)
    //(木相*25+110*技能等级/3)*(随机数(1.2+狂暴撕裂附加值) -狂暴撕裂附加值+0.4)
    //(木相*25+110*技能等级/3)*(随机数(1.6+2*狂暴撕裂附加值) -狂暴撕裂附加值+0.2)
    damage = (int)(110+110*skillLevel/3.0*(Random(1,160+2*baojiAdd)/100.0-baojiAdd/100.0+0.2))
            + pM->attack;
    damage += m_members[src-1].normalDamageImprove;// = imporveDamage; 
    
    CUser *pTarUser = GetUser(target);
    SMonsterInst *pTarMonster = GetMonster(target);
    SPet *pTarPet = GetPet(target);
    
    if((pTarMonster != NULL) && (pTarMonster->noAdd))
    {
        baojidu = 0;
    }
    
    uint8 targetXiang = 0;
    int recovery = 0;
    if(pTarUser != NULL)
    {
        recovery = pTarUser->GetRecovery();
        targetXiang = pTarUser->GetXiang();
    }
    else if(pTarMonster != NULL)
    {
        recovery = pTarMonster->recovery;
        targetXiang = pTarMonster->xiang;
    }
    else if(pTarPet != NULL)
    {
        recovery = pTarPet->GetRecovery();
        targetXiang = pTarPet->xiang;
    }
    
    damage -= recovery;
    if(targetXiang == EXShuiXiang)
        damage = (int)(damage*1.3);
        
    if(GetRecoveryImporve(target) != 0)
        damage /= 2;
    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo) || (m_type == EFTMatch))
        damage /= 2;
    if(damage < 0)
        damage = 1;
    
    SMonsterInst *pMonster = GetMonster(target);
    if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 5;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage = 1;
    }
    
    if(tarState == EMSDizzy)
    {
        SetState(target,0);
    }
    msg<<src<<(uint8)EOTSkill<<(uint8)21<<(uint8)1;
    
    msg<<target<<PRO_SUCCESS;//<<(uint8)0<<damage;//<<(uint8)0<<(uint8)0;
    
    uint8 protecter = GetProtecter(target);
    int protectDamage = 0;
    if((protecter != 0) 
        && (GetState(protecter) < EMSDied)
        && (GetState(protecter) != EMSDizzy))
    {
        damage /= 2;
        protectDamage = damage;
        DecreaseHp(protecter,damage);
    }
    
    if(baojidu > 0)
    {
        //cout<<"普通攻击:"<<damage<<endl;
        damage += CalculateRate(damage,baojidu,100) + baojiAdd;
        //cout<<"爆击度:"<<baojidu<<" 爆击:"<<damage<<endl;
        //damage = CalculateRate(damage,120,100) + add;
        msg<<(uint8)1<<damage;
    }
    else
    {
        msg<<(uint8)0<<damage;
    }
        
    DecreaseHp(target,damage,src);
    
    //CUser *pUserTar = GetUser(target);
    int fanzhendu = 0;
    //if(pUserTar != 0)
    fanzhendu = GetFanZhen(target);
    if((fanzhendu > 0) && (tarState < EMSDied))
    {
        int fanzhenVal = CalculateRate(damage,fanzhendu,100);
        if(fanzhenVal <= 0)
            fanzhenVal = 1;
        msg<<(uint8)1<<fanzhenVal;
        DecreaseHp(src,fanzhenVal);
    }
    else
    {
        msg<<(uint8)0<<0;
    }
    msg<<protectDamage<<useMp;
    return 1;
}

uint8 CFight::UseSkill22(CNetMessage &msg,SMonsterInst *pM,int skillLevel,uint8 src,uint8 target)
{
    /*
    ACTION=11 破甲碎击                       1反击、2反震
    +--------+-----+-------+-------+---------+---------+------+-------+------+
    | TARGET | HIT | BAOJI | POUND | DAMAGE  | RESPOND | RESV | BAOHU | MANA |
    +--------+-----+-------+-------+---------+---------+------+-------+------+
    |    1   |  1  |   1   |   1   | 4*POUND |   1     |  4   |   4   |   4  |
    +--------+-----+-------+-------+---------+---------+------+-------+------+
    */
    int damage = 0;
    uint8 tarState = GetState(target);
    int useMp = skillLevel*5+25;
    DecreaseMp(src,useMp);
    int hitRatio = CalculateHitRatio(src,target);
    if(Random(0,100) > hitRatio)
    {
        msg<<src<<(uint8)11<<(uint8)22<<target<<PRO_ERROR<<useMp;
        return 1;
    }
    if((tarState == EMSFrost) || (skillLevel <= 0)
        || (tarState == EMSqiankunzhao) || (tarState == EMSshenlongzhao))
    {
        if(tarState == EMSqiankunzhao)
            SetState(target,0);
        msg<<src<<(uint8)11<<(uint8)22<<target<<PRO_SUCCESS<<(uint8)0<<(uint8)1<<0<<(uint8)0<<0<<0<<useMp;
        return 1;
    }
    //土相*15+110*技能等级/3+110
    damage = (int)(110*skillLevel/3+10) + pM->attack;
    damage += m_members[src-1].normalDamageImprove;// = imporveDamage; 
    CUser *pTarUser = GetUser(target);
    SMonsterInst *pTarMonster = GetMonster(target);
    SPet *pTarPet = GetPet(target);
    
    uint8 targetXiang = 0;
    int recovery = 0;
    if(pTarUser != NULL)
    {
        recovery = pTarUser->GetRecovery();
        targetXiang = pTarUser->GetXiang();
    }
    else if(pTarMonster != NULL)
    {
        recovery = pTarMonster->recovery;
        targetXiang = pTarMonster->xiang;
    }
    else if(pTarPet != NULL)
    {
        recovery = pTarPet->GetRecovery();
        targetXiang = pTarPet->xiang;
    }
    
    msg<<src<<(uint8)11<<(uint8)24<<target<<PRO_SUCCESS;
    damage -= recovery;
    
    if(targetXiang == EXShuiXiang)
        damage = (int)(damage*1.6);
        
    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo) || (m_type == EFTMatch))
        damage /= 2;
    if(GetRecoveryImporve(target) != 0)
        damage /= 2;
    
    SMonsterInst *pMonster = GetMonster(target);
    if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 5;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage = 1;
    }
          
    uint8 protecter = GetProtecter(target);
    
    int protectDamage = 0;
    if((protecter != 0) 
        && (GetState(protecter) < EMSDied)
        && (GetState(protecter) != EMSDizzy))
    {
        damage /= 2;
        protectDamage = damage;
        DecreaseHp(protecter,damage);
    }
    
    if(damage < 0)
        damage = 1;
                
    if(tarState == EMSDizzy)
    {
        SetState(target,0);
    }
        
    int lianjiAdd = 0;
    if(GetLianJiShu(src,lianjiAdd) > 1)
    {
        SMonsterInst *pMonster = GetMonster(target);
        if((pMonster != NULL) && (pMonster->noAdd))
        {
            lianjiAdd = 0;
        }
        damage += lianjiAdd;
        msg<<(uint8)2;
        msg<<(uint8)2<<damage<<(damage/2);
        damage += damage/2;
    }
    else
    {
        msg<<(uint8)0;
        msg<<(uint8)1<<damage;
    }
    
    DecreaseHp(target,damage,src);
        
    CUser *pUserTar = GetUser(target);
    /*if(pUserTar == NULL)
    {
        msg<<(uint8)0<<0;
    }*/
    if(GetState(target) < EMSDied)
    {
        int fanzhendu = 0;
        //if(pUserTar != 0)
        fanzhendu = GetFanZhen(target);
        if((fanzhendu > 0) && (tarState < EMSDied))
        {
            int fanzhenVal = CalculateRate(damage,fanzhendu,100);
            if(fanzhenVal <= 0)
                fanzhenVal = 1;
            msg<<(uint8)2<<fanzhenVal;
            DecreaseHp(src,fanzhenVal);
        }
        else if(IsFanJi(pUserTar,GetMonster(target)))
        {
            //反击
            int fanjiVal = CalculateDamage(target,src);
            msg<<(uint8)1<<fanjiVal;
            DecreaseHp(src,fanjiVal);
        }
        else
        {
            msg<<(uint8)0<<0;
        }    
    }
    else
    {
        msg<<(uint8)0<<0;
    }
    msg<<protectDamage<<useMp;
    return 1;
}

uint8 CFight::UseSkill23(CNetMessage &msg,SMonsterInst *pM,int skillLevel,uint8 src,uint8 target)
{
    if(!m_members[src-1].useSkill23)
    {//第一轮使用
        //技能等级*18+22
        int useMp = (int)(skillLevel*18+22);
        
        
        DecreaseMp(src,useMp);
        msg<<src<<(uint8)12<<useMp;
        m_members[src-1].useSkill23 = true;
        SetSpeedImprove(src,skillLevel*2);
        SetSpeedTurn(src,1);
        return 1;
    }
    //第二轮使用
    m_members[src-1].useSkill23 = false;
    
    int damage = 0;
    uint8 tarState = GetState(target);
    int hitRatio = CalculateHitRatio(src,target);
    if(Random(0,100) > hitRatio)
    {
        msg<<src<<(uint8)EOTSkill<<(uint8)23<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<damage
            <<(uint8)0<<0<<0<<0;
        return 1;
    }
    
    if((tarState == EMSFrost) || (skillLevel <= 0)
        || (tarState == EMSqiankunzhao) || (tarState == EMSshenlongzhao))
    {
        if(tarState == EMSqiankunzhao)
            SetState(target,0);
        msg<<src<<(uint8)EOTSkill<<(uint8)23<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<damage
            <<(uint8)0<<0<<0<<0;
        return 1;
    }
    
    int baojiAdd = 0;
    int baojidu = GetBaoJi(src,baojiAdd,50);
    //(木相*25+120*技能等级/3)* (1.8+狂暴撕裂附加值) 
    //(木相*25+120*技能等级/3)* (1.6+狂暴撕裂附加值) 
    //(木相*25+120*技能等级/3)* (0.5+技能等级/100) 
    damage = (int)((120*skillLevel/3.0)*(skillLevel/100.0+0.5))
            + pM->attack;
    damage += m_members[src-1].normalDamageImprove;// = imporveDamage; 
    
    CUser *pTarUser = GetUser(target);
    SMonsterInst *pTarMonster = GetMonster(target);
    SPet *pTarPet = GetPet(target);
    
    if((pTarMonster != NULL) && (pTarMonster->noAdd))
    {
        baojidu = 0;
    }
    
    int recovery = 0;
    uint8 targetXiang = 0;
    if(pTarUser != NULL)
    {
        recovery = pTarUser->GetRecovery();
        targetXiang = pTarUser->GetXiang();
    }
    else if(pTarMonster != NULL)
    {
        recovery = pTarMonster->recovery;
        targetXiang = pTarMonster->xiang;
    }
    else if(pTarPet != NULL)
    {
        recovery = pTarPet->GetRecovery();
        targetXiang = pTarPet->xiang;
    }
    
    damage -= recovery;
    
    if(targetXiang == EXShuiXiang)
        damage = (int)(damage*1.3);
        
    if(GetRecoveryImporve(target) != 0)
        damage /= 2;
    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo) || (m_type == EFTMatch))
        damage /= 2;
    if(damage < 0)
        damage = 1;
    
    SMonsterInst *pMonster = GetMonster(target);
    if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
    {
        SPet *pPet = GetPet(src);
        if((pPet == NULL) || (pPet->tmplId != 101))
            damage /= 5;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage /= 10;
    }
    else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
    {
        SPet *pPet = GetPet(src);
        if(pPet == NULL)
            damage = 1;
    }
    
    if(targetXiang == EXHuoXiang)
        damage = (int)(damage*1.05);
    else if(targetXiang == EXTuXiang)
        damage = (int)(damage*1.3);
         
    if(tarState == EMSDizzy)
    {
        SetState(target,0);
    }
    
    msg<<src<<(uint8)EOTSkill<<(uint8)23<<(uint8)1;
    
    msg<<target<<PRO_SUCCESS;//<<(uint8)0<<damage;//<<(uint8)0<<(uint8)0;
    
    uint8 protecter = GetProtecter(target);
    int protectDamage = 0;
    if((protecter != 0) 
        && (GetState(protecter) < EMSDied)
        && (GetState(protecter) != EMSDizzy))
    {
        damage /= 2;
        protectDamage = damage;
        DecreaseHp(protecter,damage);
    }
    
    if(baojidu > 0)
    {
        damage += CalculateRate(damage,baojidu,100) + baojiAdd;
        //damage = CalculateRate(damage,120,100) + add;
        msg<<(uint8)1<<damage;
    }
    else
    {
        msg<<(uint8)0<<damage;
    }
        
    DecreaseHp(target,damage,src);
    
    //CUser *pUserTar = GetUser(target);
    int fanzhendu = 0;
    //if(pUserTar != 0)
    fanzhendu = GetFanZhen(target);
    if((fanzhendu > 0) && (tarState < EMSDied))
    {
        int fanzhenVal = CalculateRate(damage,fanzhendu,100);
        if(fanzhenVal <= 0)
            fanzhenVal = 1;
        msg<<(uint8)1<<fanzhenVal;
        DecreaseHp(src,fanzhenVal);
    }
    else
    {
        msg<<(uint8)0<<0;
    }
    msg<<protectDamage<<0;
    return 1;
}

int GetEffectiveTurn(int skillLevel,int srcDaoHang,int tarDaoHang)
{
    if(tarDaoHang == 0xfffffff)
        return 0;
    int turn = (int)(skillLevel/40.0*srcDaoHang/tarDaoHang);
    if(turn <= 1)
    {
        turn = 2;
    }
    else if(turn > 5)
    {
        turn = 5;
    }
    return turn;
}

uint8 CFight::SpecialSkill(uint8 src,uint8 target,uint8 skillId,CNetMessage &msg)
{
    if(GetState(src) == EMSForget)
        return 0;
    if(GetState(src) == EMSFrost)
    {
        msg<<src<<(uint8)EOTSpecial<<skillId<<target<<PRO_ERROR<<0<<0<<0;
        return 1;
    }
    uint8 tarState = GetState(target);
    if(tarState == EMSDied)
    {
        uint8 targets[GROUP2_BEGIN];
        uint8 num = 0;
        GetAnotherGroup(src,targets,num);
        
        GetExcept(target,targets,num);
        
        if(num <= 0)
            return 0;
        target = RandSelect(targets,num);
        tarState = GetState(target);
    }
    int tarMaxHp = GetMaxHp(target);
    int damage = (int)(tarMaxHp*0.4);
    SMonsterInst *pMonster = GetMonster(src);
    if((pMonster != NULL) && (pMonster->addData != 0))
    {
        damage = tarMaxHp*pMonster->addData/100;
    }
    if(GetRecoveryImporve(target) != 0)
        damage = 1;
    DecreaseHp(target,damage);
    msg<<src<<(uint8)EOTSpecial<<skillId<<target<<PRO_SUCCESS<<damage<<0<<0;
    return 1;
}

uint8 CFight::SkillButtle(uint8 src,uint8 target,uint8 skillId,CNetMessage &msg)
{
    uint8 tarState = GetState(target);
    uint8 skillType = GetSkillType(skillId);
#ifdef DEBUG
    cout<<endl;
    cout<<(int)src<<":use skill:"<<(int)skillId<<" to:"<<(int)target<<endl;
    cout<<endl;
#endif    
    if(((skillId != 121) && (tarState >= EMSDied)) || IsEmpty(target))
    {
        if(((skillType >= ESTImproveDamage) && (skillType <= ESTImproveRecovery))
            || (skillType >= ESTsiwangchanmian))
        {
            target = src;
            tarState = GetState(target);
        }
        else
        {
            uint8 targets[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(src,targets,num);
            
            GetExcept(target,targets,num);
            
            if(num <= 0)
                return 0;
            target = RandSelect(targets,num);
            tarState = GetState(target);
        }
    }
    if(GetState(src) == EMSForget)
        return 0;
        
    CUser *pUserSrc = GetUser(src);
    
    if(pUserSrc != NULL)
    {
        if(!NotDecreaseNaiJiu())
            pUserSrc->DecreaseWuQiNaiJiu(1);
    }
    CUser *pUser = GetUser(target);
    int skillLevel = 1;
    int srcSkillLevel = 1;
    int effectiveTurn = 1;//EFFECTIVE_TRUN
    int srcDaohang = 0;
    int targetDaohang = 0;
    //double fangDangHao = 0;pUser->GetLevel();
        //biaozhun = biaozhun*biaozhun*biaozhun/620;
    if(pUser != NULL)
    {
        if(!NotDecreaseNaiJiu())
            pUser->DecreaseFangJuNaiJiu(2);
    }
    targetDaohang = GetTargetDaoHang(target);
    
    pUser = GetUser(src);
    
    if(pUser != NULL)
    {
        //if(pUser->GetMp() <= 0)
            //return 0;
            
        skillLevel = pUser->GetSkillLevel(skillId);
        srcSkillLevel = pUser->GetSrcSkillLevel(skillId);
        //（技能等级/40）*（施法者道行值/标准道行），最小值1，最大值5
        
        if(targetDaohang < 1)
            targetDaohang = 1;

#ifdef DEBUG
        cout<<"src dao hang:"<<pUser->GetDaoHang()<<endl;
        cout<<"target dao hang:"<<targetDaohang<<endl;
        cout<<"skill level:"<<(int)skillLevel<<endl;
        cout<<"effective turn:"<<effectiveTurn<<endl;
#endif        
        srcDaohang = pUser->GetDaoHang();
    }
    else
    {
        SMonsterInst *pMonster = GetMonster(src);
        if(pMonster != NULL)
        {
            //怪物道行=等级*等级*等级/775
            srcDaohang = pMonster->daohang;//pMonster->level*pMonster->level*pMonster->level/775;
            skillLevel = pMonster->GetSkillLevel(skillId);
            //cout<<skillId<<":"<<skillLevel<<endl;
            if(skillLevel == 0)
                skillLevel = pMonster->level;
            srcSkillLevel = skillLevel;
        }
        else
        {
            SPet *pPet = GetPet(src);
            if(pPet != NULL)
            {
                skillLevel = 0;
                srcDaohang = pPet->wuxue;
                for(uint8 i = 0; i < SPet::MAX_SKILL_NUM; i++)
                {
                    if(pPet->skill[i] == skillId)
                    {
                        skillLevel = pPet->skillLevel[i];
                        srcSkillLevel = skillLevel;
                        break;
                    }
                }
                if((skillLevel == 0) || (pPet->mp <= 0))
                {
                    return 0;
                }
            }
        }
    }
    effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
    if(skillId == 21)
    {
        if(pUser != NULL)
        {
            return UseSkill21(msg,pUser,skillLevel,src,target);
        }
        else
        {
            SMonsterInst *pMonster = GetMonster(src);
            if(pMonster != NULL)
                return UseSkill21(msg,pMonster,skillLevel,src,target);
            return 0;
        }
        return 0;
    }
    else if(skillId == 22)
    {
        if(pUser != NULL)
        {
            return UseSkill22(msg,pUser,skillLevel,src,target);
        }
        else
        {
            SMonsterInst *pMonster = GetMonster(src);
            if(pMonster != NULL)
                return UseSkill22(msg,pMonster,skillLevel,src,target);
            return 0;
        }
        return 0;
    }
    else if(skillId == 23)
    {
        if(pUser != NULL)
        {
            return UseSkill23(msg,pUser,skillLevel,src,target);
        }
        else
        {
            SMonsterInst *pMonster = GetMonster(src);
            if(pMonster != NULL)
                return UseSkill23(msg,pMonster,skillLevel,src,target);
            return 0;
        }
        return 0;
    }
    else if(skillId == 24)
    {
        if(pUser != NULL)
        {
            return UseSkill24(msg,pUser,skillLevel,src,target);
        }
        else
        {
            SMonsterInst *pMonster = GetMonster(src);
            if(pMonster != NULL)
                return UseSkill24(msg,pMonster,skillLevel,src,target);
            return 0;
        }
        return 0;
    }
    else if(skillId == 121)
    {
        if(pUser != NULL)
        {
            return UseSkill121(msg,pUser,skillLevel,src,target);
        }
        return 0;
    }
    //负面技能命中率
    //（基础命中率+技能等级*0.0017）*（攻方道行/防方道行），封顶值90%，最小值10%
    //bool miss = false;
    //int fuMianMingZhong = 0;
    
    if(srcDaohang <= 0)
        srcDaohang = 1;
    if(targetDaohang <= 0)
        targetDaohang = 1;
    
    //uint8 num = GetSkillAttackNum(skillId);
    int useMp = SkillUseMp(src,skillId,srcSkillLevel,skillType,GetSkillJie(skillId));
    if(useMp < 0)
        return 0;
        
    switch(skillType)
    {
        //咒术
    case ESTDamageHp:
        {
            int damage = 0;
            if(skillLevel <= 0)
            {
                msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<damage
                    <<(uint8)0<<0<<0<<useMp;
                return 1;
            }
            damage = CalculateSkillDamage(src,target,skillId);
            /*if(tarState == EMSDizzy)
            {
                SetState(target,0);
            }
            else if(tarState == EMSruyiquan)
            {
                if((m_members[target-1].stateAddData1 > 0)
                    && (m_members[target-1].stateAddData2 > 0))
                {
                    m_members[target-1].stateAddData1--;
                    if(damage < m_members[target-1].stateAddData2)
                    {
                        m_members[target-1].stateAddData2 -= damage;
                        damage = 0;
                    }
                    else
                    {
                        damage -= m_members[target-1].stateAddData2;
                        m_members[target-1].stateAddData2 = 0;
                    }
                }
                else
                {
                    SetState(target,0);
                }
            }*/
            
            msg<<src<<(uint8)EOTSkill<<skillId;//
            
            uint8 targets[GROUP2_BEGIN];
            uint8 num = 0;
            SMonsterInst *pMonster = GetMonster(src);
            if(pMonster != NULL)
            {
                num = pMonster->GetSkillAttackNum(skillId);
            }
            if(num == 0)
                num = GetSkillAttackNum(skillId,skillLevel);
            uint8 anotherGroupNum = 0;
            GetAnotherGroup(src,targets,anotherGroupNum);
            uint16 pos = msg.GetDataLen();
            msg<<num;
            
            for(uint8 i = 0; i < num; i++)
            {
                tarState = GetState(target);
                if(tarState == EMSFrost)
                {
                    msg<<target<<PRO_SUCCESS<<(uint8)0<<0<<(uint8)0<<0<<0;
                    continue;
                }
                else if(tarState == EMSruyiquan)
                {
                    if((m_members[target-1].stateAddData1 > 0)
                        && (m_members[target-1].stateAddData2 > 0))
                    {
                        m_members[target-1].stateAddData1--;
                        if(damage < m_members[target-1].stateAddData2)
                        {
                            m_members[target-1].stateAddData2 -= damage;
                            damage = 0;
                        }
                        else
                        {
                            damage -= m_members[target-1].stateAddData2;
                            m_members[target-1].stateAddData2 = 0;
                        }
                    }
                    else
                    {
                        SetState(target,0);
                    }
                }
                msg<<target<<PRO_SUCCESS;//<<(uint8)0<<damage;//<<(uint8)0<<(uint8)0;
                int add = 0;
                uint8 protecter = GetProtecter(target);
                int protectDamage = 0;
                if((protecter > 0) && (num < anotherGroupNum) 
                    && (GetState(protecter) < EMSDied)
                    && (GetState(protecter) != EMSDizzy))
                {
                    damage /= 2;
                    protectDamage = damage;
                    DecreaseHp(protecter,damage);
                }
                SMonsterInst *pMonster = GetMonster(target);
                if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_LANG))
                {
                    SPet *pPet = GetPet(src);
                    if((pPet == NULL) || (pPet->tmplId != 101))
                        damage /= 10;
                }
                else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_DU_MU))
                {
                    SPet *pPet = GetPet(src);
                    if((pPet == NULL) || (pPet->tmplId != 101))
                        damage /= 5;
                }
                else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ONLY_PET))
                {
                    SPet *pPet = GetPet(src);
                    if(pPet == NULL)
                        damage /= 10;
                }
                else if((pMonster != NULL) && (pMonster->GetCeLue() == ONLY_PET))
                {
                    SPet *pPet = GetPet(src);
                    if(pPet == NULL)
                        damage = 1;
                }
                
                if(FaShuBaoJi(GetUser(src),add,GetMonster(src)))
                {
                    SMonsterInst *pM = GetMonster(target);
                    if((pM != NULL) && (pM->noAdd))
                    {
                        add = 0;
                    }
                    damage = CalculateRate(damage,120,100) + add;
                    if((pM != NULL) && (pM->GetCeLue() == ONLY_PET))
                    {
                        SPet *pPet = GetPet(src);
                        if(pPet == NULL)
                            damage = 1;
                    }
                    msg<<(uint8)1<<damage;
                }
                else
                {
                    msg<<(uint8)0<<damage;
                }
                
                DecreaseHp(target,damage,src);
                int fantan = FaShuFanTan(GetUser(target),GetMonster(target));
                if((fantan > 0) && (GetState(target) < EMSDied))
                {
                    fantan = CalculateRate(damage,fantan,100);
                    if(fantan == 0)
                        fantan = 1;
                    msg<<(uint8)1<<fantan;
                    DecreaseHp(src,fantan);
                }
                else
                {
                    msg<<(uint8)0<<0;
                }
                GetExcept(target,targets,anotherGroupNum);
                msg<<protectDamage;
                if(anotherGroupNum <= 0)
                {
                    i++;
                    msg.WriteData(pos,&i,1);
                    break;
                }
                target = RandSelect(targets,anotherGroupNum);
                if(GetState(target) >= EMSDied)
                    break;
                damage = CalculateSkillDamage(src,target,skillId);
                //DecreaseHp(target,damage);
                //msg<<target<<PRO_SUCCESS<<(uint8)0<<damage;
            }
            msg<<useMp;
            return 1;
        }
    //遗忘
    case ESTForget://
        {//负面技能命中率： （基础命中率+技能等级*0.0017）*（攻方道行/防方道行），封顶值90%，最小值10%
            msg<<src<<(uint8)EOTSkill<<skillId;
            uint16 pos = msg.GetDataLen();
            uint8 num = 1;
            CUser *pTar = GetUser(target);
            if(pTar != NULL)
            {
                targetDaohang += 100*pTar->GetItemKangYiWang();
            }
            effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
            if(FuMianSkillMiss(skillId,skillLevel,srcDaohang,targetDaohang))
            {
                msg<<num<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
            }
            else
            {
                msg<<num<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                SetState(target,EMSForget);
                SetTurn(target,effectiveTurn);
            }
            
            uint8 tarNum = GetSkillAttackNum(skillId,skillLevel);
            uint8 targets[GROUP2_BEGIN];
            uint8 anotherGroupNum = 0;
            GetAnotherGroup(src,targets,anotherGroupNum);
            
            uint8 i = 0;
            for(; i < tarNum-1; i++)
            {
                GetExcept(target,targets,anotherGroupNum);
                if(anotherGroupNum <= 0)
                {
                    break;
                }
                target = RandSelect(targets,anotherGroupNum);
                targetDaohang = GetTargetDaoHang(target);
                CUser *pTar = GetUser(target);
                if(pTar != NULL)
                {
                    targetDaohang += 100*pTar->GetItemKangYiWang();
                }
                //effectiveTurn = (int)(skillLevel/40.0*pUser->GetDaoHang()/targetDaohang);
                effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
                if(FuMianSkillMiss(skillId,skillLevel,srcDaohang,targetDaohang))
                {
                    msg<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                }
                else
                {
                    msg<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                    SetState(target,EMSForget);
                    SetTurn(target,effectiveTurn);
                }
            }
            num += i;
            if(i > 0)
            {
                msg.WriteData(pos,&num,1);
            }
            msg<<useMp;
            return 1;
        }
    //下毒
    case ESTEnvenom:
        {//负面技能命中率： （基础命中率+技能等级*0.0017）*（攻方道行/防方道行），封顶值90%，最小值10%
            if(tarState == EMSFrost)
            {
                //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
                msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn
                    <<(uint8)0<<0;
                    msg<<useMp;
                return 1;
            }
            CUser *pTar = GetUser(target);
            if(pTar != NULL)
            {
                targetDaohang += 100*pTar->GetItemKangZhongDu();
            }
            effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
            msg<<src<<(uint8)EOTSkill<<skillId;
            uint16 pos = msg.GetDataLen();
            uint8 num = 1;
            
            int decreaseHp = (int)(skillLevel*0.08);
            int tarMaxHp = GetMaxHp(target);
            switch(GetSkillJie(skillId))
            {
            case 1:
                decreaseHp += 2;
                decreaseHp = CalculateRate(tarMaxHp,decreaseHp,100);
                if(decreaseHp > 2000)
                    decreaseHp = 2000;
                break;
            case 2:
                decreaseHp += 3;
                decreaseHp = CalculateRate(tarMaxHp,decreaseHp,100);
                if(decreaseHp > 2500)
                    decreaseHp = 2500;
                break;
            case 3:
                decreaseHp += 5;
                decreaseHp = CalculateRate(tarMaxHp,decreaseHp,100);
                if(decreaseHp > 3000)
                    decreaseHp = 3000;
                break;
            case 4:
                decreaseHp += 10;
                decreaseHp = CalculateRate(tarMaxHp,decreaseHp,100);
                if(decreaseHp > 4000 )
                    decreaseHp = 4000;
                break;
            }
            if(decreaseHp <= 0)
                decreaseHp = 1;
            if(FuMianSkillMiss(skillId,skillLevel,srcDaohang,targetDaohang))
            {
                msg<<num<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn<<(uint8)0<<decreaseHp;
            }
            else
            {
                SetState(target,EMSEnvenom);
                SetTurn(target,effectiveTurn);
                m_members[target-1].stateAddData1 = decreaseHp;
                DecreaseHp(target,decreaseHp);
                msg<<num<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<decreaseHp;
            }
            
            uint8 tarNum = GetSkillAttackNum(skillId,skillLevel);
            uint8 targets[GROUP2_BEGIN];
            uint8 anotherGroupNum = 0;
            GetAnotherGroup(src,targets,anotherGroupNum);
            
            uint8 i = 0;
            for(; i < tarNum-1; i++)
            {
                GetExcept(target,targets,anotherGroupNum);
                if(anotherGroupNum <= 0)
                {
                    break;
                }
                target = RandSelect(targets,anotherGroupNum);
                CUser *pTar = GetUser(target);
                targetDaohang = GetTargetDaoHang(target);
                if(pTar != NULL)
                {
                    targetDaohang += 100*pTar->GetItemKangZhongDu();
                }                
                //effectiveTurn = (int)(skillLevel/40.0*pUser->GetDaoHang()/targetDaohang);
                //effectiveTurn = GetEffectiveTurn(skillLevel,pUser->GetDaoHang(),targetDaohang);
                effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
                decreaseHp = (int)(skillLevel*0.08);
                tarMaxHp = GetMaxHp(target);
                switch(GetSkillJie(skillId))
                {
                case 1:
                    decreaseHp += 2;
                    decreaseHp = CalculateRate(tarMaxHp,decreaseHp,100);
                    if(decreaseHp > 2000)
                        decreaseHp = 2000;
                    break;
                case 2:
                    decreaseHp += 3;
                    decreaseHp = CalculateRate(tarMaxHp,decreaseHp,100);
                    if(decreaseHp > 2500)
                        decreaseHp = 2500;
                    break;
                case 3:
                    decreaseHp += 5;
                    decreaseHp = CalculateRate(tarMaxHp,decreaseHp,100);
                    if(decreaseHp > 3000)
                        decreaseHp = 3000;
                    break;
                case 4:
                    decreaseHp += 10;
                    decreaseHp = CalculateRate(tarMaxHp,decreaseHp,100);
                    if(decreaseHp > 4000 )
                        decreaseHp = 4000;
                    break;
                }
                if(decreaseHp <= 0)
                    decreaseHp = 1;
                if(FuMianSkillMiss(skillId,skillLevel,srcDaohang,targetDaohang))
                {
                    msg<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn<<(uint8)0<<decreaseHp;
                }
                else
                {
                    SetState(target,EMSEnvenom);
                    SetTurn(target,effectiveTurn);
                    m_members[target-1].stateAddData1 = decreaseHp;
                    DecreaseHp(target,decreaseHp);
                    msg<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<decreaseHp;
                }
            }
            num += i;
            if(i > 0)
            {
                msg.WriteData(pos,&num,1);
            }
            msg<<useMp;
            return 1;
        }
    //冰冻
    case ESTFrost:
        {//负面技能命中率： （基础命中率+技能等级*0.0017）*（攻方道行/防方道行），封顶值90%，最小值10%
            /*if(tarState == EMSFrost)
            {
                //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
                msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn
                    <<(uint8)0<<0;
                return 1;
            }*/
            //msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn
                //<<(uint8)0<<0;
            msg<<src<<(uint8)EOTSkill<<skillId;
            uint16 pos = msg.GetDataLen();
            uint8 num = 1;
            
            CUser *pTar = GetUser(target);
            if(pTar != NULL)
            {
                targetDaohang += 100*pTar->GetItemKangBingDong();
            }
            effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
            if(FuMianSkillMiss(skillId,skillLevel,srcDaohang,targetDaohang))
            {
                msg<<num<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
            }
            else
            {
                msg<<num<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                SetState(target,EMSFrost);
                SetTurn(target,effectiveTurn);
                SetSkill23State(target,false);
            }
            
            uint8 tarNum = GetSkillAttackNum(skillId,skillLevel);
            uint8 targets[GROUP2_BEGIN];
            uint8 anotherGroupNum = 0;
            GetAnotherGroup(src,targets,anotherGroupNum);
            
            uint8 i = 0;
            for(; i < tarNum-1; i++)
            {
                GetExcept(target,targets,anotherGroupNum);
                if(anotherGroupNum <= 0)
                {
                    break;
                }
                target = RandSelect(targets,anotherGroupNum);
                targetDaohang = GetTargetDaoHang(target);
                CUser *pTar = GetUser(target);
                if(pTar != NULL)
                {
                    targetDaohang += 100*pTar->GetItemKangBingDong();
                }
                if(targetDaohang == 0)
                    targetDaohang = 1;
                //effectiveTurn = (int)(skillLevel/40.0*pUser->GetDaoHang()/targetDaohang);
                effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
                
                if(FuMianSkillMiss(skillId,skillLevel,srcDaohang,targetDaohang))
                {
                    msg<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                }
                else
                {
                    msg<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                    SetState(target,EMSFrost);
                    SetTurn(target,effectiveTurn);
                    SetSkill23State(target,false);
                }
            }
            num += i;
            if(i > 0)
            {
                msg.WriteData(pos,&num,1);
            }
            msg<<useMp;
            return 1;
        }
    //昏睡
    case ESTDizzy:
        {//负面技能命中率： （基础命中率+技能等级*0.0017）*（攻方道行/防方道行），封顶值90%，最小值10%
            if(tarState == EMSFrost)
            {
                //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
                msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn
                    <<(uint8)0<<0;
                    msg<<useMp;
                return 1;
            }
            //msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn
                //<<(uint8)0<<0;
            msg<<src<<(uint8)EOTSkill<<skillId;
            uint16 pos = msg.GetDataLen();
            uint8 num = 1;
            CUser *pTar = GetUser(target);
            if(pTar != NULL)
            {
                targetDaohang += 100*pTar->GetItemKangHunShui();
            }
            effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
            if(FuMianSkillMiss(skillId,skillLevel,srcDaohang,targetDaohang))
            {
                msg<<num<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
            }
            else
            {
                msg<<num<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                SetState(target,EMSDizzy);
                SetTurn(target,effectiveTurn);
                SetSkill23State(target,false);
            }
            
            uint8 tarNum = GetSkillAttackNum(skillId,skillLevel);
            uint8 targets[GROUP2_BEGIN];
            uint8 anotherGroupNum = 0;
            GetAnotherGroup(src,targets,anotherGroupNum);
            
            uint8 i = 0;
            for(; i < tarNum-1; i++)
            {
                GetExcept(target,targets,anotherGroupNum);
                if(anotherGroupNum <= 0)
                {
                    break;
                }
                target = RandSelect(targets,anotherGroupNum);
                targetDaohang = GetTargetDaoHang(target);
                CUser *pTar = GetUser(target);
                if(pTar != NULL)
                {
                    targetDaohang += 100*pTar->GetItemKangHunShui();
                }
                //effectiveTurn = (int)(skillLevel/40.0*pUser->GetDaoHang()/targetDaohang);
                effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
                if(FuMianSkillMiss(skillId,skillLevel,srcDaohang,targetDaohang))
                {
                    msg<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                }
                else
                {
                    msg<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                    SetState(target,EMSDizzy);
                    SetTurn(target,effectiveTurn);
                    SetSkill23State(target,false);
                }
            }
            num += i;
            if(i > 0)
            {
                msg.WriteData(pos,&num,1);
            }
        }
        msg<<useMp;
        return 1;
    //混乱
    case ESTConfusion:
        {//负面技能命中率： （基础命中率+技能等级*0.0017）*（攻方道行/防方道行），封顶值90%，最小值10%
            if(tarState == EMSFrost)
            {
                //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
                msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn
                        <<(uint8)0<<0;
                        msg<<useMp;
                return 1;
            }
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
            //msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn
                    //<<(uint8)0<<0;
            msg<<src<<(uint8)EOTSkill<<skillId;
            uint16 pos = msg.GetDataLen();
            uint8 num = 1;
            CUser *pTar = GetUser(target);
            if(pTar != NULL)
            {
                targetDaohang += 100*pTar->GetItemKangHunLuan();
            }
            effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
            if(FuMianSkillMiss(skillId,skillLevel,srcDaohang,targetDaohang))
            {
                msg<<num<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
            }
            else
            {
                msg<<num<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                SetState(target,EMSConfusion);
                SetTurn(target,effectiveTurn);
            }
            
            uint8 tarNum = GetSkillAttackNum(skillId,skillLevel);
            uint8 targets[GROUP2_BEGIN];
            uint8 anotherGroupNum = 0;
            GetAnotherGroup(src,targets,anotherGroupNum);
            
            uint8 i = 0;
            for(; i < tarNum-1; i++)
            {
                GetExcept(target,targets,anotherGroupNum);
                if(anotherGroupNum <= 0)
                {
                    break;
                }
                target = RandSelect(targets,anotherGroupNum);
                targetDaohang = GetTargetDaoHang(target);
                if(pTar != NULL)
                {
                    targetDaohang += 100*pTar->GetItemKangHunLuan();
                }
                //effectiveTurn = (int)(skillLevel/40.0*pUser->GetDaoHang()/targetDaohang);
                //effectiveTurn = GetEffectiveTurn(skillLevel,pUser->GetDaoHang(),targetDaohang);
                effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
                if(FuMianSkillMiss(skillId,skillLevel,srcDaohang,targetDaohang))
                {
                    msg<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                }
                else
                {
                    msg<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                    SetState(target,EMSConfusion);
                    SetTurn(target,effectiveTurn);
                }
            }
            num += i;
            if(i > 0)
            {
                msg.WriteData(pos,&num,1);
            }
        }
        msg<<useMp;
        return 1;
    //法攻提升
    case ESTImproveDamage:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn
                    <<(uint8)0<<0;
        }
        else
        {
            effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
            if(effectiveTurn < 1)
                effectiveTurn = 2;
            else if(effectiveTurn > 5)
                effectiveTurn = 5;
            ++effectiveTurn;
            msg<<src<<(uint8)EOTSkill<<skillId;
            uint16 pos = msg.GetDataLen();
            uint8 num = 1;
            msg<<num<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
            //msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn
                //<<(uint8)0<<0;
            int imporveFaShu = 0;
            switch(GetSkillJie(skillId))
            {
            case 1:
                imporveFaShu = (int)(100*(1+skillLevel*0.06));
                break;
            case 2:
                imporveFaShu = (int)(150*(1+skillLevel*0.06));
                break;
            case 3:
                imporveFaShu = (int)(200*(1+skillLevel*0.06));
                break;
            case 4:
                imporveFaShu = (int)(350*(1+skillLevel*0.06));
                break;
            }
            SetDamageImprove(target,imporveFaShu);
            SetDamageTrun(target,effectiveTurn);
            
            uint8 tarNum = GetSkillAttackNum(skillId,skillLevel);
            uint8 targets[GROUP2_BEGIN];
            uint8 anotherGroupNum = 0;
            GetMeGroup(src,targets,anotherGroupNum);
            
            uint8 i = 0;
            for(; i < tarNum-1; i++)
            {
                GetExcept(target,targets,anotherGroupNum);
                if(anotherGroupNum <= 0)
                {
                    break;
                }
                target = RandSelect(targets,anotherGroupNum);
                imporveFaShu = 0;
                switch(GetSkillJie(skillId))
                {
                case 1:
                    imporveFaShu = (int)(100*(1+skillLevel*0.06));
                    break;
                case 2:
                    imporveFaShu = (int)(150*(1+skillLevel*0.06));
                    break;
                case 3:
                    imporveFaShu = (int)(200*(1+skillLevel*0.06));
                    break;
                case 4:
                    imporveFaShu = (int)(350*(1+skillLevel*0.06));
                    break;
                }
                msg<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                SetDamageImprove(target,imporveFaShu);
                SetDamageTrun(target,effectiveTurn);
            }
            num += i;
            if(i > 0)
            {
                msg.WriteData(pos,&num,1);
            }
        }
        msg<<useMp;
        return 1;
    //血量提升
    case ESTImproveHp:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn
                    <<(uint8)0<<0;
        }
        else
        {
            //标准道行 （等级*等级*等级）/620
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
            //（技能等级/40）*（施法者道行值/标准道行），最小值1，最大值5
            effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
            if(effectiveTurn < 1)
                effectiveTurn = 2;
            else if(effectiveTurn > 5)
                effectiveTurn = 5;
            int improveHp = 0;
            switch(GetSkillJie(skillId))
            {
            case 1:
                improveHp = (int)(200*(1+skillLevel*0.06));
                break;
            case 2:
                improveHp = (int)(skillLevel*40+160);
                break;
            case 3:
                improveHp = (int)(skillLevel*27+570);
                break;
            case 4:
                improveHp = (int)(skillLevel*45+1000);
                break;
            }
            ++effectiveTurn;
            if(GetHpImprove(target) <= 0)
            {
                SetHpImprove(target,improveHp);
                SetHpTurn(target,effectiveTurn);
                CUser *pUser = GetUser(target);
                SPet *pPet = GetPet(target);
                SMonsterInst *pMonster = GetMonster(target);
                if(pUser != NULL)
                {
                    pUser->SetMaxHp(pUser->GetMaxHp()+improveHp);
                }
                else if(pPet != NULL)
                {
                    pPet->maxHp += improveHp;
                }
                else if(pMonster != NULL)
                {
                    pMonster->maxHp += improveHp;
                }
                DecreaseHp(target,-improveHp);
            }
            else
            {
                effectiveTurn = 0;
                improveHp /= 2;
                DecreaseHp(target,-improveHp);
            }
            msg<<src<<(uint8)EOTSkill<<skillId;
            uint16 pos = msg.GetDataLen();
            uint8 num = 1;
            msg<<num<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<improveHp;
            //SetState(target,EMSForget);
            //SetTurn(target,effectiveTurn);
            //msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn
                //<<(uint8)0<<improveHp;
            uint8 tarNum = GetSkillAttackNum(skillId,skillLevel);
            uint8 targets[GROUP2_BEGIN];
            uint8 anotherGroupNum = 0;
            GetMeGroup(src,targets,anotherGroupNum);
            
            uint8 i = 0;
            for(; i < tarNum-1; i++)
            {
                effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
                if(effectiveTurn < 1)
                    effectiveTurn = 2;
                else if(effectiveTurn > 5)
                    effectiveTurn = 5;
                effectiveTurn++;
                GetExcept(target,targets,anotherGroupNum);
                if(anotherGroupNum <= 0)
                {
                    break;
                }
                target = RandSelect(targets,anotherGroupNum);
                improveHp = 0;
                switch(GetSkillJie(skillId))
                {
                case 1:
                    improveHp = (int)(200*(1+skillLevel*0.06));
                    break;
                case 2:
                    improveHp = (int)(skillLevel*40+160);
                    break;
                case 3:
                    improveHp = (int)(skillLevel*27+570);
                    break;
                case 4:
                    improveHp = (int)(skillLevel*45+1000);
                    break;
                }
                //++effectiveTurn;
                if(GetHpImprove(target) <= 0)
                {
                    SetHpImprove(target,improveHp);
                    SetHpTurn(target,effectiveTurn);
                    CUser *pUser = GetUser(target);
                    SPet *pPet = GetPet(target);
                    SMonsterInst *pMonster = GetMonster(target);
                    if(pUser != NULL)
                    {
                        pUser->SetMaxHp(pUser->GetMaxHp()+improveHp);
                    }
                    else if(pPet != NULL)
                    {
                        pPet->maxHp += improveHp;
                    }
                    else if(pMonster != NULL)
                    {
                        pMonster->maxHp += improveHp;
                    }
                    DecreaseHp(target,-improveHp);
                }
                else
                {
                    effectiveTurn = 0;
                    improveHp /= 2;
                    DecreaseHp(target,-improveHp);
                }
                
                msg<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<improveHp;
            }
            num += i;
            if(i > 0)
            {
                msg.WriteData(pos,&num,1);
            }
        }
        msg<<useMp;
        return 1;
    //物攻提升
    case ESTImproveRecovery:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn
                    <<(uint8)0<<0;
        }
        else
        {
            effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
            if(effectiveTurn < 1)
                effectiveTurn = 2;
            else if(effectiveTurn > 5)
                effectiveTurn = 5;
            ++effectiveTurn;
            
            msg<<src<<(uint8)EOTSkill<<skillId;
            
            uint16 pos = msg.GetDataLen();
            uint8 num = 1;
            msg<<num<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
            //msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn
                //<<(uint8)0<<0;
            
            int imporveDamage = 0;
            switch(GetSkillJie(skillId))
            {
            case 1:
                imporveDamage = (int)(150*(1+skillLevel*0.06));
                break;
            case 2:
                imporveDamage = (int)(200*(1+skillLevel*0.06));
                break;
            case 3:
                imporveDamage = (int)(300*(1+skillLevel*0.06));
                break;
            case 4:
                imporveDamage = (int)(500*(1+skillLevel*0.06));
                break;
            }
            //SetRecoveryImporve(target,imporveDamage);
            //SetRecoveryTurn(target,effectiveTurn);
            if((target > 0) && (target <= MAX_MEMBER))
            {
                m_members[target-1].normalDamageImprove = imporveDamage; 
                m_members[target-1].normalDamageTurn = effectiveTurn; 
            }
            
            uint8 tarNum = GetSkillAttackNum(skillId,skillLevel);
            uint8 targets[GROUP2_BEGIN];
            uint8 anotherGroupNum = 0;
            GetMeGroup(src,targets,anotherGroupNum);
            
            uint8 i = 0;
            for(; i < tarNum-1; i++)
            {
                GetExcept(target,targets,anotherGroupNum);
                if(anotherGroupNum <= 0)
                {
                    break;
                }
                target = RandSelect(targets,anotherGroupNum);
                msg<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
            //msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn
                //<<(uint8)0<<0;
            
                int imporveDamage = 0;
                switch(GetSkillJie(skillId))
                {
                case 1:
                    imporveDamage = (int)(150*(1+skillLevel*0.06));
                    break;
                case 2:
                    imporveDamage = (int)(200*(1+skillLevel*0.06));
                    break;
                case 3:
                    imporveDamage = (int)(300*(1+skillLevel*0.06));
                    break;
                case 4:
                    imporveDamage = (int)(500*(1+skillLevel*0.06));
                    break;
                }
                //SetRecoveryImporve(target,imporveDamage);
                //SetRecoveryTurn(target,effectiveTurn);
                if((target > 0) && (target <= MAX_MEMBER))
                {
                    m_members[target-1].normalDamageImprove = imporveDamage; 
                    m_members[target-1].normalDamageTurn = effectiveTurn; 
                }
            }
            num += i;
            if(i > 0)
            {
                msg.WriteData(pos,&num,1);
            }
        }
        msg<<useMp;
        return 1;
    //速度提升
    case ESTImproveSpeed:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn
                    <<(uint8)0<<0;
        }
        else
        {
            effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
            //effectiveTurn = (int)(skillLevel/40.0 * pUser->GetDaoHang()/
                                //(pUser->GetLevel() * pUser->GetLevel()* pUser->GetLevel() / 620));
            if(effectiveTurn < 1)
                effectiveTurn = 2;
            else if(effectiveTurn > 5)
                effectiveTurn = 5;
            ++effectiveTurn;
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
            //msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn
                //<<(uint8)0<<0;
            msg<<src<<(uint8)EOTSkill<<skillId;
            
            uint16 pos = msg.GetDataLen();
            uint8 num = 1;
            msg<<num<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
            
            int imporveSpeed = 0;
            switch(GetSkillJie(skillId))
            {
            case 1:
                imporveSpeed = (int)(30*(1+skillLevel*0.04));
                break;
            case 2:
                imporveSpeed = (int)(30*(1+skillLevel*0.04));
                break;
            case 3:
                imporveSpeed = (int)(30*(1+skillLevel*0.04));
                break;
            case 4:
                imporveSpeed = (int)(30*(1+skillLevel*0.04));
                break;
            }
            SetSpeedImprove(target,imporveSpeed);
            SetSpeedTurn(target,effectiveTurn);
            
            uint8 tarNum = GetSkillAttackNum(skillId,skillLevel);
            uint8 targets[GROUP2_BEGIN];
            uint8 anotherGroupNum = 0;
            GetMeGroup(src,targets,anotherGroupNum);
            
            uint8 i = 0;
            for(; i < tarNum-1; i++)
            {
                GetExcept(target,targets,anotherGroupNum);
                if(anotherGroupNum <= 0)
                {
                    break;
                }
                target = RandSelect(targets,anotherGroupNum);
                msg<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
            
                int imporveSpeed = 0;
                switch(GetSkillJie(skillId))
                {
                case 1:
                    imporveSpeed = (int)(30*(1+skillLevel*0.04));
                    break;
                case 2:
                    imporveSpeed = (int)(30*(1+skillLevel*0.04));
                    break;
                case 3:
                    imporveSpeed = (int)(30*(1+skillLevel*0.04));
                    break;
                case 4:
                    imporveSpeed = (int)(30*(1+skillLevel*0.04));
                    break;
                }
                SetSpeedImprove(target,imporveSpeed);
                SetSpeedTurn(target,effectiveTurn);
            }
            num += i;
            if(i > 0)
            {
                msg.WriteData(pos,&num,1);
            }
        }
        msg<<useMp;
        return 1;
    //回避提升
    case ESTImproveDodge:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<effectiveTurn
                    <<(uint8)0<<0;
        }
        else
        {
            effectiveTurn = GetEffectiveTurn(skillLevel,GetTargetDaoHang(src),targetDaohang);
            //effectiveTurn = (int)(skillLevel/40.0 * pUser->GetDaoHang()/
                                //(pUser->GetLevel() * pUser->GetLevel()* pUser->GetLevel() / 620));
            if(effectiveTurn < 1)
                effectiveTurn = 2;
            else if(effectiveTurn > 5)
                effectiveTurn = 5;
            ++effectiveTurn;
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<effectiveTurn<<(uint8)0<<(uint8)0<<(uint8)0;
            //msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn
                //<<(uint8)0<<0;
            msg<<src<<(uint8)EOTSkill<<skillId;
            
            uint16 pos = msg.GetDataLen();
            uint8 num = 1;
            msg<<num<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
            //SetState(target,EMSForget);
            //SetDodgeImprove(target,imporveDodage);
            //SetTurn(target,effectiveTurn);
            
            int imporveDodage = 0;
            switch(GetSkillJie(skillId))
            {
            case 1:
                imporveDodage = (int)(4+skillLevel*0.025);
                break;
            case 2:
                imporveDodage = (int)(3+skillLevel*0.025);
                break;
            case 3:
                imporveDodage = (int)(4+skillLevel*0.025);
                break;
            case 4:
                imporveDodage = (int)(7+skillLevel*0.025);
                break;
            }
            SetDodgeImprove(target,imporveDodage);
            SetDodgeTurn(target,effectiveTurn);
            
            uint8 tarNum = GetSkillAttackNum(skillId,skillLevel);
            uint8 targets[GROUP2_BEGIN];
            uint8 anotherGroupNum = 0;
            GetMeGroup(src,targets,anotherGroupNum);
            
            uint8 i = 0;
            for(; i < tarNum-1; i++)
            {
                GetExcept(target,targets,anotherGroupNum);
                if(anotherGroupNum <= 0)
                {
                    break;
                }
                target = RandSelect(targets,anotherGroupNum);
                msg<<target<<PRO_SUCCESS<<(uint8)0<<effectiveTurn<<(uint8)0<<0;
                //SetState(target,EMSForget);
                //SetTurn(target,effectiveTurn);
                
                int imporveDodage = 0;
                switch(GetSkillJie(skillId))
                {
                case 1:
                    imporveDodage = (int)(4+skillLevel*0.025);
                    break;
                case 2:
                    imporveDodage = (int)(3+skillLevel*0.025);
                    break;
                case 3:
                    imporveDodage = (int)(4+skillLevel*0.025);
                    break;
                case 4:
                    imporveDodage = (int)(7+skillLevel*0.025);
                    break;
                }
                SetDodgeImprove(target,imporveDodage);
                SetDodgeTurn(target,effectiveTurn);
            }
            num += i;
            if(i > 0)
            {
                msg.WriteData(pos,&num,1);
            }
        }
        msg<<useMp;
        return 1;
        
        //宠物天书技能
    //151	神圣之光 攻击技能	可以清除对方身上的辅助类状态
    case ESTshenshengzhiguang:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<1
                    <<(uint8)0<<0;
        }
        else
        {
            SetDamageImprove(target,0);
            SetHpImprove(target,0);
            SetRecoveryImporve(target,0);
            SetSpeedImprove(target,0);
            SetDodgeImprove(target,0);
            //如意圈、神龙罩、乾坤罩
            if((tarState == EMSruyiquan) 
                || (tarState == EMSqiankunzhao) 
                || (tarState == EMSshenlongzhao))
            {
                SetState(target,0);
            }
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<1
                <<(uint8)0<<0;
        }
        msg<<useMp;
        return 1;
        
    //152	漫天血舞 攻击技能	被法术击中目标的气血会有一定量被施法者吸收，以补充施法者的气血							
    case ESTmantianxuewu:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<1<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSpecial<<skillId<<target<<PRO_ERROR<<0<<0;
        }
        else
        {
            int srcHp = GetMaxHp(src);
            int tarHp= GetMaxHp(target);
            srcHp = CalculateRate(srcHp,1+skillLevel/10,100);
            tarHp = CalculateRate(tarHp,1+skillLevel/10,100);
            int decHp = min(srcHp,tarHp);
            msg<<src<<(uint8)EOTSpecial<<skillId<<target<<PRO_SUCCESS<<decHp<<decHp;
            DecreaseHp(src,-decHp);
            /*SPet *pPet = GetPet(src);
            if(pPet != NULL)
            {
                pPet->hp += srcHp;
                if(pPet->hp > pPet->maxHp)
                    pPet->hp = pPet->maxHp;
            }*/
            DecreaseHp(target,decHp);  
        }
        msg<<useMp;
        return 1;
    
    //153	舍命一击 攻击技能	被法术命中的对象的法力值会下降，而施法者气血降至1点
    case ESTshemingyiji:
        if(tarState == EMSFrost)
        {
            msg<<src<<(uint8)EOTSpecial<<skillId<<target<<PRO_ERROR<<0<<0;
        }
        else
        {
            SPet *pPet = GetPet(src);
            SMonsterInst *pMonster = GetMonster(src);
            int hp = 0;
            if(pPet != NULL)
            {
                hp = pPet->hp;
                pPet->hp = 1;
            }
            else if(pMonster != NULL)
            {
                hp = pMonster->hp;
                pMonster->hp = 1;
            }
            else
                msg<<src<<(uint8)EOTSpecial<<skillId<<target<<PRO_ERROR<<0<<0;
            if(hp != 0)
            {
                int addMp = (int)((hp - 1)*(1+skillLevel/40.0));
                DecreaseMp(target,addMp);
                msg<<src<<(uint8)EOTSpecial<<skillId<<target<<PRO_SUCCESS<<1<<addMp;
            }
        }
        msg<<useMp;
        return 1;
    
    //154	死亡缠绵 辅助技能	对己方对象使用后，对象使用物理攻击时，将必定出现连击效果							
    case ESTsiwangchanmian:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<1
                    <<(uint8)0<<0;
        }
        else
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<1
                <<(uint8)0<<0;
            SetState(target,EMSsiwangchanmian);
            SetTurn(target,0xff);
            m_members[target-1].stateAddData1 = skillLevel+50;
            //增加target物理攻击判断
        }
        msg<<useMp;
        return 1;
    
    //155	乾坤罩 辅助技能	对己方对象使用后，被施法者在受到物理攻击时，可免疫对方物理攻击一次，并对攻击者造成一定数值的反弹伤害							
    case ESTqiankunzhao:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<1
                    <<(uint8)0<<0;
        }
        else
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<1
                <<(uint8)0<<0;
            SetState(target,EMSqiankunzhao);
            SetTurn(target,0xff);
            m_members[target-1].stateAddData1 = skillLevel;
            //增加target物理攻击判断
        }
        msg<<useMp;
        return 1;
    
    //156	如意圈 辅助技能	使用后，被施用对象在一定回合数内抵御一定伤害的法术攻击							
    case ESTruyiquan:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<1
                    <<(uint8)0<<0;
        }
        else
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<1
                <<(uint8)0<<0;
            uint8 turn = (int)(2+skillLevel/30.0);
            if(turn > 5)
                turn = 5;
            uint8 times = (int)(1.5+skillLevel/30.0);
            int damage = skillLevel*250+750; 
            //GetQianKunInfo(skillLevel,turn,times,damage);
            SetState(target,EMSruyiquan);
            SetTurn(target,turn);
            m_members[target-1].stateAddData1 = times;
            m_members[target-1].stateAddData2 = damage;
            //增加target法术攻击判断
        }
        msg<<useMp;
        return 1;
    
    //157	神龙罩 辅助技能	使用后，被施用对象在一定回合数内抵御一定伤害的物理攻击
    case ESTshenlongzhao:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<1
                    <<(uint8)0<<0;
        }
        else
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<1
                <<(uint8)0<<0;
            SetState(target,EMSshenlongzhao);
            int turn = (int)(2+skillLevel/30.0);
            if(turn > 5)
                turn = 5;
            int times = (int)(1.5+skillLevel/30.0);
            int damage = skillLevel*250+750;
            SetTurn(target,turn);
            m_members[target-1].stateAddData1 = times;
            m_members[target-1].stateAddData2 = damage;
            //增加target物理攻击判断
        }
        msg<<useMp;
        return 1;
        
     //158	翻转乾坤 辅助技能	使用成功后，可改变目标相性							
    case ESTfanzhuanqiankun:
        if(tarState == EMSFrost)
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_ERROR<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_ERROR<<(uint8)0<<1
                    <<(uint8)0<<0;
        }
        else
        {
            //msg<<src<<(uint8)EOTSkill<<skillId<<target<<PRO_SUCCESS<<1<<(uint8)0<<(uint8)0<<(uint8)0;
            msg<<src<<(uint8)EOTSkill<<skillId<<(uint8)1<<target<<PRO_SUCCESS<<(uint8)0<<1
                <<(uint8)0<<0;
            //持续会合数 = 1 + INT（技能等级 / 40 ）
            //目标数 = 1.6 + INT（技能等级 / 50）
        }
        msg<<useMp;
        return 1;
    default:
        return 0;
    }
    return 0;
}

uint8 CFight::CatchMonster(uint8 src,uint8 target,CNetMessage &msg)
{
    /*
    +--------+-----+-----+--------+
    | TARGET | RES | LEN | REASON |
    +--------+-----+-----+--------+
    |    1   |  1  |  2  |   Var  |
    +--------+-----+-----+--------+
    */
    CUser *pUser = GetUser(src);
    
    if(pUser == NULL)
        return 0;
        
    msg<<src<<(uint8)EOTCatch<<target;
    SMonsterInst *pMonster = GetMonster(target);
    
    if(pMonster == NULL)
    {
        msg<<PRO_ERROR<<"无法捕捉";
        return 1;
    }
    
    if(pMonster->GetCeLue() == CL_MISSION21_BAOBAO)
    {
        if((pUser->GetTeam() != 0) && (pUser->GetTeam() != pUser->GetRoleId()))
        {
            msg<<PRO_ERROR<<"无法捕捉";
            return 1;
        }
        DelMember(target);
        msg<<PRO_SUCCESS;
        if(pUser == NULL)
            return 0;
        const char *pMission = pUser->GetMission(21);
        if(pMission == NULL)
            return 0;
        char buf[128];
        snprintf(buf,128,"%s",pMission);
        char *p[3];
        if(SplitLine(p,3,buf) == 3)
        {
            if(atoi(p[2]) != 0)
                return 0;
            bool updateMiss = false;
            uint16 drop[] = {
                2038,10	//九尾狐妖残骸    
                ,2039,15 	//九尾狐妖精魄    
                ,2040,18 	//九尾狐妖内丹    
                ,2041,28	//凤凰残骸	    
                ,2042,33 	//凤凰精魄	    
                ,2043,36 	//凤凰内丹	    
                ,2044,56	//玄武残骸	    
                ,2045,66	//玄武精魄	    
                ,2046,71 	//玄武内丹	    
                ,2047,86	//青龙残骸	    
                ,2048,96	//青龙精魄	    
                ,2049,99 	//青龙内丹	    
                ,2050,100 	//年兽内丹
            };
            uint8 r = Random(0,100);
            for(uint16 i = 0; i < sizeof(drop)/sizeof(uint16)/2; i++)
            {
                if(r <= drop[2*i+1])
                {
                    updateMiss = pUser->AddPackage(drop[2*i]);
                    break;
                }
            }
            if(updateMiss)
            {
                uint16 mId = atoi(p[0]);
                int monsterId = atoi(p[1]);
                snprintf(buf,sizeof(buf),"%d|%d|1",mId,monsterId);
                pUser->UpdateMission(21,buf);
            }
        }
        return 1;
    }
    
    if(pMonster->GetCeLue() == CL_GU_MU_TIAN_LU)
    {
        DelMember(target);
        msg<<PRO_SUCCESS;
        const char *info = pUser->GetMission(20);
        if(info == NULL)
            return 1;
        char buf[32];
        snprintf(buf,sizeof(buf),"%s",info);
        char *p[3];
        int num = SplitLine(p,3,buf);
        if(num == 3)
        {
            num = atoi(p[1]);
            int i = atoi(p[0]);
            int j = atoi(p[2]);
            if((i == 15) && (j < 10))
            {
                j++;
                snprintf(buf,sizeof(buf),"%d|%d|%d",i,num,j);
                pUser->UpdateMission(20,buf);
            }
        }    
        return 1;
    }
    else if(pMonster->GetCeLue() == CL_YOU_LANG)
    {
        DelMember(target);
        msg<<PRO_SUCCESS;
        const char *info = pUser->GetMission(20);
        if(info == NULL)
            return 1;
        char buf[32];
        snprintf(buf,sizeof(buf),"%s",info);
        char *p[2];
        int num = SplitLine(p,2,buf);
        if(num == 2)
        {
            num = atoi(p[1])+1;
            int i = atoi(p[0]);
            if(i == 13)
            {
                snprintf(buf,sizeof(buf),"%d|%d",i,num);
                pUser->UpdateMission(20,buf);
            }
        }    
        return 1;
    }
    else if(pMonster->GetCeLue() == CL_SHAN_TAN)
    {
        DelMember(target);
        msg<<PRO_SUCCESS;
        const char *info = pUser->GetMission(17);
        if(info == NULL)
            return 1;
        char buf[32];
        snprintf(buf,sizeof(buf),"%s",info);
        char *p[2];
        int num = SplitLine(p,2,buf);
        if(num == 2)
        {
            num = atoi(p[1])+1;
            int i = atoi(p[0]);
            if(i == 6)
            {
                snprintf(buf,sizeof(buf),"%d|%d",i,num);
                pUser->UpdateMission(17,buf);
            }
        }    
        return 1;
    }
    if((pMonster->GetCeLue() == CL_SHUYAO)
        && (pMonster->tmplId == 100))
    {
        if(Random(0,100) > 30)
        {
            msg<<PRO_ERROR<<"捕捉失败";
            return 1;
        }
        DelMember(target);
        msg<<PRO_SUCCESS;
        return 1;
    }
    
    if(m_type != EFTMeetMonster)
    {
        msg<<PRO_ERROR<<"不允许捕捉";
        return 1;
    }
    
    if((GetState(target) >= EMSDied) || IsEmpty(target))
    {
        msg<<PRO_ERROR<<"不允许捕捉";
        return 1;
    }
    
    if(pMonster->type == EMTTongLing)
    {
        msg<<PRO_ERROR<<"头领不能捕捉";
        return 1;
    }
    //if(pUser->IsAutoFight())
        //return 0;
    if(!UserCanGetPet(pUser,pMonster->tmplId))
    {
        msg<<PRO_ERROR<<"等级过高无法捕捉";
        return 1;
    }
    if(Random(0,100) > 30)
    {
        msg<<PRO_ERROR<<"捕捉失败";
        return 1;
    }
    uint8 type = 0;
    SharePetPtr pet = SingletonMonsterManager::instance().CreatePet(pMonster,false,&type); 
    if(!pUser->AddPet(pet))
    {
        msg<<PRO_ERROR<<"携带已达上限";
        return 1;
    }
    if(type == EPQdingji)
    {
        //天啊，XXX捉到了一只顶级的XX宝宝，大家恭喜他吧！
        char buf[128];
        snprintf(buf,127,"清明时节雨纷纷,%s上坟时捉到了一只顶级的%s",pUser->GetName(),pet->name);
        SysInfoToAllUser(buf);
        SaveCatchPet(pUser->GetRoleId(),pet.get());
    }
    else if(type == EPQxiyou)
    {
        //XXX捉到了一只稀有的XXX宝宝，真走运啊
        //清明时节雨纷纷，XXX上坟时捉到了一只XXX的XXX
        char buf[128];
        snprintf(buf,127,"清明时节雨纷纷,%s上坟时捉到了一只稀有的%s",pUser->GetName(),pet->name);
        SysInfoToAllUser(buf);
        SaveCatchPet(pUser->GetRoleId(),pet.get());
    }
    DelMember(target);
    msg<<PRO_SUCCESS;
    return 1;
}

uint8 CFight::ZhaoHuanPet(uint8 user,uint8 pet,CNetMessage &msg)
{
    if(OneGroupAllDie() != 0)
    {
        return 0;
    }
    
    CUser *pUser = GetUser(user);
    msg<<user<<(uint8)EOTCallUp;
    if(pUser == NULL)
    {
        msg<<PRO_ERROR;
        return 1;
    }
    uint8 chuZhanPet = pUser->GetChuZhanPet();
    uint8 qiPet = pUser->GetQiPet();
    if(qiPet == pet)
    {
        msg<<PRO_ERROR;
        return 1;
    }
    pUser->SetChuZhanPet(pet);
    SharePetPtr pPet;
    
    bool chuZhanGuo = (m_members[user-1].petMask & (1<<pet)) != 0;
    
    if((!pUser->GetChuZhanPet(pPet))
        || chuZhanGuo)
    {
        //SendSysInfo(pUser,"已出战过，不能召唤");
        pUser->SetChuZhanPet(chuZhanPet);
        msg<<PRO_ERROR;
        return 1;
    }
    if((pPet->level > pUser->GetLevel()) && (pPet->level - pUser->GetLevel() > 10))
    {
        SendSysInfo(pUser,"等级太高无法出战");
        pUser->SetChuZhanPet(chuZhanPet);
        msg<<PRO_ERROR;
        return 1;
    }
    else if(pPet->shouming <= 0)
    {
        SendSysInfo(pUser,"宠物寿命太低不能出战");
        pUser->SetChuZhanPet(chuZhanPet);
        msg<<PRO_ERROR;
        return 1;
    }
    
    pUser->SetChuZhanPet(pet);
    AddPet(pUser,pPet,user+1);
    msg<<PRO_SUCCESS<<(uint8)(user+1)<<pPet->tmplId<<pPet->name
        <<pPet->level<<pPet->maxHp<<pPet->hp<<pPet->maxMp<<pPet->mp;
    uint8 state = 2;
    if(pUser->GetGenSuiPet() == pUser->GetChuZhanPet())
        state = 3;
    pUser->UpdatePetInfo(pUser->GetChuZhanPet(),1,state);
    return 1;
}

void CFight::MonsterSkillCeLue(SMonsterInst *pMonster,uint8 pos)
{
    if(pos >= MAX_MEMBER)
        return;
    switch(pMonster->GetCeLue())
    {
    case CL_ZHAO_HUA_NIU:
        {
            uint8 num = 0;
            uint8 targets[GROUP2_BEGIN];
            GetMeGroup(pos+1,targets,num);
            if((num < 6) && ((m_fightTurn % 2) == 0))
            {
                m_members[pos].option = EOTZhaoHuan;
                break;
            }
        }
        break;
    case CL_GU_MU:
        {
            uint8 num = 0;
            uint8 targets[GROUP2_BEGIN];
            GetMeGroup(pos+1,targets,num);
            if(num < 6)
                m_members[pos].option = EOTZhaoHuan;
        }
        break;
    case CL_LIU_ER_MI_HOUH:
        {
            uint8 num = 0;
            uint8 targets[GROUP2_BEGIN];
            GetMeGroup(pos+1,targets,num);
            if((num < 6) && (Random(0,2) == 0))
            {
                m_members[pos].option = EOTZhaoHuan;
                break;
            }
            if(pMonster->hp*100/pMonster->maxHp < 20)
            {
                m_members[pos].option = EOTSkill;
                m_members[pos].para = 111;
                m_members[pos].target = pos+1;
                break;
            }
            num = 0;
            GetAnotherGroup(pos+1,targets,num);
            m_members[pos].option = EOTNormal;
            m_members[pos].target = RandSelect(targets,num);
            if(Random(0,1) == 1)
            {
                m_members[pos].option = EOTSkill;
                m_members[pos].para = 7;
            }
        }
        break;
    case CL_MEI_HAN:
        {
            m_members[pos].target = 7;
        }
        break;
    case CL_FENG_YA_LANG:
        {
            uint8 num = 0;
            uint8 targets[GROUP2_BEGIN];
            GetMeGroup(pos+1,targets,num);
            if((num < 6) && ((m_fightTurn % 2) == 0))
            {
                m_members[pos].option = EOTZhaoHuan;
                break;
            }
            uint16 skillId = 0;
            m_members[pos].option = EOTNormal;
            uint8 skillNum = pMonster->GetSkillNum();
            if(skillNum > 0)
            {
                if(Random(0,skillNum) != 0)
                {
                    skillId = pMonster->RandSelSkill();
                    m_members[pos].option = EOTSkill;
                    m_members[pos].para = skillId;
                }
            }
            m_members[pos].option = EOTNormal;
            uint8 target[GROUP2_BEGIN];
            num = 0;
            GetAnotherGroup(pos+1,target,num);
            if(num > 0)
            {
                m_members[pos].target = RandSelect(target,num);
            }
            else
            {
                m_members[pos].target = 0;
            }
        }
        break;
    case CL_FY_XIONG_LANG:
        {
            uint8 skillNum = pMonster->GetSkillNum();
            uint16 skillId = 0;
            m_members[pos].option = EOTNormal;
            if(skillNum > 0)
            {
                if(Random(0,skillNum) != 0)
                {
                    skillId = pMonster->RandSelSkill();
                    m_members[pos].option = EOTSkill;
                    m_members[pos].para = skillId;
                }
            }
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            if(num > 0)
            {
                m_members[pos].target = RandSelect(target,num);
            }
            else
            {
                m_members[pos].target = 0;
            }
            bool use155_153 = false;
            if(skillId == 153)
            {
                for(uint8 i = 0; i < num; i++)
                {
                    if((GetUser(target[i]) != NULL) && (GetState(target[i]) < EMSDied))
                    {
                        m_members[pos].target = target[i];
                        m_members[pos].para = skillId;
                        use155_153 = true;
                        break;
                    }
                }
            }
            if(skillId == 155)
            {
                m_members[pos].target = 0;
                uint8 target[GROUP2_BEGIN];
                uint8 num = 0;
                GetMeGroup(pos+1,target,num);
                
                for(uint8 i = 0; i < num; i++)
                {
                    SMonsterInst *pMonster = GetMonster(target[i]);
                    if((pMonster != NULL) && (pMonster->GetCeLue() == CL_FENG_YA_LANG)
                        && (GetState(target[i]) != EMSqiankunzhao))
                    {
                        m_members[pos].target = target[i];
                        m_members[pos].para = skillId;
                        use155_153 = true;
                        break;
                    }
                }
            }
            if(!use155_153)
            {
                if(Random(0,1) == 0)
                {
                    m_members[pos].option = EOTSkill;
                    m_members[pos].para = 24;
                }
                else 
                {
                    m_members[pos].option = EOTNormal;
                }
            }
        }
        break;
    case CL_FIRST_MIN_FANG:
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            uint32 recovery = 0xffffffff;
            for(uint8 i = 0; i < num; i++)
            {
                SPet *pPet = GetPet(target[i]);
                CUser *pUser = GetUser(target[i]);
                if(pPet != NULL)
                {
                    if((uint32)pPet->GetRecovery() < recovery)
                    {
                        m_members[pos].target = target[i];
                        recovery = pPet->GetRecovery();
                    }
                }
                else if(pUser != NULL)
                {
                    if((uint32)pUser->GetRecovery() < recovery)
                    {
                        m_members[pos].target = target[i];
                        recovery = pUser->GetRecovery();
                    }
                }
            }
        }
        break;
    case CL_XIAN_HU_LU:
        {
            uint8 skillNum = pMonster->GetSkillNum();
            uint16 skillId = 0;
            if(skillNum > 0)
            {
                if(Random(0,skillNum) != 0)
                {
                    skillId = pMonster->RandSelSkill();
                }
            }
            if(pMonster->hp*100/pMonster->maxHp < 30)
                skillId = 112;
            else if(skillId == 112)
                skillId = 7;
            if(skillId != 0)
            {
                m_members[pos].option = EOTSkill;
                if(skillId == 112)
                {
                    m_members[pos].target = pos+1;
                }
                m_members[pos].para = skillId;
            }
            else
            {
                m_members[pos].option = EOTNormal;
                m_members[pos].para = 0;
            }
            if(m_members[pos].para != 112)
            {
                uint8 target[GROUP2_BEGIN];
                uint8 num = 0;
                GetAnotherGroup(pos+1,target,num);
                if(num > 0)
                {
                    m_members[pos].target = RandSelect(target,num);
                }
                else
                {
                    m_members[pos].target = 0;
                }
            }
        }
        break;
    case CL_SHUYAO:
        {
            uint8 skillNum = pMonster->GetSkillNum();
            uint16 skillId = 0;
            if(skillNum > 0)
            {
                if(Random(0,skillNum) != 0)
                {
                    skillId = pMonster->RandSelSkill();
                }
            }
            if(pMonster->hp * 100/pMonster->maxHp < 20) 
            {
                skillId = 112;
            }
            else if(skillId == 112)
            {
                skillId = 0;
            }
            if(skillId != 0)
            {
                m_members[pos].option = EOTSkill;
                if(skillId == 112)
                {
                    m_members[pos].target = pos+1;
                }
                m_members[pos].para = skillId;
            }
            else
            {
                m_members[pos].option = EOTNormal;
            }
            if((skillId == 0) || (skillId != 112))
            {
                uint8 target[GROUP2_BEGIN];
                uint8 num = 0;
                GetAnotherGroup(pos+1,target,num);
                if(num > 0)
                {
                    m_members[pos].target = RandSelect(target,num);
                }
                else
                {
                    m_members[pos].target = 0;
                }
            }
            if(!m_useZhaoHuanSkill && (pMonster->hp*100/pMonster->maxHp < 30))
            {
                m_members[pos].option = EOTZhaoHuan;
                m_useZhaoHuanSkill = true;
            }
        }
        break;
    case FIRST_HULUO:
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            for(uint8 i = 1; i <= num; i++)
            {
                SPet *pPet = GetPet(i);
                if((pPet != NULL) && (pPet->tmplId == 100))
                {
                    m_members[pos].target = i;
                    break;
                }
            }
        }
        break;
    case CL_LAN_RUO_BOS:
        {
            m_members[pos].option = EOTSkill;
            m_members[pos].para = pMonster->RandSelSkill();
            if(m_members[pos].para == 111)
                m_members[pos].target = pos+1;
            else
            {
                uint8 target[GROUP2_BEGIN];
                uint8 num = 0;
                GetAnotherGroup(pos+1,target,num);
                if(num > 0)
                {
                    m_members[pos].target = RandSelect(target,num);
                }
                else
                {
                    m_members[pos].target = 0;
                }
            }
        }
        break;
    case CE_MEI_YING:
        {
            m_members[pos].option = EOTSkill;
            m_members[pos].para = pMonster->RandSelSkill();
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            if(num > 0)
            {
                m_members[pos].target = RandSelect(target,num);
            }
            else
            {
                m_members[pos].target = 0;
            }
        }
        break;
    case CL_LAN_RUO_LAOLAO:
        {
            bool zhaohuan = false;
            for(uint8 i = 1; i <= 6; i++)
            {
                if((GetMonster(i) == NULL) || (GetState(i) == EMSDied))
                {
                    zhaohuan = true;
                    break;
                }
            }
            if(zhaohuan)
            {
                m_members[pos].option = EOTZhaoHuan;
                m_useZhaoHuanSkill = false;
                m_members[pos].para = 0;
                m_members[pos].target = 0;
            }
            else
            {
                m_members[pos].option = EOTSkill;
                m_members[pos].para = 111;
                m_members[pos].target = pos+1;
            }
        }
        break;
    case CE_HUAN_YING:
        /*if((!m_useSpeekSkill) && pMonster->recovery == MAX_INT)
        {
            m_members[pos].option = EOTSpeek;
            m_members[pos].para = 0;
            m_members[pos].target = 0;
            m_useSpeekSkill = true;
        }*/
        if((pMonster->recovery < MAX_INT) && (!m_useZhaoHuanSkill))
        {
            m_members[pos].option = EOTZhaoHuan;
            m_members[pos].para = 0;
            m_members[pos].target = 0;
            m_useZhaoHuanSkill = true;
        }
        break;
    case CE_LUE_BA_XIA:
        if(Random(0,100) <= 10)
        {
            m_members[pos].option = EOTSkill;
            m_members[pos].para = 155;
            m_members[pos].target = pos+1;
        }
        else
        {
            m_members[pos].option = EOTNormal;
            m_members[pos].para = 0;
        }
        break;
    case CE_LUE_CHAO_FENG:
        if(Random(0,100) <= 10)
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            if(num > 0)
            {
                m_members[pos].target = RandSelect(target,num);
            }
            else
            {
                m_members[pos].target = 0;
            }
            m_members[pos].option = EOTSkill;
            m_members[pos].para = 53;
        }
        break;
    case CE_LUE_QI_LING:
        if(((pMonster->hp < pMonster->maxHp*0.5)||(pMonster->hp < pMonster->maxHp*0.3))
            && (pMonster->UseSkill(112) > 0))
        {
            m_members[pos].option = EOTSkill;
            m_members[pos].para = 112;
            m_members[pos].target = pos+1;
        }
        else if(Random(0,100) <= 20)
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            if(num > 0)
            {
                m_members[pos].target = RandSelect(target,num);
            }
            else
            {
                m_members[pos].target = 0;
            }
            m_members[pos].option = EOTSkill;
            if(Random(0,100) < 50)
                m_members[pos].para = 53;
            else
                m_members[pos].para = 57;
        }
        else
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            if(num > 0)
            {
                m_members[pos].target = RandSelect(target,num);
            }
            else
            {
                m_members[pos].target = 0;
            }
            m_members[pos].option = EOTSkill;
            m_members[pos].para = 15;
        }
        break;
    case CL_TAO_PAO:
        //if(Random(0,100) <= 20)
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            if(num > 0)
            {
                m_members[pos].target = RandSelect(target,num);
            }
            else
            {
                m_members[pos].target = 0;
            }
            //cout<<(int)pos<<":"<<(int)GetState(pos+1)<<endl;
            if(GetState(pos+1) == EMSForget)
            {
                m_members[pos].option = EOTNormal;
                m_members[pos].para = 0;
            }
            else
            {
                m_members[pos].option = EOTSpeek;
                m_members[pos].para = SKILL_TAO_PAO;
            }
        }
        break;
    /*case CL_MISSION21_BOSS:
        if(m_fightTurn == 0)
        {
            m_members[pos].option = EOTSpeek;
            m_members[pos].para = MISSION21_BOSS;
        }
        break;*/
    case CL_SHAN_TAN:
        m_members[pos].option = EOTSpeek;
        m_members[pos].para = SHAOTAO_TAO_PAO;
        break;
    case CL_ONLY_LANG:
        m_members[pos].option = EOTSpeek;
        m_members[pos].para = YUGUAI_TAO_PAO;
        break;
    case CL_ZI_BAO:
        //if(Random(0,100) <= 50)
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            if(num > 0)
            {
                m_members[pos].target = RandSelect(target,num);
            }
            else
            {
                m_members[pos].target = 0;
            }
            if(GetState(pos+1) == EMSForget)
            {
                m_members[pos].option = EOTNormal;
                m_members[pos].para = 0;
            }
            else
            {
                m_members[pos].option = EOTSpecial;
                m_members[pos].para = SKILL_ZI_BAO;
            }
        }
        break;
    case ONLY_HUN_LUAN:
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            m_members[pos].target = RandSelect(target,num);
            m_members[pos].option = EOTSkill;
            m_members[pos].para = 61;
        }
        break;
    case SUN_SHI_NAIJIU:
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            uint8 users[GROUP2_BEGIN];
            uint8 userNum = 0;
            for(uint8 i = 0; i < num; i++)
            {
                if(GetUser(target[i]) != NULL)
                {
                    users[userNum] = target[i];
                    userNum++;
                }
            }
            if(userNum > 0)
            {
                m_members[pos].target = RandSelect(users,userNum);
            }
            else
            {
                m_members[pos].target = RandSelect(target,num);
            }
            m_members[pos].option = EOTNormal;
            m_members[pos].para = 0;
            CUser *pUser = GetUser(m_members[pos].target);
            if(pUser != NULL)
                pUser->DecreaseAllNaiJiu(pMonster->addData);
            SPet *pPet = GetPet(m_members[pos].target);
            if((pPet != NULL) && (pPet->kaiJia.tmplId != 0) && (pPet->kaiJia.naijiu != 0))
            {
                CUser *pUser = GetUser(m_members[pos].target-1);
                if(pUser != NULL)
                    pUser->DesKaiJiaNaijiu(pMonster->addData);
            }
        }
        break;
    case ONLY_DU_57:
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            m_members[pos].target = RandSelect(target,num);
            m_members[pos].option = EOTSkill;
            m_members[pos].para = 57;
        }
        break;
    case ONLY_SKILL_22:
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            m_members[pos].target = RandSelect(target,num);
            m_members[pos].option = EOTSkill;
            if(Random(0,1) == 0)
                m_members[pos].para = 22;
            else
                m_members[pos].para = 24;
        }
        break;
    case ONLY_QIANKUN_SHENLONG:
        {
            if((m_members[pos].state != EMSshenlongzhao) && (m_members[pos].state != EMSruyiquan))
            {
                if(pMonster->addData == 0)
                {
                    m_members[pos].target = pos+1;
                    m_members[pos].option = EOTSkill;
                    m_members[pos].para = 157;
                    pMonster->addData = 1;
                }
                else if(pMonster->addData == 1)
                {
                    m_members[pos].target = pos+1;
                    m_members[pos].option = EOTSkill;
                    m_members[pos].para = 156;
                    pMonster->addData = 0;
                }
            }
            else
            {
                uint8 target[GROUP2_BEGIN];
                uint8 num = 0;
                GetAnotherGroup(pos+1,target,num);
                m_members[pos].target = RandSelect(target,num);
                m_members[pos].option = EOTNormal;
                m_members[pos].para = 0;
            }
        }
        break;
    case ONLY_QIANKUN_SHENLONG_YX:
        {
            if((m_members[pos].state != EMSshenlongzhao) && (m_members[pos].state != EMSruyiquan))
            {
                if(pMonster->addData == 0)
                {
                    m_members[pos].target = pos+1;
                    m_members[pos].option = EOTSkill;
                    m_members[pos].para = 157;
                    pMonster->addData = 1;
                }
                else if(pMonster->addData == 1)
                {
                    m_members[pos].target = pos+1;
                    m_members[pos].option = EOTSkill;
                    m_members[pos].para = 156;
                    pMonster->addData = 0;
                }
            }
            else
            {
                uint8 target[GROUP2_BEGIN];
                uint8 num = 0;
                GetAnotherGroup(pos+1,target,num);
                m_members[pos].target = RandSelect(target,num);
                m_members[pos].option = EOTSkill;
                if((pMonster->GetSkillLevel(19) > 0) && (m_fightTurn % 6 == 0))
                {
                    m_members[pos].para = 19;
                }
                else
                {
                    m_members[pos].para = 152;
                }
            }
        }
        break;
    case USE_SKILL_111:
        if(pMonster->hp * 100 / pMonster->maxHp <= 50)
        {
            m_members[pos].target = pos+1;
            m_members[pos].option = EOTSkill;
            m_members[pos].para = 111;
        }
        else
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            m_members[pos].target = RandSelect(target,num);
            m_members[pos].option = EOTNormal;
            m_members[pos].para = 0;
        }
        break;
    case SHI_YONG_53_61:
        if(m_fightTurn % 5 == 0)
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            m_members[pos].target = RandSelect(target,num);
            m_members[pos].option = EOTSkill;
            if(Random(0,1) == 0)
                m_members[pos].para = 53;
            else 
                m_members[pos].para = 61;
        }
        break;
    case FIRST_QI_LING:
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            m_members[pos].target = RandSelect(target,num);
            m_members[pos].option = EOTSkill;
        }
        break;
    //十回合后战斗结束(自身死亡)
    case FIRST_QI_LING_MONSTER:
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            for(uint8 i = 0; i < num; i++)
            {
                SMonsterInst *pMonster = GetMonster(target[i]);
                if(pMonster->tmplId == 102)
                {
                    m_members[pos].target = target[i];
                    break;
                }
            }
            m_members[pos].option = EOTNormal;
            if(m_fightTurn >= 10)
            {
                SetState(pos+1,EMSDied);
                SetState(m_members[pos].target,EMSDied);
            }
        }
        break;
    //十回合后战斗结束(自身死亡)
    case FIRST_XIAN_REN_MONSTER:
        {
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            for(uint8 i = 0; i < num; i++)
            {
                SMonsterInst *pMonster = GetMonster(target[i]);
                if(pMonster->tmplId == 45)
                {
                    m_members[pos].target = target[i];
                    break;
                }
            }
            m_members[pos].option = EOTNormal;
            if(m_fightTurn == 9)
            {
                CNetMessage chat;
                chat.SetType(PRO_MSG_CHAT);
                chat<<(uint8)5<<(int)(pos+1)<<pMonster->name<<"准备同归于尽吧！";
                BroadcastMsg(chat);
            }
            if(m_fightTurn >= 10)
            {
                SetState(pos+1,EMSDied);
                SetState(m_members[pos].target,EMSDied);
            }
        }
        break;
    }
}

void CFight::SetAllUserDie()
{
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        if(GetUser(pos) != NULL)
        {
            SetState(pos,EMSDied);
        }
        else if(GetPet(pos) != NULL)
        {
            SetState(pos,EMSDied);
        }
    }
}

void CFight::ChangeHuanYinPos()
{
    uint8 num = 0;
    uint8 pos = 0;
    for(uint8 i = 0; i < GROUP2_BEGIN; i++)
    {
        SMonsterInst *pMonster = GetMonster(i+1);
        if(pMonster != NULL)
        {
            num++;
            if(pMonster->recovery < MAX_INT)
                pos = i;
        }
    }
    if(num == 6)
    {
        uint8 r = 0;
        while((r = Random(0,5)) == pos)
        {
        }
        std::swap(m_members[r],m_members[pos]);
    }
}

void CFight::CalculateFightResult(list<uint32> &userList)
{
    if(GetSysTime() - m_turnBegin < 3)
        return;
    m_turnBegin = GetSysTime();
    m_useSpeekSkill = false;
    
    //防御、保护优先,并且monster选择行动方式
    for(uint8 pos = 0; pos < MAX_MEMBER; pos++)
    {
        if(m_members[pos].state >= EMSDied)
            continue;
        if(m_members[pos].memPtr.empty())
            continue;
        bool monster = false;
        
        if(m_members[pos].memPtr.type() == typeid(ShareMonsterPtr))
        {
            monster = true;
        }
        else if(m_members[pos].memPtr.type() == typeid(ShareUserPtr))
        {
            if(pos == MAX_MEMBER-1)
                break;
            CUser *pUser = (boost::any_cast<ShareUserPtr>(m_members[pos].memPtr)).get();
            if(pUser != NULL)
            {
                if(pUser->IsAutoFight())
                {
                    m_members[pos].select = true;
                    pUser->GetAutoFightOp(m_members[pos].option,
                                          m_members[pos].para,
                                          m_members[pos].target,
                                          m_members[pos+1].option,
                                          m_members[pos+1].para,
                                          m_members[pos+1].target);
                }
                else if(!m_members[pos].select && !m_members[pos].useSkill23)
                {
                    m_members[pos].option = 0;
                    m_members[pos].para = 0;
                    m_members[pos].target= 0;
                    m_members[pos+1].option = 0;
                    m_members[pos+1].para = 0;
                    m_members[pos+1].target= 0;
                }
            }
        }
        SMonsterInst *pMonster = GetMonster(pos+1);
        if(pMonster != NULL)
        {
            //如果是怪，选择行动方式
            
            //1 30 60
            //1 2 4 
            int skillId = 0;
            uint8 skillNum = pMonster->GetSkillNum();
            if(skillNum > 0)
            {
                if(pMonster->onlySkill)
                {
                    skillId = pMonster->RandSelSkill();
                }
                else if(Random(0,skillNum) != 0)
                {
                    skillId = pMonster->RandSelSkill();
                }
            }
            else if(pMonster->xiang != 0)
            {
                uint8 skillNum = 1;
                if(pMonster->level >= 60)
                    skillNum = 3;
                else if(pMonster->level >= 30)
                    skillNum = 2;
                int r = Random(0,skillNum);
                switch(r)
                {
                case 1:
                    skillId = 4*(pMonster->xiang-1)+1;
                    break;
                case 2:
                    skillId = 4*(pMonster->xiang-1)+2;
                    break;
                case 3:
                    skillId = 4*(pMonster->xiang-1)+4;
                    break;
                }
            }
            //GetRecoveryImporve(src);
            if(pMonster->GetSkillLevel(112) > 0)
            {
                if((m_members[pos].hpImprove == 0) 
                    && (pMonster->hp < pMonster->maxHp*0.2)
                    && (pMonster->UseSkill(112) > 0))
                {
                    skillId = 112;
                }
                else
                {
                    skillId = 0;
                }
            }
            if((skillId != 112) && (pMonster->GetSkillLevel(120) > 0))
            {
                if((m_members[pos].normalDamageImprove == 0) && (pMonster->hp < pMonster->maxHp/2))
                {
                    skillId = 120;
                }
                else
                {
                    skillId = 0;
                }
            }
            if(pMonster->GetSkillLevel(119) > 0)
            {
                if((m_members[pos].normalDamageImprove == 0)
                    && ((m_fightTurn % 5) == 0))
                {
                    skillId = 119;
                }
                else
                {
                    skillId = 0;
                }
            }
            
            if(pMonster->GetSkillLevel(156) > 0)
            {
                if(m_members[pos].state != EMSruyiquan)
                {
                    skillId = 156;
                }
                else if(skillId == 156)
                {
                    skillId = 0;
                }
            }
            if(pMonster->GetSkillLevel(157) > 0)
            {
                if(m_members[pos].state != EMSshenlongzhao)
                {
                    skillId = 157;
                }
                else if(skillId == 157)
                {
                    skillId = 0;
                }
            }   
            if(skillId != 0)
            {
                m_members[pos].option = EOTSkill;
                m_members[pos].para = skillId;
            }
            else
            {
                m_members[pos].option = EOTNormal;
                m_members[pos].para = 0;
            }
            
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            
            if((skillId == 120) || (skillId == 112) || (skillId == 119)
                || (skillId == 156) || (skillId == 157)
                || (skillId == 111))
            {
                m_members[pos].target = pos+1;
            }
            else
            {
                if(num > 0)
                {
                    m_members[pos].target = RandSelect(target,num);
                }
                else
                {
                    m_members[pos].target = 0;
                }
            }
            MonsterSkillCeLue(pMonster,pos);
            if(GetState(pos+1) == EMSConfusion)
            {
                m_members[pos].option = EOTNormal;
                m_members[pos].para = 0;
            }
        }
        else if(m_members[pos].option == 0)
        {//玩家超时没有选择
            m_members[pos-1].turnBegin = 0;
            m_members[pos].option = EOTNormal;
            m_members[pos].para = 0;
            uint8 target[GROUP2_BEGIN];
            uint8 num = 0;
            GetAnotherGroup(pos+1,target,num);
            if(num > 0)
            {
                m_members[pos].target = RandSelect(target,num);
            }
            else
            {
                m_members[pos].target = 0;
            }
        }
        else
        {
            if(m_members[pos].option == EOTRecovery)// 防御 OPV=0
            {
                //SetState(pos+1,EMSRecovery);
                CUser *pUser = GetUser(pos+1);
                //SMonsterInst *pMonster = GetMonster(pos+1);
                SPet *pPet = GetPet(pos+1);
                if(pUser != NULL)
                {
                    m_members[pos].recoveryImporve = 1;//CalculateRate(pUser->GetRecovery(),10,100);
                    //pUser->AddRecovery(m_members[pos].recoveryImporve);
                }
                else if(pPet != NULL)
                {
                    m_members[pos].recoveryImporve = 1;//CalculateRate(pPet->recovery,10,100);
                    //pPet->recovery += m_members[pos].recoveryImporve;
                }
                m_members[pos].recoveryTurn = 4;
            }
            else if(m_members[pos].option == EOTProtect)// 保护)
            {
                //SetState(GetTarget(pos+1),EMSProtected);
                SetProtecter(GetTarget(pos+1),pos+1);
            }
        }
    }
    
    uint8 allMem[MAX_MEMBER];
    uint8 num = 0;
    GetAllMember(allMem,num);
    
    SortBySpeed(allMem,num);
    
    CNetMessage msg;
    msg.SetType(PRO_BATTLE);
    msg<<(uint8)1;//表示战斗结果
    uint8 damageNum = 0;
    uint16 damageNumPos = msg.GetDataLen();
    msg<<damageNum;
    
    for(uint8 i = 0; i < num; i++)
    {
        uint8 option = 0;
        int para = 0;
        uint8 target = 0;
        GetOption(allMem[i],option,para,target);
        if((GetState(allMem[i]) >= EMSDied) || IsEmpty(allMem[i]))
        {
            continue;
        }
        uint8 srcState = GetState(allMem[i]);
        if(srcState == EMSEnvenom)
        {
            //DecreaseHp(allMem[i],CalculateRate(GetMaxHp(allMem[i]),20,100));
            DecreaseHp(allMem[i],m_members[allMem[i]-1].stateAddData1);
#ifdef DEBUG
            cout<<"下毒减血:"<<m_members[allMem[i]-1].stateAddData1<<endl;
#endif
        }
        
        switch(option)
        {
        case EOTProtect:// 保护)
            if((srcState < EMSDied) 
                && (srcState != EMSConfusion)//混乱
                && (srcState != EMSDizzy)//昏睡
                && (srcState != EMSFrost))//冰冻
            {
                msg<<allMem[i]<<(uint8)EOTProtect<<PRO_SUCCESS;
                damageNum += 1;
            }
            break;
        case EOTRecovery:// 防御 OPV=0
            if((srcState < EMSDied) && (srcState != EMSConfusion) && (srcState != EMSFrost))
            {
                msg<<allMem[i]<<(uint8)EOTRecovery<<PRO_SUCCESS;
                damageNum += 1;
            }
            break;
        default://玩家超时，或者是怪物
            break;
        }
    }
    
    for(uint8 i = 0; i < num; i++)
    {
        if(OneGroupAllDie() != 0)
            break;
        uint8 option = 0;
        int para = 0;
        uint8 target = 0;
        GetOption(allMem[i],option,para,target);
        uint8 state = GetState(allMem[i]);
        
        if((state == EMSFrost) //|| (GetState(target) == EMSFrost)
            || (state == EMSDizzy)
            || (state >= EMSDied) 
            || IsEmpty(allMem[i]))
        {
            continue;
        }
        //if(state >= EMSDied)
            //cout<<"已死亡"<<endl;
#ifdef DEBUG
        cout<<endl;
        CUser *pUser = GetUser(allMem[i]);
        if(pUser != NULL)
            cout<<"src name:"<<pUser->GetName();
        cout<<" src pos:"<<(int)allMem[i]<<" state:"<<(int)state<<endl;
        cout<<"target pos:"<<(int)target<<" state:"<<(int)GetState(target)<<endl;
        cout<<endl;
#endif
        if(state == EMSConfusion)
        {
            uint8 member[MAX_MEMBER];
            uint8 num = 0;
            GetLiveMember(member,num);
            if(!GetSkill23State(allMem[i]))
            {
                option = EOTNormal;
                para = 0;
            }
            GetExcept(allMem[i],member,num);
            target = RandSelect(member,num);
            //tarState = GetState(target);
        }
        switch(option)
        {
        case EOTNormal://
            damageNum += NormalButtle(allMem[i],target,msg);
            break;
        case EOTSkill://
            damageNum += SkillButtle(allMem[i],target,para,msg);
            break;
        case EOTSpecial:
            damageNum += SpecialSkill(allMem[i],target,para,msg);
            break;
        case EOTSpeek:
            if(GetState(allMem[i]) == EMSForget)
                break;
            damageNum++;
            if(para == SKILL_TAO_PAO)
            {
                if(Random(0,4) == 0)
                {
                    msg<<allMem[i]<<(uint8)EOTSpeek<<(uint8)para<<PRO_SUCCESS<<"修仙的来了，大家快跑";
                    SetAllUserDie();
                }
                else
                {
                    msg<<allMem[i]<<(uint8)EOTSpeek<<(uint8)para<<PRO_ERROR<<"修仙的来了，大家快跑";
                }
            }
            else if(para == MISSION21_BOSS)
            {
                msg<<allMem[i]<<(uint8)EOTSpeek<<(uint8)SKILL_TAO_PAO<<PRO_SUCCESS<<"打倒我能得到更多奖励";
            }
            else if(para == SHAOTAO_TAO_PAO)
            {
                msg<<allMem[i]<<(uint8)EOTSpeek<<(uint8)SKILL_TAO_PAO<<PRO_SUCCESS<<"我是山桃宝宝";
                DelMember(allMem[i]);
                //SetAllUserDie();
            }
            else if(para == YUGUAI_TAO_PAO)
            {
                msg<<allMem[i]<<(uint8)EOTSpeek<<(uint8)SKILL_TAO_PAO<<PRO_SUCCESS<<"";
                DelMember(allMem[i]);
            }
            else
            {
                msg<<allMem[i]<<(uint8)EOTSpeek<<(uint8)para<<PRO_SUCCESS<<"看我移形换位";
                ChangeHuanYinPos();
            }
            break;
        case EOTZhaoHuan:
            {
                SMonsterInst *pMonster = GetMonster(allMem[i]);
                if((pMonster != NULL) && (pMonster->GetCeLue() == CL_GU_MU))
                {
                    damageNum += ZhaoHuanGuMu(allMem[i],msg);
                }
                else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_ZHAO_HUA_NIU))
                {
                    damageNum += ZhaoHuanNiu(allMem[i],msg);
                }
                else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_SHUYAO))
                {
                    damageNum += ZhaoHuanHuLu(allMem[i],msg);
                }
                else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_LIU_ER_MI_HOUH))
                {
                    damageNum += ZhaoHuanShiHou(allMem[i],msg);
                }
                else if((pMonster != NULL) && (pMonster->GetCeLue() == CL_FENG_YA_LANG))
                {
                    damageNum += ZhaoHuanLang(allMem[i],msg);
                }
                else
                {
                    damageNum += ZhaoHuanSkill(allMem[i],msg);
                }
            }
            break;
        case EOTUseItem:// = 3,// 使用道具 OPV=道具所在背包索引
            {
                if((OneGroupAllDie() > 0) || (GetState(target) >= EMSDied))
                    break;
                CUser *pUser  = GetUser(allMem[i]);
                int addHp = -1,addMp = -1;
                if(pUser == NULL)
                {
                    pUser = GetUser(allMem[i] - 1);
                }
                if(pUser == NULL)
                    break;
                CUser *pTarUser = GetUser(target);
                SPet *pTarPet = GetPet(target);
                SMonsterInst *pMonster = GetMonster(target);
                if(pMonster != NULL)
                {
                    if(pUser == NULL)
                        pUser = GetUser(allMem[i]-1);
                    if(pUser == NULL)
                        break;
                    pUser->FightUseItem(para,addHp,addMp,pMonster->maxHp,
                                        pMonster->maxMp,pMonster->hp,pMonster->mp,false);
                    if(addHp > 0)
                    {
                        pMonster->hp += addHp;
                        //pUser->UpdatePetInfo(pMonster->GetChuZhanPet(),5,pMonster->hp);
                    }
                    if(addMp > 0)
                    {
                        pMonster->mp += addMp;
                        //pUser->UpdatePetInfo(pMonster->GetChuZhanPet(),6,pMonster->mp);
                    }
                    if((addHp != -1) || (addMp != -1))
                    {
                        msg<<allMem[i]<<(uint8)EOTUseItem<<target<<addHp<<addMp;
                        damageNum++;
                    }
                }
                else if(pTarUser != NULL)
                {
                    bool pkQieCuo = false;
                    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo)
                        || (m_type == EFTMatch))
                        pkQieCuo = true;
                    pUser->FightUseItem(para,addHp,addMp,pTarUser->GetMaxHp(),
                                        pTarUser->GetMaxMp(),pTarUser->GetHp(),
                                        pTarUser->GetMp(),pkQieCuo);
                    if(addHp > 0)
                        pTarUser->AddHp(addHp);
                    if(addMp > 0)
                        pTarUser->AddMp(addMp);
                    if((addHp != -1) || (addMp != -1))
                    {
                        msg<<allMem[i]<<(uint8)EOTUseItem<<target<<addHp<<addMp;
                        damageNum++;
                    }
                }
                else if(pTarPet != NULL)
                {
                    pTarUser = GetUser(target-1);
                    if(pTarUser == NULL)
                        break;
                    bool pkQieCuo = false;
                    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo)
                        || (m_type == EFTMatch))
                        pkQieCuo = true;
                    pUser->FightUseItem(para,addHp,addMp,pTarPet->maxHp,
                                        pTarPet->maxMp,pTarPet->hp,pTarPet->mp,pkQieCuo);
                    if(addHp > 0)
                    {
                        pTarPet->hp += addHp;
                        pUser->UpdatePetInfo(pTarUser->GetChuZhanPet(),5,pTarPet->hp);
                    }
                    if(addMp > 0)
                    {
                        pTarPet->mp += addMp;
                        pUser->UpdatePetInfo(pTarUser->GetChuZhanPet(),6,pTarPet->mp);
                    }
                    if((addHp != -1) || (addMp != -1))
                    {
                        msg<<allMem[i]<<(uint8)EOTUseItem<<target<<addHp<<addMp;
                        damageNum++;
                    }
                }
                break;
            }
        case EOTCallUp:// = 5,// 召唤 OPV=宠物索引
            damageNum += ZhaoHuanPet(allMem[i],para,msg);
            break;
        case EOTCallBack:// = 6,// 召回 
            {
                CUser *pUser  = GetUser(allMem[i]);
                if(pUser != NULL)
                {
                    if(pUser->HaveBitSet(157))
                    {
                        msg<<allMem[i]<<(uint8)EOTCallBack<<PRO_ERROR;
                        damageNum += 1;
                        break;
                    }
                    if(pUser->IsAutoFight())
                        break;
                    uint8 state = 0;
                    if(pUser->GetGenSuiPet() == pUser->GetChuZhanPet())
                        state = 1;
                    SPet *pPet = GetPet(allMem[i]+1);
                    if((pPet != NULL) && (pPet->bangDing == 1))
                    {
                        state |= 0x4;
                    }
                    pUser->UpdatePetInfo(pUser->GetChuZhanPet(),1,state);
                    pUser->SetChuZhanPet(0xff);
                }
                DelMember(allMem[i] + 1);
                msg<<allMem[i]<<(uint8)EOTCallBack<<PRO_SUCCESS;
                damageNum += 1;
                break;
            }
        case EOTCatch:// = 8,// 捕捉
            {
                if((GetState(target) >= EMSDied) || IsEmpty(target))
                {
                    uint8 targets[GROUP2_BEGIN];
                    uint8 num = 0;
                    GetAnotherGroup(allMem[i],targets,num);
                    GetExcept(target,targets,num);
                    if(num <= 0)
                        break;
                    target = RandSelect(targets,num);
                }
                damageNum += CatchMonster(allMem[i],target,msg);
                break;
            }
        case EOTEscape:// = 9// 逃跑
            {
                if((m_type == EFTMatch) || (!m_canTaoPao))
                {
                    msg<<allMem[i]<<(uint8)EOTEscape<<PRO_ERROR;
                    damageNum++;
                    break;
                }
                CUser *pUser = GetUser(allMem[i]);
                if(pUser == NULL)
                    break;
                if(pUser->IsAutoFight())
                    break;
                //逃跑成功率=92-0.016*用户等级*用户等级，最大值90，最小值10
                int val = (int)(92-0.016*pUser->GetLevel()*pUser->GetLevel());
                if(val > 90)
                    val = 90;
                else if(val < 10)
                    val = 10;
                if(Random(0,100) > val)
                {
                    msg<<allMem[i]<<(uint8)EOTEscape<<PRO_ERROR;
                    damageNum++;
                    break;
                }
                msg<<allMem[i]<<(uint8)EOTEscape<<PRO_SUCCESS;
                m_mutex.unlock();
                int script = 0;
                string call = pUser->GetCall(script);
            
                if(!call.empty())
                {
                    CCallScript *pCallScript = FindScript(script);//(name);
                    if(pCallScript != NULL)
                    {
                        pUser->SetCallScript(pCallScript->GetScriptId());
                        //cout<<"脚本战斗结束"<<script<<":"<<call.c_str()<<state<<endl;
                        pCallScript->Call(call.c_str(),"uif",pUser,2,this);
                    }
                }
                if(pUser->GetScene() != NULL)
                    pUser->GetScene()->TempLeaveTeam(pUser);
                m_mutex.lock();
                
                SetState(allMem[i],EMSEscape);
                int curHp = pUser->GetHp();
                pUser->Init();
                if(curHp != pUser->GetHp())
                {
                    pUser->SendUpdateInfo(9,pUser->GetHp());
                }
                damageNum++;
                if((GetPet(allMem[i]+1) != NULL) && (GetState(allMem[i]+1) < EMSDied))
                {
                    SetState(allMem[i]+1,EMSEscape);
                }
                const char *info = pUser->GetMission(20);
                if(info == NULL)
                    break;
                for(uint8 i = 1; i <= MAX_MEMBER; i++)
                {
                    SMonsterInst *pMonster = GetMonster(i);
                    if((pMonster != NULL) && (pMonster->GetCeLue() == CL_GU_MU))
                    {
                        char buf[32];
                        snprintf(buf,sizeof(buf),"%s",info);
                        char *p[3];
                        int num = SplitLine(p,3,buf);
                        if(num == 3)
                        {
                            num = atoi(p[1]);
                            int i = atoi(p[0]);
                            if(i == 15)
                            {
                                snprintf(buf,32,"15|%d|0",num);
                                pUser->UpdateMission(20,buf);
                            }
                        }
                        break;
                    }
                }
            }
            break;
        /*case EOTProtect:// 保护)
            msg<<allMem[i]<<(uint8)EOTProtect<<PRO_SUCCESS;
            damageNum += 1;
            break;
        case EOTRecovery:// 防御 OPV=0
            msg<<allMem[i]<<(uint8)EOTRecovery<<PRO_SUCCESS;
            damageNum += 1;
            break;*/
        default://玩家超时，或者是怪物
            break;
        }
        //一轮结束，清理本轮的数据
    }
    
    if(damageNum == 0)
        return;
        
    msg.WriteData(damageNumPos,&damageNum,sizeof(damageNum));
    
    CSocketServer &sock = SingletonSocket::instance();
    for(uint8 pos = 0; pos < MAX_MEMBER; pos++)
    {
        //if(GetState(pos+1) == EMSEscape)
            //continue;
        if(!m_members[pos].memPtr.empty() 
            && (m_members[pos].memPtr.type() == typeid(ShareUserPtr)))
        {
            CUser *pUser = (boost::any_cast<ShareUserPtr>(m_members[pos].memPtr)).get();                
            if(pUser->GetFightId() == 0)
                continue;
            sock.SendMsg(pUser->GetSock(),msg);
            //cout<<endl<<pUser->GetName()<<endl;
            if(m_members[pos].state == EMSEscape)
            {
                DelMember(pos+1);
                SendUserPos(pUser);
                pUser->SetFight(0,0);
                UpdateUserInfo(pUser,userList);
            }
        }
        TurnOver(pos+1);
    }
    msg.SetType(GUAGNZHAN_BATTLE);
    for(list<int>::iterator i = m_guanZhanSock.begin(); i != m_guanZhanSock.end(); i++)
    {
        sock.SendMsg(*i,msg);
    }
    m_fightTurn++;
    //TestFun();
}

void CFight::TestFun()
{
    cout<<endl<<"+++++++++++"<<endl;
    for(uint8 i = 1; i < 12; i++)
    {
        SMonsterInst *pMonster = GetMonster(i);
        if(pMonster != NULL)
        {
            cout<<(int)i<<"-----"<<pMonster->maxHp<<":"<<pMonster->hp<<endl;
        }
    }
    cout<<"+++++++++++"<<endl;
}
void CFight::Logout(uint8 pos)
{
    if(pos > MAX_MEMBER)
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    SetState(pos,EMSEscape);
    SPet *pPet = GetPet(pos+1);
    if(pPet != NULL)
    {
        SetState(pos+1,EMSEscape);
    }
    CNetMessage msg;
    msg.SetType(PRO_BATTLE);
    //离开战斗
    msg<<(uint8)2<<pos;
    
    CSocketServer &sock = SingletonSocket::instance();
    
    for(uint8 i = 0; i <= MAX_MEMBER; i++)
    {
        if(i != (pos-1))
        {
            CUser *pUser = GetUser(i);
            if(pUser != NULL)
                sock.SendMsg(pUser->GetSock(),msg);
        }
    }
}

void CFight::UserBattle(CNetMessage &msg,CUser *pUser)
{
    if(m_userOpTime == 0)
        m_userOpTime = GetSysTime();
        
    uint8 playOp = 0,petOp = 0,playTar = 0,petTar = 0;
    int playPara = 0,petPara = 0;
    msg>>playOp>>playPara>>playTar
        >>petOp>>petPara>>petTar;
        
    list<uint32> userList;
    if(m_pScene != NULL)
        m_pScene->GetUserList(userList);
        
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    uint8 pos = pUser->GetFightPos() - 1;
    
    if(playOp == 10)
    {//设置自动战斗
        SendUserSelect(pos+1);
        if(pos < MAX_MEMBER)
        {
            if(playPara == 1)
            {
                playOp = m_members[pos].option;
                playPara = m_members[pos].para;
                playTar = m_members[pos].target;
                petOp = m_members[pos+1].option;
                if(playOp == 0)
                {
                    playOp = EOTNormal;
                    petOp = EOTNormal;
                }
                SetOption(pUser->GetFightPos(),playOp,playPara,playTar);
                pUser->SetAutoFightTurn(25);//自动战斗轮次
                /*pUser->SetAutoFight(playOp,
                                      m_members[pos].para,
                                      m_members[pos].target,
                                      petOp,
                                      m_members[pos+1].para,
                                      m_members[pos+1].target);*/
                msg.ReWrite();
                msg.SetType(PRO_BATTLE);
                msg<<(uint8)4<<(uint8)pUser->GetAutoFightTurn();//通知客户端自动回合数
                CSocketServer &sock = SingletonSocket::instance();
                sock.SendMsg(pUser->GetSock(),msg);
                uint16 mask = 0;
                for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
                {
                    CUser *pUser = GetUser(pos);
                    if(pUser != NULL)
                        mask |= 1<<pos;
                }
                if((m_beginTurnMask == mask) && AllUserOption())
                {
                    CalculateFightResult(userList);
                }
            }
            else if(playPara == 0)
            {
                CancelAutoFight(pUser);
                m_members[pos].select = false;
            }
        }
    }
    else if(playOp == 11)
    {//开始下一回合
        if(pUser->IsAutoFight())
        {
            SendUserSelect(pUser->GetFightPos());
        }
        m_beginTurnMask |= (1<<pUser->GetFightPos());
        uint16 mask = 0;
        for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
        {
            CUser *pUser = GetUser(pos);
            if(pUser != NULL)
                mask |= 1<<pos;
        }
        //(AllUserAutoFight(mask)
        if(AllUserOption() && (mask == m_beginTurnMask))
        {
            CalculateFightResult(userList);
            m_beginTurnMask = 0;
        }
    }
    else
    {
        pUser->SaveAutoFight(playOp,playPara,playTar,petOp,petPara,petTar);
        SendUserSelect(pUser->GetFightPos());
        SetOption(pUser->GetFightPos(),playOp,playPara,playTar);
        if(petOp != 0)
            SetOption(pUser->GetFightPos()+1,petOp,petPara,petTar);
        uint16 mask = 0;
        for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
        {
            CUser *pUser = GetUser(pos);
            if(pUser != NULL)
                mask |= 1<<pos;
        }
        m_beginTurnMask |= (1<<pUser->GetFightPos());
        if(m_beginTurnMask == mask)
        {
            if(AllUserOption())
            {
                CalculateFightResult(userList);
                m_beginTurnMask = 0;
            }
        }
    }
}
void CFight::CancelAutoFight(CUser *pUser)
{
    if(pUser->IsAutoFight())
    {
        pUser->SetAutoFightTurn(0);
        CNetMessage msg;
        msg.SetType(PRO_BATTLE);
        msg<<(uint8)3;//通知客户端取消战斗
        CSocketServer &sock = SingletonSocket::instance();
        sock.SendMsg(pUser->GetSock(),msg);
    }
}

void CFight::SendUserSelect(uint8 pos)
{
    CNetMessage msg;
    msg.SetType(PRO_BATTLE);
    msg<<(uint8)6<<pos;
    CSocketServer &sock = SingletonSocket::instance();
    
    for(uint8 i = 0; i < MAX_MEMBER; i++)
    {
        CUser *pUser = GetUser(i+1);
        if((pUser != NULL) && (GetState(i+1) != EMSEscape))
        {
            sock.SendMsg(pUser->GetSock(),msg);
        }
    }
    msg.SetType(GUAGNZHAN_BATTLE);
    for(list<int>::iterator i = m_guanZhanSock.begin(); i != m_guanZhanSock.end(); i++)
    {
        sock.SendMsg(*i,msg);
    }
}

void CFight::GetAllMember(uint8 *arr,uint8 &num)
{
    num = 0;
    for(uint8 i = 0; i < MAX_MEMBER; i++)
    {
        if(!m_members[i].memPtr.empty())
        {
            arr[num] = i+1;
            num++;
        }
    }
}

void CFight::GetMeGroup(uint8 me,uint8 *arr,uint8 &num)
{
    uint8 i;
    num = 0;
    if(me <= GROUP2_BEGIN)
    {
        for(i = 0; i < GROUP2_BEGIN; i++)
        {
            if((!m_members[i].memPtr.empty()) && (m_members[i].state < EMSDied))
            {
                arr[num] = i+1;
                num++;
            }
        }
    }
    else
    {
        for(i = GROUP2_BEGIN; i < MAX_MEMBER; i++)
        {
            if((!m_members[i].memPtr.empty()) && (m_members[i].state < EMSDied))
            {
                arr[num] = i+1;
                num++;
            }
        }
    }
}

uint8 CFight::GetAnotherGroupNum(uint8 me)
{
    uint8 num = 0;
    uint8 i = 0;
    if(me <= GROUP2_BEGIN)
    {
        for(i = GROUP2_BEGIN; i < MAX_MEMBER; i++)
        {
            if(!m_members[i].memPtr.empty())
            {
                num++;
            }
        }
    }
    else
    {
        for(i = 0; i < GROUP2_BEGIN; i++)
        {
            if(!m_members[i].memPtr.empty())
            {
                num++;
            }
        }
    }
    return num;
}

void CFight::GetAnotherGroup(uint8 me,uint8 *arr,uint8 &num)
{
    uint8 i;
    num = 0;
    if(me <= GROUP2_BEGIN)
    {
        for(i = GROUP2_BEGIN; i < MAX_MEMBER; i++)
        {
            if((!m_members[i].memPtr.empty()) && (m_members[i].state < EMSDied))
            {
                arr[num] = i+1;
                num++;
            }
        }
    }
    else
    {
        for(i = 0; i < GROUP2_BEGIN; i++)
        {
            if((!m_members[i].memPtr.empty()) && (m_members[i].state < EMSDied))
            {
                arr[num] = i+1;
                num++;
            }
        }
    }
}

void CFight::GetExcept(uint8 except,uint8 *arr,uint8 &num)
{
    for(uint8 i = 0; i < num; i++)
    {
        if(except == arr[i])
        {
            memmove(arr+i,arr+i+1,num-i);
            num--;
            return;
        }
    }
}

struct SSortMemberSpeed
{
    bool operator()(const uint8 &m1,const uint8 &m2)
    {
        int speed1 = 0;
        int speed2 = 0;
        
        CUser *pUser = pFight->GetUser(m1);
        SMonsterInst *pMonster = pFight->GetMonster(m1);
        SPet *pPet = pFight->GetPet(m1);
        if(pUser != NULL)
            speed1 = pUser->GetSpeed();
        else if(pMonster != NULL)
            speed1 = pMonster->speed;
        else if(pPet != NULL)
            speed1 = pPet->speed;
        speed1 += pFight->GetSpeedImprove(m1);
        
        pUser = pFight->GetUser(m2);
        pMonster = pFight->GetMonster(m2);
        pPet = pFight->GetPet(m2);
        
        if(pUser != NULL)
            speed2 = pUser->GetSpeed();
        else if(pMonster != NULL)
            speed2 = pMonster->speed;
        else if(pPet != NULL)
            speed2 = pPet->speed;
        speed2 += pFight->GetSpeedImprove(m2);    
        return speed1 > speed2;
    }
    CFight *pFight;
};

void CFight::SortBySpeed(uint8 *arr,uint8 num)
{
    SSortMemberSpeed sortSpeed;
    sortSpeed.pFight = this;
    std::sort(arr,arr+num,sortSpeed);
}

bool CFight::AllUserAutoFight(uint16 &userMask)
{
    bool flag = true;
    userMask = 0;
    for(uint8 i = 1; i <= MAX_MEMBER; i++)
    {
        CUser *pUser = GetUser(i);
        if(pUser != NULL)
        {
            if((pUser->IsAutoFight())
                || ((GetState(i) >= EMSDied) && (GetPet(i+1) == NULL))
                || ((GetState(i) >= EMSDied) && (GetPet(i+1) != NULL) && (GetState(i+1) >= EMSDied)))
            {
                userMask |= (1<<i);
            }
            else
            {
                flag = false;
            }
        }
    }
    return flag;
}

bool CFight::FightTimeout()
{
    if(m_fightIsEnd)
        return true;
    if(GetSysTime() - m_beginTime > m_timeOut)
        return true;
    
    bool flag = false;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        if(((m_userOpTime != 0) && (GetSysTime() - m_userOpTime > FIGHT_TIMEOUT))
            || (OneGroupAllDie() > 0))
        {
            flag = true;
        }
    }
    if(!flag)
        return false;
    list<uint32> userList;
    if(m_pScene != NULL)
        m_pScene->GetUserList(userList);
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CalculateFightResult(userList);
    return true;
}

int CFight::GetDaoQian5(int turn,int type)
{
    int toMuLevel = 1;
    int louLuoNum = 0;
    int louLuoLevel = 1;
    //头目46
    for(uint8 i = 0; i < MAX_MEMBER; i++)
    {
        SMonsterInst *pMonster = GetMonster(i+1);
        if(pMonster != NULL)
        {
            if((pMonster->tmplId == 9) || (pMonster->tmplId == 8) || (pMonster->tmplId == 41))
                toMuLevel = pMonster->level;
            else
            {
                louLuoNum++;
                louLuoLevel = pMonster->level;
            }
        }
    }
    /*
    5 每一个任务的潜能提供：
	10~29级  怪物提供潜能=（10+轮次）*0.7*（头目等级*90+2700）/155+喽罗个数*（10+轮次）*0.06*（喽罗等级*90+2700）/155									
    30~59级  怪物提供潜能=（10+轮次）*0.65*（头目等级*150+5000）/155+喽罗个数*（10+轮次）*0.07*（喽罗等级*150+5000）/155									
    60~90级  怪物提供潜能=（10+轮次）*0.6*（头目等级*200+7000）/155+喽罗个数*（10+轮次）*0.08*（喽罗等级*200+7000）/155									
    type 0 10~29级
	type 1 30~59级
	type 2 60~90级
	*/
	turn += 10;
	switch(type)
	{
	case 0:
	    return (int)(((turn*0.7*(toMuLevel*90+2700))/155.0)
            +louLuoNum*(turn*0.06*(louLuoLevel*90+2700)/155.0));
	case 1:
	    return (int)(((turn*0.65*(toMuLevel*150+5000))/155.0)
            +louLuoNum*(turn*0.07*(louLuoLevel*150+5000)/155.0));
	case 2:
	    return (int)(((turn*0.6*(toMuLevel*200+7000))/155.0)
            +louLuoNum*(turn*0.08*(louLuoLevel*200+7000)/155.0));
	default:
	    return 0;
	}
}

int CFight::GetDaoQian6(int turn,int type)
{
    int toMuLevel = 1;
    int louLuoNum = 0;
    int louLuoLevel = 1;
    //头目46
    for(uint8 i = 0; i < MAX_MEMBER; i++)
    {
        SMonsterInst *pMonster = GetMonster(i+1);
        if(pMonster != NULL)
        {
            if((pMonster->tmplId == 9) || (pMonster->tmplId == 8) || (pMonster->tmplId == 41))
                toMuLevel = pMonster->level;
            else
            {
                louLuoNum++;
                louLuoLevel = pMonster->level;
            }
        }
    }
    /*
    6 每一个任务的道行提供：
	 10~29级  怪物提供道行=（10+轮次）*0.7*（头目等级+30）/155+喽罗个数*（10+轮次）*0.06*（喽罗等级+30）/155									
    30~59级  怪物提供道行=（10+轮次）*0.65*（头目等级*1.5+50）/155+喽罗个数*（10+轮次）*0.07*（喽罗等级*1.5+50）/155									
    60~90级  怪物提供道行=（10+轮次）*0.6*（头目等级*2+70）/155+喽罗个数*（10+轮次）*0.08*（喽罗等级*2+70）/155									

	type 0 10~29级
	type 1 30~59级
	type 2 60~90级
	*/
    turn += 10;
	switch(type)
	{
	case 0:
	    return (int)(((turn*0.7*(toMuLevel+30))/155.0)
            +louLuoNum*(turn*0.06*(louLuoLevel+30)/155.0));
	case 1:
	    return (int)(((turn*0.65*(toMuLevel*1.5+50))/155.0)
            +louLuoNum*(turn*0.07*(louLuoLevel*1.5+50)/155.0));
	case 2:
	    return (int)(((turn*0.6*(toMuLevel*2+70))/155.0)
            +louLuoNum*(turn*0.08*(louLuoLevel*2+70)/155.0));
	default:
	    return 0;
	}
}

void CFight::AddShengWang()
{
    if((m_type == EFTPlayerPk) || (m_type == EFTPlayerQieCuo))
        return;
        
    CUser *pTeamLeader = NULL;
    CUser *pLowUser = NULL;//最低等级玩家
    int flag = 0;//1加声望，2扣声望
    uint8 monsterNum = 0;
    
    const uint8 maxNum = 3;
    CUser *pHots[maxNum];
    uint8 userNum = 0;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        CUser *pUser = GetUser(pos);
        if(pUser != NULL)
        {
            pHots[userNum] = pUser;
            userNum++;
            
            if(pUser->GetTeam() == pUser->GetRoleId())
            {
                pTeamLeader = pUser;
            }
            else
            {
                if(pLowUser == NULL)
                {
                    pLowUser = pUser;
                    if(GetState(pos) == EMSDied)
                        flag = 2;
                    else
                        flag = 1;
                }
                else if(pUser->GetLevel() < pLowUser->GetLevel())
                {
                    pLowUser = pUser;
                    if(GetState(pos) == EMSDied)
                        flag = 2;
                    else
                        flag = 1;
                }
            }
        }
        else if(GetMonster(pos) != NULL)
        {
            monsterNum++;
        }
    }
    if((pTeamLeader != NULL) && (pLowUser != NULL) 
        && (pTeamLeader->GetLevel() - pLowUser->GetLevel() > 15)
        && (flag != 0) && (monsterNum != 0))
    {
        if(flag == 1)
        {
            pTeamLeader->SetData16(0,pTeamLeader->GetData16(0) + monsterNum);
        }
        else if(flag == 2)
        {
            if(pTeamLeader->GetData16(0) > 100)
            {
                pTeamLeader->SetData16(0,pTeamLeader->GetData16(0) - 100);
            }
            else
            {
                pTeamLeader->SetData16(0,0);
            }
        }
        pTeamLeader->SendUpdateInfo(1,pTeamLeader->GetData16(0));
    }
    
    if(userNum == 2)
    {
        uint16 val1 = 0;
        uint16 val2 = 0;
        if(pHots[0]->GetHotVal(pHots[1]->GetRoleId(),val1) 
            && pHots[1]->GetHotVal(pHots[0]->GetRoleId(),val2) )
        {
            if(val1 < 10000)
            {
                val1++;
                pHots[0]->SetHotVal(pHots[1]->GetRoleId(),val1);
            }
            if(val2 < 10000)
            {
                val2++;
                pHots[1]->SetHotVal(pHots[0]->GetRoleId(),val2);
            }
        }
        if(pHots[0]->GetData32(6) == pHots[1]->GetRoleId())
        {
            pHots[0]->SetData32(10,pHots[0]->GetData32(10)+1);
            pHots[1]->SetData32(10,pHots[1]->GetData32(10)+1);
        }
    }
    else if(userNum == 3)
    {
        uint16 val1 = 0;
        uint16 val2 = 0;
        if(pHots[0]->GetHotVal(pHots[1]->GetRoleId(),val1) 
            && pHots[1]->GetHotVal(pHots[0]->GetRoleId(),val2) )
        {
            if(val1 < 10000)
            {
                val1++;
                pHots[0]->SetHotVal(pHots[1]->GetRoleId(),val1);
            }
            if(val2 < 10000)
            {
                val2++;
                pHots[1]->SetHotVal(pHots[0]->GetRoleId(),val2);
            }
        }
        if(pHots[1]->GetHotVal(pHots[2]->GetRoleId(),val1) 
            && pHots[2]->GetHotVal(pHots[1]->GetRoleId(),val2) )
        {
            if(val1 < 10000)
            {
                val1++;
                pHots[1]->SetHotVal(pHots[2]->GetRoleId(),val1);
            }
            if(val2 < 10000)
            {
                val2++;
                pHots[2]->SetHotVal(pHots[1]->GetRoleId(),val2);
            }
        }
        if(pHots[0]->GetHotVal(pHots[2]->GetRoleId(),val1) 
            && pHots[2]->GetHotVal(pHots[0]->GetRoleId(),val2) )
        {
            if(val1 < 10000)
            {
                val1++;
                pHots[0]->SetHotVal(pHots[2]->GetRoleId(),val1);
            }
            if(val2 < 10000)
            {
                val2++;
                pHots[2]->SetHotVal(pHots[0]->GetRoleId(),val2);
            }
        }
    }
}

void CFight::SaveFightLog()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    bool saveLog = false;
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        if((GetUser(pos) != NULL) && (GetState(pos) == EMSDied))
        {
            saveLog = true;
            break;
        }
    }
    if(!saveLog)
        return;
    
    char *sqlBuf = new char[4096];
    if(sqlBuf == NULL)
        return;
    
    for(uint8 pos = 1; pos <= MAX_MEMBER; pos++)
    {
        CUser *pUser = GetUser(pos);
        if(pUser == NULL)
            continue;
        string dropItem;
        //string shuxing;
        char shuxing[128];
        if(m_pScene != NULL)
        {
            snprintf(shuxing,sizeof(shuxing),"%d|%d|%d|%d",m_pScene->GetId(),m_pScene->GetMapId(),m_visibleMonsterId,pUser->GetBangPai());
        }
        string equip;
        pUser->GetDropItem(dropItem);
        //pUser->MakeSaveShuXing(shuxing);
        pUser->MakeSaveEquip(equip);
        snprintf(sqlBuf,4095,"INSERT INTO fight_log (fight_id,role_id,mozhang1,mozhang2,"\
                "daohang1,daohang2,exp1,exp2,drop_item,shuxing,equip,pos,time) "\
                "VALUES (%d,%d,%d,%d,%d,%d,%d,%d,'%s','%s','%s',%d,%lu)",
                m_id,pUser->GetRoleId(),
                m_members[pos-1].srcPkVal,pUser->GetPkVal(),
                m_members[pos-1].srcDaoHang,pUser->GetDaoHang(),
                m_members[pos-1].srcExp,pUser->GetExp(),
                dropItem.c_str(),shuxing,equip.c_str(),pos,GetSysTime());
        pDb->Query(sqlBuf);
    }
    delete []sqlBuf;
}

/*******************************************************
CFightManager member function
*******************************************************/
ShareFightPtr CFightManager::CreateFight()
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    ShareFightPtr ptr;
    /*if(m_gcFights.size() > 0)
    {
        ptr = *m_gcFights.begin();
        ptr->SetId(m_curFightId);
        m_gcFights.pop_front();
    }
    else*/
    {
        CFight *pFight = new CFight;
        pFight->SetId(m_curFightId);
        ptr.reset(pFight);
    }
    atomic_increment((int*)&m_curFightId);
    return ptr;
}

ShareFightPtr CFightManager::FindFight(uint32 id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    ShareFightPtr ptr;
    m_fights.Find(id,ptr);
    return ptr;
}

CFightManager::CFightManager():m_curFightId(1)
,m_onlineUser(SingletonOnlineUser::instance())
{
    CDespatchCommand &despatch = SingletonDespatch::instance();

    SCommand cmdFun[] = {
        {PRO_BATTLE,boost::bind(&CFightManager::UserBattle,this,_1,_2)}
    };
    despatch.AddCommandDeal(cmdFun,sizeof(cmdFun)/sizeof(SCommand));
}

void CFightManager::UserBattle(CNetMessage *pMsg,int sock)
{
    try
    {
        if(pMsg == NULL)
            return;
        CNetMessage &msg = *pMsg;
        
        ShareUserPtr ptr = m_onlineUser.GetUserBySock(sock);
        CUser *pUser = ptr.get();
        if(pUser == NULL)
        {
            return;
        }
        
        int id = pUser->GetFightId();
        if((id == 0) || (pUser->GetFightPos() == 0))
            return;
        
        ShareFightPtr fightPtr;
        {
            boost::recursive_mutex::scoped_lock lk(m_mutex);
            m_fights.Find(id,fightPtr);
        }
        CFight *pFight = fightPtr.get();
        if(pFight == NULL)
        {
            uint8 playOp;
            int playPara;
            msg>>playOp>>playPara;        
            if(playOp == 10)
            {
                if(playPara == 1)
                {//设置自动战斗
                    pUser->SetAutoFightTurn(25);//自动战斗轮次
                    msg.ReWrite();
                    msg.SetType(PRO_BATTLE);
                    msg<<(uint8)4<<(uint8)pUser->GetAutoFightTurn();//通知客户端自动回合数
                    CSocketServer &sock = SingletonSocket::instance();
                    sock.SendMsg(pUser->GetSock(),msg);
                }
                else if(playPara == 0)
                {//取消自动战斗
                    pUser->SetAutoFightTurn(0);
                    msg.ReWrite();
                    msg.SetType(PRO_BATTLE);
                    msg<<(uint8)3;//通知客户端取消战斗
                    CSocketServer &sock = SingletonSocket::instance();
                    sock.SendMsg(pUser->GetSock(),msg);
                }
            }
            return;
        }
        
        pFight->UserBattle(msg,pUser);
        pFight->IsFightEnd();
    }
    catch(...)
    {
        cout<<"catch CFightManager::UserBattle error"<<endl;
    }
}
/*
bool CFightManager::EachFight(uint32 id,ShareFightPtr pFight)
{
    if(pFight->FightTimeout())
    {
        if(pFight->IsFightEnd())
        {
            ShareFightPtr ptr;
            m_fights.Erase(id,ptr);
            if(ptr.get() != NULL)
            {
                ptr->SaveFightLog();
                //ptr->Clear();
                //m_gcFights.push_back(ptr);
            }
            return false;
        }
    }
    return true;
}
*/
void CFightManager::AddFight(ShareFightPtr ptr)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    m_fights.Insert(ptr->GetId(),ptr);
}

static bool EachFight(uint32 id,list<uint32> *pFightList)
{
    pFightList->push_back(id);
    return true;
}

void CFightManager::RunFightTimeOut()
{
    list<uint32> fightList;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        m_fights.ForEach(boost::bind(EachFight,_1,&fightList));
    }
    for(list<uint32>::iterator i = fightList.begin(); i != fightList.end(); i++)
    {
        ShareFightPtr pFight = FindFight(*i);
        if(pFight.get() != NULL)
        {
            if(pFight->FightTimeout())
            {
                if(pFight->IsFightEnd())
                {
                    pFight->SaveFightLog();
                    pFight->AddShengWang();
                    pFight->SendGuanZhanOver();
                    boost::recursive_mutex::scoped_lock lk(m_mutex);
                    m_fights.Erase(*i);
                }
            }
        }
    }
}
