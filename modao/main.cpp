#include <iostream>
#include "pack_list.h"
#include "pack_deal.h"
#include "main.h"
#include "net_msg.h"
#include "database.h"
#include "singleton.h"
#include "ini_file.h"
#include "self_typedef.h"
#include "call_script.h"
#include "script_call.h"
#include "huo_dong.h"
#include <signal.h>
#include <boost/bind.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <fstream>
#include <zlib.h>

const char *gConfigFile = "config";
//
#include <boost/thread/thread.hpp>


using namespace std;

static void EachUserNoLock(CUser *pUser,CDatabaseSql *pDb)
{
    pUser->SaveData(pDb,false);
}

static bool sExit = true;
/*
static void SigHandlerNoLock(int)
{
    COnlineUser &onlineUser = SingletonOnlineUser::instance();
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    onlineUser.ForEachUserNoLock(boost::bind(EachUserNoLock,_1,pDb));
    sExit = false;
    cout<<"nolock save data"<<endl;
}
*/
static void SigHandlerCreateCore(int)
{
    SingletonCBangPaiManager::instance().SaveZhongZhi();
    COnlineUser &onlineUser = SingletonOnlineUser::instance();
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    onlineUser.ForEachUserNoLock(boost::bind(EachUserNoLock,_1,pDb));
    abort();
    sExit = false;
}

static void EachUser(CUser *pUser,CDatabaseSql *pDb)
{
    pUser->SaveData(pDb);
}

static void SigHandler(int)
{
    SingletonCBangPaiManager::instance().SaveZhongZhi();
    COnlineUser &onlineUser = SingletonOnlineUser::instance();
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    onlineUser.ForEachUser(boost::bind(EachUser,_1,pDb));
    sExit = false;
    cout<<"lock save data"<<endl;
}

static void SaveUserData()
{
    struct sigaction sig;
    sigemptyset(&sig.sa_mask);
    sig.sa_handler = SIG_IGN;
    sig.sa_flags      = SA_ONESHOT;//SA_NOMASK;
    
    for(int i = 0; i < SIGRTMIN; i++)
    {
        if((i != SIGINT) || (i != SIGTERM) 
            || (i != SIGSEGV) || (i != SIGABRT)
            || (i != SIGFPE))
        {
            sigaction(i,&sig,NULL);
        }
    }
    
    sig.sa_handler = &SigHandler;
    sigaction(SIGINT,&sig,NULL);
    
    sig.sa_handler = &SigHandler;//SigHandlerNoLock;
    sigaction(SIGFPE,&sig,NULL);
    sigaction(SIGTERM,&sig,NULL);
    sigaction(SIGSEGV,&sig,NULL);
    sigaction(SIGABRT,&sig,NULL);
    
    sig.sa_handler = &SigHandlerCreateCore;
    sigaction(SIGRTMIN,&sig,NULL);
    
    time_t t;// = GetSysTime();
    
    while(sExit)
    {
        sleep(1);
        SetSysTime(time(NULL));
        t = GetSysTime();
        tm *pTm = localtime(&t);
        if(pTm == NULL)
            continue;
            
        if(GetClearTime() == 0)
        {
            if(pTm->tm_wday == 0)
            {
                t -= pTm->tm_wday*3600*24+pTm->tm_hour*3600+pTm->tm_min*60+pTm->tm_sec + 6*24*3600;
            }
            else
            {
                t -= pTm->tm_wday*3600*24+pTm->tm_hour*3600+pTm->tm_min*60+pTm->tm_sec;// + 6*24*3600;
                t +=  3600*24;
            }
            SetClearTime(t);
            //cout<<ctime(&t)<<endl;
        }
        if(pTm->tm_wday == 1)
        {
            if(t - GetClearTime() > 24*3600)
            {
                SetClearTime(t);
            }
        }
    }
}

bool CMainClass::Init()
{
        
    if(GetScript() == NULL)
        return false;
    memset(m_gonggao,0,sizeof(m_gonggao));
    memset(m_sysInfo,0,sizeof(m_sysInfo));
    
    string num = CIniFile::GetValue("threadnum","server",gConfigFile);
    
    string serverPort = CIniFile::GetValue("port","server",gConfigFile);
    
    string version = CIniFile::GetValue("client_version","server",gConfigFile);
    string forceUpdate = CIniFile::GetValue("force_update","server",gConfigFile);
    string updateMsg = CIniFile::GetValue("update_msg","server",gConfigFile);
    string updateUrl = CIniFile::GetValue("update_url","server",gConfigFile);
    
    if(!SingletonSceneManager::instance().Init())
        return false;
        
    packDeal.SetVerInfo(version,forceUpdate,updateMsg,updateUrl);
    
    m_threadNum = atoi(num.c_str());
    if(m_thread == NULL)
        m_thread = new boost::thread*[m_threadNum+3];
    for(int i = 0; i < atoi(num.c_str()); i++)
    {
        boost::thread *pTh = new boost::thread(boost::bind(&CMainClass::DealPackThread,this));
        //pTh->detach();
        m_thread[i] = pTh;
    }
    m_thread[m_threadNum] = new boost::thread(boost::bind(&CMainClass::TimeOut,this));
    m_thread[m_threadNum+1] = new boost::thread(&SaveUserData);
    m_thread[m_threadNum+2] = new boost::thread(boost::bind(&CMainClass::IdleThread,this));
    
    if(!m_socketServer.Init(MAX_CON_USER))
        return false;
    if(!m_socketServer.Bind(NULL,serverPort.c_str()))
        return false;
    
    return true;
}

void CMainClass::Join()
{
    for(uint8 i = 0; i < m_threadNum+3; i++)
    {
        m_thread[i]->join();
    }
    delete []m_thread;
    m_thread = NULL;
}

void CMainClass::DealPackThread()
{
    int sock;
    while (sExit)
    {
        CNetMessage *pMsg = m_socketServer.GetPackage(sock);
        
        if(pMsg != NULL)
        {
            m_despatch.Despatch(pMsg,sock);
            //cout<<pMsg->GetType()<<endl;
            delete pMsg;
        }
    }
}
/*
static void UpdateEachUser(CUser *pUser,int day)
{
    pUser->UpdateMission(day);
}

void CMainClass::UpdateUser(int day)
{
    m_onlineUser.ForEachUser(boost::bind(UpdateEachUser,_1,day));
}

static void UserTimeOut(CUser *pUser)
{
    pUser->TimeOut();
}
*/
struct SSceneVisibleMonster
{
    uint16 sceneId;
    uint8 x,y;
};

const SSceneVisibleMonster sMonster[] = 
{//sceneId,(x,y)
    {11,15,15},
    {11,4,9},
    {11,22,25},
    {12,20,9},
    {12,4,25},
    {12,3,5},
    {13,25,9},
    {13,25,25},
    {13,6,18},
    {14,24,23},
    {14,6,22},
    {14,15,5},
    {15,10,23},
    {15,9,11},
    {15,23,16},
    {16,22,22},
    {16,21,4},
    {16,5,20},
    {17,10,9},
    {17,25,14},
    {17,6,27},
    {18,4,5},
    {18,23,11},
    {18,23,23},
    {19,8,6},
    {19,24,19},
    {19,9,25},
    {41,7,16},
    {41,24,19},
    {41,14,26},
    {41,9,20},
    {41,15,12},
    {45,12,4},
    {45,11,10},
    {45,4,22},
    {45,15,28},
    {45,19,17},
    {51,13,4},
    {51,10,15},
    {51,13,27},
    {51,24,20},
    {51,14,19},
    {72,21,22},
    {72,20,6},
    {72,3,10},
    {72,11,22},
    {72,4,23},
    {55,6,19},
    {55,17,26},
    {55,23,17},
    {55,16,12},
    {55,10,5},
    {54,26,12},
    {54,21,21},
    {54,6,23},
    {54,4,5},
    {54,11,11},
    {2,24,23},
    {2,4,25},
    {2,15,15},
    {2,6,11},
    {2,20,14},
    {1,10,13},
    {1,5,25},
    {1,24,7},
    {3,23,6},
    {3,12,14},
    {3,4,6},
    {3,5,25},
    {3,25,24}
};

void CMainClass::InitMonster(SVisibleMonster &monster,uint8 x,uint8 y)
{
    monster.x = x;
    monster.y = y;
    if(Random(0,100) > 50)
        monster.face = 6;
    else
        monster.face = 4;
    monster.type = 2;
    monster.pic = 51;
    monster.flag = 0;//0正常,1战斗中
}

void CMainClass::BeginNianShou()
{
    uint16 sceneId[] = 
    {
        69	,	21,26,
        67	,	4,11,
        68	,11,4,
        111,	22,27,
        112,	17,11,
        120,5,26,
        122,11,27,
        123,7,3,
        126,	21,29,
        92	,	4,13,
        83	,21,22,
        81	,14,17,
        61	,22,3,
        //59	,	27,18,
        56	,19,29,
        55	,	11,5
    };
    
    CSceneManager &sceneMgr = SingletonSceneManager::instance();
    uint8 i = Random(0,sizeof(sceneId)/sizeof(sceneId[0])/3-1);
    
    cout<<sceneId[3*i]<<endl;
    
    CNetMessage msg;
    msg.SetType(PRO_SYSTEM_INFO);
#ifdef QQ
    msg<<"【劳动最光荣活动】开始了!拿起钓竿去越亭护城河钓鱼吧!野外池塘虾兵蟹将也出动,快去消灭它们!";
#else
    msg<<"春来啦,越亭护城河的鱼汛到了!拿起钓竿去钓吧!野外池塘虾兵蟹将也出动,快去消灭它们!";
#endif
    
    m_onlineUser.ForEachUser(boost::bind(&CMainClass::SendSysInfo,this,&msg,_1));
    
    CScene *pScene = sceneMgr.FindScene(sceneId[3*i]);
    if(pScene != NULL)
    {
        SVisibleMonster monster;
        monster.face = 6;
        monster.type = 4;
        if(Random(0,100) >= 70)
            monster.pic = 31;
        else
            monster.pic = 36;
        monster.x = sceneId[3*i+1];
        monster.y = sceneId[3*i+2];
        monster.id = monster.pic;
        monster.flag = 0;
        pScene->AddVisibleMonster(monster);
    }
}
/*void CMainClass::BeginNianShou()
{
    uint16 sceneId[] = {11,12,13,14,15,16,17,18,19,
        21,22,23,24,25,26,27,28,29,
        31,32,33,34,35,36,37,38,39
    };
    
    //2，54，41，11---19
    //uint16 sceneId[] = {2,54,41,11,12,13,14,15,16,17,18,19};
    
    CNetMessage msg;
    msg.SetType(PRO_SYSTEM_INFO);
    msg<<"捣蛋小雪仙活动开始了，大家快积攒雪球去抽奖吧！";
    
    m_onlineUser.ForEachUser(boost::bind(&CMainClass::SendSysInfo,this,&msg,_1));
    
    CSceneManager &sceneMgr = SingletonSceneManager::instance();
    
    for(uint8 i = 0; i < sizeof(sceneId)/sizeof(uint16); i++)
    {
        CScene *pScene = sceneMgr.FindScene(sceneId[i]);
        //SPoint points[6] = {0};
        if(pScene != NULL)
        {
            int num = 0;
            for(uint8 j = 0; j < 8; j++)
            {
                SVisibleMonster monster;
                pScene->GetCanWalkPos(monster.x,monster.y);
                monster.id = num++;
                monster.face = 6;
                monster.type = 4;
                monster.pic = 56;
                monster.flag = 0;
                pScene->AddVisibleMonster(monster);
            }
        }
    }
}
*/
void CMainClass::EndNianShou()
{
    CSceneManager &sceneMgr = SingletonSceneManager::instance();
    for(uint8 i = 0; i < sizeof(sMonster)/sizeof(SSceneVisibleMonster); i++)
    {
        CScene *pScene = sceneMgr.FindScene(sMonster[i].sceneId);
        if(pScene == NULL)
            continue;
        pScene->ClearVisibleMonster();
    }
}
/*
void ClearVisibleMonster()
{
    //uint16 sceneId[] = {2,54,41,11,12,13,14,15,16,17,18,19};
    uint16 sceneId[] = {11,12,13,14,15,16,17,18,19,
        21,22,23,24,25,26,27,28,29,
        31,32,33,34,35,36,37,38,39
    };
    
    CSceneManager &sceneMgr = SingletonSceneManager::instance();
    
    for(uint8 i = 0; i < sizeof(sceneId)/sizeof(uint16); i++)
    {
        CScene *pScene = sceneMgr.FindScene(sceneId[i]);
        if(pScene != NULL)
        {
            pScene->ClearVisibleMonster();
        }
    }
}
*/
void ClearVisibleMonster()
{
    uint16 sceneId[] = 
    {
        69	,	21,26,
        67	,	4,11,
        68	,11,4,
        111,	22,27,
        112,	17,11,
        120,5,26,
        122,11,27,
        123,7,3,
        126,	21,29,
        92	,	4,13,
        83	,21,22,
        81	,14,17,
        61	,22,3,
        //59	,	27,18,
        56	,19,29,
        55	,	11,5
    };
    CSceneManager &sceneMgr = SingletonSceneManager::instance();
    
    for(uint8 i = 0; i < sizeof(sceneId)/sizeof(uint16)/3; i++)
    {
        CScene *pScene = sceneMgr.FindScene(sceneId[3*i]);
        if(pScene != NULL)
        {
            pScene->ClearVisibleMonster();
        }
    }
}
struct SDelMonster
{
    uint16 monsterId;
    time_t addTime;
    uint16 sceneId;
    uint16 visibleMonster;
};

static SDelMonster sMissionMonster[3] = {
    {YE_ZHU_WANG_ID,0,51,6},
    {QIAN_NIAN_JIANG_SHI_ID,0,71,10},
    {THREE_SISTER_ID,0,59,14}
};
    
void SetAddTime(uint16 mId,time_t t)
{
    for(uint8 i = 0; i < 3; i++)
    {
        if(sMissionMonster[i].monsterId == mId)
        {
            sMissionMonster[i].addTime = t;
            break;
        }
    }
}
/*
npc 160 越亭郡东门(51) 161 天戟峰(71) 162  天涯海角(59) 类型：明怪（被杀死后，每1分钟刷新一次）

名称：野猪王 类型：明怪（被杀死后，每1分钟刷新一次）（越亭郡东门：18.9） 
造型：野猪 强度：主线猪妖2/3强度。+2只小野猪。（怪物头目强度） 经验：200 奖励：无 
点击显示对话： 哼唧……哼唧…… 击杀野猪王 取消  

名称：千年僵尸 类型：明怪（被杀死后，每1分钟刷新一次）（天戟峰：11.17） 造型：僵尸 
强度：招讨使强度+5只僵尸喽啰。（怪物头目强度） 经验：500 奖励：无 
点击显示对话： 来吧，我将让你变成下一个随从. 击杀千年僵尸 取消  

名称：大姐/二姐/三姐 类型：明怪（被杀死后，每1分钟刷新一次）（天涯海角：22.21） 
造型：狐妖 强度：招讨使强度 经验：1000 奖励：无（任务品：狐妖精魂。无任务不掉落） 
点击显示对话： 哼，敢打我们三姐妹的注意，让你知道厉害. 击杀狐妖三姐妹 取消
*/
void CMainClass::VisibleMonsterMis()
{
    CSceneManager &sceneMgr = SingletonSceneManager::instance();
    for(uint8 i = 0; i < 3; i++)
    {
        if(GetSysTime() >= sMissionMonster[i].addTime)
        {
            sMissionMonster[i].addTime = 0x7fffffff;
            CScene *pScene = sceneMgr.FindScene(sMissionMonster[i].sceneId);
            if(pScene != NULL)
            {
                SVisibleMonster monster;
                //pScene->GetCanWalkPos(monster.x,monster.y);
                if(i == 0)
                {
                    monster.x = 18;
                    monster.y = 9;
                }
                else if(i == 1)
                {
                    monster.x = 11;
                    monster.y = 17;
                }
                else
                {
                    monster.x = 22;
                    monster.y = 21;
                }
                monster.id = sMissionMonster[i].monsterId;
                monster.face = 6;
                monster.type = 2;
                monster.pic = sMissionMonster[i].visibleMonster;
                monster.flag = 0;
                pScene->AddVisibleMonster(monster);
            }
        }
    }
}

void CMainClass::GuiJieHuoDong()
{
    if(!m_inGuiJie)
    {
        time_t t = GetSysTime();
        struct tm *pTm = localtime(&t);
        if(pTm == NULL)
            return;
            
        //if(pTm->tm_hour == 21)*/
        if(pTm->tm_hour == MING_GUAI_TIME)
        {
            m_inGuiJie = true;
            m_guiJieBegin = GetSysTime();
            m_addMonster = m_guiJieBegin;
            ClearVisibleMonster();
            BeginNianShou();
        }
    }
    else
    {
        if(GetSysTime() - m_guiJieBegin >= 60*60)
        {
            m_inGuiJie = false;
            ClearVisibleMonster();
            /*CNetMessage msg;
            msg.SetType(PRO_SYSTEM_INFO);
            msg<<"捣蛋小雪仙活动结束了，希望你能用雪球抽得大奖！";
            m_onlineUser.ForEachUser(boost::bind(&CMainClass::SendSysInfo,this,&msg,_1));*/
        }
        if(m_inGuiJie)
        {
            if(GetSysTime() - m_addMonster >= 8*60)
            {
                ClearVisibleMonster();
                BeginNianShou();
                m_addMonster = GetSysTime();
            }
        }
    }
}

void CMainClass::NianShouHuoDong()
{
    if(!m_inNianShou)
    {
        time_t t = GetSysTime();
        struct tm *pTm = localtime(&t);
        if(pTm == NULL)
            return;
        if(pTm->tm_hour == 19)// || (pTm->tm_hour == 13))
        {
            m_inNianShou = true;
            m_nianShouBegin = GetSysTime();
            BeginFightHuoDong();
            //BeginNianShou();
            CNetMessage msg;
            msg.SetType(PRO_SYSTEM_INFO);
            SetLeftDropNum(200);
            msg<<"百花仙使从天界下凡巡游,大家快去找她要礼物吧！";
            m_onlineUser.ForEachUser(boost::bind(&CMainClass::SendSysInfo,this,&msg,_1));
        }
    }
    else
    {
        if(GetSysTime() - m_nianShouBegin > 60*60)
        {
            m_inNianShou = false;
            EndFightHuoDong();
            //EndNianShou();
            CNetMessage msg;
            msg.SetType(PRO_SYSTEM_INFO);
            msg<<"百花仙使带来的礼物发放完毕,请大家明天再来！";
            m_onlineUser.ForEachUser(boost::bind(&CMainClass::SendSysInfo,this,&msg,_1));
        }
    }
}

static int sOnlineNum;
void SetOnlineUserNum(int num)
{
    sOnlineNum = num;
}
int GetOnlineUserNum()
{
    return sOnlineNum;
}

void CMainClass::TimeOut()
{
    time_t saveOnlineNum = GetSysTime();
    time_t userTimeOut = GetSysTime();
    while(sExit)
    {
        if(GetSysTime() - userTimeOut > 10)
        {
            list<uint32> userList;
            int num = 0;
            m_onlineUser.GetUserList(userList);
            for(list<uint32>::iterator i = userList.begin(); i != userList.end(); i++)
            {
                ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(*i);
                CUser *pUser = ptr.get();
                if(pUser != NULL)
                {
                    pUser->TimeOut();
                    num++;
                }
            }
            SetOnlineUserNum(num);
            if(GetSysTime() - saveOnlineNum > 1800)
            {
                saveOnlineNum = GetSysTime();
                CGetDbConnect getDb;
                CDatabaseSql *pDb = getDb.GetDbConnect();
                char sql[128];
                snprintf(sql,127,"INSERT INTO online_user_num (num,time) VALUES (%d,"\
                        "FROM_UNIXTIME(%lu))",num,GetSysTime());            
                if (pDb != NULL)
                {
                    pDb->Query(sql);
                }
            }
            NianShouHuoDong();
#ifdef QQ
            GuiJieHuoDong();
#endif
            VisibleMonsterMis();
        }
        sleep(1);
    }
}
//const int SAVE_USER_INFO_TIME = ;

void CMainClass::SendSysInfo(CNetMessage *msg,CUser *pUser)
{
    m_socketServer.SendMsg(pUser->GetSock(),*msg);
}

void CMainClass::SendMsgToUser()
{
    if(GetSysTime() - m_readMsgTime > 300)
    {//读取系统消息
        m_readMsgTime = GetSysTime();
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        if(pDb == NULL)
            return;
        if(!pDb->Query("select type,msg,time_space,begin_time,end_time from notice"))
            return;
        char **row;
        m_gonggao[0] = 0;
        m_sysInfo[0] = 0;
        m_cxGongGao[0] = 0;
        while((row = pDb->GetRow()) != NULL)
        {
            time_t beginTime,endTime;
            beginTime = atoi(row[3]);
            endTime = atoi(row[4]);
            if((beginTime < GetSysTime()) && (GetSysTime() < endTime))
            {
                if(atoi(row[0]) == 0)
                {
                    if(row[1] != NULL)
                        snprintf(m_gonggao,sizeof(m_gonggao),row[1]);
                }
                else if(atoi(row[0]) == 1)
                {
                    if(row[1] != NULL)
                        snprintf(m_sysInfo,sizeof(m_sysInfo),row[1]);
                    m_sysInfoTimeSpace = 60*atoi(row[2]);
                }
                else if(atoi(row[0]) == 2)
                {
                    if(row[1] != NULL)
                        snprintf(m_cxGongGao,sizeof(m_cxGongGao),row[1]);   
                }
            }
        }
        char sql[256];
        snprintf(sql,255,"select beilv from huodong_time where "\
            "UNIX_TIMESTAMP(begin_time)<%lu and UNIX_TIMESTAMP(end_time)>%lu",GetSysTime(),
            GetSysTime());
        if(!pDb->Query(sql))
            return;
        row = pDb->GetRow();
        if(row != NULL)
        {
            if(!InHuoDong())
            {
                SetHuoDong(true);
                SetHuoDongBeiLv(atoi(row[0]));
                CNetMessage msg;
                msg.SetType(PRO_SYSTEM_INFO);
                msg<<"打怪双倍奖励开始了！";
                m_onlineUser.ForEachUser(boost::bind(&CMainClass::SendSysInfo,this,&msg,_1));
            }
        }
        else
        {
            SetHuoDong(false);
            SetHuoDongBeiLv(1);
        }
    }
    if((GetSysTime() - m_sendTime > m_sysInfoTimeSpace)
        && (m_sysInfo[0] != 0))
    {
        m_sendTime = GetSysTime();
        CNetMessage msg;
        msg.SetType(PRO_SYSTEM_INFO);
        msg<<m_sysInfo;
        m_onlineUser.ForEachUser(boost::bind(&CMainClass::SendSysInfo,this,&msg,_1));
        //cout<<"系统消息:"<<m_sysInfo<<endl;
    }
}

void CMainClass::SendGongGao(int sock)
{
    if(m_gonggao[0] != 0)
    {
        CNetMessage msg;
        msg.SetType(PRO_GONGGAO);
        msg<<m_gonggao;
        m_socketServer.SendMsg(sock,msg);
    }
    //cout<<"公告:"<<m_gonggao<<endl;
}

bool CMainClass::AddTongBao(uint32 userId,int money,char *msg,uint8 type)
{
    ShareUserPtr ptr;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return false;
    char buf[256];
    snprintf(buf,255,"select role0,role1,role2,role3,role4,role5 from user_info where id=%u",userId);
    if(!pDb->Query(buf))
        return false;
    char **row = pDb->GetRow();
    if(row == NULL)
        return false;
    for(uint8 i = 0; i < 6; i++)
    {
        if((row[i] != NULL) && (atoi(row[i]) != 0))
        {
            ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(atoi(row[i]));
            CUser *pUser = ptr.get();
            if(pUser != NULL)
            {
                if(money > 0)
                {
                    if(msg != NULL)
                        SendPopMsg(pUser,msg);
                    pUser->AddTongBao(money,type);
                    if(money >= 1000)
                    {
                        pUser->SetData32(12,pUser->GetData32(12) + money/1000*3);
                    }
                    /*time_t t = GetSysTime();
                    struct tm *pTm = localtime(&t);
                    if((pTm != NULL) && (pTm->tm_mday < 25))
                    {
                        if(money >= 2000)
                        {
                            pUser->AddBangDingPackage(646);
                        }
                    }*/
                }
                else
                {
                    if(msg != NULL)
                        SendPopMsg(pUser,msg);
                }
                return true;
            }
        }
    }
    return false;
}

uint32 GetMaster(uint32 roleId,int &maxMoney)
{
    maxMoney = 0;
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return 0;
    char sql[128];
    snprintf(sql,127,"select m_id,val from master_prentice where p_id=%d",roleId);
    
    if(!pDb->Query(sql))
        return 0;
    char **row = pDb->GetRow();
    if(row != NULL)
    {
        if(atoi(row[1]) < 10000)
            maxMoney = 10000 - atoi(row[1]);
        return atoi(row[0]);
    }
    return 0;
}

#ifdef QQ
const int TONGBAO_BEILV = 100;
void CMainClass::QQChongZhi()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;             //0    1  2       3    4       5           6        7
    char buf[256] = "select id,user_id,money,state,err_msg,role_id,role_level,type"\
                    " from cz_complete where is_deal=0 limit 1";
    if(!pDb->Query(buf))
        return;
    char **row = pDb->GetRow();
    if(row != NULL)
    {
        uint8 type = 0;
        if(atoi(row[7]) == 2)
            type = 1;
            
        uint32 id = atoi(row[0]);
        uint32 userId = atoi(row[1]);
        //uint32 pId = atoi(row[5]);
        //int maxMoney = 0;
        //uint32 masterId = GetMaster(pId,maxMoney);
        int money = atoi(row[2])*TONGBAO_BEILV;
        int state = atoi(row[3]);
        if(state == 0)
        {
            if(!AddTongBao(userId,money,row[4],type))
            {
                if(type == 1)
                {
                    sprintf(buf,"update user_info set bd_money=bd_money+%d where id = %d",money,userId);
                    pDb->Query(buf);
                }
                else
                {
                    sprintf(buf,"update user_info set money=money+%d where id = %d",money,userId);
                    pDb->Query(buf);
                }
                snprintf(buf,256,"INSERT INTO `cz_notice` (user_id,msg) VALUES (%u,'%s')",
                    userId,row[4]);
                pDb->Query(buf);
            }
        }
        else
        {
            if(!AddTongBao(userId,0,row[4],type))
            {
                //sprintf(buf,"update user_info set money=money+%d where id = %d",money*100,userId);
                //pDb->Query(buf);
                snprintf(buf,256,"INSERT INTO `cz_notice` (user_id,msg) VALUES (%u,'%s')",
                    userId,row[4]);
                pDb->Query(buf);
            }
        }
        snprintf(buf,255,"update cz_complete set is_deal=1 where id=%u",id);
        pDb->Query(buf);
    }
}
#endif

void CMainClass::ChongZhi()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;             //0     1  2       3   4       5           6        7
    char buf[256] = "select id,user_id,money,state,err_msg,role_id,role_level,role_name"\
                    " from cz_complete where is_deal=0 limit 1";
    if(!pDb->Query(buf))
        return;
    char **row = pDb->GetRow();
    if(row != NULL)
    {
        uint32 id = atoi(row[0]);
        uint32 userId = atoi(row[1]);
        uint32 pId = atoi(row[5]);
        int maxMoney = 0;
        uint32 masterId = GetMaster(pId,maxMoney);
        int money = atoi(row[2]);
        int state = atoi(row[3]);
        if(state == 0)
        {
            if(!AddTongBao(userId,money*100,row[4]))
            {
                sprintf(buf,"update user_info set money=money+%d where id = %d",money*100,userId);
                pDb->Query(buf);
                snprintf(buf,256,"INSERT INTO `cz_notice` (user_id,msg) VALUES (%u,'%s')",
                    userId,row[4]);
                pDb->Query(buf);
            }
            int level = atoi(row[6]);
            if((masterId != 0) && (level >= 15) && (level <= 49))
            {
                int beilv = 0;
                if((level >= 15) && (level <= 24))
                {
                    beilv = 5;
                }
                else if((level >= 25) && (level <= 34))
                {
                    beilv = 5;
                }
                else if((level >= 35) && (level <= 44))
                {
                    beilv = 5;
                }
                else if((level >= 45) && (level <= 49))
                {
                    beilv = 5;
                }
                int addmoney = (int)(money*beilv);
                sprintf(buf,"select id from user_info where role0=%d or role1=%d or "\
                        "role2=%d or role3=%d or role4=%d or role5=%d",
                        masterId,masterId,masterId,masterId,masterId,masterId);
                pDb->Query(buf);
                char **row = pDb->GetRow();
                if(addmoney > maxMoney)
                    addmoney = maxMoney;
                if(addmoney > 0)
                {
                    char msg[256];
                    snprintf(msg,256,"因为您徒弟充值,您获得由系统提供的师傅奖励%d通宝",addmoney);
                    if(row != NULL)
                    {
                        int mId = atoi(row[0]);
                        if(!AddTongBao(mId,addmoney,msg))
                        {
                            sprintf(buf,"update user_info set money=money+%d where id = %d",addmoney,mId);
                            pDb->Query(buf);
                            snprintf(buf,256,"INSERT INTO `cz_notice` (user_id,msg) VALUES (%u,'%s')",
                                mId,msg);
                            pDb->Query(buf);
                        }
                    }
                    sprintf(buf,"update master_prentice set val=val+%d where m_id=%d and p_id=%d",
                        addmoney,masterId,pId);
                    pDb->Query(buf);
                }
            }
        }
        else
        {
            if(!AddTongBao(userId,0,row[4]))
            {
                //sprintf(buf,"update user_info set money=money+%d where id = %d",money*100,userId);
                //pDb->Query(buf);
                snprintf(buf,256,"INSERT INTO `cz_notice` (user_id,msg) VALUES (%u,'%s')",
                    userId,row[4]);
                pDb->Query(buf);
            }
        }
        snprintf(buf,255,"update cz_complete set is_deal=1 where id=%u",id);
        pDb->Query(buf);
    }
                                //0 1       2            3       4
    snprintf(buf,255,"select type,card_num,card_password,money,role_id,id from chong_tongbao where state=0 limit 1");
    if(pDb->Query(buf))
    {
        row = pDb->GetRow();
        if(row == NULL)
            return;
        uint8 type = atoi(row[0]);
        const char *ip = "127.0.0.1";
        if(type == 1)
        {
            snprintf(buf,255,"wget  \"http://%s:8100/DBTest/go2ypay_jh.jsp"\
                        "?userid=%s&cardid=%s&cardpsw=%s&money=%s\" &",
                        ip,row[4],row[1],row[2],row[3]);
        }//type 1神州行 2  sd  3 zt   4 lt
        else if(type == 2)
        {
            snprintf(buf,255,"wget  \"http://%s:8100/DBTest/go2ypay_jhsd.jsp"\
                        "?userid=%s&cardid=%s&cardpsw=%s&money=%s\" &",
                        ip,row[4],row[1],row[2],row[3]);
        }
        else if(type == 3)
        {
            snprintf(buf,255,"wget \"http://%s:8100/DBTest/go2ypay_jhzt.jsp"\
                        "?userid=%s&cardid=%s&cardpsw=%s&money=%s\" &",
                        ip,row[4],row[1],row[2],row[3]);
        }
        else if(type == 4)
        {
            snprintf(buf,255,"wget \"http://%s:8100/DBTest/go2ypay_jhlt.jsp"\
                        "?userid=%s&cardid=%s&cardpsw=%s&money=%s\" &",
                        ip,row[4],row[1],row[2],row[3]);
        }
        
        system(buf);
        snprintf(buf,255,"update chong_tongbao set state = 1 where id=%s",row[5]);
        pDb->Query(buf);
        snprintf(buf,255,"rm -rf go2ypay_jh* &");
        system(buf);
    }
}

extern void RandSelWaZhongPoint();
extern time_t GetWaZhongTime();
extern bool InWaZhongHuoDong();
extern void SetWaZhongTime(time_t t);
extern void HuoDongTimer();
extern void MLSTimer();
extern void WWTimer();

void GongGao()
{
    time_t t = GetSysTime();
    struct tm *pTm = localtime(&t);
    if(pTm == NULL)
        return;
    int hour = pTm->tm_hour;
    int min = pTm->tm_min;
    if((pTm->tm_wday == 1) || (pTm->tm_wday == 3) || (pTm->tm_wday == 5))// || (pTm->tm_wday == 1))
    {
        if((hour == 11) && (min <= 30))
        {
            if((min % 5 == 0) && (pTm->tm_sec <= 0))
            {
#ifdef QQ
                SysInfoToAllUser("11:00-12:00【科考送经验】正在越亭郡北文曲星处进行,答题给经验可上榜！");
#else
                SysInfoToAllUser("11:00-12:00【无上智慧榜活动】正在越亭郡北文曲星处进行,大家快去答题吧！");
#endif
            }
        }
    }
}

void CMainClass::IdleThread()
{
    time_t t = time(NULL);
    m_sendTime = 0;
    m_readMsgTime = 0;
    
    time_t setScriptValTime = 0;
    time_t saveBangpai = GetSysTime();
    
    time_t fight = GetSysTime();
    CSceneManager &sceneMgr = SingletonSceneManager::instance();
    CBangPaiManager &bangMgr = SingletonCBangPaiManager::instance();
    while(sExit)
    {
        if(GetSysTime() - t > 20)
        {
            t = GetSysTime();
            SendMsgToUser();
        }
        if(GetSysTime() - setScriptValTime > 2*3600)
        {
            time_t t = GetSysTime();
            struct tm *pTm = localtime(&t);
            if(pTm == NULL)
                return;
            if((pTm->tm_wday == 1) && (pTm->tm_hour == 0))
            {
                SetScriptVal(0,0);
                SetScriptVal(1,0);
                SetScriptVal(2,0);
                SetScriptVal(3,0);
                SetScriptVal(4,0);
                SetScriptVal(5,0);
                SetScriptVal(6,0);
                SetScriptVal(7,0);
            }
        }
        if(GetSysTime() - saveBangpai > 3600)
        {
            bangMgr.SaveZhongZhi();
            saveBangpai = GetSysTime();
        }
        if(GetSysTime() - fight > 5)
        {
            fight = GetSysTime();
            m_fightMgr.RunFightTimeOut();
            sceneMgr.Timer();
            bangMgr.Timer();
        }

#ifdef QQ
        QQChongZhi();
#else
        ChongZhi();
#endif
        MLSTimer();//马拉松
        GongGao();
        HDTimer();//乌龟赛跑
        //HuoDongTimer();//科考送通宝
        
        /*if(InWaZhongHuoDong() && (GetSysTime() - GetWaZhongTime() > 300))
        {
            RandSelWaZhongPoint();
            SetWaZhongTime(GetSysTime());
        }*/
#ifndef NEW_QU
        WWTimer();//仙境之战，新区无
#endif
        sleep(1);
    }
}

extern void SaveMarryList();
extern void ReadMarryList();
extern void ReadTongJi();
extern void SaveTongJi();
extern void ReadWWInfo();
extern void SaveWWInfo();

void CMainClass::Run()
{
    //int count = 0;
    //time_t t = GetSysTime();
    ReadMarryList();
    ReadTongJi();
    ReadWWInfo();
    ReadTouZhu();
    while(sExit)
    {
        m_socketServer.DespatchEvent(1000);
        //m_fightMgr.RunFightTimeOut();
        /*if(GetSysTime() - t > 5)
        {
            t = GetSysTime();
            m_fightMgr.RunFightTimeOut();
        }*/
    }
    SaveTouZhu();
    SaveWWInfo();
    SaveMarryList();
    SaveTongJi();
    Join();
}

CMainClass *gpMain;

#ifdef TEST
int main(int argc,char **argv)
{
    uint8 data[256];
    for(int i = 0; i < 256; i++)
    {
        data[i] = i;
    }
    string str;
    Compress(data,sizeof(data),str);
    cout<<str.length()<<endl;
    return 0;
}
#else
int main(int argc,char **argv)
{
    {
        
        //CCallScript readItem("add_item.lua");
        //readItem.Call("AddItem","d",0);
        //readItem.Call("AddMonster","d",0);
        
        
        string user = CIniFile::GetValue("username","database",gConfigFile);
        string password = CIniFile::GetValue("password","database",gConfigFile);
        string host = CIniFile::GetValue("host","database",gConfigFile);
        string db = CIniFile::GetValue("dbname","database",gConfigFile);
        string port = CIniFile::GetValue("port","database",gConfigFile);
    
        CDbPool *pPool = CDbPool::CreateInstance();
        pPool->SetDbConfigure(user,password,host,db,port);
        
        ReadMonster();
        ReadItem();
        InitMission();
    }
    srand(time(NULL));
    //acl_init();
    ReadScriptVal();
    
    CMainClass *pMain = new CMainClass;
    if(pMain->Init())
    {
        gpMain = pMain;
        pMain->Run();
    }
    
    WriteScriptVal();
    
    delete pMain;
    //acl_destroy();
    return 0;
}
#endif

