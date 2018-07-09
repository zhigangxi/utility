#ifndef _MAIN_CLASS_H_
#define _MAIN_CLASS_H_
#include "pack_deal.h"
#include "singleton.h"
#include "socket_server.h"

//160 越亭郡东门(51) 161 天戟峰(71) 162  天涯海角(59) 类型：明怪（被杀死后，每1分钟刷新一次）
const uint16 YE_ZHU_WANG_ID         = 0x70;
const uint16 QIAN_NIAN_JIANG_SHI_ID = 0x71;
const uint16 THREE_SISTER_ID        = 0x72;

class CUser;
class CMainClass
{
public:
    CMainClass():m_despatch(SingletonDespatch::instance()),
        m_socketServer(SingletonSocket::instance()),
        m_fightMgr(SingletonFightManager::instance()),
        m_onlineUser(SingletonOnlineUser::instance()),
        m_thread(NULL),m_nianShouBegin(0),m_inNianShou(false),
        m_guiJieBegin(0),m_inGuiJie(false),m_addMonster(0)
    {
    }
    bool Init();
    void Run();
    void UserLogOut(CUser *pUser)
    {
        packDeal.UserLogout(pUser);
    }
    void SendGongGao(int sock);
    void SendSysInfo(CNetMessage *msg,CUser *pUser);
    char *GetCZGongGao()
    {
        return m_cxGongGao;
    }
private:
    void VisibleMonsterMis();
    
    void NianShouHuoDong();//活动
    //13:00-13:30,20:00-20:30
    void GuiJieHuoDong();
    
    void BeginNianShou();
    void EndNianShou();
    void InitMonster(SVisibleMonster &monster,uint8 x,uint8 y);
    
    void IdleThread();
#ifdef QQ
    void QQChongZhi();
#endif
    void ChongZhi();
    bool AddTongBao(uint32 userId,int money,char *msg,uint8 type = 0);
    
    void SendMsgToUser();
    void TimeOut();
    //void UpdateUser(int day);
    void DealPackThread();
    void Join();
    CPackageDeal packDeal;
    CDespatchCommand &m_despatch;
    CSocketServer &m_socketServer;
    CFightManager &m_fightMgr;
    COnlineUser &m_onlineUser;
    boost::thread **m_thread;
    int m_threadNum;
    time_t m_nianShouBegin;
    bool m_inNianShou;
    time_t m_guiJieBegin;
    bool m_inGuiJie;
    time_t m_addMonster;
    
    const static int MAX_NOTICE_MSG_LEN = 512;
    int m_sysInfoTimeSpace;
    time_t m_sendTime;
    time_t m_readMsgTime;
    char m_gonggao[MAX_NOTICE_MSG_LEN];
    char m_sysInfo[MAX_NOTICE_MSG_LEN];
    char m_cxGongGao[MAX_NOTICE_MSG_LEN];
};
#endif
