#include "scene_manager.h"
#include "database.h"
#include "singleton.h"
#include "socket_server.h"
#include "net_msg.h"
#include "user.h"
#include "call_script.h"
#include "script_call.h"
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/if.hpp>
using namespace boost;

//组队

class CUserTeam
{
public:
    CUserTeam()
    {
        for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
        {
            m_members[i] = 0;
        }
        for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
        {
            m_leaveMembers[i] = 0;
        }
        //m_num = 0;
    }
    
    uint8 GetMemberNum()
    {
        uint8 num = 0;
        for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
        {
            if(m_members[i] != 0)
            {
                num++;
            }
        }
        return num;
    }
    uint8 GetLeaveNum()
    {
        uint8 num = 0;
        for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
        {
            if(m_leaveMembers[i] != 0)
            {
                num++;
            }
        }
        return num;
    }
    bool Join(uint32 userId);
        
    void Exit(uint32 userId);
    
    void AskForJoinTeam(uint32 userId)
    {
        m_askForJoin.push_back(userId);
    }
    bool IsAskedForJoin(uint32 userId)
    {
        list<uint32>::iterator i = m_askForJoin.begin();
        for(; i != m_askForJoin.end(); i++)
        {
            if(*i == userId)
                return true;
        }
        return false;
    }
    void DelAskForJoinTeam(uint32 userId)
    {
        m_askForJoin.remove(userId);
    }
    
    void DelAllAskForJoinTeam()
    {
        m_askForJoin.clear();
    }
    
    const char *GetHeadName()
    {
        return m_headName.c_str();
    }
    uint32 GetHeadId()//队伍id即队长id
    {
        return m_members[0];//队长即队伍里面的第一个
    }
    bool SetNewHead(CUser *pUser)
    {
        for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
        {
            if(m_members[i] == pUser->GetRoleId())
            {
                m_headName = pUser->GetName();
                m_headXiang = pUser->GetXiang();
                m_headLevel = pUser->GetLevel();
                std::swap(m_members[0],m_members[i]);
                return true;
            }
        }
        return false;
    }
    void GetMember(uint32 *members,uint8 &num);
    void GetLeaveMem(uint32 *members,uint8 &num)
    {
        num = 0;
        for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
        {
            if(m_leaveMembers[i] != 0)
            {
                members[num] = m_leaveMembers[i];
                num++;
            }
        }
    }
    list<uint32> *GetAskForJoin()
    {
        return &m_askForJoin;
    }
    
    void AddRequestList(uint32 id)
    {
        m_requestList.push_back(id);
    }
    bool InRequest(uint32 id)
    {
        for(list<uint32>::iterator i = m_requestList.begin(); i != m_requestList.end(); i++)
        {
            if(id == *i)
                return true;
        }
        return false;
    }
    void DelRequest(uint32 id)
    {
        m_requestList.remove(id);
    }
    uint8 GetHeadXiang()
    {
        return m_headXiang;
    }
    uint8 GetHeadLevel()
    {
        return m_headLevel;
    }
    void ReturnTeam(uint32 roleId)
    {
        for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
        {
            if(m_leaveMembers[i] == roleId)
            {
                m_leaveMembers[i] = 0;
                for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
                {
                    if(m_members[i] == 0)
                    {
                        m_members[i] = roleId;
                        break;
                    }
                }
                break;
            }
        }
    }
    //暂离队伍
    void TempLeaveTeam(uint32 roleId)
    {
        for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
        {
            if(m_members[i] == roleId)
            {
                m_members[i] = 0;
                for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
                {
                    if(m_leaveMembers[i] == 0)
                    {
                        m_leaveMembers[i] = roleId;
                        break;
                    }
                }
                //m_num--;
                break;
            }
        }
    }
private:
    
    uint32 m_members[MAX_TEAM_MEMBER];
    uint32 m_leaveMembers[MAX_TEAM_MEMBER];//暂离
    string m_headName;
    uint8 m_headXiang;
    uint8 m_headLevel;
    
    list<uint32> m_askForJoin;//请求加入的玩家
    list<uint32> m_requestList;//邀请列表
    //uint8 m_num;
};

void CUserTeam::GetMember(uint32 *members,uint8 &num)
{
    num = 0;
    for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
    {
        if(m_members[i] != 0)
        {
            members[num] = m_members[i];
            num++;
        }
    }
}

bool CUserTeam::Join(uint32 userId)
{
    for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
    {
        if(m_members[i] == userId)
        {
            return false;
        }
        else if(m_members[i] == 0)
        {
            m_members[i] = userId;
            if(i == 0)
            {
                COnlineUser &m_onlineUser = SingletonOnlineUser::instance();
                ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(userId);
                CUser *pUser = ptr.get();
                if(pUser != NULL)
                {
                    m_headName = pUser->GetName();
                    m_headXiang = pUser->GetXiang();
                    m_headLevel = pUser->GetLevel();
                }
            }
            //m_num++;
            return true;
        }
    }
    return false;
}

void CUserTeam::Exit(uint32 userId)
{
    for(uint8 i = 0; i < MAX_TEAM_MEMBER; i++)
    {
        if(m_members[i] == userId)
        {
            //m_num--;
            m_members[i] = 0;
        }
        if(m_leaveMembers[i] == userId)
        {
            //m_num--;
            m_leaveMembers[i] = 0;
        }
    }
}
    
CScene::CScene(CScene &scene):
m_socketServer(SingletonSocket::instance())
,m_onlineUser(SingletonOnlineUser::instance())
,m_monsterManager(SingletonMonsterManager::instance())
,m_fightManager(SingletonFightManager::instance())
{
    m_id                       = scene.m_id;
    m_mapId                    = scene.m_mapId;
    m_name                     = scene.m_name;
    m_npcList                  = scene.m_npcList;
    m_curVisibleId = 0;
    //m_userList                 = scene.m_userList;
    
    memcpy(m_monsters,scene.m_monsters,sizeof(m_monsters));
    m_monsterNum               = scene.m_monsterNum;
                               
    //此处跳转点需要重新insert，这样直接相等存在隐患
    m_jumpTo                   = scene.m_jumpTo;
    m_canWalkPos               = scene.m_canWalkPos;
    
    m_pScript = scene.m_pScript;
    m_killMonsterNum = 0;
    m_addJump = false;
    
    m_state = 0;
    m_usedFuBen = false;
    m_groupId = scene.m_groupId;
    Init();
    m_fightType = scene.m_fightType;
    m_jsBangZhanJF = 0;
    m_sendJFGongGao = 0;
    m_inBangZhan = false;
    m_x = scene.m_x;
    m_y = scene.m_y;
}

bool CScene::HaveNpc(uint8 x,uint8 y)
{
    CNpcManager &npcManager = SingletonNpcManager::instance();
    for(list<uint16>::iterator i = m_npcList.begin(); i != m_npcList.end(); i++)
    {
        SNpcInstance *pNpc = npcManager.GetNpcInstance(*i);
        if((pNpc != NULL) && (pNpc->x == x) && (pNpc->y == y))
        {
            return true;
        }
    }
    return false;
}

CScene::CScene(uint16 id,uint16 mapId,const char *name,char *monsters)
:m_id(id),m_mapId(id),m_name(name),m_jumpTo(10),m_userTeams(100)
,m_socketServer(SingletonSocket::instance())
,m_onlineUser(SingletonOnlineUser::instance())
,m_monsterManager(SingletonMonsterManager::instance())
,m_fightManager(SingletonFightManager::instance())
{
    m_isPaiMing = false;
    m_matchBegin = time(NULL);
    m_groupId = 0;
    m_curVisibleId = 0;
    char *monster[MAX_MONSTER_NUM];
    if(monsters != NULL)
    {
        m_monsterNum = SplitLine(monster,MAX_MONSTER_NUM,monsters);
        for(uint8 i = 0; i < m_monsterNum; i++)
        {
            m_monsters[i] = atoi(monster[i]);
        }
    }
    else
    {
        m_monsterNum = 0;
    }
            
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    format fmt("select from_x,from_y,to_scene,to_x,to_y,face from jump_point where from_scene=%1%");
    fmt % (int)m_id;
    if ((pDb != NULL)
        && (pDb->Query(fmt.str().c_str())))
    {
        char **row;
        while ((row = pDb->GetRow()) != NULL)
        {
            uint8 fromX = (uint8)atoi(row[0]);
            uint8 fromY = (uint8)atoi(row[1]);
            SJumpTo *pJump = new SJumpTo;
            pJump->sceneId = (uint16)atoi(row[2]);
            pJump->x = (uint8)atoi(row[3]);
            pJump->y = (uint8)atoi(row[4]);
            pJump->face = (uint8)atoi(row[5]);
            InsertJumpPoint(fromX,fromY,pJump);
        }
    }
    /*if(m_mapId == 280)
    {
        m_npcList.push_back(160);
        m_npcList.push_back(161);
        m_npcList.push_back(162);
    }*/
    CNpcManager &npcManager = SingletonNpcManager::instance();
    npcManager.GetSceneNpc(id,&m_npcList);
    
    m_pScript = NULL;
    char fileName[32];
    sprintf(fileName,"%d.lua",m_id+10000);
    if(access(fileName,R_OK) == 0)
    {
        m_pScript = new CCallScript(m_id+10000);
    }
    
    snprintf(fileName,31,"dat/%d.map",m_id);
    
    m_addJump = false;
    m_killMonsterNum = 0;
    m_state = 0;
    m_usedFuBen = false;
    m_jsBangZhanJF = 0;
    m_sendJFGongGao = 0;
    m_inBangZhan = false;
    Init();
    
    FILE *file = fopen(fileName,"r");
    if(file == NULL)
        return;
    const int MAX_MAP_SIZE = 1024*3;
    uint8 buf[MAX_MAP_SIZE];
    
    int len = fread(buf,1,MAX_MAP_SIZE,file);
    fclose(file);
    
    if(len < buf[1] * buf[2] + 3)
        return;
    uint8 canWalk[MAX_MAP_SIZE] = {0};
    uint8 x,y;
    for(int i = 3; i < buf[1]*buf[2]+3; i++)
    {
        if((buf[i]&0x80) == 0)
        {
            /*SPoint pos;
            pos.x = (i - 3)%buf[1];
            pos.y = (i - 3)/buf[1];
            m_canWalkPos.push_back(pos);*/
            x = (i - 3)%buf[1];
            y = (i - 3)/buf[1];
            if(!HaveNpc(x,y))
                canWalk[i] = 1;
        }
    }
    for(int i = 3; i < buf[1]*buf[2]+3; i++)
    {
        if(canWalk[i] == 1)
        {
            int x = (i - 3)%buf[1];
            int y = (i - 3)/buf[1];
            if((x > 0) && (y > 0) && (x < buf[1]) && (y < buf[2])
                && (canWalk[3+(x-1)+buf[1]*(y-1)] == 1)
                && (canWalk[3+(x-1)+buf[1]*(y)] == 1)
                && (canWalk[3+(x-1)+buf[1]*(y+1)] == 1)
                && (canWalk[3+(x)+buf[1]*(y-1)] == 1)
                && (canWalk[3+(x)+buf[1]*(y+1)] == 1)
                && (canWalk[3+(x+1)+buf[1]*(y-1)] == 1)
                && (canWalk[3+(x+1)+buf[1]*(y)] == 1)
                && (canWalk[3+(x+1)+buf[1]*(y+1)] == 1))
            {
                SPoint pos;
                pos.x = x;
                pos.y = y;
                m_canWalkPos.push_back(pos);
            }
        }
    }
}

CScene::~CScene()
{
    m_userList.clear();
    m_jumpTo.DelAll();
}

void CScene::InsertJumpPoint(uint8 x,uint8 y,SJumpTo *pJump)
{
    m_jumpTo.Insert(x<<8|y,pJump);    
}

bool CScene::GetJumpPoint(uint8 x,uint8 y,SJumpTo *&pJump)
{
    if(m_jumpTo.Find(x<<8|y,pJump))
        return true;
    if(m_addJump && (m_jumpPoint.x == x) && (m_jumpPoint.y == y))
    {
        pJump = &m_jumpToPoint;
        return true;
    }
    return false;
}

static void TeamMemberMove(ShareUserPtr ptr,uint8 x,uint8 y,uint8 face)
{
    ptr->SetPos(x,y);
    ptr->SetFace(face);
}

void CScene::DiaoYuFight(CUser *pUser)
{
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetCanTaoPao(false);
    
    pFight->SetFightType(CFight::EFTScript);
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 0;
        CUserTeam *pTeam = NULL;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {
            num = 2*pTeam->GetMemberNum();
            AddTeamToFight(pFight,pTeam,7);
        }
        else
        {
            num = 2;
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(pUser->GetRoleId());
            if(p.get() == NULL)
                return;
            pUser->SetFight(pFight->GetId(),pFight->AddUser(p,9));
        }
        for(uint8 i = 0; i < num; i++)
        {
            ShareMonsterPtr ptr;
            SMonsterInst *pInst = new SMonsterInst;
            ptr.reset(pInst);
            pInst->type = EMTNormal;
            pInst->tmplId = 22;
            pInst->name = "夜叉";
            if(i == 0)
                pInst->chatMsg = "贪吃的人类，受死吧";
            pInst->xiang = 0;
            pInst->level = 30;
            pInst->exp = 0;
            pInst->daohang = 0;
            pInst->maxHp = 20;
            pInst->recovery = 0x7fffffff;
            pInst->hp = pInst->maxHp;
            pInst->maxMp = 30;
            pInst->mp = pInst->maxHp;
            pInst->speed = 10000;
            pInst->attack = 300;
            pInst->addSkillAttack = 0;
            
            pInst->tizhi = 0;//体质
            pInst->liliang = 0;//力量
            pInst->minjie = 0;//敏捷
            pInst->lingxing = 0;//灵性    
            pInst->naili = 0;//耐力
            pInst->noAdd = true;    
            pFight->AddMonster(ptr,i+1);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LanLuoHu(CUser *pUser,uint16 tmplId)
{
    ShareMonsterPtr ptr;
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = tmplId;
    if(tmplId == 40)
    {
        pInst->name = "闹事者";
        pInst->chatMsg = "武功再高，也怕菜刀";
    }
    else if(tmplId == 6)
    {
        pInst->name = "野猪";
        pInst->chatMsg = "流氓会武术，谁也挡不住";
    }
    else
    {
        pInst->name = "拦路虎";
        pInst->chatMsg = "打的不是劫，是寂寞";
    }
    
    pInst->xiang = 0;
    pInst->level = 30;
    pInst->exp = 0;
    pInst->daohang = 0;
    pInst->maxHp = 30;
    pInst->recovery = 0x7fffffff;
    pInst->hp = pInst->maxHp;
    pInst->maxMp = 30;
    pInst->mp = pInst->maxHp;
    pInst->speed = 10000;
    pInst->attack = 300;
    pInst->addSkillAttack = 0;
    
    pInst->tizhi = 0;//体质
    pInst->liliang = 0;//力量
    pInst->minjie = 0;//敏捷
    pInst->lingxing = 0;//灵性    
    pInst->naili = 0;//耐力
    pInst->noAdd = true;    
    
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetCanTaoPao(false);
    pFight->AddMonster(ptr,3);
    pFight->SetFightType(CFight::EFTScript);//CFight::EFTScript);
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 0;
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            if(pUser->IsMaxMonster())
                num = 2;
            else
                num = Random(1,2);
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(pUser->GetRoleId());
            if(p.get() == NULL)
                return;
            pUser->SetFight(pFight->GetId(),pFight->AddUser(p,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::BaZouMission(CUser *pUser)
{
    const char *pMission = pUser->GetMission(22);
    if(pMission == NULL)
        return;
    
    char buf[128];
    //碰到拦路虎或者闹事者1|0|0|0|time 第二位0时触发战斗,或者以下情况
    snprintf(buf,128,"%s",pMission);
    const uint8 num = 6;
    char *p[num];
    if(SplitLine(p,num,buf) != num)
    {
        return;
    }
    int intP[num];
    for(int i = 0; i < num; i++)
    {
        intP[i] = atoi(p[i]);
    }
    if((m_mapId == 72) && (intP[5] == 1) && (Random(0,2) == 0))
    {
        //触发战斗
        LanLuoHu(pUser,25);
        intP[5] = 0;
        sprintf(buf,"%d|%d|%d|%d|%d|%d",intP[0],intP[1],intP[2],intP[3],intP[4],intP[5]);
        pUser->UpdateMission(22,buf);
    }
    else if((m_mapId == 68) && (intP[5] == 1) && (Random(0,2) == 0))
    {
        //触发战斗
        LanLuoHu(pUser,40);
        intP[5] = 0;
        sprintf(buf,"%d|%d|%d|%d|%d|%d",intP[0],intP[1],intP[2],intP[3],intP[4],intP[5]);
        pUser->UpdateMission(22,buf);
    }
    else if((m_mapId == 125) && (intP[1] == 0) && (intP[5] == 1) && (Random(0,2) == 0))
    {
        //碰到小赌鬼
        intP[1] = 3;
        intP[5] = 0;
        //DelNpc(pUser,431);
        AddDefaultNpc(pUser,431,m_mapId,pUser->GetX(),pUser->GetY());
        CNetMessage msg;
        msg.SetType(PRO_OPEN_INTERACT);
        msg<<PRO_SUCCESS;
        m_socketServer.SendMsg(pUser->GetSock(),msg);   
        
        sprintf(buf,"%d|%d|%d|%d|%d|%d",intP[0],intP[1],intP[2],intP[3],intP[4],intP[5]);
        pUser->UpdateMission(22,buf);
     
        pUser->SetCall(431,"");
        CCallScript *pScript = FindScript(431);//(name);
        if(pScript != NULL)
        {
            pUser->SetCallScript(pScript->GetScriptId());
            pScript->Call("NpcMain","u",pUser);
        }
    }
    else if((m_mapId == 88) && (intP[1] == 0) && (intP[5] == 1) && (Random(0,2) == 0))
    {//碰到乞丐
        intP[1] = 2;
        intP[5] = 0;
        if(Random(0,1) == 0)
        {//要钱
            intP[2] = 0;
        }
        else
        {//要东西
            /*if(intP[0] <= 2)
            {
                uint16 items[] = {2,3,4,52,53,54,102,103,104,152,153,154,202,203,204,655,656,657,658,659,660,705,706,707,708,709,710};
                intP[2] = RandSelect(items,sizeof(items)/sizeof(items[0]));
            }
            else
            {//
                uint16 items[] = {4,5,6,54,55,56,104,105,106,154,155,156,204,205,206,660,661,662,663,664,665,710,711,712,713,714,715};
                intP[2] = RandSelect(items,sizeof(items)/sizeof(items[0]));
            }*/
            uint16 items[] = {1,2,3,51,52,53,101,102,103,151,152,153,201,202,203,251,252,253,265,266,267,301,302,303,315,316,317,351,352,353,381,382,383,651,652,653,654,655,656,701,702,703,704,705,706};
            intP[2] = RandSelect(items,sizeof(items)/sizeof(items[0]));
        }
        AddDefaultNpc(pUser,430,m_mapId,pUser->GetX(),pUser->GetY());
        CNetMessage msg;
        msg.SetType(PRO_OPEN_INTERACT);
        msg<<PRO_SUCCESS;
        m_socketServer.SendMsg(pUser->GetSock(),msg);  
        
        sprintf(buf,"%d|%d|%d|%d|%d|%d",intP[0],intP[1],intP[2],intP[3],intP[4],intP[5]);
        pUser->UpdateMission(22,buf);
        
        pUser->SetCall(430,"");
        CCallScript *pScript = FindScript(430);//(name);
        if(pScript != NULL)
        {
            pUser->SetCallScript(pScript->GetScriptId());
            pScript->Call("NpcMain","u",pUser);
        }
    }
    else if((m_mapId == 57)&& (intP[1] == 0) && (intP[5] == 1) && (Random(0,2) == 0))
    {
        //碰到小赌鬼
        intP[1] = 3;
        intP[5] = 0;
        AddDefaultNpc(pUser,431,m_mapId,pUser->GetX(),pUser->GetY());
        CNetMessage msg;
        msg.SetType(PRO_OPEN_INTERACT);
        msg<<PRO_SUCCESS;
        m_socketServer.SendMsg(pUser->GetSock(),msg);   
        
        sprintf(buf,"%d|%d|%d|%d|%d|%d",intP[0],intP[1],intP[2],intP[3],intP[4],intP[5]);
        pUser->UpdateMission(22,buf);
     
        pUser->SetCall(431,"");
        CCallScript *pScript = FindScript(431);//(name);
        if(pScript != NULL)
        {
            pUser->SetCallScript(pScript->GetScriptId());
            pScript->Call("NpcMain","u",pUser);
        }
    }
    else if((intP[5] == 0) && (m_mapId != 72) && (m_mapId != 68) && (m_mapId != 125) && (m_mapId != 88) && (m_mapId != 57))
    {
        intP[5] = 1;
        sprintf(buf,"%d|%d|%d|%d|%d|%d",intP[0],intP[1],intP[2],intP[3],intP[4],intP[5]);
        pUser->UpdateMission(22,buf);
    }
}

void CScene::UserMove(CUser *pUser,uint8 dir,uint8 step)
{
    BaZouMission(pUser);
    
    CNetMessage msg;
    msg.ReWrite();
    msg.SetType(PRO_ROLE_MOVE);
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    msg<<pUser->GetRoleId()<<dir<<step;
    //BroadcastMsg(m_socketServer,m_onlineUser,msg);
    BroadcastMsgExcept(msg,pUser);
    
    if(pUser->GetTeam() == pUser->GetRoleId())
    {
        ForEachTeamMember(pUser->GetTeam(),
            boost::bind(TeamMemberMove,_1,pUser->GetX(),pUser->GetY(),pUser->GetFace()));
    }
}

void CScene::UpdateUserInfo(CUser *pUser)
{
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_PLAYER);
    
    CSocketServer &sock = SingletonSocket::instance();
    COnlineUser &onlineUser = SingletonOnlineUser::instance();
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    list<uint32>::iterator iter = m_userList.begin();
    for(; iter != m_userList.end(); iter++)
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

void CScene::Exit(CUser *pUser)
{
    //LeaveTeam(pUser);
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    m_userList.remove(pUser->GetRoleId());
        
    if((pUser->GetTeam() == 0) || (pUser->GetTeam() == pUser->GetRoleId()))
    {
        CNetMessage msg;
        msg.SetType(PRO_IN_OUT_SCENE);
        msg<<pUser->GetRoleId()<<(uint8)0;
        BroadcastMsgExceptSameTeam(pUser,msg);
        //BroadcastMsg(msg);
    }
    if((m_mapId == 307) || (m_mapId == 306))
    {
        if(m_userList.empty())
        {
            m_usedFuBen = false;
            m_visibleMonsters.clear();
            m_killMonsterNum = 0;
        }
    }
}

CSceneManager::CSceneManager():
m_curFuBenId(time(NULL)),
m_isInit(false),
m_npcManager(SingletonNpcManager::instance())
{
    m_matchRunTime = 0;
}

static bool InitScenePathInfo(uint16,CScene *pScene)
{
    pScene->InitPathInfo();
    return true;
}

bool CSceneManager::Init()
{
    if(m_isInit)
        return true;
    if(!m_npcManager.Init())
        return false;

    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
                        //0 1   2       3       4           5       6 7 8       9
    string sql("select id,name,map_id,monster,fight_type,fight_step,x,y,pai_ming,group_id"\
        " from game_scene");
    if ((pDb != NULL)
        && (pDb->Query(sql.c_str())))
    {
        char **row;
        while ((row = pDb->GetRow()) != NULL)
        {
            CScene *pScene = new CScene(atoi(row[0]),atoi(row[0]),row[1],row[3]);
            pScene->SetFightType(atoi(row[4]));
            pScene->SetFightStep(atoi(row[5]));
            pScene->SetX(atoi(row[6]));
            pScene->SetY(atoi(row[7]));
            pScene->SetGroupId(atoi(row[9]));
            m_sceneList.Insert(pScene->GetId(),pScene);
        }
    }
    else
    {
        return false;
    }
    m_sceneList.ForEach(boost::bind(InitScenePathInfo,_1,_2));
    
    return true;
}

/*
CSceneManager::~CSceneManager()
{
    m_sceneList.DelAll();
}
*/

CScene *CSceneManager::FindScene(int id)
{
    CScene *pScene = NULL;
    boost::mutex::scoped_lock lk(m_bpMutex);
    m_sceneList.Find(id,pScene);
    return pScene;
}

static bool EachJumpPoint(int,SJumpTo *pJumpTo,CNetMessage *pMsg,uint8 *pNum)
{
    CScene *pScene = SingletonSceneManager::instance().FindScene(pJumpTo->sceneId);
    if(pScene != NULL)
    {
        string name = "【";
        name.append(pScene->GetName());
        name.append("】");
        *pMsg<<name<<pJumpTo->sceneId<<pJumpTo->x<<pJumpTo->y;
        (*pNum)++;
    }
    return true;
}

static void InitNpc(uint16 sceneId,CNetMessage &msg,uint8 &num)
{
    CNpcManager &npcManager = SingletonNpcManager::instance();
    CSceneManager &sceneMgr = SingletonSceneManager::instance();
    
    CScene *pScene = sceneMgr.FindScene(sceneId);
    if(pScene == NULL)
        return;
        
    list<uint16> *pNpcList = pScene->GetNpcList();
    for(list<uint16>::iterator i = pNpcList->begin(); i != pNpcList->end(); i++)
    {
        SNpcInstance *pNpc = npcManager.GetNpcInstance(*i);
        if(pNpc != NULL)
        {
            msg<<pNpc->pNpc->name<<(uint16)pNpc->sceneId<<pNpc->x<<pNpc->y;
            num++;
        }
    }
}

void CScene::InitPathInfo()
{
    m_pathInfo.SetType(PRO_CLIENT_PATH);
    uint8 num = 0;
    uint16 pos = m_pathInfo.GetDataLen();
    m_pathInfo<<num;
    
    m_jumpTo.ForEach(boost::bind(EachJumpPoint,_1,_2,&m_pathInfo,&num));
    
    if((m_id >= 11) && (m_id <= 20))
    {
        for(int i = 11; i <= 20; i++)
        {
            InitNpc(i,m_pathInfo,num);
        }
    }
    else if((m_id >= 21) && (m_id <= 29))
    {
        for(int i = 21; i <= 29; i++)
        {
            InitNpc(i,m_pathInfo,num);
        }
    }
    else if((m_id >= 31) && (m_id <= 39))
    {
        for(int i = 31; i <= 39; i++)
        {
            InitNpc(i,m_pathInfo,num);
        }
    }
    else
    {
        InitNpc(m_id,m_pathInfo,num);
    }
    m_pathInfo.WriteData(pos,&num,1);
}

void CScene::SendPathInfo(int sock)
{
    m_socketServer.SendMsg(sock,m_pathInfo);
}

bool CScene::CreateTeam(CUser *pUser,uint32 request)
{
    CNetMessage msg;
    msg.SetType(PRO_USER_TEAM);
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CUserTeam *pTeam = NULL;
    if(pUser->GetTeam() != 0)
    {
        m_userTeams.Find(pUser->GetTeam(),pTeam);
    }
    
    if(pTeam == NULL)
    {
        msg<<(uint8)1;
        msg<<PRO_SUCCESS;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    
        pTeam = new CUserTeam;
        m_userTeams.Insert(pUser->GetRoleId(),pTeam);
        pTeam->Join(pUser->GetRoleId());
        
        pUser->SetTeam(pUser->GetRoleId());
    
        msg.ReWrite();
        msg.SetType(PRO_UPDATE_TEAM);
        msg<<(uint8)1<<pUser->GetRoleId();
            
        BroadcastMsg(msg);
    }
    if(request != 0)
    {
        if(pTeam->GetMemberNum() >= MAX_TEAM_MEMBER)
        {
            SendSysInfo(pUser,"队伍已满员");
            return true;
        }
        pTeam->AddRequestList(request);
        /*
        +----+-----+-----+------+-------+-------+
        | OP | CID | LEN | NAME | XIANG | LEVEL |
        +----+-----+-----+------+-------+-------+
        |  1 |  4  |  2  |  Var |   1   |   1   |
        +----+-----+-----+------+-------+-------+
        */
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(request);
        if(p.get() != NULL)// && (p->GetTeam() == 0))//pUser->GetTeam()))
        {
            if(p->GetTeam() == 0)
            {
                if(p->TempLeaveTeam() != 0)
                {
                    if(p->TempLeaveTeam() == pUser->GetTeam())
                    {
                        msg.ReWrite();
                        msg.SetType(PRO_USER_TEAM);
                        msg<<(uint8)14<<pUser->GetRoleId()<<pUser->GetName()<<pUser->GetXiang()<<pUser->GetLevel();
                        m_socketServer.SendMsg(p->GetSock(),msg);        
                    }
                    else
                    {
                        SendSysInfo(pUser,"该玩家已有队伍");
                    }
                    return true;
                }
                msg.ReWrite();
                msg.SetType(PRO_USER_TEAM);
                msg<<(uint8)6<<pUser->GetRoleId()<<pUser->GetName()<<pUser->GetXiang()<<pUser->GetLevel();
                m_socketServer.SendMsg(p->GetSock(),msg);        
            }
            else
            {
                SendSysInfo(pUser,"该玩家已有队伍");
            }
        }
    }
    
    return true;
}

void CScene::AskForJoinTeam(CUser *pUser,uint32 headId)
{
    if(pUser->GetFightId() != 0)
        return;
    
    /*ShareUserPtr pHead = m_onlineUser.GetUserByRoleId(headId);
    if(pHead.get() != NULL)
    {
        headId = pHead->GetTeam();
    }*/
    CNetMessage msg;
    msg.SetType(PRO_USER_TEAM);
        
    if(pUser->GetTeam() == 0)
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(headId,pTeam))
            return;
        if(pTeam->GetMemberNum() >= MAX_TEAM_MEMBER)
        {
            SendSysInfo(pUser,"队伍已满员");
            return;
        }
         
        if(pTeam->InRequest(pUser->GetRoleId()))
        {//队长邀请
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(headId);
            if(p.get() == NULL)
                return;
                
            pUser->SetTeam(headId);
            pTeam->DelRequest(pUser->GetRoleId());
            
            //msg<<(uint8)4<<PRO_SUCCESS;
            //m_socketServer.SendMsg(pUser->GetSock(),msg);
            pTeam->Join(pUser->GetRoleId());
            string str = pUser->GetName();
            str += "加入队伍";
            
            SendSysInfo(p.get(),str.c_str());
            SendSysInfo(pUser,str.c_str());
            msg.ReWrite();
            msg.SetType(PRO_UPDATE_TEAM);
            uint8 x,y;
            p->GetPos(x,y);
            pUser->SetPos(x,y);
            msg<<(uint8)2<<headId<<pUser->GetRoleId()<<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
            BroadcastMsg(msg);
        }
        else if(!pTeam->IsAskedForJoin(pUser->GetRoleId()))
        {
            ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(headId);
            if(ptr.get() != NULL)
            {
                msg<<(uint8)2<<PRO_SUCCESS;
                m_socketServer.SendMsg(pUser->GetSock(),msg);
                if(pUser->GetAskForJoinTeam() != 0)
                {
                    CUserTeam *pAsked = NULL;
                    if(m_userTeams.Find(headId,pAsked))
                    {//删除之前的申请
                        pAsked->DelAskForJoinTeam(pUser->GetAskForJoinTeam());
                    }
                }
                pUser->SetAskForJoinTeam(headId);
                pTeam->AskForJoinTeam(pUser->GetRoleId());
                msg.ReWrite();
                msg.SetType(PRO_USER_TEAM);
                msg<<(uint8)4<<pUser->GetRoleId()<<pUser->GetName()<<pUser->GetXiang()
                    <<pUser->GetLevel();
                m_socketServer.SendMsg(ptr->GetSock(),msg);
                return;
            }
        }
    }
}

bool CScene::MakeTeamList(uint32 id,CUserTeam *pTeam,uint8 page,CNetMessage *msg,uint8 *teamNum,uint8 *tolNum)
{
    (*tolNum)++;
    if(*teamNum >= ONE_PAGE_MAX_NUM)
        return false;

    if(*tolNum > ONE_PAGE_MAX_NUM * (page+1))
        return false;
    
    if(*tolNum < ONE_PAGE_MAX_NUM * page)
    {
        return true;
    }
    /*
    +-----+-----+------+------+ 
    | CID | LEN | NAME | MNUM | 
    +-----+-----+------+------+ 
    |  4  |  2  |  Var |  1   | 
    +-----+-----+------+------+ 
    */
    (*teamNum)++;
    *msg<<pTeam->GetHeadId()<<pTeam->GetHeadName()
        <<pTeam->GetHeadXiang()<<pTeam->GetHeadLevel()<<pTeam->GetMemberNum();
    
    return true;
}

void CScene::GetTeamList(CUser *pUser,uint8 page)
{
    page -= 1;
    /*
    +----+------+------+
    | OP | PAGE | TNUM |
    +----+------+------+
    |  1 |  1   |  1   |
    +----+------+------+
    */

    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CNetMessage msg;
    msg.SetType(PRO_USER_TEAM);
    uint8 teamNum = 0;
    uint8 tolNum = 0;
    msg<<(uint8)3<<page;
    uint16 pos = msg.GetDataLen();
    msg<<teamNum;
    
    m_userTeams.ForEach(boost::bind(&CScene::MakeTeamList,this,_1,_2,page,&msg,&teamNum,&tolNum));
    msg.WriteData(pos,&teamNum,sizeof(teamNum));
    SingletonSocket::instance().SendMsg(pUser->GetSock(),msg);
}

void CScene::ForEachUser(boost::function<void(ShareUserPtr)> f)
{
    list<uint32>::iterator iter = m_userList.begin();
    for(; iter != m_userList.end(); iter++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(*iter);
        if(p.get() == NULL) 
            continue;
        f(p);
    }
}

void CScene::NotInTeamUser(uint8 page,CNetMessage &msg)
{
    if(page < 1)
        return;

    page--;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    int begin = ONE_PAGE_MAX_NUM*page;
    
    list<uint32>::iterator iter = m_userList.begin();
    int num = 0;
    uint8 pNum = 0;
    uint8 pos = msg.GetDataLen();
    msg<<pNum;
    for(; iter != m_userList.end(); iter++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(*iter);
        if(num > begin + ONE_PAGE_MAX_NUM)
            return;
        else if((num >= begin) && (p.get() != NULL))
        {
            /*
            +----+------+------+-----+-----+------+-------+-------+
            | OP | PAGE | PNUM | MID | LEN | NAME | XIANG | LEVEL |
            +----+------+------+-----+-----+------+-------+-------+
            |  1 |  1   |  1   |  4  |  2  |  Var |   1   |   1   |
            +----+------+------+-----+-----+------+-------+-------+
            */
            pNum++;
            msg<<p->GetRoleId()<<p->GetName()<<p->GetXiang()<<p->GetLevel();
        }
        num++;
    }
    msg.WriteData(pos,&pNum,1);
}

//不给同组队人发
void CScene::BroadcastMsgExceptSameTeam(CUser *pUser,CNetMessage &msg)
{
    list<uint32>::iterator iter = m_userList.begin();
    for(; iter != m_userList.end(); iter++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(*iter);
        if(p.get() == NULL) 
            continue;
        if((pUser->GetRoleId() != p->GetRoleId()
            && ((pUser->GetTeam() == 0) || (p->GetTeam() != pUser->GetTeam()))))
        {
            m_socketServer.SendMsg(p->GetSock(),msg);
        }
    }
}

void CScene::BroadcastMsg(CNetMessage &msg,bool chatMsg)
{
    list<uint32>::iterator iter = m_userList.begin();
    
    for(; iter != m_userList.end(); iter++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(*iter);
        if(p.get() != NULL)
        {
            if((!chatMsg) || ((p->GetChatChannel() & 2) != 0))
                m_socketServer.SendMsg(p->GetSock(),msg);
        }
    }
}

void CScene::BroadcastMsgExcept(CNetMessage &msg,CUser *pUser)
{
    list<uint32>::iterator iter = m_userList.begin();
    
    for(; iter != m_userList.end(); iter++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(*iter);
        if(p.get() == NULL) 
            continue;
        if(p->GetRoleId() == pUser->GetRoleId())
            continue;
        if(!p->UserInfoIsOpen() 
            && ((p->GetTeam() == 0) || (pUser->GetTeam() != p->GetTeam())))
            continue;
        m_socketServer.SendMsg(p->GetSock(),msg);
    }
}

void CScene::AllowJoinTeam(CUser *pUser,uint32 member)
{    
    ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(member);
    CNetMessage msg;
    //msg.SetType(PRO_USER_TEAM);
        
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    CUserTeam *pTeam = NULL;
    m_userTeams.Find(pUser->GetTeam(),pTeam);
    
    if((ptr.get() == NULL)
        || (ptr->GetTeam() != 0)
        || (pTeam == NULL)
        || (ptr->GetAskForJoinTeam() != pUser->GetTeam())
        || (pUser->GetScene() != ptr->GetScene())
        || (ptr->GetFightId() != 0)
        || (pUser->GetFightId() != 0))
    {
        return;
    }
    if(ptr->HaveBitSet(156))
    {
        SendSysInfo(ptr.get(),"任务中不能组队");
        return;
    }
    if(!pTeam->Join(member))
    {
        SendPopMsg(ptr.get(),"队伍已满无法加入");
        return;
    }
    ptr->SetTeam(pUser->GetTeam());
    /*if(pTeam->GetMemberNum() >= MAX_TEAM_MEMBER)
    {
        pTeam->DelAllAskForJoinTeam();
    }*/
    pTeam->DelAskForJoinTeam(member);
    
    //msg<<(uint8)4<<PRO_SUCCESS;
    //m_socketServer.SendMsg(pUser->GetSock(),msg);
    
    uint32 members[MAX_TEAM_MEMBER];
    uint8 num = 0;
    pTeam->GetMember(members,num);
    
    string str = ptr->GetName();
    str += "加入队伍";
    for(uint8 i = 0; i < num; i++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
        if(p.get() != NULL)
        {
            SendSysInfo(p.get(),str.c_str());
        }
    }
    
    msg.ReWrite();
    msg.SetType(PRO_UPDATE_TEAM);
    
    uint8 x,y;
    pUser->GetPos(x,y);
    ptr->SetPos(x,y);
    
    msg<<(uint8)2<<pUser->GetRoleId()<<member<<x<<y<<pUser->GetFace();
    
    BroadcastMsg(msg);
}

void CScene::GetAskForUserList(CUser *pUser)
{
    //page从1开始
    CNetMessage msg;
    msg.SetType(PRO_USER_TEAM);
    msg<<(uint8)5;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    CUserTeam *pTeam = NULL;
    if((!m_userTeams.Find(pUser->GetTeam(),pTeam))
        || (pUser->GetTeam() != pTeam->GetHeadId()))
    {
        msg<<(uint8)0;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    uint8 askNum = 0;
    uint8 pos = msg.GetDataLen();
    msg<<askNum;
    
    list<uint32> *userList = pTeam->GetAskForJoin();
    list<uint32>::iterator iter = userList->begin();
    int num = 0;
    for(; iter != userList->end(); iter++)
    {
        num++;
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(*iter);
        if(p.get() != NULL)
        {
            askNum++;
            msg<<p->GetRoleId()<<p->GetName()<<p->GetLevel();
        }
        if(askNum >= ONE_PAGE_MAX_NUM)
            break;
    }
    msg.WriteData(pos,&askNum,sizeof(askNum));
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

uint8 CScene::GetTeamMem(uint32 teamId,uint32 members[MAX_TEAM_MEMBER])
{
    CUserTeam *pTeam = NULL;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_userTeams.Find(teamId,pTeam))
    {
        uint8 num = 0; 
        pTeam->GetMember(members,num);
        return num;
    } 
    return 0;   
}

void CScene::SetNewHead(CUser *pUser,uint32 newHead)
{
    uint32 teamId = pUser->GetTeam();
    if(teamId != pUser->GetRoleId())
        return;
        
    ShareUserPtr p = m_onlineUser.GetUserByRoleId(newHead);
    CUser *pNewHead = p.get();
    if(pNewHead == NULL)
    {
        return;
    }
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CUserTeam *pTeam = NULL;
    if(!m_userTeams.Find(teamId,pTeam))
    {
        return;
    }
    if(!pTeam->SetNewHead(pNewHead))
    {
        return;
    }
    uint32 leaveMem[MAX_TEAM_MEMBER];
    uint8 leaveNum = 0;
    pTeam->GetLeaveMem(leaveMem,leaveNum);
    for(uint8 i = 0; i < leaveNum; i++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(leaveMem[i]);
        if(p.get() != NULL)
        {
            p->SetTempLeaveTeam(pNewHead->GetRoleId());
        }
    }
    uint32 members[MAX_TEAM_MEMBER];
    uint8 memNum = 0;
    pTeam->GetMember(members,memNum);
    for(uint8 i = 0; i < memNum; i++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
        if(p.get() != NULL)
        {
            p->SetTeam(pNewHead->GetRoleId());
        }
    }
    
    pUser->SetTeam(pNewHead->GetRoleId());
    pNewHead->SetTeam(pNewHead->GetRoleId());
    m_userTeams.Erase(teamId);
    m_userTeams.Insert(pNewHead->GetRoleId(),pTeam);
    
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_TEAM);
    msg<<(uint8)5<<pNewHead->GetRoleId()
        <<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
    BroadcastMsg(msg);
}

void CScene::GetTeamMembers(CUser *pUser)
{
    CNetMessage msg;
    msg.SetType(PRO_USER_TEAM);
    msg<<(uint8)8;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CUserTeam *pTeam = NULL;
    uint32 teamId = pUser->GetTeam();
    if(teamId == 0)
        teamId = pUser->TempLeaveTeam();
        
    if((teamId == 0)
        || (!m_userTeams.Find(teamId,pTeam)))
    {
        msg<<(uint8)0;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return;
    }
    
    uint32 members[MAX_TEAM_MEMBER];
    uint32 leaveMem[MAX_TEAM_MEMBER];
    uint8 num = 0;
    uint8 leaveNum = 0;
    pTeam->GetMember(members,num);
    pTeam->GetLeaveMem(leaveMem,leaveNum);
    msg<<(uint8)(num+leaveNum);
    for(uint8 i = 0; i < num; i++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
        if(p.get() != NULL)
        {//1暂离,0在队伍中
            msg<<p->GetRoleId()<<p->GetName()<<p->GetXiang()<<p->GetLevel()<<(uint8)0;
        }
    }
    for(uint8 i = 0; i < leaveNum; i++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(leaveMem[i]);
        if(p.get() != NULL)
        {//0暂离,1在队伍中
            msg<<p->GetRoleId()<<p->GetName()<<p->GetXiang()<<p->GetLevel()<<(uint8)1;
        }
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CScene::TempLeaveTeam(CUser *pUser)
{
    uint32 teamId = pUser->GetTeam();
    if(teamId == 0)
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CUserTeam *pTeam = NULL;
    if(!m_userTeams.Find(pUser->GetTeam(),pTeam))
    {
        return;
    }
    
    if(pUser->GetTeam() == pUser->GetRoleId())
    {
        //队长不得暂离队伍，只有在战斗中队长逃跑才会调用此函数
        //这时候解散队伍
        CNetMessage msg;
        msg.SetType(PRO_UPDATE_TEAM);
        msg<<(uint8)4<<teamId<<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
        BroadcastMsg(msg);
        uint32 members[MAX_TEAM_MEMBER];
        uint8 num = 0;
        pTeam->GetMember(members,num);
        for(uint8 i = 0; i < num; i++)
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
            if(p.get() != NULL)
            {
                SendSysInfo(p.get(),"队伍已解散");
                p->SetTeam(0);
                p->SetTempLeaveTeam(0);
            }
        }
        uint32 leaveMem[MAX_TEAM_MEMBER];
        uint8 leaveNum = 0;
        pTeam->GetLeaveMem(leaveMem,leaveNum);
        for(uint8 i = 0; i < leaveNum; i++)
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(leaveMem[i]);
            if(p.get() != NULL)
            {
                SendSysInfo(p.get(),"队伍已解散");
                p->SetTeam(0);
                p->SetTempLeaveTeam(0);
                m_socketServer.SendMsg(p->GetSock(),msg);
            }
        }
        m_userTeams.Erase(teamId);
        delete pTeam;
        /*uint32 members[MAX_TEAM_MEMBER];
        uint8 num = 0;
        pTeam->GetMember(members,num);
        for(uint8 i = 1; i < num; i++)
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
            pUser = p.get();
            if((pUser != NULL) && (pUser->TempLeaveTeam() == 0))
            {
                pUser->SetTempLeaveTeam(teamId);
                pUser->SetTeam(0);
                pTeam->TempLeaveTeam(pUser->GetRoleId());
                CNetMessage msg;
                msg.SetType(PRO_UPDATE_TEAM);
                msg<<(uint8)6<<pTeam->GetHeadId()<<pUser->GetRoleId()
                    <<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
                BroadcastMsg(msg);
            }
        }*/
    }
    else
    {
        pUser->SetTempLeaveTeam(pUser->GetTeam());
        pUser->SetTeam(0);
        pTeam->TempLeaveTeam(pUser->GetRoleId());
        CNetMessage msg;
        msg.SetType(PRO_UPDATE_TEAM);
        msg<<(uint8)6<<pTeam->GetHeadId()<<pUser->GetRoleId()
            <<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
        BroadcastMsg(msg);
    }
}

void CScene::ReturnTeam(CUser *pUser)
{
    ShareUserPtr p = m_onlineUser.GetUserByRoleId(pUser->TempLeaveTeam());
    CUser *pTeamHead = p.get();
    if(pTeamHead == NULL)
        return;
        
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CUserTeam *pTeam = NULL;
    if(!m_userTeams.Find(pUser->TempLeaveTeam(),pTeam))
    {
        return;
    }
    if((abs(pTeamHead->GetX()-pUser->GetX()) > 5) 
        || (abs(pTeamHead->GetY() - pUser->GetY()) > 5))
    {
        SendSysInfo(pUser,"队伍不在附近，无法归队");
        return;
    }
    pUser->SetTeam(pUser->TempLeaveTeam());
    pUser->SetTempLeaveTeam(0);
    pUser->SetPos(pTeamHead->GetX(),pTeamHead->GetY());
    
    pTeam->ReturnTeam(pUser->GetRoleId());
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_TEAM);
    msg<<(uint8)7<<pTeam->GetHeadId()<<pUser->GetRoleId()
        <<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
    BroadcastMsg(msg);
}

bool CScene::LeaveSceneTeam(uint32 teamId,CUser *pUser)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CUserTeam *pTeam = NULL;
    if(!m_userTeams.Find(teamId,pTeam))
    {
        return false;
    }
    pUser->SetTeam(0);
    pUser->SetTempLeaveTeam(0);
    
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_TEAM);
    
    if(teamId == pUser->GetRoleId())
    {//队长离开队伍
        uint8 num = pTeam->GetMemberNum();// + pTeam->GetLeaveNum();
        if(num == 1)
        {
            msg<<(uint8)4<<teamId<<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
            BroadcastMsg(msg);
            uint32 members[MAX_TEAM_MEMBER];
            uint8 num = 0;
            pTeam->GetMember(members,num);
            for(uint8 i = 0; i < num; i++)
            {
                ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
                if(p.get() != NULL)
                {
                    SendSysInfo(p.get(),"队伍已解散");
                    p->SetTeam(0);
                    p->SetTempLeaveTeam(0);
                }
            }
            uint32 leaveMem[MAX_TEAM_MEMBER];
            uint8 leaveNum = 0;
            pTeam->GetLeaveMem(leaveMem,leaveNum);
            for(uint8 i = 0; i < leaveNum; i++)
            {
                ShareUserPtr p = m_onlineUser.GetUserByRoleId(leaveMem[i]);
                if(p.get() != NULL)
                {
                    SendSysInfo(p.get(),"队伍已解散");
                    p->SetTeam(0);
                    p->SetTempLeaveTeam(0);
                    m_socketServer.SendMsg(p->GetSock(),msg);
                }
            }
            m_userTeams.Erase(teamId);
            delete pTeam;
        }
        else
        {
            uint32 members[MAX_TEAM_MEMBER];
            uint8 num = 0;
            pTeam->GetMember(members,num);
            string str = pUser->GetName();
            str += "离开队伍";
            
            CUser *pNewHead = NULL;
            for(uint8 i = 0; i < num; i++)
            {
                ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
                if(p.get() != NULL)
                {
                    SendSysInfo(p.get(),str.c_str());
                    if((pNewHead == NULL) && (p->GetRoleId() != pUser->GetRoleId()))
                    {
                        pNewHead = p.get();
                    }
                }
            }
            if(pNewHead == NULL)
                return true;
            pTeam->ReturnTeam(pNewHead->GetRoleId());
            pNewHead->SetTempLeaveTeam(0);
            pNewHead->SetTeam(pNewHead->GetRoleId());
            m_userTeams.Erase(teamId);
            m_userTeams.Insert(pNewHead->GetRoleId(),pTeam);
            pTeam->SetNewHead(pNewHead);
            pTeam->Exit(pUser->GetRoleId());
            
            uint32 leaveMem[MAX_TEAM_MEMBER];
            uint8 leaveNum = 0;
            pTeam->GetLeaveMem(leaveMem,leaveNum);
            for(uint8 i = 0; i < leaveNum; i++)
            {
                ShareUserPtr p = m_onlineUser.GetUserByRoleId(leaveMem[i]);
                if(p.get() != NULL)
                {
                    p->SetTempLeaveTeam(pNewHead->GetRoleId());
                }
            }
            for(uint8 i = 0; i < num; i++)
            {
                ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
                if((p.get() != NULL) && (p.get() != pUser))
                {
                    p->SetTeam(pNewHead->GetRoleId());
                }
            }
            msg<<(uint8)3<<pTeam->GetHeadId()<<pUser->GetRoleId()
                <<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
            BroadcastMsg(msg);
        }
    }
    else
    {//队员离开队伍
        uint32 members[MAX_TEAM_MEMBER];
        uint8 num = 0;
        pTeam->GetMember(members,num);
        string str = pUser->GetName();
        str += "离开队伍";
        for(uint8 i = 0; i < num; i++)
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
            if(p.get() != NULL)
            {
                SendSysInfo(p.get(),str.c_str());
            }
        }
        pTeam->Exit(pUser->GetRoleId());
        msg<<(uint8)3<<pTeam->GetHeadId()<<pUser->GetRoleId()
            <<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
        BroadcastMsg(msg);
    }
    return true;
}

void CScene::LeaveTeam(CUser *pUser)
{
    uint32 teamId = pUser->GetTeam();
    if(teamId == 0)
        teamId = pUser->TempLeaveTeam();
    
    if(teamId == 0)
    {
        return;
    }
    
    if(!LeaveSceneTeam(teamId,pUser))
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(teamId);
        if(p.get() != NULL)
        {
            CScene *pScene = p->GetScene();
            if(pScene != this)
            {
                pScene->LeaveTeam(pUser);
                CNetMessage msg;
                msg.SetType(PRO_UPDATE_TEAM);
                msg<<(uint8)3<<teamId<<pUser->GetRoleId()
                    <<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
                m_socketServer.SendMsg(pUser->GetSock(),msg);
            }
        }
    }
}

void CScene::NotAllowJoin(CUser *pUser,uint32 member)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CUserTeam *pTeam = NULL;
    if((!m_userTeams.Find(pUser->GetTeam(),pTeam))
        || (pTeam->GetHeadId() != pUser->GetRoleId()))
    {
        return;
    }
    pTeam->DelAskForJoinTeam(member);
    
    ShareUserPtr p = m_onlineUser.GetUserByRoleId(member);
    if(p.get() == NULL)
    {
        return;
    }
    string str = pUser->GetName();
    str += "拒绝你加入队伍";
    SendSysInfo(p.get(),str.c_str());
}

void CScene::DelTeamMember(CUser *pUser,uint32 member)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CUserTeam *pTeam = NULL;
    if((!m_userTeams.Find(pUser->GetTeam(),pTeam))
        || (member == pTeam->GetHeadId())
        || (pTeam->GetHeadId() != pUser->GetRoleId()))
    {
        return;
    }
    
    
    ShareUserPtr p = m_onlineUser.GetUserByRoleId(member);
    if(p.get() == NULL)
    {
        return;
    }
    SendSysInfo(p.get(),"您已经被请出队伍");
    
    p->SetTeam(0);
    p->SetTempLeaveTeam(0);
    pTeam->Exit(member);
    
    string str = p->GetName();
    str += "被逐出队伍";
    
    uint32 members[MAX_TEAM_MEMBER];
    uint8 num = 0;
    pTeam->GetMember(members,num);
    for(uint8 i = 0; i < num; i++)
    {
        p = m_onlineUser.GetUserByRoleId(members[i]);
        if(p.get() != NULL)
        {
            SendSysInfo(p.get(),str.c_str());
        }
    }
    CNetMessage msg;
    msg.SetType(PRO_UPDATE_TEAM);
    msg<<(uint8)3<<pTeam->GetHeadId()<<member<<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
    BroadcastMsg(msg);
}

/*static void AddUserToFight(ShareUserPtr pUser,CFight *pFight,uint8 *pos)
{
    pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,*pos));
    *pos += 2;
}*/

int CScene::GetUserNum()
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    return m_userList.size();
}

bool CScene::FindFacePlayer(CUser *pUser,ShareUserPtr &find)
{
    uint8 x,y;
    pUser->GetFacePos(x,y);
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    for(list<uint32>::iterator i = m_userList.begin(); i != m_userList.end(); i++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(*i);
        if(p.get() != NULL)
        {
            if((p->GetX() == x) && (p->GetY() == y))
            {
                find = p;
                return true;
            }
        }
    }
    return false;
}

bool CScene::InGuiYu()
{
    return ((m_mapId >= GUIYU_SCENE_ID) && (m_mapId < GUIYU_SCENE_ID + MAX_GUI_YU_LAYER));
}

bool CScene::InMatchScene()
{
    return ((m_mapId >= MATCH_SCENE_ID) && (m_mapId < MATCH_SCENE_ID + MATCH_SCENE_NUM));
}

extern int WWGetWinBang();
extern bool InWorldWar();

void CScene::PlayerPk(ShareUserPtr pUser,uint32 roleId,bool yaoqing)
{
    bool haveLimit = true;
    if(((m_mapId >= GUIYU_SCENE_ID) && (m_mapId <= GUIYU_SCENE_ID + MAX_GUI_YU_LAYER))
        || ((m_mapId >= 262) && (m_mapId <= 266)))
    {
        haveLimit = false;
    }
    
    if(((m_mapId == 300) || (m_mapId == 305)) && ((m_id>>16) == (int)pUser->GetBangPai()))
    {
        haveLimit = false;
    }
    
    ShareUserPtr p;
    if(roleId == 0)
    {
        if(!FindFacePlayer(pUser.get(),p))
        {
            SendSysInfo(pUser.get(),"相邻位置无玩家");
            return;
        }
    }
    else
    {
        p = m_onlineUser.GetUserByRoleId(roleId);
    }
    
    if(p.get() == NULL)
    {
        SendSysInfo(pUser.get(),"对方已下线");
        return;
    }
    
    if(pUser->GetScene() != p->GetScene())
    {
        SendSysInfo(pUser.get(),"不在同一场景不能PK");
        return;
    }
    
    if(p->GetFightId() != 0)
    {
        SendSysInfo(pUser.get(),"对方在战斗中，不能PK");
        return;
    }
    /*
    if(p->HaveItem(1558) || 
            p->HaveItem(1559) || 
            p->HaveItem(1560) || 
            p->HaveItem(1561) || 
            p->HaveItem(1562))*/
    int worldWarId = 0;
    if(InWorldWar())    
    {
        /*if((WWGetWinBang() != 0)
            && (WWGetWinBang() == (int)pUser->GetBangPai())
            && ((int)p->GetBangPai() != WWGetWinBang()))*/
        //if((m_fightType & 1) == 0)
        
        if(!IsFangShou(p->GetBangPai()) && IsFangShou(pUser->GetBangPai()))
        {
            if((m_mapId < 270) || (m_mapId > 276))
            {
                SendSysInfo(pUser.get(),"本场景不允许PK");
                return;
            }
            haveLimit = false;
            worldWarId = 0xffff;
        }
        else if(haveLimit)
        {
            return;
        }
        
        if(pUser->GetPkMiss(roleId) != 0)
        {
            SendSysInfo(pUser.get(),"仙境之战期间无法通缉");
            return;
        }
    }
    else if(pUser->GetPkMiss(roleId) != 0)
    {//通缉任务
        if(pUser->GetTeam() != 0)
        {
            SendSysInfo(pUser.get(),"组队状态下不能PK");
            return;
        }
        if(m_mapId <= 3)
        {
            SendSysInfo(pUser.get(),"此场景不能PK");
            return;
        }
        if(p->GetTeam() != 0)
            TempLeaveTeam(p.get());
        uint8 pkTimes = p->GetData8(9);
        if(pkTimes >= 5)
        {
            SendSysInfo(pUser.get(),"对方今天已经被杀超过5次，无法再次PK");
            return;
        }
        /*if(!p->CanPk())
        {
            SendSysInfo(pUser.get(),"5分钟内禁止PK");
            return;
        }
        p->SetPkTime(GetSysTime());*/
        haveLimit = false;
    }
    else
    {
        if(haveLimit)
        {
            if(pUser->GetLevel() <= 30)
            {
                SendSysInfo(pUser.get(),"三十级以下玩家不能PK");
                return;
            }
        }
        
        if((m_fightType & 1) == 0)
        {
            SendSysInfo(pUser.get(),"本场景不允许PK");
            return;
        }
        if(haveLimit)
        {
            if(p->GetLevel() <= 30)
            {
                SendSysInfo(pUser.get(),"三十级以下玩家不能PK");
                return;
            }
            if((p->GetTeam() != 0) && (p->GetTeam() != p->GetRoleId()))
                return;
        }
    }
    
    if(yaoqing && haveLimit)
    {
        if(p->HaveIgnore(pUser->GetRoleId()))
            return;
        CNetMessage msg;
        msg.SetType(PRO_USER_PK);
        msg<<pUser->GetRoleId()<<pUser->GetName();
        m_socketServer.SendMsg(p->GetSock(),msg);
        return;
    }
    
    ShareFightPtr pFight = m_fightManager.CreateFight();
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        if((pUser->GetTeam() != 0) && (p->GetTeam() == pUser->GetTeam()))
            return;
            
        pFight->SetFightType(CFight::EFTPlayerPk);
        pFight->SetVisibleMonsterId(worldWarId);
        
        CUserTeam *pTeam = NULL;
        if((pUser->GetTeam() != 0) && (m_userTeams.Find(pUser->GetTeam(),pTeam)))
        {
            if(yaoqing)
                AddTeamToFight(pFight,pTeam,1);
            else
                AddTeamToFight(pFight,pTeam,7);
        }
        else
        {
            if(yaoqing)
                pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,3));
            else
                pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        }
        
        if((p->GetTeam() != 0) && (m_userTeams.Find(p->GetTeam(),pTeam)))
        {
            if(yaoqing)
                AddTeamToFight(pFight,pTeam,7);
            else
                AddTeamToFight(pFight,pTeam,1);
        }
        else
        {
            if(yaoqing)
                p->SetFight(pFight->GetId(),pFight->AddUser(p,9));
            else
                p->SetFight(pFight->GetId(),pFight->AddUser(p,3));
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

ShareMonsterPtr CScene::CreateMonster(CUser *pUser,bool &baobao)
{
    EMonsterType type = EMTNormal;
    
    CMonsterManager &monsterMgr = SingletonMonsterManager::instance();
    SMonsterTmpl *pMonster = monsterMgr.GetTmpl(m_monsters[0]);
    if(!baobao)
    {
        int baobaoGaiLv = 0;
        if(pMonster != NULL)
            baobaoGaiLv = (100-pMonster->maxLevel)/10;
        if(baobaoGaiLv < 1)
            baobaoGaiLv = 1;
        time_t t = GetSysTime();
        struct tm *pTm = localtime(&t);
        if((pTm != NULL) && (pTm->tm_hour == CHATCH_PET_HUO_DONG))
            baobaoGaiLv = 10;
            
        if(Random(1,100) <= baobaoGaiLv)
        {//(100 - level)/10,最低1％
            type = EMTBaoBao;
        }
        baobao = true;
    }
    else if(Random(1,100) <= 1)
    {
        type = EMTTongLing;
    }
    //todo
    //type = EMTTongLing;
    ShareMonsterPtr pShareMonster;
    if(pUser->IsMinMonster())
        pShareMonster = m_monsterManager.CreateMonster(RandSelect(m_monsters,m_monsterNum),type,false,true);
    else
        pShareMonster = m_monsterManager.CreateMonster(RandSelect(m_monsters,m_monsterNum),type);
/*    
#ifdef DEBUG
    cout<<"monster info++++++++++"<<endl;
    cout<<"monster level:"<<(int)pShareMonster->level<<endl;
    cout<<"monster Hp:"<<pShareMonster->hp<<endl;
    cout<<"++++++++++++++++++++++"<<endl;
#endif
*/
    return pShareMonster;
}

void CScene::AddMonsterToFight(ShareFightPtr &pFight,uint8 num,uint8 begin,CUser *pUser)
{
    bool baobao = false;
    switch(num)
    {
    case 1:
        {
            ShareMonsterPtr pShareMonster = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster,begin+3);
        }
        break;
    case 2:
        {
            ShareMonsterPtr pShareMonster = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster,begin+3);
            ShareMonsterPtr pShareMonster1 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster1,begin+5);
        }
        break;
    case 3:
        {
            ShareMonsterPtr pShareMonster = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster,begin+1);
            ShareMonsterPtr pShareMonster1 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster1,begin+3);
            ShareMonsterPtr pShareMonster2 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster2,begin+5);
        }
        break;
    case 4:
        {
            ShareMonsterPtr pShareMonster = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster,begin);
            ShareMonsterPtr pShareMonster1 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster1,begin+2);
            ShareMonsterPtr pShareMonster2 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster2,begin+4);
            ShareMonsterPtr pShareMonster3 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster3,begin+3);
        }
        break;
    case 5:
        {
            ShareMonsterPtr pShareMonster = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster,begin);
            ShareMonsterPtr pShareMonster1 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster1,begin+2);
            ShareMonsterPtr pShareMonster2 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster2,begin+4);
            ShareMonsterPtr pShareMonster3 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster3,begin+1);
            ShareMonsterPtr pShareMonster4 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster4,begin+5);
        }
        break;
    case 6:
        {
            ShareMonsterPtr pShareMonster = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster,begin);
            ShareMonsterPtr pShareMonster1 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster1,begin+1);
            ShareMonsterPtr pShareMonster2 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster2,begin+2);
            ShareMonsterPtr pShareMonster3 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster3,begin+3);
            ShareMonsterPtr pShareMonster4 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster4,begin+4);
            ShareMonsterPtr pShareMonster5 = CreateMonster(pUser,baobao);
            pFight->AddMonster(pShareMonster5,begin+5);
        }
        break;
    }
}

uint8 CScene::AddTeamToFight(ShareFightPtr &pFight,CUserTeam *pTeam,uint8 begin)
{
    uint8 maxLevel = 1;
    uint32 members[MAX_TEAM_MEMBER];
    uint8 teamNum = 0;
    pTeam->GetMember(members,teamNum);
    switch(teamNum)
    {
    case 1:
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[0]);
            if((p.get() != NULL) && (p->GetTeam() == pTeam->GetHeadId()))
            {
                p->SetFight(pFight->GetId(),pFight->AddUser(p,begin + 2));
                maxLevel = p->GetLevel();
            }
        }
        break;
    case 2:
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[0]);
            if((p.get() != NULL) && (p->GetTeam() == pTeam->GetHeadId()))
            {
                p->SetFight(pFight->GetId(),pFight->AddUser(p,begin));
                if(p->GetLevel() > maxLevel)
                    maxLevel = p->GetLevel();
            }
            ShareUserPtr p1 = m_onlineUser.GetUserByRoleId(members[1]);
            if((p1.get() != NULL)&& (p1->GetTeam() == pTeam->GetHeadId()))
            {
                p1->SetFight(pFight->GetId(),pFight->AddUser(p1,begin+4));
                if(p1->GetLevel() > maxLevel)
                    maxLevel = p1->GetLevel();
            }
        }
        break;
    case 3:
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[0]);
            if((p.get() != NULL)&& (p->GetTeam() == pTeam->GetHeadId()))
            {
                p->SetFight(pFight->GetId(),pFight->AddUser(p,begin));
                if(p->GetLevel() > maxLevel)
                    maxLevel = p->GetLevel();
            }
            ShareUserPtr p1 = m_onlineUser.GetUserByRoleId(members[1]);
            if((p1.get() != NULL) && (p1->GetTeam() == pTeam->GetHeadId()))
            {
                p1->SetFight(pFight->GetId(),pFight->AddUser(p1,begin+2));
                if(p1->GetLevel() > maxLevel)
                    maxLevel = p1->GetLevel();
            }
            ShareUserPtr p2 = m_onlineUser.GetUserByRoleId(members[2]);
            if((p2.get() != NULL) && (p2->GetTeam() == pTeam->GetHeadId()))
            {
                p2->SetFight(pFight->GetId(),pFight->AddUser(p2,begin+4));
                if(p2->GetLevel() > maxLevel)
                    maxLevel = p2->GetLevel();
            }
        }
        break;
    }
    return maxLevel;
}

void GetChengZhang(uint8 level,int &xue,int &fali,int &sudu,int &wugong,int &fagong)
{
    const uint8 CHENG_ZHANG_VAL[] = {
        5,50,45,20,35,30,
        10,55,45,20,40,35,
        15,60,50,20,40,35,
        20,65,50,20,45,40,
        25,70,50,20,45,40,
        30,75,50,20,45,40,
        35,80,55,20,45,40,
        40,80,60,25,45,40,
        45,80,65,25,50,45,
        50,80,60,25,50,45,
        55,80,65,25,50,45,
        60,80,70,30,55,50,
        65,80,70,35,60,55,
        70,85,75,40,65,60,
        75,85,75,50,70,65,
        80,90,75,60,80,75,
        85,95,75,60,85,80,
        90,95,85,75,90,85,
        95,100,85,75,90,85,
        100,100,85,80,90,85,
        105,100,85,80,95,90,
        110,105,90,80,100,95,
        115,105,90,85,105,100,
        120,110,90,85,110,105,
        125,115,90,85,115,110
    };
    int num = sizeof(CHENG_ZHANG_VAL)/6;
    uint8 pos = level/5;
    if(pos >= num)
        return;
    xue = CHENG_ZHANG_VAL[6*pos+1];
    fali = CHENG_ZHANG_VAL[6*pos+2];
    sudu = CHENG_ZHANG_VAL[6*pos+3];
    wugong = CHENG_ZHANG_VAL[6*pos+4];
    fagong = CHENG_ZHANG_VAL[6*pos+5];
}

static ShareMonsterPtr CreateScriptMonster(CMonsterManager &manager,uint32 tmplId,uint8 level,bool isTouMu,uint8 fightType,
                        string touMuName = "")
{
    ShareMonsterPtr ptr;
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = tmplId;
    pInst->xiang = Random(1,5);
    
    SMonsterTmpl *pTmpl = manager.GetTmpl(tmplId);
    if(pTmpl == NULL)
    {
        return ptr;
    }
    
    pInst->level = level;
    pInst->exp = 0;
    if(fightType == 1)
    {
        if(isTouMu)
        {
            pInst->liliang = 4*pInst->level;
            pInst->lingxing = 4*pInst->level;
            if(touMuName.size() <= 0)
                pInst->name = "妖魔";
            else
                pInst->name = touMuName;
        }
        else
        {
            pInst->liliang = 2*pInst->level;
            pInst->lingxing = 2*pInst->level;
            pInst->name = "喽罗";
        }
        pInst->tizhi = pInst->level;
        pInst->naili = pInst->level;
        pInst->minjie = pInst->level;
    }
    else
    {
        if(isTouMu)
        {
            pInst->liliang = 5*pInst->level;
            pInst->lingxing = 5*pInst->level;
            if(touMuName.size() <= 0)
                pInst->name = "妖魔";
            else
                pInst->name = touMuName;
        }
        else
        {
            pInst->liliang = 4*pInst->level;
            pInst->lingxing = 4*pInst->level;
            pInst->name = "喽罗";
        }
        pInst->tizhi = pInst->level;
        pInst->naili = pInst->level;
        pInst->minjie = pInst->level;
    }
    
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(level,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
        
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;
    
    return ptr;
}

void CScene::AddScriptFight(ShareFightPtr &pFight,uint8 num,uint8 begin,uint8 level,
                        uint8 toMuBeiLv,uint8 louLuoBeiLv,int touMuId,int louLuoId,
                        string touMuName,uint8 fightType)
{
    ShareMonsterPtr pShareMonster = CreateScriptMonster(m_monsterManager,touMuId,level,true,fightType,touMuName);
    pShareMonster->maxHp *= toMuBeiLv;
    pShareMonster->hp = pShareMonster->maxHp;
    
    pFight->AddMonster(pShareMonster,begin+3);
    if(num == 1)
        return;
    
    if(level < 5)
        level = 1;
    else
        level -= 5;
        
    switch(num)
    {
    case 2:
        {
            ShareMonsterPtr pShareMonster1 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pShareMonster1->maxHp *= louLuoBeiLv;
            pShareMonster1->hp = pShareMonster1->maxHp;
            pFight->AddMonster(pShareMonster1,begin+5);
        }
        break;
    case 3:
        {
            ShareMonsterPtr pShareMonster1 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster1,begin+1);
            ShareMonsterPtr pShareMonster2 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster2,begin+5);
            pShareMonster1->maxHp *= louLuoBeiLv;
            pShareMonster1->hp = pShareMonster1->maxHp;
            pShareMonster2->maxHp *= louLuoBeiLv;
            pShareMonster2->hp = pShareMonster2->maxHp;
            
        }
        break;
    case 4:
        {
            ShareMonsterPtr pShareMonster1 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster1,begin+2);
            ShareMonsterPtr pShareMonster2 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster2,begin+4);
            ShareMonsterPtr pShareMonster3 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster3,begin);
            pShareMonster1->maxHp *= louLuoBeiLv;
            pShareMonster1->hp = pShareMonster1->maxHp;
            pShareMonster2->maxHp *= louLuoBeiLv;
            pShareMonster2->hp = pShareMonster2->maxHp;
            pShareMonster3->maxHp *= louLuoBeiLv;
            pShareMonster3->hp = pShareMonster3->maxHp;
        }
        break;
    case 5:
        {
            ShareMonsterPtr pShareMonster1 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster1,begin+2);
            ShareMonsterPtr pShareMonster2 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster2,begin+4);
            ShareMonsterPtr pShareMonster3 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster3,begin+1);
            ShareMonsterPtr pShareMonster4 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster4,begin+5);
            pShareMonster1->maxHp *= louLuoBeiLv;
            pShareMonster1->hp = pShareMonster1->maxHp;
            pShareMonster2->maxHp *= louLuoBeiLv;
            pShareMonster2->hp = pShareMonster2->maxHp;
            pShareMonster3->maxHp *= louLuoBeiLv;
            pShareMonster3->hp = pShareMonster3->maxHp;
            pShareMonster4->maxHp *= louLuoBeiLv;
            pShareMonster4->hp = pShareMonster4->maxHp;

        }
        break;
    case 6:
        {
            ShareMonsterPtr pShareMonster1 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster1,begin+1);
            ShareMonsterPtr pShareMonster2 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster2,begin+2);
            ShareMonsterPtr pShareMonster3 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster3,begin);
            ShareMonsterPtr pShareMonster4 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster4,begin+4);
            ShareMonsterPtr pShareMonster5 = CreateScriptMonster(m_monsterManager,louLuoId,level,false,fightType);
            pFight->AddMonster(pShareMonster5,begin+5);
            pShareMonster1->maxHp *= louLuoBeiLv;
            pShareMonster1->hp = pShareMonster1->maxHp;
            pShareMonster2->maxHp *= louLuoBeiLv;
            pShareMonster2->hp = pShareMonster2->maxHp;
            pShareMonster3->maxHp *= louLuoBeiLv;
            pShareMonster3->hp = pShareMonster3->maxHp;
            pShareMonster4->maxHp *= louLuoBeiLv;
            pShareMonster4->hp = pShareMonster4->maxHp;
            pShareMonster5->maxHp *= louLuoBeiLv;
            pShareMonster5->hp = pShareMonster5->maxHp;
        }
        break;
    }
}

void CScene::BaiYueFight(CUser *pU)
{
    if(pU == NULL)
        return;
    
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        ShareMonsterPtr pShareMonster = m_monsterManager.CreateMonster(14,30,EMTNormal,"拜月圣使");
        pShareMonster->maxHp *= 6;
        pShareMonster->hp = pShareMonster->maxHp;
        
        pFight->AddMonster(pShareMonster,4);
        
        pShareMonster = m_monsterManager.CreateMonster(14,30,EMTNormal,"拜月圣使");
        pFight->AddMonster(pShareMonster,6);
        
        pShareMonster = m_monsterManager.CreateMonster(14,30,EMTNormal,"拜月圣使");
        pFight->AddMonster(pShareMonster,2);
                
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShuiGuiFight(CUser *pU)
{
    if(pU == NULL)
        return;
    
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        ShareMonsterPtr pShareMonster = m_monsterManager.CreateMonster(22,30,EMTNormal,"水鬼喽啰");
        pShareMonster->maxHp *= 6;
        pShareMonster->hp = pShareMonster->maxHp;
        
        pFight->AddMonster(pShareMonster,4);
                
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::DengLongGuiFight(CUser *pU)
{
    if(pU == NULL)
        return;
    
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        ShareMonsterPtr pShareMonster = m_monsterManager.CreateMonster(7,30,EMTTongLing);
        pShareMonster->maxHp *= 3;
        pShareMonster->hp = pShareMonster->maxHp;
        
        pFight->AddMonster(pShareMonster,4);
        
        pShareMonster = m_monsterManager.CreateMonster(7,EMTTongLing);
        pFight->AddMonster(pShareMonster,6);
        
        pShareMonster = m_monsterManager.CreateMonster(7,EMTTongLing);
        pFight->AddMonster(pShareMonster,2);
                
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}


static ShareMonsterPtr CreateKuLou(uint8 level)
{
    SMonsterInst *pInst = new SMonsterInst;
    ShareMonsterPtr ptr(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 39;
    pInst->name = "骷髅";
    pInst->level = level;
    pInst->exp = 0;
    pInst->liliang = 4*pInst->level;
    pInst->lingxing = 4*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    pInst->xiang = 0;
    
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(level,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
        
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
        
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;

    pInst->maxHp *= 5;
    
    pInst->hp = pInst->maxHp;
    pInst->pMonster = NULL;
    return ptr;
}

void CScene::ThreeKuLou(CUser *pU)
{
    if(pU == NULL)
        return;
    
    if(pU->GetFightId() != 0)
        return;
        
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        for(uint8 i = 1; i <= 6; i+=2)
        {
            ShareMonsterPtr ptr = CreateKuLou(pUser->GetLevel());
            pFight->AddMonster(ptr,i);
        }
    
        CUserTeam *pTeam = NULL;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {
            AddTeamToFight(pFight,pTeam,7);            
        }
        else
        {
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

//脚本触发，召讨使任务
void CScene::ZhaoTao1(CUser *pU)
{
    if(pU == NULL)
        return;
    
    if(pU->GetFightId() != 0)
        return;
        
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        uint8 num = 0;
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            if(pUser->IsMaxMonster())
                num = 2;
            else
                num = 1;//Random(1,2);
            AddScriptFight(pFight,num,1,pU->GetLevel(),8,6,9,39,"贼党");
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            if(pUser->IsMaxMonster())
                num = 2*pTeam->GetMemberNum();
            else
                num = pTeam->GetMemberNum();//Random(pTeam->GetMemberNum(),2*pTeam->GetMemberNum());
            AddScriptFight(pFight,num,1,pU->GetLevel(),8,6,9,39,"贼党");
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ZhaoTao2(CUser *pU)
{
    if(pU == NULL)
        return;
    if(pU->GetFightId() != 0)
        return;
        
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        CUserTeam *pTeam = NULL;
        uint8 num = 0;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            if(pUser->IsMaxMonster())
                num = 2;
            else
                num = Random(1,2);
            AddScriptFight(pFight,num,1,pU->GetLevel(),10,8,29,39,"邪灵");
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            if(pUser->IsMaxMonster())
                num = 2*pTeam->GetMemberNum();
            else
                num = Random(pTeam->GetMemberNum(),2*pTeam->GetMemberNum());
            //uint8 num = 0;
            //num = Random(pTeam->GetMemberNum(),2*pTeam->GetMemberNum());
            AddScriptFight(pFight,num,1,pU->GetLevel(),10,8,29,39,"邪灵");
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    
    m_fightManager.AddFight(pFight);
}

void CScene::ZhaoTao3(CUser *pU)
{
    if(pU == NULL)
        return;
    
    if(pU->GetFightId() != 0)
        return;
        
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        uint8 num = 0;
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            if(pUser->IsMaxMonster())
                num = 2;
            else
                num = Random(1,2);
            AddScriptFight(pFight,num,1,pU->GetLevel(),15,12,41,39);
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            if(pUser->IsMaxMonster())
                num = 2*pTeam->GetMemberNum();
            else
                num = Random(pTeam->GetMemberNum(),2*pTeam->GetMemberNum());
            AddScriptFight(pFight,num,1,pU->GetLevel(),15,12,41,39);
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::HumanNpcFight(CUser *pU,int npcId)
{
    if(pU == NULL)
        return;
    
    uint32 roleId = pU->GetHumanNcpRoleId(npcId);
    if(roleId == 0)
        return;
        
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        ShareUserPtr ptr(new CUser);
        ptr->ReadData(roleId);    
        ptr->SetRoleId(roleId);
        ptr->SetPet(NULL);
        //ptr->SetAutoFight(1,0,0,0,0,0xffff);
        ptr->SetAutoFightTurn(0xffff);
        pFight->AddUser(ptr,4);
        
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LouLuoFight(CUser *pU)
{
    if(pU == NULL)
        return;
    
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        ShareMonsterPtr pShareMonster = m_monsterManager.CreateMonster(Random(1,50),15,EMTTongLing,"妖怪喽罗");
        pFight->AddMonster(pShareMonster,4);
        
        if(Random(0,1) == 0)
        {
            pShareMonster = m_monsterManager.CreateMonster(Random(1,50),15,EMTTongLing,"帮凶");
            pFight->AddMonster(pShareMonster,6);
        }
                
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::WuNianFight(CUser *pU)
{
    if(pU == NULL)
        return;
    
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        ShareMonsterPtr pShareMonster = m_monsterManager.CreateMonster(Random(1,50),20,EMTNormal,"无念");
        pShareMonster->maxHp *= 20;
        pShareMonster->hp = pShareMonster->maxHp;
        
        pFight->AddMonster(pShareMonster,4);
        
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::WuXinFight(CUser *pU)
{
    if(pU == NULL)
        return;
    
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        ShareMonsterPtr pShareMonster = m_monsterManager.CreateMonster(Random(1,50),25,EMTNormal,"无心");
        pShareMonster->maxHp *= 30;
        pShareMonster->hp = pShareMonster->maxHp;
        
        pFight->AddMonster(pShareMonster,4);
        
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

//脚本触发，遇固定敌人
void CScene::ShiMenFight(CUser *pU)
{
    if(pU == NULL)
        return;
    
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
                
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            uint8 num = Random(1,2);
            AddScriptFight(pFight,num,1,pUser->GetLevel(),8,6,46,39);
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            uint8 num = Random(pTeam->GetMemberNum(),2*pTeam->GetMemberNum());
            AddScriptFight(pFight,num,1,pUser->GetLevel(),8,6,46,39);
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    
    m_fightManager.AddFight(pFight);
}

void CScene::EnterFight(CUser *pU,int monsterId,int level)
{
    if(pU == NULL)
        return;
    
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        ShareMonsterPtr pShareMonster = m_monsterManager.CreateMonster(monsterId,level,EMTNormal);
        
        pFight->AddMonster(pShareMonster,3);
        
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    
    m_fightManager.AddFight(pFight);
}

void CScene::GetUserList(list<uint32> &userList)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    userList = m_userList;
}

ShareMonsterPtr CScene::CreateMeiYing(uint8 level)
{
    SMonsterInst *pInst = new SMonsterInst;
    ShareMonsterPtr ptr(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 39;
    pInst->name = "魅影战士";
    pInst->level = level;
    pInst->exp = 0;
    pInst->liliang = 6*pInst->level;
    pInst->lingxing = 6*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    pInst->SetCeLue(CE_MEI_YING);
    pInst->xiang = 0;
    pInst->AddSkill(57,60);
    
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(level,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
        
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
        
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;

    pInst->maxHp *= 10;
    
    pInst->hp = pInst->maxHp;
    pInst->pMonster = NULL;
    return ptr;
}

void CScene::AddMeiYingMonster(ShareFightPtr &pFight,uint8 num,uint8 begin,uint8 level)
{
    switch(num)
    {
    case 1:
        {
            ShareMonsterPtr pShareMonster = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster,begin+3);
        }
        break;
    case 2:
        {
            ShareMonsterPtr pShareMonster = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster,begin+3);
            ShareMonsterPtr pShareMonster1 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster1,begin+5);
        }
        break;
    case 3:
        {
            ShareMonsterPtr pShareMonster = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster,begin+1);
            ShareMonsterPtr pShareMonster1 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster1,begin+3);
            ShareMonsterPtr pShareMonster2 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster2,begin+5);
        }
        break;
    case 4:
        {
            ShareMonsterPtr pShareMonster = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster,begin);
            ShareMonsterPtr pShareMonster1 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster1,begin+2);
            ShareMonsterPtr pShareMonster2 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster2,begin+4);
            ShareMonsterPtr pShareMonster3 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster3,begin+3);
        }
        break;
    case 5:
        {
            ShareMonsterPtr pShareMonster = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster,begin);
            ShareMonsterPtr pShareMonster1 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster1,begin+2);
            ShareMonsterPtr pShareMonster2 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster2,begin+4);
            ShareMonsterPtr pShareMonster3 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster3,begin+1);
            ShareMonsterPtr pShareMonster4 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster4,begin+5);
        }
        break;
    case 6:
        {
            ShareMonsterPtr pShareMonster = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster,begin);
            ShareMonsterPtr pShareMonster1 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster1,begin+1);
            ShareMonsterPtr pShareMonster2 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster2,begin+2);
            ShareMonsterPtr pShareMonster3 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster3,begin+3);
            ShareMonsterPtr pShareMonster4 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster4,begin+4);
            ShareMonsterPtr pShareMonster5 = CreateMeiYing(level);
            pFight->AddMonster(pShareMonster5,begin+5);
        }
        break;
    }
}

ShareMonsterPtr CScene::CreateDiaoXiangMonster(uint8 level)
{
    SMonsterInst *pInst = new SMonsterInst;
    ShareMonsterPtr ptr(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 45;
    pInst->name = "雕像守护神";
    pInst->level = level;
    pInst->exp = 0;
    pInst->liliang = 6*pInst->level;
    pInst->lingxing = 6*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    pInst->xiang = 0;
    
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(level,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
        
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
        
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;

    pInst->maxHp *= 10;
    
    pInst->hp = pInst->maxHp;
    pInst->pMonster = NULL;
    return ptr;
}
void CScene::DiaoXiangFight(CUser *pU,int id)
{
    if((pU == NULL) || (pU->GetFightId() != 0))
        return;
        
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    ShareUserPtr pUser = m_onlineUser.GetUserBySock(pU->GetSock());
    if(pUser.get() == NULL)
        return;
    pFight->SetFightType(CFight::EFTDiaoXiang);
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        CUserTeam *pTeam = NULL;
        uint8 level = 1;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {
            level = AddTeamToFight(pFight,pTeam,7);
        }
        else
        {
            level = pUser->GetLevel();
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        }
        ShareMonsterPtr ptr = CreateDiaoXiangMonster(90);
        pFight->AddMonster(ptr,3);
        pFight->BeginFight(m_socketServer,this);
        pFight->SetDiaoXiangId(id);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::MeiYingFight(ShareUserPtr &pUser)
{
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->SetFightType(CFight::EFMeiYIng);
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 0;
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            num = Random(1,2);
            AddMeiYingMonster(pFight,num,1,pUser->GetLevel());
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            num = Random(pTeam->GetMemberNum(),2*pTeam->GetMemberNum());
            uint8 level = AddTeamToFight(pFight,pTeam,7);
            AddMeiYingMonster(pFight,num,1,level);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

static bool HaveShanTao(CUser *pUser,uint16 mapId)
{
    if(pUser == NULL)
        return false;
    
    if((mapId == 52) && (Random(0,100) <= 5))
    {
        char buf[128];
        const char *pMission = pUser->GetMission(17);
        if(pMission != NULL)
        {
            snprintf(buf,128,"%s",pMission);
            char *p[2];
            if(SplitLine(p,2,buf) >= 2)
            {
                if((atoi(p[0]) == 6) && (atoi(p[1]) < 30))
                {
                    return true;
                }
            }
        }
    }
    return false;
}


static bool HaveLangBattle11(CUser *pUser,uint16 mapId)
{
    if(pUser == NULL)
        return false;
    
    if((mapId == 156) && (Random(0,100) <= 20))
    {
        char buf[128];
        const char *pMission = pUser->GetMission(20);
        if(pMission != NULL)
        {
            snprintf(buf,128,"%s",pMission);
            char *p[2];
            if(SplitLine(p,2,buf) >= 2)
            {
                if((atoi(p[0]) == 13) && (atoi(p[1]) < 30))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

static bool HaveDuE(CUser *pUser,uint16 mapId)
{
    if(pUser == NULL)
        return false;
    
    if(((mapId == 50) || (mapId == 53) || (mapId == 97) || (mapId ==121) || (mapId == 152))
         && (Random(0,100) <= 5))
    {
        char buf[128];
        const char *pMission = pUser->GetMission(20);
        if(pMission != NULL)
        {
            snprintf(buf,128,"%s",pMission);
            char *p[2];
            if(SplitLine(p,2,buf) >= 2)
            {
                if((atoi(p[0]) == 9) && (atoi(p[1]) < 10000))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool HaveLangBattle7(CUser *pUser,uint16 mapId)
{
    if(pUser == NULL)
        return false;
    
    if((mapId == 66) && (Random(0,100) <= 50))
    {
        char buf[128];
        const char *pMission = pUser->GetMission(20);
        if(pMission != NULL)
        {
            snprintf(buf,128,"%s",pMission);
            char *p[2];
            if(SplitLine(p,2,buf) == 2)
            {
                if((atoi(p[0]) == 15) && (atoi(p[1]) < 10))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

//0 无任务，
//1 怪物宝宝。此时怪物宝宝出现在普通怪物群里。捕捉到即可按以上概率得到任务品。如果杀死，则获得相应的残骸。
//2 概率触发BOSS战。BOSS出现时战斗中对话：[打倒我能得到更多奖励]。BOSS强度：
//任务21 "sid|mid|0" sid场景id mid怪物id(47,48,49,50)
static int HaveMission21(CUser *pUser,uint16 mapId,uint16 &monsterId)
{
    if(pUser == NULL)
        return 0;
    if(Random(0,100) < 90)
        return 0;
    const char *pMission = pUser->GetMission(21);
    if(pMission == NULL)
        return 0;
    char buf[128];
    snprintf(buf,128,"%s",pMission);
    char *p[3];
    if(SplitLine(p,3,buf) == 3)
    {
        uint16 mId = atoi(p[0]);
        monsterId = atoi(p[1]);
        if((monsterId != 47) && (monsterId != 48) && (monsterId != 49) && (monsterId != 50))
            return 0;
        uint16 state = atoi(p[2]);
        if((mId == mapId) && (state == 0))
        {
            if(Random(0,100) < 90)
                return 1;
            else 
                return 2;
        }
    }
    return 0;
}

void CScene::MeetEnemy(ShareUserPtr pUser)
{
    if((m_mapId >= 262) && (m_mapId <= 266))
    {
        CSceneManager &scene = SingletonSceneManager::instance();
        CScene *pGroup = scene.FindScene(m_groupId);
        
        if(pGroup->GetOtherLeftMeiYing(pUser->GetBangPai()) > 0)
        {
            MeiYingFight(pUser);
            return;
        }
    }
    
    if(m_monsterNum <= 0)
        return;
    
    if(pUser->IsUpMonster())
    {
        CMonsterManager &monsterMgr = SingletonMonsterManager::instance();
        SMonsterTmpl *pMonster = monsterMgr.GetTmpl(m_monsters[0]);
        if(pMonster == NULL)
            return;
        if(pMonster->maxLevel < pUser->GetLevel())
            return;
    }
    
    if(HaveLangBattle11(pUser.get(),m_mapId))
    {
        LangBattle11(pUser.get());
        return;
    }
    uint16 monsterId = 0;
    int state = HaveMission21(pUser.get(),m_mapId,monsterId);
    if(state == 2)
    {
        Mission21Boss(pUser.get(),monsterId);
        return;
    }

    if(YeWaiShiYao(pUser.get()))
        return;

    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
#ifdef DEBUG
        cout<<"Create fight error"<<endl;
#endif            
        return;
    }
    pFight->SetFightType(CFight::EFTMeetMonster);
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        //1人1－3
        //两人2－4
        //三人3－6    
        uint8 num = 0;
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            if(pUser->IsMaxMonster())
                num = 2;
            else
                num = Random(1,2);
            AddMonsterToFight(pFight,num,1,pUser.get());
            
            if(HaveShanTao(pUser.get(),m_mapId))
            {
                uint8 xiang = 3;
                ShareMonsterPtr ptr = CreateQCFightMonster(24,EMTTongLing,42,"山桃宝宝",
                    1,8,8,xiang,6);
                ptr->SetCeLue(CL_SHAN_TAN);
                //cout<<"山桃宝宝速度:"<<ptr->speed<<endl;
                ptr->type = EMTNormal;
                pFight->AddMonster(ptr,3);
                //pFight->SetFightType(CFight::EFTScript);
            }
            else if(HaveDuE(pUser.get(),m_mapId))
            {
                uint8 xiang = Random(1,5);
                ShareMonsterPtr ptr = CreateQCFightMonster(20,EMTTongLing,56,"毒蛾大仙",
                    8,9,9,xiang);
                ptr->SetCeLue(CL_DU_E_DA_XIAN);
                ptr->type = EMTNormal;
                ptr->AddSkill((xiang-1)*4+3,90);
                ptr->AddSkill((xiang-1)*4+4,90);
                pFight->AddMonster(ptr,3);
            }
            else if(state == 1)
            {
                string name;
                if(monsterId == 47)
                    name = "九尾狐妖宝宝";
                else if(monsterId == 48)
                    name = "凤凰宝宝";
                else if(monsterId == 49)
                    name = "玄武宝宝";
                else if(monsterId == 50)
                    name = "青龙宝宝";
                if(name.length() > 0)
                {
                    ShareMonsterPtr ptr = CreateQCFightMonster(monsterId,EMTBaoBao,1,(char*)name.c_str(),
                        8,9,9,0);
                    ptr->SetCeLue(CL_MISSION21_BAOBAO);
                    pFight->AddMonster(ptr,3);
                }
            }
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            if(pTeam->GetMemberNum() == 2)
            {
                if(pUser->IsMaxMonster())
                    num = 4;
                else
                    num = Random(2,4);
            }
            else 
            {
                if(pUser->IsMaxMonster())
                    num = 6;
                else
                    num = Random(3,6);
            }
            AddMonsterToFight(pFight,num,1,pUser.get());
            
            if(HaveShanTao(pUser.get(),m_mapId))
            {
                uint8 xiang = 3;
                ShareMonsterPtr ptr = CreateQCFightMonster(24,EMTTongLing,42,"山桃宝宝",
                    1,8,8,xiang,6);
                ptr->SetCeLue(CL_SHAN_TAN);
                ptr->type = EMTNormal;
                //cout<<"山桃宝宝速度:"<<ptr->speed<<endl;
                //pFight->SetFightType(CFight::EFTScript);
                pFight->AddMonster(ptr,3);
            }
            else if(HaveDuE(pUser.get(),m_mapId))
            {
                uint8 xiang = Random(1,5);
                ShareMonsterPtr ptr = CreateQCFightMonster(20,EMTTongLing,56,"毒蛾大仙",
                    8,9,9,xiang);
                ptr->SetCeLue(CL_DU_E_DA_XIAN);
                ptr->type = EMTNormal;
                ptr->AddSkill((xiang-1)*4+3,90);
                ptr->AddSkill((xiang-1)*4+4,90);
                pFight->AddMonster(ptr,3);
            }
            else if(state == 1)
            {
                string name;
                if(monsterId == 47)
                    name = "九尾狐妖宝宝";
                else if(monsterId == 48)
                    name = "凤凰宝宝";
                else if(monsterId == 49)
                    name = "玄武宝宝";
                else if(monsterId == 50)
                    name = "青龙宝宝";
                if(name.length() > 0)
                {
                    ShareMonsterPtr ptr = CreateQCFightMonster(monsterId,EMTBaoBao,1,(char*)name.c_str(),
                        8,9,9,0);
                    ptr->SetCeLue(CL_MISSION21_BAOBAO);
                    pFight->AddMonster(ptr,3);
                }
            }
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

int CScene::GetTeamMemNum(uint32 teamId)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    CUserTeam *pTeam = NULL;
    if(!m_userTeams.Find(teamId,pTeam))
        return 0;
        
    uint32 members[MAX_TEAM_MEMBER];
    uint8 num = 0;
    pTeam->GetMember(members,num);
    return num;
}

CUser *CScene::GetTeamMember1(uint32 teamId)
{
    CUserTeam *pTeam = NULL;
    uint8 num = 0;
    uint32 members[MAX_TEAM_MEMBER];
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);    
        if(!m_userTeams.Find(teamId,pTeam))
            return NULL;
        pTeam->GetMember(members,num);
    }
    
    if(num >= 2)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[1]);
        if(p.get() != NULL)
        {
            return p.get();
        }
    }
    return NULL;
}

CUser *CScene::GetTeamMember2(uint32 teamId)
{
    uint32 members[MAX_TEAM_MEMBER];
    uint8 num = 0;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(teamId,pTeam))
            return NULL;
        
        pTeam->GetMember(members,num);
    }
    if(num >= 3)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[2]);
        if(p.get() != NULL)
        {
            return p.get();
        }
    }
    return NULL;
}

void CScene::ForEachTeamMember(uint32 teamId,boost::function<void(ShareUserPtr)> f)
{
    CUserTeam *pTeam = NULL;
    if(!m_userTeams.Find(teamId,pTeam))
        return;
        
    uint32 members[MAX_TEAM_MEMBER];
    uint8 num = 0;
    pTeam->GetMember(members,num);
    for(uint8 i = 0; i < num; i++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
        if(p.get() != NULL)
        {
            f(p);
        }
    }
    /*num = 0;
    pTeam->GetLeaveMem(members,num);
    for(uint8 i = 0; i < num; i++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
        if(p.get() != NULL)
        {
            f(p);
        }
    }*/
}

void CScene::NoLockChangeScene(CUser *pUser,CScene *pOldScene)
{
    if(pOldScene != NULL)
    {
        pOldScene->Exit(pUser);
        
        CUserTeam *pTeam = NULL;
        if((pUser->GetTeam() == pUser->GetRoleId())
            && (pOldScene->m_userTeams.Find(pUser->GetRoleId(),pTeam)))
        {//队长切换地图
            pOldScene->m_userTeams.Erase(pUser->GetRoleId());
            m_userTeams.Insert(pUser->GetRoleId(),pTeam);
            Enter(pUser);
            
            if(pTeam->GetHeadId() == pUser->GetRoleId())
            {//队长跳转，队员也跳转
                CNetMessage msg;
                msg.SetType(PRO_JUMP_SCENE);
                msg<<m_mapId<<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
                /*ForEachTeamMember(pTeam,boost::lambda::if_then(
                    (boost::lambda::_1->*&CUser::GetTeam)()!=_1->GetRoleId(),(m_socketServer.SendMsg(
                    (boost::lambda::_1->*&CUser::GetSock)(),msg),
                    (boost::lambda::_1->*&CUser::SetPos)()(pUser->GetX(),pUser->GetY()),
                    (boost::lambda::_1->*&CUser::SetFace)()(pUser->GetFace()),
                    (boost::lambda::_1->*&CUser::EnterScene)()(this)));*/
                    
                uint32 members[MAX_TEAM_MEMBER];
                uint8 num = 0;
                pTeam->GetMember(members,num);
                for(uint8 i = 1; i < num; i++)
                {
                    ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
                    if(p.get() != NULL)
                    {
                        m_socketServer.SendMsg(p->GetSock(),msg);
                        p->SetPos(pUser->GetX(),pUser->GetY());
                        p->SetFace(pUser->GetFace());
                        p->EnterScene(this);
                    }
                }
            }
            return;
        }
    }
    Enter(pUser);
}

void CScene::ChangeScene(CUser *pUser,CScene *pOldScene)
{
    /*if((pOldScene != NULL) 
        && (pOldScene->GetMapId() == LAN_RUO_DI_GONG_ID) 
        && (!pOldScene->m_addJump))
        return;*/
    AddShiYao(false);
    AddGuiYuMonster();
    AddLanRuoMonster();
    boost::recursive_mutex *pLock1 = NULL;
    boost::recursive_mutex *pLock2 = NULL;
    if(pOldScene != NULL)
    {
        if(m_id < pOldScene->GetId())
        {
            pLock1 = &m_mutex;
            pLock2 = &(pOldScene->m_mutex);
        }
        else
        {
            pLock2 = &m_mutex;
            pLock1 = &(pOldScene->m_mutex);
        }
    }
    else
    {
        pLock1 = &m_mutex;
    }
    
    if((pLock1 != NULL) && (pLock2 != NULL))
    {
        boost::recursive_mutex::scoped_lock lk(*pLock1);
        boost::recursive_mutex::scoped_lock lk1(*pLock2);
        NoLockChangeScene(pUser,pOldScene);
    }
    else if(pLock1 != NULL)
    {
        boost::recursive_mutex::scoped_lock lk(*pLock1);
        NoLockChangeScene(pUser,pOldScene);
    }
}

void CScene::SendUserList(CUser *pUser)
{
    CNetMessage msg;
    msg.SetType(PRO_USER_LIST);
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    list<uint32>::iterator iter = m_userList.begin();
    
    uint8 num = 0;
    uint8 pos = msg.GetDataLen();
    msg<<num;
    for(; iter != m_userList.end(); iter++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(*iter);
        if(p.get() != NULL) 
        {
            if((p->GetTeam() != 0) && (p->GetTeam() != p->GetRoleId()))
                continue;
            if(++num >= SEND_MAX_USER_NUM)
                break;
            msg<<p->GetRoleId()<<p->GetX()<<p->GetY()<<p->GetFace();
            uint32 teamId = p->GetTeam();
            CUserTeam *pTeam = NULL;
            if(teamId != 0)
            {
                m_userTeams.Find(teamId,pTeam);
            }
            if(pTeam != NULL)
            {
                uint32 members[MAX_TEAM_MEMBER];
                uint8 memNum = 0;
                pTeam->GetMember(members,memNum);
                msg<<(uint8)1<<(uint8)(memNum-1);
                for(uint8 i = 1; i < memNum; i++)
                {
                    msg<<members[i];
                }
            }
            else
            {
                msg<<(uint8)0;
            }
        }
    }
    msg.WriteData(pos,&num,sizeof(num));
    m_socketServer.SendMsg(pUser->GetSock(),msg);
    
    if(m_visibleMonsters.size() > 0)
    {
        msg.ReWrite();
        msg.SetType(MSG_SERVER_MONSTER);
        msg<<(uint8)m_visibleMonsters.size();
        for(list<SVisibleMonster>::iterator i = m_visibleMonsters.begin(); 
            i != m_visibleMonsters.end(); i++)
        {
            msg<<i->id<<i->x<<i->y<<i->face<<i->type<<i->pic;
        }
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
    
    if(m_npcList.size() <= 0)
    {
        msg.ReWrite();
        msg.SetType(PRO_NPC_LIST);
        pos = msg.GetDataLen();
        msg<<num;
        num = pUser->AddNpcInfo(m_mapId,msg);//(m_id,msg);
        
        if(num > 0)
        {
            msg.WriteData(pos,&num,1);
            m_socketServer.SendMsg(pUser->GetSock(),msg);
        }
        return;
    }
#ifdef DEBUG
    cout<<"send npc list to:"<<pUser->GetRoleId()<<endl;
#endif
    
    CNpcManager &npcManager = SingletonNpcManager::instance();
    list<uint16>::iterator i = m_npcList.begin();
    msg.ReWrite();
    msg.SetType(PRO_NPC_LIST);
    pos = msg.GetDataLen();
    msg<<(uint8)m_npcList.size();
    for (; i != m_npcList.end(); i++)
    {
        SNpcInstance *pInst = npcManager.GetNpcInstance(*i);
        if (pInst != NULL)
        {
            //msg<<pInst->id<<pInst->pNpc->name<<pInst->x<<pInst->y<<pInst->pNpc->pic;
            pInst->MakeNpcInfo(msg);
        }
    }
    num = pUser->AddNpcInfo(m_mapId,msg);//(m_id,msg);
    num += m_npcList.size();
    if(m_dynamicNpc.size() > 0)
    {
        for(list<SNpcInstance*>::iterator i = m_dynamicNpc.begin(); i != m_dynamicNpc.end(); i++)
        {
            num++;
            (*i)->MakeNpcInfo(msg);
        }
    }
    if(num > 0)
    {
        msg.WriteData(pos,&num,1);
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CScene::Enter(CUser *pUser)
{
    CNetMessage msg;
    if(m_mapId == 305)
    {
        CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
        CBangPai *pBangPai = bPMgr.FindBangPai(m_id>>16);
        if(pBangPai != NULL)
        {
            pBangPai->MakeZZMsg(msg);
            m_socketServer.SendMsg(pUser->GetSock(),msg);
        }
    }
    if(m_mapId == 306)
    {
        pUser->SetData32(11,GetSysTime());
    }
    if(m_addJump)
    {
        msg.ReWrite();
        msg.SetType(MSG_SERVER_JUMP_POINT);
        msg<<(uint8)0<<m_jumpPoint.x<<m_jumpPoint.y;
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
    msg.ReWrite();
    msg.SetType(PRO_USER_LIST);
    //boost::recursive_mutex::scoped_lock lk(m_mutex);
    //m_userList.remove(pUser->GetRoleId());
    m_userList.push_front(pUser->GetRoleId());
    
    uint8 num = 1;
    uint8 pos = msg.GetDataLen();
    msg<<num;
    
    if(pUser->GetTeam() == 0)// || (pUser->TempLeaveTeam()))
    {
        msg<<pUser->GetRoleId()<<pUser->GetX()<<pUser->GetY()<<pUser->GetFace()<<(uint8)0;
    }
    else
    {
        msg<<pUser->GetTeam()<<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
        uint32 teamId = pUser->GetTeam();
        CUserTeam *pTeam = NULL;
        if(teamId != 0)
        {
            m_userTeams.Find(teamId,pTeam);
        }
        if(pTeam != NULL)
        {
            uint32 members[MAX_TEAM_MEMBER];
            uint8 memNum = 0;
            pTeam->GetMember(members,memNum);
            msg<<(uint8)1<<(uint8)(memNum-1);
            for(uint8 i = 1; i < memNum; i++)
            {
                msg<<members[i];
            }
        }
        else
        {
            msg<<(uint8)0;
        }
    }
    list<uint32>::iterator iter = m_userList.begin();
    iter ++;
    for(; iter != m_userList.end(); iter++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(*iter);
        if(p.get() != NULL) 
        {
            if(pUser->GetTeam() == p->GetRoleId())
                continue;
            if((p->GetTeam() != 0) && (p->GetTeam() != p->GetRoleId()))
                continue;
            if(++num >= SEND_MAX_USER_NUM)
                break;
            msg<<p->GetRoleId()<<p->GetX()<<p->GetY()<<p->GetFace();
            uint32 teamId = 0;
            if(p->GetRoleId() == p->GetTeam())
                teamId = p->GetRoleId();
            CUserTeam *pTeam = NULL;
            if(teamId != 0)
            {
                m_userTeams.Find(teamId,pTeam);
            }
            if(pTeam != NULL)
            {
                uint32 members[MAX_TEAM_MEMBER];
                uint8 memNum = 0;
                pTeam->GetMember(members,memNum);
                msg<<(uint8)1<<(uint8)(memNum-1);
                for(uint8 i = 1; i < memNum; i++)
                {
                    msg<<members[i];
                }
            }
            else
            {
                msg<<(uint8)0;
            }
        }
    }
    msg.WriteData(pos,&num,sizeof(num));
    m_socketServer.SendMsg(pUser->GetSock(),msg);
    
    if((pUser->GetTeam() == 0) || (pUser->GetTeam() == pUser->GetRoleId()))
    {
        msg.ReWrite();
        msg.SetType(PRO_IN_OUT_SCENE);
        uint32 teamId = 0;
        if(pUser->GetRoleId() == pUser->GetTeam())
            teamId = pUser->GetTeam();
        msg<<pUser->GetRoleId()<<(uint8)1<<pUser->GetX()<<pUser->GetY()<<pUser->GetFace();
        CUserTeam *pTeam = NULL;
        if(teamId != 0)
        {
            m_userTeams.Find(teamId,pTeam);
        }
        if(pTeam != NULL)
        {
            uint32 members[MAX_TEAM_MEMBER];
            uint8 memNum = 0;
            pTeam->GetMember(members,memNum);
            msg<<(uint8)1<<(uint8)(memNum-1);
            for(uint8 i = 1; i < memNum; i++)
            {
                msg<<members[i];
            }
        }
        else
        {
            msg<<(uint8)0;
        }
        BroadcastMsgExceptSameTeam(pUser,msg);
    }
    if(m_visibleMonsters.size() > 0)
    {
        msg.ReWrite();
        msg.SetType(MSG_SERVER_MONSTER);
        msg<<(uint8)m_visibleMonsters.size();
        for(list<SVisibleMonster>::iterator i = m_visibleMonsters.begin(); 
            i != m_visibleMonsters.end(); i++)
        {
            msg<<i->id<<i->x<<i->y<<i->face<<i->type<<i->pic;
        }
        m_socketServer.SendMsg(pUser->GetSock(),msg);
    }
    
    if(m_npcList.size() <= 0)
    {
        msg.ReWrite();
        msg.SetType(PRO_NPC_LIST);
        pos = msg.GetDataLen();
        msg<<num;
        num = pUser->AddNpcInfo(m_mapId,msg);//(m_id,msg);
        
        if(num > 0)
        {
            msg.WriteData(pos,&num,1);
            m_socketServer.SendMsg(pUser->GetSock(),msg);
        }
        return;
    }
#ifdef DEBUG
    cout<<"send npc list to:"<<pUser->GetRoleId()<<endl;
#endif
    
    CNpcManager &npcManager = SingletonNpcManager::instance();
    list<uint16>::iterator i = m_npcList.begin();
    msg.ReWrite();
    msg.SetType(PRO_NPC_LIST);
    pos = msg.GetDataLen();
    msg<<(uint8)m_npcList.size();
    for (; i != m_npcList.end(); i++)
    {
        SNpcInstance *pInst = npcManager.GetNpcInstance(*i);
        if (pInst != NULL)
        {
            //msg<<pInst->id<<pInst->pNpc->name<<pInst->x<<pInst->y<<pInst->pNpc->pic;
            pInst->MakeNpcInfo(msg);
        }
    }
    num = pUser->AddNpcInfo(m_mapId,msg);//(m_id,msg);
    num += m_npcList.size();
    if(m_dynamicNpc.size() > 0)
    {
        for(list<SNpcInstance*>::iterator i = m_dynamicNpc.begin(); i != m_dynamicNpc.end(); i++)
        {
            num++;
            (*i)->MakeNpcInfo(msg);
        }
    }
    if(num > 0)
    {
        msg.WriteData(pos,&num,1);
    }
    m_socketServer.SendMsg(pUser->GetSock(),msg);
}

void CScene::MakeNearPlayerList(CUser *pUser,uint8 page,CNetMessage &msg)
{
    page -= 1;
    
    uint8 num = 0;
    uint8 pos = msg.GetDataLen();
    msg<<num;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    list<uint32>::iterator iter = m_userList.begin();
    /*for(int i = 0; i < ONE_PAGE_MAX_NUM * page; i++)
    {
        if(iter != m_userList.end())
        {
            iter++;
        }
        else
        {
            return;
        }
    }*/
    
    for(; iter != m_userList.end(); iter++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(*iter);
        if((p.get() == NULL) || (p->GetRoleId() == pUser->GetRoleId()))
            continue;
        num++;
        msg<<p->GetRoleId()<<p->GetName()<<p->GetLevel();
        if(num >= 50)
            break;
    }
    msg.WriteData(pos,&num,sizeof(num));
}

void CScene::MakeCloseUser(CUser *pUser,CNetMessage &msg,int bId,int xzBangId)
{
    uint8 num = 0; 
    uint8 pos = msg.GetDataLen();
    msg<<num;
    
    list<uint32> userList;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        userList = m_userList;
    }
    
    list<uint32>::iterator iter = userList.begin();
    uint8 disX,disY;
    for(; iter != userList.end(); iter++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(*iter);
        if((p.get() == NULL) || (p->GetRoleId() == pUser->GetRoleId()))
            continue;
        disX = abs(pUser->GetX() - p->GetX());
        disY = abs(pUser->GetY() - p->GetY());
        if((disX <= 3) && (disY <= 3))
        {
            uint8 state = 0;
            if(p->GetFightId() != 0)
                state |= 1;
            if(p->GetTeam() == p->GetRoleId())
                state |= 2;
            
            if(p->HaveShop())
                state |= 4;
            if(xzBangId != 0)
            {
                if((bId == (int)p->GetXZBangId()) || (xzBangId == (int)p->GetBangPai()))
                    state |= 8;
            }
            else
            {
                CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
                CBangPai *pBangPai1 = NULL;
                CBangPai *pBangPai2 = NULL;
                if(bId != 0)
                    pBangPai1 = bPMgr.FindBangPai(bId);
                if(pUser->GetBangPai() != 0)
                    pBangPai2 = bPMgr.FindBangPai(p->GetBangPai());
                
                if((pBangPai1 != NULL) && (pBangPai2 != NULL))
                {
                    if((pBangPai1->GetXuanZhanBang() == (int)pBangPai2->GetId()) 
                        || (pBangPai2->GetXuanZhanBang() == (int)pBangPai1->GetId()))
                    {
                        state |= 8;
                    } 
                }
            }
            if(InWorldWar())    
            {
                //if(((WWGetWinBang() == (int)p->GetBangPai())&& ((int)bId != WWGetWinBang()))
                //        || ((WWGetWinBang() == (int)bId) && ((int)p->GetBangPai()!= WWGetWinBang())))
                if((IsFangShou(p->GetBangPai()) && !IsFangShou(bId))
                    || (!IsFangShou(p->GetBangPai()) && IsFangShou(bId)))
                state |= 8;
            }
            num++;
            msg<<p->GetRoleId()<<p->GetName()<<state<<p->GetLevel();
            if(num >= 9)
                break;
        }
    }
    msg.WriteData(pos,&num,sizeof(num));
}

void CScene::PlayerAskForMatch(ShareUserPtr pUser,uint32 roleId)
{
    if((m_fightType & 2) == 0)
    {
        SendSysInfo(pUser.get(),"本场景不允许切磋");
        return;
    }
    
    ShareUserPtr p;    
    if(roleId == 0)
    {
        if(!FindFacePlayer(pUser.get(),p))
        {
            SendSysInfo(pUser.get(),"相邻位置无玩家");
            return;
        }
    }
    else
    {
        p = m_onlineUser.GetUserByRoleId(roleId);
    }
    
    if(p.get() == NULL)
    {
        SendSysInfo(pUser.get(),"对方已下线");
        return;
    }
    if(p->GetScene() != this)
        return;
    
    uint32 teamHeadid = p->GetTeam();
    if(teamHeadid != 0)
    {
        p = m_onlineUser.GetUserByRoleId(teamHeadid);
        if(p.get() == NULL)
        {
            SendSysInfo(pUser.get(),"对方已下线");
            return;
        }
    }
    
    if(p->HaveIgnore(pUser->GetRoleId()))
        return;
        
    if(p->GetFightId() != 0)
    {
        SendSysInfo(pUser.get(),"对方在战斗中，不能切磋");
        return;
    }
    if((pUser->GetTeam() != 0) && (p->GetTeam() == pUser->GetTeam()))
        return;
    
    CNetMessage msg;
    msg.SetType(PRO_PLYAER_MATCH);
    msg<<(uint8)1<<pUser->GetRoleId()<<pUser->GetName();
    m_socketServer.SendMsg(p->GetSock(),msg);
    
    p->AddAskForMatchUser(pUser->GetRoleId());
}

void CScene::AcceptAskForMatch(ShareUserPtr pUser,bool accept,uint32 roleId)
{
    ShareUserPtr p = m_onlineUser.GetUserByRoleId(roleId);
    
    if(p.get() == NULL)
    {
        SendSysInfo(pUser.get(),"对方已下线");
        return;
    }
    if(!pUser->InAskForMatchUser(p->GetRoleId()))
    {
        return;
    }
    
    if((p->GetFightId() != 0) || (pUser->GetFightId() != 0))
    {
        SendSysInfo(pUser.get(),"战斗中，不能切磋");
        return;
    }
    if(p->GetScene() != pUser->GetScene())
    {
        SendSysInfo(pUser.get(),"你与对方不在同一场景，不能切磋");
        return;
    }
    
    CNetMessage msg;
    msg.SetType(PRO_PLYAER_MATCH);
    
    if(!accept)
    {
        msg<<(uint8)2<<(uint8)0;
        m_socketServer.SendMsg(p->GetSock(),msg);
        pUser->DelAskForMatchUser(roleId);
        return;
    }
    if((pUser->GetTeam() != 0) && (pUser->GetTeam() == p->GetTeam()))
    {
        return;
    }
    
    ShareFightPtr pFight = m_fightManager.CreateFight();
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        pUser->ClearAskForMatchUser();
        
        pFight->SetFightType(CFight::EFTPlayerQieCuo);
        
        CUserTeam *pTeam = NULL;
        if((pUser->GetTeam() != 0) && (m_userTeams.Find(pUser->GetTeam(),pTeam)))
        {
            AddTeamToFight(pFight,pTeam,1);
        }
        else
        {
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,3));
        }
        
        if((p->GetTeam() != 0) && (m_userTeams.Find(p->GetTeam(),pTeam)))
        {
            AddTeamToFight(pFight,pTeam,7);
        }
        else
        {
            p->SetFight(pFight->GetId(),pFight->AddUser(p,9));
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::SceneChat(CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    BroadcastMsg(msg,true);
}

static void SendChatMsg(ShareUserPtr pUser,CSocketServer *pSock,CNetMessage *msg)
{
    if((pUser->GetChatChannel() & 4) != 0)
        pSock->SendMsg(pUser->GetSock(),*msg);
}

void CScene::TeamChat(uint32 teamId,CNetMessage &msg)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    //ForEachTeamMember(teamId,boost::bind(SendChatMsg,_1,&m_socketServer,&msg));
    CUserTeam *pTeam = NULL;
    if(!m_userTeams.Find(teamId,pTeam))
        return;
        
    uint32 members[MAX_TEAM_MEMBER];
    uint8 num = 0;
    pTeam->GetMember(members,num);
    for(uint8 i = 0; i < num; i++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
        if(p.get() != NULL)
        {
            SendChatMsg(p,&m_socketServer,&msg);
        }
    }
    num = 0;
    pTeam->GetLeaveMem(members,num);
    for(uint8 i = 0; i < num; i++)
    {
        ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
        if(p.get() != NULL)
        {
            SendChatMsg(p,&m_socketServer,&msg);
        }
    }
}

void CScene::AddNpc(uint16 tmplId,uint8 x,uint8 y,uint8 direct)
{
    CNpcManager &npcManager = SingletonNpcManager::instance();
    SNpcTemplate *pNpc = npcManager.GetNpcTemplate(tmplId);
    if(pNpc == NULL)
        return;
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    SNpcInstance *pInst = new SNpcInstance();
    pInst->id = tmplId;
    pInst->direct = direct;
    pInst->templateId = tmplId;
    pInst->pNpc = pNpc;
    pInst->sceneId = m_mapId;
    pInst->x = x;
    pInst->y = y;
    m_dynamicNpc.push_back(pInst);
    
    CNetMessage msg;
    
    for(list<uint32>::iterator i = m_userList.begin(); i != m_userList.end(); i++)
    {
        ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(*i);
        CUser *pUser = ptr.get();
        if(pUser != NULL)
        {
            msg.SetType(PRO_ADD_NPC);
            pInst->MakeNpcInfo(msg);
            m_socketServer.SendMsg(pUser->GetSock(),msg);
        }
    }
}
void CScene::ModifyNpcPos(uint16 id,uint8 x,uint8 y)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(list<SNpcInstance*>::iterator i = m_dynamicNpc.begin(); i != m_dynamicNpc.end(); i++)
    {
        if((*i)->id == id)
        {
            (*i)->x = x;
            (*i)->y = y;
            return;
        }
    }
}

void CScene::AddNpc(uint16 id,bool sendMsg)
{
    CNpcManager &npcManager = SingletonNpcManager::instance();
    SNpcTemplate *pNpc = npcManager.GetNpcTemplate(id);
    if(pNpc == NULL)
        return;
        
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_dynamicNpcPoint.size() <= 0)
        return;
        
    uint8 pos = Random(1,m_dynamicNpcPoint.size());
    pos -= 1;
    uint8 x,y;
    x = m_dynamicNpcPoint[pos].x;
    y = m_dynamicNpcPoint[pos].y;
    m_dynamicNpcPoint.erase(m_dynamicNpcPoint.begin() + pos);
    
    SNpcInstance *pInst = new SNpcInstance();
    pInst->id = id;
    pInst->templateId = id;
    pInst->pNpc = pNpc;
    pInst->sceneId = m_mapId;
    pInst->x = x;
    pInst->y = y;
    m_dynamicNpc.push_back(pInst);
    
    if(sendMsg)
    {
        CNetMessage msg;
        
        for(list<uint32>::iterator i = m_userList.begin(); i != m_userList.end(); i++)
        {
            ShareUserPtr ptr = m_onlineUser.GetUserByRoleId(*i);
            CUser *pUser = ptr.get();
            if(pUser != NULL)
            {
                msg.SetType(PRO_ADD_NPC);
                pInst->MakeNpcInfo(msg);
                m_socketServer.SendMsg(pUser->GetSock(),msg);
            }
        }
    }
}

SNpcInstance *CScene::FindNpc(uint8 x,uint8 y)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    for(list<SNpcInstance*>::iterator i = m_dynamicNpc.begin(); i != m_dynamicNpc.end(); i++)
    {
        if(((*i)->x == x) && ((*i)->y == y))
        {
            return *i;
        }
    }
    return NULL;
}

SNpcInstance *CScene::FindNpc(uint16 id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    for(list<SNpcInstance*>::iterator i = m_dynamicNpc.begin(); i != m_dynamicNpc.end(); i++)
    {
        if((*i)->id == id)
        {
            return *i;
        }
    }
    return NULL;
}

void CScene::InitNpcPoint(SPoint *pPoint,uint8 num)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    m_dynamicNpcPoint.clear();
    for(uint8 i = 0; i < num; i++)
    {
        m_dynamicNpcPoint.push_back(pPoint[i]);
    }
}

void CScene::DelNpc(uint16 id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(list<SNpcInstance*>::iterator i = m_dynamicNpc.begin(); i != m_dynamicNpc.end(); i++)
    {
        if((*i)->id == id)
        {
            CNetMessage msg;
            msg.SetType(PRO_DEL_NPC);
            msg<<(*i)->id;
            BroadcastMsg(msg);
                        
            delete *i;
            m_dynamicNpc.erase(i);
            return;
        }
    }
}

void CScene::DelNpc(uint8 x,uint8 y)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(list<SNpcInstance*>::iterator i = m_dynamicNpc.begin(); i != m_dynamicNpc.end(); i++)
    {
        if(((*i)->x == x) && ((*i)->y == y))
        {
            CNetMessage msg;
            msg.SetType(PRO_DEL_NPC);
            msg<<(*i)->id;
            BroadcastMsg(msg);
                        
            delete *i;
            m_dynamicNpc.erase(i);
            SPoint point = {x,y};
            m_dynamicNpcPoint.push_back(point);
            return;
        }
    }
    //vector<SPoint> m_dynamicNpcPoint;
}

bool CScene::InitEpisodeBattle(CUser *p,ShareFightPtr &pFight,ShareUserPtr &pUser)
{
    if(p == NULL)
        return false;
    
    pUser = m_onlineUser.GetUserBySock(p->GetSock());
    if(pUser.get() == NULL)
        return false;
        
    pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return false;
    }
    pFight->SetFightType(CFight::EFTScript);
    return true;
}
                                            //3,5
uint8 CScene::AddEpisodeBattleMonster(ShareFightPtr &pFight,uint8 ind,uint8 pos,uint8 level,SMonsterInst **ppMonster)
{
    //id name level,hp,mp,speed,物攻,法术攻击,向性(0物理，<0随机向性),气血倍率
    struct SEpisodeMonster
    {
        uint16 id;
        string name;
        uint8 level;
        int hp;
        int mp;
        int speed;
        int wugong;
        int fagong;
        int xiang;
        int hpBeiLv;
        int li;
        int ling;
        int daohang;
        const char *skills;
    };
    
    //id name level,hp,mp,speed,物攻,法术攻击,向性(0物理，<0随机向性),气血倍率
    SEpisodeMonster monsters[] = 
    {
        //{41, "怨灵",10,60,45,20,40,35,0,10,6,6,2000},
        //{41 ,"怨灵",60,10,10,10,15,-25,5,10,0,0,2000},
        {29,"厉鬼",10,55,45,20,40,35,0,10,6,6,2000,"10|6|12|6|19|6|20|6"},
        {3, "乌龟精",15,80,55,10,30,25,0,10,6,6,6750},
        {4, "蝙蝠怪",15,50,50,30,45,40,0,10,6,6,6750},
        {5, "竹妖",20,70,50,20,35,40,1,10,6,6,16000},
        {6, "野猪精",20,85,30,10,45,30,0,10,6,6,16000},
        {24, "封印神龛",24,70,50,20,50,50,-1,15,6,6,27648},
        {38, "血魔",30,85,40,40,55,50,-1,15,6,6,54000},
        {42, "云翳",55,105,50,35,70,65,-1,15,7,7,332750},
        {39, "魔军先锋",60,110,60,60,80,80,-1,15,7,7,432000},
        {46, "魔军精英",65,120,65,60,80,80,-1,15,7,7,549250},
        {42, "魔军将领",68,130,65,60,85,85,-1,15,7,7,628864},
        {40, "魔军追兵",72,130,70,60,90,90,-1,15,7,7,746496},
        {21, "春秋不败",78,130,75,60,100,95,-1,20,7,7,949104},
        {42, "魔礼青",80,130,80,70,100,95,-1,20,7,7,1024000},
        {42, "魔礼红",82,135,80,70,100,95,-1,20,7,7,1102736},
        {42, "魔礼海",82,140,80,70,100,95,-1,20,7,7,1102736},
        {42, "魔礼海",86,140,85,70,105,100,-1,20,7,7,1272112},
        {42, "魔礼寿",88,150,90,70,110,105,-1,20,7,7,1362944},
        {40, "魔亲卫军",3,120,90,60,90,85,-1,15,7,7,1024000},
        {33, "傲世狂刀",50,100,50,25,70,65,-1,20,6,6,250000},
        {14, "拜月圣使",38,90,50,60,70,65,0,15,6,6,109744},
        {34, "千年树妖",45,100,50,60,75,70,-1,18,7,7,182250},
        {38, "树妖精",42,90,50,40,70,65,-1,15,7,7,148176},
        {19, "嗜血幼蛟",35,90,50,50,70,70,-1,15,6,6,85750},//喽罗
        {19, "赤炼魔蛟",38,100,50,60,70,70,-1,15,7,7,109744},
        {22, "樊溪鱼怪",40,100,40,20,50,50,-1,15,6,6,128000}//根据人数出
    };
    
    if(ind >= sizeof(monsters)/sizeof(SEpisodeMonster))
        return 0;
        
    ShareMonsterPtr ptr;
    SMonsterInst *pInst = new SMonsterInst;
    if(ppMonster != NULL)
        *ppMonster = pInst;
    ptr.reset(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = monsters[ind].id;
    pInst->name = monsters[ind].name;
        
    if(monsters[ind].xiang < 0)
        pInst->xiang = Random(1,5);
    else
        pInst->xiang = monsters[ind].xiang;
    
    if(level != 0)
        pInst->level = level;
    else
        pInst->level = monsters[ind].level;
        
    pInst->exp = 0;
    pInst->liliang = pInst->level;
    pInst->lingxing = pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
        
    pInst->liliang = (monsters[ind].li + 1) * pInst->level;
    pInst->lingxing = (monsters[ind].ling + 1) * pInst->level;
    pInst->daohang = monsters[ind].daohang;
    
    int addHp = monsters[ind].hp;
    int addMp = monsters[ind].mp;
    int addSpeed = monsters[ind].speed;
    int addAttack = monsters[ind].wugong;
    int addSkillAttack = monsters[ind].fagong;
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;
    pInst->daohang = 3*pInst->level*pInst->level*pInst->level;
    if(monsters[ind].skills != NULL)
    {
        char *p[20];
        char skills[32];
        strncpy(skills,monsters[ind].skills,31);
        uint8 num = SplitLine(p,20,skills);
        if(num%2 == 0)
        {
            for(uint8 i = 0; i < num/2; i++)
            {
                pInst->AddSkill(atoi(p[2*i]),atoi(p[2*i+1]));
            }
        }
    }
    
    ptr->maxHp *= monsters[ind].hpBeiLv;
    ptr->hp = ptr->maxHp;
    
    pFight->AddMonster(ptr,pos);
    
    /*cout<<ptr->attack<<endl
        <<ptr->addSkillAttack<<endl
        <<ptr->hp<<endl
        <<ptr->recovery<<endl;*/
        
    return monsters[ind].level;
}

uint8 CScene::BeginEpisodeBattle(ShareFightPtr &pFight,ShareUserPtr &pUser)
{
    CUserTeam *pTeam = NULL;
    if(m_userTeams.Find(pUser->GetTeam(),pTeam))
    {
        uint8 level = AddTeamToFight(pFight,pTeam,7);
        pFight->BeginFight(m_socketServer,this);
        return level;
    }
    else
    {
        pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        pFight->BeginFight(m_socketServer,this);
        return pUser->GetLevel();
    }
}

ShareMonsterPtr CScene::CreateQCFightMonster(uint32 monsterId,EMonsterType type,uint8 level,const char *name,
    int xue,int liliang,int lingli,uint8 xiang,int minjie)
{
    ShareMonsterPtr ptr;
    SMonsterTmpl *pTmpl = m_monsterManager.GetTmpl(monsterId);
    if(pTmpl == NULL)
    {
        return ptr;
    }
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = type;
    pInst->tmplId = monsterId;
    pInst->xiang = pTmpl->xiang;
    
    pInst->name = pTmpl->name;
    pInst->level = level;
    
    pInst->exp = 0;
    pInst->liliang = pInst->level;
    pInst->lingxing = pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    if(minjie != 0)
        pInst->minjie = pInst->level*minjie;
    else
        pInst->minjie = pInst->level;
    pInst->liliang += pInst->level*liliang;    
    pInst->lingxing += pInst->level*lingli;
    
    pInst->Init(pTmpl,false);
    pInst->maxHp *= xue;
    pInst->hp = pInst->maxHp;
    if(name != NULL)
        pInst->name = name;
    
    return ptr;
}

void CScene::QiChongBattle1(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        for(uint8 i = 3; i <= 5; i++)
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(13,EMTTongLing,33,"酒店伙计",
                8,7,7,0);
            ptr->tmplId = 9;
            ptr->AddSkill(19,60);
            ptr->lianji = 60;//连击概率
            ptr->lianjiAdd = 100;//连击追加
            ptr->lianjiShu = 2;//连击数
            ptr->mingzhong = 500;
            pFight->AddMonster(ptr,i);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle2(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    uint8 num = GetTeamMemNum(pU->GetRoleId());
    num *= 2;
    if(num <= 0)
        num = 2;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        for(uint8 i = 1; i <= num; i++)
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(14,EMTTongLing,34,"树蚁",
                7,6,6,0);
            ptr->tmplId = 2;
            ptr->AddSkill(7,60);
            ptr->lianji = 60;//连击概率
            ptr->lianjiAdd = 100;//连击追加
            ptr->lianjiShu = 2;//连击数
            ptr->mingzhong = 500;
            pFight->AddMonster(ptr,i);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle3(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        ShareMonsterPtr ptr = CreateQCFightMonster(14,EMTTongLing,35,"仙葫芦",
                20,8,9,1);
        ptr->tmplId = 100;
        ptr->AddSkill(7,60);
        ptr->AddSkill(152,90);
        pFight->AddMonster(ptr,3);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle4(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = GetTeamMemNum(pU->GetRoleId())*2;
        if(num <= 0)
            num = 2;
        for(uint8 i = 1; i <= num; i++)
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(15,EMTTongLing,36,"窥伺种子的怪物",
                7,7,7,Random(1,5));
            ptr->tmplId = 17;
            ptr->AddSkill(7,60);
            ptr->AddSkill(8,60);
            pFight->AddMonster(ptr,i);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle5(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        ShareMonsterPtr ptr = CreateQCFightMonster(18,EMTTongLing,40,"巨灵怪",
                20,9,8,1);
        ptr->tmplId = 55;
        ptr->AddSkill(3,60);
        ptr->AddSkill(57,90);
        ptr->lianji = 60;//连击概率
        ptr->lianjiAdd = 300;//连击追加
        ptr->lianjiShu = 3;//连击数
        ptr->mingzhong = 1000;
        pFight->AddMonster(ptr,4);
        ShareMonsterPtr ptr1 = CreateQCFightMonster(17,EMTTongLing,40,"窥伺种子的怪物",
                20,9,8,1);
        pFight->AddMonster(ptr1,2);
        ShareMonsterPtr ptr2 = CreateQCFightMonster(17,EMTTongLing,40,"窥伺种子的怪物",
                20,9,8,1);
        pFight->AddMonster(ptr2,6);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle6(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        for(uint8 i = 1; i <= 6; i++)
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(16,EMTTongLing,40,"酒店伙计",
                8,8,7,0);
            ptr->tmplId = 9;
            ptr->AddSkill(19,60);
            pFight->AddMonster(ptr,i);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle7(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        for(uint8 i = 2; i <= 4; i++)
        {
            uint8 xiang = Random(1,5);
            ShareMonsterPtr ptr = CreateQCFightMonster(17,EMTTongLing,40,"嗜酒大仙",
                8,7,7,xiang);
            ptr->tmplId = 27;
            ptr->AddSkill((xiang-1)*4+3,60);
            ptr->AddSkill((xiang-1)*4+4,60);
            ptr->SetCeLue(FIRST_HULUO);
            pFight->AddMonster(ptr,i);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle8(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        for(uint8 i = 3; i <= 5; i++)
        {
            uint8 xiang = 4;
            ShareMonsterPtr ptr = CreateQCFightMonster(17,EMTTongLing,41,"冤魂",
                8,8,8,xiang);
            ptr->tmplId = 24;
            ptr->AddSkill((xiang-1)*4+4,60);
            ptr->AddSkill(152,90);
            ptr->mingzhong = 1000;
            ptr->SetCeLue(CL_HL_DIE_END);
            pFight->AddMonster(ptr,i);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle9(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 3;
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(18,EMTTongLing,45,"隐龙洞主",
                30,9,9,xiang);
            ptr->tmplId = 49;
            ptr->AddSkill((xiang-1)*4+3,60);
            ptr->AddSkill((xiang-1)*4+4,60);
            ptr->AddSkill(61,90);
            ptr->AddSkill(57,60);
            ptr->mingzhong = 1000;
            ptr->fashubaojilv = 60;//法术爆击率
            ptr->fashubaojiadd = 500;
            pFight->AddMonster(ptr,3);
        }
        
        xiang = 4;
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(17,EMTTongLing,41,"冤魂",
                8,8,8,xiang);
            ptr->tmplId = 24;
            ptr->AddSkill((xiang-1)*4+4,60);
            ptr->AddSkill(152,90);
            ptr->mingzhong = 1000;
            ptr->SetCeLue(CL_HL_DIE_END);
            pFight->AddMonster(ptr,1);
        }
        
        xiang = 4;
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(17,EMTTongLing,41,"冤魂",
                8,8,8,xiang);
            ptr->tmplId = 24;
            ptr->AddSkill((xiang-1)*4+4,60);
            ptr->AddSkill(152,90);
            ptr->mingzhong = 1000;
            ptr->SetCeLue(CL_HL_DIE_END);
            pFight->AddMonster(ptr,5);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle10(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 3;
        ShareMonsterPtr ptr = CreateQCFightMonster(17,EMTTongLing,42,"山桃宝宝",
            30,8,8,xiang);
        ptr->tmplId = 24;
        ptr->SetCeLue(CL_SHAN_TAN);
        pFight->AddMonster(ptr,3);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle11(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        CUserTeam *pTeam = NULL;
        uint8 num = 0;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {
            AddTeamToFight(pFight,pTeam,7);
            num = 3*pTeam->GetMemberNum();
        }
        else
        {
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            num = 3;
        }
        if(num > 6)
            num = 6;
        for(uint8 i = 1; i <= num; i++)
        {
            uint8 xiang = 3;
            ShareMonsterPtr ptr = CreateQCFightMonster(17,EMTTongLing,43,"石猴",
                8,7,7,xiang);
            ptr->AddSkill(7,60);
            ptr->AddSkill(152,90);
            ptr->lianji = 60;
            ptr->lianjiShu = 2;
            ptr->lianjiAdd = 200;
            ptr->mingzhong = 1000;
            pFight->AddMonster(ptr,i);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle12(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 0;
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(20,EMTTongLing,48,"六耳妖猴",
                    25,8,8,xiang);
            ptr->tmplId = 46;
            ptr->AddSkill(7,60);
            ptr->AddSkill(111,3000);
            ptr->SetCeLue(CL_LIU_ER_MI_HOUH);        
            pFight->AddMonster(ptr,3);
        }
        {
            xiang = 3;
            ShareMonsterPtr ptr = CreateQCFightMonster(17,EMTTongLing,43,"石猴",
            8,7,7,xiang);
            ptr->AddSkill(7,60);
            ptr->AddSkill(152,90);
            ptr->lianji = 60;
            ptr->lianjiShu = 2;
            ptr->lianjiAdd = 200;
            ptr->mingzhong = 1000;
            pFight->AddMonster(ptr,1);
        }
        {
            xiang = 3;
            ShareMonsterPtr ptr = CreateQCFightMonster(17,EMTTongLing,43,"石猴",
            8,7,7,xiang);
            ptr->AddSkill(7,60);
            ptr->AddSkill(152,90);
            ptr->lianji = 60;
            ptr->lianjiShu = 2;
            ptr->lianjiAdd = 200;
            ptr->mingzhong = 1000;
            pFight->AddMonster(ptr,5);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::QiChongBattle13(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 2;
        ShareMonsterPtr ptr = CreateQCFightMonster(21,EMTTongLing,50,"树妖王",
                30,8,8,xiang);
        ptr->tmplId = 38;
        ptr->AddSkill(7,60);
        ptr->AddSkill(61,90);
        ptr->AddSkill(57,90);
        ptr->AddSkill(112,3000);
        ptr->SetCeLue(CL_SHUYAO);
        pFight->AddMonster(ptr,3);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::QiChongBattle14(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 2;
        ShareMonsterPtr ptr = CreateQCFightMonster(21,EMTTongLing,50,"仙葫芦",
                20,9,9,xiang);
        ptr->tmplId = 100;
        ptr->AddSkill(7,60);
        ptr->AddSkill(61,90);
        ptr->AddSkill(57,90);
        ptr->AddSkill(112,3000);
        ptr->fanshang = 60;
        ptr->fanshangadd = 60;
        ptr->SetCeLue(CL_XIAN_HU_LU);
        pFight->AddMonster(ptr,1);
        
        xiang = 3;
        ShareMonsterPtr ptr1 = CreateQCFightMonster(21,EMTTongLing,50,"仙葫芦",
                20,9,9,xiang);
        ptr1->tmplId = 100;
        ptr1->AddSkill(7,60);
        ptr1->AddSkill(61,90);
        ptr1->AddSkill(57,90);
        ptr1->AddSkill(112,3000);
        ptr1->fanshang = 60;
        ptr1->fanshangadd = 60;
        ptr1->SetCeLue(CL_XIAN_HU_LU);
        pFight->AddMonster(ptr1,5);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LangBattle1(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 0;
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(101,EMTTongLing,55,"锋牙狼王",
                   25,9,1,xiang);
            ptr->AddSkill(24,100);
            ptr->SetCeLue(CL_FENG_YA_LANG);
            pFight->AddMonster(ptr,3);
        }
        {
            ShareMonsterPtr ptr;
            ptr = CreateQCFightMonster(12,EMTTongLing,53,"亲卫雄狼",
                7,7,1,0);
            ptr->AddSkill(24,100);
            ptr->AddSkill(153,100);
            ptr->AddSkill(155,100);
            ptr->SetCeLue(CL_FY_XIONG_LANG);
            pFight->AddMonster(ptr,1);
        }
        {
            ShareMonsterPtr ptr;
            ptr = CreateQCFightMonster(12,EMTTongLing,53,"亲卫雄狼",
                7,7,1,0);
            ptr->AddSkill(24,100);
            ptr->AddSkill(153,100);
            ptr->AddSkill(155,100);
            ptr->SetCeLue(CL_FY_XIONG_LANG);
            pFight->AddMonster(ptr,5);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LangBattle2(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 1;
        ShareMonsterPtr ptr = CreateQCFightMonster(23,EMTTongLing,56,"梅寒",
               30,9,9,xiang);
        ptr->AddSkill(57,90);
        ptr->AddSkill(4,90);
        ptr->AddSkill(3,90);
        ptr->SetCeLue(CL_MEI_HAN);
        ptr->tmplId = 52;
        pFight->AddMonster(ptr,3);
        
        xiang = 0;
        ShareMonsterPtr ptr1 = CreateQCFightMonster(20,EMTTongLing,56,"锋牙狼王",
               4,7,1,xiang);
        ptr1->tmplId = 101;
        ptr1->AddSkill(24,100);
        pFight->AddMonster(ptr1,7);
        
        //BeginEpisodeBattle(pFight,pUser);
        pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LangBattle3(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang;
        for(uint8 i = 1; i <= 6; i++)
        {
            xiang = Random(1,5);
            ShareMonsterPtr ptr = CreateQCFightMonster(43,EMTTongLing,56,"食肉兽",
                    8,8,8,xiang);
            ptr->AddSkill(4*(xiang-1)+3,90);
            ptr->AddSkill(4*(xiang-1)+4,90);
            pFight->AddMonster(ptr,i);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LangBattle4(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 0;
        ShareMonsterPtr ptr = CreateQCFightMonster(101,EMTTongLing,55,"锋牙狼王",
               30,9,1,xiang);
        ptr->AddSkill(24,100);
        ptr->daohang = 250000;
        ptr->mingzhong = 3000;
        ptr->SetCeLue(CL_FIRST_MIN_FANG);
        pFight->AddMonster(ptr,3);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LangBattle5(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 0;
        uint8 num = Random(5,6);
        for(uint8 i = 1; i <= num; i++)
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(101,EMTTongLing,55,"魔狼",
                   8,8,1,xiang);
            ptr->AddSkill(24,100);
            pFight->AddMonster(ptr,i);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LangBattle6(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 3;
        for(uint8 i = 1; i <= 6; i++)
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(24,EMTTongLing,60,"魔鱼怪",
                   1,1,1,xiang,2);
            ptr->tmplId = 22;
            ptr->daohang = 0xfffffff;
            ptr->SetCeLue(CL_ONLY_LANG);
            pFight->AddMonster(ptr,i);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LangBattle7(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 2;
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(34,EMTTongLing,65,"古木",
                   30,9,9,xiang);
            ptr->AddSkill(7,90);
            ptr->AddSkill(57,60);
            ptr->AddSkill(111,3000);
            ptr->daohang = 250000;
            ptr->fashubaojilv = 60;
            ptr->fanshangadd = 60;
            ptr->fanshang = 100;
            ptr->fanshangadd = 60;
            ptr->SetCeLue(CL_GU_MU);
            pFight->AddMonster(ptr,3);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LangBattle8(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 0;
        ShareMonsterPtr ptr = CreateQCFightMonster(23,EMTTongLing,65,"梅寒",
               30,10,10,xiang);
        ptr->AddSkill(20,90);
        ptr->AddSkill(57,90);
        ptr->AddSkill(24,90);
        ptr->AddSkill(61,90);
        ptr->tmplId = 52;
        pFight->AddMonster(ptr,3);
        
        //for(uint8 i = 2; i <= 3; i++)
        {
            xiang = Random(1,5);
            ShareMonsterPtr ptr1 = CreateQCFightMonster(39,EMTTongLing,56,"亲卫魔族",
                   10,9,9,xiang);
            ptr1->AddSkill(4*(xiang-1)+3,90);
            ptr1->AddSkill(4*(xiang-1)+4,90);
            ptr1->AddSkill(152,90);
            ptr1->AddSkill(111,2000);
            pFight->AddMonster(ptr1,1);
        }
        {
            xiang = Random(1,5);
            ShareMonsterPtr ptr1 = CreateQCFightMonster(39,EMTTongLing,56,"亲卫魔族",
                   10,9,9,xiang);
            ptr1->AddSkill(4*(xiang-1)+3,90);
            ptr1->AddSkill(4*(xiang-1)+4,90);
            ptr1->AddSkill(152,90);
            ptr1->AddSkill(111,2000);
            pFight->AddMonster(ptr1,5);
        }
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(20,EMTTongLing,50,"幼狼",
                           4,6,1,0);
            ptr->SetCeLue(CL_MONSTER_DIE_END);
            ptr->daohang = 180000;
            ptr->tmplId = 101;
            ptr->AddSkill(24,100);
            ptr->baojilv = 60;
            ptr->baojiadd = 60;
            ptr->mingzhong = 2000;
            pFight->AddMonster(ptr,7);
        }
        pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LangBattle9(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 0;
        uint8 r = 0;
        for(uint8 i = 1; i <= 6; i++)
        {
            r = Random(1,3);
            if(r == 1)
            {
                xiang = 5;
                ShareMonsterPtr ptr = CreateQCFightMonster(33,EMTTongLing,65,"牛头妖王",
                       9,9,8,xiang);
                ptr->AddSkill(152,100);
                ptr->AddSkill(24,100);
                pFight->AddMonster(ptr,i);
            }
            else if(r == 2)
            {
                xiang = 5;
                ShareMonsterPtr ptr = CreateQCFightMonster(36,EMTTongLing,65,"蟹将王",
                       10,9,1,xiang);
                ptr->AddSkill(11,90);
                ptr->AddSkill(12,90);
                ptr->AddSkill(61,90);
                pFight->AddMonster(ptr,i);
            }
            else
            {
                xiang = 5;
                ShareMonsterPtr ptr = CreateQCFightMonster(39,EMTTongLing,65,"骷髅王",
                       7,8,8,xiang);
                ptr->AddSkill(53,100);
                ptr->AddSkill(61,100);
                ptr->AddSkill(65,100);
                pFight->AddMonster(ptr,i);
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::LangBattle10(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 0;
        ShareMonsterPtr ptr = CreateQCFightMonster(29,EMTTongLing,70,"荒神",
               30,11,11,xiang);
        ptr->AddSkill(20,100);
        ptr->AddSkill(111,5000);
        ptr->AddSkill(24,120);
        ptr->tmplId = 55;
        //每2回合增援 蟹将王/牛头妖王/骷髅王 之一个。
        ptr->SetCeLue(CL_ZHAO_HUA_NIU);
        pFight->AddMonster(ptr,3);
        
        //只有宠物才可对它有效的伤血。人体对其造成的伤害为真实伤害的1/10
        ShareMonsterPtr ptr1 = CreateQCFightMonster(7,EMTTongLing,65,"神农鼎",
               8,9,9,xiang);
        ptr1->AddSkill(24,90);
        ptr1->AddSkill(3,90);
        ptr1->SetCeLue(CL_ONLY_PET);
        pFight->AddMonster(ptr1,5);
        
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}


/*
11.幼狼
程序触发，不需导出脚本函数
任务20，"13|num"
在隐龙洞6层行走，遇到的怪物10%概率遇到魔狼群，怪物均为 "魔狼"，图片为坐狼图片。30%概率出现[幼狼]，捕获后结束战斗则记获得1只幼狼。
怪物名：魔狼
数量：6或5
形态：坐狼
道行：180000
强度：55级首领怪，8倍血，8力。属相0（无属性），纯物理。
携带技能：
嗜血重击（24），LV100
携带属性：
爆击+300
狂暴撕裂+300
法术闪避+100
命中+2000
特点：
集中攻击我方防御最底的单位。
幼狼：
怪物名：幼狼
数量：1
形态：野狼
强度：1级首领怪，纯物理。
*/
void CScene::LangBattle11(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 0;
        for(uint8 i = 1; i <= 5; i++)
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(21,EMTTongLing,55,"魔狼",
                           8,9,1,xiang);
            ptr->tmplId = 101;
            ptr->daohang = 180000;
            ptr->fashubaojilv = 60;
            ptr->fashubaojiadd = 300;
            ptr->mingzhong = 2000;
            ptr->AddSkill(24,100);
            ptr->SetCeLue(CL_FIRST_MIN_FANG);
            pFight->AddMonster(ptr,i);
        }
        if(Random(0,100) < 30)
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(12,EMTTongLing,1,"幼狼",
                           1,1,1,xiang);
            ptr->SetCeLue(CL_YOU_LANG);
            pFight->AddMonster(ptr,6);
        }
        else
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(21,EMTTongLing,55,"魔狼",
                           8,9,1,xiang);
            ptr->tmplId = 101;
            ptr->daohang = 180000;
            ptr->fashubaojilv = 60;
            ptr->fashubaojiadd = 300;
            ptr->mingzhong = 2000;
            ptr->AddSkill(24,100);
            ptr->SetCeLue(CL_FIRST_MIN_FANG);
            pFight->AddMonster(ptr,6);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::Mission21Boss(CUser *pU,uint16 monsterId)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = Random(1,5);
        uint16 monster = 2*(pU->GetLevel() - 1)%5+1;
        if(monster == 0)
            monster = 1;
        else if(monster > 50)
            monster = 50;
        ShareMonsterPtr ptr = CreateQCFightMonster(monster,EMTTongLing,pUser->GetLevel(),"BOSS",
                           1,6,6,xiang);
        ptr->tmplId = monsterId;
        ptr->AddSkill(4*(xiang-1)+3,90);
        ptr->AddSkill(4*(xiang-1)+4,90);
        ptr->AddSkill(61,90);
        ptr->AddSkill(53,90);
        ptr->SetCeLue(CL_MISSION21_BOSS);
        pFight->AddMonster(ptr,3);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

/*
喽罗战斗
BOSS名：魔族
数量：6。
形态：牛头妖
强度：60级首领怪，8倍血，8力8灵。属相0（无属性）
携带技能：
嗜血重击（24），LV90
吸血怒击（152），LV90
携带属性：
全属性抵抗+500
连击率+300
连击数+2
连击追加+200
命中率+1000
*/
void CScene::LangBattle12(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 xiang = 0;
        for(uint8 i = 2; i <= 4; i++)
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(24,EMTTongLing,60,"魔族",
                           8,9,9,xiang);
            ptr->AddSkill(152,120);
            ptr->tmplId = 33;
            ptr->lianji = 60;
            ptr->lianjiShu = 2;
            ptr->lianjiAdd = 200;
            ptr->mingzhong = 1000;
            ptr->AddSkill(24,90);
            pFight->AddMonster(ptr,i);
        }
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(20,EMTTongLing,50,"幼狼",
                           4,6,1,0);
            ptr->SetCeLue(CL_MONSTER_DIE_END);
            ptr->daohang = 180000;
            ptr->tmplId = 101;
            ptr->AddSkill(24,100);
            ptr->baojilv = 60;
            ptr->baojiadd = 60;
            ptr->mingzhong = 2000;
            pFight->AddMonster(ptr,7);
        }  
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::EpisodeBattle1(CUser *pU)
{   
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,0,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,0,3+i);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::EpisodeBattle2(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,1,3);
        AddEpisodeBattleMonster(pFight,2,5);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::EpisodeBattle3(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,3,3);
        AddEpisodeBattleMonster(pFight,4,5);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle4(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,5,3);
        
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,5,3+i);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle5(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,6,3);
        
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,5,3+i);
                }
            }
        }
        
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle6(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,7,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,7,3+i);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle7(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,8,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,8,3+i);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle8(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,9,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,9,3+i);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle9(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,10,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,10,3+i);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::EpisodeBattle10(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,11,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,11,3+i);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle11(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddEpisodeBattleMonster(pFight,12,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,18,3+i,level - 3);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::EpisodeBattle12(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddEpisodeBattleMonster(pFight,13,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,18,3+i,level - 3);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle13(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddEpisodeBattleMonster(pFight,14,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,18,3+i,level - 3);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle14(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddEpisodeBattleMonster(pFight,15,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,18,3+i,level - 3);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle15(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddEpisodeBattleMonster(pFight,16,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,18,3+i,level - 3);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle16(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddEpisodeBattleMonster(pFight,17,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,18,3+i,level - 3);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::EpisodeBattle17(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,19,3);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::EpisodeBattle18(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,21,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,22,3+i);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::EpisodeBattle19(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,20,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,20,3+i);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::EpisodeBattle20(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,24,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,23,3+i);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

static void AddCoupleTrialMonster(CFight *pFight,uint8 level)
{
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(level,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    {
        ShareMonsterPtr ptr;
        SMonsterInst *pInst = new SMonsterInst;
        ptr.reset(pInst);
        pInst->level = level;
        pInst->tmplId = 13;
        pInst->xiang = 0;
        pInst->exp = 0; 
        pInst->name = "试炼仙子";
        pInst->liliang = 4*pInst->level;
        pInst->lingxing = 4*pInst->level;
        pInst->tizhi = pInst->level;
        pInst->naili = pInst->level;
        pInst->minjie = pInst->level;
        pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
        pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
        pInst->hp = pInst->maxHp;
        pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
        pInst->mp = pInst->maxHp;
        pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
        pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
        pInst->addSkillAttack = addSkillAttack;
        pFight->AddMonster(ptr,1);
    }
    
    {
        ShareMonsterPtr ptr;
        SMonsterInst *pInst = new SMonsterInst;
        ptr.reset(pInst);
        pInst->level = level;
        pInst->tmplId = 13;
        pInst->xiang = 0;
        pInst->exp = 0; 
        pInst->name = "试炼仙子";
        pInst->liliang = 4*pInst->level;
        pInst->lingxing = 4*pInst->level;
        pInst->tizhi = pInst->level;
        pInst->naili = pInst->level;
        pInst->minjie = pInst->level;
        pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
        pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
        pInst->hp = pInst->maxHp;
        pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
        pInst->mp = pInst->maxHp;
        pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
        pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
        pInst->addSkillAttack = addSkillAttack;
        pFight->AddMonster(ptr,3);
    }
    
    {
        ShareMonsterPtr ptr;
        SMonsterInst *pInst = new SMonsterInst;
        ptr.reset(pInst);
        pInst->level = level;
        pInst->tmplId = 13;
        pInst->xiang = 0;
        pInst->exp = 0; 
        pInst->name = "试炼仙子";
        pInst->liliang = 4*pInst->level;
        pInst->lingxing = 4*pInst->level;
        pInst->tizhi = pInst->level;
        pInst->naili = pInst->level;
        pInst->minjie = pInst->level;
        pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
        pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
        pInst->hp = pInst->maxHp;
        pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
        pInst->mp = pInst->maxMp;
        pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
        pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
        pInst->addSkillAttack = addSkillAttack;
        pFight->AddMonster(ptr,5);
    }
}

void CScene::FightCoupleTrial(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    
    pFight->SetFightType(CFight::EFTScript);
    
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 0;
        CUserTeam *pTeam = NULL;
        if(m_userTeams.Find(pU->GetTeam(),pTeam))
        {//组队，或者只有队长一人
            num = Random(pTeam->GetMemberNum(),2*pTeam->GetMemberNum());
            uint8 maxLevel = AddTeamToFight(pFight,pTeam,7);
            AddCoupleTrialMonster(pFight.get(),maxLevel);
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(pU->GetRoleId());
            if(p.get() == NULL)
                return;
            pU->SetFight(pFight->GetId(),pFight->AddUser(p,9));
            AddCoupleTrialMonster(pFight.get(),pU->GetLevel());
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::EpisodeBattle21(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        AddEpisodeBattleMonster(pFight,25,3);
        if(pU->GetTeam() != 0)
        {
            CUserTeam *pTeam = NULL;
            if(m_userTeams.Find(pU->GetTeam(),pTeam))
            {
                for(uint8 i = 1; i < pTeam->GetMemberNum(); i++)
                {
                    AddEpisodeBattleMonster(pFight,25,3+i);
                }
            }
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::AddVisibleMonster(SVisibleMonster &monster)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(monster.id == 0)
        monster.id = m_visibleMonsters.size();
    m_visibleMonsters.push_back(monster);
    CNetMessage msg;
    msg.SetType(MSG_SERVER_ADD_MONSTER);
    msg<<monster.id<<monster.x<<monster.y<<monster.face<<monster.type<<monster.pic;
    BroadcastMsg(msg);
}

void CScene::ClearVisibleMonster()
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    for(list<SVisibleMonster>::iterator i = m_visibleMonsters.begin(); 
        i != m_visibleMonsters.end(); i++)
    {
        CNetMessage msg;
        msg.SetType(MSG_SERVER_REMOVE_MONSTER);
        msg<<i->id;
        BroadcastMsg(msg);
    }
    m_visibleMonsters.clear();
}


void CScene::NianShouBattle(ShareUserPtr pUser)
{
    uint8 maxLevel = 0;
    if(pUser->GetTeam() != 0)
    {
        uint32 members[MAX_TEAM_MEMBER];
        uint8 num = 0;
        CUserTeam *pTeam = NULL;
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam))
            return;
        pTeam->GetMember(members,num);
        for(uint8 i = 0; i < num; i++)
        {
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(members[i]);
            if(p.get() != NULL)
            {
                if(p->GetLevel() > maxLevel)
                {
                    maxLevel = p->GetLevel();
                }
                p->SetHuoDongFightTime(GetSysTime());
            }
        }
        if(maxLevel < 20)
            return;
    }
    else
    {
        maxLevel = pUser->GetLevel();
        if(maxLevel < 20)
            return;
        pUser->SetHuoDongFightTime(GetSysTime());
    }
    
    ShareFightPtr pFight = m_fightManager.CreateFight();
    uint16 monsterId = 2*((maxLevel-1)/5+1);
    SMonsterTmpl *pTmpl = m_monsterManager.GetTmpl(monsterId);
    if(pTmpl == NULL)
    {
        return;
    }
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        pFight->SetFightType(CFight::EFTNianShou);
        
        CUserTeam *pTeam = NULL;
        if((pUser->GetTeam() != 0) && (m_userTeams.Find(pUser->GetTeam(),pTeam)))
        {
            uint8 num = pTeam->GetMemberNum();
            //num = Random(num,2*num);
            for(uint8 i = 0; i < num; i++)
            {
                SMonsterInst *pInst = new SMonsterInst;
                pInst->type = EMTNormal;
                pInst->tmplId = 52;
                pInst->xiang = Random(1,5);
                pInst->name = "百花仙子";
                pInst->level = maxLevel;
                pInst->exp = 0;                
                pInst->liliang = 3*maxLevel;
                pInst->lingxing = 3*maxLevel;
                pInst->tizhi = maxLevel;
                pInst->naili = maxLevel;
                pInst->minjie = maxLevel;
                ShareMonsterPtr pShareMonster(pInst);// = ShareFightPtr(pInst);
                pShareMonster->Init(pTmpl);
                pInst->maxHp *= 20;
                pInst->hp = pInst->maxHp;
                pFight->AddMonster(pShareMonster,i+1);
            }
            AddTeamToFight(pFight,pTeam,7);
        }
        else
        {
            for(uint8 i = 0; i < 1; i++)
            {
                SMonsterInst *pInst = new SMonsterInst;
                pInst->type = EMTNormal;
                pInst->tmplId = 52;
                pInst->xiang = Random(1,5);
                pInst->name = "百花仙子";
                pInst->level = maxLevel;
                pInst->exp = 0;                
                pInst->liliang = 3*maxLevel;
                pInst->lingxing = 3*maxLevel;
                pInst->tizhi = maxLevel;
                pInst->naili = maxLevel;
                pInst->minjie = maxLevel;
                ShareMonsterPtr pShareMonster(pInst);// = ShareFightPtr(pInst);
                pShareMonster->Init(pTmpl);
                pInst->maxHp *= 20;
                pInst->hp = pInst->maxHp;
                pFight->AddMonster(pShareMonster,3+i);
            }    
	        pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        }
        
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void InitGuiYuMonster(ShareMonsterPtr &ptr,uint8 monsterLevel,uint8 level)
{
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = EMTNormal;
    
    pInst->level = monsterLevel;
    pInst->exp = 0;
    pInst->tizhi = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(monsterLevel,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    uint8 r = Random(0,3);
    pInst->xiang = 0;
    if(r == 0)
    {
        pInst->tmplId = 32;
        pInst->name = "恶鬼";
        if(level == 1)
        {
            pInst->liliang = 3*pInst->level;
            pInst->lingxing = 3*pInst->level;
            pInst->naili = pInst->level;
            pInst->minjie = pInst->level;
        }
        else if(level == 2)
        {
            pInst->liliang = 4*pInst->level;
            pInst->lingxing = 4*pInst->level;
            pInst->naili = 2*pInst->level;
            pInst->minjie = pInst->level;
        }
        else if(level == 3)
        {
            pInst->liliang = 5*pInst->level;
            pInst->lingxing = 5*pInst->level;
            pInst->naili = 3*pInst->level;
            pInst->minjie = 2*pInst->level;
        }
    }
    else if(r == 1)
    {
        pInst->tmplId = 29;
        pInst->name = "厉鬼";
        if(level == 1)
        {
            pInst->liliang = 4*pInst->level;
            pInst->lingxing = pInst->level;
            pInst->naili = pInst->level;
            pInst->minjie = pInst->level;
        }
        else if(level == 2)
        {
            pInst->liliang = 5*pInst->level;
            pInst->lingxing = pInst->level;
            pInst->naili = 2*pInst->level;
            pInst->minjie = 2*pInst->level;
        }
        else if(level == 3)
        {
            pInst->liliang = 6*pInst->level;
            pInst->lingxing = pInst->level;
            pInst->naili = 2*pInst->level;
            pInst->minjie = 3*pInst->level;
            pInst->AddSkill(119,200);
        }
    }
    else if(r == 2)
    {
        pInst->tmplId = 35;
        pInst->name = "无常鬼";
        pInst->xiang = Random(0,5);
        if(level == 1)
        {
            pInst->liliang = 2*pInst->level;
            pInst->lingxing = 3*pInst->level;
            pInst->naili = pInst->level;
            pInst->minjie = pInst->level;
        }
        else if(level == 2)
        {
            pInst->liliang = 3*pInst->level;
            pInst->lingxing = 4*pInst->level;
            pInst->naili = 2*pInst->level;
            pInst->minjie = 2*pInst->level;
        }
        else if(level == 3)
        {
            pInst->liliang = 3*pInst->level;
            pInst->lingxing = 5*pInst->level;
            pInst->naili = 2*pInst->level;
            pInst->minjie = 3*pInst->level;
        }
        pInst->daohang /= 4;
        pInst->AddSkill(53,90);
        pInst->AddSkill(57,90);
        pInst->AddSkill(61,90);
        pInst->AddSkill(65,90);
        pInst->AddSkill(20,90);
    }
    else
    {
        pInst->tmplId = 30;
        pInst->name = "女鬼";
        pInst->xiang = Random(0,5);
        if(level == 1)
        {
            pInst->liliang = pInst->level;
            pInst->lingxing = 4*pInst->level;
            pInst->naili = 2*pInst->level;
            pInst->minjie = 2*pInst->level;
        }
        else if(level == 2)
        {
            pInst->liliang = pInst->level;
            pInst->lingxing = 5*pInst->level;
            pInst->naili = 2*pInst->level;
            pInst->minjie = 2*pInst->level;
        }
        else if(level == 3)
        {
            pInst->liliang = pInst->level;
            pInst->lingxing = 6*pInst->level;
            pInst->naili = 2*pInst->level;
            pInst->minjie = 3*pInst->level;
        }
        pInst->AddSkill(3,monsterLevel);
        pInst->AddSkill(8,monsterLevel);
        pInst->AddSkill(16,monsterLevel);
        pInst->AddSkill(12,monsterLevel);
        pInst->AddSkill(20,monsterLevel);
    }
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;
    pInst->maxHp *= 3;//4;
    ptr->hp = ptr->maxHp;
}

void CScene::AddGuYuFightMonster(ShareFightPtr pFight,uint8 num,uint8 monsterLevel)
{
    int level = m_mapId - GUIYU_SCENE_ID + 1;
    if((level < 0) || (level > 3))
        level = 3;
    for(uint8 i = 0; i < num; i++)
    {
        ShareMonsterPtr ptr;
        InitGuiYuMonster(ptr,monsterLevel,level);
        pFight->AddMonster(ptr,1+i);
    }
}

void CScene::YeZhuFight(ShareUserPtr pUser,uint16 monsterId)
{
    if(pUser->GetFightId() != 0)
        return;
    SVisibleMonster vMonster;
    if(!FindVisibleMonster(monsterId,vMonster,1))
        return;
    if(vMonster.flag != 0)
    {
        SendPopMsg(pUser.get(),"有人正在战斗中");
        return;
    }
    
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {       
        return;
    }
    
    pFight->SetVisibleMonsterId(monsterId);
    pFight->SetFightType(CFight::EFVisibleMonsterMis);
    pFight->SetJLType(EFJLType6);
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    {
        CUserTeam *pTeam = NULL;
        SMonsterInst *pMonster = NULL;
        AddEpisodeBattleMonster(pFight,4,3,15,&pMonster);
        if(pMonster != NULL)
        {
            pMonster->maxHp = pMonster->maxHp*2/3;
            pMonster->hp = pMonster->maxHp;
            pMonster->recovery = pMonster->recovery*2/3;
            pMonster->attack = pMonster->attack*2/3;
            pMonster->chatMsg = "哼唧...哼唧...";
            pMonster->exp = 60;
        }
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(6,EMTTongLing,15,"野猪",
                    1,2,2,0);
            ptr->type = EMTNormal;
            pFight->AddMonster(ptr,1);
            ptr->exp = 70;
        }
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(6,EMTTongLing,15,"野猪",
                    1,2,2,0);
            ptr->type = EMTNormal;
            pFight->AddMonster(ptr,5);
            ptr->exp = 70;
        }
        
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {//组队，或者只有队长一人
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::QianNianJiangShiFight(ShareUserPtr pUser,uint16 monsterId)
{
    if(pUser->GetFightId() != 0)
        return;
    SVisibleMonster vMonster;
    if(!FindVisibleMonster(monsterId,vMonster,1))
        return;
    if(vMonster.flag != 0)
    {
        SendPopMsg(pUser.get(),"有人正在战斗中");
        return;
    }
    
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {       
        return;
    }
    
    pFight->SetVisibleMonsterId(monsterId);
    pFight->SetFightType(CFight::EFVisibleMonsterMis);
    pFight->SetJLType(EFJLType7);
    
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    {
        CUserTeam *pTeam = NULL;
        
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(10,EMTTongLing,25,"千年僵尸",
                    5,5,5,0);
            pFight->AddMonster(ptr,3);
            ptr->chatMsg = "成为我的奴仆吧！";
            /*ptr->maxHp = ptr->maxHp*2/3;
            ptr->hp = ptr->maxHp;
            ptr->recovery = ptr->recovery*2/3;
            ptr->attack = ptr->attack*2/3;*/
            ptr->type = EMTNormal;
        }
        //pFight->AddMonster(ptr,1);
        for(uint8 i = 1; i <= 6; i++)
        {
            if(i == 3)
                continue;
            ShareMonsterPtr ptr = CreateQCFightMonster(10,EMTTongLing,25,"僵尸",
                    1,2,2,0);
            ptr->type = EMTNormal;
            pFight->AddMonster(ptr,i);
        }
        
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {//组队，或者只有队长一人
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

bool CScene::YeWaiShiYao(CUser *pUser)
{
    int shiYaoSpace = GetSysTime() - pUser->m_yewaiShiYao;
    if(shiYaoSpace < 1800)
        return false;
        
    uint16 maps[] = {2,3,41,45,51,54,55,56,57,69,71,72,85,86,87,88,89,90,91,92,94,112,113,123,124,125,126,157};
    for(uint8 i = 0; i < sizeof(maps)/sizeof(maps[0]); i++)
    {
        if(m_mapId == maps[i])
            return false;
    }
    uint8 r = Random(0,100);
    //if(r <= 2)//2
    if(r <= 2)//2
    {
        pUser->m_yewaiShiYao = GetSysTime();
        CNetMessage msg;
        msg.SetType(MSG_SERVER_PROMPT);
        msg<<"遇到了肆虐的十恶妖，是否进行挑战？";
        m_socketServer.SendMsg(pUser->GetSock(),msg);
        return true;
    }
    return false;
}

void CScene::ShiYaoFight(CUser *pUser,uint16 monsterId)
{
    if(m_state == 1)
    {
        switch(monsterId)
        {
        case 1:
            ShiYaoPT8(pUser,CFight::EFTGuiYu);
            break;
        case 2:
            ShiYaoPT2(pUser,CFight::EFTGuiYu);
            break;
        case 3:
            ShiYaoPT3(pUser,CFight::EFTGuiYu);
            break;
        case 4:
            ShiYaoPT4(pUser,CFight::EFTGuiYu);
            break;
        case 5:
            ShiYaoPT5(pUser,CFight::EFTGuiYu);
            break;
        case 6:
            ShiYaoPT6(pUser,CFight::EFTGuiYu);
            break;
        case 7:
            ShiYaoPT7(pUser,CFight::EFTGuiYu);
            break;
        case 8:
            ShiYaoPT1(pUser,CFight::EFTGuiYu);
            break;
        case 9:
            ShiYaoPT9(pUser,CFight::EFTGuiYu);
            break;
        case 10:
            ShiYaoPT10(pUser,CFight::EFTGuiYu);
            break;
        }
    }
    else if(m_state == 2)
    {
        switch(monsterId)
        {
        case 1:
            ShiYaoYX8(pUser,CFight::EFTGuiYu);
            break;
        case 2:
            ShiYaoYX2(pUser,CFight::EFTGuiYu);
            break;
        case 3:
            ShiYaoYX3(pUser,CFight::EFTGuiYu);
            break;
        case 4:
            ShiYaoYX4(pUser,CFight::EFTGuiYu);
            break;
        case 5:
            ShiYaoYX5(pUser,CFight::EFTGuiYu);
            break;
        case 6:
            ShiYaoYX6(pUser,CFight::EFTGuiYu);
            break;
        case 7:
            ShiYaoYX7(pUser,CFight::EFTGuiYu);
            break;
        case 8:
            ShiYaoYX1(pUser,CFight::EFTGuiYu);
            break;
        case 9:
            ShiYaoYX9(pUser,CFight::EFTGuiYu);
            break;
        case 10:
            ShiYaoYX10(pUser,CFight::EFTGuiYu);
            break;
        }
    }
}

void CScene::ThreeSister(ShareUserPtr pUser,uint16 monsterId)
{
    if(pUser->GetFightId() != 0)
        return;
    SVisibleMonster vMonster;
    if(!FindVisibleMonster(monsterId,vMonster,1))
        return;
    if(vMonster.flag != 0)
    {
        SendPopMsg(pUser.get(),"有人正在战斗中");
        return;
    }
    
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {       
        return;
    }
    
    pFight->SetVisibleMonsterId(monsterId);
    pFight->SetFightType(CFight::EFVisibleMonsterMis);
    pFight->SetJLType(EFJLType8);
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    {
        CUserTeam *pTeam = NULL;
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(14,EMTTongLing,29,"大姐",
                    5,5,5,0);
            ptr->chatMsg = "我们不是好惹的。";
            ptr->type = EMTNormal;
            pFight->AddMonster(ptr,1);
        }
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(14,EMTTongLing,29,"二姐",
                    5,5,5,0);
            ptr->type = EMTNormal;
            pFight->AddMonster(ptr,3);
        }
        {
            ShareMonsterPtr ptr = CreateQCFightMonster(14,EMTTongLing,29,"三姐",
                    5,5,5,0);
            ptr->type = EMTNormal;
            pFight->AddMonster(ptr,5);
        }
        
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {//组队，或者只有队长一人
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::GuiYuFight(ShareUserPtr pUser,uint16 monsterId)
{
    if(pUser->GetFightId() != 0)
        return;
    SVisibleMonster vMonster;
    if(!FindVisibleMonster(monsterId,vMonster,1))
        return;
    if(vMonster.flag != 0)
    {
        SendPopMsg(pUser.get(),"有人正在战斗中");
        return;
    }
    
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    
    pFight->SetVisibleMonsterId(monsterId);
    //pFight->AddMonster(ptr,3);
    pFight->SetFightType(CFight::EFTGuiYu);
    
    if(m_mapId == LAN_RUO_DI_GONG_ID)
    {
        pFight->SetJLType(EFJLType1);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 0;
        CUserTeam *pTeam = NULL;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {//组队，或者只有队长一人
            num = 6;//Random(pTeam->GetMemberNum(),2*pTeam->GetMemberNum());
            uint8 maxLevel = 75;
            AddTeamToFight(pFight,pTeam,7);
            AddGuYuFightMonster(pFight,num,maxLevel);
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            num = 6;//Random(1,2);
            AddGuYuFightMonster(pFight,num,75);
            //AddTeamToFight(pFight,pTeam,7);
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
    }
    else
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 0;
        CUserTeam *pTeam = NULL;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {//组队，或者只有队长一人
            num = Random(pTeam->GetMemberNum(),2*pTeam->GetMemberNum());
            uint8 maxLevel = AddTeamToFight(pFight,pTeam,7);
            AddGuYuFightMonster(pFight,num,maxLevel);
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            num = Random(1,2);
            AddGuYuFightMonster(pFight,num,pUser->GetLevel());
            //AddTeamToFight(pFight,pTeam,7);
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

void CScene::NianShouBattle(CUser *pUser,uint16 monsterId)
{
    /*if(pUser->GetLevel() >= 40)
    {
        SendPopMsg(pUser,"你的等级过高无法参与此活动");
        return;
    }*/
    
    SVisibleMonster vMonster;
    if(!FindVisibleMonster(monsterId,vMonster,1))
        return;
    if(vMonster.flag != 0)
    {
        SendPopMsg(pUser,"有人正在参与活动");
        return;
    }
    ShareMonsterPtr ptr;
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = monsterId;
    if(monsterId == 31)
        pInst->name = "虾兵";
    else
        pInst->name = "蟹将";
    pInst->xiang = 0;
    pInst->level = 30;
    pInst->exp = 0;
    pInst->daohang = monsterId;
    pInst->maxHp = 40;
    pInst->recovery = 0xfffffff;
    pInst->hp = pInst->maxHp;
    pInst->maxMp = 30;
    pInst->mp = pInst->maxHp;
    pInst->speed = 10000;
    pInst->attack = 300;
    pInst->addSkillAttack = 0;
    
    pInst->tizhi = 0;//体质
    pInst->liliang = 0;//力量
    pInst->minjie = 0;//敏捷
    pInst->lingxing = 0;//灵性    
    pInst->naili = 0;//耐力    
    pInst->noAdd = true;
    
    ShareFightPtr pFight = m_fightManager.CreateFight();
    
    if(pFight.get() == NULL)
    {
        return;
    }
    pFight->AddMonster(ptr,3);
    pFight->SetFightType(CFight::EFGuiJie);
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 0;
        CUserTeam *pTeam = NULL;
        if(!m_userTeams.Find(pUser->GetTeam(),pTeam)
            || (pTeam->GetMemberNum() == 1))
        {//组队，或者只有队长一人
            if(pUser->IsMaxMonster())
                num = 2;
            else
                num = Random(1,2);
            ShareUserPtr p = m_onlineUser.GetUserByRoleId(pUser->GetRoleId());
            if(p.get() == NULL)
                return;
            pUser->SetFight(pFight->GetId(),pFight->AddUser(p,9));
            pFight->BeginFight(m_socketServer,this);
        }
        else
        {
            AddTeamToFight(pFight,pTeam,7);
            pFight->BeginFight(m_socketServer,this);
        }
    }
    m_fightManager.AddFight(pFight);
}

bool CScene::FindVisibleMonster(uint16 id,SVisibleMonster &monster,uint8 flag)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_visibleMonsters.size() <= 0)
        return false;
    for(list<SVisibleMonster>::iterator i = m_visibleMonsters.begin(); 
        i != m_visibleMonsters.end(); i++)
    {
        if(i->id == id)
        {
            monster = *i;
            i->flag = flag;
            return true;
        }
    }
    return false;
}

void CScene::DelVisibleMonster(uint16 id)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(m_visibleMonsters.size() <= 0)
        return;
    for(list<SVisibleMonster>::iterator i = m_visibleMonsters.begin(); 
        i != m_visibleMonsters.end(); i++)
    {
        if(i->id == id)
        {
            m_visibleMonsters.erase(i);
            CNetMessage msg;
            msg.SetType(MSG_SERVER_REMOVE_MONSTER);
            msg<<id;
            BroadcastMsg(msg);
            return;
        }
    }
}

bool CScene::GetCanWalkPos(uint8 &x,uint8 &y)
{
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    if(m_canWalkPos.empty())
        return false;
    uint32 walkPosNum = Random(0,m_canWalkPos.size()-1);
    if(walkPosNum >= m_canWalkPos.size())
        return false;
    SPoint pos = m_canWalkPos[walkPosNum];
    x = pos.x;
    y = pos.y;
    return true;
}

void CScene::AddJumpPoint(uint8 x,uint8 y,uint8 toX,uint8 toY,uint16 sceneId)
{
    m_addJump = true;
    /*
    SJumpTo *pJump = new SJumpTo;
    pJump->sceneId = sceneId;
    pJump->x = toX;
    pJump->y = toY;
    pJump->face = 8;
    InsertJumpPoint(x,y,pJump);
    */
    m_jumpPoint.x = x;
    m_jumpPoint.y = y;
    m_jumpToPoint.x = toX;
    m_jumpToPoint.y = toY;
    m_jumpToPoint.face = 8;
    m_jumpToPoint.sceneId = sceneId;
    
    CNetMessage msg;
    msg.SetType(MSG_SERVER_JUMP_POINT);
    msg<<(uint8)0<<x<<y;
    BroadcastMsg(msg);
}

void CScene::AddShiYao(bool killMonster)
{
    if(m_mapId != 306)
    {
        return;
    }
    if(m_killMonsterNum >= 10)
    {
        COnlineUser &onlineUser = SingletonOnlineUser::instance();
        for(list<uint32>::iterator i = m_userList.begin(); i != m_userList.end(); i++)
        {
            ShareUserPtr ptr = onlineUser.GetUserByRoleId(*i);
            CUser *pUser = ptr.get();
            if(pUser != NULL)
            {
                if((pUser->GetData32(11) != 0) && (GetSysTime()-pUser->GetData32(11) > 0))
                {
                    if(m_state == 2)
                    {
                        pUser->SetData16(8,GetSysTime()-pUser->GetData32(11));
                        pUser->AddTitle(EUT9);
                    }
                }
                if(m_state == 1)
                {
                    char buf[128];
                    snprintf(buf,sizeof(buf),"恭喜【%s】成功挑战普通难度全部十妖",pUser->GetName());
                    SysInfoToAllUser(buf);
                }
            }
        }
        return;
    }
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    if(!m_visibleMonsters.empty() && !killMonster)
        return;
        
    if(m_canWalkPos.empty())
        return;
    uint32 walkPosNum = m_canWalkPos.size();
        
    uint32 canWalkPos = Random(0,m_canWalkPos.size()-1);
    if(canWalkPos >= walkPosNum)
        return;
    m_killMonsterNum++;
    SPoint pos = m_canWalkPos[canWalkPos];
    
    SVisibleMonster monster;
    monster.id = m_killMonsterNum;//m_visibleMonsters.size();
    m_curVisibleId = m_killMonsterNum;
    monster.x = pos.x;
    monster.y = pos.y;
    monster.face = 4;
    monster.type = 2;
    switch(m_killMonsterNum)
    {
    case 1:
        monster.pic = 39;
        break;
    case 2:
        monster.pic = 24;
        break;
    case 3:
        monster.pic = 41;//44;
        break;
    case 4:
        monster.pic = 45;
        break;
    case 5:
        monster.pic = 46;
        break;
    case 6:
        monster.pic = 34;
        break;
    case 7:
        monster.pic = 29;
        break;
    case 8:
        monster.pic = 14;
        break;
    case 9:
        monster.pic = 10;
        break;
    case 10:
        monster.pic = 44;//41;
        break;
    }
    monster.flag = 0 ;
    m_visibleMonsters.push_back(monster);
    
    if(!m_userList.empty())
    {
        CNetMessage msg;
        msg.SetType(MSG_SERVER_ADD_MONSTER);
        msg<<monster.id<<monster.x<<monster.y<<monster.face<<monster.type<<monster.pic;
        BroadcastMsg(msg);
    }
}

void CScene::AddLanRuoMonster(bool killMonster)
{
    if(m_mapId != LAN_RUO_DI_GONG_ID)
        return;
    if(killMonster)
    {
        m_killMonsterNum++;
        if(m_killMonsterNum >= 30)
        {
            uint16 r = Random(0,m_canWalkPos.size()-1);
            if(r >= m_canWalkPos.size())
                return;
            SPoint point = {14,14};
            InitNpcPoint(&point,1);
            AddNpc(350,true);
        }
    }
    if(m_killMonsterNum > 30-LANRUO_MONSTER_NUM)
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    if(m_canWalkPos.empty())
        return;
    uint32 walkPosNum = m_canWalkPos.size();
    int count = 0;
    int num = LANRUO_MONSTER_NUM - m_visibleMonsters.size();
    if(num <= 0)
        return;
        
    for(uint32 i = 0; i < walkPosNum; i++)
    {
        uint32 canWalkPos = Random(0,m_canWalkPos.size()-1);
        if(canWalkPos >= walkPosNum)
            return;
        SPoint pos = m_canWalkPos[canWalkPos];
        bool flag = true;//判断是否重复
        for(list<SVisibleMonster>::iterator iter = m_visibleMonsters.begin(); 
            iter != m_visibleMonsters.end(); iter++)
        {
            if((pos.x == iter->x) && (pos.y == iter->y))
            {
                flag = false;
                break;
            }
        }
        if(flag)
        {
            SVisibleMonster monster;
            m_curVisibleId++;
            if(m_curVisibleId >= 0xffff)
                m_curVisibleId = 0;
            monster.id = m_curVisibleId;//m_visibleMonsters.size();
            monster.x = pos.x;
            monster.y = pos.y;
            monster.face = 4;
            monster.type = 2;
            uint8 r = Random(0,3);
            if(r == 0)
                monster.pic = 29;
            else if(r == 1)
                monster.pic = 30;
            else if(r == 1)
                monster.pic = 32;
            else
                monster.pic = 35;
            monster.flag = 0 ;
            m_visibleMonsters.push_back(monster);
            //cout<<"可见怪数量:"<<m_visibleMonsters.size()<<endl;
            if(!m_userList.empty())
            {
                CNetMessage msg;
                msg.SetType(MSG_SERVER_ADD_MONSTER);
                msg<<monster.id<<monster.x<<monster.y<<monster.face<<monster.type<<monster.pic;
                BroadcastMsg(msg);
            }
            count++;
            if(count >= num)
                break;
        }
    }    
}

void CScene::AddGuiYuMonster()
{
    if((m_mapId < GUIYU_SCENE_ID) || (m_mapId > GUIYU_SCENE_ID + MAX_GUI_YU_LAYER))
        return;
    boost::recursive_mutex::scoped_lock lk(m_mutex);
    
    if(m_canWalkPos.empty())
        return;
    uint32 walkPosNum = m_canWalkPos.size();
    int count = 0;
    int num = GUI_YU_MAX_MONSTER_NUM - m_visibleMonsters.size();
    if(num <= 0)
        return;
        
    for(uint32 i = 0; i < walkPosNum; i++)
    {
        uint32 canWalkPos = Random(0,m_canWalkPos.size()-1);
        if(canWalkPos >= walkPosNum)
            return;
        SPoint pos = m_canWalkPos[canWalkPos];
        bool flag = true;//判断是否重复
        for(list<SVisibleMonster>::iterator iter = m_visibleMonsters.begin(); 
            iter != m_visibleMonsters.end(); iter++)
        {
            if((pos.x == iter->x) && (pos.y == iter->y))
            {
                flag = false;
                break;
            }
        }
        if(flag)
        {
            SVisibleMonster monster;
            m_curVisibleId++;
            if(m_curVisibleId >= 0xffff)
                m_curVisibleId = 0;
            monster.id = m_curVisibleId;//m_visibleMonsters.size();
            monster.x = pos.x;
            monster.y = pos.y;
            monster.face = 4;
            monster.type = 2;
            uint8 r = Random(0,3);
            if(r == 0)
                monster.pic = 29;
            else if(r == 1)
                monster.pic = 30;
            else if(r == 1)
                monster.pic = 32;
            else
                monster.pic = 35;
            monster.flag = 0;
            m_visibleMonsters.push_back(monster);
            //cout<<"可见怪数量:"<<m_visibleMonsters.size()<<endl;
            if(!m_userList.empty())
            {
                CNetMessage msg;
                msg.SetType(MSG_SERVER_ADD_MONSTER);
                msg<<monster.id<<monster.x<<monster.y<<monster.face<<monster.type<<monster.pic;
                BroadcastMsg(msg);
            }
            count++;
            if(count >= num)
                break;
        }
    }    
}

/*void CScene::ThieveObject(CUser *pUser,CNetMessage &msg)
{
    if(pUser->GetBangPai() == 0)
    {
        msg<<PRO_ERROR<<"没有帮派不能偷";
        return;
    }
}*/

//获得帮派场景
CScene *CSceneManager::GetBangPaiScene(int id,int bangPaiId)
{
    boost::mutex::scoped_lock lk(m_bpMutex);
    CScene *pScene = NULL;
    int bpSceneId = (bangPaiId<<16)|id;
    if(m_bangPaiScene.Find(bpSceneId,pScene))
        return pScene;
    
    m_sceneList.Find(id,pScene);
    if(pScene == NULL)
        return NULL;
    CScene *pBpScene = new CScene(*pScene);
    pBpScene->SetId(bpSceneId);
    m_bangPaiScene.Insert(pBpScene->GetId(),pBpScene);
    return pBpScene;
}

CScene *CSceneManager::FindMarryHall(int id)
{
    boost::mutex::scoped_lock lk(m_bpMutex);
    for(list<CScene*>::iterator i = m_marryHall.begin(); i != m_marryHall.end(); i++)
    {
        if((*i)->GetId() == id)
        {
            return *i;
        }
    }
    return NULL;
}

CScene *CSceneManager::GetMarryHall(int id)
{
    boost::mutex::scoped_lock lk(m_bpMutex);
    for(list<CScene*>::iterator i = m_marryHall.begin(); i != m_marryHall.end(); i++)
    {
        if((*i)->GetId() == id)
        {
            return *i;
        }
    }
    CScene *pScene = NULL;
    m_sceneList.Find(250,pScene);
    if(pScene == NULL)
        return NULL;
    CScene *pMarryScene = new CScene(*pScene);
    pMarryScene->SetId(id);
    m_marryHall.push_back(pMarryScene);
    return pMarryScene;
}

bool CSceneManager::FindMapGroupScene(int id,CScene *pScene,int mapId,int groupId,
                    CScene **ppScene)
{
    if((pScene->GetMapId() == mapId)
        && (pScene->GetGroupId() == groupId))
    {
        *ppScene = pScene;
        return false;
    }
    return true;
}

bool CSceneManager::FindGroupScene(int id,CScene *pScene,int groupId,list<int> *sceneList)
{
    if(pScene->GetGroupId() == groupId)
    {
        sceneList->push_back(id);
    }
    return true;
}

void CSceneManager::GetGroupScene(int groupId,list<int> &sceneList)
{
    boost::mutex::scoped_lock lk(m_bpMutex);
    //boost::bind(EachJumpPoint,_1,_2,&m_pathInfo,&num)
    m_sceneList.ForEach(boost::bind(&CSceneManager::FindGroupScene,
                                    this,_1,_2,groupId,&sceneList));
}

CScene *CSceneManager::FindScene(int mapId,int groupId)
{
    CScene *pScene = NULL;
    boost::mutex::scoped_lock lk(m_bpMutex);
    //boost::bind(EachJumpPoint,_1,_2,&m_pathInfo,&num)
    m_sceneList.ForEach(boost::bind(&CSceneManager::FindMapGroupScene,
                                    this,_1,_2,mapId,groupId,&pScene));
    return pScene;
}

//帮战场景260-265,帮战场景id帮战id+1000+帮id
CScene *CSceneManager::GetBangZhanScene(int bId,CScene **ppScene)
{
    boost::mutex::scoped_lock lk(m_bpMutex);
    CScene *pScene = NULL;
    if(m_sceneList.Find(bId,pScene))
    {
        m_sceneList.Find(bId+1,*ppScene);
        return pScene;
    }
    CScene *retS = NULL;
    for(int i = 0; i <= 6; i++)
    {
        m_sceneList.Find(i+260,pScene);
        if(pScene == NULL)
            return NULL;
        CScene *pBpScene = new CScene(*pScene);
        //pBpScene->SetId(bangPaiId);
        pBpScene->SetId(bId+i);
        pBpScene->SetGroupId(bId);
        m_sceneList.Insert(bId+i,pBpScene);
        if(i == 0)
            retS = pBpScene;
        else if(i == 1)
            *ppScene = pBpScene;
    }
    return retS;
}

//删除帮派场景
void CSceneManager::DelBangPaiScene(int bangPaiId)
{
    boost::mutex::scoped_lock lk(m_bpMutex);
    CScene *pScene = NULL;
    m_bangPaiScene.Erase(bangPaiId,pScene);
    delete pScene;
}

//兰若地宫（场景id LAN_RUO_DI_GONG_ID）
CScene *CSceneManager::GetLanRuoDiGong()
{
    CScene *pScene = NULL;
    boost::mutex::scoped_lock lk(m_bpMutex);
    /*if(m_gcLanRuo.size() >= 2)
    {
        list<CScene*>::iterator i = m_gcLanRuo.begin();
        pScene = *i;
        i++;
        CScene *pDiGong = *i;
        pScene->SetId(m_curFuBenId++);
        pScene->SetGroupId(pScene->GetId());
        pDiGong->SetId(m_curFuBenId++);
        pDiGong->SetGroupId(pScene->GetId());
        m_sceneList.Insert(pDiGong->GetId(),pDiGong);
        m_fuBenScene.push_back(pDiGong);
        pDiGong->m_usedFuBen = true;
        pScene->m_usedFuBen = true;
        pDiGong->m_startTime = GetSysTime();
        pScene->m_startTime = GetSysTime();
        m_sceneList.Insert(pDiGong->GetId(),pDiGong);
        m_sceneList.Insert(pScene->GetId(),pScene);
        return pScene;
    }*/
        
    m_sceneList.Find(LAN_RUO_DI_GONG_ID,pScene);
    if(pScene == NULL)
        return NULL;

    pScene = new CScene(*pScene);
    pScene->SetId(m_curFuBenId++);
    pScene->SetMapId(LAN_RUO_DI_GONG_ID);
    
    CScene *pDiGong = NULL;
    m_sceneList.Find(304,pDiGong);
    if(pDiGong != NULL)
    {
        pDiGong = new CScene(*pDiGong);
        pScene->SetGroupId(pScene->GetId());
        pDiGong->SetId(m_curFuBenId++);
        pDiGong->SetGroupId(pScene->GetId());
        pDiGong->m_usedFuBen = true;
        pDiGong->m_startTime = GetSysTime();
        pScene->m_startTime = GetSysTime();
        m_sceneList.Insert(pDiGong->GetId(),pDiGong);
        m_sceneList.Insert(pScene->GetId(),pScene);
        pScene->m_usedFuBen = true;
        //m_fuBenScene.push_back(pDiGong);
    }
    return pScene;
}

CScene *CSceneManager::GetFuBen(int sceneId)
{
    CScene *pScene = NULL;
    boost::mutex::scoped_lock lk(m_bpMutex);
    for(list<CScene*>::iterator i = m_fuBenScene.begin(); i != m_fuBenScene.end(); i++)
    {
        if(((*i)->GetMapId() == sceneId) && (!(*i)->m_usedFuBen))
        {
            pScene = *i;
            pScene->m_usedFuBen = true;
            return pScene;
        }
    }
    
    m_sceneList.Find(sceneId,pScene);
    if(pScene == NULL)
        return NULL;

    pScene = new CScene(*pScene);
    pScene->SetId(m_curFuBenId++);
    pScene->SetMapId(sceneId);
    m_sceneList.Insert(pScene->GetId(),pScene);
    m_fuBenScene.push_back(pScene);
    pScene->m_usedFuBen = true;
    
    return pScene;
}
//鬼域副本,分1、2、3层
CScene *CSceneManager::GetGuiYu(int level)
{
    if((level <= 0) || (level > MAX_GUI_YU_LAYER))
        return NULL;

    uint16 sceneId = GUIYU_SCENE_ID + level -1;
    CScene *pScene = NULL;
    {
        boost::mutex::scoped_lock lk(m_bpMutex);    
        for(list<CScene*>::iterator i = m_fuBenScene.begin(); i != m_fuBenScene.end(); i++)
        {
            if(((*i)->GetMapId() == sceneId) && ((*i)->GetUserNum() < MAX_GUI_YU_NUM))
            {
                pScene = *i;
                return pScene;
            }
        }
        m_sceneList.Find(sceneId,pScene);
        if(pScene == NULL)
            return NULL;
        if(pScene->GetUserNum() < MAX_GUI_YU_NUM)
            return pScene;
        pScene = new CScene(*pScene);
        pScene->SetId(m_curFuBenId++);
        pScene->SetMapId(sceneId);
        m_sceneList.Insert(pScene->GetId(),pScene);
        m_fuBenScene.push_back(pScene);
    }
    return pScene;
}

void CScene::WenTianBattle(CUser *pU)
{
    if(pU == NULL)
        return;
    
    SMonsterInst *pInst = new SMonsterInst;
    ShareMonsterPtr ptr(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 9;
    pInst->name = "打手";
    pInst->xiang = 0;
    pInst->level = pU->GetLevel();
    pInst->exp = 0;
    pInst->liliang = 4*pInst->level;
    pInst->lingxing = 3*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(pU->GetLevel(),addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;
    pInst->AddSkill(11,pInst->level);
    pInst->AddSkill(14,pInst->level);
    pInst->AddSkill(20,pInst->level);
    if(Random(0,1) == 0)
        pInst->AddSkill(156,180);
    else
        pInst->AddSkill(157,180);
        
    ptr->maxHp *= 5;
    ptr->hp = ptr->maxHp;
    
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        pFight->AddMonster(ptr,3);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::OpenPackBattle1(CUser *pU)
{
    if(pU == NULL)
        return;
    
    SMonsterInst *pInst = new SMonsterInst;
    ShareMonsterPtr ptr(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 33;
    pInst->name = "武痴";
    pInst->xiang = 0;
    pInst->level = 28;
    pInst->exp = 0;
    pInst->liliang = 6*pInst->level;
    pInst->lingxing = pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = 500;    
    
    int addHp=85;
    int addMp=40;
    int addSpeed=40;
    int addAttack=60;
    int addSkillAttack=50;
    //GetChengZhang(pU->GetLevel(),addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;
    pInst->AddSkill(120,50);
    pInst->AddSkill(112,500,3);
    ptr->maxHp *= 15;
    ptr->hp = ptr->maxHp;
    
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        pFight->AddMonster(ptr,3);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::OpenPackBattle2(CUser *pU)
{
    if(pU == NULL)
        return;
    
    SMonsterInst *pInst = new SMonsterInst;
    ShareMonsterPtr ptr(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 50;
    pInst->name = "青龙";
    pInst->xiang = 0;
    pInst->level = 30;//pU->GetLevel();
    pInst->exp = 0;
    pInst->liliang = 6*pInst->level;
    pInst->lingxing = 7*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    
    int addHp=90;
    int addMp=40;
    int addSpeed=40;
    int addAttack=50;
    int addSkillAttack=50;
    //GetChengZhang(pU->GetLevel(),addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;
    pInst->SetCeLue(CE_FANZHENG);
    pInst->AddSkill(57,100);
    pInst->AddSkill(61,100);
    pInst->AddSkill(20,30);
    pInst->AddSkill(15,30);
    pInst->AddSkill(3,30);
    ptr->maxHp *= 20;
    ptr->hp = ptr->maxHp;
    pInst->daohang = 700;
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        pFight->AddMonster(ptr,3);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::FindYaoNieBattle(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    
    uint8 num = 1;
    if(pUser->GetTeam() != 0)
        num = GetTeamMemNum(pUser->GetTeam());
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        //pFight->AddMonster(ptr,3);
        for(uint8 i = 0; i < num; i++)
        {
            SMonsterInst *pInst = new SMonsterInst;
            ShareMonsterPtr ptr(pInst);
            pInst->type = EMTNormal;
            pInst->xiang = 0;
            pInst->level = pU->GetLevel();
            pInst->exp = 0;
            
            if(Random(0,1) == 0)
            {
                pInst->tmplId = 3;
                pInst->name = "霸下";
                pInst->SetCeLue(CE_LUE_BA_XIA);
                pInst->AddSkill(155,90);
                pInst->liliang = 4*pInst->level;
                pInst->lingxing = 3*pInst->level;
            }
            else
            {
                pInst->tmplId = 14;
                pInst->name = "嘲风";
                pInst->SetCeLue(CE_LUE_CHAO_FENG);
                pInst->liliang = 3*pInst->level;
                pInst->lingxing = 4*pInst->level;
                if(Random(0,1) == 0)
                {
                    pInst->xiang = 3;
                    pInst->AddSkill(11,60);
                    pInst->AddSkill(12,60);
                }
                else
                {
                    pInst->xiang = 4;
                    pInst->AddSkill(15,60);
                    pInst->AddSkill(16,60);
                }
                pInst->AddSkill(53,60);
                //水攻击技能： 11，12
                //火攻击技能：15，16
                //分神术：53
            }
            
            pInst->tizhi = pInst->level;
            pInst->naili = pInst->level;
            pInst->minjie = pInst->level;
            pInst->daohang = pInst->level*pInst->level*pInst->level/2;    
            
            int addHp,addMp,addSpeed,addAttack,addSkillAttack;
            GetChengZhang(pU->GetLevel(),addHp,addMp,addSpeed,addAttack,addSkillAttack);
            
            pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
            
            //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
            pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
            
            pInst->hp = pInst->maxHp;
            
            pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
            
            pInst->mp = pInst->maxHp;
            
            //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
            pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
            
            //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
            pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
            
            pInst->addSkillAttack = addSkillAttack;
                
            ptr->maxHp *= 8;
            ptr->hp = ptr->maxHp;
            pFight->AddMonster(ptr,3+i);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

//寻宝战斗
void CScene::XunBaoFight(CUser *pU)
{
    SMonsterInst *pInst = new SMonsterInst;
    ShareMonsterPtr ptr(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 54;
    pInst->name = "珍兽";
    pInst->level = pU->GetLevel();
    pInst->exp = 0;
    pInst->liliang = 5*pInst->level;
    pInst->lingxing = 4*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    pInst->SetCeLue(CE_LUE_QI_LING);
    
    if(Random(0,1) == 0)
    {
        pInst->xiang = 3;
        pInst->AddSkill(11,60);
        pInst->AddSkill(12,60);
    }
    else
    {
        pInst->xiang = 4;
        pInst->AddSkill(15,60);
        pInst->AddSkill(16,60);
    }
    //pInst->AddSkill(15,90);    
    pInst->AddSkill(53,90);
    pInst->AddSkill(57,90);
    pInst->AddSkill(112,1000,3);
    //15	90级
    //53，57	90级	20%概率施放其一			
    //112	1000级	50%血以下和30%血时各对自己施放一次。（共2次）			

    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(pU->GetLevel(),addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;

    uint8 num = 1;
    uint8 teamMemNum = 0;
    if((pU->GetTeam() != 0) && ((teamMemNum = GetTeamMemNum(pU->GetTeam())) > 1))
    {
        num = teamMemNum;
    }
    ptr->maxHp *= num*10;
    ptr->hp = ptr->maxHp;
    
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        pFight->AddMonster(ptr,3);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}
//击败狂兽
void CScene::JiBaiKSBattle(CUser *pU)
{
    SMonsterInst *pInst = new SMonsterInst;
    ShareMonsterPtr ptr(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 51;
    pInst->name = "麒麟";
    pInst->level = pU->GetLevel();
    pInst->exp = 0;
    pInst->liliang = 5*pInst->level;
    pInst->lingxing = 4*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    pInst->SetCeLue(CE_LUE_QI_LING);
    
    if(Random(0,1) == 0)
    {
        pInst->xiang = 3;
        pInst->AddSkill(11,60);
        pInst->AddSkill(12,60);
    }
    else
    {
        pInst->xiang = 4;
        pInst->AddSkill(15,60);
        pInst->AddSkill(16,60);
    }
    //pInst->AddSkill(15,90);    
    pInst->AddSkill(53,90);
    pInst->AddSkill(57,90);
    pInst->AddSkill(112,1000,3);
    //15	90级				
    //53，57	90级	20%概率施放其一			
    //112	1000级	50%血以下和30%血时各对自己施放一次。（共2次）			

    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(pU->GetLevel(),addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;

    uint8 num = 1;
    uint8 teamMemNum = 0;
    if((pU->GetTeam() != 0) && ((teamMemNum = GetTeamMemNum(pU->GetTeam())) > 1))
    {
        num = teamMemNum;
    }
    ptr->maxHp *= num*10;
    ptr->hp = ptr->maxHp;
    
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        pFight->AddMonster(ptr,3);
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void InitLanRuoMonster2(ShareMonsterPtr &ptr,uint8 monsterLevel)
{
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 21;
    pInst->name = "侍女";
    pInst->level = monsterLevel;
    pInst->exp = 0;
    pInst->liliang = 7*pInst->level;
    pInst->lingxing = 7*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    pInst->SetCeLue(CL_LAN_RUO_BOS);
    
    if(Random(0,1) == 0)
    {
        pInst->xiang = 3;
    }
    else
    {
        pInst->xiang = 5;
    }
    pInst->AddSkill(20,90);
    pInst->AddSkill(111,2000);
    
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(monsterLevel,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;

    ptr->maxHp *= 20;
    ptr->hp = ptr->maxHp;
}

void InitLanRuoMonster2Fu(ShareMonsterPtr &ptr,uint8 monsterLevel,uint16 monsterId = 0)
{
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 10;
    if(monsterId != 0)
        pInst->tmplId = monsterId;
        
    pInst->name = "喽罗";
    pInst->level = monsterLevel;
    pInst->exp = 0;
    pInst->liliang = 4*pInst->level;
    pInst->lingxing = 4*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    pInst->SetCeLue(CL_LAN_RUO_BOS_FU);
    
    if(Random(0,1) == 0)
    {
        pInst->xiang = 3;
    }
    else
    {
        pInst->xiang = 5;
    }
    //pInst->AddSkill(15,90);    
    pInst->AddSkill(152,250);
    pInst->AddSkill(57,90);
    pInst->AddSkill(61,90);
    pInst->AddSkill(11,90);
    
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(monsterLevel,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;

    ptr->maxHp *= 18;
    ptr->hp = ptr->maxHp;
}

//二阶段
void CScene::LanRuoBattle(CUser *pU)
{
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetJLType(EFJLType2);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        CUserTeam *pTeam = NULL;
        uint8 level = 80;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {//组队，或者只有队长一人
            AddTeamToFight(pFight,pTeam,7);
        }
        else
        {
            pU->GetLevel();
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        }
        ShareMonsterPtr ptr;
        InitLanRuoMonster2(ptr,level);
        pFight->AddMonster(ptr,4);
        InitLanRuoMonster2Fu(ptr,level);
        pFight->AddMonster(ptr,1);
        InitLanRuoMonster2Fu(ptr,level);
        pFight->AddMonster(ptr,5);
        //BeginEpisodeBattle(pFight,pUser);
        pFight->BeginFight(m_socketServer,this);
        pFight->SetDelNpc(14,14);
    }
    m_fightManager.AddFight(pFight);
}

void InitLanRuoMonster3(ShareMonsterPtr &ptr,uint8 monsterLevel)
{
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 7;
    pInst->name = "逃跑小妖";
    pInst->level = monsterLevel;
    pInst->exp = 0;
    pInst->liliang = 5*pInst->level;
    pInst->lingxing = 4*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    pInst->SetCeLue(CE_LUE_QI_LING);
    
    if(Random(0,1) == 0)
    {
        pInst->xiang = 3;
        pInst->AddSkill(11,60);
        pInst->AddSkill(12,60);
    }
    else
    {
        pInst->xiang = 4;
        pInst->AddSkill(15,60);
        pInst->AddSkill(16,60);
    }
    //pInst->AddSkill(15,90);    
    pInst->AddSkill(53,90);
    pInst->AddSkill(57,90);
    pInst->AddSkill(112,1000,3);
    //15	90级				
    //53，57	90级	20%概率施放其一			
    //112	1000级	50%血以下和30%血时各对自己施放一次。（共2次）			

    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(monsterLevel,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = 0xffffff;
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = 1;//(int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;

    ptr->maxHp = 3;
    ptr->hp = ptr->maxHp;
}

//三阶段
void CScene::LanRuoBattle3(CUser *pU)
{
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetJLType(EFJLType3);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        CUserTeam *pTeam = NULL;
        uint8 level = 80;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {//组队，或者只有队长一人
            AddTeamToFight(pFight,pTeam,7);
        }
        else
        {
            pU->GetLevel();
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        }
        
        for(uint8 i = 0; i < 6; i++)
        {
            ShareMonsterPtr ptr;
            InitLanRuoMonster3(ptr,level);
            ptr->SetCeLue(CL_TAO_PAO);
            pFight->AddMonster(ptr,i+1);
        }
        //BeginEpisodeBattle(pFight,pUser);
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void InitLanRuoMonster4(ShareMonsterPtr &ptr,uint8 monsterLevel)
{
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 24;
    pInst->name = "鬼火";
    pInst->level = monsterLevel;
    pInst->exp = 0;
    pInst->liliang = 5*pInst->level;
    pInst->lingxing = 4*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    //pInst->SetCeLue(CE_LUE_QI_LING);
    
    if(Random(0,1) == 0)
    {
        pInst->xiang = 3;
        pInst->AddSkill(11,60);
        pInst->AddSkill(12,60);
    }
    else
    {
        pInst->xiang = 4;
        pInst->AddSkill(15,60);
        pInst->AddSkill(16,60);
    }
    //pInst->AddSkill(15,90);    
    pInst->AddSkill(53,90);
    pInst->AddSkill(57,90);
    pInst->AddSkill(112,1000,3);
    //15	90级				
    //53，57	90级	20%概率施放其一			
    //112	1000级	50%血以下和30%血时各对自己施放一次。（共2次）			

    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(monsterLevel,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = 0xffffff;
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = 200000;//(int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;

    ptr->maxHp = 3;
    ptr->hp = ptr->maxHp;
}

//四阶段
void CScene::LanRuoBattle4(CUser *pU)
{
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetJLType(EFJLType4);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        
        CUserTeam *pTeam = NULL;
        uint8 level = 80;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {//组队，或者只有队长一人
            AddTeamToFight(pFight,pTeam,7);
        }
        else
        {
            pU->GetLevel();
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        }
        
        for(uint8 i = 0; i < 6; i++)
        {
            ShareMonsterPtr ptr;
            InitLanRuoMonster4(ptr,level);
            ptr->SetCeLue(CL_ZI_BAO);
            pFight->AddMonster(ptr,i+1);
        }
        pFight->BeginFight(m_socketServer,this);
        //BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void InitLanRuoMonster5(ShareMonsterPtr &ptr,uint8 monsterLevel)
{
    SMonsterInst *pInst = new SMonsterInst;
    ptr.reset(pInst);
    pInst->type = EMTNormal;
    pInst->tmplId = 34;
    pInst->name = "姥姥";
    pInst->level = monsterLevel;
    pInst->exp = 0;
    pInst->liliang = 7*pInst->level;
    pInst->lingxing = 7*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = 7*pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = pInst->level*pInst->level*pInst->level;    
    pInst->SetCeLue(CL_LAN_RUO_LAOLAO);
    
    if(Random(0,1) == 0)
    {
        pInst->xiang = 3;
    }
    else
    {
        pInst->xiang = 5;
    }
    //pInst->AddSkill(15,90);    
    //pInst->AddSkill(202,90);
    pInst->AddSkill(111,2000);

    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(monsterLevel,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
    
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;

    ptr->maxHp *= 30;
    ptr->hp = ptr->maxHp;
}

//五阶段
void CScene::LanRuoBattle5(CUser *pU)
{
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetJLType(EFJLType5);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        CUserTeam *pTeam = NULL;
        uint8 level = 80;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {//组队，或者只有队长一人
            AddTeamToFight(pFight,pTeam,7);
        }
        else
        {
            pU->GetLevel();
            pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        }
        
        ShareMonsterPtr ptr;
        InitLanRuoMonster5(ptr,level);
        pFight->AddMonster(ptr,4);
        InitLanRuoMonster2Fu(ptr,level,39);
        pFight->AddMonster(ptr,1);
        InitLanRuoMonster2Fu(ptr,level,39);
        pFight->AddMonster(ptr,5);
        
        pFight->BeginFight(m_socketServer,this);
        //BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::Init()
{
    memset(m_jifens,0,sizeof(m_jifens));
    memset(m_kuangs,0,sizeof(m_kuangs));
    memset(m_bangpais,0,sizeof(m_bangpais));
    memset(m_meiYingNum,0,sizeof(m_meiYingNum));
    memset(m_meiYingStartTime,0,sizeof(m_meiYingStartTime));
    memset(m_meiYingTimeOut,0,sizeof(m_meiYingTimeOut));
}

void CScene::SetDiaoXiangBang(int bId,int diaoXiangId)
{
    /*if(m_diaoXiangBang != 0)
    {
        //int jifen = GetJiFen(m_diaoXiangBang);
        //jifen += (GetSysTime() - m_zhanDiaoXiangTime)/30;
        //SetJiFen(m_diaoXiangBang,jifen);
    }*/
    m_zhanDiaoXiangTime[diaoXiangId] = GetSysTime();
    m_diaoXiangBang[diaoXiangId] = bId;
}
int CScene::GetJiFen(int bId)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if(m_bangpais[i] == bId)
            return m_jifens[i];
    }
    return 0;
}
int CScene::GetKuang(int bId)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if(m_bangpais[i] == bId)
            return m_kuangs[i];
    }
    return 0;
}
int CScene::GetOtherJiFen(int bId)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if(m_bangpais[i] != bId)
            return m_jifens[i];
    }
    return 0;
}
int CScene::GetOtherKuang(int bId)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if(m_bangpais[i] != bId)
            return m_kuangs[i];
    }
    return 0;
}
void CScene::SetJiFen(int bId,int jifen)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if((m_bangpais[i] == bId) || (m_bangpais[i] == 0))
        {
            m_jifens[i] = jifen;
            m_bangpais[i] = bId;
            return;
        }
    }
}
void CScene::SetOtherJiFen(int bId,int jifen)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if(m_bangpais[i] != bId)
        {
            m_jifens[i] = jifen;
            return;
        }
    }
}
void CScene::SetOtherKuang(int bId,int kuang)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if(m_bangpais[i] != bId)
        {
            m_kuangs[i] = kuang;
            return;
        }
    }
}
//魅影数量，持续时间
void CScene::StartMeiYing(int bId,int num,int timeOut)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if((m_bangpais[i] == 0) || (m_bangpais[i] == bId))
        {
            m_meiYingNum[i] = num;
            m_meiYingStartTime[i] = GetSysTime();
            m_meiYingTimeOut[i] = timeOut;
            m_bangpais[i] = bId;
            return;
        }
    }
}
int CScene::GetMeYingLeftTime(int bId)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if(m_bangpais[i] == bId)
        {
            if(m_meiYingStartTime[i] + m_meiYingTimeOut[i] <= GetSysTime())
                return 0;
            return m_meiYingTimeOut[i] - (GetSysTime() - m_meiYingStartTime[i]);
        }
    }
    return 0;
}

int CScene::GetOtherLeftMeiYing(int bId)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if(m_bangpais[i] != bId)
        {
            if(m_meiYingStartTime[i] + m_meiYingTimeOut[i] <= GetSysTime())
                return 0;
            if(m_meiYingNum[i] <= 0)
                return 0;
            return m_meiYingNum[i]--;
        }
    }
    return 0;
}

void CScene::SetKuang(int bId,int kuang)
{
    for(uint8 i = 0; i < 2; i++)
    {
        if((m_bangpais[i] == 0) || (m_bangpais[i] == bId))
        {
            m_kuangs[i] = kuang;
            m_bangpais[i] = bId;
            return;
        }
    }
}

void CScene::GroupClear(int sceneId,uint8 x,uint8 y)
{
    m_usedFuBen = false;
    
    COnlineUser &onlineUser = SingletonOnlineUser::instance();
    ClearVisibleMonster();
    
    list<uint32> userList;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        userList = m_userList;
        for(list<SNpcInstance*>::iterator i = m_dynamicNpc.begin(); i != m_dynamicNpc.end(); i++)
        {
            delete *i;
        }
        m_dynamicNpc.clear();
    }
    for(list<uint32>::iterator i = userList.begin(); i != userList.end(); i++)
    {
        ShareUserPtr ptr = onlineUser.GetUserByRoleId(*i);
        CUser *pUser = ptr.get();
        if(pUser != NULL)
        {
            TransportUser(pUser,sceneId,x,y,8);////301,14,20,8);
        }
    }
}

void CScene::GroupSetState(int state)
{
    CSceneManager &scene = SingletonSceneManager::instance();
    
    CScene *pScene = NULL;
    list<int> sceneList;
    scene.GetGroupScene(m_groupId,sceneList);
    for(list<int>::iterator i = sceneList.begin(); i != sceneList.end(); i++)
    {
        pScene = scene.FindScene(*i);
        if(pScene != NULL)
        {
            //cout<<pScene->GetId()<<":"<<state<<endl;
            pScene->m_state = state;
        }
    }
}
void CScene::Clear()
{
    CSceneManager &scene = SingletonSceneManager::instance();
    
    CScene *pScene = NULL;
    list<int> sceneList;
    scene.GetGroupScene(m_groupId,sceneList);
    for(list<int>::iterator i = sceneList.begin(); i != sceneList.end(); i++)
    {
        pScene = scene.FindScene(*i);
        if(pScene != NULL)
        {
            pScene->GroupClear(301,14,20);
        }
    }
}
int CScene::GetState()
{
    return m_state;
}
void CScene::SetState(int state)
{
    GroupSetState(state);
}

void InitQiLinMonster(SMonsterInst *pInst,uint16 tmplId,uint8 monsterLevel)
{
    pInst->type = EMTNormal;
    pInst->tmplId = tmplId;
    //pInst->name = "试炼天兵";
    pInst->level = monsterLevel;
    pInst->exp = 0;
    pInst->liliang = 5*pInst->level;
    pInst->lingxing = 4*pInst->level;
    pInst->tizhi = pInst->level;
    pInst->naili = pInst->level;
    pInst->minjie = pInst->level;
    pInst->daohang = 0xffffff;//pInst->level*pInst->level*pInst->level;    
    pInst->xiang = 0;
        
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(monsterLevel,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0))*2;
        
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60)*2;
        
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80)*2;
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80)*2;
    
    pInst->addSkillAttack = addSkillAttack*2;

    pInst->hp = pInst->maxHp;
}

void CScene::QiLinFightYao(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level;
        CUserTeam *pTeam = NULL;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {
            level = AddTeamToFight(pFight,pTeam,7);
        }
        else
        {
            pU->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            level = pUser->GetLevel();
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            InitQiLinMonster(pInst,39,level);
            pInst->name = "妖兵";
            ShareMonsterPtr ptr(pInst);
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            InitQiLinMonster(pInst,39,level);
            pInst->name = "妖兵";
            ShareMonsterPtr ptr(pInst);
            pFight->AddMonster(ptr,5);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::QiLinFightXian(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level;
        CUserTeam *pTeam = NULL;
        if(m_userTeams.Find(pUser->GetTeam(),pTeam))
        {
            level = AddTeamToFight(pFight,pTeam,7);
        }
        else
        {
            pU->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
            level = pUser->GetLevel();
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            InitQiLinMonster(pInst,45,level);
            pInst->name = "仙兵";
            ShareMonsterPtr ptr(pInst);
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            InitQiLinMonster(pInst,45,level);
            pInst->name = "仙兵";
            ShareMonsterPtr ptr(pInst);
            pFight->AddMonster(ptr,5);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}
void CScene::ChuShiFight(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        uint8 r = Random(0,5);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        for(uint8 i = 0; i < 6; i++)
        {
            SMonsterInst *pInst = new SMonsterInst;
            if(i == r)
                InitHuanYingMonster(pInst,pU->GetLevel(),true);
            else
                InitHuanYingMonster(pInst,pU->GetLevel(),false);
            ShareMonsterPtr ptr(pInst);
            pFight->AddMonster(ptr,i+1);
        }
        BeginEpisodeBattle(pFight,pUser);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::BangZhanJiFen()
{
    //m_jsBangZhanJF;//计算帮战积分时间
    time_t t = GetSysTime();
    tm *pTm = localtime(&t);
    if(pTm == NULL)
        return;
    if(!m_inBangZhan && (pTm->tm_hour == BANG_ZHAN_BEGIN_TIME))
    {
        m_inBangZhan = true;
        m_jifens[0] = 0;
        m_jifens[1] = 0;
        m_bangpais[0] = 0;
        m_bangpais[1] = 0;
        m_sendJFGongGao = GetSysTime();
    }
    else if((pTm->tm_hour == BANG_ZHAN_END_TIME) && m_inBangZhan)
    {
        m_inBangZhan = false;
        CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
        CBangPai *pBangPai1 = bPMgr.FindBangPai(m_bangpais[0]);
        CBangPai *pBangPai2 = bPMgr.FindBangPai(m_bangpais[1]);
        if((pBangPai1 == NULL) || (pBangPai2 == NULL))
            return;
        if(m_jifens[0] > m_jifens[1])
        {
            pBangPai1->SetFanRong(pBangPai1->GetFanRong() + 50);
            pBangPai2->SetFanRong(pBangPai2->GetFanRong() - 50);
        }
        else if(m_jifens[0] == m_jifens[1])
        {
            pBangPai1->SetFanRong(pBangPai1->GetFanRong() - 20);
            pBangPai2->SetFanRong(pBangPai2->GetFanRong() - 20);
        }
        else 
        {
            pBangPai1->SetFanRong(pBangPai1->GetFanRong() - 50);
            pBangPai2->SetFanRong(pBangPai2->GetFanRong() + 50);
        }
        char buf[256];
        snprintf(buf,256,"%s帮积分%d,%s帮积分%d。"
                ,pBangPai1->GetName().c_str(),m_jifens[0]
                ,pBangPai2->GetName().c_str(),m_jifens[1]);
        if(m_jifens[0] > m_jifens[1])
        {
            snprintf(buf+strlen(buf),256-strlen(buf),"%s帮获胜",pBangPai1->GetName().c_str());
        }
        else if(m_jifens[1] > m_jifens[0])
        {
            snprintf(buf+strlen(buf),256-strlen(buf),"%s帮获胜",pBangPai2->GetName().c_str());
        }
        SysInfoToAllUser(buf);//SendSysInfoToGroup(m_groupId,buf);
        
        /*CSceneManager &scene = SingletonSceneManager::instance();
    
        CScene *pScene = NULL;
        list<int> sceneList;
        scene.GetGroupScene(m_groupId,sceneList);
        for(list<int>::iterator i = sceneList.begin(); i != sceneList.end(); i++)
        {
            pScene = scene.FindScene(*i);
            if(pScene != NULL)
            {
                pScene->GroupClear(BANG_PAI_SCENE_ID,15,15);
            }
        }*/
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        sprintf(buf,"update xz_bang set jifen1=%d,jifen2=%d where bangpai1=%d and bangpai2=%d"
                ,m_jifens[0],m_jifens[1],m_bangpais[0],m_bangpais[1]);
        if (pDb != NULL)
        {
            pDb->Query(buf);
        }
        return;
    }
    if(!m_inBangZhan)
        return;
        
    int jifen[2] = {0};
    if(GetSysTime() - m_jsBangZhanJF > 30)
    {
        m_jsBangZhanJF = GetSysTime();
        for(map<int,int>::iterator i = m_diaoXiangBang.begin(); i != m_diaoXiangBang.end();i++)
        {
            if(i->second == m_bangpais[0])
                jifen[0] += 3;
            else if(i->second == m_bangpais[1])
                jifen[1] += 3;
        }
        for(uint8 i = 0; i < 2; i++)
        {
            if(jifen[i] == 6)
                jifen[i] = 7;
            else if(jifen[i] == 9)
                jifen[i] = 12;
        }
        for(uint8 i = 0; i < 2; i++)
        {
            m_jifens[i] += jifen[i];
        }
    }
    if(GetSysTime() - m_sendJFGongGao > 600)
    {
        m_sendJFGongGao = GetSysTime();
        CBangPaiManager &bPMgr = SingletonCBangPaiManager::instance();
        CBangPai *pBangPai1 = bPMgr.FindBangPai(m_bangpais[0]);
        CBangPai *pBangPai2 = bPMgr.FindBangPai(m_bangpais[1]);
        if((pBangPai1 == NULL) || (pBangPai2 == NULL))
            return;
        char buf[256];
        snprintf(buf,256,"%s帮积分%d,%s帮积分%d"
                ,pBangPai1->GetName().c_str(),m_jifens[0]
                ,pBangPai2->GetName().c_str(),m_jifens[1]);
        SysInfoToGroupUser(m_groupId,buf);//SysInfoToAllUser(buf);
    }
}

bool CSceneManager::FindMapSceneList(int id,CScene *pScene,int mapId,list<CScene*> *pSceneList)
{
    if(pScene->GetMapId() == mapId)
    {
        pSceneList->push_back(pScene);
    }
    return true;
}

char *CScene::GetMatchPaiMing()
{
    if(!m_isPaiMing)
        return NULL;
        
    static char paiming[256];
    int num = m_jifenUsers.size();
    num = min(num,10);
    paiming[0] = 0;
    
    for(int i = 0; i < num; i++)
    {
        if(i != 0)
            strcat(paiming,"|");
        strcat(paiming,m_jifenUsers[i].name.c_str());
    }
    if(paiming[0] == 0)
        return NULL;
    return paiming;
}

extern bool HaveMarryHall(uint32 id);

void CSceneManager::Timer()
{
    if(GetSysTime() - m_matchRunTime > RUN_MATCH_SPACE)
    {
        m_matchRunTime = GetSysTime();
        for(uint16 i = MATCH_SCENE_ID; i < MATCH_SCENE_ID + MATCH_SCENE_NUM; i++)
        {
            CScene *pScene = FindScene(i);
            if(pScene != NULL)
            {
                pScene->Match();
            }
        }
    }
    list<CScene*> sceneList;
    {
        boost::mutex::scoped_lock lk(m_bpMutex);
        m_sceneList.ForEach(boost::bind(&CSceneManager::FindMapSceneList,
                                            this,_1,_2,302,&sceneList));
    }
    for(list<CScene*>::iterator i = sceneList.begin(); i != sceneList.end(); i++)
    {
        CScene *pScene = FindScene((*i)->GetId()+1);
        if((pScene != NULL) 
            && ((*i)->GetId() != 302)
            && (GetSysTime() - pScene->m_startTime > 3600*2))
        {
            (*i)->Clear();
            m_gcLanRuo.push_back(*i);
            m_gcLanRuo.push_back(pScene);
            boost::mutex::scoped_lock lk(m_bpMutex);
            m_sceneList.Erase((*i)->GetId());
            m_sceneList.Erase(pScene->GetId());
        }
    }
    
    sceneList.clear();
    {
        boost::mutex::scoped_lock lk(m_bpMutex);
        m_sceneList.ForEach(boost::bind(&CSceneManager::FindMapSceneList,
                                            this,_1,_2,260,&sceneList));
    }
    CScene *pClearUser = NULL;
    {
        boost::mutex::scoped_lock lk(m_bpMutex);
        for(list<CScene*>::iterator i = m_marryHall.begin(); i != m_marryHall.end(); i++)
        {
            if(!HaveMarryHall((*i)->GetId()))
            {
                pClearUser = *i;
                m_marryHall.erase(i);
                break;
            }
        }
    }
    if(pClearUser != NULL)
    {
        list<uint32> userList;
        pClearUser->GetUserList(userList);
        COnlineUser &onlineUser = SingletonOnlineUser::instance();
        for(list<uint32>::iterator i = userList.begin(); i != userList.end(); i++)
        {
            ShareUserPtr ptr = onlineUser.GetUserByRoleId(*i);
            CUser *pUser = ptr.get();
            if(pUser != NULL)
            {
                SendPopMsg(pUser,"婚礼时间已到，礼堂关闭");
                TransportUser(pUser,20,22,14,8);
            }
        }
    }
    for(list<CScene*>::iterator i = sceneList.begin(); i != sceneList.end(); i++)
    {
        (*i)->BangZhanJiFen();
    }
}
/*
bool CScene::FindMatchTeam(uint32 teamId,CUserTeam *pTeam,vector<uint32> *pTeamList)
{
    ShareUserPtr ptr = SingletonOnlineUser::instance().GetUserByRoleId(teamId);
    CUser *pUser = ptr.get();
    if((pUser == NULL) || (pUser->GetFightId() != 0))
        return true;
    if(pTeam->GetMemberNum() == 3)
    {
        pTeamList->push_back(teamId);
    }
    return true;
}
*/

void CScene::FindMatchUser(vector<uint32> &userList)
{
    COnlineUser &onlineUser = SingletonOnlineUser::instance();
    boost::recursive_mutex::scoped_lock lk(m_mutex);
        
    for(list<uint32>::iterator i = m_userList.begin(); i != m_userList.end(); i++)
    {
        ShareUserPtr ptr = onlineUser.GetUserByRoleId(*i);
        CUser *pUser = ptr.get();
        if(pUser != NULL)
        {
            if(pUser->GetFightId() != 0)
                continue;
            if(pUser->GetTeam() == 0)
            {
                userList.push_back(pUser->GetRoleId());
            }
            else if(pUser->GetTeam() == pUser->GetRoleId())
            {
                userList.push_back(pUser->GetRoleId());
            }
        }
    }
}

struct SSortUserJifen
{
    bool operator()(const SJiFenUser &m1,const SJiFenUser &m2)
    {
        return m1.jifen > m2.jifen;
    }
};

int CScene::GetUserJiFen(uint32 roleId)
{
    int num = m_jifenUsers.size();
    for(int i = 0; i < num; i++)
    {
        if(m_jifenUsers[i].roleId == roleId)
        {
            return m_jifenUsers[i].jifen;
        }
    }
    return 0;
}

void CScene::SetUserJiFen(uint32 roleId,char *name,short jifen)
{
    if(GetSysTime() - m_matchBegin > MATCH_TIME)
        return;
    int num = m_jifenUsers.size();
    for(int i = 0; i < num; i++)
    {
        if(m_jifenUsers[i].roleId == roleId)
        {
            m_jifenUsers[i].jifen += jifen;
            return;
        }
    }
    SJiFenUser userJifen;
    userJifen.roleId = roleId;
    userJifen.name = name;
    userJifen.jifen = jifen;
    m_jifenUsers.push_back(userJifen);
}

void CScene::SetOffLineTitle(uint32 roleId,uint8 title)
{
    CUser *pUser = new CUser;
    auto_ptr<CUser> user(pUser);
    CGetDbConnect getDb;
    CDatabaseSql *pDb = getDb.GetDbConnect();
    if(pDb == NULL)
        return;
    char sql[4096];
    snprintf(sql,512,"select title from role_info where id=%u",roleId);
    if(!pDb->Query(sql))
        return;
    char **row = pDb->GetRow();
    if(row == NULL)
        return;
    pUser->ReadTitle(row[0]);  
    pUser->AddTitle(title);
    string str;
    pUser->GetTitleStr(str);
    snprintf(sql,4096,"update role_info set title='%s' where id=%u",
        str.c_str(),roleId);
    pDb->Query(sql);
}

void CScene::Match()
{
    time_t t = GetSysTime();
    tm *pTm = localtime(&t);
    if(pTm == NULL)
        return;
    if(((pTm->tm_hour == MATCH_BEGIN_TIME) && (pTm->tm_min < 3))
        || ((pTm->tm_hour == TEAM_MATCH_BEGIN_TIME) && (pTm->tm_min < 3)))
    {
        m_jifenUsers.clear();
        //format fmt("%1%赛正式开始！");
        //fmt%m_name.c_str();
        //SysInfoToAllUser(fmt.str().c_str());
        m_isPaiMing = false;
        m_matchBegin = GetSysTime();
        return;
    }
    else if(GetSysTime() - m_matchBegin > MATCH_TIME)
    {
        if(m_isPaiMing)
            return;
        m_isPaiMing = true;
        int num = m_jifenUsers.size();
        if(num <= MAX_JIFEN_USER_NUM)
            return;
            
        SSortUserJifen sortJifen;
        std::sort(m_jifenUsers.begin(),m_jifenUsers.end(),sortJifen);
        
        format fmt("本周%1%赛圆满结束，恭喜玩家:%2% %3% %4%获得了前三名");
        fmt%m_name.c_str();
        
        COnlineUser &onlineUser = SingletonOnlineUser::instance();
        
        for(uint8 i = 0; i < MAX_JIFEN_USER_NUM; i++)
        {
            fmt%m_jifenUsers[i].name.c_str();
            ShareUserPtr ptr = onlineUser.GetUserByRoleId(m_jifenUsers[i].roleId);
            CUser *pUser = ptr.get();
            if(pUser != NULL)
            {
                pUser->AddTitle(i+1);
            }
            else
            {
                SetOffLineTitle(m_jifenUsers[i].roleId,i+1);
            }
        }
        SysInfoToAllUser(fmt.str().c_str());
        
        CGetDbConnect getDb;
        CDatabaseSql *pDb = getDb.GetDbConnect();
        if(pDb == NULL)
            return;
        
        for(int i = 0; i < num; i++)
        {
            format sql("INSERT INTO leitai_paiming (id,map_id,role_id,name,jifen) VALUES "\
                "(%1%,%2%,%3%,'%4%',%5%)");
            sql % i % m_mapId % m_jifenUsers[i].roleId % m_jifenUsers[i].name.c_str()
                % m_jifenUsers[i].jifen;
            pDb->Query(sql.str().c_str());
        }
        int tongBao = 0;
        /*320 	40级擂台赛：2000通宝		
        321 	50级擂台赛：3000通宝		
        322 	60级擂台赛：4000通宝		
        323 	高级擂台赛：5000通宝*/
        if(m_mapId == 320)
            tongBao = 2000;
        else if(m_mapId == 321)
            tongBao = 3000;
        else if(m_mapId == 322)
            tongBao = 4000;
        else if(m_mapId == 323)
            tongBao = 5000;  
        num = min(10,num);
        num = Random(0,num-1);
        AddTongBao(m_jifenUsers[num].roleId,tongBao);
        char buf[512];
        snprintf(buf,512,"恭喜本次擂台赛【%s】被系统抽中，获得擂台奖励通宝%d！",
            m_jifenUsers[num].name.c_str(),tongBao);
        SysInfoToAllUser(buf);
        return;
    }
    if((pTm->tm_hour != MATCH_BEGIN_TIME) && (pTm->tm_hour != TEAM_MATCH_BEGIN_TIME))
        return;
        
    vector<uint32> userList;
    FindMatchUser(userList);
    
    uint32 user1,user2;
    uint32 r;
    COnlineUser &onlineUser = SingletonOnlineUser::instance();
    uint32 userNum = userList.size()/2;
    for(uint32 i = 0; i < userNum; i++)
    {
        r = Random(0,userList.size()-1);
        if(r >= userList.size())
            return;
        user1 = userList[r];
        userList.erase(userList.begin()+r);
        r = Random(0,userList.size()-1);
        if(r >= userList.size())
            return;
        user2 = userList[r];
        userList.erase(userList.begin()+r);
        
        ShareFightPtr pFight = m_fightManager.CreateFight();
        pFight->SetFightType(CFight::EFTMatch);
        
        {
            boost::recursive_mutex::scoped_lock lk(m_mutex);
            ShareUserPtr ptr = onlineUser.GetUserByRoleId(user1);
            if(ptr.get() == NULL)
                continue;
            if(ptr->GetTeam() == 0)
                ptr->SetFight(pFight->GetId(),pFight->AddUser(ptr,3));
            else
            {
                CUserTeam *pTeam = NULL;
                if(m_userTeams.Find(ptr->GetRoleId(),pTeam))
                {
                    AddTeamToFight(pFight,pTeam,1);
                }
            }
            ptr = onlineUser.GetUserByRoleId(user2);
            if(ptr.get() == NULL)
                continue;
            if(ptr->GetTeam() == 0)
                ptr->SetFight(pFight->GetId(),pFight->AddUser(ptr,9));
            else
            {
                CUserTeam *pTeam = NULL;
                if(m_userTeams.Find(ptr->GetRoleId(),pTeam))
                {
                    AddTeamToFight(pFight,pTeam,7);
                }
            }
        }
        pFight->BeginFight(m_socketServer,this);
        m_fightManager.AddFight(pFight);
    }
}

void InitMonsterByZhaoTao(SMonsterInst *pInst)
{
    CMonsterManager &manager = SingletonMonsterManager::instance();
    SMonsterTmpl *pTmpl = manager.GetTmpl(pInst->tmplId);
    if(pTmpl != NULL)
    {
        pInst->exp = pTmpl->exp;
        pInst->pMonster = pTmpl;
    }
    int addHp,addMp,addSpeed,addAttack,addSkillAttack;
    GetChengZhang(pInst->level,addHp,addMp,addSpeed,addAttack,addSkillAttack);
    pInst->type = EMTNormal;
    pInst->xiang = 0;
    pInst->maxHp = (int)((pInst->level*5+(pInst->tizhi-pInst->level)) * (40*addHp/100.0));
    
    //防御：	(等级*5+(耐力-等级))*(4*血成长/100)+60
    pInst->recovery = (int)((pInst->level*5+(pInst->naili-pInst->level))*(4*addHp/100.0) + 60);
        
    pInst->hp = pInst->maxHp;
    
    pInst->maxMp = (int)((pInst->level*5+(pInst->lingxing-pInst->level)) * (12*addMp/100.0) + 80);
    
    pInst->mp = pInst->maxHp;
    
    //速度：	等级*(2.2+敏捷*0.05)+(速度成长-等级)*(1.5)+10
    pInst->speed = (int)(pInst->level*(2.2+pInst->minjie*0.05)+(addSpeed-pInst->level)*1.5+10);
    
    //物攻：	(等级*5+(力量-等级))*(12*物攻成长/100)+80
    pInst->attack = (int)((pInst->level*5+(pInst->liliang-pInst->level))*(12*addAttack/100.0)+80);
    
    pInst->addSkillAttack = addSkillAttack;
    
    pInst->daohang = 4*pInst->level*pInst->level*pInst->level;
}

uint8 CScene::AddUserToFight(ShareFightPtr &pFight,ShareUserPtr &pUser,uint8 *pTeamMemNum)
{
    CUserTeam *pTeam = NULL;
    if(m_userTeams.Find(pUser->GetTeam(),pTeam))
    {
        if(pTeamMemNum != NULL)
            *pTeamMemNum = pTeam->GetMemberNum();
        return AddTeamToFight(pFight,pTeam,7);
    }
    else
    {
        if(pTeamMemNum != NULL)
            *pTeamMemNum = 1;
        pUser->SetFight(pFight->GetId(),pFight->AddUser(pUser,9));
        return pUser->GetLevel();
    }
}

void CScene::ShiYaoYW1(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(1);
        //pFight->SetFightType((CFight::EFightType)type);
        pFight->SetFightType(CFight::EFTGuiYu);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = min((int)level,60);
        SMonsterInst *pInst = new SMonsterInst;
        pInst->tmplId = 14;
        pInst->name = "天魅妖";
        ShareMonsterPtr ptr(pInst);
        pInst->level = level;
        pInst->liliang = 2*level;//力量
        pInst->lingxing = 2*level;//灵性
        pInst->tizhi = level;//体质
        pInst->naili = level;//耐力
        pInst->minjie = level;//敏捷
        InitMonsterByZhaoTao(pInst);
        pInst->SetCeLue(ONLY_HUN_LUAN);
        
        pInst->maxHp *= 8;
        pInst->hp = pInst->maxHp;
        pInst->AddSkill(62,100);
        pInst->chatMsg = "能挡住我的魅惑吗？";
        pFight->AddMonster(ptr,3);
        pFight->BeginFight(m_socketServer,this);
        pFight->SetJLType(EFJLType9);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYW2(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(2);
        pFight->SetFightType(CFight::EFTGuiYu);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 1;
        uint8 level = AddUserToFight(pFight,pUser,&num);
        level = min((int)level,60);
        if(num > 1)
            num = 6;
        for(uint8 i = 1; i <= num; i++)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 24;
            pInst->name = "血爆妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 2*level;//力量
            pInst->lingxing = 2*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            
            pInst->maxHp = 10;
            pInst->hp = pInst->maxHp;
            pInst->recovery = 0x7fffffff;
            pInst->addData = 5;
            pInst->SetCeLue(CL_ZI_BAO);
            if(i == 3)
                pInst->chatMsg = "以血为引，粉身碎骨。";
            pFight->AddMonster(ptr,i);
        }
        pFight->BeginFight(m_socketServer,this);
        pFight->SetJLType(EFJLType9);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYW3(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(3);
        pFight->SetFightType(CFight::EFTGuiYu);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 1;
        uint8 level = AddUserToFight(pFight,pUser,&num);
        level = min((int)level,60);
        if(num > 1)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 44;
            pInst->name = "蛊魔妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 5*level;
            pInst->liliang = 5*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7fffffff;
            pInst->SetCeLue(ONLY_PET);
            pInst->AddSkill(3,level);
            pInst->AddSkill(7,level);
            pInst->AddSkill(11,level);
            pInst->AddSkill(15,level);
            pInst->AddSkill(19,level);
            pInst->maxHp *= 5;
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,1);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 44;
            pInst->name = "蛊魔妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 5*level;
            pInst->liliang = 5*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7fffffff;
            pInst->SetCeLue(ONLY_PET);
            pInst->AddSkill(3,level);
            pInst->AddSkill(7,level);
            pInst->AddSkill(11,level);
            pInst->AddSkill(15,level);
            pInst->AddSkill(19,level);
            pInst->maxHp *= 5;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "人是伤不了我的。";
            pFight->AddMonster(ptr,3);
        }
        if(num > 1)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 44;
            pInst->name = "蛊魔妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 5*level;
            pInst->liliang = 5*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7fffffff;
            pInst->SetCeLue(ONLY_PET);
            pInst->AddSkill(3,level);
            pInst->AddSkill(7,level);
            pInst->AddSkill(11,level);
            pInst->AddSkill(15,level);
            pInst->AddSkill(19,level);
            pInst->maxHp *= 5;
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,5);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    pFight->SetJLType(EFJLType9);
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYW4(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(4);
        pFight->SetFightType(CFight::EFTGuiYu);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = min((int)level,60);
        SMonsterInst *pInst = new SMonsterInst;
        pInst->tmplId = 45;
        pInst->name = "金刚妖";
        ShareMonsterPtr ptr(pInst);
        pInst->level = level;
        pInst->liliang = 7*level;//力量
        pInst->lingxing = 7*level;//灵性
        pInst->tizhi = level;//体质
        pInst->naili = level;//耐力
        pInst->minjie = level;//敏捷
        InitMonsterByZhaoTao(pInst);
        
        pInst->maxHp *= 20;
        pInst->hp = pInst->maxHp;
        pInst->chatMsg = "我是打不死的。";
        pFight->AddMonster(ptr,3);
        pFight->BeginFight(m_socketServer,this);
    }
    pFight->SetJLType(EFJLType9);
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYW5(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(5);
        pFight->SetFightType(CFight::EFTGuiYu);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 1;
        uint8 level = AddUserToFight(pFight,pUser,&num);
        level = min((int)level,60);
        if(num > 1)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 46;
            pInst->name = "龌龊妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 2*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            pInst->daohang = level*level*level/2;
            InitMonsterByZhaoTao(pInst);
            pInst->mingzhong = 10000;
            pInst->maxHp *= 8;
            pInst->hp = pInst->maxHp;
            pInst->addData = 50;
            pInst->SetCeLue(SUN_SHI_NAIJIU);
            pFight->AddMonster(ptr,1);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 46;
            pInst->name = "龌龊妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 2*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            pInst->daohang = level*level*level/2;
            InitMonsterByZhaoTao(pInst);
            pInst->mingzhong = 10000;
            pInst->maxHp *= 8;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "我能毁坏你的装备。";
            pInst->addData = 50;
            pInst->SetCeLue(SUN_SHI_NAIJIU);
            pFight->AddMonster(ptr,3);
        }
        if(num > 1)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 46;
            pInst->name = "龌龊妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 2*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            pInst->daohang = level*level*level/2;
            InitMonsterByZhaoTao(pInst);
            pInst->mingzhong = 10000;
            pInst->maxHp *= 8;
            pInst->addData = 50;
            pInst->hp = pInst->maxHp;
            pInst->SetCeLue(SUN_SHI_NAIJIU);
            pFight->AddMonster(ptr,5);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    pFight->SetJLType(EFJLType9);
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYW6(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(6);
        pFight->SetFightType(CFight::EFTGuiYu);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 1;
        uint8 level = AddUserToFight(pFight,pUser,&num);
        level = min((int)level,60);
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 34;
            pInst->name = "万毒妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 2*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 10;
            pInst->daohang = level*level*level*4;//0x7ffffff;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "让你尝尝剧毒的厉害。";
            pInst->daohang = 0x7ffffff;
            pInst->AddSkill(57,level*2/3);
            pInst->SetCeLue(ONLY_DU_57);
            pFight->AddMonster(ptr,3);
        }
        if(num > 1)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 34;
            pInst->name = "万毒妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 2*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 10;
            pInst->hp = pInst->maxHp;
            pInst->AddSkill(57,level*2/3);
            pInst->daohang = level*level*level*4;//0x7ffffff;
            pInst->SetCeLue(ONLY_DU_57);
            pFight->AddMonster(ptr,4);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    pFight->SetJLType(EFJLType9);
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYW7(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(7);
        pFight->SetFightType(CFight::EFTGuiYu);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 1;
        uint8 level = AddUserToFight(pFight,pUser,&num);
        level = min((int)level,60);
        if(num > 1)
            num = 6;
        for(uint8 i = 1; i <= num; i++)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 29;
            pInst->type = EMTNormal;
            pInst->name = "阴月妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 6*level;//力量
            pInst->lingxing = 6*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->AddSkill(22,level);
            pInst->SetCeLue(ONLY_SKILL_22);
            pInst->maxHp *= 5;
            pInst->hp = pInst->maxHp;
            if(i == 3)
                pInst->chatMsg = "别让我抓到你。";
            pFight->AddMonster(ptr,i);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    pFight->SetJLType(EFJLType9);
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYW8(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(8);
        pFight->SetFightType(CFight::EFTGuiYu);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 1;
        uint8 level = AddUserToFight(pFight,pUser,&num);
        level = min((int)level,60);
        if(num > 1)
            num = 6;
        for(uint8 i = 1; i <= num; i++)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 39;
            pInst->name = "白骨妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 3*level;//力量
            pInst->lingxing = 3*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            
            pInst->baojilv = 500;
            pInst->baojiadd = level*10;
            pInst->AddSkill(21,level/2);
            pInst->maxHp *= 5;
            pInst->hp = pInst->maxHp;
            if(i == 3)
                pInst->chatMsg = "防得住我的攻击吗？";
            pFight->AddMonster(ptr,i);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    pFight->SetJLType(EFJLType9);
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYW9(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(9);
        pFight->SetFightType(CFight::EFTGuiYu);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 1;
        uint8 level = AddUserToFight(pFight,pUser,&num);
        level = min((int)level,60);
        if(num > 1)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 10;
            pInst->name = "双面妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 7*level;
            pInst->liliang = 7*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 8;
            pInst->speed = 0x7ffffff;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "双妖合力，天下无敌。";
            pInst->daohang = 0x7ffffff;
            pInst->AddSkill(156,10000);
            pInst->AddSkill(157,10000);
            pInst->addData = 0;
            pInst->SetCeLue(ONLY_QIANKUN_SHENLONG);
            pFight->AddMonster(ptr,3);
        }
        
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 10;
            pInst->name = "双面妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 7*level;
            pInst->liliang = 7*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 8;
            pInst->speed = 0x7ffffff;
            pInst->hp = pInst->maxHp;
            pInst->daohang = 0x7ffffff;
            pInst->AddSkill(156,10000);
            pInst->AddSkill(157,10000);
            pInst->addData = 1;
            pInst->SetCeLue(ONLY_QIANKUN_SHENLONG);
            pFight->AddMonster(ptr,4);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    pFight->SetJLType(EFJLType9);
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYW10(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(10);
        pFight->SetFightType(CFight::EFTGuiYu);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 num = 1;
        uint8 level = AddUserToFight(pFight,pUser,&num);
        level = min((int)level,60);
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 41;
            pInst->name = "凶灵妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 7*level;
            pInst->liliang = 7*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = 8*level;//敏捷
            pInst->daohang = level*level*level;
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 3;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "你打的中我吗？";
            pInst->SetCeLue(DI_MING_ZHONG);
            pInst->addData = 20;
            pInst->quanKang = level*100;
            pFight->AddMonster(ptr,3);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    pFight->SetJLType(EFJLType9);
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoPT1(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(8);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 50;
        SMonsterInst *pInst = new SMonsterInst;
        pInst->tmplId = 14;
        pInst->name = "天魅妖";
        ShareMonsterPtr ptr(pInst);
        pInst->level = level;
        pInst->liliang = 7*level;//力量
        pInst->lingxing = 7*level;//灵性
        pInst->tizhi = level;//体质
        pInst->naili = level;//耐力
        pInst->minjie = level;//敏捷
        InitMonsterByZhaoTao(pInst);
        pInst->speed *= 5;
        pInst->recovery = level*50;
        
        pInst->SetCeLue(ONLY_HUN_LUAN);
        
        pInst->maxHp *= 24;
        pInst->hp = pInst->maxHp;
        pInst->AddSkill(61,60);
        pInst->chatMsg = "能挡住我的魅惑吗？";
        pFight->AddMonster(ptr,3);
        pFight->BeginFight(m_socketServer,this);
    }
    
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoPT2(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(2);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 50;
        for(uint8 i = 1; i <= 6; i++)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 24;
            pInst->name = "血爆妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 2*level;//力量
            pInst->lingxing = 2*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            
            pInst->maxHp = 30;
            pInst->hp = pInst->maxHp;
            pInst->recovery = 0x7fffffff;
            pInst->addData = 20;
            pInst->SetCeLue(CL_ZI_BAO);
            if(i == 3)
                pInst->chatMsg = "以血为引，粉身碎骨。";
            pFight->AddMonster(ptr,i);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoPT3(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(3);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 50;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 41;
            pInst->name = "凶灵妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 9*level;
            pInst->liliang = 21*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = 8*level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->attack = 10000;
            pInst->speed *= 5;
            pInst->recovery = level*50;
            pInst->maxHp *= 9;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "你打的中我吗？";
            pInst->SetCeLue(DI_MING_ZHONG);
            pInst->addData = 20;
            pInst->quanKang = level*500;
            pFight->AddMonster(ptr,3);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoPT4(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(4);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 50;
        SMonsterInst *pInst = new SMonsterInst;
        pInst->tmplId = 45;
        pInst->name = "金刚妖";
        ShareMonsterPtr ptr(pInst);
        pInst->level = level;
        pInst->liliang = 13*level;//力量
        pInst->lingxing = 9*level;//灵性
        pInst->tizhi = level;//体质
        pInst->naili = level;//耐力
        pInst->minjie = level;//敏捷
        InitMonsterByZhaoTao(pInst);
        pInst->recovery = level*10;
        pInst->maxHp *= 60;
        pInst->faShuFanTanLv = 100;
        pInst->faShuFanTan = 20;
    
        pInst->fanshang = 100;//
        pInst->fanshangadd = 20;//
        pInst->hp = pInst->maxHp;
        pInst->chatMsg = "我是打不死的。";
        pFight->AddMonster(ptr,3);
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoPT5(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(5);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 50;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 46;
            pInst->name = "龌龊妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 13*level;
            pInst->liliang = 13*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->mingzhong = 10000;
            pInst->maxHp *= 12;
            pInst->recovery = level*10;
            pInst->hp = pInst->maxHp;
            pInst->addData = 100;
            pInst->SetCeLue(SUN_SHI_NAIJIU);
            pFight->AddMonster(ptr,1);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 46;
            pInst->name = "龌龊妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 13*level;
            pInst->liliang = 13*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->mingzhong = 10000;
            pInst->recovery = level*10;
            pInst->maxHp *= 12;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "我能毁坏你的装备。";
            pInst->addData = 100;
            pInst->SetCeLue(SUN_SHI_NAIJIU);
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 46;
            pInst->name = "龌龊妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 13*level;
            pInst->liliang = 13*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->mingzhong = 10000;
            pInst->maxHp *= 12;
            pInst->addData = 100;
            pInst->recovery = level*10;
            pInst->hp = pInst->maxHp;
            pInst->SetCeLue(SUN_SHI_NAIJIU);
            pFight->AddMonster(ptr,5);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoPT6(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(6);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 50;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 34;
            pInst->name = "万毒妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 2*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 30;
            pInst->speed *= 5;
            pInst->recovery = level*50;
            pInst->daohang = 0x7ffffff;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "让你尝尝剧毒的厉害。";
            pInst->daohang = 0x7ffffff;
            pInst->AddSkill(57,60);
            pInst->SetCeLue(ONLY_DU_57);
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 34;
            pInst->name = "万毒妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 2*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 30;
            pInst->speed *= 5;
            pInst->recovery = level*50;
            pInst->daohang = 0x7ffffff;
            pInst->hp = pInst->maxHp;
            pInst->AddSkill(57,60);
            pInst->daohang = level*level*level*4;//0x7ffffff;
            pInst->SetCeLue(ONLY_DU_57);
            pFight->AddMonster(ptr,4);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoPT7(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(7);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 50;
        for(uint8 i = 1; i <= 6; i++)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 29;
            pInst->type = EMTNormal;
            pInst->name = "阴月妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 13*level;//力量
            pInst->lingxing = level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = 70;//level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->AddSkill(22,2*level);
            pInst->SetCeLue(ONLY_SKILL_22);
            pInst->maxHp *= 15;
            pInst->recovery = level*50;
            pInst->hp = pInst->maxHp;
            if(i == 3)
                pInst->chatMsg = "别让我抓到你。";
            pFight->AddMonster(ptr,i);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoPT8(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(1);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 50;
        for(uint8 i = 1; i <= 6; i++)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 39;
            pInst->name = "白骨妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 7*level;//力量
            pInst->lingxing = 7*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->mingzhong = 10000;
            pInst->baojilv = 500;
            pInst->baojiadd = level*40;
            //pInst->AddSkill(21,level/2);
            pInst->maxHp *= 15;
            pInst->recovery = level*50;
            pInst->hp = pInst->maxHp;
            if(i == 3)
                pInst->chatMsg = "防得住我的攻击吗？";
            pFight->AddMonster(ptr,i);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoPT9(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(9);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 50;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 10;
            pInst->name = "双面妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 9*level;
            pInst->liliang = 21*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 24;
            pInst->speed = 0x7ffffff;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "双妖合力，天下无敌。";
            pInst->daohang = 0x7ffffff;
            pInst->AddSkill(156,10000);
            pInst->AddSkill(157,10000);
            pInst->addData = 0;
            pInst->attack = 10000;
            pInst->SetCeLue(ONLY_QIANKUN_SHENLONG);
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 10;
            pInst->name = "双面妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 9*level;
            pInst->liliang = 21*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->attack = 10000;
            pInst->maxHp *= 24;
            pInst->speed = 0x7ffffff;
            pInst->hp = pInst->maxHp;
            pInst->daohang = 0x7ffffff;
            pInst->AddSkill(156,10000);
            pInst->AddSkill(157,10000);
            pInst->addData = 1;
            pInst->SetCeLue(ONLY_QIANKUN_SHENLONG);
            pFight->AddMonster(ptr,4);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoPT10(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(10);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 50;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 44;
            pInst->name = "蛊魔妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 17*level;
            pInst->liliang = 7*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7fffffff;
            pInst->recovery = level*50;
            pInst->SetCeLue(ONLY_PET);
            pInst->AddSkill(3,2*level);
            pInst->AddSkill(7,2*level);
            pInst->AddSkill(11,2*level);
            pInst->AddSkill(15,2*level);
            pInst->AddSkill(19,2*level);
            pInst->maxHp *= 15;
            pInst->attack = 7000;
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,1);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 44;
            pInst->name = "蛊魔妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 17*level;
            pInst->liliang = 7*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7fffffff;
            pInst->recovery = level*50;
            pInst->SetCeLue(ONLY_PET);
            pInst->AddSkill(3,2*level);
            pInst->AddSkill(7,2*level);
            pInst->AddSkill(11,2*level);
            pInst->AddSkill(15,2*level);
            pInst->AddSkill(19,2*level);
            pInst->maxHp *= 15;
            pInst->attack = 7000;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "人是伤不了我的。";
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 44;
            pInst->name = "蛊魔妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 17*level;
            pInst->liliang = 7*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7fffffff;
            pInst->SetCeLue(ONLY_PET);
            pInst->AddSkill(3,2*level);
            pInst->AddSkill(7,2*level);
            pInst->AddSkill(11,2*level);
            pInst->AddSkill(15,2*level);
            pInst->AddSkill(19,2*level);
            pInst->recovery = level*50;
            pInst->maxHp *= 15;
            pInst->attack = 7000;
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,5);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    pFight->SetJLType(EFJLType10);
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYX1(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(8);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        SMonsterInst *pInst = new SMonsterInst;
        pInst->tmplId = 14;
        pInst->name = "天魅妖";
        ShareMonsterPtr ptr(pInst);
        pInst->level = level;
        pInst->liliang = 11*level;//力量
        pInst->lingxing = 11*level;//灵性
        pInst->tizhi = level;//体质
        pInst->naili = level;//耐力
        pInst->minjie = level;//敏捷
        InitMonsterByZhaoTao(pInst);
        pInst->daohang = (int)(pInst->daohang*2.2);
        pInst->SetCeLue(ONLY_HUN_LUAN);
        pInst->speed *= 20;
        pInst->recovery = level*100;
        pInst->maxHp *= 60;
        pInst->hp = pInst->maxHp;
        pInst->AddSkill(61,200);
        pInst->chatMsg = "能挡住我的魅惑吗？";
        pInst->fanshang = 100;//
        pInst->fanshangadd = 30;//
        pFight->AddMonster(ptr,3);
        pFight->BeginFight(m_socketServer,this);
    }
    //pFight->SetJLType(EFJLType11);
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYX2(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(2);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        for(uint8 i = 1; i <= 6; i++)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 24;
            pInst->name = "血爆妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 2*level;//力量
            pInst->lingxing = 2*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang *= 2;
            pInst->maxHp = 30;
            pInst->hp = pInst->maxHp;
            pInst->recovery = 0x7fffffff;
            pInst->addData = 40;
            //pInst->noAdd = true;
            pInst->SetCeLue(CL_ZI_BAO);
            if(i == 3)
                pInst->chatMsg = "以血为引，粉身碎骨。";
            pFight->AddMonster(ptr,i);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYX3(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(3);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 41;
            pInst->name = "凶灵妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 7*level;
            pInst->liliang = 7*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = 800*level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 20;
            pInst->daohang *= 2;
            pInst->speed *= 10;
            pInst->recovery = level*100;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "抵抗法术是我的专长！";
            pInst->AddSkill(152,180);
            pInst->AddSkill(19,level);
            pInst->onlySkill = true;
            pInst->SetCeLue(DI_MING_ZHONG);
            pInst->addData = 10;
            pInst->fashubaojilv = 500;
            pInst->fashubaojiadd = 10000;
            pInst->quanKang = level*1000;
            pFight->AddMonster(ptr,3);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYX4(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(4);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        SMonsterInst *pInst = new SMonsterInst;
        pInst->tmplId = 45;
        pInst->name = "金刚妖";
        ShareMonsterPtr ptr(pInst);
        pInst->level = level;
        pInst->liliang = 15*level;//力量
        pInst->lingxing = 10*level;//灵性
        pInst->tizhi = level;//体质
        pInst->naili = level;//耐力
        pInst->minjie = level;//敏捷
        InitMonsterByZhaoTao(pInst);
        pInst->daohang *= 2;
        pInst->speed *= 20;
        pInst->recovery = level*10;
        pInst->maxHp *= 100;
        
        pInst->faShuFanTanLv = 100;
        pInst->faShuFanTan = 50;
    
        pInst->fanshang = 100;//
        pInst->fanshangadd = 50;//
        
        pInst->hp = pInst->maxHp;
        pInst->chatMsg = "我是打不死的。";
        pFight->AddMonster(ptr,3);
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYX5(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(5);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 46;
            pInst->name = "龌龊妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 17*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            pInst->daohang = level*level*level/2;
            InitMonsterByZhaoTao(pInst);
            pInst->mingzhong = 10000;
            pInst->daohang *= 2;
            pInst->speed *= 20;
            pInst->recovery = level*200;
            pInst->maxHp *= 15;
            pInst->hp = pInst->maxHp;
            pInst->SetCeLue(SUN_SHI_NAIJIU);
            pInst->addData = 500;
            pFight->AddMonster(ptr,1);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 46;
            pInst->name = "龌龊妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 17*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            pInst->daohang = level*level*level/2;
            InitMonsterByZhaoTao(pInst);
            pInst->mingzhong = 10000;
            pInst->daohang *= 2;
            pInst->speed *= 20;
            pInst->recovery = level*200;
            pInst->addData = 500;
            pInst->maxHp *= 15;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "我能毁坏你的装备。";
            pInst->SetCeLue(SUN_SHI_NAIJIU);
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 46;
            pInst->name = "龌龊妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 17*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            pInst->daohang = level*level*level/2;
            InitMonsterByZhaoTao(pInst);
            pInst->mingzhong = 10000;
            pInst->daohang *= 2;
            pInst->speed *= 20;
            pInst->recovery = level*200;
            pInst->maxHp *= 15;
            pInst->addData = 500;
            pInst->SetCeLue(SUN_SHI_NAIJIU);
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,5);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYX6(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(6);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 34;
            pInst->name = "万毒妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 2*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 50;
            pInst->daohang = level*level*level*4;//0x7ffffff;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "让你尝尝剧毒的厉害。";
            pInst->daohang = 0x7ffffff;
            pInst->speed *= 20;
            pInst->recovery = level*100;
            pInst->AddSkill(57,90);
            pInst->SetCeLue(ONLY_DU_57);
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 34;
            pInst->name = "万毒妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = level;
            pInst->liliang = 2*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 50;
            pInst->hp = pInst->maxHp;
            pInst->daohang = 0x7ffffff;
            pInst->speed *= 20;
            pInst->recovery = level*100;
            pInst->AddSkill(57,90);
            pInst->SetCeLue(ONLY_DU_57);
            pFight->AddMonster(ptr,4);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYX7(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(7);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        for(uint8 i = 1; i <= 6; i++)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 29;
            pInst->type = EMTNormal;
            pInst->name = "阴月妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 11*level;//力量
            pInst->lingxing = 16*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->attack = 40000;
            pInst->AddSkill(22,100);
            pInst->AddSkill(24,100);
            pInst->daohang *= 2;
            pInst->speed *= 20;
            pInst->recovery = level*100;
            pInst->SetCeLue(ONLY_SKILL_22);
            pInst->maxHp *= 25;
            pInst->hp = pInst->maxHp;
            if(i == 3)
                pInst->chatMsg = "别让我抓到你。";
            pFight->AddMonster(ptr,i);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYX8(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(1);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        for(uint8 i = 1; i <= 6; i++)
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 39;
            pInst->name = "白骨妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 500*level;//力量
            pInst->lingxing = level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            
            pInst->attack = 10000;
            pInst->daohang *= 2;
            pInst->speed *= 20;
            pInst->recovery = level*100;
                        
            pInst->baojilv = 500;
            pInst->baojiadd = level*50;
            //pInst->AddSkill(21,90);
            pInst->maxHp *= 25;
            pInst->hp = pInst->maxHp;
            if(i == 3)
                pInst->chatMsg = "防得住我的攻击吗？";
            pFight->AddMonster(ptr,i);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYX9(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(9);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 10;
            pInst->name = "双面妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 11*level;
            pInst->liliang = 11*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 60;
            pInst->speed = 0x7ffffff;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "双妖合力，天下无敌。";
            pInst->daohang = 0x7ffffff;
            pInst->speed = 0x7ffffff;
            pInst->recovery = level*50;
            pInst->AddSkill(156,10000);
            pInst->AddSkill(157,1000);
            pInst->AddSkill(19,level);
            pInst->addData = 0;
            pInst->AddSkill(152,180);
            pInst->faShuFanTanLv = 100;
            pInst->faShuFanTan = 30;
            pInst->fanshang = 100;//
            pInst->fanshangadd = 30;
            pInst->fashubaojilv = 500;
            pInst->fashubaojiadd = 10000;

            pInst->SetCeLue(ONLY_QIANKUN_SHENLONG_YX);
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 10;
            pInst->name = "双面妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 11*level;
            pInst->liliang = 11*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 60;
            pInst->speed = 0x7ffffff;
            pInst->recovery = level*50;
            pInst->hp = pInst->maxHp;
            pInst->daohang = 0x7ffffff;
            pInst->AddSkill(156,10000);
            pInst->AddSkill(157,10000);
            pInst->AddSkill(152,180);
            pInst->AddSkill(19,level);
            pInst->faShuFanTanLv = 100;;
            pInst->faShuFanTan = 30;
            pInst->fanshang = 100;//
            pInst->fanshangadd = 30;
            pInst->addData = 1;
            pInst->fashubaojilv = 500;
            pInst->fashubaojiadd = 10000;
            pInst->SetCeLue(ONLY_QIANKUN_SHENLONG_YX);
            pFight->AddMonster(ptr,4);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::ShiYaoYX10(CUser *pU,int type)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(10);
        pFight->SetFightType((CFight::EFightType)type);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 44;
            pInst->name = "蛊魔妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 15*level;
            pInst->liliang = 11*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7fffffff;
            pInst->SetCeLue(ONLY_PET);
            pInst->AddSkill(3,level);
            pInst->AddSkill(7,level);
            pInst->AddSkill(11,level);
            pInst->AddSkill(15,level);
            pInst->AddSkill(19,level);
            pInst->onlySkill = true;
            pInst->maxHp *= 25;
            pInst->recovery = level*100;
            pInst->fashubaojilv = 100;
            pInst->fashubaojiadd = 3000;
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,1);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 44;
            pInst->name = "蛊魔妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 15*level;
            pInst->liliang = 11*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7fffffff;
            pInst->SetCeLue(ONLY_PET);
            pInst->AddSkill(3,level);
            pInst->AddSkill(7,level);
            pInst->AddSkill(11,level);
            pInst->AddSkill(15,level);
            pInst->AddSkill(19,level);
            pInst->maxHp *= 25;
            pInst->onlySkill = true;
            pInst->recovery = level*100;
            pInst->fashubaojilv = 100;
            pInst->fashubaojiadd = 3000;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "人是伤不了我的。";
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 44;
            pInst->name = "蛊魔妖";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 15*level;
            pInst->liliang = 11*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7fffffff;
            pInst->SetCeLue(ONLY_PET);
            pInst->AddSkill(3,level);
            pInst->AddSkill(7,level);
            pInst->AddSkill(11,level);
            pInst->AddSkill(15,level);
            pInst->AddSkill(19,level);
            pInst->maxHp *= 25;
            pInst->onlySkill = true;
            pInst->recovery = level*100;
            pInst->fashubaojilv = 100;
            pInst->fashubaojiadd = 3000;
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,5);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    pFight->SetJLType(EFJLType11);
    m_fightManager.AddFight(pFight);
}

void CScene::NuYanQiLing(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetFightType(CFight::EFTScript);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 75;
        SMonsterInst *pInst = new SMonsterInst;
        pInst->tmplId = 102;
        pInst->name = "怒焰金麟";
        ShareMonsterPtr ptr(pInst);
        pInst->level = level;
        pInst->liliang = level;//力量
        pInst->lingxing = 24*level;//灵性
        pInst->tizhi = level;//体质
        pInst->naili = level;//耐力
        pInst->minjie = level;//敏捷
        InitMonsterByZhaoTao(pInst);
        pInst->daohang *= 10;
        pInst->speed *= 10;
        pInst->recovery += 10000;
        pInst->quanKang = 5000;
        pInst->fashubaojilv = 500;//法术爆击率
        pInst->fashubaojiadd = 3000;//法术爆击追加
        pInst->maxHp *= 50;
        pInst->hp = pInst->maxHp;
        pInst->AddSkill(3,100);
        pFight->AddMonster(ptr,3);
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::XianBing1(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetFightType(CFight::EFTScript);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 45;
            pInst->name = "仙界追兵";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = 21*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang *= 5;
            pInst->speed *= 10;
            pInst->recovery += 10000;
            pInst->maxHp *= 15;
            pInst->AddSkill(3,30);
            pInst->AddSkill(7,30);
            pInst->AddSkill(11,30);
            pInst->AddSkill(15,30);
            pInst->AddSkill(19,30);
            pInst->fashubaojilv = 200;//法术爆击率
            pInst->fashubaojiadd = 2000;//法术爆击追加
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,3);
        }        
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 45;
            pInst->name = "仙界追兵";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = 21*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang *= 5;
            pInst->speed *= 10;
            pInst->recovery += 10000;
            pInst->AddSkill(3,30);
            pInst->AddSkill(7,30);
            pInst->AddSkill(11,30);
            pInst->AddSkill(15,30);
            pInst->AddSkill(19,30);
            pInst->fashubaojilv = 200;//法术爆击率
            pInst->fashubaojiadd = 2000;//法术爆击追加
            pInst->maxHp *= 15;
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,4);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 102;
            pInst->name = "怒焰金麟";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->recovery = 0x7ffffff;
            pInst->maxHp = 5;
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,7);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::XianBing2(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetFightType(CFight::EFTScript);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 45;
            pInst->name = "仙界将军";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 6*level;//力量
            pInst->lingxing = 11*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang *= 2;
            pInst->speed *= 10;
            pInst->maxHp *= 20;
            pInst->fanshang = 500;
            pInst->AddSkill(3,30);
            pInst->AddSkill(7,30);
            pInst->AddSkill(11,30);
            pInst->AddSkill(15,30);
            pInst->AddSkill(19,30);
            pInst->hp = pInst->maxHp;
            pInst->fanshang = 100;
            pInst->fanshangadd = 30;
            pInst->faShuFanTanLv = 100;
            pInst->faShuFanTan = 30;
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 45;
            pInst->name = "仙界将军";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 6*level;//力量
            pInst->lingxing = 11*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang *= 2;
            pInst->speed *= 10;
            pInst->maxHp *= 30;
            pInst->AddSkill(3,30);
            pInst->AddSkill(7,30);
            pInst->AddSkill(11,30);
            pInst->AddSkill(15,30);
            pInst->AddSkill(19,30);
            
            pInst->hp = pInst->maxHp;
            pInst->fanshang = 100;
            pInst->fanshangadd = 30;
            pInst->faShuFanTanLv = 100;
            pInst->faShuFanTan = 30;
            pFight->AddMonster(ptr,4);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 102;
            pInst->name = "怒焰金麟";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->recovery = 0x7ffffff;
            pInst->maxHp = 5;
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,7);
        }
                
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::YaoBing1(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetFightType(CFight::EFTScript);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 39;
            pInst->name = "妖界追兵";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = 21*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang *= 5;
            pInst->speed *= 10;
            pInst->recovery += 10000;
            pInst->maxHp *= 15;
            pInst->AddSkill(3,30);
            pInst->AddSkill(7,30);
            pInst->AddSkill(11,30);
            pInst->AddSkill(15,30);
            pInst->AddSkill(19,30);
            pInst->fashubaojilv = 200;//法术爆击率
            pInst->fashubaojiadd = 2000;//法术爆击追加
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,3);
        }        
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 39;
            pInst->name = "妖界追兵";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = 21*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang *= 5;
            pInst->speed *= 10;
            pInst->recovery += 10000;
            pInst->maxHp *= 15;
            pInst->AddSkill(3,30);
            pInst->AddSkill(7,30);
            pInst->AddSkill(11,30);
            pInst->AddSkill(15,30);
            pInst->AddSkill(19,30);
            pInst->fashubaojilv = 200;//法术爆击率
            pInst->fashubaojiadd = 2000;//法术爆击追加
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,4);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 102;
            pInst->name = "怒焰金麟";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->recovery = 0x7ffffff;
            pInst->maxHp = 5;
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,7);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::YaoBing2(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetFightType(CFight::EFTScript);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 39;
            pInst->name = " 妖界将军";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 6*level;//力量
            pInst->lingxing = 11*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang *= 2;
            pInst->speed *= 10;
            pInst->maxHp *= 20;
            pInst->fanshang = 500;
            pInst->AddSkill(3,30);
            pInst->AddSkill(7,30);
            pInst->AddSkill(11,30);
            pInst->AddSkill(15,30);
            pInst->AddSkill(19,30);
            pInst->hp = pInst->maxHp;
            pInst->fanshang = 100;
            pInst->fanshangadd = 30;
            pInst->faShuFanTanLv = 100;
            pInst->faShuFanTan = 30;
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 39;
            pInst->name = " 妖界将军";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 6*level;//力量
            pInst->lingxing = 11*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang *= 2;
            pInst->speed *= 10;
            pInst->maxHp *= 20;
            pInst->fanshang = 500;
            pInst->AddSkill(3,30);
            pInst->AddSkill(7,30);
            pInst->AddSkill(11,30);
            pInst->AddSkill(15,30);
            pInst->AddSkill(19,30);
            pInst->hp = pInst->maxHp;
            pInst->fanshang = 100;
            pInst->fanshangadd = 30;
            pInst->faShuFanTanLv = 100;
            pInst->faShuFanTan = 30;
            pFight->AddMonster(ptr,4);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 102;
            pInst->name = "怒焰金麟";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->recovery = 0x7ffffff;
            pInst->maxHp = 5;
            pInst->hp = pInst->maxHp;
            pFight->AddMonster(ptr,7);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::MenPaiChuanSong(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetFightType(CFight::EFTScript);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 80;
        SMonsterInst *pInst = new SMonsterInst;
        pInst->tmplId = 47;
        pInst->name = "门派传送人";
        ShareMonsterPtr ptr(pInst);
        pInst->level = level;
        pInst->liliang = 11*level;//力量
        pInst->lingxing = 25*level;//灵性
        pInst->tizhi = level;//体质
        pInst->naili = level;//耐力
        pInst->minjie = level;//敏捷
        InitMonsterByZhaoTao(pInst);
        pInst->tmplId = 57;
        pInst->xiang = pU->GetXiang();
        pInst->daohang = 0x7ffffff;
        pInst->speed *= 10;
        pInst->recovery += 5000;
        pInst->maxHp *= 100;
        pInst->hp = pInst->maxHp;
        
        if(pInst->xiang == EXJinXiang)
        {
            pInst->AddSkill(3,100);
            pInst->AddSkill(4,100);
        }
        else if(pInst->xiang == EXMuXiang)
        {
            pInst->AddSkill(7,100);
            pInst->AddSkill(8,100);
        }
        else if(pInst->xiang == EXShuiXiang)
        {
            pInst->AddSkill(11,100);
            pInst->AddSkill(12,100);
        }
        else if(pInst->xiang == EXHuoXiang)
        {
            pInst->AddSkill(15,100);
            pInst->AddSkill(16,100);
        }
        else if(pInst->xiang == EXTuXiang)
        {
            pInst->AddSkill(19,100);
            pInst->AddSkill(20,100);
        }
        pInst->faShuFanTanLv = 40;
        pInst->faShuFanTan = 100;
        pInst->fashubaojilv = 50;//法术爆击率
        pInst->fashubaojiadd = 100;//法术爆击追加
        pInst->fanshang = 100;
        pInst->fanshangadd = 40;
        pFight->AddMonster(ptr,3);
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::QingLongBaoBao(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetFightType(CFight::EFTScript);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 80;
        SMonsterInst *pInst = new SMonsterInst;
        pInst->tmplId = 50;
        pInst->name = "青龙宝宝";
        ShareMonsterPtr ptr(pInst);
        pInst->level = level;
        pInst->liliang = level;//力量
        pInst->lingxing = 21*level;//灵性
        pInst->tizhi = level;//体质
        pInst->naili = level;//耐力
        pInst->minjie = level;//敏捷
        InitMonsterByZhaoTao(pInst);
        pInst->daohang = (int)(pInst->daohang*1.5);
        pInst->speed *= 10;
        pInst->recovery += 15000;
        pInst->maxHp *= 70;
        pInst->hp = pInst->maxHp;
        pInst->AddSkill(3,100);
        pInst->AddSkill(7,100);
        pInst->AddSkill(11,100);
        pInst->AddSkill(15,100);
        pInst->AddSkill(19,100);
        pInst->AddSkill(53,100);
        pInst->AddSkill(61,100);
        pInst->SetCeLue(SHI_YONG_53_61);
        
        pInst->fashubaojilv = 80;
        pInst->fashubaojiadd = 3000;
        pInst->fanshang = 100;//
        pInst->fanshangadd = 50;//
        pFight->AddMonster(ptr,3);
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

void CScene::KuLouDaWang(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetFightType(CFight::EFTScript);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 100;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 39;
            pInst->name = "骷髅大王";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 100*level;//力量
            pInst->lingxing = level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang *= 10;
            pInst->speed *= 10;
            pInst->recovery += 12000;
            pInst->maxHp *= 100;
            pInst->hp = pInst->maxHp;
            pInst->AddSkill(22,100);
            pInst->AddSkill(156,3000);
            pInst->attack = 17000;
            pInst->quanKang = 3000;
            pInst->baojilv = 60;
            pInst->baojiadd = 5000;
            pInst->fanJiLv = 60;
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 39;
            pInst->name = "喽罗";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 50*level;//力量
            pInst->lingxing = 9*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang *= 10;
            pInst->speed *= 10;
            pInst->recovery += 5000;
            pInst->maxHp *= 30;
            pInst->hp = pInst->maxHp;
            pInst->AddSkill(111,400);
            pInst->quanKang = 3000;
            pInst->baojilv = 100;
            pInst->baojiadd = 1000;
            pInst->faShuFanTanLv = 100;
            pInst->faShuFanTan = 50;    
            pInst->fanshang = 100;//
            pInst->fanshangadd = 50;//
            pInst->attack = 15000;
            pInst->SetCeLue(USE_SKILL_111);
            pFight->AddMonster(ptr,1);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 39;
            pInst->name = "喽罗";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = 50*level;//力量
            pInst->lingxing = 9*level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->attack = 15000;
            pInst->daohang *= 10;
            pInst->speed *= 10;
            pInst->recovery += 5000;
            pInst->maxHp *= 30;
            pInst->hp = pInst->maxHp;
            pInst->AddSkill(111,400);
            pInst->quanKang = 3000;
            pInst->baojilv = 100;
            pInst->baojiadd = 1000;
            pInst->faShuFanTanLv = 100;
            pInst->faShuFanTan = 50;    
            pInst->fanshang = 100;//
            pInst->fanshangadd = 50;//
            pInst->SetCeLue(USE_SKILL_111);
            pFight->AddMonster(ptr,5);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

//仙人（剧情）
void CScene::JuQingXianRen(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetFightType(CFight::EFTScript);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 45;
            pInst->name = "仙人";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7ffffff;
            pInst->speed = 0x7fffffff;
            pInst->recovery += 10000;
            pInst->maxHp *= 200;
            pInst->hp = pInst->maxHp;
            pInst->baojilv = 100;
            pInst->baojiadd = 10000;
            pInst->SetCeLue(FIRST_QI_LING_MONSTER);
            pFight->AddMonster(ptr,7);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 102;
            pInst->name = "麒麟";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7ffffff;
            pInst->speed = 0x7fffffff;
            pInst->recovery += 10000;
            pInst->maxHp *= 200;
            pInst->AddSkill(3,100);
            pInst->hp = pInst->maxHp;
            pInst->SetCeLue(FIRST_XIAN_REN_MONSTER);
            pFight->AddMonster(ptr,3);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

//麒麟（剧情）
void CScene::JuQingQiLing(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetFightType(CFight::EFTScript);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 45;
            pInst->name = "仙人";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7ffffff;
            pInst->speed = 0x7fffffff;
            pInst->recovery += 10000;
            pInst->maxHp *= 200;
            pInst->hp = pInst->maxHp;
            pInst->baojilv = 100;
            pInst->baojiadd = 10000;
            pInst->SetCeLue(FIRST_QI_LING_MONSTER);
            pFight->AddMonster(ptr,3);
        }
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 102;
            pInst->name = "麒麟";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->liliang = level;//力量
            pInst->lingxing = level;//灵性
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->daohang = 0x7ffffff;
            pInst->speed = 0x7fffffff;
            pInst->recovery += 10000;
            pInst->maxHp *= 200;
            pInst->hp = pInst->maxHp;
            pInst->AddSkill(3,100);
            pInst->SetCeLue(FIRST_XIAN_REN_MONSTER);
            pFight->AddMonster(ptr,7);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}

//冥妖王（剧情）
//血量为50倍，物理反伤50%，每5回合使用一次，三阶法术，等级200级，法爆+500，暴击追加+3000（同蛊魔妖的技能相同）
void CScene::JuQingMingYao(CUser *pU)
{
    if(pU->GetFightId() != 0)
        return;
    ShareUserPtr pUser;
    ShareFightPtr pFight;
    if(!InitEpisodeBattle(pU,pFight,pUser))
        return;
    {
        pFight->SetVisibleMonsterId(10);
        pFight->SetFightType(CFight::EFTScript);
        boost::recursive_mutex::scoped_lock lk(m_mutex);
        uint8 level = AddUserToFight(pFight,pUser);
        level = 70;
        {
            SMonsterInst *pInst = new SMonsterInst;
            pInst->tmplId = 41;
            pInst->name = "冥妖王";
            ShareMonsterPtr ptr(pInst);
            pInst->level = level;
            pInst->lingxing = 7*level;
            pInst->liliang = 7*level;
            pInst->tizhi = level;//体质
            pInst->naili = level;//耐力
            pInst->minjie = 800*level;//敏捷
            InitMonsterByZhaoTao(pInst);
            pInst->maxHp *= 30;
            pInst->daohang *= 2;
            pInst->speed *= 10;
            pInst->recovery = level*100;
            pInst->hp = pInst->maxHp;
            pInst->chatMsg = "除了麒麟我什么都不怕";
            pInst->AddSkill(152,180);
            pInst->AddSkill(19,level);
            pInst->fashubaojilv = 100;//法术爆击率
            pInst->fashubaojiadd = 10000;//法术爆击追加
            pInst->fanshang = 100;//
            pInst->fanshangadd = 50;//
            pInst->onlySkill = true;
            pInst->SetCeLue(DI_MING_ZHONG_NOT_QILING);
            pInst->addData = 10;
            pInst->quanKang = level*1000;
            pFight->AddMonster(ptr,3);
        }
        pFight->BeginFight(m_socketServer,this);
    }
    m_fightManager.AddFight(pFight);
}
