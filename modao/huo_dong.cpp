#include "huo_dong.h"
#include "singleton.h"
#include "scene_manager.h"
#include "database.h"

enum ESPState
{
    ESPNormal,
    ESPLei,
    ESPFeng,
    ESPRest,//休息
    ESPEnd
};

struct SWuGuiInfo
{
    uint16 id;
    uint16 y;
    uint16 speed;
    uint16 tili;//体力	像素/秒	霉运(基础概率）
    uint8 direct;
    uint8 meiYun; 
    uint8 state;
    time_t stopEnd;//休息结束时间
    int shitou;
    int yugan;
    int touzhu;//投注数
    double x;
};

struct STouZhuUser
{
    uint32 roleId;
    int first;
    int second;
    int times;
};

const uint16 END_POINT = 86*16;
static bool sSaiPaoBegin = false;
static time_t sMoveTime = 0;
const int WU_GUI_NUM = 3;
static SWuGuiInfo sWuGuiList[WU_GUI_NUM];
static uint16 sWuGuiMingCi[WU_GUI_NUM];
const int WU_GUI_INFOS[WU_GUI_NUM*5] = {85,100,400,60,61,
                                        90,105,370,40,41,
                                        95,110,340,20,21};

static list<STouZhuUser> sTouZhuUserList;
static list<STouZhuUser> sOldTouZhuUserList;
static int sTouZhuUserNum = 0;
//static int sTolZhuNum     = 0;

const int MAX_TILI = 110;
const int MAX_SPEED = 5;
const int MAX_MEIYUN = 90;
const int MIN_TILI = 30;
const int MIN_SPEED = 2;
const int MIN_MEIYUN = 20;

bool InSaiPao()
{
    return sSaiPaoBegin;
}

void UseYuGan(int id)
{
    for(int i = 0; i < WU_GUI_NUM; i++)
    {
        if(sWuGuiList[i].id == id)
        {
            if(sWuGuiList[i].state == ESPRest)
            {
                sWuGuiList[i].tili += 2;
            }
            else
            {
                sWuGuiList[i].yugan++;
                sWuGuiList[i].tili += 2;
            }
        }
    }
}

void UseShiTou(int id)
{
    for(int i = 0; i < WU_GUI_NUM; i++)
    {
        if((sWuGuiList[i].id == id) && (sWuGuiList[i].state != ESPRest))
        {
            if(sWuGuiList[i].tili < 50)
                return;
            sWuGuiList[i].shitou++;
            sWuGuiList[i].tili -= 3;
            if(sWuGuiList[i].tili < MIN_TILI)
                sWuGuiList[i].tili = MIN_TILI;
        }
    }
}

void WuGuiSaiPaoBegin()
{
    CSceneManager &sceneMgr = SingletonSceneManager::instance();
    CScene *pScene = sceneMgr.FindScene(280);
    
    if(pScene != NULL)
    {
        uint8 pos[WU_GUI_NUM] = {0,1,2};
        for(int i = 0; i < WU_GUI_NUM; i++)
        {
            uint8 r = Random(0,2);
            std::swap(pos[r],pos[(r+1)/WU_GUI_NUM]);
        }
        for(int i = 0; i < WU_GUI_NUM; i++)
        {
            sWuGuiMingCi[i] = 0;
            sWuGuiList[i].id = 160+i;
            sWuGuiList[i].x = 56;
            sWuGuiList[i].y = 232+3*i*16;
            sWuGuiList[i].direct = 6;
            sWuGuiList[i].shitou = 0;
            sWuGuiList[i].yugan = 0;
            sWuGuiList[i].speed = WU_GUI_INFOS[5*pos[i]+2];
            sWuGuiList[i].tili = Random(WU_GUI_INFOS[5*pos[i]],WU_GUI_INFOS[5*pos[i]+1]);
            sWuGuiList[i].meiYun = Random(WU_GUI_INFOS[5*pos[i]+3],WU_GUI_INFOS[5*pos[i]+4]);
            sWuGuiList[i].state = ESPNormal;
            pScene->AddNpc(sWuGuiList[i].id,(int)sWuGuiList[i].x/16,sWuGuiList[i].y/16,sWuGuiList[i].direct);
        }
        sMoveTime = GetSysTime();
        CNetMessage msg;
        msg.SetType(MSG_SERVER_VISUAL_EFFECT);
        msg<<(uint8)1;
        SendSceneMsg(msg,pScene);
    }
}

static time_t sJiSuanMenYun = 0;

const char *NORMAL_SPEAK[] = {
    "赛跑不是龟干的事[#6]"
    ,"再砸我就把头缩进去[#4]"
    ,"别怨我跑得慢~龟壳很重"
    ,"再砸我就脑袋缩起来[#4]"
    ,"你们选我是对的"
    ,"我是忍者神龟[#9]"
    ,"为了参加比赛我减了几斤"
    ,"希望龙卷风把我吹到终点"
    ,"说龟速度慢，那是谣传"
    ,"昨晚happy都没劲了[#7]"
    ,"我最喜欢吃鱼干了[#6]"
    ,"恨爹不是刚"
    ,"唉，不给力呀！"
    ,"跑得不是步，是寂寞"
};

//被雷劈时语句			
const char *LEI_SPEAK[] = {
"我的脑袋现在有点晕[#8]",
"我刚做的发型被毁了[#6]	"
};		

//中龙卷风时语句			
const char *FENG_SPEAK[] = {
"这么大的风还是休息一下",																																																																																																																																																																																																																																																															
"还是躲在龟壳里安全[#6]"
};		
			
//恢复体力时语句			
const char *TILI_HUIFU_SPEAK[] = {
"你们下手也太狠了[#6]",		
"这还真不是龟该干的事"
};
			
//已经有龟到达终点时，最慢的那只龟语句			
const char *LAST_WUGUI_SPEAK[] = {
"唉，看来没希望了[#8]",
"加速，大家给我鱼干[#13]"
};

static time_t sSpeakTime = 0;

void JiSuanPos(CScene *pScene,int timeSpace)
{
    uint16 speakNcpId = 0;
    const char *pChatMsg = NULL;
    bool haveWin = false;
    double runDistance = END_POINT;
    uint16 lastNpcId = 0;
    
    for(int i = 0; i < WU_GUI_NUM; i++)
    {
        if(sWuGuiList[i].state == ESPEnd)
        {
            haveWin = true;
        }
        else if(runDistance > sWuGuiList[i].x)
        {
            runDistance = sWuGuiList[i].x;
            lastNpcId = sWuGuiList[i].id;
        }
        if((speakNcpId == 0) && (Random(0,100) < 30))
        {
            if(GetSysTime() - sSpeakTime > 60)
            {
                speakNcpId = sWuGuiList[i].id;
                uint8 pos = Random(0,sizeof(NORMAL_SPEAK)/sizeof(char*)-1);
                pChatMsg = NORMAL_SPEAK[pos];
                sSpeakTime = GetSysTime();
            }
        }
        CSceneManager &sceneMgr = SingletonSceneManager::instance();
        CScene *pScene = sceneMgr.FindScene(280);
        if(pScene != NULL)
        {
            pScene->ModifyNpcPos(sWuGuiList[i].id,(int)sWuGuiList[i].x/16,sWuGuiList[i].y/16);
        }
        
        CNetMessage msgLei;
        CNetMessage msgFeng;
        bool haveLei = false;
        bool haveFeng = false;
        if((GetSysTime() - sJiSuanMenYun >= 10) && (sWuGuiList[i].state == ESPNormal))
        {
            int meiyun = (int)(sWuGuiList[i].meiYun + sWuGuiList[i].touzhu*10/30
                + sWuGuiList[i].yugan/30*2 - sWuGuiList[i].shitou/30*2);
            if(meiyun > MAX_MEIYUN)
                meiyun = MAX_MEIYUN;
            else if(meiyun < MIN_MEIYUN)
                meiyun = MIN_MEIYUN;
            
            //cout<<i<<"  meiyun:"<<meiyun<<" tili:"<<sWuGuiList[i].tili<<endl;
                
            if(Random(0,100) < meiyun)
            {
                if(Random(0,100) < 70)
                {
                    msgLei.SetType(MSG_SERVER_NPC_EFFECT);
                    msgLei<<sWuGuiList[i].id<<(uint8)1;
                    sWuGuiList[i].stopEnd = GetSysTime()+2;
                    sWuGuiList[i].state = ESPLei;
                    sWuGuiList[i].tili -= 3;
                    haveLei = true;
                    if((speakNcpId == 0) && (Random(0,100) < 20))
                    {
                        speakNcpId = sWuGuiList[i].id;
                        uint8 pos = Random(0,sizeof(LEI_SPEAK)/sizeof(char*)-1);
                        pChatMsg = LEI_SPEAK[pos];
                    }
                }
                else
                {
                    msgFeng.SetType(MSG_SERVER_NPC_EFFECT);
                    msgFeng<<sWuGuiList[i].id<<(uint8)2;
                    sWuGuiList[i].stopEnd = GetSysTime()+3;
                    sWuGuiList[i].state = ESPFeng;
                    sWuGuiList[i].tili -= 5;
                    haveFeng = true;
                    if((speakNcpId == 0) && (Random(0,100) < 20))
                    {
                        speakNcpId = sWuGuiList[i].id;
                        uint8 pos = Random(0,sizeof(FENG_SPEAK)/sizeof(char*)-1);
                        pChatMsg = FENG_SPEAK[pos];
                    }
                }
            }
        }
        if(sWuGuiList[i].state == ESPRest)
        {
            sWuGuiList[i].tili += 2;
        }
        if((sWuGuiList[i].state == ESPFeng) || (sWuGuiList[i].state == ESPLei))
        {
            if(sWuGuiList[i].stopEnd < GetSysTime())
                sWuGuiList[i].state = ESPNormal;
        }
        if(sWuGuiList[i].tili <= 30)
        {
            sWuGuiList[i].state = ESPRest;
            sWuGuiList[i].stopEnd = GetSysTime()+3;
            
            if((speakNcpId == 0) && (Random(0,100) < 20))
            {
                speakNcpId = sWuGuiList[i].id;
                uint8 pos = Random(0,sizeof(TILI_HUIFU_SPEAK)/sizeof(char*)-1);
                pChatMsg = TILI_HUIFU_SPEAK[pos];
            }
        }
        if((sWuGuiList[i].tili >= 50) && (sWuGuiList[i].state == ESPRest))
        {
            if(sWuGuiList[i].stopEnd < GetSysTime())
                sWuGuiList[i].state = ESPNormal;
        }
        CNetMessage msg;
        msg.SetType(MSG_SERVER_NPC_MOVE);
        
        if((sWuGuiList[i].stopEnd >= GetSysTime())
            || (sWuGuiList[i].state == ESPRest))
        {
            msg<<sWuGuiList[i].id<<(uint16)sWuGuiList[i].x<<sWuGuiList[i].y
                <<sWuGuiList[i].direct<<(uint16)0;
        }
        else
        {
            msg<<sWuGuiList[i].id<<(uint16)sWuGuiList[i].x<<sWuGuiList[i].y
                <<sWuGuiList[i].direct<<sWuGuiList[i].speed;
        }
        
        CSocketServer &sock = SingletonSocket::instance();
        list<uint32> userList;
        pScene->GetUserList(userList);
        COnlineUser &onlineUser = SingletonOnlineUser::instance();
        
        for(list<uint32>::iterator iter = userList.begin(); iter != userList.end(); iter++)
        {
            ShareUserPtr ptr = onlineUser.GetUserByRoleId(*iter);
            CUser *pUser = ptr.get();
            if(pUser == NULL) 
                continue;
            sock.SendMsg(pUser->GetSock(),msg);
            
            if(haveFeng)
            {
                sock.SendMsg(pUser->GetSock(),msgFeng);
            }
            else if(haveLei)
            {
                sock.SendMsg(pUser->GetSock(),msgLei);
            }
            //else if(sWuGuiList[i].state == ESPRest)
        }
        double speed;
        if((sWuGuiList[i].stopEnd >= GetSysTime()) 
            || (sWuGuiList[i].state == ESPEnd)
            || (sWuGuiList[i].state == ESPRest))
        {
            speed = 0;
        }
        else
        {
            speed = (double)sWuGuiList[i].speed/100;
            speed += (double)sWuGuiList[i].yugan/30/6;
            speed -= (double)sWuGuiList[i].shitou/30/6;
            if(speed > MAX_SPEED)
                speed = MAX_SPEED;
            if(speed < MIN_SPEED)
                speed = MIN_SPEED;
        }
            
        sWuGuiList[i].x += speed*timeSpace;
        //cout<<i<<"  speed:"<<speed<<endl
            //<<" x:"<<sWuGuiList[i].x<<" y:"<<sWuGuiList[i].y<<endl;
            
        if(sWuGuiList[i].x >= END_POINT)
        {
            bool haveMingCi = false;
            sWuGuiList[i].state = ESPEnd;
            sWuGuiList[i].x = END_POINT;
            for(int j = 0; j < WU_GUI_NUM; j++)
            {
                if(sWuGuiMingCi[j] == sWuGuiList[i].id)
                {
                    haveMingCi = true;
                    break;
                }
            }
            if(!haveMingCi)
            {
                for(int j = 0; j < WU_GUI_NUM; j++)
                {
                    if(sWuGuiMingCi[j] == 0)
                    {
                        sWuGuiMingCi[j] = sWuGuiList[i].id;
                        break;
                    }
                }
            }
        }
    }
    if(GetSysTime() - sJiSuanMenYun >= 10)
        sJiSuanMenYun = GetSysTime();

    
    if((haveWin) && (GetSysTime() - sSpeakTime > 60))
    {
        if(Random(0,100) < 30)
        {
            speakNcpId = lastNpcId;
            uint8 pos = Random(0,sizeof(LAST_WUGUI_SPEAK)/sizeof(char*)-1);
            pChatMsg = LAST_WUGUI_SPEAK[pos];
            sSpeakTime = GetSysTime();
        }
    }
    if((speakNcpId != 0) && (pChatMsg != NULL))
    {
        CNetMessage chat;
        chat.SetType(MSG_SERVER_NPC_SAY);
        chat<<speakNcpId<<pChatMsg;
        SendSceneMsg(chat,pScene);
    }
}

//结束87*16
void HDTimer()
{
    time_t t = GetSysTime();
    struct tm *pTm = localtime(&t);
    if(pTm == NULL)
        return;
    int hour = pTm->tm_hour;
    int min = pTm->tm_min;
    int sec = pTm->tm_sec;
    if((pTm->tm_wday != 2) && (pTm->tm_wday != 4) && (pTm->tm_wday != 6))
        return;
        
    //if(((hour == 12) || (hour == 13)) && (min >= 30))
    if((hour == WU_GUI_BEGIN_TIME-1) && (min >= 30))
    {
        if((sec <= 0) && (min % 5 == 0))
        {
#ifdef QQ
            SysInfoToAllUser("【小龟跑得快】第一场11:00开始，请通过越亭郡车夫传送到欢乐场,找驯兽师处提前下注");
#else
            SysInfoToAllUser("【小龟快跑】第一场11:00开始，请通过越亭郡车夫传送到欢乐场,找驯兽师处提前下注");
#endif
        }
    }
    if((hour == WU_GUI_BEGIN_TIME) && (min >= 30))
    {
        if((sec <= 0) && (min % 5 == 0))
        {
#ifdef QQ
            SysInfoToAllUser("【小龟跑得快】第二场12:00开始，请通过越亭郡车夫传送到欢乐场,找驯兽师处提前下注");
#else
            SysInfoToAllUser("【小龟快跑】第二场12:00开始，请通过越亭郡车夫传送到欢乐场,找驯兽师处提前下注");
#endif
        }
    }
    if(((hour == WU_GUI_BEGIN_TIME) || (hour == WU_GUI_BEGIN_TIME+1)) && (min <= 3) && !sSaiPaoBegin)
    //if(!sSaiPaoBegin)
    {
        WuGuiSaiPaoBegin();
        sSaiPaoBegin = true;
    }
    if(sMoveTime == 0)
        sMoveTime = GetSysTime();
    if(sSaiPaoBegin)
    {
        CSceneManager &sceneMgr = SingletonSceneManager::instance();
        CScene *pScene = sceneMgr.FindScene(280);
        if(pScene == NULL)
            return;
            
        int timeSpace = GetSysTime() - sMoveTime;
        if(timeSpace <= 0)
            return;
            
        sMoveTime = GetSysTime();
        JiSuanPos(pScene,timeSpace);
        
        bool gameOver = true;
        for(int i = 0; i < WU_GUI_NUM; i++)
        {
            if(sWuGuiList[i].x < END_POINT)
            {
                gameOver = false;
            }
        }
        
        if(gameOver)
        {
            CNetMessage msg;
            msg.SetType(MSG_SERVER_VISUAL_EFFECT);
            msg<<(uint8)1;
            list<uint32> userList;
            pScene->GetUserList(userList);
            COnlineUser &onlineUser = SingletonOnlineUser::instance();
            CSocketServer &sock = SingletonSocket::instance();
            char buf[128];
            SNpcInstance *pNpc = pScene->FindNpc(sWuGuiMingCi[0]);
            CNetMessage sysMsg1;
            sysMsg1.SetType(PRO_SYSTEM_INFO);
            if(pNpc != NULL)
            {
                sprintf(buf,"恭喜%s获得第一名",pNpc->pNpc->name.c_str());
                sysMsg1<<buf;
            }
            pNpc = pScene->FindNpc(sWuGuiMingCi[1]);
            CNetMessage sysMsg2;
            sysMsg2.SetType(PRO_SYSTEM_INFO);
            if(pNpc != NULL)
            {
                sprintf(buf,"恭喜%s获得第二名，本次活动结束。",pNpc->pNpc->name.c_str());
                sysMsg2<<buf;
            }
            for(list<uint32>::iterator iter = userList.begin(); iter != userList.end(); iter++)
            {
                ShareUserPtr ptr = onlineUser.GetUserByRoleId(*iter);
                CUser *pUser = ptr.get();
                if(pUser == NULL) 
                    continue;
                sock.SendMsg(pUser->GetSock(),msg);
                sock.SendMsg(pUser->GetSock(),sysMsg1);
                sock.SendMsg(pUser->GetSock(),sysMsg2);
            }
            CGetDbConnect getDb;
            CDatabaseSql *pDb = getDb.GetDbConnect();
            int yugan = 0;
            int shitou = 0;
            int xiazhu = 0;
            for(list<STouZhuUser>::iterator i = sTouZhuUserList.begin(); i != sTouZhuUserList.end(); i++)
            {
                xiazhu += (*i).times;
            }
            for(uint8 i = 0; i < WU_GUI_NUM; i++)
            {
                yugan += sWuGuiList[i].yugan;
                shitou += sWuGuiList[i].shitou;
                sWuGuiList[i].touzhu = 0;
            }
            if(pDb != NULL)
            {
                char sql[128];            
                pDb->Query("truncate table touzhu_info");
                snprintf(sql,128,"INSERT INTO touzhu_info (yugan,shitou,xiazhu,jiangli) VALUES (%d,%d,%d,0)",yugan,shitou,xiazhu);
                pDb->Query(sql);
            }
            pScene->DelNpc(160);
            pScene->DelNpc(161);
            pScene->DelNpc(162);
            
            sOldTouZhuUserList = sTouZhuUserList;
            sTouZhuUserList.clear();
            
            //sTolZhuNum = 0;
            userList.clear();
            
            for(list<STouZhuUser>::iterator i = sOldTouZhuUserList.begin(); i != sOldTouZhuUserList.end(); i++)
            {
                //sTolZhuNum += (*i).times;
                if(find(userList.begin(),userList.end(),i->roleId) == userList.end())
                    userList.push_back(i->roleId);
            } 
            sTouZhuUserNum = userList.size();
            
            sSaiPaoBegin = false;
            
            SaveTouZhu();
        }
    }
}

static void AddXiaZhu(uint32 roleId,int first,int second,int num)
{
    for(int i = 0; i < WU_GUI_NUM; i++)
    {
        if(sWuGuiList[i].id == first)
        {
            sWuGuiList[i].touzhu += num;
        }
        if(sWuGuiList[i].id == second)
        {
            sWuGuiList[i].touzhu += num;
        }
    }
}

int UserTouZhu(uint32 roleId,int first,int second,int num)
{
    //当其中一个NPC投注数是(400+玩家投注数）>=1/2时，其他玩家在对此NPC下注时提示
    uint8 times = 0;
    
    //投注组合
    uint16 touZhuZuHe[] = 
    {
        160,161,
        160,162,
        161,160,
        161,162,
        162,160,
        162,161,
        160,0,  
        161,0,  
        162,0  
    };
    const int ZUHE_NUM = sizeof(touZhuZuHe)/sizeof(uint16)/2;
    int touZhuNum[ZUHE_NUM] = {0};
    int normalZhu = 0;
    int expertZhu = 0;
    
    for(list<STouZhuUser>::iterator i = sTouZhuUserList.begin(); i != sTouZhuUserList.end(); i++)
    {
        if((*i).roleId == roleId)
        {
            //haveTouzhu = true;
            times += (*i).times;
            //if(times + num > 40)
                //return 1;
        }
        for(uint8 j = 0; j < ZUHE_NUM; j++)
        {
            if((touZhuZuHe[2*j] == i->first) && (touZhuZuHe[2*j+1] == i->second))
            {
                touZhuNum[j] += i->times;
                break;
            }
        }
        if(i->second == 0)
            normalZhu += i->times;
        else
            expertZhu += i->times;
    }
    int tolNum;
    if(second == 0)
        tolNum = normalZhu;
    else
        tolNum = expertZhu;
        
    for(uint8 j = 0; j < ZUHE_NUM; j++)
    {
        if((touZhuZuHe[2*j] == first) && (touZhuZuHe[2*j+1] == second))
        {
            if(touZhuNum[j]+num >= (tolNum+200-touZhuNum[j])/2)
            {
                return 2;
            }
            break;
        }
    }
    STouZhuUser touzhu;
    touzhu.roleId = roleId;
    touzhu.first = first;
    touzhu.second = second;
    touzhu.times = num;
    sTouZhuUserList.push_back(touzhu);
    AddXiaZhu(roleId,first,second,num);
    //if(!haveTouzhu)
        //sTouZhuUserNum++;
        
    return 0;
}

int GetNormalZhu(int guanjun)
{
    int times = 0;
    for(list<STouZhuUser>::iterator i = sTouZhuUserList.begin(); i != sTouZhuUserList.end(); i++)
    {
        if(((*i).first == guanjun) && ((*i).second == 0))
        {
            times += (*i).times;
        }
    }
    return times;
}

//得到专家下注数 
int GetExpertZhu(int guanjun,int yajun)
{
    int times = 0;
    for(list<STouZhuUser>::iterator i = sTouZhuUserList.begin(); i != sTouZhuUserList.end(); i++)
    {
        if(((*i).first == guanjun) && ((*i).second == yajun))
        {
            times += (*i).times;
        }
    }
    return times;
}

//得到我的下注数 
int GetMyZhu(CUser *pUser,int guanjun,int yajun)
{
    if(pUser == NULL)
        return 0;
    int times = 0;
    uint32 roleId = pUser->GetRoleId();
    for(list<STouZhuUser>::iterator i = sTouZhuUserList.begin(); i != sTouZhuUserList.end(); i++)
    {
        if(((*i).roleId == roleId) && ((*i).first == guanjun) && ((*i).second == yajun))
        {
            times += (*i).times;
        }
    }
    return times;
}

/*
普通玩家获得的奖金=个人猜中数*600+活动基金/10/猜总投注数
专家玩家获得的奖金=个人猜中数*600+活动基金/3/猜中总注数）												
每次活动时系统出5W游戏币作为基础奖励基金												
参与玩家每增加10人，系统增加投入5W游戏币，上限50W												
*/
int GetJingcaiAward(CUser *pUser)
{
    if(pUser == NULL)
        return 0;
        
    int jiJin = (sTouZhuUserNum/10+1)*50000;//活动基金
    if(jiJin > 500000)
        jiJin = 500000;
        
    int meNormalCaiZhong = 0;
    int meExpertCaiZhong = 0;
    int normalTol = 0;
    int expertTol = 0;
    int caiNormal = 0;
    int caiExpert = 0;
    
    uint32 roleId = pUser->GetRoleId();
    for(list<STouZhuUser>::iterator i = sOldTouZhuUserList.begin(); i != sOldTouZhuUserList.end(); i++)
    {
        if((*i).roleId == roleId)
        {
            if(((*i).first == sWuGuiMingCi[0]) && ((*i).second == sWuGuiMingCi[1]))
            {
                meExpertCaiZhong += (*i).times;
            }
            else if(i->first == sWuGuiMingCi[0])
            {
                meNormalCaiZhong += (*i).times;
            }
            i->roleId = 0;
        }
        if(((*i).first == sWuGuiMingCi[0]) && ((*i).second == sWuGuiMingCi[1]))
        {
            caiExpert += (*i).times;
        }
        else if(i->first == sWuGuiMingCi[0])
        {
            caiNormal += (*i).times;
        }
        if(((*i).first != 0) && ((*i).second != 0))
            expertTol += (*i).times;
        else
            normalTol += (*i).times;
    }
    if(caiNormal <= 0)
        caiNormal = 1;
    if(caiExpert <= 0)
        caiExpert = 1;
    int normal = 0;
    if(meNormalCaiZhong > 0)
        normal = (int)(((jiJin*2/5+normalTol*600)*0.7)*meNormalCaiZhong/caiNormal);
    int expert = 0;
    if(meExpertCaiZhong > 0)
        expert = (int)(((jiJin*3/5+expertTol*600)*0.9)*meExpertCaiZhong/caiExpert);
    normal += expert;
    if(normal > 0)
    {
        pUser->AddMoney(normal);
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        char sql[128];            
        snprintf(sql,128,"update touzhu_info set jiangli=jiangli+%d",normal);
        pDb->Query(sql);
    }
    //记录鱼干、石头使用情况，下注情况，每个人领取奖金
    return normal;
}

void SaveTouZhu()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    char sql[1024];
    
    pDb->Query("truncate table touzhu");
    for(list<STouZhuUser>::iterator i = sTouZhuUserList.begin(); i != sTouZhuUserList.end(); i++)
    {
        snprintf(sql,1024,"INSERT INTO touzhu (role_id,guanyu,yanjun,times,type) VALUES (%d,%d,%d,%d,0)",
            i->roleId,i->first,i->second,i->times);
        pDb->Query(sql);
    }
    for(list<STouZhuUser>::iterator i = sOldTouZhuUserList.begin(); i != sOldTouZhuUserList.end(); i++)
    {
        snprintf(sql,1024,"INSERT INTO touzhu (role_id,guanyu,yanjun,times,type) VALUES (%d,%d,%d,%d,1)",
            i->roleId,i->first,i->second,i->times);
        pDb->Query(sql);
    }
}

void ReadTouZhu()
{
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    if(!pDb->Query("select role_id,guanyu,yanjun,times from touzhu where type=0"))
        return;
    char **row;
    while((row = pDb->GetRow()) != NULL)
    {
        STouZhuUser tu;
        tu.roleId = atoi(row[0]);
        tu.first = atoi(row[1]);
        tu.second = atoi(row[2]);
        tu.times = atoi(row[3]);
        sTouZhuUserList.push_back(tu);
    }
    
    if(!pDb->Query("select role_id,guanyu,yanjun,times from touzhu where type=1"))
        return;
    while((row = pDb->GetRow()) != NULL)
    {
        STouZhuUser tu;
        tu.roleId = atoi(row[0]);
        tu.first = atoi(row[1]);
        tu.second = atoi(row[2]);
        tu.times = atoi(row[3]);
        sOldTouZhuUserList.push_back(tu);
    }
    //sTolZhuNum = 0;
    list<uint32> userList;
    
    for(list<STouZhuUser>::iterator i = sOldTouZhuUserList.begin(); i != sOldTouZhuUserList.end(); i++)
    {
        //sTolZhuNum += (*i).times;
        if(find(userList.begin(),userList.end(),i->roleId) == userList.end())
            userList.push_back(i->roleId);
    }
    sTouZhuUserNum = userList.size();
}

char *GetMingCi()
{
    if(sWuGuiMingCi[0] == 0)
        return NULL;
    static char buf[32];
    snprintf(buf,32,"%d|%d|%d",sWuGuiMingCi[0],sWuGuiMingCi[1],sWuGuiMingCi[2]);
    return buf;
}
